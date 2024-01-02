// SPDX-License-Identifier: GPL-2.0
/*
 * AX99100 PCIe to Multi I/O Controller - SPI driver
 *
 * drivers/spi/ax99100-pci-spi.c
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

#define DRIVER_DESC "AX99100 PCIe to Multi I/O Controller - SPI driver"
#define DRIVER_NAME "ax99100-pci-spi"

#define HILSCHER_PCI_VENDOR_ID  0x15cf
#define HILSCHER_PCI_DEVICE_ID  0x0090

#define HILSCHER_PCI_SUB_VENDOR_ID       0x15cf
#define HILSCHER_PCI_SUB_DEVICE_ID_GPIO  0x1002
#define HILSCHER_PCI_SUB_DEVICE_ID_SPI   0x6001

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>

#include <linux/version.h>

/* -------------------------------------------------------------------------- */
/* ------ Global chip settings ---------------------------------------------- */
/* -------------------------------------------------------------------------- */

#define MAX_SPEED_HZ    (DIV_ROUND_UP(125000000, 3)) /* ~41,67MHz (max. Chip frequency = 42MHz) */
#define MIN_SPEED_HZ    (DIV_ROUND_UP(125000000, 255)) /* ~490,197kHz */

/* NOTE: Currently only the internal CS decoder is supported! */
#define NUM_CHIPSELECT  (3)

#define DMA_MIN_SIZE    (8)
#define DMA_MAX_SIZE    (64 * 1024)

/* -------------------------------------------------------------------------- */
/* ------ Regdefs ----------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

#define regdef(mask, shift, name) \
static inline uint8_t g##name(uint8_t val)\
{ \
	return (val >> shift) & mask; \
} \
static inline uint8_t s##name(uint8_t val) \
{ \
	return (val & mask) << shift; \
}

/* ------ BAR0 Register (I/O mapped) ---------------------------------------- */

#pragma pack(1)
struct regdef_spi {
	uint8_t cm;
	uint8_t css;
	uint8_t res0[2];
	uint8_t br;
	uint8_t ds;
	uint8_t dt;
	uint8_t sdaof;
	uint8_t stof[8];
	uint8_t sdfl0;
	uint8_t sdfl1;
	uint8_t ssol;
	uint8_t sdc;
	uint8_t mis;
};
#pragma pack()

regdef(0x1, 0, CM_SSP)
regdef(0x1, 1, CM_CPHA)
regdef(0x1, 2, CM_CPOL)
regdef(0x1, 3, CM_LSB)
regdef(0x1, 4, CM_SPIMEN)
regdef(0x1, 5, CM_ASS)
regdef(0x1, 6, CM_SWE)
regdef(0x1, 7, CM_SSOE)

regdef(0x3, 0, CSS_SPICKS)
regdef(0x1, 2, CSS_DIVEN)

regdef(0xff, 0, BR_DIVIDER)

regdef(0x7, 0, SSOL_SS)
regdef(0x1, 3, SSOL_EDE)
regdef(0x7, 4, SSOL_STOL)

regdef(0x1, 0, SDC_ETDMA)
regdef(0x1, 1, SDC_ERDMA)
regdef(0x1, 2, SDC_OPCFE)
regdef(0x1, 3, SDC_DMA_GO)
regdef(0x1, 4, SDC_FBT)
regdef(0x1, 5, SDC_CSS)
regdef(0x1, 6, SDC_STCIE)
regdef(0x1, 7, SDC_STERRIE)

regdef(0x1, 0, MIS_STC)
regdef(0x1, 1, MIS_STERR)

/* ------ BAR1 Register (MEM mapped) ---------------------------------------- */

#pragma pack(1)
struct regdef_dma {
	uint32_t sar0;
	uint32_t sar1;
	uint32_t lr;
	uint32_t star;
	uint32_t stpr;
	uint32_t sr;
	uint32_t bntxr; /* TBNTS / RBNRR*/
};
#pragma pack()

#pragma pack(1)
struct regdef_common {
	uint32_t swrst;
};
#pragma pack()

/* -------------------------------------------------------------------------- */
/* ------ Global variables -------------------------------------------------- */
/* -------------------------------------------------------------------------- */

struct priv_data {
	struct pci_dev *pci;

	struct spi_master *spi_master;
	struct spi_device *spi[NUM_CHIPSELECT];

	struct {
		struct regdef_spi __iomem *spi;
		struct regdef_dma __iomem *txdma;
		struct regdef_dma __iomem *rxdma;
		struct regdef_common __iomem *common;
	} reg;

	/* Used for OP-Code-Field transfer mode */
	const uint8_t *tx_buf;
	uint8_t *rx_buf;
	uint32_t tx_len;
	uint32_t rx_len;

	/* Used for DMA transfer mode */
	struct sg_table tx_sgt;
	struct sg_table rx_sgt;

	struct {
		uint32_t use_opcf : 1; /* OP-Code field usage */
		uint32_t use_dma : 1; /* DMA usage */
	} flags;
};

/* -------------------------------------------------------------------------- */
/* ------ Macros ------------------------------------------------------------ */
/* -------------------------------------------------------------------------- */

#define iomod8(cmask, smask, addr) \
do { \
	uint8_t val = ioread8(addr); \
	val &= ~(cmask); \
	val |= (smask); \
	iowrite8(val, addr); \
} while (0)

#define iomod32(cmask, smask, addr) \
do { \
	uint32_t val = ioread32(addr); \
	val &= ~(cmask); \
	val |= (smask); \
	iowrite32(val, addr); \
} while (0)

/* -------------------------------------------------------------------------- */
/* ------ Help functions ---------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/**
 * ax99100_pci_spi_set_frequency: Set requested transfer frequency.
 *
 * @spi:   Pointer to the device structure which provides information about the slave device.
 * @freq:  Requested SPI frequency which should be configured.
 */
static void ax99100_pci_spi_set_frequency(struct spi_device *spi, uint32_t freq)
{
	struct priv_data *pd = spi_master_get_devdata(spi->master);

	if (freq > spi->max_speed_hz)
		dev_warn(&spi->dev, "The requested frequency (%dkHz) exceeds the defined max. frequency (%dkHz) of the SPI device.",
			 freq/1000, spi->max_speed_hz/1000);

	iomod8(sCSS_DIVEN(1), 0, &pd->reg.spi->css);
	iomod8(sBR_DIVIDER(-1), sBR_DIVIDER(DIV_ROUND_UP(125000000, freq)), &pd->reg.spi->br);
	iomod8(0, sCSS_DIVEN(1), &pd->reg.spi->css);
}

/**
 * ax99100_pci_spi_set_mode: Set requested SPI transfer mode.
 *
 * @spi:   Pointer to the device structure which provides information about the slave device.
 * @mode:  Requested SPI mode which should be configured.
 */
static void ax99100_pci_spi_set_mode(struct spi_device *spi, uint32_t mode)
{
	struct priv_data *pd = spi_master_get_devdata(spi->master);

	iomod8(sCM_CPHA(-1) | sCM_CPOL(-1),
	       sCM_CPHA(!!(mode & SPI_CPHA)) | sCM_CPOL(!!(mode & SPI_CPOL)), &pd->reg.spi->cm);
}

/**
 * ax99100_pci_spi_enable_irq:  Enable IRQs of SPI controller.
 *
 * @sm:  Pointer to the spi_master structure which provides information about the controller.
 */
static void ax99100_pci_spi_enable_irq(struct spi_master *sm)
{
	struct priv_data *pd = spi_master_get_devdata(sm);

	iomod8(0, sSDC_STCIE(1) | sSDC_STERRIE(1), &pd->reg.spi->sdc);
}

/**
 * ax99100_pci_spi_disable_irq:  Disable IRQs of SPI controller.
 *
 * @sm:  Pointer to the spi_master structure which provides information about the controller.
 */
static void ax99100_pci_spi_disable_irq(struct spi_master *sm)
{
	struct priv_data *pd = spi_master_get_devdata(sm);

	iomod8(sSDC_STCIE(1) | sSDC_STERRIE(1), 0, &pd->reg.spi->sdc);
}

/* -------------------------------------------------------------------------- */
/* ------ DMA functions ----------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/**
 * ax99100_pci_spi_do_dma_transfer:  Processing the SPI DMA transfer.
 *
 * @sm:  Pointer to the spi_master structure which provides information about the controller.
 *
 * Return:  Always 0
 */
static inline void ax99100_pci_spi_do_dma_transfer(struct spi_master *sm)
{
	struct priv_data *pd = spi_master_get_devdata(sm);
	struct scatterlist *rx_sgl, *tx_sgl;

	if (!pd->rx_sgt.nents && !pd->tx_sgt.nents) {
		pd->flags.use_dma = 0;

		/* Disable IRQs */
		ax99100_pci_spi_disable_irq(sm);

		/* Wake up the framework */
		spi_finalize_current_transfer(sm);

		return;
	}

	pd->flags.use_dma = 1;
	if (pd->rx_sgt.nents--) {
		rx_sgl = pd->rx_sgt.sgl++;
		iowrite32(lower_32_bits(sg_dma_address(rx_sgl)), &pd->reg.rxdma->sar0); /* Set DMA lower 32-bit addr */
		iowrite32(upper_32_bits(sg_dma_address(rx_sgl)), &pd->reg.rxdma->sar1); /* Set DMA upper 32-bit addr */
		iowrite32(sg_dma_len(rx_sgl), &pd->reg.rxdma->lr); /* Set DMA transfer length */
		iowrite32(0x1, &pd->reg.rxdma->star); /* Enable RX DMA*/
		iomod8(0, sSDC_ERDMA(1), &pd->reg.spi->sdc);
	}
	if (pd->tx_sgt.nents--) {
		tx_sgl = pd->tx_sgt.sgl++;
		iowrite32(lower_32_bits(sg_dma_address(tx_sgl)), &pd->reg.txdma->sar0); /* Set DMA lower 32-bit addr */
		iowrite32(upper_32_bits(sg_dma_address(tx_sgl)), &pd->reg.txdma->sar1); /* Set DMA upper 32-bit addr */
		iowrite32(sg_dma_len(tx_sgl), &pd->reg.txdma->lr); /* Set DMA transfer length */
		iowrite32(0x1, &pd->reg.txdma->star); /* Enable TX DMA*/
		iomod8(0, sSDC_ETDMA(1), &pd->reg.spi->sdc);
	}

	ax99100_pci_spi_enable_irq(sm);

	/* Start DMA processing */
	iomod8(0, sSDC_DMA_GO(1), &pd->reg.spi->sdc);
}

/* -------------------------------------------------------------------------- */
/* ------ OPCF functions ---------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/**
 * ax99100_pci_spi_read_fifo:  Read as many bytes as possible from STOF registers.
 *
 * @sm:  Pointer to the spi_master structure which provides information about the controller.
 */
static inline void ax99100_pci_spi_read_fifo(struct spi_master *sm)
{
	struct priv_data *pd = spi_master_get_devdata(sm);
	uint8_t nbytes = min_t(uint32_t, pd->rx_len, 8);
	uint8_t byte, i = 0;

	if (!nbytes)
		return;

	/* Read STOF data */
	while (nbytes-- && pd->rx_len--) {
		byte = ioread8(&pd->reg.spi->stof[i++]);
		if (pd->rx_buf)
			*pd->rx_buf++ = byte;
	}
}

/**
 * ax99100_pci_spi_write_fifo:  Write as many bytes as possible to STOF registers.
 *
 * @sm:  Pointer to the spi_master structure which provides information about the controller.
 */
static inline void ax99100_pci_spi_write_fifo(struct spi_master *sm)
{
	struct priv_data *pd = spi_master_get_devdata(sm);
	uint8_t nbytes = min_t(uint32_t, pd->tx_len, 8);
	uint8_t byte, i = 0;

	if (!nbytes) {
		pd->flags.use_opcf = 0;

		/* Disable IRQs */
		ax99100_pci_spi_disable_irq(sm);

		/* Wake up the framework */
		spi_finalize_current_transfer(sm);

		return;
	}
	pd->flags.use_opcf = 1;

	/* Set STOF length */
	iomod8(sSSOL_STOL(-1), sSSOL_STOL(nbytes - 1), &pd->reg.spi->ssol);

	/* Write STOF data */
	while (nbytes-- && pd->tx_len--) {
		byte = pd->tx_buf ? *pd->tx_buf++ : 0;
		iowrite8(byte, &pd->reg.spi->stof[i++]);
	}

	ax99100_pci_spi_enable_irq(sm);

	/* Start OPCF processing */
	iomod8(0, sSDC_OPCFE(1) | sSDC_DMA_GO(1), &pd->reg.spi->sdc);
}

/* -------------------------------------------------------------------------- */
/* ------ API functions of SPI master --------------------------------------- */
/* -------------------------------------------------------------------------- */

/**
 * ax99100_pci_spi_can_dma:  Checks for a possible dma transfer.
 *
 * @sm:    Pointer to the spi_master structure which provides information about the controller.
 * @spi:   Pointer to the device structure which provides information about the slave device.
 * @transfer:  Pointer to the spi_transfer structure which provide information about next transfer parameters
 *
 * Return:  true, if a dma transfer is possible; otherwise false
 */
static bool ax99100_pci_spi_can_dma(struct spi_master *sm, struct spi_device *spi, struct spi_transfer *transfer)
{
	if (transfer->len < DMA_MIN_SIZE)
		return false;
	return true;
}

/**
 * ax99100_pci_spi_set_cs:  Select or deselect the chip select line.
 *
 * @spi:       Pointer to the spi_device structure
 * @deselect:  Select(0) or deselect (1) the chip select line
 */
static void ax99100_pci_spi_set_cs(struct spi_device *spi, bool deselect)
{
	struct priv_data *pd = spi_master_get_devdata(spi->master);

	/* NOTE:
	 * To prevent toggling of SCLK and MOSI/MISO,
	 * we do the SPI mode selection in front of processing the chip select.
	 */
	ax99100_pci_spi_set_mode(spi, spi->mode);

	if (gSSOL_EDE(ioread8(&pd->reg.spi->ssol)))
		iomod8(sSSOL_SS(-1), sSSOL_SS((deselect) ? (-1) : (spi->chip_select)), &pd->reg.spi->ssol);
	else
		iomod8(sSSOL_SS(-1), sSSOL_SS((deselect) ? (-1) : ~(1 << spi->chip_select)), &pd->reg.spi->ssol);
}

/**
 * ax99100_pci_spi_transfer_one:  Initiates the SPI transfer.
 *
 * @sm:   Pointer to the spi_master structure which provides information about the controller.
 * @spi:  Pointer to the spi_device structure
 * @transfer:  Pointer to the spi_transfer structure which provide information about next transfer parameters
 *
 * Return:  Always 1 to signal that the transfer is still in progress
 */
static int ax99100_pci_spi_transfer_one(struct spi_master *sm, struct spi_device *spi, struct spi_transfer *transfer)
{
	struct priv_data *pd = spi_master_get_devdata(sm);

	ax99100_pci_spi_set_frequency(spi, transfer->speed_hz);

	/* MSB or LSB first? */
	iomod8(sCM_LSB(-1), sCM_LSB((spi->mode & SPI_LSB_FIRST) ? 1 : 0), &pd->reg.spi->cm);

	if ((transfer->rx_sg.nents > 0) || (transfer->tx_sg.nents > 0)) {
		/* As we modify the sg_table structures while processing,
		 * we creates private copies of these!
		 */
		pd->rx_sgt = transfer->rx_sg;
		pd->tx_sgt = transfer->tx_sg;

		/* Start DMA transfer */
		ax99100_pci_spi_do_dma_transfer(sm);
	} else {
		/* As we modify the buffer pointers and length fields while processing,
		 * we creates private copies of these!
		 */
		pd->rx_buf = transfer->rx_buf;
		pd->tx_buf = transfer->tx_buf;
		pd->rx_len = transfer->len;
		pd->tx_len = transfer->len;

		/* Start OPCF transfer */
		ax99100_pci_spi_write_fifo(sm);
	}

	return 1;
}

/* -------------------------------------------------------------------------- */
/* ------ IRQ functions ----------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/**
 * ax99100_pci_spi_isr: Interrupt subroutine of SPI controller.
 *
 * @irq:     IRQ number
 * @dev_id:  Pointer to the spi_master structure which provides information about the controller.
 *
 * Return:  IRQ_HANDLED on success; error value otherwise
 */
static irqreturn_t ax99100_pci_spi_isr(int irq, void *dev_id)
{
	struct spi_master *sm = dev_id;
	struct priv_data *pd = spi_master_get_devdata(sm);
	uint8_t status;

	status = ioread8(&pd->reg.spi->mis);
	if (status & sMIS_STC(1)) {
		if (pd->flags.use_dma)
			ax99100_pci_spi_do_dma_transfer(sm);

		if (pd->flags.use_opcf) {
			/* Read as many bytes as possible from OPCF */
			ax99100_pci_spi_read_fifo(sm);
			/* Write as many bytes as possible to OPCF */
			ax99100_pci_spi_write_fifo(sm);
		}
	}
	if (status & sMIS_STERR(1)) {
		dev_err(&sm->cur_msg->spi->dev, "SPI transceiver error interrupt occurred\n");
		/* TODO: Implementation of error handling! */
	}
	iowrite8(status, &pd->reg.spi->mis);

	return IRQ_HANDLED;
}

/* -------------------------------------------------------------------------- */
/* ------ Driver / Module functions ----------------------------------------- */
/* -------------------------------------------------------------------------- */

struct spi_board_info spi_slave_devices[NUM_CHIPSELECT] = {0};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
    module_param_named(mode0, spi_slave_devices[0].mode, uint, 0644);
    module_param_named(mode1, spi_slave_devices[1].mode, uint, 0644);
    module_param_named(mode2, spi_slave_devices[2].mode, uint, 0644);
#else
    module_param_named(mode0, spi_slave_devices[0].mode, ushort, 0644);
    module_param_named(mode1, spi_slave_devices[1].mode, ushort, 0644);
    module_param_named(mode2, spi_slave_devices[2].mode, ushort, 0644);
#endif

module_param_string(modalias0, spi_slave_devices[0].modalias, sizeof(spi_slave_devices[0].modalias), 0644);
module_param_named(max_speed_hz0, spi_slave_devices[0].max_speed_hz, uint, 0644);

module_param_string(modalias1, spi_slave_devices[1].modalias, sizeof(spi_slave_devices[0].modalias), 0644);
module_param_named(max_speed_hz1, spi_slave_devices[1].max_speed_hz, uint, 0644);

module_param_string(modalias2, spi_slave_devices[2].modalias, sizeof(spi_slave_devices[0].modalias), 0644);
module_param_named(max_speed_hz2, spi_slave_devices[2].max_speed_hz, uint, 0644);

/**
 * ax99100_pci_spi_new_devices: Add optional SPI slave devices defined by module parameters.
 */
static void ax99100_pci_spi_new_devices(struct spi_master *sm)
{
	struct priv_data *pd = spi_master_get_devdata(sm);
	int i;

	for (i = 0; i < NUM_CHIPSELECT; i++) {
		if (!strlen(spi_slave_devices[i].modalias))
			continue;

		spi_slave_devices[i].chip_select = i;

		pd->spi[i] = spi_new_device(sm, &spi_slave_devices[i]);
		if (!pd->spi[i])
			dev_info(&pd->pci->dev,
				 "Add SPI slave (cs=%i, modalias=%s, mode=%d, max_speed_hz=%d) failed!\n", i,
				 spi_slave_devices[i].modalias, spi_slave_devices[i].mode,
				 spi_slave_devices[i].max_speed_hz);
		else
			dev_info(&pd->pci->dev,
				 "SPI slave (cs=%i, modalias=%s, mode=%d, max_speed_hz=%d) successfully added.\n", i,
				 spi_slave_devices[i].modalias, spi_slave_devices[i].mode,
				 spi_slave_devices[i].max_speed_hz);
	}
}

/**
 * ax99100_pci_spi_unregister_devices: Unregister optional SPI slave devices defined by module parameters.
 */
static void ax99100_pci_spi_unregister_devices(struct spi_master *sm)
{
	struct priv_data *pd = spi_master_get_devdata(sm);
	int i;

	for (i = 0; i < NUM_CHIPSELECT; i++) {
		if (pd->spi[i]) {
			spi_unregister_device(pd->spi[i]);
			pd->spi[i] = NULL;
		}
	}
}

/**
 * ax99100_pci_spi_chip_init: Initialize the SPI controller chip.
 *
 * @sm:  Pointer to the spi_master structure which provides information about the controller.
 */
static void ax99100_pci_spi_chip_init(struct spi_master *sm)
{
	struct priv_data *pd = spi_master_get_devdata(sm);

	/* Reset chip function */
	iowrite32(0x1, &pd->reg.common->swrst);
	mdelay(10);

	/* Use internal 125MHz PLL clock as clock source */
	iomod8(sCSS_SPICKS(-1), sCSS_SPICKS(0), &pd->reg.spi->css);

	/* Enable SPI controller and slave select pins */
	iowrite8(sCM_SPIMEN(1) | sCM_SSOE(1), &pd->reg.spi->cm);
}

/**
 * ax99100_pci_spi_chip_dinit: Deinitialize the SPI controller chip.
 *
 * @sm:  Pointer to the spi_master structure which provides information about the controller.
 */
static void ax99100_pci_spi_chip_deinit(struct spi_master *sm)
{
	struct priv_data *pd = spi_master_get_devdata(sm);

	/* Reset chip function */
	iowrite32(0x1, &pd->reg.common->swrst);
	mdelay(10);
}

/**
 * ax99100_pci_spi_probe:
 *
 * @pci:  Pointer to the pci device of spi master.
 * @pci_id:
 *
 * Return:  0 on success; error value otherwise
 */
static int ax99100_pci_spi_probe(struct pci_dev *pci, const struct pci_device_id *pci_id)
{
	struct device *dev = &pci->dev;
	struct priv_data *pd;
	struct spi_master *sm;
	int err;

	sm = spi_alloc_master(dev, sizeof(*pd));
	if (!sm)
		return -ENOMEM;

	pd = spi_master_get_devdata(sm);
	pd->spi_master = sm;

	pci_set_drvdata(pci, pd);
	pd->pci = pci;

	/* Enable PCI device, request regions and enable bus-master for DMA transfers */
	err = pci_enable_device(pci);
	if (err) {
		dev_err(dev, "Enable PCI device failed!\n");
		goto err0;
	}
	err = pci_request_regions(pci, dev_name(dev));
	if (err) {
		dev_info(dev, "Request PCI regions failed!\n");
		goto err1;
	}
	err = dma_set_mask(&pci->dev, DMA_BIT_MASK(64));
	if (err) {
		dev_info(dev, "Set DMA mask failed!\n");
		goto err2;
	}
	pci_set_master(pci);

	/* Map required IO regions */
	pd->reg.spi = pci_iomap_range(pci, 0, 0x0, sizeof(*pd->reg.spi));
	if (!pd->reg.spi) {
		dev_err(dev, "Map PCI IO bar0 failed (spi)!\n");
		err = -ENODEV;
		goto err5;
	}

	/* Map required memory regions */
	pd->reg.txdma = pci_iomap_range(pci, 1, 0x80, sizeof(*pd->reg.txdma));
	if (!pd->reg.txdma) {
		dev_err(dev, "Map PCI memory bar1 failed (txdma)!\n");
		err = -ENODEV;
		goto err2;
	}
	pd->reg.rxdma = pci_iomap_range(pci, 1, 0x100, sizeof(*pd->reg.rxdma));
	if (!pd->reg.rxdma) {
		dev_err(dev, "Map PCI memory bar1 failed (rxdma)!\n");
		err = -ENODEV;
		goto err3;
	}
	pd->reg.common = pci_iomap_range(pci, 1, 0x238, sizeof(*pd->reg.common));
	if (!pd->reg.common) {
		dev_err(dev, "Map PCI memory bar1 failed (common)!\n");
		err = -ENODEV;
		goto err4;
	}

	/* Install IRQ handler */
	err = devm_request_irq(dev, pci->irq, ax99100_pci_spi_isr, IRQF_SHARED, KBUILD_MODNAME, sm);
	if (err) {
		dev_err(dev, "Register IRQ%d failed!\n", pci->irq);
		goto err6;
	}

	/* Configure the SPI master structure */
	sm->dev.of_node = dev->of_node;
	sm->mode_bits = SPI_CPOL | SPI_CPHA | SPI_LSB_FIRST;
	sm->bits_per_word_mask = SPI_BPW_MASK(8);
	sm->max_speed_hz = MAX_SPEED_HZ;
	sm->min_speed_hz = MIN_SPEED_HZ;
	sm->num_chipselect = NUM_CHIPSELECT;

	sm->set_cs = ax99100_pci_spi_set_cs;
	sm->transfer_one = ax99100_pci_spi_transfer_one;

	sm->can_dma = ax99100_pci_spi_can_dma;
	sm->max_dma_len = DMA_MAX_SIZE;

	ax99100_pci_spi_chip_init(sm);

	err = devm_spi_register_master(dev, sm);
	if (err) {
		dev_err(dev, "Register SPI master failed!\n");
		goto err7;
	}

	dev_info(dev, "%s successfully initialized!\n", dev_name(&sm->dev));

	/* Add optional SPI devices defined by module parameters */
	ax99100_pci_spi_new_devices(sm);

	return 0;

err7:
	ax99100_pci_spi_chip_deinit(sm);
err6:
	pci_iounmap(pci, pd->reg.spi);
err5:
	pci_iounmap(pci, pd->reg.common);
err4:
	pci_iounmap(pci, pd->reg.rxdma);
err3:
	pci_iounmap(pci, pd->reg.txdma);
err2:
	pci_clear_master(pci);
	pci_release_regions(pci);
err1:
	pci_disable_device(pci);
err0:
	spi_master_put(sm);

	return err;
}

/**
 * ax99100_pci_spi_remove:
 *
 * @pci:
 */
static void ax99100_pci_spi_remove(struct pci_dev *pci)
{
	struct priv_data *pd = pci_get_drvdata(pci);
	struct spi_master *sm = pd->spi_master;

	ax99100_pci_spi_unregister_devices(sm);
	ax99100_pci_spi_chip_deinit(sm);

	pci_iounmap(pci, pd->reg.spi);
	pci_iounmap(pci, pd->reg.common);
	pci_iounmap(pci, pd->reg.rxdma);
	pci_iounmap(pci, pd->reg.txdma);
	pci_clear_master(pci);
	pci_release_regions(pci);
	pci_disable_device(pci);

	dev_info(&pci->dev, "%s successfully removed!\n", dev_name(&sm->dev));
}

/* ------------------------------------------------------------------------- */

static const struct pci_device_id ax99100_pci_spi_devices[] = {
	{ PCI_DEVICE_SUB(HILSCHER_PCI_VENDOR_ID, HILSCHER_PCI_DEVICE_ID,
		HILSCHER_PCI_SUB_VENDOR_ID, HILSCHER_PCI_SUB_DEVICE_ID_SPI)},
	{ },
};
MODULE_DEVICE_TABLE(pci, ax99100_pci_spi_devices);

/* ------------------------------------------------------------------------- */

static struct pci_driver ax99100_pci_spi_driver = {
	.name = DRIVER_NAME,
	.id_table = ax99100_pci_spi_devices,
	.probe = ax99100_pci_spi_probe,
	.remove = ax99100_pci_spi_remove,
};

static int __init ax99100_pci_spi_init(void)
{
	pr_info("%s: %s\n", DRIVER_NAME, DRIVER_DESC);
	return pci_register_driver(&ax99100_pci_spi_driver);
}
module_init(ax99100_pci_spi_init);

static void __exit ax99100_pci_spi_exit(void)
{
	pci_unregister_driver(&ax99100_pci_spi_driver);
}
module_exit(ax99100_pci_spi_exit);

MODULE_AUTHOR("Hilscher Gesellschaft fuer Systemautomation mbH");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL v2");
