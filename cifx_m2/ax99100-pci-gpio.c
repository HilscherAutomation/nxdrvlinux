// SPDX-License-Identifier: GPL-2.0
/*
 * AX99100 PCIe to Multi I/O Controller - GPIO driver
 *
 * drivers/gpio/ax99100-pci-gpio.c
 *
 * (C) Copyright 2019 Hilscher Gesellschaft fuer Systemautomation mbH
 * http://www.hilscher.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#define DRIVER_DESC "AX99100 PCIe to Multi I/O Controller - GPIO driver"
#define DRIVER_NAME "ax99100-pci-gpio"

#define HILSCHER_PCI_VENDOR_ID  0x15cf
#define HILSCHER_PCI_DEVICE_ID  0x0090

#define HILSCHER_PCI_SUB_VENDOR_ID       0x15cf
#define HILSCHER_PCI_SUB_DEVICE_ID_GPIO  0x1002
#define HILSCHER_PCI_SUB_DEVICE_ID_SPI   0x6001

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <linux/version.h>

/* -------------------------------------------------------------------------- */
/* ------ Regdefs ----------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

#define regdef(mask, shift, name) \
static inline uint8_t g##name(uint8_t val) \
{ \
	return (val >> shift) & mask; \
} \
static inline uint8_t s##name(uint8_t val) \
{ \
	return (val & mask) << shift; \
}

/* ------ BAR1 Register (MEM mapped) ---------------------------------------- */

#pragma pack(1)
struct regdef_common {
	uint32_t swrst;
};
#pragma pack()

#pragma pack(1)
struct serial_port_interrupt_register {
	uint32_t gisr;
	uint32_t gicr;
	uint32_t gier;
};
#pragma pack()

regdef(0x1, 4, GIxR) /* GISR, GICR, GIER */

/* ------ BAR5 Register (MEM mapped) ---------------------------------------- */

#pragma pack(1)
struct gpio_register {
	uint32_t pin;
	uint32_t dir;
	uint32_t em;
	uint32_t od;
	uint32_t pu;
	uint32_t eds;
	uint32_t ede;
	uint32_t ctr;
};
#pragma pack()

/* -------------------------------------------------------------------------- */
/* ------ Macros ------------------------------------------------------------ */
/* -------------------------------------------------------------------------- */

#define iomod32(cmask, smask, addr) \
do { \
	uint32_t val = ioread32(addr); \
	val &= ~(cmask); \
	val |= (smask); \
	iowrite32(val, addr); \
} while (0)

/* -------------------------------------------------------------------------- */
/* ------ Global variables -------------------------------------------------- */
/* -------------------------------------------------------------------------- */

struct priv_data {
	struct pci_dev *pci;

	struct gpio_chip gpio_chip;
	spinlock_t lock;
	struct {
		struct regdef_common __iomem *common;
		struct serial_port_interrupt_register __iomem *global_irq;
		struct gpio_register __iomem *gpio;
	} reg;
};

/* -------------------------------------------------------------------------- */
/* ------ Help functions ---------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void ax99100_pci_gpio_reg_dump(struct priv_data *pd)
{
	dev_info(&pd->pci->dev, "Register dump ...\n");
	dev_info(&pd->pci->dev, "=================\n");
	dev_info(&pd->pci->dev, "PIN 0x%08x\n", ioread32(&pd->reg.gpio->pin));
	dev_info(&pd->pci->dev, "DIR 0x%08x\n", ioread32(&pd->reg.gpio->dir));
	dev_info(&pd->pci->dev, "EM  0x%08x\n", ioread32(&pd->reg.gpio->em));
	dev_info(&pd->pci->dev, "OD  0x%08x\n", ioread32(&pd->reg.gpio->od));
	dev_info(&pd->pci->dev, "PU  0x%08x\n", ioread32(&pd->reg.gpio->pu));
	dev_info(&pd->pci->dev, "EDS 0x%08x\n", ioread32(&pd->reg.gpio->eds));
	dev_info(&pd->pci->dev, "EDE 0x%08x\n", ioread32(&pd->reg.gpio->ede));
	dev_info(&pd->pci->dev, "CTR 0x%08x\n", ioread32(&pd->reg.gpio->ctr));
}

/* -------------------------------------------------------------------------- */
/* ------ IRQ Chip API functions -------------------------------------------- */
/* -------------------------------------------------------------------------- */

/**
 * ax99100_pci_gpio_irq_mask:
 *
 * @id:
 */
static void ax99100_pci_gpio_irq_mask(struct irq_data *id)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(id);
	struct priv_data *pd = gpiochip_get_data(gc);
	uint32_t offset = id->hwirq;
	unsigned long flags;

	spin_lock_irqsave(&pd->lock, flags);

	/* Disable GPIO IRQ */
	iomod32(BIT(offset), 0, &pd->reg.gpio->ede);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
	gpiochip_disable_irq( gc, id->hwirq);
#endif
	spin_unlock_irqrestore(&pd->lock, flags);
}

/**
 * ax99100_pci_gpio_irq_unmask:
 *
 * @id:
 */
static void ax99100_pci_gpio_irq_unmask(struct irq_data *id)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(id);
	struct priv_data *pd = gpiochip_get_data(gc);
	uint32_t offset = id->hwirq;
	unsigned long flags;

	spin_lock_irqsave(&pd->lock, flags);

	/* Enable GPIO IRQ */
	iomod32(0, BIT(offset), &pd->reg.gpio->ede);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
	gpiochip_enable_irq( gc, id->hwirq);
#endif

	spin_unlock_irqrestore(&pd->lock, flags);
}

/**
 * ax99100_pci_gpio_irq_set_type:
 *
 * @id:
 * @type:
 *
 * Return:  0 on success; error value otherwise
 */
static int ax99100_pci_gpio_irq_set_type(struct irq_data *id, uint32_t type)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(id);
	struct priv_data *pd = gpiochip_get_data(gc);
	uint32_t offset = id->hwirq;
	unsigned long flags;
	int rc = 0;

	spin_lock_irqsave(&pd->lock, flags);

	switch (type & IRQ_TYPE_SENSE_MASK) {
	case IRQ_TYPE_EDGE_RISING:
		iomod32(BIT(offset), 0, &pd->reg.gpio->ctr);
		iomod32(0, BIT(offset), &pd->reg.gpio->em);
		break;
	case IRQ_TYPE_EDGE_FALLING:
		iomod32(BIT(offset), 0, &pd->reg.gpio->ctr);
		iomod32(BIT(offset), 0, &pd->reg.gpio->em);
		break;
	case IRQ_TYPE_EDGE_BOTH:
		iomod32(0, BIT(offset), &pd->reg.gpio->ctr);
		break;
	case IRQ_TYPE_NONE:
		break;
	default:
		rc = -EINVAL;
	}

	spin_unlock_irqrestore(&pd->lock, flags);

	return rc;
}

/* -------------------------------------------------------------------------- */
/* ------ GPIO Chip API functions ------------------------------------------- */
/* -------------------------------------------------------------------------- */

/**
 * ax99100_pci_gpio_direction_input:
 *
 * @gc:
 * @offset:
 *
 * Return:  0 on success; error value otherwise
 */
static int ax99100_pci_gpio_direction_input(struct gpio_chip *gc, uint32_t offset)
{
	struct priv_data *pd = gpiochip_get_data(gc);
	unsigned long flags;

	if ((offset >= 2) && (offset <= 5)) {
		dev_err(&pd->pci->dev, "Error: gpio%d is output only!\n", offset);
		return -EINVAL;
	}

	spin_lock_irqsave(&pd->lock, flags);

	iomod32(0, BIT(offset), &pd->reg.gpio->dir);

	spin_unlock_irqrestore(&pd->lock, flags);

	return 0;
}

/**
 * ax99100_pci_gpio_direction_output:
 *
 * @gc:
 * @offset:
 * @value:
 *
 * Return:  0 on success; error value otherwise
 */
static int ax99100_pci_gpio_direction_output(struct gpio_chip *gc, uint32_t offset, int value)
{
	struct priv_data *pd = gpiochip_get_data(gc);
	unsigned long flags;

	spin_lock_irqsave(&pd->lock, flags);

	iomod32(BIT(offset), 0, &pd->reg.gpio->dir);
	if (value)
		iomod32(0, BIT(offset), &pd->reg.gpio->pin);
	else
		iomod32(BIT(offset), 0, &pd->reg.gpio->pin);

	spin_unlock_irqrestore(&pd->lock, flags);

	return 0;
}

/**
 * ax99100_pci_gpio_get:
 *
 * @gc:
 * @offset:
 *
 * Return:  0 on success; error value otherwise
 */
static int ax99100_pci_gpio_get(struct gpio_chip *gc, uint32_t offset)
{
	struct priv_data *pd = gpiochip_get_data(gc);

	if ((offset >= 2) && (offset <= 5))
		dev_warn(&pd->pci->dev, "gpio%d is output only and can't be read back!\n", offset);

	return !!(ioread32(&pd->reg.gpio->pin) & BIT(offset));
}

/**
 * ax99100_pci_gpio_set:
 *
 * @gc:
 * @offset:
 * @value:
 */
static void ax99100_pci_gpio_set(struct gpio_chip *gc, uint32_t offset, int value)
{
	struct priv_data *pd = gpiochip_get_data(gc);
	unsigned long flags;

	spin_lock_irqsave(&pd->lock, flags);

	if (value)
		iomod32(0, BIT(offset), &pd->reg.gpio->pin);
	else
		iomod32(BIT(offset), 0, &pd->reg.gpio->pin);

	spin_unlock_irqrestore(&pd->lock, flags);
}

/* -------------------------------------------------------------------------- */
/* ------ IRQ functions ----------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/**
 * ax99100_pci_gpio_isr: IRQ chain handler of GPIO chip.
 *
 * @desc:  IRQ number
 */
static void ax99100_pci_gpio_isr(struct irq_desc *desc)
{
	struct priv_data *pd = gpiochip_get_data(irq_desc_get_handler_data(desc));
	struct irq_chip *ic = irq_desc_get_chip(desc);
#if KERNEL_VERSION(4, 15, 0) >  LINUX_VERSION_CODE
	struct irq_domain *irqdomain = pd->gpio_chip.irqdomain;
#else
	struct irq_domain *irqdomain = pd->gpio_chip.irq.domain;
#endif
	unsigned long status, i;

	chained_irq_enter(ic, desc);

	/* Confirm global IRQ */
	iowrite32(sGIxR(1), &pd->reg.global_irq->gicr);

	/* Confirm GPIO IRQ */
	status = ioread32(&pd->reg.gpio->eds);
	iowrite32(status, &pd->reg.gpio->eds);

	for_each_set_bit(i, (const unsigned long *)&status, pd->gpio_chip.ngpio)
		generic_handle_irq(irq_find_mapping(irqdomain, i));

	chained_irq_exit(ic, desc);
}

/* -------------------------------------------------------------------------- */
/* ------ Driver / Module functions ----------------------------------------- */
/* -------------------------------------------------------------------------- */

/**
 * ax99100_pci_gpio_chip_init: Initialize the GPIO chip.
 *
 * @pd:
 */
static void ax99100_pci_gpio_chip_init(struct priv_data *pd)
{
	/* Reset chip function */
	iowrite32(0x1, &pd->reg.common->swrst);
	mdelay(10);

	/* Disable global GPIO IRQ */
	iomod32(sGIxR(1), 0, &pd->reg.global_irq->gier);

	/* Disable / Confirm GPIO IRQs */
	iowrite32(0x0, &pd->reg.gpio->ede);
	iowrite32(0x00ffffff, &pd->reg.gpio->eds);

	iowrite32(0x0, &pd->reg.gpio->pin);
	iowrite32(0x00ffffc3, &pd->reg.gpio->dir); /* GPIO[5:2] are outputs */
	iowrite32(0x0, &pd->reg.gpio->em);
	iowrite32(0x0, &pd->reg.gpio->od);
	iowrite32(0x0, &pd->reg.gpio->pu);
	iowrite32(0x0, &pd->reg.gpio->ctr);
}

/**
 * ax99100_pci_gpio_chip_init: Initialize the GPIO chip.
 *
 * @pd:
 */
static void ax99100_pci_gpio_chip_deinit(struct priv_data *pd)
{
	/* Reset chip function */
	iowrite32(0x1, &pd->reg.common->swrst);
	mdelay(10);

	/* Disable global GPIO IRQ */
	iomod32(sGIxR(1), 0, &pd->reg.global_irq->gier);

	/* Disable GPIO IRQs */
	iowrite32(0x0, &pd->reg.gpio->ede);
}

/**
 * ax99100_pci_gpio_get_ngpio:
 *
 * @pci:
 *
 * Return:  Amount of available GPIOs.
 */
static int ax99100_pci_gpio_get_ngpio(struct pci_dev *pci)
{
	struct pci_dev *dummy_pci;
	int ngpio = 8; /* GPIO[7:0] */

	do {
		dummy_pci = pci_get_slot(pci->bus, 2);
		if (dummy_pci)
			break;
		ngpio += 8; /* GPIO[15:8] */

		dummy_pci = pci_get_slot(pci->bus, 3);
		if (!dummy_pci) {
			ngpio += 8; /* GPIO[23:16] */
			break;
		}
		if (dummy_pci->subsystem_device == HILSCHER_PCI_SUB_DEVICE_ID_SPI)
			ngpio += 1; /* GPIO[16] */
	} while (0);

	return ngpio;
}

static int ax99100_pci_gpio_irq_init_hw(struct gpio_chip *gc)
{
	struct priv_data *pd = gpiochip_get_data(gc);

	/* Confirm / Enable global GPIO IRQ */
	iowrite32(sGIxR(1), &pd->reg.global_irq->gicr);
	iomod32(0, sGIxR(1), &pd->reg.global_irq->gier);

	return 0;
}

static const struct irq_chip s_ic = {
	.name = KBUILD_MODNAME,
	.irq_mask = ax99100_pci_gpio_irq_mask,
	.irq_unmask = ax99100_pci_gpio_irq_unmask,
	.irq_set_type = ax99100_pci_gpio_irq_set_type,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
	.flags = IRQCHIP_IMMUTABLE,
	GPIOCHIP_IRQ_RESOURCE_HELPERS,
#endif
};

/**
 * ax99100_pci_gpio_probe:
 *
 * @pci:
 * @pci_id:
 *
 * Return:  0 on success; error value otherwise
 */
static int ax99100_pci_gpio_probe(struct pci_dev *pci, const struct pci_device_id *pci_id)
{
	struct device *dev = &pci->dev;
	struct priv_data *pd;
	struct gpio_chip *gc;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
	struct gpio_irq_chip *girq;
#endif
	int err;

	pd = devm_kzalloc(dev, sizeof(*pd), GFP_KERNEL);
	if (!pd)
		return -ENOMEM;

	pci_set_drvdata(pci, pd);
	pd->pci = pci;
	
	/* Enable PCI device and request regions */
	err = pci_enable_device(pci);
	if (err) {
		dev_err(dev, "Enable PCI device failed!\n");
		goto err0;
	}
	err = pci_request_regions(pci, KBUILD_MODNAME);
	if (err) {
		dev_info(dev, "Request PCI regions failed!\n");
		goto err1;
	}

	/* Map required memory regions */
	pd->reg.common = pci_iomap_range(pci, 1, 0x238, sizeof(*pd->reg.common));
	if (!pd->reg.common) {
		dev_err(dev, "Map PCI memory bar1 failed (common)!\n");
		err = -ENODEV;
		goto err2;
	}
	pd->reg.global_irq = pci_iomap_range(pci, 1, 0x3a0, sizeof(*pd->reg.global_irq));
	if (!pd->reg.global_irq) {
		dev_err(dev, "Map PCI memory bar1 failed (global_irq)!\n");
		err = -ENODEV;
		goto err3;
	}
	pd->reg.gpio = pci_iomap_range(pci, 5, 0x3c0, sizeof(*pd->reg.gpio));
	if (!pd->reg.gpio) {
		dev_err(dev, "Map PCI memory bar5 failed (gpio)!\n");
		err = -ENODEV;
		goto err4;
	}

	/* Configure the GPIO chip structure */
	gc = &pd->gpio_chip;
	gc->label = KBUILD_MODNAME;
	gc->parent = dev;
	gc->owner = THIS_MODULE;
	gc->base = -1;
	gc->ngpio = ax99100_pci_gpio_get_ngpio(pci);
	gc->direction_input = ax99100_pci_gpio_direction_input;
	gc->direction_output = ax99100_pci_gpio_direction_output;
	gc->get = ax99100_pci_gpio_get;
	gc->set = ax99100_pci_gpio_set;

	ax99100_pci_gpio_chip_init(pd);

	spin_lock_init(&pd->lock);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0)
	err = gpiochip_add_data(gc, pd);
	if (err) {
		dev_err(dev, "Register GPIO-Chip failed!\n");
		goto err5;
	}
#endif
	if (pci->irq > 0) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0)
		err = gpiochip_irqchip_add(gc, (struct irq_chip*)&s_ic, 0, handle_simple_irq, IRQ_TYPE_NONE);
		if (err < 0) {
			dev_err(dev, "Register IRQ-Chip failed!\n");
			goto err6;
		}
		gpiochip_set_chained_irqchip(gc, (struct irq_chip*)&s_ic, pci->irq, ax99100_pci_gpio_isr);
		ax99100_pci_gpio_irq_init_hw(gc);
#else
		girq = &gc->irq;
		gpio_irq_chip_set_chip( girq, &s_ic);
		girq->parent_handler = ax99100_pci_gpio_isr;
		girq->num_parents = 1;
		girq->parents = devm_kcalloc(dev, girq->num_parents,
					     sizeof(*girq->parents),
					     GFP_KERNEL);
		if (!girq->parents)
			return -ENOMEM;

		girq->parents[0] = pci->irq;
		girq->default_type = IRQ_TYPE_NONE;
		girq->handler = handle_level_irq;
		girq->init_hw = ax99100_pci_gpio_irq_init_hw;

		err = devm_gpiochip_add_data(dev, gc, pd);
		if (err < 0) {
			dev_err(dev, "Register IRQ-Chip failed!\n");
			goto err6;
		}
#endif
	}

	dev_info(dev, "gpiochip%d with %d GPIOs successfully probed!\n", gc->base, gc->ngpio);

	return 0;

err6:
	gpiochip_remove(gc);
	ax99100_pci_gpio_chip_deinit(pd);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0)
err5:
#endif
	pci_iounmap(pci, pd->reg.gpio);
err4:
	pci_iounmap(pci, pd->reg.global_irq);
err3:
	pci_iounmap(pci, pd->reg.common);
err2:
	pci_release_regions(pci);
err1:
	pci_disable_device(pci);
err0:
	kfree(pd);
	return err;
}

/**
 * ax99100_pci_gpio_remove:
 *
 * @pci:
 */
static void ax99100_pci_gpio_remove(struct pci_dev *pci)
{
	struct priv_data *pd = pci_get_drvdata(pci);

	if (pd) {
		/* No need to call gpiochip_remove(), the gpio chip automatically will  */
		/* be released when the device is unbound. See devm_gpiochip_add_data(). */
		ax99100_pci_gpio_chip_deinit(pd);

		pci_iounmap(pci, pd->reg.gpio);
		pci_iounmap(pci, pd->reg.global_irq);
		pci_iounmap(pci, pd->reg.common);
	}
	pci_release_regions(pci);
	pci_disable_device(pci);

	dev_info(&pci->dev, "gpiochip%d successfully removed!\n", pd->gpio_chip.base);
}

/* ------------------------------------------------------------------------- */

static const struct pci_device_id ax99100_pci_gpio_devices[] = {
	{ PCI_DEVICE_SUB(HILSCHER_PCI_VENDOR_ID, HILSCHER_PCI_DEVICE_ID,
		HILSCHER_PCI_SUB_VENDOR_ID, HILSCHER_PCI_SUB_DEVICE_ID_GPIO)},
	{ },
};
MODULE_DEVICE_TABLE(pci, ax99100_pci_gpio_devices);

/* ------------------------------------------------------------------------- */

static struct pci_driver ax99100_pci_gpio_driver = {
	.name = DRIVER_NAME,
	.id_table = ax99100_pci_gpio_devices,
	.probe = ax99100_pci_gpio_probe,
	.remove = ax99100_pci_gpio_remove,
};

static int __init ax99100_pci_gpio_init(void)
{
	pr_info("%s: %s\n", DRIVER_NAME, DRIVER_DESC);
	return pci_register_driver(&ax99100_pci_gpio_driver);
}
module_init(ax99100_pci_gpio_init);

static void __exit ax99100_pci_gpio_exit(void)
{
	pci_unregister_driver(&ax99100_pci_gpio_driver);
}
module_exit(ax99100_pci_gpio_exit);

MODULE_AUTHOR("Hilscher Gesellschaft fuer Systemautomation mbH");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL v2");
