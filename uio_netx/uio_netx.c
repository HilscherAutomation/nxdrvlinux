// SPDX-License-Identifier: GPL-2.0
/*
 * UIO Hilscher netX card driver
 *
 * (C) 2007 Hans J. Koch <hjk@linutronix.de>
 * (C) 2013 Sebastian Doell <sdoell@hilscher.com> Added DMA Support
 * (C) 2014 Sebastian Doell <sdoell@hilscher.com> Added support for memory mapped
 * devices given by user via command line (e.g. ISA)
 * (C) 2017 Sebastian Doell <sdoell@hilscher.com> Added device-tree support
 * (C) 2019 Sebastian Doell <sdoell@hilscher.com> Added netx4000 support
 *
 */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/uio_driver.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/version.h>
#include <asm/cacheflush.h>
#include <linux/miscdevice.h>

#ifdef CONFIG_OF
	#include <linux/of.h>
	#include <linux/platform_device.h>
	#include <linux/of_address.h>
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
	#define ioremap_nocache ioremap
#endif

#define MAX_USER_CARDS 10

#define UIO_NETX_VERSION "2.2.0"

int           addr_cnt = 0;
unsigned long custom_dpm_len[MAX_USER_CARDS] = {0};
int           len_cnt = 0;
int           custom_irq[MAX_USER_CARDS] = {0};
int           irq_cnt = 0;

#ifdef CONFIG_64BIT
	phys_addr_t custom_dpm_addr[MAX_USER_CARDS] = {0};
	module_param_array(custom_dpm_addr, ullong, &addr_cnt, S_IRUGO);
#else
	unsigned long custom_dpm_addr[MAX_USER_CARDS] = {0};
	module_param_array(custom_dpm_addr, ulong, &addr_cnt, S_IRUGO);
#endif
MODULE_PARM_DESC(custom_dpm_addr, "Start address of DPM (array)");
module_param_array(custom_dpm_len, ulong, &len_cnt, S_IRUGO);
MODULE_PARM_DESC(custom_dpm_len, "Length of DPM (array)");
module_param_array(custom_irq, int, &irq_cnt, S_IRUGO);
MODULE_PARM_DESC(custom_irq, "IRQ number (array)");

#ifdef DMA_SUPPORT
#define DMA_BUFFER_COUNT 1
#define DMA_BUFFER_SIZE  8*8*1024

unsigned long dma_disable = 0;
unsigned long dma_buffer_count = DMA_BUFFER_COUNT;
unsigned long dma_buffer_size = DMA_BUFFER_SIZE;

module_param(dma_disable, ulong, S_IRUGO);
MODULE_PARM_DESC(dma_disable, "Disable DMA buffer allocation.");
module_param(dma_buffer_count, ulong, S_IRUGO);
MODULE_PARM_DESC(dma_buffer_count, "Number of DMA-buffers to use.");
module_param(dma_buffer_size, ulong, S_IRUGO);
MODULE_PARM_DESC(dma_buffer_size, "Size of a DMA-buffer.");
#endif

#define PCI_VENDOR_ID_HILSCHER          0x15CF
#define PCI_DEVICE_ID_HILSCHER_NETX     0x0000
#define PCI_DEVICE_ID_HILSCHER_NETPLC   0x0010
#define PCI_DEVICE_ID_HILSCHER_NETJACK  0x0020
#define PCI_DEVICE_ID_HILSCHER_NETX4000 0x4000
#define PCI_SUBDEVICE_ID_NXSB_PCA       0x3235
#define PCI_SUBDEVICE_ID_NXPCA          0x3335
#define PCI_SUBDEVICE_ID_NETPLC_RAM     0x0000
#define PCI_SUBDEVICE_ID_NETPLC_FLASH   0x0001
#define PCI_SUBDEVICE_ID_NETJACK_RAM    0x0000
#define PCI_SUBDEVICE_ID_NETJACK_FLASH  0x0001

#define DPM_HOST_INT_EN0	0xfff0
#define DPM_HOST_INT_STAT0	0xffe0
#define PLX_GPIO_OFFSET         0x15
#define PLX_TIMING_OFFSET       0x0a

#define DPM_HOST_INT_MASK	0xe600ffff
#define DPM_HOST_INT_GLOBAL_EN	0x80000000
#define PLX_GPIO_DATA0_MASK     0x00000004
#define PLX_GPIO_DATA1_MASK     0x00000020

#define NX_PCA_PCI_8_BIT_DPM_MODE  0x5431F962
#define NX_PCA_PCI_16_BIT_DPM_MODE 0x4073F8E2
#define NX_PCA_PCI_32_BIT_DPM_MODE 0x40824122

#define NETX_DPM_SIZE_64K 0x10000

/* number of bar */
#define DPM_BAR     0 /* points to the DPM -> netX, netPLC, netJACK */
#define EXT_MEM_BAR 1 /* points to the optional extended memory     */
#define PLX_DPM_BAR 2 /* points to the DPM -> netXPLX               */
#define PXA_PLX_BAR 0 /* timing config register                     */

/* index of uio_info structure's memory array */
#define DPM_INDEX     0 /* first mapping describes DPM              */
#define EXT_MEM_INDEX 1 /* second mapping describes extended memory */

#define DPM_MEM_NAME "dpm"
#define EXT_MEM_NAME "extmem"
#define DMA_MEM_NAME "dma"

DEFINE_MUTEX(custom_list_lock);
uint8_t card_count = 0;

struct pxa_dev_info {
	uint32_t __iomem *plx;
	uint8_t dpm_mode;
	uint32_t plx_timing;
};

struct uio_netx_priv {
	int32_t dmacount;
	int32_t memcount;
	struct pxa_dev_info *pxa_info;
	int8_t no_irq_stat;
};


#define NETX_NAME_LEN_MAX 64
struct netx_custom_dev {
	struct list_head   list;
	struct miscdevice* misc;
	struct uio_info*   info;
	struct device*     dev;
#ifdef DMA_SUPPORT
	int                dma_enable;
#endif
	char device_name[NETX_NAME_LEN_MAX];
	phys_addr_t* dpm_addr;
	unsigned long* dpm_len;
	int* irq;
};

static LIST_HEAD(custom_list);

static int netx_enable_irq(struct uio_info *dev_info, s32 irq_on)
{
	if (dev_info->irq == 0)
		return -EPERM; /* Not supported! -> there is no interrupt registered */

	return 0;
}

static irqreturn_t netx_handler(int irq, struct uio_info *dev_info)
{
	if(((struct uio_netx_priv*)dev_info->priv)->pxa_info != NULL)
	{
		/* This is a PLX device and cannot produce an IRQ */
		return IRQ_NONE;
	} else
	{
		/* check if the device provides a global interrupt status reg */
		if (!((struct uio_netx_priv*)dev_info->priv)->no_irq_stat) {

			void __iomem *int_enable_reg = dev_info->mem[0].internal_addr
							+ DPM_HOST_INT_EN0;
			void __iomem *int_status_reg = dev_info->mem[0].internal_addr
					+ DPM_HOST_INT_STAT0;

			/* Is one of our interrupts enabled and active ? */
			if (!(ioread32(int_enable_reg) & ioread32(int_status_reg)
				& DPM_HOST_INT_MASK))
				return IRQ_NONE;

			/* Disable interrupt */
			iowrite32(ioread32(int_enable_reg) & ~DPM_HOST_INT_GLOBAL_EN,
					int_enable_reg);
		}
		return IRQ_HANDLED;
	}
}

static int netx_pxa_set_plx_timing(struct uio_info *info)
{
	struct uio_netx_priv *priv = (struct uio_netx_priv *) info->priv;
	uint32_t __iomem *plx_timing;
	if (!priv->pxa_info)
		return -ENODEV;
	plx_timing = priv->pxa_info->plx + PLX_TIMING_OFFSET;
	*plx_timing = priv->pxa_info->plx_timing;
	return 0;
}

static int netx_pxa_get_plx_timing(struct uio_info *info)
{
	struct uio_netx_priv *priv = (struct uio_netx_priv *) info->priv;
	if (!priv->pxa_info)
		return -ENODEV;
	switch (priv->pxa_info->dpm_mode) {
	case 8:
		priv->pxa_info->plx_timing = NX_PCA_PCI_8_BIT_DPM_MODE;
		break;
	case 16:
		priv->pxa_info->plx_timing = NX_PCA_PCI_16_BIT_DPM_MODE;
		break;
	case 32:
		priv->pxa_info->plx_timing = NX_PCA_PCI_32_BIT_DPM_MODE;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int netx_pxa_get_dpm_mode(struct uio_info *info)
{
	struct uio_netx_priv *priv = (struct uio_netx_priv *) info->priv;
	uint32_t __iomem *plx_gpio;
	if (!priv->pxa_info)
		return -ENODEV;
	plx_gpio = priv->pxa_info->plx + PLX_GPIO_OFFSET;
	if ((*plx_gpio & PLX_GPIO_DATA0_MASK) &&
	   ~(*plx_gpio & PLX_GPIO_DATA1_MASK))
		priv->pxa_info->dpm_mode = 8;
	else if (~(*plx_gpio & PLX_GPIO_DATA0_MASK) &&
		 (*plx_gpio & PLX_GPIO_DATA1_MASK))
		priv->pxa_info->dpm_mode = 32;
	else if (~(*plx_gpio & PLX_GPIO_DATA0_MASK) &&
		~(*plx_gpio & PLX_GPIO_DATA1_MASK))
		priv->pxa_info->dpm_mode = 16;
	else
		return -EINVAL;
	return 0;
}

#ifdef DMA_SUPPORT
int create_dma_buffer(struct device *dev, struct uio_info *info, struct uio_mem *dma_mem)
{
	void __iomem *addr;
	dma_addr_t busaddr;

	/* Allocate DMA-capable buffer */
	addr = dma_alloc_coherent(dev, dma_buffer_size,
			&busaddr,
			GFP_KERNEL);
	if (!addr) {
		dev_warn(dev, "Unable to allocate requested DMA-capable"
			" block of size 0x%lx during mmap in uio.\n",
			dma_buffer_size);
		return -ENOMEM;
	}

	/* Store the physical address and index as the
	 * first two long words for userspace access */
	memset(addr ,0 ,dma_buffer_size);
	dma_mem->addr = busaddr;
	dma_mem->internal_addr = addr;
	dma_mem->size = dma_buffer_size;
	dma_mem->name = DMA_MEM_NAME;
	dma_mem->memtype = UIO_MEM_PHYS;
	return 0;
}

int release_dma_mem(struct device *dev, struct uio_info *info)
{
	struct uio_netx_priv *priv = info->priv;

	while(priv->dmacount-->0) {
		priv->memcount--;
		dma_free_coherent(dev,
				info->mem[priv->memcount].size,
				(void*)(info->mem[priv->memcount].internal_addr),
				(dma_addr_t) info->mem[priv->memcount].addr);
		info->mem[priv->memcount].addr          = 0;
		info->mem[priv->memcount].size          = 0;
		info->mem[priv->memcount].internal_addr = 0;
	}
	return 0;
}

static int add_dma(struct device *dev, struct uio_info *info)
{
	struct uio_netx_priv *priv = info->priv;
	int i = 0;
	int ret = 0;

	if (MAX_UIO_MAPS<(priv->memcount+dma_buffer_count)) {
		dev_info(dev, "Base uio driver does not serve enough memory\n"
			"regions for dma allocation (see MAX_UIO_MAPS)!\n");
		return -ENOMEM;
	}
	for (;i<dma_buffer_count;i++) {
		if ((ret = create_dma_buffer(dev, info, &info->mem[i+priv->memcount])))
			goto err_dma;
		dev_info(dev, "DMA buffer allocated (addr/size:0x%llX/0x%lX)\n",
			info->mem[i+priv->memcount].addr,
			(long unsigned int)info->mem[i+priv->memcount].size);
		priv->dmacount++;
	}
	priv->memcount+=dma_buffer_count;
	return 0;
err_dma:
	release_dma_mem(dev, info);
	return ret;
}
#endif

#ifndef __devinit
        #define __devinit
#endif

static int __devinit netx_pci_probe(struct pci_dev *dev,
					const struct pci_device_id *id)
{
	struct uio_info *info;
	int bar;
	info = kzalloc(sizeof(struct uio_info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->version = UIO_NETX_VERSION;
	if (!(info->priv = (struct uio_netx_priv *) kzalloc(sizeof(struct uio_netx_priv), GFP_KERNEL)))
		goto out_priv;
	if (pci_enable_device(dev))
		goto out_free;
	if (pci_request_regions(dev, "netx"))
		goto out_disable;
	switch (id->device) {
	case PCI_DEVICE_ID_HILSCHER_NETX:
		bar = DPM_BAR;
		info->name = "netx";
		break;
	case PCI_DEVICE_ID_HILSCHER_NETPLC:
		bar = DPM_BAR;
		info->name = "netplc";
		break;
	case PCI_DEVICE_ID_HILSCHER_NETJACK:
		bar = DPM_BAR;
		info->name = "netjack";
		break;
	case PCI_DEVICE_ID_HILSCHER_NETX4000:
		bar = DPM_BAR;
		info->name = "netx";
		break;
	default:
		bar = PLX_DPM_BAR;
		info->name = "netx_plx";
	}
	/* BAR 0 or 2 points to the card's dual port memory */
	info->mem[DPM_INDEX].addr = pci_resource_start(dev, bar);
	if (!info->mem[DPM_INDEX].addr) {
		dev_err( &dev->dev, "Error retrieving the memory address of the device!\n");
		goto out_release;
	}

	info->mem[DPM_INDEX].internal_addr = ioremap_nocache(
		pci_resource_start(dev, bar),
		pci_resource_len(dev, bar));

	if (!info->mem[DPM_INDEX].internal_addr) {
		dev_err( &dev->dev, "Error mapping the DPM of the device!\n");
		goto out_release;
	}

	dev_info(&dev->dev, "DPM at 0x%llX\n", info->mem[DPM_INDEX].addr);
	info->mem[DPM_INDEX].size = pci_resource_len(dev, bar);
	info->mem[DPM_INDEX].memtype = UIO_MEM_PHYS;
	info->mem[DPM_INDEX].name = DPM_MEM_NAME;
	((struct uio_netx_priv*)(info->priv))->memcount = 1;

	/* map extended mem (BAR 1 points to the extended memory) */
	info->mem[EXT_MEM_INDEX].addr = pci_resource_start(dev, EXT_MEM_BAR);

	/* extended memory is optional, so don't care if it is not present */
	if (info->mem[EXT_MEM_INDEX].addr) {
		info->mem[EXT_MEM_INDEX].internal_addr = ioremap_nocache(
		pci_resource_start(dev, EXT_MEM_BAR),
		pci_resource_len(dev, EXT_MEM_BAR));

		if (!info->mem[EXT_MEM_INDEX].internal_addr)
			goto out_unmap;

		dev_info(&dev->dev, "extended memory at 0x%llX\n", info->mem[EXT_MEM_INDEX].addr);
		info->mem[EXT_MEM_INDEX].size    = pci_resource_len(dev, EXT_MEM_BAR);
		info->mem[EXT_MEM_INDEX].memtype = UIO_MEM_PHYS;
		info->mem[EXT_MEM_INDEX].name = EXT_MEM_NAME;
		((struct uio_netx_priv*)(info->priv))->memcount++;
	}

	info->irq = dev->irq;
# if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20) && LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0))
	info->irq_flags = IRQF_DISABLED | IRQF_SHARED;
# else
	info->irq_flags = IRQF_SHARED;
# endif
	info->handler = netx_handler;
	info->irqcontrol = netx_enable_irq;

	if ((id->device == PCI_DEVICE_ID_HILSCHER_NETX) ||
		(id->device == PCI_DEVICE_ID_HILSCHER_NETPLC) ||
		(id->device == PCI_DEVICE_ID_HILSCHER_NETJACK) ||
		(id->device == PCI_DEVICE_ID_HILSCHER_NETX4000)) {
		/* make sure all interrupts are disabled */
		iowrite32(0, info->mem[DPM_INDEX].internal_addr + DPM_HOST_INT_EN0);
		((struct uio_netx_priv*)(info->priv))->pxa_info = NULL;
	} else if (id->subdevice == PCI_SUBDEVICE_ID_NXPCA) {
		/* map PLX registers */
		struct pxa_dev_info *pxa_info = (struct pxa_dev_info *)
			kzalloc(sizeof(struct pxa_dev_info), GFP_KERNEL);
		if (!pxa_info)
			goto out_unmap;
		((struct uio_netx_priv*)(info->priv))->pxa_info = pxa_info;
		/* set PXA PLX Timings */
		pxa_info->plx = ioremap_nocache(
			pci_resource_start(dev, PXA_PLX_BAR),
			pci_resource_len(dev, PXA_PLX_BAR));
		if (!pxa_info->plx)
			goto out_unmap;
		if (netx_pxa_get_dpm_mode(info))
			goto out_unmap_plx;
		if (netx_pxa_get_plx_timing(info))
			goto out_unmap_plx;
		if (netx_pxa_set_plx_timing(info))
			goto out_unmap_plx;
	} else {
		struct pxa_dev_info *pxa_info = (struct pxa_dev_info *)
			kzalloc(sizeof(struct pxa_dev_info), GFP_KERNEL);
		if (!pxa_info)
			goto out_free_pxa;
		pxa_info->plx = NULL;
		pxa_info->plx_timing = 0;
		pxa_info->dpm_mode = 0;
		((struct uio_netx_priv*)info->priv)->pxa_info = pxa_info;
	}
#ifdef DMA_SUPPORT
	if ((!dma_disable) && (add_dma(&dev->dev, info)))
		dev_warn( &dev->dev, "Error reserving memory for dma!\n");
#endif
	if (uio_register_device(&dev->dev, info)) {
		if (id->subdevice != PCI_SUBDEVICE_ID_NXPCA)
			goto out_unmap;
		else
			goto out_unmap_plx;
	}
	pci_set_drvdata(dev, info);
	if ((id->device == PCI_DEVICE_ID_HILSCHER_NETX) || (id->device == PCI_DEVICE_ID_HILSCHER_NETX4000))
		dev_info(&dev->dev,
			"registered CifX card\n");
	else if (id->device == PCI_DEVICE_ID_HILSCHER_NETPLC)
		dev_info(&dev->dev,
			"registered netPLC card\n");
	else if (id->device == PCI_DEVICE_ID_HILSCHER_NETJACK)
		dev_info(&dev->dev, "registered netJACK card\n");
	else if (id->subdevice == PCI_SUBDEVICE_ID_NXSB_PCA)
		dev_info(&dev->dev,
			"registered NXSB-PCA adapter card\n");
	else {
		struct pxa_dev_info *pxa_info = (struct pxa_dev_info *)
			((struct uio_netx_priv*)info->priv)->pxa_info;
		dev_info(&dev->dev,
			"registered NXPCA-PCI adapter card in %d bit mode\n",
			pxa_info->dpm_mode);
	}
	return 0;
out_unmap_plx:
	iounmap(((struct uio_netx_priv*)(info->priv))->pxa_info->plx);
out_free_pxa:
	kfree(((struct uio_netx_priv*)info->priv)->pxa_info);
out_unmap:
#ifdef DMA_SUPPORT
	release_dma_mem(&dev->dev, info);
#endif
	iounmap(info->mem[DPM_INDEX].internal_addr);
	if (info->mem[EXT_MEM_INDEX].internal_addr)
		iounmap(info->mem[EXT_MEM_INDEX].internal_addr);
out_release:
	pci_release_regions(dev);
out_disable:
	pci_disable_device(dev);
out_priv:
	kfree(info->priv);
out_free:
	kfree(info);
	return -ENODEV;
}

static void netx_pci_remove(struct pci_dev *dev)
{
	struct uio_info *info = pci_get_drvdata(dev);
	struct pxa_dev_info *pxa_info = ((struct uio_netx_priv*)info->priv)->pxa_info;
	if (pxa_info) {
		if ( pxa_info->plx)
			iounmap(pxa_info->plx);

		kfree(pxa_info);
	} else {
		/* Disable all interrupts (interrupt is only enabled for none-PCA devices) */
		iowrite32(0, info->mem[DPM_INDEX].internal_addr + DPM_HOST_INT_EN0);
	}
	uio_unregister_device(info);
#ifdef DMA_SUPPORT
	release_dma_mem(&dev->dev, info);
#endif
	pci_release_regions(dev);
	pci_disable_device(dev);
	pci_set_drvdata(dev, NULL);
	iounmap(info->mem[DPM_INDEX].internal_addr);
	if (info->mem[EXT_MEM_INDEX].internal_addr)
		iounmap(info->mem[EXT_MEM_INDEX].internal_addr);
	kfree(info->priv);
	kfree(info);
}

static struct pci_device_id netx_pci_ids[] = {
	{
		.vendor =	PCI_VENDOR_ID_HILSCHER,
		.device =	PCI_DEVICE_ID_HILSCHER_NETX,
		.subvendor =	0,
		.subdevice =	0,
	},
	{
		.vendor =	PCI_VENDOR_ID_PLX,
		.device =	PCI_DEVICE_ID_PLX_9030,
		.subvendor =	PCI_VENDOR_ID_PLX,
		.subdevice =	PCI_SUBDEVICE_ID_NXSB_PCA,
	},
	{
		.vendor =	PCI_VENDOR_ID_PLX,
		.device =	PCI_DEVICE_ID_PLX_9030,
		.subvendor =	PCI_VENDOR_ID_PLX,
		.subdevice =	PCI_SUBDEVICE_ID_NXPCA,
	},
	{
		.vendor =	PCI_VENDOR_ID_HILSCHER,
		.device =	PCI_DEVICE_ID_HILSCHER_NETPLC,
		.subvendor =	PCI_VENDOR_ID_HILSCHER,
		.subdevice =	PCI_SUBDEVICE_ID_NETPLC_RAM,
	},
	{
		.vendor =	PCI_VENDOR_ID_HILSCHER,
		.device =	PCI_DEVICE_ID_HILSCHER_NETPLC,
		.subvendor =	PCI_VENDOR_ID_HILSCHER,
		.subdevice =	PCI_SUBDEVICE_ID_NETPLC_FLASH,
	},
	{
		.vendor = PCI_VENDOR_ID_HILSCHER,
		.device = PCI_DEVICE_ID_HILSCHER_NETJACK,
		.subvendor =  PCI_VENDOR_ID_HILSCHER,
		.subdevice =  PCI_SUBDEVICE_ID_NETJACK_RAM,
	},
	{
		.vendor = PCI_VENDOR_ID_HILSCHER,
		.device = PCI_DEVICE_ID_HILSCHER_NETJACK,
		.subvendor =  PCI_VENDOR_ID_HILSCHER,
		.subdevice =  PCI_SUBDEVICE_ID_NETJACK_FLASH,
	},
	{
		.vendor = PCI_VENDOR_ID_HILSCHER,
		.device = PCI_DEVICE_ID_HILSCHER_NETX4000,
		.subvendor =  0,
		.subdevice =  0,
	},
	{ 0, }
};

/* publish PCI ids, to provide automatic load for known PCI cards */
MODULE_DEVICE_TABLE(pci, netx_pci_ids);

static struct pci_driver netx_pci_driver = {
	.name = "netx",
	.id_table = netx_pci_ids,
	.probe = netx_pci_probe,
	.remove = netx_pci_remove,
};

static int misc_counter = 0;
static int create_misc_device(struct netx_custom_dev* custom)
{
	int ret = 0;

	if (custom->dev == NULL) {
		char* name = kzalloc(16, GFP_KERNEL);
		if (name == NULL) {
			printk("uio_netx - custom card(%d): "\
			"Error allocating memory for custom netx device!\n", card_count);
			return -ENOMEM;
		}
		if ((custom->misc = kzalloc(sizeof(struct miscdevice), GFP_KERNEL)) != NULL) {
			sprintf(name, "netx_custom%d", misc_counter++);
			custom->misc->minor = MISC_DYNAMIC_MINOR;
			custom->misc->name  = name;
			if ((ret = misc_register( custom->misc))) {
				kfree( custom->misc->name);
				kfree( custom->misc);
				printk("uio_netx - custom card(%d): "\
				"Error creating misc-device (ret=%d)!\n", card_count, ret);
				return -ENOMEM;
			}
			custom->dev = custom->misc->this_device;
#ifdef DMA_SUPPORT
			/* this device is passed via command line so use command line DMA settings */
			custom->dma_enable = !(dma_disable);
#endif
		} else {
			printk("uio_netx - custom card(%d): "\
				"Error allocating memory for custom netx device!\n", card_count);
			kfree(name);
			return -ENOMEM;
		}
	}
	return 0;
}

void delete_misc_device(struct miscdevice* misc)
{
	if (misc) {
		misc_deregister( misc);
		kfree( misc->name);
		kfree( misc);
	}
}

void free_netx_custom_dev(struct netx_custom_dev* custom)
{
	if (custom) {
		if (custom->misc) {
			delete_misc_device(custom->misc);
		}
		if (custom->dpm_addr)
			kfree(custom->dpm_addr);
		if (custom->dpm_len)
			kfree(custom->dpm_len);
		if (custom->irq)
			kfree(custom->irq);
		kfree(custom);
	}
}

struct netx_custom_dev* alloc_netx_custom_dev(struct device* dev){
	struct netx_custom_dev* custom = kzalloc(sizeof(struct netx_custom_dev), GFP_KERNEL);

	if (custom == NULL) {
		printk("uio_netx - custom card: "\
			"Error allocating memory for custom netx device!\n");
		return NULL;
	} else {
		sprintf( custom->device_name, "netx_custom");
		if (dev == NULL) {
			if (create_misc_device(custom))
				kfree(custom);
		} else {
			custom->dev = dev;
		}
	}
	return custom;
}

static int map_custom_card( struct netx_custom_dev* custom, int no_of_maps)
{
	int ret,i = 0;
	struct uio_info* info = NULL;

	if (no_of_maps == 0)
		return -EINVAL;

	info = kzalloc(sizeof(struct uio_info), GFP_KERNEL);
	if ((!info) || (!(info->priv = kzalloc(sizeof(struct uio_netx_priv), GFP_KERNEL)))) {
		dev_info(custom->dev, "uio_netx - custom card(%d): "\
			"Error allocating memory for custom netx device!\n", card_count);

		if (info) kfree( info);
		return -ENOMEM;
	}
	info->version = UIO_NETX_VERSION;
	info->name = custom->device_name;
	ret = 0;
	for (i=0;(i<no_of_maps) && (ret == 0);i++) {
		info->mem[DPM_INDEX+i].addr = custom->dpm_addr[i];
		info->mem[DPM_INDEX+i].internal_addr = ioremap_nocache(custom->dpm_addr[i], custom->dpm_len[i]);
		info->mem[DPM_INDEX+i].size = custom->dpm_len[i];
		info->mem[DPM_INDEX+i].memtype = UIO_MEM_PHYS;
		info->mem[DPM_INDEX+i].name = (i == 0) ? DPM_MEM_NAME : EXT_MEM_NAME;
		((struct uio_netx_priv*)(info->priv))->memcount++;

		if (info->mem[DPM_INDEX+i].internal_addr == NULL)
			ret = -1;
	}
	/* check if everthing is mapped */
	if (ret != 0) {
		while(--i>0) {
			iounmap(custom->info->mem[i].internal_addr);
		}
		kfree( info->priv);
		kfree( info);
		return -ENOMEM;
	}
	/* check if device provides global interrupt status reg */
	if (custom->dpm_len[0]<NETX_DPM_SIZE_64K)
		((struct uio_netx_priv*)(info->priv))->no_irq_stat = 1;

	info->irq = custom->irq[0];
# if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20) && LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0))
	info->irq_flags = IRQF_DISABLED;
# else
	info->irq_flags = 0;
# endif
	info->handler = netx_handler;
	info->irqcontrol = netx_enable_irq;
#ifdef DMA_SUPPORT
	if ((custom->dma_enable) && (add_dma( custom->dev, info)))
		dev_info(custom->dev, "uio_netx - custom card(%d): "\
			"Error reserving memory for DMA!\n", card_count);
#endif
	if ((ret = uio_register_device( custom->dev, info))) {
		dev_info(custom->dev, "uio_netx - custom card(%d): "\
		       "Error registering custom netx device (ret=%d)!\n", card_count, ret);
		kfree( info->priv);
		kfree( info);
		return -ENOMEM;
	} else {
		custom->info = info;
		mutex_lock( &custom_list_lock);
		list_add(&custom->list, &custom_list);
		card_count++;
		mutex_unlock( &custom_list_lock);
		dev_info(custom->dev, "uio_netx - custom card(%d): "\
		       "Successfuly registered custom netx device (DPM at 0x%llX)!\n", card_count-1, custom->dpm_addr[0]);
	}
	return 0;
}

void unmap_custom_cards(struct device* dev)
{
	struct list_head *pos;
	struct list_head *next;

	mutex_lock( &custom_list_lock);
	list_for_each_safe( pos, next, &custom_list) {
		struct netx_custom_dev* custom = list_entry( pos, struct netx_custom_dev, list);
		/* in case dev is given delete only this element */
		if (dev != NULL) {
			if (custom->dev != dev) {
				custom = NULL;
			}
		}
		if (custom != NULL) {
			struct uio_netx_priv* priv = (struct uio_netx_priv*)custom->info->priv;
			uio_unregister_device(custom->info);

			while(priv->memcount-->0) {
				if (NULL != custom->info->mem[priv->memcount].internal_addr)
					iounmap(custom->info->mem[priv->memcount].internal_addr);
			}
#ifdef DMA_SUPPORT
			release_dma_mem(custom->dev, custom->info);
#endif
			kfree( custom->info->priv);
			kfree( custom->info);

			list_del( pos);
			card_count--;
			free_netx_custom_dev( custom);
			/* in case dev is given we are done here */
			if (dev != NULL) {
				break;
			}
		}
	}
	mutex_unlock( &custom_list_lock);
}

#ifdef CONFIG_OF
int get_dt_parameter(struct platform_device *pd, struct netx_custom_dev* custom)
{
	const void* ptr = NULL;
	struct resource res;
	int mappings = 0;
	int ret = 0;

	while ((ret = of_address_to_resource(custom->dev->of_node, mappings, &res)) == 0) {
		mappings++;
	}
	dev_info( custom->dev, "uio_netx - get_dt_parameter: found %d mappings!\n",mappings);
	if (mappings == 0)
		return -ENODATA;

	custom->dpm_addr = kzalloc((mappings)*sizeof(phys_addr_t), GFP_KERNEL);
	custom->dpm_len  = kzalloc((mappings)*sizeof(unsigned long), GFP_KERNEL);
	custom->irq  = kzalloc((mappings)*sizeof(int), GFP_KERNEL);

	if (!(custom->dpm_addr) || !(custom->dpm_len) || !(custom->irq)) {
		dev_err( custom->dev, "uio_netx - get_dt_parameter: failed to allocate memory for device mapping-parameter!\n");
		return -ENOMEM;
	}
	while ((mappings>0) && (ret = of_address_to_resource(custom->dev->of_node, --mappings, &res)) == 0) {
		custom->dpm_addr[mappings] = res.start;
		custom->dpm_len[mappings] = resource_size(&res);
		custom->irq[mappings] = 0;
	}
	/* set irq information */
	custom->irq[0] = platform_get_irq_byname(pd, "card");

	/* build device name (/sys/class/uio[x]/name) */
	if (NULL != (ptr = of_get_property(custom->dev->of_node, "startuptype", NULL))) {
		snprintf( custom->device_name, NETX_NAME_LEN_MAX, "netx_custom,%s,", (char*)ptr);
	} else {
		sprintf( custom->device_name, "netx_custom,-,");
	}
	if (NULL != (ptr = of_get_property(custom->dev->of_node, "alias", NULL))) {
		snprintf( custom->device_name+(strlen(custom->device_name)),NETX_NAME_LEN_MAX-strlen(custom->device_name), "%s", (char*)ptr);
	} else {
		snprintf( custom->device_name+(strlen(custom->device_name)),NETX_NAME_LEN_MAX-strlen(custom->device_name), "-");
	}
#if defined DMA_SUPPORT
	ptr = of_get_property(custom->dev->of_node, "dma", NULL);
	custom->dma_enable = !!!(*(uint32_t*)ptr) ? 1 : 0;
#endif
	return mappings;
}

static int __devinit netx_dt_probe(struct platform_device *pd)
{
	int ret = -ENOMEM;
	struct netx_custom_dev* custom = alloc_netx_custom_dev(&pd->dev);

	if (custom != NULL) {
		/* try to get required information */
		if ((ret = get_dt_parameter(pd, custom))>0) {
			dev_info( custom->dev, "uio_netx - netx_dt_probe: device DPM @0x%llX ...+0x%lX (@irq=%d)\n", custom->dpm_addr[0], (unsigned int)custom->dpm_len[0], custom->irq[0]);
			ret = map_custom_card( custom, ret);
		} else {
			dev_err( custom->dev, "uio_netx - netx_dt_probe: failed to request resources (%d)\n", ret);
		}
		if (ret<0)
			free_netx_custom_dev(custom);
	}
	return ret;
}

static int netx_dt_remove(struct platform_device *pd)
{
	unmap_custom_cards( &pd->dev);
	return 0;
}

/****************************************************
 *** netX0: netX@f8034000 {
 ***			status = "ok";
 ***			compatible = "hilscher,uio-netx";
 ***			reg = <0xf8034000 0x10000>; // can be multiple (1. DPM 2... extended memory)
 ***			interrupt-names = "card"; // only "card" supported
 ***			interrupts = <168 IRQ_TYPE_LEVEL_HIGH>;
 ***			dma = <1>; // enable / disable
 ***			startuptype = "auto"; // specifies startup behaviour: flash,ram,auto,donttouch
 ***			alias = "custom-device"; // device name
 *** }
 ***************************************************/

static struct of_device_id const uio_netx_of_match[] __refconst = {
	{ .compatible = "hilscher,uio-netx", },
	{}
};

MODULE_DEVICE_TABLE(of, uio_netx_of_match);

static struct platform_driver uio_netx_platform_driver = {
	.probe = netx_dt_probe,
	.remove = netx_dt_remove,
	.driver = {
		.name = "uio-netx",
		.owner = THIS_MODULE,
		.of_match_table = uio_netx_of_match,
	},
};
#endif //CONFIG_OF

static int __init netx_init_module(void)
{
	INIT_LIST_HEAD(&custom_list);
	mutex_init( &custom_list_lock);

	/* parameter are given via command line */
	if (addr_cnt) {/* custom card definition given via command line (e.g. ISA) */
		if ((addr_cnt == len_cnt) && (addr_cnt == irq_cnt)) {
			int max = addr_cnt;
			while(addr_cnt>0) {
				struct netx_custom_dev* custom = alloc_netx_custom_dev(NULL);
				if (custom != NULL) {
					if ((custom->dpm_addr = kzalloc(sizeof(phys_addr_t),GFP_KERNEL)))
						*custom->dpm_addr = custom_dpm_addr[max - addr_cnt];
					if ((custom->dpm_len = kzalloc(sizeof(unsigned long),GFP_KERNEL)))
						*custom->dpm_len = custom_dpm_len[max - addr_cnt];
					if ((custom->irq = kzalloc(sizeof(int),GFP_KERNEL)))
						*custom->irq = custom_irq[max - addr_cnt];

					if (map_custom_card( custom, 1)) {
						/* failure during mapping custom device */
						free_netx_custom_dev(custom);
					}
				}
				addr_cnt--;
			}
		} else {
			printk("uio_netx - Error registering passed user card. Invalid number of arguments!");
		}
	}
#ifdef CONFIG_OF
	/* register driver for device tree */
	platform_driver_register(&uio_netx_platform_driver);
#endif
	/* and pci */
	return pci_register_driver(&netx_pci_driver);
}

static void __exit netx_exit_module(void)
{
	pci_unregister_driver(&netx_pci_driver);
#ifdef CONFIG_OF
	platform_driver_unregister(&uio_netx_platform_driver);
#endif
	unmap_custom_cards(NULL);
}

module_init(netx_init_module);
module_exit(netx_exit_module);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Hans J. Koch, Manuel Traut, Sebastian Doell");
MODULE_DESCRIPTION("Device driver for netX hardware\n\t\tHilscher Gesellschaft fuer Systemautomation mbH");
