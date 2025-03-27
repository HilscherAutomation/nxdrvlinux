// SPDX-License-Identifier: MIT
/**************************************************************************************
 *
 * Copyright (c) 2025, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 * Description: libcifx driver/toolkit initialization
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

#ifdef VFIO_SUPPORT
  #include <sys/ioctl.h>
  #include <linux/vfio.h>
#ifdef VFIO_CDEV
  #include <linux/iommufd.h>
#endif
#endif

extern char*            g_szDriverBaseDir;
extern void*            g_pvTkitLock;
extern unsigned long    g_ulDeviceCount;
extern PDEVICEINSTANCE* g_pptDevices;
#ifdef CIFXETHERNET
extern void*            g_eth_list_lock;
#endif
FILE*                   g_logfd = 0;
static int              s_netx_device_counter = 0;

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

static char* s_cifx_device_type_str[] = { "uio",
                                          "spi",
#ifdef VFIO_SUPPORT
                                          "vfio",
#endif
                                          "unknown"};

#define COS_THREAD_STACK_MIN  0x1000  /* Stack size needed by Thread for
                                         handling Toolkit's cyclic actions */

#define SYSFS_PCI_DEV_PATH  "/sys/bus/pci/devices/"

#define IS_HILSCHER_PCI_DEV(x,id) (((sysfs_get_pci_id( x, "vendor", id) == 0) && (*id == HILSCHER_PCI_VENDOR_ID)) ? 1 : 0)

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

#ifdef CIFX_TOOLKIT_DMA
  static void cifx_unmap_dma_buffer(struct CIFX_DEVICE_T *device);
#endif
static int sysfs_get_pci_id( char* dev_path, char* file, unsigned int* id);
static int pci_get_hilscher_device_id_by_idx( int index, char* device_id, uint32_t name_len, int full_path);

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

int check_if_locked( int fd) {
  int ret = -EPERM;

  /* lock file access (non blocking) */
  if (flock( fd, LOCK_EX | LOCK_NB) < 0) {
    ret = -errno;
    if (errno == EWOULDBLOCK) {
        DBG( "cifX device may be opened by another process!\n");
    }
  } else {
    ret = 0;
  }
  return ret;
}

#include <libgen.h> /* basename */
int get_link_base_name( char* path, char* link_path, int buf_len, char** result) {
  int ret = -EINVAL;

  if ( (path == NULL) || (link_path == NULL) )
    return ret;

  if ( ((ret = readlink( path, link_path, buf_len)) > 0) && (ret < buf_len) ) {
    /* it's not terminated... */
    link_path[ret] = '\0';
    /* get directory name (link to iommu group) */
    *result = basename( link_path);
    ret = 0;
  } else {
    ret = -errno;
  }
  return ret;
}

#ifdef VFIO_SUPPORT
#ifdef VFIO_CDEV
static struct vfio_device_bind_iommufd vfio_bind = {
    .argsz = sizeof(vfio_bind),
    .flags = 0,
};
#endif
struct vfio_group_status group_status =
                                { .argsz = sizeof(group_status) };
struct vfio_iommu_type1_info iommu_info = { .argsz = sizeof(iommu_info) };
struct vfio_device_info device_info = { .argsz = sizeof(device_info) };

/*****************************************************************************/
/*! Helper function to open a vfio device
*     \param device_path      path to pci device (sysfs)
*     \param vfio_num         device number
*     \param fCheckAccess     If != 0 the driver denies access if card is already
*                             accesed
*     \return 0 on success, < 0 on error                                     */
/*****************************************************************************/
#ifdef VFIO_CDEV
static int cifx_vfio_open_cdev( char* device_path, int vfio_num, int fCheckAccess, void* pdev)
{
  char dev_name[CIFX_MAX_FILE_NAME_LENGTH];
  struct vfio_fd* pfd;
  struct CIFX_DEVICE_T* device = (struct CIFX_DEVICE_T*)pdev;
  int ret;

  if ( (device == NULL) || (strlen(device_path) == 0) )
    return -EINVAL;

  if ( ((pfd = calloc( 1, sizeof(struct vfio_fd))) == NULL ) ||
       ((pfd->device_path = calloc( 1, strlen(device_path))) == NULL) )  {
    ERR( "Error allocating memory for device '%s'!\n", device_path);
    ret = -ENOMEM;
    goto alloc_err;
  }

  strncpy( pfd->device_path, device_path, strlen(device_path));

  pfd->vfio_num = vfio_num;
  snprintf( dev_name, CIFX_MAX_FILE_NAME_LENGTH, "/dev/vfio/devices/vfio%d", pfd->vfio_num);
  if ((pfd->vfio_fd = open(dev_name,O_RDWR)) < 0) {
    ret = -errno;
    ERR( "Error opening vfio device %s! (err=%d)\n", dev_name, ret);
    goto alloc_err;
  }
  if ((fCheckAccess != 0) && (ret = check_if_locked( pfd->vfio_fd)) != 0)
    goto block_err;

  if ((pfd->iommu_fd = open("/dev/iommu", O_RDWR)) >= 0) {
    vfio_bind.iommufd = pfd->iommu_fd;
    /* unbind is automatically done when closing */
    if (ioctl( pfd->vfio_fd, VFIO_DEVICE_BIND_IOMMUFD, &vfio_bind) < 0) {
      ret = -errno;
      ERR( "Error binding \"%s\" to IOMMU cdev interface! (err=%d)\n", dev_name, ret);
    } else {
      device->userparam = (void*)pfd;
      device->uio_num = UIO_NUM_VFIO_DEVICE;
      return 0;
    }
    close(pfd->iommu_fd);
  } else {
    ret = -errno;
    ERR( "Error opening iommu for %s (err=%d)\n", dev_name, ret);
  }

block_err:
  close(pfd->vfio_fd);
alloc_err:
  if (pfd != NULL) {
    if (pfd->device_path != NULL)
      free(pfd->device_path);
    free(pfd);
  }
  return ret;
}
#endif /* VFIO_CDEV */
static int cifx_vfio_open(char* device_path, int vfio_num, int fCheckAccess, struct CIFX_DEVICE_T* device)
{
  struct vfio_fd* pfd = NULL;
  char* device_id = NULL;
  char link_path[CIFX_MAX_FILE_NAME_LENGTH];
  char group_path[CIFX_MAX_FILE_NAME_LENGTH];
  char* group;
  int ret;

  /* for non-cdev devices vfio_num does not exis */
  (void)vfio_num;

  if (device == NULL)
    return -EINVAL;

  /* TODO: the kernel controls it or no chance per device? */
  if ( (fCheckAccess) || (strlen(device_path) == 0) )
    return -EINVAL;

  if ( ((pfd = calloc( 1, sizeof(struct vfio_fd))) == NULL) ||
       ((pfd->device_path = calloc( 1, strlen(device_path))) == NULL) )  {
    ERR( "Error allocating memory - for device '%s'!\n", device_path);
    ret = -ENOMEM;
    goto alloc_err;
  }

  pfd->container = -1;
  pfd->group = -1;
  pfd->vfio_fd = -1;

  strncpy( pfd->device_path, device_path, strlen(device_path));
  snprintf( group_path, CIFX_MAX_FILE_NAME_LENGTH, "%s/iommu_group", pfd->device_path);
  if ((ret = get_link_base_name( group_path, link_path, CIFX_MAX_FILE_NAME_LENGTH, &group)) != 0)
    goto open_err;

  snprintf( group_path, CIFX_MAX_FILE_NAME_LENGTH, "/dev/vfio/%s", group);
  /* get device pci id */
  device_id = basename(pfd->device_path);

  /* Create a new container */
  if ((pfd->container = open("/dev/vfio/vfio", O_RDWR)) < 0) {
    ret = -errno;
    ERR( "Error opening container (ret=%d)!\n", ret);
    goto open_err;
  }

  if (ioctl(pfd->container, VFIO_GET_API_VERSION) != VFIO_API_VERSION) {
    ret = -EINVAL;
    ERR( "Error - invalid VFIO API version!\n");
    goto open_err;
  }

  if (ioctl(pfd->container, VFIO_CHECK_EXTENSION, VFIO_TYPE1_IOMMU) < 0) {
    ret = -EINVAL;
    ERR( "Error - expecting \"VFIO_TYPE1_IOMMU\" support!\n");
    goto open_err;
  }
  /* get group within container */
  if ((pfd->group = open( group_path, O_RDWR)) < 0) {
    ret = -errno;
    ERR( "Error - opening IOMMU group (ret=%d)!\n", ret);
    goto open_err;
  } else {
    /* Test the group is viable and available */
    if (ioctl(pfd->group, VFIO_GROUP_GET_STATUS, &group_status) < 0) {
      ret = -errno;
      ERR( "Error - VFIO_GROUP_GET_STATUS (ret=%d)!\n", ret);
      goto open_err;
    } else {
      if (!(group_status.flags & VFIO_GROUP_FLAGS_VIABLE)) {
        ret = -EAGAIN;
        ERR( "Not all devices are under VFIO control (ret=%d)!\n", ret);
        goto open_err;
      } else {
        /* Add the group to the container */
        if (ioctl(pfd->group, VFIO_GROUP_SET_CONTAINER, &pfd->container) < 0) {
          ret = -errno;
          ERR( "Error - VFIO_GROUP_SET_CONTAINER (ret=%d)!\n", ret);
          goto open_err;
        } else {
          /* Enable the IOMMU model we want */
          if (ioctl(pfd->container, VFIO_SET_IOMMU, VFIO_TYPE1_IOMMU) < 0) {
            ret = -errno;
            ERR( "Error - VFIO_SET_IOMMU (ret=%d)!\n", ret);
            goto open_err;
          } else {
            /* Get a file descriptor for the device */
            if ((pfd->vfio_fd = ioctl(pfd->group, VFIO_GROUP_GET_DEVICE_FD, device_id)) <0) {
              ret = -errno;
              ERR( "Error - VFIO_GROUP_GET_DEVICE_FD (ret=%d)!\n", ret);
              goto open_err;
            } else {
              if ((fCheckAccess != 0) && (ret = check_if_locked( pfd->vfio_fd)) != 0)
                goto open_err;

              device->userparam = (void*)pfd;
              device->uio_num = UIO_NUM_VFIO_DEVICE;
              return 0;
            }
          }
        }
      }
    }
  }
open_err:
  if (pfd->vfio_fd >= 0)
    close(pfd->vfio_fd);
  if (pfd->group >= 0)
    close(pfd->group);
  if (pfd->container >= 0)
    close(pfd->container);
alloc_err:
  if (pfd != NULL) {
    if (pfd->device_path != NULL)
      free(pfd->device_path);
    free(pfd);
  }
  return ret;
}
#endif /* VFIO_SUPPORT */

/*****************************************************************************/
/*! Helper function to open a uio device
*     \param uio_num          Number of uio device
*     \param fCheckAccess     If != 0 the driver denies access if card is already
*                             accesed
*     \return fd of uio, < 0 on error                                        */
/*****************************************************************************/
int cifx_uio_open(int uio_num, int fCheckAccess)
{
  char dev_name[16];
  int fd;
  int ret;

  sprintf(dev_name, "/dev/uio%d", uio_num);

  if ((fd = open(dev_name,O_RDWR)) < 0) {
    fd = -errno;
  } else {
    if ((fCheckAccess != 0) && (ret = check_if_locked( fd)) != 0) {
      close(fd);
      return ret;
    }
  }
  return fd;
}

/*****************************************************************************/
/*! Helper function to open a cifx (uio/vfio) device
*     \param dev_num          device number of device
*     \param fCheckAccess     If != 0 the driver denies access if card is already
*                             accesed
*     \return 0 on success, < 0 on error                                     */
/*****************************************************************************/
static int cifx_open( char* path, int dev_type, int dev_num, int fCheckAccess, struct CIFX_DEVICE_T* device)
{
  if (device == NULL)
    return -EINVAL;

#ifndef VFIO_SUPPORT
  (void)path;
#endif

  if (dev_type == eCIFX_DEVICE_TYPE_UIO) {
    if ((device->uio_fd = cifx_uio_open( dev_num, fCheckAccess)) >= 0) {
      device->uio_num = dev_num;
      return 0;
    } else {
      return device->uio_fd;
    }
  }
#ifdef VFIO_SUPPORT
  if (dev_type == eCIFX_DEVICE_TYPE_VFIO) {
#ifdef VFIO_CDEV
    /* in case a vfio device number provided it's the cdev interface */
    if (dev_num >= 0)
      return cifx_vfio_open_cdev( path, dev_num, fCheckAccess, device);
#endif
    return cifx_vfio_open( path, dev_num, fCheckAccess, device);
  }
#endif
  return -EINVAL;
}

/*****************************************************************************/
/*! function closes a cifx (uio/vfio) device
*     \param device   pointer to device structure                            */
/*****************************************************************************/
static void cifx_close( struct CIFX_DEVICE_T* device) {
  if (device != NULL) {
    if (device->uio_fd >= 0) {
      /* close uio_fd */
      flock( device->uio_fd, LOCK_UN);
      close(device->uio_fd);
      device->uio_fd = -1;
    }
#ifdef VFIO_SUPPORT
    if (device->userparam != NULL) {
      if (device->uio_num == UIO_NUM_VFIO_DEVICE) {
        struct vfio_fd* pfd = (struct vfio_fd*)device->userparam;

        if (pfd != NULL) {
          if (pfd->vfio_fd >= 0)
            close(pfd->vfio_fd);
          if (pfd->device_path != NULL)
            free(pfd->device_path);
#ifdef VFIO_CDEV
          if (pfd->iommu_fd >= 0)
            close(pfd->iommu_fd);
#endif
          if (pfd->group >= 0)
            close(pfd->group);
          if (pfd->container >= 0)
            close(pfd->container);

          free(pfd);
        }
      }
    }
#endif
  }
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
      if ((buf != NULL) && ((long unsigned int)(buf+1-ret)<strlen(ret))) {
        buf = strstr(buf+1, ",");
        if ((buf != NULL) && ((long unsigned int)(buf+1-ret)<strlen(ret))) {
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
      if ((next != NULL) && ((long unsigned int)(next+1-buf)<strlen(buf))) {
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

#ifdef VFIO_SUPPORT
/*****************************************************************************/
/*! Returns parameter of pci resource file (phys addr, len, flags)
*     \param device_path path to pci device (syfs)
*     \param resource    number of the resource/region
*     \param param       number of resource (phys addr=0, len=1, flags=2)
*     \param ret_val     pointer returning the requested parameter
*     \return 0 on success or < 0 on error                                   */
/*****************************************************************************/
static int vfio_get_pci_resource( char* device_path, uint8_t resource, uint8_t param, unsigned long* ret_val) {
  int ret = -EINVAL;
  char tmp_buf[1024];
  char* pb = tmp_buf;
  FILE *fs = NULL;

  if (param > 2)
    return -EINVAL;

  snprintf( tmp_buf, 1024, "%s/resource", device_path);
  if ((fs = fopen( tmp_buf, "r")) != NULL) {
    for (int i=0; (i<=resource) && (pb != NULL); i++)
      pb = fgets(tmp_buf, sizeof(tmp_buf), fs);

    if (pb != NULL) {
      long long unsigned int val[3];
      if ((ret = sscanf (pb, "0x%llx 0x%llx 0x%llx", &val[0], &val[1], &val[2])) == 3) {
        *ret_val = val[param];
        ret = 0;
      }
    }
    fclose(fs);
  } else {
    ret = -errno;
  }
  return ret;
}

#define VFIO_REGION_OFFSET (0x10000000000)
#define VFIO_BAR_OFF(bar) (bar * VFIO_REGION_OFFSET)

/*****************************************************************************/
/*! Returns the region size of an vfio device
*     \param device   pointer to device structure
*     \param region   number of requested the region
*     \param size     pointer returning the region size on success
*     \return 0 on success or < 0 on error                                   */
/*****************************************************************************/
int cifx_vfio_get_region_size( struct CIFX_DEVICE_T* device, int region, long unsigned int* size) {
  int ret = -EINVAL;
  struct vfio_fd* pfd = GET_VFIO_PARAM_FROM_DEV(device);
  struct vfio_region_info reg = { .argsz = sizeof(reg),
                                  .index = region};

  if (pfd != NULL) {
    if (ioctl( pfd->vfio_fd, VFIO_DEVICE_GET_REGION_INFO, &reg) < 0) {
      ret = -errno;
      ERR( "Error - VFIO_DEVICE_GET_REGION_INFO (err=%d)\n", ret);
    } else {
      *size = reg.size;
      return 0;
    }
  }
  return ret;
}

/*****************************************************************************/
/*! Map the memory of a vfio device
*     \param device   pointer to device structure
*     \param bar      number of mapping
*     \param dpmbase  Pointer to returned virtual base address of memory area
*     \param dpmaddr  Pointer to returned physical address of memory area
*     \param dpmlen   Pointer to returned length of memory area
*     \return 0 if mapping succeeded or < 0 in case of an error              */
/*****************************************************************************/
static int cifx_vfio_map_mem( struct CIFX_DEVICE_T* device,
                     int bar,
                     void** dpmbase,
                     unsigned long* dpmaddr,
                     unsigned long* dpmlen,
                     unsigned long flags)
{
  int ret = -EINVAL;
  struct vfio_fd* pfd = GET_VFIO_PARAM_FROM_DEV(device);

  if (pfd != NULL) {
    if ((ret = vfio_get_pci_resource( pfd->device_path, bar, 0, dpmaddr)) != 0) {
      ERR( "Failed retrieving physical address vfio_get_pci_resource() (err=%d)\n", ret);
      return ret;
    }
    if ((ret = cifx_vfio_get_region_size( device, bar, dpmlen)) != 0) {
      ERR( "cifx_vfio_get_region_size() (err=%d)\n", ret);
      return ret;
    }
    *dpmbase = mmap(0, *dpmlen, PROT_READ | PROT_WRITE,
                                  MAP_SHARED | MAP_LOCKED | MAP_POPULATE | flags,
                                  pfd->vfio_fd, VFIO_BAR_OFF(bar));

    if (*dpmbase != (void*)-1) {
      return 0;
    } else {
      ret = -errno;
      //ERR( "Error - mmap() (err=%d) fd=%d / addr=0x%lX / len=0x%lX / off=0x%lX!\n",
      //          ret, pfd->vfio_fd, *dpmaddr, *dpmlen, VFIO_BAR_OFF(bar));
    }
  }
  return ret;
}
#endif /* VFIO_SUPPORT */

/*****************************************************************************/
/*! Map the memory of a uio device
*     \param fd       fd returned by uio_open
*     \param bar      number of mapping
*     \param dpmbase  Pointer to returned virtual base address of memory area
*     \param dpmaddr  Pointer to returned physical address of memory area
*     \param dpmlen   Pointer to returned length of memory area
*     \return 0 if mapping succeeded or < 0 in case of an error              */
/*****************************************************************************/
static int cifx_uio_map_mem(int uio_fd, int uio_num,
                     int map_num, void** membase,
                     unsigned long* memaddr,
                     unsigned long* memlen,
                     unsigned long flags)
{
  int ret = -EINVAL;

  if( (~0UL != (*memaddr = cifx_uio_get_mem_addr(uio_num, map_num))) &&
      (~0UL != (*memlen  = cifx_uio_get_mem_size(uio_num, map_num))) )
  {
    *membase = mmap(NULL, *memlen,
                    PROT_READ|PROT_WRITE,
                    MAP_SHARED|MAP_LOCKED|MAP_POPULATE|flags,
                    uio_fd, map_num * getpagesize());

    if(*membase != (void*)-1) {
      ret = 0;
    } else {
      ret = -errno;
    }
  }
  return ret;
}

/*****************************************************************************/
/*! Map the memory of a cifx device
*     \param device   pointer to device structure
*     \param bar      number of mapping
*     \param dpmbase  Pointer to returned virtual base address of memory area
*     \param dpmaddr  Pointer to returned physical address of memory area
*     \param dpmlen   Pointer to returned length of memory area
*     \return 0 if mapping succeeded or < 0 in case of an error              */
/*****************************************************************************/
static int cifx_map_mem( struct CIFX_DEVICE_T* device,
                     int bar_num,
                     void** membase,
                     unsigned long* memaddr,
                     unsigned long* memlen,
                     unsigned long flags) {
  if ( (device == NULL) || (membase == NULL) ||
       (memaddr == NULL) || (memlen == NULL))
    return -EINVAL;

  if (device->uio_num >= 0) {
    int map_no = 0;
    /* find correct mapping number */
    if (CIFX_NO_ERROR == find_memtype( device->uio_num, bar_num, &map_no)) {
      return cifx_uio_map_mem( device->uio_fd,
                  device->uio_num,
                  map_no,
                  membase,
                  memaddr,
                  memlen,
                  flags);
    }
#ifdef VFIO_SUPPORT
  } else if (device->uio_num == UIO_NUM_VFIO_DEVICE) {
    return cifx_vfio_map_mem( device,
                  bar_num,
                  membase,
                  memaddr,
                  memlen,
                  flags);
#endif
  }
  return -EINVAL;
}

/*****************************************************************************/
/*! Unmaps memory
*     \param addr     pointer to memory mapped area
*     \param len      len of the memory mapped area                          */
/*****************************************************************************/
static void cifx_unmap_mem( void* addr, size_t len) {
  if (addr != NULL)
    munmap( addr, len);
}

/*****************************************************************************/
/*! Unmaps all memory mapped for this device
*     \param device   pointer to device structure                            */
/*****************************************************************************/
static void cifx_unmap( struct CIFX_DEVICE_T* device) {
  if (device != NULL) {
#ifdef CIFX_TOOLKIT_DMA
    if (device->dma_buffer_cnt)
      cifx_unmap_dma_buffer(device);
#endif
    cifx_unmap_mem( device->dpm, device->dpmlen);
    device->dpm = NULL;
    if (device->extmem != NULL) {
      cifx_unmap_mem( device->extmem, device->extmemlen);
      device->extmem = NULL;
    }
  }
}

#ifdef CIFX_TOOLKIT_DMA
#ifdef VFIO_SUPPORT

struct vfio_iommu_type1_dma_map dma_map = { .argsz = sizeof(dma_map) };

#ifdef VFIO_CDEV
static struct iommu_ioas_alloc alloc_data  = {
        .size = sizeof(alloc_data),
        .flags = 0,
};
static struct vfio_device_attach_iommufd_pt attach_data = {
        .argsz = sizeof(attach_data),
        .flags = 0,
};
static struct iommu_ioas_map map = {
        .size = sizeof(map),
        .flags = IOMMU_IOAS_MAP_READABLE |
                 IOMMU_IOAS_MAP_WRITEABLE |
                 IOMMU_IOAS_MAP_FIXED_IOVA,
        .__reserved = 0,
};
#endif

/*****************************************************************************/
/*! Map the DMA memory of a vfio device
*     \param device   cifx device                                            */
/*****************************************************************************/
static void cifx_vfio_map_dma_buffer( struct CIFX_DEVICE_T* device)
{
  void *membase = NULL;
  unsigned long memaddr = 0;
  unsigned long memlen = (CIFX_DMA_BUFFER_COUNT*CIFX_DEFAULT_DMA_BUFFER_SIZE);
  struct vfio_fd* pfd = GET_VFIO_PARAM_FROM_DEV(device);
  int DMACounter = 0;

  if (pfd == NULL) {
    ERR( "Invalid parameter passed!\n");
    goto err_ioctl;
  }

  /* Allocate some space and setup a DMA mapping */
  membase = mmap(0, memlen, PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS | MAP_LOCKED|MAP_POPULATE, 0, 0);

#ifdef VFIO_CDEV
  if (pfd->vfio_num >= 0) {
    if (ioctl( pfd->iommu_fd, IOMMU_IOAS_ALLOC, &alloc_data) < 0) {
      ERR( "Error IOMMU_IOAS_ALLOC (ret=%d)\n", errno);
      goto err_ioctl;
    } else {
      attach_data.pt_id = alloc_data.out_ioas_id;

      if (ioctl( pfd->vfio_fd, VFIO_DEVICE_ATTACH_IOMMUFD_PT, &attach_data) < 0) {
        ERR( "Error VFIO_DEVICE_ATTACH_IOMMUFD_PT (ret=%d)\n", errno);
        goto err_ioctl;
      } else {
        map.user_va = (uintptr_t)membase;
        map.iova = memaddr; /*  starting at 0x0 from device view */
        map.length = memlen;
        map.ioas_id = alloc_data.out_ioas_id;

        if (ioctl( pfd->iommu_fd, IOMMU_IOAS_MAP, &map) < 0) {
          ERR( "IOMMU_IOAS_MAP (ret=%d)\n", errno);
          goto err_ioctl;
        }
      }
    }
  } else
#endif
  {
    dma_map.vaddr = (uintptr_t)membase;
    dma_map.size = memlen;
    dma_map.iova = 0; /* starting at 0x0 from device view */
    dma_map.flags = VFIO_DMA_MAP_FLAG_READ | VFIO_DMA_MAP_FLAG_WRITE;
    if (ioctl(pfd->container, VFIO_IOMMU_MAP_DMA, &dma_map) < 0) {
      ERR( "VFIO_IOMMU_MAP_DMA (ret=%d)\n", errno);
      goto err_ioctl;
    }
  }

  if ((DMACounter = memlen/(CIFX_DEFAULT_DMA_BUFFER_SIZE))) {
    while(DMACounter) {
      device->dma_buffer[device->dma_buffer_cnt].ulSize            = CIFX_DEFAULT_DMA_BUFFER_SIZE;
      device->dma_buffer[device->dma_buffer_cnt].ulPhysicalAddress = memaddr;
      device->dma_buffer[device->dma_buffer_cnt].pvBuffer          = membase;
      memaddr += CIFX_DEFAULT_DMA_BUFFER_SIZE;
      membase = (uint8_t*)membase + CIFX_DEFAULT_DMA_BUFFER_SIZE;

      DBG( "DMA buffer %d found at 0x%p / size=%d\n", device->dma_buffer_cnt,
         device->dma_buffer[device->dma_buffer_cnt].pvBuffer,
         device->dma_buffer[device->dma_buffer_cnt].ulSize);

      device->dma_buffer_cnt++;
      DMACounter--;
    }
  }
  return;

err_ioctl:
  munmap( membase, memlen);
}
#endif /* VFIO_SUPPORT */

/*****************************************************************************/
/*! Map the DMA memory of a uio device
 * The function examines the ../uio[x]/maps/ directory searching for dynamic
 * mappable resources (dyn_map[x]) for dma support (->name=dma)
*     \param device   cifx device                                             */
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
                     0) == 0)
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
              membase = (uint8_t*)membase + CIFX_DEFAULT_DMA_BUFFER_SIZE;

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
      DBG("If DMA is required, the uio_netx driver needs to be build with DMA support!\n");
    }
  }
}

/*****************************************************************************/
/*! Map the DMA memory of a uio/vfio device
 *     \param device pointer to a cifx device                                */
/*****************************************************************************/
static void cifx_map_dma_buffer(struct CIFX_DEVICE_T* device) {
  if (device == NULL)
    return;

  if (device->uio_num >= 0) {
    cifx_uio_map_dma_buffer( device);
#ifdef VFIO_SUPPORT
  } else if (device->uio_num == UIO_NUM_VFIO_DEVICE) {
    cifx_vfio_map_dma_buffer( device);
#endif
  }
}

/*****************************************************************************/
/*! Unmap the DMA memory of a uio device
 *     \param device pointer to a cifx device                                */
/*****************************************************************************/
static void cifx_uio_unmap_dma_buffer(struct CIFX_DEVICE_T *device) {
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

/*****************************************************************************/
/*! Unmap the DMA memory of a cifx  device
 *     \param device pointer to a cifx device                                */
/*****************************************************************************/
static void cifx_unmap_dma_buffer(struct CIFX_DEVICE_T *device) {
  if (device == NULL)
    return;

  if (device->uio_num >= 0) {
    cifx_uio_unmap_dma_buffer(device);
#ifdef VFIO_SUPPORT
  } else if (device->uio_num == UIO_NUM_VFIO_DEVICE) {
    cifx_unmap_mem( device->dma_buffer[0].pvBuffer, (8*CIFX_DEFAULT_DMA_BUFFER_SIZE));
#endif
  }
}
#endif //CIFX_TOOLKIT_DMA

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

#if defined(VFIO_SUPPORT) || !defined(CIFX_NO_PCIACCESS_LIB)
int cifx_hil_pci_flash_based( int device_id, int subdevice_id) {
  if ( ((device_id == NETPLC100C_PCI_DEVICE_ID) && (subdevice_id == NETPLC100C_PCI_SUBYSTEM_ID_FLASH)) ||
       ((device_id == NETJACK100_PCI_DEVICE_ID) && (subdevice_id == NETJACK100_PCI_SUBYSTEM_ID_FLASH)) ||
       (device_id == CIFX4000_PCI_DEVICE_ID) ) {
    return 1;
  }
  return 0;
}

int cifx_hil_pci_flash_based_by_path( char* pci_path) {
  unsigned int vendor_id, device_id, subdevice_id;

  if (pci_path != NULL) {
    if (IS_HILSCHER_PCI_DEV(pci_path, &vendor_id)) {
      if ( (sysfs_get_pci_id( pci_path, "device", &device_id) == 0) &&
           (sysfs_get_pci_id( pci_path, "subsystem_device", &subdevice_id) == 0) ) {
        return cifx_hil_pci_flash_based( device_id, subdevice_id);
      }
    }
  }
  return -1;
}

  /*****************************************************************************/
  /*! Try to find a matching PCI card by verifying the physical BAR addresses
   *     \param dev_instance   Device to search for. PCI data will be inserted into
                               O/S dependent part
   *     \return !=0 if a matching PCI card was found                           */
  /*****************************************************************************/
  static int match_pci_card(PDEVICEINSTANCE dev_instance, unsigned long ulPys_Addr)
  {
#ifdef VFIO_SUPPORT
    PCIFX_DEVICE_INTERNAL_T internal = (PCIFX_DEVICE_INTERNAL_T)dev_instance->pvOSDependent;

    if (IS_VFIO_DEVICE(internal)) {
      int ret = 0;
      if ((ret = cifx_hil_pci_flash_based_by_path( GET_VFIO_PARAM(internal)->device_path)) >= 0) {
        if (ret > 0)
          dev_instance->eDeviceType = eCIFX_DEVICE_FLASH_BASED;

        return 1;
      }
      return 0;
    }
#endif /* VFIO_SUPPORT */
#if !defined(CIFX_NO_PCIACCESS_LIB)
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
        size_t bar;

        pci_device_probe(dev);

        for(bar = 0; bar < sizeof(dev->regions) / sizeof(dev->regions[0]); ++bar)
        {
          PCIFX_DEVICE_INTERNAL_T internal = (PCIFX_DEVICE_INTERNAL_T)dev_instance->pvOSDependent;

          if(dev->regions[bar].base_addr == (pciaddr_t)ulPys_Addr) {

            DBG("matched pci card @ bus=%d,dev=%d,func=%d,vendor=0x%x,device=0x%x,subvendor=0x%x,subdevice=0x%x\n",
                  dev->bus, dev->dev, dev->func,
                  dev->vendor_id, dev->device_id, dev->subvendor_id, dev->subdevice_id);

            /* detect flash based card by device- and sub_device id  */
            if (dev->vendor_id == HILSCHER_PCI_VENDOR_ID) {
              if (cifx_hil_pci_flash_based( dev->device_id, dev->subdevice_id) == 1)
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
#else
    {
      if (g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace( dev_instance,
                   TRACE_LEVEL_ERROR,
                   "cifX Driver was compiled without PCI support. Unable to handle requested (uio based) PCI card @0x%lx!",
                   ulPys_Addr);
      }
    }
#endif /* CIFX_NO_PCIACCESS_LIB */
    return 0;
  }
#endif /* defined(VFIO_SUPPORT) || !defined(CIFX_NO_PCIACCESS_LIB) */

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

  ptDevice->notify(ptDevice, (CIFX_NOTIFY_E)eEvent);
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
    ERR( "Error initializing device! Misconfigured HW-Function Interface (read- or write-function is not defined)!\n");
    return CIFX_INVALID_PARAMETER;
  }
#endif
  if(NULL == (ptInternalDev = (PCIFX_DEVICE_INTERNAL_T)malloc(sizeof(*ptInternalDev))))
  {
    ERR( "Error allocating internal device structures\n");
  } else if(NULL == (ptDevInstance = (PDEVICEINSTANCE)malloc(sizeof(*ptDevInstance))))
  {
    ERR( "Error allocating internal device structures\n");
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

    /* get the device type and underlying driver info */
    /* note that the SPI device may use uio_fd as well for its irq handle */
    if ((ptDevice->uio_fd >= 0) && (ptDevice->uio_num >= 0))
      ptInternalDev->device_type = eCIFX_DEVICE_TYPE_UIO;
    else if ((ptDevice->userparam != NULL) && (ptDevice->uio_num == UIO_NUM_SPI_DEVICE))
      ptInternalDev->device_type = eCIFX_DEVICE_TYPE_SPI;
    else if ((ptDevice->userparam != NULL) && (ptDevice->uio_num == UIO_NUM_VFIO_DEVICE))
      ptInternalDev->device_type = eCIFX_DEVICE_TYPE_VFIO;
    else
      ptInternalDev->device_type = eCIFX_DEVICE_TYPE_UNKNOWN;

#ifdef CIFX_DRV_HWIF
    /* set to the linux default read/write function, to be able to handle all devices (also memory mapped) */
    ptDevInstance->pfnHwIfRead  = HWIFDPMRead;
    ptDevInstance->pfnHwIfWrite = HWIFDPMWrite;
#endif

#ifdef CIFX_TOOLKIT_DMA
    if (ptDevice->dma_buffer_cnt)
    {
      unsigned int i = 0;
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
        USER_Trace(ptDevInstance, 0, " Type : %s", s_cifx_device_type_str[ptInternalDev->device_type]);
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
#if defined(VFIO_SUPPORT) || !defined(CIFX_NO_PCIACCESS_LIB)
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
      if(ptDevInstance->ulDPMSize >= NETX_DPM_MEMORY_SIZE) {
        uint32_t ulVal = 0;
        /* Make sure to disable IRQs before passing device to Toolkit, since */
        /* the toolkit does not care about the irq right from the beginning. */
        HWIF_READN( ptDevInstance, &ulVal, ptDevInstance->pbDPM+IRQ_CFG_REG_OFFSET, sizeof(ulVal));
        ulVal &= ~HOST_TO_LE32(MSK_IRQ_EN0_INT_REQ);
        HWIF_WRITEN( ptDevInstance, ptDevInstance->pbDPM+IRQ_CFG_REG_OFFSET, (void*)&ulVal, sizeof(ulVal));
      }
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
  int temp;

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

    if(poll_interval != (unsigned long)CIFX_POLLINTERVAL_DISABLETHREAD)
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
        ERR( "Failed to initialize attributes for polling thread (pthread_attr_init=%d)\n", ret);
        lRet = CIFX_DRV_INIT_STATE_ERROR;

      } else if (init_params->poll_priority && ( (ret = pthread_attr_setinheritsched( &polling_thread_attr, PTHREAD_EXPLICIT_SCHED)) != 0))
      {
        ERR( "Failed to set the polling thread attributes (pthread_attr_setinheritsched=%d)\n", ret);
        lRet = CIFX_DRV_INIT_STATE_ERROR;

      } else if (init_params->poll_priority && ( (ret = pthread_attr_setschedpolicy(&polling_thread_attr, init_params->poll_schedpolicy)) != 0))
      {
        ERR( "Failed to set scheduler policy of polling thread (pthread_attr_setschedpolicy=%d)\n", ret);
        lRet = CIFX_DRV_INIT_STATE_ERROR;

      /* Setup Stack size to minimum */
      } else if( (ret = pthread_attr_setstacksize(&polling_thread_attr, PTHREAD_STACK_MIN + tStackSize)) != 0)
      {
        ERR( "Failed to set stack size of polling thread (pthread_attr_setstacksize=%d)\n", ret);
        lRet = CIFX_DRV_INIT_STATE_ERROR;

      /* Set polling thread priority */
      } else if(init_params->poll_priority && ((ret = pthread_attr_setschedparam(&polling_thread_attr, &sched_param) != 0)))
      {
        ERR( "Failed to set priority of polling thread (pthread_attr_setschedparam=%d)\n", ret);
        lRet = CIFX_DRV_INIT_STATE_ERROR;
      } else if( (ret = pthread_create(&polling_thread, &polling_thread_attr, cifXPollingThread, (void*)poll_interval)) != 0 )
      {
        ERR( "Could not create polling thread (pthread_create=%d)\n", ret);
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
            ERR( "Adding user device %d failed!\n", temp);
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
   UNREFERENCED_PARAMETER(hDriver);

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
  if(ulSize < strlen(LINUXCIFXDRV_VERSION))
    return CIFX_INVALID_BUFFERSIZE;

  OS_Strncpy(szVersion, LINUXCIFXDRV_VERSION, OS_Strlen(LINUXCIFXDRV_VERSION));

  return CIFX_NO_ERROR;
}

/*****************************************************************************/
/*! Returns to an int convered PCI ids (vendor,device,subdevice...)
*   \param dev_path   path of pci device (syfs)
*   \param file       which id or whatever file
*   \param id         returned id
*   \returns 0 on success < 0 on error                                       */
/*****************************************************************************/
static int sysfs_get_pci_id( char* dev_path, char* file, unsigned int* id) {
  char tmp_buf[CIFX_MAX_FILE_NAME_LENGTH];
  char* pb = dev_path;
  int ret = -EINVAL;
  FILE* f;

  if ( (dev_path == NULL) || (id == NULL) )
    return ret;

  if (file != NULL) {
    snprintf( tmp_buf, CIFX_MAX_FILE_NAME_LENGTH, "%s/%s", dev_path, file);
    pb = tmp_buf;
  }
  if ((f = fopen( pb, "r")) != NULL) {
    if ((fscanf( f, "0x%x", id)) == 1)
      ret = 0;

    fclose( f);
  }
  return ret;
}

/*****************************************************************************/
/*! Scans directory 'path' for given file that matches match an returns on success
*   it's device number
*   \param path       pci device path (sysfs)
*   \param match      scanf match e.g. "uio%u" or "vfio%u"
*   \param num_dev    the returned device id
*   \returns 0 on success < 0 on error                                       */
/*****************************************************************************/
int scan_dir_for_dev_file_idx(char* path, char* match, int* num_dev) {
  int ret = -EINVAL;
  struct dirent **namelist;
  int entries = 0;

  if ( (entries = scandir( path, &namelist, 0, alphasort)) > 0) {
    for (int i=0;i<entries;i++) {
      if ( (ret < 0) && (sscanf( namelist[i]->d_name, match, num_dev) == 1) ) {
        ret = 0;
      }
      free(namelist[i]);
    }
    free(namelist);
  }
  return ret;
}

/*****************************************************************************/
/*! Scans directory 'pci_path' (to pci device) to find a reference to the driver
*   (uio or vfio)
*   \param pci_path   pci device path (sysfs)
*   \param dev_type   the returned device type (uio or vfio)
*   \param dev_num    the device index e.g. uio0 -> 0
*   \returns 0 on success < 0 on error                                       */
/*****************************************************************************/
int pci_get_device_type_and_num(char* pci_path, CIFX_DEVICE_TYPE_E * dev_type, int* dev_num) {
  int ret = -EINVAL;
  char dev_type_path[CIFX_MAX_FILE_NAME_LENGTH];
  char link_path[CIFX_MAX_FILE_NAME_LENGTH];
  char* driver;

  snprintf( dev_type_path, CIFX_MAX_FILE_NAME_LENGTH, "%s/driver", pci_path);
  /* on success result provides the assigend driver */
  if (get_link_base_name( dev_type_path, link_path, CIFX_MAX_FILE_NAME_LENGTH, &driver) == 0) {
    /* netx=uio_netx driver */
    if (strcmp(driver, "netx") == 0) {
#ifndef CIFX_NO_PCIACCESS_LIB
      snprintf(dev_type_path, CIFX_MAX_FILE_NAME_LENGTH, "%s/uio/", pci_path);
      if ( (ret = scan_dir_for_dev_file_idx(dev_type_path, "uio%u", dev_num)) == 0) {
        DBG("Identified device '%s' as uio_netx based\n", dev_type_path);
        *dev_type = eCIFX_DEVICE_TYPE_UIO;
        return 0;
      } else {
        ERR("Error extracting uio device number of '%s'\n", dev_type_path);
      }
#else
#ifndef VFIO_SUPPORT
      (void)dev_type;
      (void)dev_num;
#endif
      ERR("Skipping found uio_netx based PCI '%s' device not as it is not supported since library is compiled with CIFX_NO_PCIACCESS_LIB!\n", pci_path);
#endif
      return ret;
    }
#ifdef VFIO_SUPPORT
    if (strcmp(driver, "vfio-pci") == 0) {
#ifdef VFIO_CDEV
      snprintf(dev_type_path, CIFX_MAX_FILE_NAME_LENGTH, "%s/vfio-dev/", pci_path);
      if ( (ret = scan_dir_for_dev_file_idx(dev_type_path, "vfio%u", dev_num)) == 0) {
        DBG("Identified device '%s' as vfio based (cdev)\n", dev_type_path);
        *dev_type = eCIFX_DEVICE_TYPE_VFIO;
        ret = 0;
        return 0;
      }
#endif
      /* assume its a non-cdev interface */
      DBG("Identified device '%s' as vfio based (legacy)\n", dev_type_path);
      *dev_type = eCIFX_DEVICE_TYPE_VFIO;
      *dev_num = -1; /* mark it as legacy interface */
      return 0;
    }
#endif
  } else {
    /* assigned driver not supported */
  }
  return ret;
}

/*****************************************************************************/
/*! Searched PCI for Hilscher device by given index and returns PCI bus id
*     \param index            number of device
*     \param device_id        returned device id
*     \param device_id_len    max length of buffer device_id
*     \param full_path        0 - name is device id / 1 - full path to device
*     \param dev_type         uio or vfio device
*     \return NULL if no device with this number was found                   */
/*****************************************************************************/
static int pci_get_hilscher_device_id_by_idx( int index, char* device_id, uint32_t device_id_len, int full_path) {
  char          pci_bus_path[] = SYSFS_PCI_DEV_PATH;
  struct dirent **namelist;
  int           pci_devs;
  int           ret = -ENODEV;
  int           vendor_devs = 0;

  pci_devs = scandir( pci_bus_path, &namelist, 0, alphasort);
  if(pci_devs > 0) {
    for (int i=0; i<pci_devs;i++) {
      char dev_path[CIFX_MAX_FILE_NAME_LENGTH];
      unsigned int id;

      snprintf( dev_path, CIFX_MAX_FILE_NAME_LENGTH,"%s/%s", pci_bus_path, namelist[i]->d_name);
      if (ret == 0) {
        //finshed to free
      } else if (!IS_HILSCHER_PCI_DEV(dev_path, &id)) {
         //skip it
      } else if (vendor_devs++ == index) {
        ret = 0;
        if (device_id != NULL) {
          if (full_path != 0) {
            snprintf( device_id, device_id_len, "%s%s", pci_bus_path, namelist[i]->d_name);
          } else {
            snprintf( device_id, device_id_len, "%s", namelist[i]->d_name);
          }
        }
      }
      free(namelist[i]);
    }
    free(namelist);
  }
  return ret;
}

/*****************************************************************************/
/*! Returns number of vfio devices
*     \return Number found uio devices                                       */
/*****************************************************************************/
static int cifx_get_pci_device_count(void) {
  int idx = 0;

  while ((pci_get_hilscher_device_id_by_idx( idx++, NULL, 0, 0)) == 0) {}
  return idx-1;
}

/*****************************************************************************/
/*! Returns number of custom uio devices (non-pci e.g. ISA or other memory mapped)
*     \return Number found uio devices                                       */
/*****************************************************************************/
static int cifx_uio_get_custom_device_count(void) {
  struct dirent**       namelist;
  int                   num_uios;
  int                   ret = 0;

  num_uios = scandir("/sys/class/uio", &namelist, 0, alphasort);
  if(num_uios > 0)
  {
    int currentuio;

    for(currentuio = 0; currentuio < num_uios; ++currentuio)
    {
      unsigned int uio_num;

      if(0 == sscanf(namelist[currentuio]->d_name,
                     "uio%u",
                     &uio_num))
      {
        /* Error extracting uio number */

      } else if (cifx_uio_validate_name( uio_num, CIFX_UIO_CUSTOM_CARD_NAME) == 1)
      {
        /* device is a netX device */
        ++ret;
      }
      free(namelist[currentuio]);
    }
    free(namelist);
  }
  return ret;
}

/*****************************************************************************/
/*! Retrieve the number of automatically detectable cifX Devices.
*     \return Number found netX/cifX uio(&vfio) devices                      */
/*****************************************************************************/
int cifXGetDeviceCount(void)
{
  int uio_count = cifx_uio_get_custom_device_count();
  int pci_count = cifx_get_pci_device_count();

  DBG("Found %d uio_netx (custom) and %d pci devices.\n", uio_count, pci_count);

  return uio_count + pci_count;
}

/*****************************************************************************/
/*! Validates if the driver can handle the device
*     \return 0 on success < 0 on error                                      */
/*****************************************************************************/
static int check_if_compatible_pci_card( char* pci_path) {
  if (pci_path != NULL) {
    unsigned int id = 0;

    if (IS_HILSCHER_PCI_DEV(pci_path, &id)) {
      /* now check if the driver can handle the device */
      if (sysfs_get_pci_id( pci_path, "subsystem_device", &id) == 0) {
         if ( (id == NETX_CHIP_PCI_DEVICE_ID) ||
              (id == NETPLC100C_PCI_DEVICE_ID) ||
              (id == NETJACK100_PCI_DEVICE_ID) ||
              (id == CIFX4000_PCI_DEVICE_ID) ) {
          return 0;
        } else {
          DBG( "Skip Hilscher device '%s' as it is not listed as a compatible device (sub device id=0x%X)\n", pci_path, id);
        }
      } else {
        ERR( "Error retrieving sub device id of '%s' - skip device\n", pci_path);
      }
    }
  }
  return -EINVAL;
}

/*****************************************************************************/
/*! Scan for cifX custom devices
*     \param iNum             Number of card to detect (0..num_of_cards)
*     \param fCheckAccess     If !=0, function denies access if device is
*                             already used by another application
*     \return NULL if no device with this number was found                   */
/*****************************************************************************/
static struct CIFX_DEVICE_T* cifx_find_custom_device( int iNum, int fCheckAccess) {
  struct dirent**       namelist;
  int                   custom_uios;
  int                   num_uios = 0;
  int                   founddevice = 0;
  struct CIFX_DEVICE_T* device = NULL;

  if ((device = malloc(sizeof(*device))) == NULL) {
    ERR( "Error allocating memory for uio_netx custom device %d\n", iNum);
    return NULL;
  }
  memset(device, 0, sizeof(*device));
  device->uio_fd = -1;

  num_uios = scandir("/sys/class/uio", &namelist, 0, alphasort);
  if(num_uios > 0)
  {
    int currentuio;
    int founddevice = 0;

    for(currentuio = 0; currentuio < num_uios; ++currentuio)
    {
      unsigned int uio_num = 0;

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

      } else if( !cifx_uio_validate_name( uio_num, CIFX_UIO_CUSTOM_CARD_NAME) )
      {
        /* device is not a custom netX device */

      } else if(custom_uios++ != iNum)
      {
        /* not the device we are looking for, so skip it */

      } else
      {
        int ret = 0;

        DBG("Found cifx %d: uio_netx custom device\n", iNum);

        if ((ret = cifx_open( NULL, eCIFX_DEVICE_TYPE_UIO, uio_num, fCheckAccess, device)) < 0) {
          ERR( "cifx_open() failed (ret=%d)\n", ret);
          goto open_err;
        } else if ((ret = cifx_map_mem( device, eMEM_DPM, (void*)&device->dpm, &device->dpmaddr, &device->dpmlen, 0)) < 0) {
          ERR( "cifx_map_mem() failed (ret=%d)\n", ret);
          goto map_err;
        } else {
          device->pci_card = 0;

          /* try to map extended memory */
          if (cifx_map_mem( device, eMEM_EXTMEM, (void*)&device->extmem, &device->extmemaddr, &device->extmemlen, 0) == 0) {
            DBG("Extended memory found (0x%X - 0x%lX)\n", (unsigned int)device->extmemaddr, device->extmemlen);
          }
#ifdef CIFX_TOOLKIT_DMA
          cifx_map_dma_buffer( device);
#endif
          founddevice = 1;
        }
      }
      free(namelist[currentuio]);
    }
    free(namelist);
  }
  if (founddevice != 0)
    return device;

map_err:
  cifx_close(device);
open_err:
  if (device != NULL)
    free(device);

  return NULL;
}

/*****************************************************************************/
/*! Scan for cifX device (uio or vfio depending on dev_type)
*     \param iNum             Number of card to detect (0..num_of_cards)
*     \param fCheckAccess     If !=0, function denies access if device is
*                             already used by another application
*     \param dev_type         uio or vfio device
*     \return NULL if no device with this number was found                   */
/*****************************************************************************/
static struct CIFX_DEVICE_T* cifx_find_pci_device(int iNum, int fCheckAccess) {
  struct dirent**       namelist;
  int                   num_devs;
  struct CIFX_DEVICE_T* device = NULL;
  int founddevice = 0;
  CIFX_DEVICE_TYPE_E dev_type;

  if ((device = malloc(sizeof(*device))) == NULL) {
    ERR( "Error allocating memory\n");
    return NULL;
  }
  memset(device, 0, sizeof(*device));
  device->uio_fd = -1;

  num_devs = scandir( SYSFS_PCI_DEV_PATH, &namelist, 0, alphasort);;
  if(num_devs > 0) {
    for(int current_dev = 0; current_dev < num_devs; ++current_dev) {
      int num_dev;
      char pci_dev_path[CIFX_MAX_FILE_NAME_LENGTH];

      snprintf( pci_dev_path, CIFX_MAX_FILE_NAME_LENGTH, "%s/%s", SYSFS_PCI_DEV_PATH, namelist[current_dev]->d_name);

      if(founddevice) {
        /* we already found the device, so skip it.
           we need to handle all data from name list, so we need to
           cycle through whole list */

      } else if( check_if_compatible_pci_card( pci_dev_path) ) {
        /* not a compatible (Hilscher) device */

      } else if ( pci_get_device_type_and_num( pci_dev_path, &dev_type, &num_dev) != 0) {
        /* device is not a Hilscher device */

      } else if(s_netx_device_counter++ != iNum) {
        /* not the device we are looking for, so skip it */

      } else {
        int ret = 0;

        DBG("Found cifx %d: PCI device ('%s')\n", iNum, pci_dev_path);

        if ((ret = cifx_open( pci_dev_path, dev_type, num_dev, fCheckAccess, device)) < 0) {
          ERR( "cifx_open() failed (ret=%d)\n", ret);
          goto open_err;
        } else if ((ret = cifx_map_mem( device, eMEM_DPM, (void*)&device->dpm, &device->dpmaddr, &device->dpmlen, 0)) < 0) {
          ERR( "cifx_map_mem() failed (ret=%d)\n", ret);
          goto map_err;
        } else {
          device->pci_card = 1;

          /* try to map extended memory */
          if (cifx_map_mem( device, eMEM_EXTMEM, (void*)&device->extmem, &device->extmemaddr, &device->extmemlen, 0) == 0) {
            DBG("Extended memory found (0x%X - 0x%lX)\n", (unsigned int)device->extmemaddr, device->extmemlen);
          }
#ifdef CIFX_TOOLKIT_DMA
          cifx_map_dma_buffer( device);
#endif
          founddevice = 1;
        }
      }
      free(namelist[current_dev]);
    }
    free(namelist);
  }
  if (founddevice != 0)
    return device;

map_err:
  cifx_close(device);
open_err:
  if (device != NULL)
    free(device);

  return NULL;
}

/*****************************************************************************/
/*! Scan for cifX Devices uio and vfio
*     \param iNum             Number of card to detect (0..num_of_cards)
*     \param fCheckAccess     If !=0, function denies access if device is
*                             already used by another application
*     \return NULL if no device with this number was found                   */
/*****************************************************************************/
struct CIFX_DEVICE_T* cifXFindDevice(int iNum, int fCheckAccess) {
  struct CIFX_DEVICE_T* device = NULL;

  s_netx_device_counter = 0;

  if (NULL == (device = cifx_find_custom_device( iNum, fCheckAccess))) {

    return cifx_find_pci_device( iNum, fCheckAccess);
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
  if(device != NULL) {
    cifx_unmap(device);
    cifx_close(device);
    free(device);
  }
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
