#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <pci/pci.h>
#include <sys/io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define INTEL_BRIDGE_DCAEN_OFFSET 	0x64
#define INTEL_BRIDGE_DCAEN_BIT		6
#define PCI_HEADER_TYPE_BRIDGE          1
#define PCI_VENDOR_ID_INTEL   		0x8086 /* lol @ intel */
#define PCI_HEADER_TYPE         	0x0e 
#define MSR_P6_DCA_CAP			0x000001f8

#define NUM_CPUS			4
void check_dca(struct pci_dev *dev)
{
	u32 dca = pci_read_long(dev, INTEL_BRIDGE_DCAEN_OFFSET);
	if (!(dca & (1 << INTEL_BRIDGE_DCAEN_BIT))) {
		printf("DCA disabled, enabling now.\n");
	        dca |= 1 << INTEL_BRIDGE_DCAEN_BIT;
		pci_write_long(dev, INTEL_BRIDGE_DCAEN_OFFSET, dca);	
	} else {
		printf("DCA already enabled!\n");
	}
}

void msr_dca_enable(void)
{
	char msr_file_name[64];
	int fd = 0, i = 0;
	u64 data;

	for (;i < NUM_CPUS; i++) {
		sprintf(msr_file_name, "/dev/cpu/%d/msr", i);
	        fd = open(msr_file_name, O_RDWR);
		if (fd < 0) { 
			perror("open failed!");
			exit(1);
		}
		if (pread(fd, &data, sizeof(data), MSR_P6_DCA_CAP) != sizeof(data)) {
			perror("reading msr failed!");
			exit(1);
		}

		printf("got msr value: %*llx\n", 1, (unsigned long long)data);
		if (!(data & 1)) {
			data |= 1;
			if (pwrite(fd, &data, sizeof(data), MSR_P6_DCA_CAP) != sizeof(data)) {
				perror("writing msr failed!");
				exit(1);
			}
		} else {
			printf("msr already enabled for CPU %d\n", i);
		}
	}
}

int main(void)
{
	struct pci_access *pacc;
	struct pci_dev *dev;
	u8 type;

	pacc = pci_alloc();
	pci_init(pacc);

	pci_scan_bus(pacc);
	for (dev = pacc->devices; dev; dev=dev->next) {
		pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES);
		if (dev->vendor_id == PCI_VENDOR_ID_INTEL) {
		    type = pci_read_byte(dev, PCI_HEADER_TYPE); 
		    if (type == PCI_HEADER_TYPE_BRIDGE) {
			check_dca(dev);
		    }
		}
	}

	msr_dca_enable();
	return 0;
}
