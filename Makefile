ifneq (${NUM_CPUS},)
  CFLAGS  = -Wall -DNUM_CPUS=${NUM_CPUS}
else
  CFLAGS   = -Wall -DNUM_CPUS=4
endif

LDFLAGS = -lpci

all:
	gcc $(CFLAGS) $(LDFLAGS) dca_force.c -o dca_force

clean:
	rm dca_force
