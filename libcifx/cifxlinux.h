/* SPDX-License-Identifier: MIT */
/**************************************************************************************
 *
 * Copyright (c) 2024, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 * Description: Header file of Linux specific driver / toolkit initialization.
 *
 * Changes:
 *
 *   Version   Date        Author   Description
 *   ----------------------------------------------------------------------------------
 *   1        02.01.24    SD        changed licensing terms
 *
 **************************************************************************************/

#ifndef __CIFX_LINUX__H
#define __CIFX_LINUX__H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>

#define LIBRARYNAME          "LinuxCIFXDrv "
#define LINUXCIFXDRV_VERSION  LIBRARYNAME VERSION_INFO

#define APIENTRY
#ifdef CIFX_TOOLKIT_HWIF
#include "cifXHWFunctions.h"
#endif
#include "cifXUser.h"
#include "cifXErrors.h"

#define CIFX_DRIVER_INIT_NOSCAN     0  /*!< Don't automatically scan and add found devices                 */
#define CIFX_DRIVER_INIT_AUTOSCAN   1  /*!< Scan automatically for devices and add them to toolkit control */
#define CIFX_DRIVER_INIT_CARDNUMBER 2  /*!< Initialize specific card                                       */

#define CIFX_POLLINTERVAL_DISABLETHREAD  (~0) /*!< Disable polling completely */
#define DMA_BUFFER_COUNT            8

/*****************************************************************************/
/*! Driver initialization structure                                          */
/*****************************************************************************/
struct CIFX_LINUX_INIT
{
  int                   init_options;   /*!< see CIFX_DRIVER_INIT_XXX defines */

  const char*           base_dir;       /*!< base directory for device configuration */
  unsigned long         poll_interval;  /*!< Poll interval in ms for non-irq cards   */
  int                   poll_priority;  /*!< Poll thread priority                    */
  unsigned long         trace_level;    /*!< see TRACE_LVL_XXX defines               */
  int                   user_card_cnt;  /*!< Number of user defined cards            */
  struct CIFX_DEVICE_T* user_cards;     /*!< Pointer to Array of user cards (must be user_card_cnt long) */

  int                   iCardNumber;
  int                   fEnableCardLocking;
  int                   poll_StackSize;   /*!< Stack size of polling thread            */
  int                   poll_schedpolicy; /*!< Schedule policy of poll thread          */
  FILE*                 logfd;
};

int32_t cifXDriverInit(const struct CIFX_LINUX_INIT* init_params);
void    cifXDriverDeinit();
int32_t cifXGetDriverVersion(uint32_t ulSize, char* szVersion);

typedef int32_t (*PFN_DRV_HWIF_INIT)   ( struct CIFX_DEVICE_T* ptDevice);
typedef void    (*PFN_DRV_HWIF_EXIT)   ( struct CIFX_DEVICE_T* ptDevice);
typedef void*   (*PFN_DRV_HWIF_MEMCPY) ( struct CIFX_DEVICE_T* ptDevice, void* pvAddr, void* pvData, uint32_t ulLen);

/*****************************************************************************/
/*! Notification events                                                      */
/*****************************************************************************/
typedef enum CIFX_NOTIFY_Etag
{
  eCIFX_EVENT_PRERESET        = 0, /*!< Event signalled, before device is reset (HW Reset) */
  eCIFX_EVENT_POSTRESET,           /*!< Called after HW reset has been executed            */
  eCIFX_EVENT_PRE_BOOTLOADER,      /*!< Called before bootloader is downloaded             */
  eCIFX_EVENT_POST_BOOTLOADER,     /*!< Called after bootloader was downloaded and started */

} CIFX_NOTIFY_E;

typedef void(*PFN_CIFX_NOTIFY_EVENT)(struct CIFX_DEVICE_T* ptDevice, CIFX_NOTIFY_E eEvent);

/*****************************************************************************/
/*! Memory types                                                             */
/*****************************************************************************/
typedef enum CIFX_MEM_TYPE_Etag
{
  eMEM_DPM,
  eMEM_EXTMEM,
  eMEM_DMA
} CIFX_MEM_TYPE_E;

/*****************************************************************************/
/*! DMA memory information                                                   */
/*****************************************************************************/
typedef struct DMABUFFER_Ttag
{
  uint32_t ulSize;            /*!< DMA buffer size  */
  uint32_t ulPhysicalAddress; /*!< Physical address of the buffer */
  void*    pvBuffer;          /*!< Pointer to the buffer */
} DMABUFFER_T;

/*****************************************************************************/
/*! Device structure for manually adding devices                             */
/*****************************************************************************/
struct CIFX_DEVICE_T
{
  unsigned char*  dpm;        /*!< virtual pointer to DPM  */
  unsigned long   dpmaddr;    /*!< physical address to DPM, this parameter will be used for PCI cards to detect bus address */
  unsigned long   dpmlen;     /*!< Length of DPM in bytes  */

  int             uio_num;    /*!< uio number, -1 for non-uio devices      */
  int             uio_fd;     /*!< uio file handle, -1 for non-uio devices */

  int             pci_card;   /*!< !=0 if device is a pci card */
  int             force_ram;  /*!< Force usage of RAM instead of flash. Card will always be reset and all
                                   files are downloaded again */

  PFN_CIFX_NOTIFY_EVENT notify; /*!< Function to call, after the card has passed several stages (usually needed on RAM based
                                     devices, that change DPM configuration during initialization) */

  void*           userparam;  /*!< User specific parameter (e.g. identifier for manual added devices */

  unsigned char*  extmem;     /*!< virtual pointer to extended memory  */
  unsigned long   extmemaddr; /*!< physical address to extended memory */
  unsigned long   extmemlen;  /*!< Length of extended memory in bytes  */

  /* DMA Buffer Structure */
  uint32_t            dma_buffer_cnt;                /*!< Number of available DMA buffers  */
  DMABUFFER_T         dma_buffer[DMA_BUFFER_COUNT];  /*!< DMA buffer definition for the device */

  /* function interface required when using the toolkit's hardware functions */
  PFN_DRV_HWIF_INIT   hwif_init;   /*!< Function initializes custom hw-function interface */
  PFN_DRV_HWIF_EXIT   hwif_deinit; /*!< Function de-initializes custom hw-function interface */
  PFN_DRV_HWIF_MEMCPY hwif_read;   /*!< Function provides read access to the DPM via custom hardware interface */
  PFN_DRV_HWIF_MEMCPY hwif_write;  /*!< Function provides write access to the DPM via custom hardware interface */
};

int                   cifXGetDeviceCount(void);
struct CIFX_DEVICE_T* cifXFindDevice(int iNum, int fForceOpenDevice);
void                  cifXDeleteDevice(struct CIFX_DEVICE_T* device);

#define CIFX_UIO_MAP_NO_FOR_DPM    0             /*!< Offset of uio driver to mmap             */

#define CIFX_UIO_PLX_CARD_NAME     "netx_plx"    /*!< uio name of a NXSB-PCA or NXPCA-PCI card */
#define CIFX_UIO_CARD_NAME         "netx"        /*!< uio name of a cifX PCI card              */
#define CIFX_UIO_NETPLC_CARD_NAME  "netplc"      /*!< uio name of a netPLC PCI card            */
#define CIFX_UIO_NETJACK_CARD_NAME "netjack"     /*!< uio name of a netJACK PCI card           */
#define CIFX_UIO_CUSTOM_CARD_NAME  "netx_custom" /*!< name of user defined cards               */

#define UIO_NETX_START_TYPE_AUTO      "auto"
#define UIO_NETX_START_TYPE_RAM       "ram"
#define UIO_NETX_START_TYPE_FLASH     "flash"
#define UIO_NETX_START_TYPE_DONTTOUCH "donttouch"

/* UIO helper functions to allow users, to find own uio devices, without the need to reimplement the uio access functions */
int           cifx_uio_open(int uio_num, int fForceOpenDevice);
int           cifx_ISA_open(void);
void          cifx_ISA_close(int isa_fd);
unsigned long cifx_uio_get_mem_size(int uio_num, int bar);
unsigned long cifx_uio_get_mem_addr(int uio_num, int bar);
int           cifx_uio_validate_name(int uio_num, const char* name);
int           cifx_uio_map_dpm(int uio_fd, int uio_num, void** dpmbase, unsigned long* dpmaddr, unsigned long* dpmlen);
int           cifx_ISA_map_dpm(int fd, void** dpmbase, int dpmaddr, int dpmlen);
void          cifx_ISA_unmap_dpm(void* dpmbase, int dpmlen);

#define CIFX_PLUGIN_GET_DEVICE_COUNT "cifx_device_count"
typedef uint32_t(*PFN_CIFX_PLUGIN_GET_DEVICE_COUNT)(void);
#define CIFX_PLUGIN_ALLOC_DEVICE "cifx_alloc_device"
typedef struct CIFX_DEVICE_T*(*PFN_CIFX_PLUGIN_ALLOC_DEVICE)(uint32_t num);
#define CIFX_PLUGIN_FREE_DEVICE "cifx_free_device"
typedef void(*PFN_CIFX_PLUGIN_FREE_DEVICE)(struct CIFX_DEVICE_T*);

#ifdef __cplusplus
}
#endif

#endif /* __CIFX_LINUX__H */
