obj-m := uio_netx.o

TMPKDIR := /lib/modules/$(shell uname -r)/build
TMPINSTALLPATH := kernel/drivers/uio/
TMPSYMVERS := /lib/modules/$(shell uname -r)/build/Module.symvers

ifdef KDIR
	ifneq ($KDIR, "")
		TMPKDIR=$(KDIR)
	endif
endif

ifdef SYMVERS
	ifneq ($SYMVERS, "")
		TMPSYMVERS := "$(SYMVERS)"
	endif
endif

ifdef INSTALL_PATH
	ifneq ($INSTALL_PATH, "")
		TMPINSTALLPATH := $(INSTALL_PATH)
	endif
endif

ifdef DISABLE_PCI_SUPPORT
	ccflags-y += -DDISABLE_PCI_SUPPORT
	#TODO: disable silently ???
	#DMA_DISABLE=1
endif

ifndef DMA_DISABLE
	ccflags-y += -DDMA_SUPPORT
endif

PWD := $(shell pwd)

ifndef CIFXDRV_ROOT_PATH
	CIFXDRV_ROOT_PATH=$(PWD)/../
endif

default:
	$(eval VERSION := $(shell $(CIFXDRV_ROOT_PATH)/scripts/drv_version.sh -g))
	@test -n "$(VERSION)" || { echo "Error - uio_netx version can't be determined (set CIFXDRV_ROOT_PATH)!"; exit 1; }

	@echo "#define UIO_NETX_VERSION \"${VERSION}"\" > uio_netx_version.h
	$(MAKE) -C $(TMPKDIR) M=$(PWD) modules

clean:
	rm -f *.o *.ko *.order *.mod* Module.symvers

modules_install:
	$(MAKE) INSTALL_MOD_DIR=$(TMPINSTALLPATH) -C $(TMPKDIR) M=$(PWD) modules_install