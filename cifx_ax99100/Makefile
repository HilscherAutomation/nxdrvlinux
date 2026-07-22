ifneq ($(KERNELRELEASE),)
# kbuild part of makefile

obj-m += ax99100-pci-spi.o
obj-$(CONFIG_AX99100_IRQ) += ax99100-pci-gpio.o

else
# normal makefile

ifeq ($(KDIR),)
	KDIR := /lib/modules/$(shell uname -r)/build
endif

PWD := $(shell pwd)

all: modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

endif
