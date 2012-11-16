KERNELDIR ?=  /lib/modules/`uname -r`/build
PWD := $(shell pwd)
INSTALLDIR ?= ../
CROSS_COMPILE ?=
CC ?= $(CROSS_COMPILE)gcc

obj-m := secbulk.o

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

install: all
	cp *.ko $(INSTALLDIR)

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.markers *.symvers *.order

.PHONY: modules modules_install clean
