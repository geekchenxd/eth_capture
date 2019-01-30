obj-m += hello.o

CURRENT = $(shell uname -r)
#KDIR = /lib/modules/$(CURRENT)/
KDIR = /root/work/workspace/git/u4n2/linux-4.9.28/
PWD = $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules;

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- clean;
	rm -rf *.o *.mod.c *.symvers .*.ko.cmd .*.o.cmd
