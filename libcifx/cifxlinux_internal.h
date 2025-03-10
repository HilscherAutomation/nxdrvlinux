/* SPDX-License-Identifier: MIT */
/**************************************************************************************
 *
 * Copyright (c) 2025, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 **************************************************************************************/

#ifndef __CIFX_LINUX_INTERNAL__H
#define __CIFX_LINUX_INTERNAL__H

#include "cifxlinux.h"

#ifndef CIFX_NO_PCIACCESS_LIB
  #include "pciaccess.h"
#endif

#include <pthread.h>
#include <stdio.h>
#include "cifXToolkit.h"

#define FORMAT_STR(type,fmt) type "%s: " fmt
#define ERR(fmt, ...)  do { \
                         if (g_ulTraceLevel & TRACE_LEVEL_ERROR) { \
                           fprintf( stderr, FORMAT_STR("ERR:",fmt), __func__, ##__VA_ARGS__); \
                         } \
                        } while (0)
#if defined(VERBOSE) || defined(DEBUG)
  #define DBG(fmt, ...)  do { \
                           if (g_ulTraceLevel & TRACE_LEVEL_DEBUG) { \
                             fprintf( stdout, FORMAT_STR("DBG:",fmt), __func__, ##__VA_ARGS__); \
                           } \
                         } while (0)
#else
  #define DBG(fmt, ...)
#endif

/* internal is pointer to CIFX_DEVICE_INTERNAL_T */
#define IS_UIO_DEVICE(internal) (internal->device_type == eCIFX_DEVICE_TYPE_UIO ? 1 : 0)
#define IS_SPI_DEVICE(internal) (internal->device_type == eCIFX_DEVICE_TYPE_SPI ? 1 : 0)
#ifdef VFIO_SUPPORT
  #define GET_VFIO_PARAM_FROM_DEV(device) ((device != NULL) ? (struct vfio_fd*)(device->userparam) : NULL)
  #define GET_VFIO_PARAM(internal)        ((internal != NULL) ? GET_VFIO_PARAM_FROM_DEV(internal->userdevice) : NULL)

  #define IS_VFIO_DEVICE(internal)    (internal->device_type == eCIFX_DEVICE_TYPE_VFIO ? 1 : 0)
  #define GET_IRQ_FD(internal)        (IS_VFIO_DEVICE(internal) ? GET_VFIO_PARAM(internal)->irq.efd : internal->userdevice->uio_fd)
  #define GET_IRQ_READ_LEN(internal)  (IS_VFIO_DEVICE(internal) ? (sizeof(uint64_t)) : (sizeof(uint32_t)))
  #define GET_IRQ_TYPE(internal)      (IS_UIO_DEVICE(internal) ? eCIFX_IRQ_TYPE_UIO : (IS_VFIO_DEVICE(internal) ? eCIFX_IRQ_TYPE_VFIO : eCIFX_IRQ_TYPE_GPIO))
#else
  #define GET_IRQ_FD(internal)        (internal->userdevice->uio_fd)
  #define GET_IRQ_READ_LEN(internal)  (sizeof(uint32_t))
  #define GET_IRQ_TYPE(internal)      (IS_UIO_DEVICE(internal) ? eCIFX_IRQ_TYPE_UIO : eCIFX_IRQ_TYPE_GPIO)
#endif

enum CIFX_IRQ_TYPE {
  eCIFX_IRQ_TYPE_UIO,
  eCIFX_IRQ_TYPE_GPIO,
#ifdef VFIO_SUPPORT
  eCIFX_IRQ_TYPE_VFIO,
#endif
};

typedef enum CIFX_DEVICE_TYPE_Etag
{
  eCIFX_DEVICE_TYPE_UIO = 0,
  eCIFX_DEVICE_TYPE_SPI,
  eCIFX_DEVICE_TYPE_VFIO,
  eCIFX_DEVICE_TYPE_UNKNOWN,
} CIFX_DEVICE_TYPE_E;

extern void* g_eth_list_lock;

typedef struct CIFX_DEVICE_INTERNAL_Ttag
{
  struct CIFX_DEVICE_T* userdevice; /*!< Device description passed by user */

  int                   set_irq_prio;           /*!< Flag if custom IRQ priority should be set */
  int                   irq_prio;               /*!< Custom IRQ thread priority  */

  int                   set_irq_scheduler_algo; /*!< Use Custom scheduling algorithm for IRQ thread */
  int                   irq_scheduler_algo;     /*!< Scheduling algorithm to use for IRQ thread (only
                                                     valid if set_irq_scheduler_algo is set) */

  pthread_attr_t        irq_thread_attr;        /*!< Interrupt thread attributes */
  pthread_t             irq_thread;             /*!< Interrupt thread handle     */
  int                   irq_stop;               /*!< flag to signal IRQ handler to stop */

  int                   user_card;        /*!< !=0 if user specified card. This card will not be deleted on exit */
  FILE                  *log_file;        /*!< Handle to logfile if any */


#ifndef CIFX_NO_PCIACCESS_LIB
  struct pci_device     pci;              /*!< pci device information if it is a pci device */
#endif

  PDEVICEINSTANCE       devinstance;      /*!< Toolkit device instance */
  int                   eth_support;

  CIFX_DEVICE_TYPE_E    device_type;

} CIFX_DEVICE_INTERNAL_T, *PCIFX_DEVICE_INTERNAL_T;


#ifdef VFIO_SUPPORT
struct vfio_irq_res {
  int efd;                             /* eventfd file descriptor required for interrupt handling */
  struct vfio_irq_set*  vfio_irq_ctrl; /* ioctl structure required to (un-)mask the interrupt     */
};

struct vfio_fd {
  int device_type;
  int vfio_num;
  int vfio_fd;
  char* device_path;
  struct vfio_irq_res irq;
  /* iommu related */
  int container;
  int group;
#ifdef VFIO_CDEV
  int iommu_fd;
#endif
};
#endif /* VFIO_SUPPORT */

#ifdef CIFXETHERNET
int USER_GetEthernet(PCIFX_DEVICE_INFORMATION ptDevInfo);
#endif

#endif /* __CIFX_LINUX_INTERNAL__H */
