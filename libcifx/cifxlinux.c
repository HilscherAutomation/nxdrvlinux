// SPDX-License-Identifier: MIT
/**************************************************************************************
 *
 * Copyright (c) 2024, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 * Description: Linux specific driver / toolkit initialization
 *
 * Changes:
 *
 *   Version   Date        Author   Description
 *   ----------------------------------------------------------------------------------
 *   1        02.01.24    SD        changed licensing terms
 *
 **************************************************************************************/

#include "cifXToolkit.h"
#include "cifXHWFunctions.h"
#include "cifxlinux.h"
#include "cifXUser.h"
#include "cifxlinux_internal.h"
#include "HilPCIDefs.h"
#ifdef CIFXETHERNET
#include "netx_tap.h"
#endif

#ifdef CIFX_PLUGIN_SUPPORT
#include <dlfcn.h>
#include <sys/queue.h>
#endif

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>

#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <limits.h>

#include <errno.h>
#include <string.h>

extern char*            g_szDriverBaseDir;
extern void*            g_pvTkitLock;
extern unsigned long    g_ulDeviceCount;
extern PDEVICEINSTANCE* g_pptDevices;
#ifdef CIFXETHERNET
extern void*            g_eth_list_lock;
#endif
FILE*                   g_logfd = 0;

#ifdef CIFX_PLUGIN_SUPPORT
struct CIFX_PLUGIN_T
{
  SLIST_ENTRY(CIFX_PLUGIN_T) tList;
  void*                      hPluginFile;
  uint32_t                   ulDeviceCount;
  struct CIFX_DEVICE_T**     aptDevices;
};

static SLIST_HEAD(PLUGIN_LIST, CIFX_PLUGIN_T) s_tPluginList = SLIST_HEAD_INITIALIZER(PLUGIN_LIST);

#endif

#define COS_THREAD_STACK_MIN  0x1000  /* Stack size needed by Thread for
                                         handling Toolkit's cyclic actions */

/*****************************************************************************/
/*! \file cifxlinux.c
*   Linux specific initialization of toolkit / driver                        */
/*****************************************************************************/

static void __init()   __attribute__((__constructor__));
static void __deinit() __attribute__((__destructor__));

static void* cifXPollingThread(void *arg);
static int            polling_thread_running = 0;
static pthread_t      polling_thread         = {0};
static pthread_attr_t polling_thread_attr    = {{0}};
static unsigned short polling_thread_stop    = 0;

#ifdef CIFX_DRV_HWIF
  void* HWIFDPMRead ( uint32_t ulOpt, void* pvDevInstance, void* pvDpmAddr, void* pvDst, uint32_t ulLen);
  void* HWIFDPMWrite( uint32_t ulOpt, void* pvDevInstance, void* pvDpmAddr, void* pvSrc, uint32_t ulLen);
#endif

/*****************************************************************************/
/*! Initialization function called on load of libcifx_tk.so                  */
/*****************************************************************************/
static void __init()
{
  //cifXTKitInit();
}

/*****************************************************************************/
/*! De-Initialization function called on unload of libcifx_tk.so             */
/*****************************************************************************/
static void __deinit()
{
  cifXDriverDeinit();
}

/*****************************************************************************/
/*! Helper function to open a uio device
*     \param uio_num          Number of uio device
*     \param fCheckAccess     If != 0 the driver denies access if card is already
*                             accesed
*     \return fd of uio, -1 on error                                         */
/*****************************************************************************/
int cifx_uio_open(int uio_num, int fCheckAccess)
{
  char dev_name[16];
  int fd;
  int iRet;

  sprintf(dev_name, "/dev/uio%d", uio_num);
  fd = open(dev_name,O_RDWR);

  /* if fCheckAccess is true lock access to the device */
  if (1 == fCheckAccess)
  {
    /* lock file access (non blocking) */
    if (0 != (iRet = flock( fd, LOCK_EX | LOCK_NB)))
    {
      if (errno == EWOULDBLOCK)
      {
        ERR( "cifX%d may be opened by another process!\n", uio_num);
      }
      close(fd);
      fd = -1;
    }
  }

  return fd;
}

/*****************************************************************************/
/*! Helper function to open a ISA device
*     \return fd -1 on error                                                 */
/*****************************************************************************/
int cifx_ISA_open(void)
{
  char dev_name[16];
  int fd;

  sprintf(dev_name, "/dev/mem");
  fd = open(dev_name,O_RDWR, O_SYNC);

  return fd;
}

/*****************************************************************************/
/*! Helper function to close a ISA device                                    */
/*****************************************************************************/
void cifx_ISA_close(int isa_fd)
{
  close( isa_fd);
}

/*****************************************************************************/
/*! Checks if mapping matches specific type (dpm, extmem, dma)
*     \param uio_num  number of the uio device
*     \param mapno    Mapping number to validate
*     \param tMemtype Memory type to be checked
*     \return CIFX_NO_ERROR if memtype matches                               */
/*****************************************************************************/
int32_t validate_memtype( int uio_num, int mapno, CIFX_MEM_TYPE_E tMemtype)
{
  int32_t ret = CIFX_DEV_FUNCTION_FAILED;
  FILE*   file;
  char    memtype[64];
  char    filename[64];

  sprintf(filename, "/sys/class/uio/uio%d/maps/map%d/name", uio_num, mapno);

  if ((file = fopen(filename,"r")) && (1==fscanf(file,"%s",memtype)))
  {
    switch (tMemtype)
    {
      case eMEM_DPM:
        ret = ( 0 == strncmp("dpm", memtype, strlen("dpm")))? CIFX_NO_ERROR: CIFX_DEV_FUNCTION_FAILED;
        break;
      case eMEM_EXTMEM:
        ret = ( 0 == strncmp("extmem", memtype, strlen("extmem")))? CIFX_NO_ERROR: CIFX_DEV_FUNCTION_FAILED;
        break;
      case eMEM_DMA:
        ret = ( 0 == strncmp("dma", memtype, strlen("dma")))? CIFX_NO_ERROR: CIFX_DEV_FUNCTION_FAILED;
        break;
      default:
        ret = CIFX_INVALID_PARAMETER;
        break;
    }
  } else
  {
    /* no name specified        -> old uio_netx driver */
    /* name file does not exist -> old uio driver */
    if ((NULL != file) || (errno == ENOENT))
    {
      /* memory type validation via requested mapno (to be downwards compatible) */
      switch (tMemtype)
      {
        case eMEM_DPM: /* first mapno is DPM */
          ret = (0 == mapno) ? CIFX_NO_ERROR : CIFX_DEV_FUNCTION_FAILED;
          break;
        case eMEM_EXTMEM:/* second mapno is ext mem */
          ret = (1 == mapno) ? CIFX_NO_ERROR : CIFX_DEV_FUNCTION_FAILED;
          break;
        case eMEM_DMA:/* dma was not supported */
        default:
          ret = CIFX_INVALID_PARAMETER;
          break;
      }
    }
  }
  if (NULL != file)
    fclose(file);

  return ret;
}

/*****************************************************************************/
/*! Iterates over mapping dir, searching for a specific memtype
*     \param uio_num  number of the uio device
*     \param memtype to search for
*     \param bar     BAR to mapping file
*     \return CIFX_NO_ERROR if memtype is found                              */
/*****************************************************************************/
int32_t find_memtype( int uio_num, CIFX_MEM_TYPE_E tMemtype, int *bar)
{
  char            addr_file[64];
  struct dirent** namelist;
  int32_t         ret = CIFX_NO_MORE_ENTRIES;
  int             num_map;
  int             found = 0;

  if (!bar)
    return CIFX_INVALID_PARAMETER;

  sprintf(addr_file, "/sys/class/uio/uio%d/maps/", uio_num);
  num_map = scandir(addr_file, &namelist, 0, alphasort);
  if(num_map > 0)
  {
    int currfile = 0;
    for(;currfile < num_map; ++currfile)
    {
      if ((0 == found) && (CIFX_NO_ERROR == validate_memtype( uio_num, currfile, tMemtype)))
      {
        *bar = currfile;
        found = 1;
      }
      free(namelist[currfile]);
    }
    free(namelist);
    if (found)
      ret = CIFX_NO_ERROR;
  }
  return ret;
}

/*****************************************************************************/
/*! Returns the memory size of a uio memory bar
*     \param uio_num  Number of uio device
*     \param bar      BAR to read memory size from
*     \return size of the bar in bytes                                       */
/*****************************************************************************/
unsigned long cifx_uio_get_mem_size(int uio_num, int bar)
{
  unsigned long ret = ~0;
  char  filename[64];
  FILE* file;

  sprintf(filename, "/sys/class/uio/uio%d/maps/map%d/size",
          uio_num, bar);

  file = fopen(filename,"r");
  if(file)
  {
    if (1!=fscanf(file, "0x%lx", &ret))
      ret = ~0;
    fclose(file);
  }

  return ret;
}

/*****************************************************************************/
/*! Returns the physical address uio memory bar
*     \param uio_num  Number of uio device
*     \param bar      BAR to read memory size from
*     \return physical address of bar                                        */
/*****************************************************************************/
unsigned long cifx_uio_get_mem_addr(int uio_num, int bar)
{
  unsigned long ret = ~0;
  char  filename[64];
  FILE* file;

  sprintf(filename, "/sys/class/uio/uio%d/maps/map%d/addr",
          uio_num, bar);

  file = fopen(filename,"r");
  if(file)
  {
    if (1!=fscanf(file,"0x%lx",&ret))
      ret = ~0;
    fclose(file);
  }
  return ret;
}

/*****************************************************************************/
/*! Returns the alias given via device-tree
*     \param uio_num  Number of uio device
*     \return pointer to buffer - needs to be freed after usage              */
/*****************************************************************************/
char* cifx_uio_get_device_alias(int uio_num)
{
  char  filename[64];
  char  ret[64];
  char* alias = NULL;
  FILE* file;

  sprintf(filename, "/sys/class/uio/uio%d/name",uio_num);

  file = fopen(filename,"r");
  if(file)
  {
    if (1==fscanf(file,"%s",ret)) {
      char* buf = strstr(ret, ",");
      if ((buf != NULL) && ((buf+1-ret)<strlen(ret))) {
        buf = strstr(buf+1, ",");
        if ((buf != NULL) && ((buf+1-ret)<strlen(ret))) {
          if (strcmp((buf+1),"-") != 0) {
            alias = (char*)malloc(CIFx_MAX_INFO_NAME_LENTH);
            sprintf(alias,"%s",buf+1);
          }
        }
      }
    }
    fclose(file);
  }
  return alias;
}

/*****************************************************************************/
/*! Returns the startuptype of the device given via device-tree
*     \param uio_num  Number of uio device
*     \return device type (ram,flash,auto,dont touch)                        */
/*****************************************************************************/
CIFX_TOOLKIT_DEVICETYPE_E cifx_uio_get_device_startuptype(int uio_num)
{
  char  buf[64];
  char  filename[64];
  char* next = NULL;
  CIFX_TOOLKIT_DEVICETYPE_E ret = eCIFX_DEVICE_AUTODETECT;
  FILE* file;

  sprintf(filename, "/sys/class/uio/uio%d/name",uio_num);

  file = fopen(filename,"r");
  if(file)
  {
    if (1==fscanf(file,"%s",buf)) {
      next = strstr(buf, ",");
      if ((next != NULL) && ((next+1-buf)<strlen(buf))) {
        char* end = strstr(next+1, ",");
        next+=1;
        if (end != NULL) {
          *end = 0;
        }
      }
    }
    fclose(file);
  }
  if (next != NULL) {
    if (strncmp(next, UIO_NETX_START_TYPE_AUTO, CIFx_MAX_INFO_NAME_LENTH) == 0) {
      ret = eCIFX_DEVICE_AUTODETECT;
    } else if (strncmp(next, UIO_NETX_START_TYPE_RAM, CIFx_MAX_INFO_NAME_LENTH) == 0) {
      ret = eCIFX_DEVICE_RAM_BASED;
    } else if (strncmp(next, UIO_NETX_START_TYPE_FLASH, CIFx_MAX_INFO_NAME_LENTH) == 0) {
      ret = eCIFX_DEVICE_FLASH_BASED;
    } else if (strncmp(next, UIO_NETX_START_TYPE_DONTTOUCH, CIFx_MAX_INFO_NAME_LENTH) == 0) {
      ret = eCIFX_DEVICE_DONT_TOUCH;
    } else {
      /* default to auto-detect */
      ret = eCIFX_DEVICE_AUTODETECT;
    }
  }
  return ret;
}

/*****************************************************************************/
/*! Check if the name of the uio matches the given name
*     \param uio_num  Number of uio device
*     \param name     Expected name of device
*     \return !=0 if name matches                                            */
/*****************************************************************************/
int cifx_uio_validate_name(int uio_num, const char* name)
{
  char  filename[64];
  char  uio_name[64];
  FILE* file;
  int   ret = 0;

  sprintf(filename, "/sys/class/uio/uio%d/name", uio_num);

  if(NULL == (file = fopen(filename,"r")))
  {
    perror("Error opening file.");

  } else
  {
    if(NULL == fgets(uio_name, sizeof(uio_name), file))
    {
      perror("Error querying name from uio");
    } else
    {
      char* end = uio_name + strlen(uio_name) - 1;
      while( (*end == '\n') || (*end == '\r') )
      {
        *end-- = '\0';
      }

      if(0 == strncmp(uio_name, name, strlen(name)))
        ret = 1;
    }

    fclose(file);
  }

  return ret;
}

/*****************************************************************************/
/*! Map the memory of a uio device
*     \param fd       fd returned by uio_open
*     \param uio_num  number of uio-device
*     \param bar      number of mapping
*     \param dpmbase  Pointer to returned virtual base address of memory area
*     \param dpmaddr  Pointer to returned physical address of memory area
*     \param dpmlen   Pointer to returned length of memory area
*     \return !=0 if mapping succeeded                                       */
/*****************************************************************************/
int cifx_uio_map_mem(int uio_fd, int uio_num,
                     int map_num, void** membase,
                     unsigned long* memaddr,
                     unsigned long* memlen,
                     unsigned long flags)
{
  int ret = 0;

  if( (~0 != (*memaddr = cifx_uio_get_mem_addr(uio_num, map_num))) &&
      (~0 != (*memlen  = cifx_uio_get_mem_size(uio_num, map_num))) )
  {
    *membase = mmap(NULL, *memlen,
                    PROT_READ|PROT_WRITE,
                    MAP_SHARED|MAP_LOCKED|MAP_POPULATE|flags,
                    uio_fd, map_num * getpagesize());

    if(*membase != (void*)-1)
      ret =1;
  }
  return ret;
}

/*****************************************************************************/
/*! Map the memory (DPM) of a uio_netx device
*     \param fd   fd returned by uio_open
*     \param dpmbase  Pointer to returned virtual base address of memory area
*     \param dpmaddr  Pointer to returned physical address of memory area
*     \param dpmlen   Pointer to returned length of memory area
*     \return !=0 if mapping succeeded                                       */
/*****************************************************************************/
int cifx_uio_map_dpm(int uio_fd, int uio_num,
                     void** dpmbase,
                     unsigned long* dpmaddr,
                     unsigned long* dpmlen)
{
  int bar = 0;

  if (CIFX_NO_ERROR == find_memtype( uio_num, eMEM_DPM, &bar))
  {
    return cifx_uio_map_mem( uio_fd, uio_num, bar, dpmbase, dpmaddr, dpmlen, 0);
  }
  return 0;
}

/*****************************************************************************/
/*! Map the extended memory of a uio_netx device
*     \param fd   fd returned by uio_open
*     \param dpmbase  Pointer to returned virtual base address of memory area
*     \param dpmaddr  Pointer to returned physical address of memory area
*     \param dpmlen   Pointer to returned length of memory area
*     \return !=0 if mapping succeeded                                       */
/*****************************************************************************/
int cifx_uio_map_ext_mem(int uio_fd, int uio_num,
                     void** dpmbase,
                     unsigned long* dpmaddr,
                     unsigned long* dpmlen)
{
  int bar = 0;

  if (CIFX_NO_ERROR == find_memtype( uio_num, eMEM_EXTMEM, &bar))
  {
    return cifx_uio_map_mem( uio_fd, uio_num, bar, dpmbase, dpmaddr, dpmlen, 0);
  }
  return 0;
}


#ifdef CIFX_TOOLKIT_DMA
/*****************************************************************************/
/*! Map the DMA memory of a uio device
 * The function examines the ../uio[x]/maps/ directory searching for dynamic
 * mappable resources (dyn_map[x]) for dma support (->name=dma)
*     \param device   uio device
*     \return !=0 if mapping succeeded                                       */
/*****************************************************************************/
void cifx_uio_map_dma_buffer(struct CIFX_DEVICE_T *device)
{
  char          addr_file[64];
  struct dirent **namelist;
  int           num_map;

  sprintf(addr_file, "/sys/class/uio/uio%d/maps/", device->uio_num);
  num_map = scandir(addr_file, &namelist, 0, alphasort);
  if(num_map > 0)
  {
    int currfile = 0;
    device->dma_buffer_cnt = 0;

    for(;(device->dma_buffer_cnt < CIFX_DMA_BUFFER_COUNT) && (currfile < num_map); ++currfile)
    {
      if (CIFX_NO_ERROR == validate_memtype( device->uio_num, currfile, eMEM_DMA))
      {
        void *membase = NULL;
        unsigned long memaddr;
        unsigned long memlen;
        if (cifx_uio_map_mem(device->uio_fd, device->uio_num,
                     currfile, &membase,
                     &memaddr,
                     &memlen,
                     0))
        {
          int DMACounter = 0;
          if ((DMACounter = memlen/(CIFX_DEFAULT_DMA_BUFFER_SIZE)))
          {
            while(DMACounter)
            {
              device->dma_buffer[device->dma_buffer_cnt].ulSize            = CIFX_DEFAULT_DMA_BUFFER_SIZE;
              device->dma_buffer[device->dma_buffer_cnt].ulPhysicalAddress = memaddr;
              device->dma_buffer[device->dma_buffer_cnt].pvBuffer          = membase;
              memaddr += CIFX_DEFAULT_DMA_BUFFER_SIZE;
              membase += CIFX_DEFAULT_DMA_BUFFER_SIZE;

              DBG("DMA buffer %d found at 0x%p / size=%d\n", device->dma_buffer_cnt,
                     device->dma_buffer[device->dma_buffer_cnt].pvBuffer,
                     device->dma_buffer[device->dma_buffer_cnt].ulSize);

              device->dma_buffer_cnt++;
              DMACounter--;
            }
          }
        } else
        {
          perror("Error mapping DMA buffer!\n");
        }
      }
    }
    while(num_map-->0) {
      free(namelist[num_map]);
    }
    free(namelist);

    if (device->dma_buffer_cnt == 0) {
      DBG("\nThe uio_netx driver does not provide memory for DMA support!\n");
      DBG("If DMA is required, the uio_netx driver needs to be build with DMA support!\n\n");
    }
  }
}

/*****************************************************************************/
/*! Unmap the DMA memory of a uio device
 *     \param device   uio device                                            */
/*****************************************************************************/
void cifx_uio_unmap_dma_buffer(struct CIFX_DEVICE_T *device) {
  char          addr_file[64];
  struct dirent **namelist;
  int           num_map;

  sprintf(addr_file, "/sys/class/uio/uio%d/maps/", device->uio_num);
  num_map = scandir(addr_file, &namelist, 0, alphasort);
  if(num_map > 0) {
    int currfile = 0;
    for(;(device->dma_buffer_cnt > 0) && (currfile < num_map); ++currfile) {
      if (CIFX_NO_ERROR == validate_memtype( device->uio_num, currfile, eMEM_DMA)) {
        uint32_t no_of_buffers = cifx_uio_get_mem_size(device->uio_num, currfile) / (CIFX_DEFAULT_DMA_BUFFER_SIZE);

        if ((no_of_buffers > 0) && (device->dma_buffer[CIFX_DMA_BUFFER_COUNT - device->dma_buffer_cnt].pvBuffer != NULL)) {
          munmap( device->dma_buffer[CIFX_DMA_BUFFER_COUNT - device->dma_buffer_cnt].pvBuffer, cifx_uio_get_mem_size(device->uio_num, currfile));
        }

        if (no_of_buffers >= device->dma_buffer_cnt)
          device->dma_buffer_cnt = 0;
        else
          device->dma_buffer_cnt -= no_of_buffers;
      }
    }
    while(num_map-->0) {
      free(namelist[num_map]);
    }
    free(namelist);
  }
}
#endif

/*****************************************************************************/
/*! Map the memory (DPM) of a ISA device
*     \param uio_fd   fd returned by cifX_ISA_open -> open /dev/mem
*     \param dpmbase  Pointer to returned virtual base address of memory area
*     \param dpmaddr  physical address of memory area
*     \param dpmlen   length of memory area
*     \return !=0 if mapping succeeded                                       */
/*****************************************************************************/
int cifx_ISA_map_dpm(int fd,
                     void** dpmbase,
                     int dpmaddr,
                     int dpmlen)
{
  int ret = 0;

  *dpmbase = mmap(NULL, dpmlen,
                    PROT_READ|PROT_WRITE,
                    MAP_SHARED|MAP_LOCKED|MAP_POPULATE,
                    fd, dpmaddr);

  if(*dpmbase != (void*)-1)
      ret =1;

  return ret;
}

/*****************************************************************************/
/*! Unmap the memory (DPM) of a ISA device
*     \param dpmbase  Pointer to returned virtual base address of memory area
*     \param dpmlen   length of memory area                                  */
/*****************************************************************************/
void cifx_ISA_unmap_dpm( void* dpmaddr, int dpmlen)
{
  munmap( dpmaddr, dpmlen);
}

#ifndef CIFX_TOOLKIT_DISABLEPCI
  /*****************************************************************************/
  /*! Try to find a matching PCI card by verifying the physical BAR addresses
  *     \param dev_instance   Device to search for. PCI data will be inserted into
                              O/S dependent part
  *     \return !=0 if a matching PCI card was found                           */
  /*****************************************************************************/
  static int match_pci_card(PDEVICEINSTANCE dev_instance, unsigned long ulPys_Addr)
  {
    struct pci_device_iterator *pci_dev_it;
    struct pci_device          *dev;
    int                         ret        = 0;

    static const struct pci_id_match id_match =
    {
      .vendor_id    = PCI_MATCH_ANY,
      .device_id    = PCI_MATCH_ANY,
      .subvendor_id = PCI_MATCH_ANY,
      .subdevice_id = PCI_MATCH_ANY,
    };

    pci_dev_it = pci_id_match_iterator_create(&id_match);

    while( NULL != (dev = pci_device_next(pci_dev_it)) )
    {
      int bar;

      pci_device_probe(dev);

      for(bar = 0; bar < sizeof(dev->regions) / sizeof(dev->regions[0]); ++bar)
      {
        if(dev->regions[bar].base_addr == (pciaddr_t)ulPys_Addr)
        {
          PCIFX_DEVICE_INTERNAL_T internal = (PCIFX_DEVICE_INTERNAL_T)dev_instance->pvOSDependent;

          DBG("matched pci card @ bus=%d,dev=%d,func=%d,vendor=0x%x,device=0x%x,subvendor=0x%x,subdevice=0x%x \n",
                dev->bus, dev->dev, dev->func,
                dev->vendor_id, dev->device_id, dev->subvendor_id, dev->subdevice_id);

          /* detect flash based card by device- and sub_device id  */
          if ( (dev->vendor_id == HILSCHER_PCI_VENDOR_ID) &&
               (
                 ((dev->device_id == NETPLC100C_PCI_DEVICE_ID) && (dev->subdevice_id == NETPLC100C_PCI_SUBYSTEM_ID_FLASH)) ||
                 ((dev->device_id == NETJACK100_PCI_DEVICE_ID) && (dev->subdevice_id == NETJACK100_PCI_SUBYSTEM_ID_FLASH)) ||
                 (dev->device_id == CIFX4000_PCI_DEVICE_ID)
               )
             )
          {
            dev_instance->eDeviceType = eCIFX_DEVICE_FLASH_BASED;
          }

          internal->pci = *dev;
          ret  = 1;
          break;
        }
      }
    }

    pci_iterator_destroy(pci_dev_it);

    return ret;
  }
#endif /* CIFX_TOOLKIT_DISABLEPCI */

/*****************************************************************************/
/*! Thread for cyclic Toolkit timer, which handles polling of COS bits on
*   non-irq cards
*     \param arg      Pollinterval in ms
*     \return NULL on termination                                            */
/*****************************************************************************/
static void* cifXPollingThread(void *arg)
{
  unsigned long pollinterval = (unsigned long)arg;
  struct timespec polling_sleep;

  polling_sleep.tv_sec = 0;
  polling_sleep.tv_nsec = pollinterval * 1000 * 1000;

  while( 0 == polling_thread_stop )
  {
    cifXTKitCyclicTimer();
    nanosleep(&polling_sleep, NULL);
  }
  return NULL;
}


/*****************************************************************************/
/*! Wraps the cifX Toolkit callback to the user's known parameters
*   The user does not need to know about our device instance, as he only
*   provided a CIFX_DEVICE_T structure
*     \param pvDeviceInstance  Device the callback was made for
*     \param eEvent            Signalled event                               */
/*****************************************************************************/
static void cifXWrapEvent(void* pvDeviceInstance, CIFX_TOOLKIT_NOTIFY_E eEvent)
{
  PDEVICEINSTANCE         ptDevInstance = (PDEVICEINSTANCE)pvDeviceInstance;
  PCIFX_DEVICE_INTERNAL_T ptInternal    = (PCIFX_DEVICE_INTERNAL_T)ptDevInstance->pvOSDependent;
  struct CIFX_DEVICE_T*   ptDevice      = ptInternal->userdevice;

  ptDevice->notify(ptDevice, eEvent);
}

/*****************************************************************************/
/*! Internal function for adding device to toolkit control
*     \param ptDevice  Device to add
*     \param num       Number to use for identifier ("cifX<num>")
*     \param user_card !=0 if card was given through user parameter
*     \return CIFX_NO_ERROR on success                                       */
/*****************************************************************************/
static int32_t cifXDriverAddDevice(struct CIFX_DEVICE_T* ptDevice, unsigned int num, int user_card)
{
  PDEVICEINSTANCE         ptDevInstance = NULL;
  PCIFX_DEVICE_INTERNAL_T ptInternalDev = NULL;
  int32_t                 ret           = CIFX_FUNCTION_FAILED;

#ifdef CIFX_DRV_HWIF
  if ( ((ptDevice->hwif_read) && (NULL == ptDevice->hwif_write)) ||
       ((ptDevice->hwif_write) && (NULL == ptDevice->hwif_read))) {
    ERR( "Error initializing device! Misconfigured HW-Function Interface (read- or write-function is not defined)!");
    return CIFX_INVALID_PARAMETER;
  }
#endif
  if(NULL == (ptInternalDev = (PCIFX_DEVICE_INTERNAL_T)malloc(sizeof(*ptInternalDev))))
  {
    ERR( "Error allocating internal device structures");
  } else if(NULL == (ptDevInstance = (PDEVICEINSTANCE)malloc(sizeof(*ptDevInstance))))
  {
    ERR( "Error allocating internal device structures");
    free(ptInternalDev);
    ptInternalDev = NULL;
  } else
  {
    memset(ptDevInstance, 0, sizeof(*ptDevInstance));
    memset(ptInternalDev, 0, sizeof(*ptInternalDev));

    ptInternalDev->userdevice  = ptDevice;
    ptInternalDev->devinstance = ptDevInstance;
    ptInternalDev->user_card   = user_card;

    ptDevInstance->pvOSDependent     = (void*)ptInternalDev;
    ptDevInstance->pbDPM             = (unsigned char*)ptDevice->dpm;
    ptDevInstance->ulDPMSize         = ptDevice->dpmlen;
    ptDevInstance->ulPhysicalAddress = ptDevice->dpmaddr;

    ptDevInstance->pbExtendedMemory      = (uint8_t*)ptDevice->extmem;
    ptDevInstance->ulExtendedMemorySize  = ptDevice->extmemlen;

#ifdef CIFX_DRV_HWIF
    /* set to the linux default read/write function, to be able to handle all devices (also memory mapped) */
    ptDevInstance->pfnHwIfRead  = HWIFDPMRead;
    ptDevInstance->pfnHwIfWrite = HWIFDPMWrite;
#endif

#ifdef CIFX_TOOLKIT_DMA
    if (ptDevice->dma_buffer_cnt)
    {
      int i = 0;
      ptDevInstance->ulDMABufferCount = ptDevice->dma_buffer_cnt;
      for (i=0;i<ptDevice->dma_buffer_cnt;i++)
      {
        ptDevInstance->atDmaBuffers[i].ulSize            = ptDevice->dma_buffer[i].ulSize;
        ptDevInstance->atDmaBuffers[i].ulPhysicalAddress = ptDevice->dma_buffer[i].ulPhysicalAddress;
        ptDevInstance->atDmaBuffers[i].pvBuffer          = ptDevice->dma_buffer[i].pvBuffer;
        ptDevInstance->atDmaBuffers[i].pvUser            = NULL;
      }
    }
#endif
    if(ptDevice->notify)
    {
      ptDevInstance->pfnNotify = cifXWrapEvent;
    }

    snprintf(ptDevInstance->szName,
             sizeof(ptDevInstance->szName),
             "cifX%u",
             num);

    if (ptDevice->uio_num>=0) {
      /* extract alias from uio device name (can be used via device-tree) */
      char* szAlias = cifx_uio_get_device_alias(ptDevice->uio_num);
      if (szAlias != NULL) {
        snprintf(ptDevInstance->szAlias,
                 sizeof(ptDevInstance->szName),
                 "%s",
                 szAlias);
        free(szAlias);
      }
    }

    /* Default to no logfile */
    ptInternalDev->log_file = NULL;

    /* Create log file if neccessary */
    if(g_ulTraceLevel > 0)
    {
      size_t pathlen     = strlen(g_szDriverBaseDir) + sizeof(ptDevInstance->szName) + 2 + 4; /* +2 for 1x NUL and 1 additional '/'
                                                                                                 +4 for extension ".log" */
      char*  logfilepath = malloc(pathlen);

      snprintf(logfilepath, pathlen, "%s/%s.log", g_szDriverBaseDir, ptDevInstance->szName);

      if (g_logfd == 0) {
        ptInternalDev->log_file = fopen(logfilepath, "w+");
      } else {
        ptInternalDev->log_file = g_logfd;
      }
      if( NULL == ptInternalDev->log_file)
      {
        perror("Error opening logfile. Traces will be printed to console!");
      } else
      {
        DRIVER_INFORMATION tDriverInfo;
        CIFXHANDLE         hDrv;

        /* Insert header into log file */
        USER_Trace(ptDevInstance, 0, "----- cifX Driver Log started ---------------------");
        if (xDriverOpen(&hDrv)) {
          USER_Trace(ptDevInstance, 0, "      %s / Error retrieving Toolkit version", LINUXCIFXDRV_VERSION);
        } else {
          xDriverGetInformation( hDrv, sizeof(tDriverInfo), &tDriverInfo);
          xDriverClose(hDrv);
          USER_Trace(ptDevInstance, 0, "      %s / Toolkit %s", LINUXCIFXDRV_VERSION, tDriverInfo.abDriverVersion);
        }
        USER_Trace(ptDevInstance, 0, " Name : %s", ptDevInstance->szName);
        USER_Trace(ptDevInstance, 0, " DPM  : 0x%lx, len=%lu", ptDevInstance->ulPhysicalAddress, ptDevInstance->ulDPMSize);
        USER_Trace(ptDevInstance, 0, "---------------------------------------------------");
      }

      free(logfilepath);
    }

    if(ptDevice->force_ram)
    {
      ptDevInstance->eDeviceType = eCIFX_DEVICE_RAM_BASED;
    } else
    {
      if (ptDevice->uio_num>=0) {
        /* in case of uio device device-type can be given (device-tree) */
        ptDevInstance->eDeviceType = cifx_uio_get_device_startuptype(ptDevice->uio_num);
      } else {
        ptDevInstance->eDeviceType = eCIFX_DEVICE_AUTODETECT;
      }
    }

    if(ptDevice->pci_card != 0)
    {
#ifndef CIFX_TOOLKIT_DISABLEPCI
      /* Try to find the card on the PCI bus. If it is not found, deny to work with this card */
      if(!match_pci_card(ptDevInstance, ptDevice->dpmaddr))
      {
        /* Don't add this device */
        if (g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                     TRACE_LEVEL_ERROR,
                     "Error finding pci device (Phys. Addr 0x%lx) on PCI bus",
                     ptDevice->dpmaddr);
        }
      } else
      {
        ptDevInstance->fPCICard = 1;
        ret = CIFX_NO_ERROR;
      }
#else
      if (g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                   TRACE_LEVEL_ERROR,
                   "cifX Driver was compiled without PCI support. Unable to handle requested PCI card @0x%lx!",
                   ptDevice->dpmaddr);
      }
#endif
    } else
    {
      ptDevInstance->fPCICard = 0;
      ret = CIFX_NO_ERROR;
    }
#ifdef CIFX_DRV_HWIF
    /* initialize the hardware function interface */
    if (ptDevice->hwif_init) {
      if (CIFX_NO_ERROR != (ret = ptDevice->hwif_init( ptDevice))) {
        if (g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          char szError[1024] ={0};
          USER_Trace(ptDevInstance,
                     TRACE_LEVEL_ERROR,
                     "Failed to initialize custom hardware interface. 'hwif_init' returns 0x%lx - %s! Skip adding custom device to toolkit!",
                     (unsigned int)ret,
                     ((CIFX_NO_ERROR == xDriverGetErrorDescription( ret,  szError, sizeof(szError))) ? szError : "Unknown error"));
        }
      }
    }
#endif
    /* Add the device to the toolkits handled device list */
    if(CIFX_NO_ERROR == ret) {
      if ((ret = cifXTKitAddDevice(ptDevInstance))) {
        if (g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          char szError[1024] ={0};
          xDriverGetErrorDescription( ret,  szError, sizeof(szError));
          USER_Trace(ptDevInstance, 0, "Error: 0x%X, <%s>\n", (unsigned int)ret, szError);
        }
#ifdef CIFX_DRV_HWIF
        /* de-initialize the hardware function interface */
        if (ptDevice->hwif_deinit)
          ptDevice->hwif_deinit( ptDevice);
#endif
      }
    }
  }

  if(CIFX_NO_ERROR != ret)
  {
    free(ptDevInstance);
    free(ptInternalDev);
#ifdef CIFXETHERNET
  } else
  {
    CIFX_DEVICE_INFORMATION tDevInfo;

    OS_Memset(&tDevInfo, 0, sizeof(tDevInfo));

    /* Initalize file information structure */
    tDevInfo.ulDeviceNumber   = ptDevInstance->ulDeviceNumber;
    tDevInfo.ulSerialNumber   = ptDevInstance->ulSerialNumber;
    tDevInfo.ulChannel        = CIFX_SYSTEM_DEVICE;
    tDevInfo.ptDeviceInstance = ptDevInstance;

    if (0 != USER_GetEthernet( &tDevInfo))
    {
      NETX_ETH_DEV_CFG_T config;

      sprintf( config.cifx_name, "%s", ptDevInstance->szName);
      if (NULL != cifxeth_create_device( &config))
      {
        ptInternalDev->eth_support = 1;
        if (g_ulTraceLevel & TRACE_LEVEL_INFO)
        {
          USER_Trace(ptDevInstance, 0, "Successfully created ethernet interface on %s", ptDevInstance->szName);
        }
      }
    }
#endif
  }

  return ret;
}

/*****************************************************************************/
/*! Linux driver initialization function
*     \param init_params  Initialization parameters
*     \return CIFX_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t cifXDriverInit(const struct CIFX_LINUX_INIT* init_params)
{
  int32_t lRet = cifXTKitInit();
  unsigned int num = 0;
  unsigned int temp;

#ifdef CIFXETHERNET
  /* in case of unordinary shutdown of the application the devices may still */
  /* exist (since we create the net devices persistent) so we clean up here  */
  cifxeth_sys_cleanup();

  g_eth_list_lock = OS_CreateLock();
#endif
  if (init_params == NULL)
    return CIFX_INVALID_PARAMETER;

  g_ulTraceLevel = init_params->trace_level;

  if(CIFX_NO_ERROR == lRet)
  {
    unsigned long poll_interval = init_params->poll_interval;
    size_t        tStackSize    = init_params->poll_StackSize;

    polling_thread_running = 0;

    if(poll_interval != CIFX_POLLINTERVAL_DISABLETHREAD)
    {
      struct sched_param sched_param = {0};
      sched_param.sched_priority = init_params->poll_priority;
      int ret = 0;

      /* Default to 500ms poll interval */
      if(poll_interval == 0)
        poll_interval = 500;

      if ( tStackSize == 0)
        tStackSize = COS_THREAD_STACK_MIN;

      /* Create COS flag polling thread for non-irq devices */
      polling_thread_stop = 0;

      if( (ret = pthread_attr_init(&polling_thread_attr)) != 0 )
      {
        ERR( "cifXDriverInit: Failed to initialize attributes for polling thread (pthread_attr_init=%d)", ret);
        lRet = CIFX_DRV_INIT_STATE_ERROR;

      } else if (init_params->poll_priority && ( (ret = pthread_attr_setinheritsched( &polling_thread_attr, PTHREAD_EXPLICIT_SCHED)) != 0))
      {
        ERR( "cifXDriverInit: Failed to set the polling thread attributes (pthread_attr_setinheritsched=%d)", ret);
        lRet = CIFX_DRV_INIT_STATE_ERROR;

      } else if (init_params->poll_priority && ( (ret = pthread_attr_setschedpolicy(&polling_thread_attr, init_params->poll_schedpolicy)) != 0))
      {
        ERR( "cifXDriverInit: Failed to set scheduler policy of polling thread (pthread_attr_setschedpolicy=%d)", ret);
        lRet = CIFX_DRV_INIT_STATE_ERROR;

      /* Setup Stack size to minimum */
      } else if( (ret = pthread_attr_setstacksize(&polling_thread_attr, PTHREAD_STACK_MIN + tStackSize)) != 0)
      {
        ERR( "cifXDriverInit: Failed to set stack size of polling thread (pthread_attr_setstacksize=%d)", ret);
        lRet = CIFX_DRV_INIT_STATE_ERROR;

      /* Set polling thread priority */
      } else if(init_params->poll_priority && ((ret = pthread_attr_setschedparam(&polling_thread_attr, &sched_param) != 0)))
      {
        ERR( "cifXDriverInit: Failed to set priority of polling thread (pthread_attr_setschedparam=%d)", ret);
        lRet = CIFX_DRV_INIT_STATE_ERROR;
      } else if( (ret = pthread_create(&polling_thread, &polling_thread_attr, cifXPollingThread, (void*)poll_interval)) != 0 )
      {
        ERR( "cifXDriverInit: Could not create polling thread (pthread_create=%d)", ret);
        lRet = CIFX_DRV_INIT_STATE_ERROR;
      }
      polling_thread_running = 1;
    }

    if(CIFX_NO_ERROR == lRet)
    {
      if (init_params->logfd != 0) {
        g_logfd = init_params->logfd;
      }
      /* Set driver base directory */
      if(NULL == init_params->base_dir)
      {
          g_szDriverBaseDir = strdup("/opt/cifx");
      } else
      {
        g_szDriverBaseDir = strdup(init_params->base_dir);
      }

      if(CIFX_DRIVER_INIT_CARDNUMBER == init_params->init_options)
      {
        struct CIFX_DEVICE_T* ptDevice;
        int                   iDevice = init_params->iCardNumber;

        if(NULL == (ptDevice = cifXFindDevice( iDevice, init_params->fEnableCardLocking)))
        {
          ERR( "Error opening device with number %u\n", iDevice);
          lRet = CIFX_INVALID_BOARD;
        } else
        {
          lRet = cifXDriverAddDevice(ptDevice, 0, 0);
          if(CIFX_NO_ERROR != lRet)
          {
            ERR( "Error adding automatically found cifX device @ Phys. Addr 0x%lX. (Status=0x%08X)\n", ptDevice->dpmaddr, lRet);
          }
        }
        /* Automatically scan for uio devices */
      } else if(CIFX_DRIVER_INIT_AUTOSCAN == init_params->init_options)
      {
        int                   iDevice;
        struct CIFX_DEVICE_T* ptDevice;
        int                   iCardCount = cifXGetDeviceCount();

        for(iDevice = 0; iDevice < iCardCount; ++iDevice)
        {
          if(NULL == (ptDevice = cifXFindDevice( iDevice, init_params->fEnableCardLocking)))
          {
            ERR( "Error opening device with number %u\n", iDevice);
          } else
          {
            uint32_t lTemp = cifXDriverAddDevice(ptDevice, num, 0);
            if(CIFX_NO_ERROR != lTemp)
            {
              ERR( "Error adding automatically found cifX device @ Phys. Addr 0x%lX. (Status=0x%08X)\n", ptDevice->dpmaddr, lTemp);
            } else
            {
              num++;
            }
          }
        }
#ifdef CIFX_PLUGIN_SUPPORT
        char szPath[CIFX_MAX_FILE_NAME_LENGTH+5];
        DIR* dir;

        snprintf(szPath, CIFX_MAX_FILE_NAME_LENGTH,
                "%s/plugins/",
                g_szDriverBaseDir);

        /* Iterate over plugins in ${basedir}/plugins folder */
        if(NULL != (dir = opendir(szPath)))
        {
          struct dirent* dirent;

          while(NULL != (dirent = readdir(dir)))
          {
            char* szExt = strstr(dirent->d_name, ".");
            if(NULL != szExt)
            {
              if(0 == strncasecmp(szExt, ".so", 3))
              {
                snprintf(szPath, sizeof(szPath),
                        "%s/plugins/%s",
                        g_szDriverBaseDir, dirent->d_name);

                void* hFile = dlopen(szPath, RTLD_NOW | RTLD_LOCAL);
                if(NULL == hFile)
                {
                  ERR( "Error loading plugin library %s with error=%s)\n", dirent->d_name, dlerror());
                } else
                {
                  PFN_CIFX_PLUGIN_GET_DEVICE_COUNT pfnCount;
                  PFN_CIFX_PLUGIN_ALLOC_DEVICE     pfnAlloc;

                  if( (NULL == (pfnCount = dlsym(hFile, CIFX_PLUGIN_GET_DEVICE_COUNT))) ||
                      (NULL == (pfnAlloc = dlsym(hFile, CIFX_PLUGIN_ALLOC_DEVICE))) ||
                      (NULL == dlsym(hFile, CIFX_PLUGIN_FREE_DEVICE)) )
                  {
                    ERR( "Error loading plugin library %s, as it does not contain required exports\n", dirent->d_name);
                    dlclose(hFile);
                  } else
                  {
                    struct CIFX_PLUGIN_T* plugin = calloc(1, sizeof(*plugin));
                    uint32_t i;

                    plugin->hPluginFile = hFile;
                    plugin->ulDeviceCount = pfnCount();
                    plugin->aptDevices = calloc(plugin->ulDeviceCount, sizeof(*plugin->aptDevices));

                    SLIST_INSERT_HEAD(&s_tPluginList, plugin, tList);

                    for(i = 0; i < plugin->ulDeviceCount; i++)
                    {
                      uint32_t lTemp;

                      plugin->aptDevices[i] = pfnAlloc(i);

                      if(NULL == plugin->aptDevices[i])
                      {
                        ERR( "Error: Plugin (%s) return no device for idx=%i\n",
                          dirent->d_name, i);
                      } else
                      {
                        lTemp = cifXDriverAddDevice(plugin->aptDevices[i], num, 1);
                        if(CIFX_NO_ERROR != lTemp)
                        {
                          ERR( "Error adding plugin (%s) device %u@0x%lX. (Status=0x%08X)\n",
                            dirent->d_name, i, plugin->aptDevices[i]->dpmaddr, lTemp);
                        } else
                        {
                          num++;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          closedir(dir);
        }
#endif
      }
      if (CIFX_NO_ERROR == lRet)
      {
        /* Add all user specified cards */
        for(temp = 0; temp < init_params->user_card_cnt; ++temp)
        {
          if(cifXDriverAddDevice(&init_params->user_cards[temp], num, 1) != CIFX_NO_ERROR )
          {
            ERR( "Adding user device #%d failed \n", temp);
          } else
          {
            ++num;
          }
        }
      }
    }
  }

  return lRet;
}


/*****************************************************************************/
/*! cifX driver restart function
*     \param hDriver     Handle to the driver
*     \param szBoardName Identifier for the Board
*     \param pvData      For further extensions can be NULL
*     \return CIFX_NO_ERROR on success                                       */
/*****************************************************************************/
 int32_t xDriverRestartDevice ( CIFXHANDLE  hDriver, char* szBoardName, void* pvData)
 {
   int32_t    lRet  = CIFX_INVALID_BOARD;
   uint32_t   ulIdx = 0;

   UNREFERENCED_PARAMETER(pvData);

   OS_EnterLock(g_pvTkitLock);

   /* Seach the device with the given name */
   for (ulIdx = 0; ulIdx < g_ulDeviceCount; ulIdx++)
   {
     /* Compare the device name */
     PDEVICEINSTANCE ptDev = g_pptDevices[ulIdx];

     if( (OS_Strcmp( ptDev->szName,  szBoardName) == 0) ||
         (OS_Strcmp( ptDev->szAlias, szBoardName) == 0) )
     {
#if CIFXETHERNET || CIFX_DRV_HWIF
       PCIFX_DEVICE_INTERNAL_T dev_intern = (PCIFX_DEVICE_INTERNAL_T)ptDev->pvOSDependent;
#endif

       if (g_ulTraceLevel & TRACE_LEVEL_DEBUG)
       {
         USER_Trace( ptDev,
                     TRACE_LEVEL_DEBUG,
                    "RESTART DEVICE requested for device: %s",
                     szBoardName);
       }
       /* Remove a device */
#ifdef CIFXETHERNET
       if (1 == dev_intern->eth_support)
       {
         NETX_ETH_DEV_CFG_T config;

         sprintf( config.cifx_name, "%s", ptDev->szName);
         cifxeth_remove_device( NULL, &config);
       }
#endif
       if ( CIFX_NO_ERROR == (lRet = cifXTKitRemoveDevice(ptDev->szName, 1)))
       {
#ifdef CIFX_DRV_HWIF
         /* de-initialize hardware interface */
         if (dev_intern->userdevice->hwif_deinit)
           dev_intern->userdevice->hwif_deinit( dev_intern->userdevice);

         /* re-initialize hardware interface */
         if (dev_intern->userdevice->hwif_init) {
           lRet = dev_intern->userdevice->hwif_init( dev_intern->userdevice);
           if (CIFX_NO_ERROR != lRet) {
             if (g_ulTraceLevel & TRACE_LEVEL_ERROR)
             {
               char szError[1024] ={0};
               USER_Trace(ptDev,
                          TRACE_LEVEL_ERROR,
                          "Failed to initialize custom hardware interface. 'hwif_init' returns 0x%lx - %s! Skip adding custom device to toolkit!",
                          (unsigned int)lRet,
                          ((CIFX_NO_ERROR == xDriverGetErrorDescription( lRet,  szError, sizeof(szError))) ? szError : "Unknown error"));
             }
           }
         }
#endif
         /* Re-insert a device */
         if ((lRet == CIFX_NO_ERROR) && (CIFX_NO_ERROR == (lRet = cifXTKitAddDevice( ptDev))))
         {
#ifdef CIFXETHERNET
           if (1 == dev_intern->eth_support)
           {
             NETX_ETH_DEV_CFG_T config;

             sprintf( config.cifx_name, "%s", ptDev->szName);
             cifxeth_create_device( &config);
           }
#endif
         } else
         {
#ifdef CIFX_DRV_HWIF
           /* de-initialize hardware interface */
           if (dev_intern->userdevice->hwif_deinit)
             dev_intern->userdevice->hwif_deinit( dev_intern->userdevice);
#endif
          }
       }
       if (g_ulTraceLevel & TRACE_LEVEL_DEBUG)
       {
         USER_Trace(ptDev,
                   TRACE_LEVEL_DEBUG,
                   "RESTART DEVICE done, (Status: 0x%08X)\n",
                   lRet);
       }
       break;
    }
  }

  OS_LeaveLock(g_pvTkitLock);

  return lRet;
}

/*****************************************************************************/
/*! Linux driver de-initialization                                           */
/*****************************************************************************/
void cifXDriverDeinit()
{
  /* Stop polling thread if it was enabled */
  if(polling_thread_running)
  {
    polling_thread_stop    = 1;
    pthread_join(polling_thread, NULL);
    pthread_attr_destroy(&polling_thread_attr);
    polling_thread_running = 0;
  }

  if (NULL == g_pvTkitLock) /* toolkit is already de-initialized */
    return;

  OS_EnterLock(g_pvTkitLock);

  /* Remove all internal device structures */
  while(g_ulDeviceCount > 0)
  {
    PDEVICEINSTANCE         devinstance = g_pptDevices[0];
    PCIFX_DEVICE_INTERNAL_T dev_intern  = (PCIFX_DEVICE_INTERNAL_T)devinstance->pvOSDependent;
#ifdef CIFXETHERNET
    if (1 == dev_intern->eth_support)
    {
      NETX_ETH_DEV_CFG_T config;

      sprintf( config.cifx_name, "%s", devinstance->szName);
      cifxeth_remove_device( NULL, &config);
    }
#endif
    cifXTKitRemoveDevice(devinstance->szName , 1);

    if( NULL != dev_intern->log_file)
    {
      USER_Trace(devinstance, 0, "----- cifX Driver Log stopped ---------------------");

      if (g_logfd == 0) {
        /* log file is under our control so close it */
        fclose(dev_intern->log_file);
      }
      dev_intern->log_file = NULL;
    }
#ifdef CIFX_DRV_HWIF
    /* de-initialize hardware interface */
    if (dev_intern->userdevice->hwif_deinit)
      dev_intern->userdevice->hwif_deinit( dev_intern->userdevice);
#endif
    /* Only process non-user card device structures */
    if(!dev_intern->user_card)
    {
      cifXDeleteDevice(dev_intern->userdevice);
      dev_intern->userdevice = NULL;
    }

    free(dev_intern);
    free(devinstance);
  }

  OS_LeaveLock(g_pvTkitLock);

#ifdef CIFX_PLUGIN_SUPPORT
  struct CIFX_PLUGIN_T* plugin;

  while(NULL != (plugin = SLIST_FIRST(&s_tPluginList)))
  {
    uint32_t i;

    for(i = 0; i < plugin->ulDeviceCount; i++)
    {
      PFN_CIFX_PLUGIN_FREE_DEVICE pfnFree = dlsym(plugin->hPluginFile, CIFX_PLUGIN_FREE_DEVICE);

      if( (NULL != pfnFree) &&
          (NULL != plugin->aptDevices[i]) )
      {
        pfnFree(plugin->aptDevices[i]);
      }
    }

    free(plugin->aptDevices);
    dlclose(plugin->hPluginFile);
    SLIST_REMOVE(&s_tPluginList, plugin, CIFX_PLUGIN_T, tList);
    free(plugin);
  }
#endif

#ifdef CIFXETHERNET
  if (NULL != g_eth_list_lock) {
    OS_DeleteLock( g_eth_list_lock);
    g_eth_list_lock = NULL;
  }
#endif

  if (NULL != g_szDriverBaseDir) {
    free(g_szDriverBaseDir);
    g_szDriverBaseDir = NULL;
  }
  cifXTKitDeinit();
}


/*****************************************************************************/
/*! Query Driver Version
*   \param ulSize     Size of the passed string
*   \param szVersion  Pointer to returned data (string)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t cifXGetDriverVersion(uint32_t ulSize, char* szVersion)
{
  if(ulSize < OS_Strlen(LINUXCIFXDRV_VERSION))
    return CIFX_INVALID_BUFFERSIZE;

  OS_Strncpy(szVersion, LINUXCIFXDRV_VERSION, OS_Strlen(LINUXCIFXDRV_VERSION));

  return CIFX_NO_ERROR;
}


/*****************************************************************************/
/*! Retrieve the number of automatically detectable cifX Devices.
*     \return Number found netX/cifX uio devices                             */
/*****************************************************************************/
int cifXGetDeviceCount(void)
{
  struct dirent**       namelist;
  int                   num_uios;
  int                   ret = 0;

  num_uios = scandir("/sys/class/uio", &namelist, 0, alphasort);
  if(num_uios > 0)
  {
    int currentuio;

    for(currentuio = 0; currentuio < num_uios; ++currentuio)
    {
      int uio_num;

      if(0 == sscanf(namelist[currentuio]->d_name,
                     "uio%u",
                     &uio_num))
      {
        /* Error extracting uio number */

      } else if( cifx_uio_validate_name(uio_num, CIFX_UIO_CARD_NAME)        ||
                 cifx_uio_validate_name(uio_num, CIFX_UIO_PLX_CARD_NAME)    ||
                 cifx_uio_validate_name(uio_num, CIFX_UIO_NETPLC_CARD_NAME) ||
                 cifx_uio_validate_name(uio_num, CIFX_UIO_NETJACK_CARD_NAME)||
                 cifx_uio_validate_name(uio_num, CIFX_UIO_CUSTOM_CARD_NAME) )
      {
        /* device is not a netX device */
        ++ret;
      }
      free(namelist[currentuio]);
    }
    free(namelist);
  }

  return ret;
}

/*****************************************************************************/
/*! Scan for cifX Devices via uio driver
*     \param iNum             Number of card to detect (0..num_of_cards)
*     \param fCheckAccess     If !=0, function denies access if device is
*                             already used by another application
*     \return NULL if no device with this number was found                   */
/*****************************************************************************/
struct CIFX_DEVICE_T* cifXFindDevice(int iNum, int fCheckAccess)
{
  struct dirent**       namelist;
  int                   num_uios;
  struct CIFX_DEVICE_T* device = NULL;

  num_uios = scandir("/sys/class/uio", &namelist, 0, alphasort);
  if(num_uios > 0)
  {
    int netx_uios = 0;
    int currentuio;
    int founddevice = 0;

    for(currentuio = 0; currentuio < num_uios; ++currentuio)
    {
      int uio_num;

      if(founddevice)
      {
        /* we already found the device, so skip it.
           we need to handle all data from name list, so we need to
           cycle through whole list */
      } else if(0 == sscanf(namelist[currentuio]->d_name,
                           "uio%u",
                           &uio_num))
      {
        /* Error extracting uio number */

      } else if( !cifx_uio_validate_name(uio_num, CIFX_UIO_CARD_NAME)         &&
                 !cifx_uio_validate_name(uio_num, CIFX_UIO_PLX_CARD_NAME)     &&
                 !cifx_uio_validate_name(uio_num, CIFX_UIO_NETPLC_CARD_NAME)  &&
                 !cifx_uio_validate_name(uio_num, CIFX_UIO_NETJACK_CARD_NAME) &&
                 !cifx_uio_validate_name(uio_num, CIFX_UIO_CUSTOM_CARD_NAME) )
      {
        /* device is not a netX device */

      } else if(netx_uios++ != iNum)
      {
        /* not the device we are looking for, so skip it */

      } else
      {
        void*         dpmbase    = NULL;
        void*         extmembase = NULL;
        unsigned long dpmaddr, dpmlen, extmemaddr, extmemlen;;
        int           uio_fd;

        if(-1 == (uio_fd = cifx_uio_open( uio_num, fCheckAccess)))
        {
          perror("Error opening uio");
        } else if(!cifx_uio_map_dpm(uio_fd, uio_num, &dpmbase, &dpmaddr, &dpmlen))
        {
          perror("Error mapping dpm");
        } else
        {
          /* try to map extended memory */
          if (cifx_uio_map_ext_mem(uio_fd, uio_num, &extmembase, &extmemaddr, &extmemlen))
          {
            DBG("Extended memory found at (0x%X - 0x%X)\n", (unsigned int)extmemaddr, (unsigned int)(extmemaddr + extmemlen));
          }

          /* Build device structure */
          device = malloc(sizeof(*device));
          memset(device, 0, sizeof(*device));
          device->uio_num = uio_num;
          device->uio_fd  = uio_fd;

          if( cifx_uio_validate_name(uio_num, CIFX_UIO_PLX_CARD_NAME) )
            device->pci_card = 0;
          else if ( cifx_uio_validate_name(uio_num, CIFX_UIO_CUSTOM_CARD_NAME) )
            device->pci_card = 0;
          else
            device->pci_card = 1;

          device->dpm     = dpmbase;
          device->dpmaddr = dpmaddr;
          device->dpmlen  = dpmlen;
          /* optional extended memory */
          device->extmem     = extmembase;
          device->extmemaddr = extmemaddr;
          device->extmemlen  = extmemlen;
#ifdef CIFX_TOOLKIT_DMA
          cifx_uio_map_dma_buffer( device);
#endif
        }
        founddevice = 1;
      }
      free(namelist[currentuio]);
    }
    free(namelist);
  }

  return device;
}

/*****************************************************************************/
/*! Delete a previously via cifXFindDevice found device. Unmaps DPM and closes
*   all open file handles to the uio driver. Also frees the pointer.
*     \param device   Device to delete                                       */
/*****************************************************************************/
void cifXDeleteDevice(struct CIFX_DEVICE_T* device)
{
  if(device->uio_fd != -1)
  {
#ifdef CIFX_TOOLKIT_DMA
    if (device->dma_buffer_cnt)
      cifx_uio_unmap_dma_buffer(device);
#endif
    /* Unmap DPM */
    munmap(device->dpm, device->dpmlen);
    if (device->extmem)
      munmap(device->extmem, device->extmemlen);
    /* close uio_fd */
    flock( device->uio_fd, LOCK_UN);
    close(device->uio_fd);
  }
  free(device);
}

#ifdef CIFX_DRV_HWIF
/*****************************************************************************/
/*! Read a number of bytes from hardware interface
 * If no hw-functions are defined DPM is accessed via simple memcpy
*   \param ulDpmAddr Address offset in DPM to read data from
*   \param pvDst     Buffer to store read data
*   \param pvDst     Number of bytes to read                                 */
/*****************************************************************************/
void* HWIFDPMRead( uint32_t ulOpt, void* pvDevInstance, void* pvDpmAddr, void* pvDst, uint32_t ulLen)
{
  PDEVICEINSTANCE         pDev          = (PDEVICEINSTANCE)pvDevInstance;
  PCIFX_DEVICE_INTERNAL_T ptInternalDev = (PCIFX_DEVICE_INTERNAL_T)pDev->pvOSDependent;
  struct CIFX_DEVICE_T*   ptDevice      = ptInternalDev->userdevice;
  (void) ulOpt;

  if (ptDevice->hwif_read)/* call the custom defined hw-read function */
    pvDst = ptDevice->hwif_read( ptDevice, pvDpmAddr, pvDst, ulLen);
  else /* if function is not defined, its a memory mapped DPM */
    OS_Memcpy( pvDst, pvDpmAddr, ulLen);

  return pvDst;
}

/*****************************************************************************/
/*! Write a number of bytes to hardware interface
 * If no hw-functions are defined DPM is accessed via simple memcpy
*   \param ulDpmAddr Address offset in DPM to read data from
*   \param pvDst     Buffer to store read data
*   \param pvDst     Number of bytes to read                                 */
/*****************************************************************************/
void* HWIFDPMWrite( uint32_t ulOpt, void* pvDevInstance, void* pvDpmAddr, void* pvSrc, uint32_t ulLen)
{
  PDEVICEINSTANCE         pDev          = (PDEVICEINSTANCE)pvDevInstance;
  PCIFX_DEVICE_INTERNAL_T ptInternalDev = (PCIFX_DEVICE_INTERNAL_T)pDev->pvOSDependent;
  struct CIFX_DEVICE_T*   ptDevice      = ptInternalDev->userdevice;
  (void) ulOpt;

  if (ptDevice->hwif_write)/* call the custom defined hw-write function */
    pvDpmAddr = ptDevice->hwif_write( ptDevice, pvDpmAddr, pvSrc, ulLen);
  else /* if function is not defined, its a memory mapped DPM */
    OS_Memcpy( pvDpmAddr, pvSrc, ulLen);

  return pvDpmAddr;
}
#endif
