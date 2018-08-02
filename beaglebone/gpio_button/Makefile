obj-m := gpio-descriptor-module.o gpio-legacy-module.o gpio-legacy-dt-module.o

KERNELDIR ?= /home/kiran/bbb/linux-4.16/

all default: modules
install: modules_install

modules modules_install help clean:
	$(MAKE) -C $(KERNELDIR) M=$(shell pwd) $@
