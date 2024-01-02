// SPDX-License-Identifier: MIT
/**************************************************************************************
 *
 * Copyright (c) 2024, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 * Description: Linux specific abstraction of the toolkit.
 *
 * Changes:
 *
 *   Version   Date        Author   Description
 *   ----------------------------------------------------------------------------------
 *   1        02.01.24    SD        changed licensing terms
 *
 **************************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <limits.h> /* for PTHREAD_STACK_MIN */
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>

#ifndef CIFX_TOOLKIT_DISABLEPCI
  #include <pciaccess.h>
#endif

#include "cifXErrors.h"
#include "OS_Dependent.h"

#include "cifxlinux.h"
#include "cifxlinux_internal.h"
#include "cifXEndianess.h"

#define IRQ_CFG_REG_OFFSET          0xfff0
#define IRQ_ENABLE_MASK             0x80000000

#define IRQ_STACK_MIN_SIZE          0x1000 /* Stack size needed by IRQ Thread
                                              calling Toolkit's ISR/DSR Handler*/

#define BLOCK64 sizeof(uint64_t)
#define BLOCK32 sizeof(uint32_t)

/*****************************************************************************/
/*! O/S Specific initialization (initializes libpciaccess)
*     \return CIFX_NO_ERROR on success                                       */
/*****************************************************************************/
int32_t OS_Init(void)
{
  int32_t ret = CIFX_NO_ERROR;
  int err = 0;
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

#ifndef CIFX_TOOLKIT_DISABLEPCI
  if(0 != (err = pci_system_init()))
  {
    fprintf( stderr, "Error initializing PCI access subsystem (pci_system_init=%d)", err);
    ret = CIFX_FUNCTION_FAILED;
  }
#endif

  return ret;
}

/*****************************************************************************/
/*! O/S Specific de-initialization (de-initializes libpciaccess)             */
/*****************************************************************************/
void OS_Deinit(void)
{
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

#ifndef CIFX_TOOLKIT_DISABLEPCI
  pci_system_cleanup();
#endif
}

/*****************************************************************************/
/*! Memory allocation wrapper (standard malloc)
*     \param ulSize Size of block to allocate
*     \return NULL on failure                                                */
/*****************************************************************************/
void* OS_Memalloc(uint32_t ulSize) {
  void *mem_ptr;
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  mem_ptr = malloc(ulSize);

  if( mem_ptr == NULL )
    perror("Memalloc failed");

  return mem_ptr;
}

/*****************************************************************************/
/*! Memory de-allocation wrapper (standard free)
*     \param pvMem  Block to free                                            */
/*****************************************************************************/
void OS_Memfree(void* pvMem) {
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  free(pvMem);
}

/*****************************************************************************/
/*! Memory resize wrapper (standard realloc)
*     \param pvMem      Block to resize
*     \param ulNewSize  New size of the block
*     \return NULL on error                                                  */
/*****************************************************************************/
void* OS_Memrealloc(void* pvMem, uint32_t ulNewSize) {
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  pvMem = realloc(pvMem, ulNewSize);

  if( (pvMem == NULL) && (ulNewSize != 0) )
    perror("Memrealloc failed");

  return pvMem;
}

/*****************************************************************************/
/*! Memset wrapper
*     \param pvMem   Memory to set
*     \param bFill   Fill byte
*     \param ulSize  Size of the fill block                                  */
/*****************************************************************************/
void OS_Memset(void* pvMem, unsigned char bFill, uint32_t ulSize) {
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  memset(pvMem, bFill, ulSize);
}

/*****************************************************************************/
/*! Memcopy wrapper
*     \param pvDest  Destination pointer
*     \param pvSrc   Source pointer
*     \param ulSize  Size to copy                                            */
/*****************************************************************************/
void OS_Memcpy(void* pvDest, void* pvSrc, uint32_t ulSize) {
  uint32_t ulDestAlignment = (uint32_t)(unsigned long)pvDest & 0x03;
  uint32_t ulSrcAlignment  = (uint32_t)(unsigned long)pvSrc & 0x03;
#ifdef VERBOSE_1
  /* printf("%s() called\n", __FUNCTION__); */
#endif
  uint8_t *pDest8 = (uint8_t*)pvDest;
  uint8_t *pSrc8 = (uint8_t*)pvSrc;
  if ( (ulDestAlignment == 0) &&
       (ulSrcAlignment == 0) )
  {
    uint32_t *pDest32 = (uint32_t*)pvDest;
    uint32_t *pSrc32  = (uint32_t*)pvSrc;

    while(ulSize>=BLOCK64) {
      *(pDest32)++ = *(pSrc32)++;
      *(pDest32)++ = *(pSrc32)++;
      ulSize-=BLOCK64;
    }
    while(ulSize>=BLOCK32) {
      *(pDest32)++ = *(pSrc32)++;
      ulSize-=BLOCK32;
    }
    pDest8 = (uint8_t*)pDest32;
    pSrc8 = (uint8_t*)pSrc32;
  }
  while(ulSize--)
    *(pDest8++) = *(pSrc8++);
}

/*****************************************************************************/
/*! Memcompare wrapper
*     \param pvBuf1  First compare buffer
*     \param pvBuf2  Second compare buffer
*     \param ulSize  Size to compare
*     \return 0 if blocks are equal                                          */
/*****************************************************************************/
int OS_Memcmp(void* pvBuf1, void* pvBuf2, uint32_t ulSize) {
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  return memcmp(pvBuf1, pvBuf2, ulSize);
}

/*****************************************************************************/
/*! Memmove wrapper (Overlapping memory copy)
*     \param pvDest  Destination buffer
*     \param pvSrc   Source buffer
*     \param ulSize  Size to move                                            */
/*****************************************************************************/
void OS_Memmove(void* pvDest, void* pvSrc, uint32_t ulSize) {
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  pvDest = memmove(pvDest, pvSrc, ulSize);
}

/*****************************************************************************/
/*! Read PCI configuration area of specified card
*     \param pvOSDependent OS Dependent parameter to identify card
*     \return Pointer to configuration data (passed to WritePCIConfig)       */
/*****************************************************************************/
void* OS_ReadPCIConfig(void* pvOSDependent) {
#ifndef CIFX_TOOLKIT_DISABLEPCI
  PCIFX_DEVICE_INTERNAL_T info    = (PCIFX_DEVICE_INTERNAL_T)pvOSDependent;

  int                     pci_ret;
  void                    *pci_buf;
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  if(!pvOSDependent)
    return NULL;

  pci_buf = malloc(256);
  if(!pci_buf)
  {
    perror("pci_buf malloc failed");
    return NULL;
  }

  if ((pci_ret = pci_device_cfg_read(&info->pci, pci_buf, 0, 256, NULL)) )
  {
#ifdef VERBOSE
    printf("libnetx: pci_read_block() returns %d\n", pci_ret);
#endif
    free( pci_buf);
    pci_buf = NULL;
  }
  return pci_buf;
#else
  return NULL;
#endif /* CIFX_TOOLKIT_DISABLEPCI */
}

/*****************************************************************************/
/*! Restore PCI configuration
*     \param pvOSDependent OS Dependent parameter to identify card
*     \param pvPCIConfig   Pointer returned from ReadPCIConfig               */
/*****************************************************************************/
void OS_WritePCIConfig(void* pvOSDependent, void* pvPCIConfig) {

#ifndef CIFX_TOOLKIT_DISABLEPCI
  int                     pci_ret;
  PCIFX_DEVICE_INTERNAL_T info    = (PCIFX_DEVICE_INTERNAL_T)pvOSDependent;

#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

  if ((pci_ret = pci_device_cfg_write(&info->pci, pvPCIConfig, 0, 256, NULL)) )
  {
#ifdef VERBOSE
    printf("libnetx: pci_write_block() returns %d\n", pci_ret);
#endif
  }
  free(pvPCIConfig);
#endif
}

/*****************************************************************************/
/*! Checks if irq occurred for an uio device
*     \param info    Pointer to internal device structure
*     \param timeout timeout in ms to wait
*     \return 1 if irq occurred / 0 if not / < 0 in case of an error         */
/*****************************************************************************/
int check_uio_irq( PCIFX_DEVICE_INTERNAL_T info, int32_t timeout) {
  int            ret   = 0;
  int            nfds  = info->userdevice->uio_fd + 1;
  struct timeval sel_timeout;
  fd_set         readfd;

  FD_ZERO(&readfd);
  FD_SET(info->userdevice->uio_fd, &readfd);

  sel_timeout.tv_sec  = 0;
  sel_timeout.tv_usec = timeout * 1000; /* Default wait timeout = 500ms */

  if ( ((ret = select(nfds, &readfd, NULL, NULL, &sel_timeout))>0) && FD_ISSET( info->userdevice->uio_fd, &readfd)) {
    uint32_t buf;
    if ((ret = read( info->userdevice->uio_fd, &buf, sizeof(buf)))>0)
      return 1;
  }
  return ret;
}

/*****************************************************************************/
/*! Checks if irq occurred for a gpio
*     \param info    Pointer to internal device structure
*     \param timeout timeout in ms to wait
*     \return 1 if irq occurred / 0 if not / < 0 in case of an error         */
/*****************************************************************************/
int check_gpio_irq( PCIFX_DEVICE_INTERNAL_T info, uint32_t timeout) {
  int            ret  = 0;
  uint8_t        bVal = 0;
  int            nfds = info->userdevice->uio_fd + 1;
  struct timeval sel_timeout;
  fd_set         exceptfd;

  FD_ZERO(&exceptfd);
  FD_SET(info->userdevice->uio_fd, &exceptfd);

  sel_timeout.tv_sec  = 0;
  sel_timeout.tv_usec = timeout * 1000; /* Default wait timeout = 500ms */

  /* set to beginning to be able to read first, see NOTE */
  lseek( info->userdevice->uio_fd, 0, SEEK_SET);
  /* NOTE: Since netx does level sensitive irqs and linux gpio only recongnize edge */
  /*       we always need to check current level to make sure not to miss an irq.    */
  if ( !( ((ret = read( info->userdevice->uio_fd, &bVal, sizeof(bVal))) > 0) && (bVal == '1') ) ) {
    /* wait for irq */
    ret = select(nfds, NULL, NULL, &exceptfd, &sel_timeout);
  }
  return ret;
}

/*****************************************************************************/
/*! Interrupt Service Thread
*     \param ptr  Pointer to internal device structure
*     \return NULL                                                           */
/*****************************************************************************/
static void *netx_irq_thread(void *ptr) {
  PCIFX_DEVICE_INTERNAL_T info      = (PCIFX_DEVICE_INTERNAL_T)ptr;
  int                     ret       = 0;
  int                     uio_irq   = 1;
  uint32_t                timeout   = 500;

  if(!info)
    return (void *) -1;

  /* check if it's an uio device or a custom */
  if ((info->userdevice != NULL) && (info->userdevice->uio_num < 0))
    uio_irq = 0;

  while( info->irq_stop == 0 )
  {
    if (uio_irq) {
      ret = check_uio_irq( info, timeout);
    } else {
      ret = check_gpio_irq( info, timeout);
    }
    if (ret == 1) {
      uint32_t ulVal = 0;
#ifdef VERBOSE_1
      printf("IRQ @status_thread\n");
#endif
      ret = cifXTKitISRHandler(info->devinstance, 1);

      switch(ret)
      {
        case CIFX_TKIT_IRQ_DSR_REQUESTED:
          cifXTKitDSRHandler(info->devinstance);
          break;

        case CIFX_TKIT_IRQ_HANDLED:
          /* Everything was done by ISR, no need to call DSR */
          break;

        case CIFX_TKIT_IRQ_OTHERDEVICE:
        default:
          /* This should never happen, as the uio driver already filters our IRQs */
          break;
      }
      if (uio_irq) {
        if(info->devinstance->ulDPMSize >= NETX_DPM_MEMORY_SIZE) {
          /* if it's open for writing enable irq again (currently function not implemented in uio_netx) */
          //write(info->userdevice->uio_fd, &bVal, 1);
          HWIF_READN(info->devinstance, &ulVal, info->devinstance->pbDPM+IRQ_CFG_REG_OFFSET, sizeof(ulVal));
          ulVal |= HOST_TO_LE32(IRQ_ENABLE_MASK);
          HWIF_WRITEN(info->devinstance, info->devinstance->pbDPM+IRQ_CFG_REG_OFFSET, (void*)&ulVal, sizeof(ulVal));
        }
      }
    }
  }
  return NULL;
}

/*****************************************************************************/
/*! Enable interrupts on the given device
*     \param pvOSDependent Pointer to internal device structure              */
/*****************************************************************************/
void OS_EnableInterrupts(void* pvOSDependent) {
  PCIFX_DEVICE_INTERNAL_T info = (PCIFX_DEVICE_INTERNAL_T)pvOSDependent;
  int                     ret  = 0;
  uint32_t                ulVal;

#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  pthread_attr_init(&info->irq_thread_attr);
  pthread_attr_setstacksize(&info->irq_thread_attr, PTHREAD_STACK_MIN + IRQ_STACK_MIN_SIZE);

  if(info->set_irq_scheduler_algo)
  {
    pthread_attr_setinheritsched( &info->irq_thread_attr, PTHREAD_EXPLICIT_SCHED);
    if( (ret = pthread_attr_setschedpolicy(&info->irq_thread_attr, info->irq_scheduler_algo)) != 0)
    {
      fprintf( stderr, "Error setting custom thread scheduling algorithm for IRQ thread (pthread_attr_setschedpolicy=%d)", ret);
    }
  }

  if(info->set_irq_prio)
  {
    struct sched_param sched_param = {0};
    sched_param.sched_priority = info->irq_prio;

    pthread_attr_setinheritsched( &info->irq_thread_attr, PTHREAD_EXPLICIT_SCHED);
    if( (ret = pthread_attr_setschedparam(&info->irq_thread_attr, &sched_param)) != 0)
    {
      fprintf( stderr, "Error setting custom thread priority in IRQ thread (pthread_attr_setschedparam=%d)", ret);
    }
  }

  if( (ret = pthread_create( &info->irq_thread, &info->irq_thread_attr, netx_irq_thread,
                      (void*)info )) != 0 )
  {
    fprintf( stderr, "Enabling Interrupts (pthread_create=%d)", ret);
  } else
  {
    info->irq_stop = 0;
    if(info->devinstance->ulDPMSize >= NETX_DPM_MEMORY_SIZE) {
      HWIF_READN(info->devinstance, &ulVal, info->devinstance->pbDPM+IRQ_CFG_REG_OFFSET, sizeof(ulVal));
      ulVal |= HOST_TO_LE32(IRQ_ENABLE_MASK);
      HWIF_WRITEN(info->devinstance, info->devinstance->pbDPM+IRQ_CFG_REG_OFFSET, (void*)&ulVal, sizeof(ulVal));
    }
  }
}

/*****************************************************************************/
/*! Disable interrupts on the given device
*     \param pvOSDependent Pointer to internal device structure              */
/*****************************************************************************/
void OS_DisableInterrupts(void* pvOSDependent) {
  PCIFX_DEVICE_INTERNAL_T info = (PCIFX_DEVICE_INTERNAL_T)pvOSDependent;
  uint32_t                ulVal;

#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

  info->irq_stop = 1;
  pthread_join(info->irq_thread, NULL);

  if(info->devinstance->ulDPMSize >= NETX_DPM_MEMORY_SIZE) {
    HWIF_READN(info->devinstance, &ulVal, info->devinstance->pbDPM+IRQ_CFG_REG_OFFSET, sizeof(ulVal));
    ulVal &= HOST_TO_LE32(~IRQ_ENABLE_MASK);
    HWIF_WRITEN(info->devinstance, info->devinstance->pbDPM+IRQ_CFG_REG_OFFSET, (void*)&ulVal, sizeof(ulVal));
  }

  pthread_attr_destroy(&info->irq_thread_attr);
}

/*****************************************************************************/
/*! Open file for reading
*     \param szFilename   File to open (including path)
*     \param pulFileSize  Returned size of the file in bytes
*     \return Handle to the file, NULL on failure                            */
/*****************************************************************************/
void* OS_FileOpen(char* szFilename, uint32_t * pulFileSize) {
  int         fd;
  struct stat buf;
#ifdef VERBOSE_1
  printf("%s(%s) called\n", __FUNCTION__, szFilename);
#endif
  fd = open(szFilename, O_RDONLY);
  if( fd == -1 )
  {
    return NULL;
  }

  if( fstat(fd, &buf) != 0 )
  {
    perror("fstat failed");
    return NULL;
  }

  *pulFileSize = buf.st_size;
#ifdef VERBOSE
  printf("opened: %s (%u bytes)\n", szFilename, *pulFileSize);
#endif

  return fdopen(fd, "r");
}

/*****************************************************************************/
/*! Read data from file
*     \param pvFile    Handle to the file (acquired by OS_FileOpen)
*     \param ulOffset  Offset to read from
*     \param ulSize    Size to read
*     \param pvBuffer  Buffer to read data into
*     \return number of bytes read                                           */
/*****************************************************************************/
uint32_t OS_FileRead(void* pvFile, uint32_t ulOffset,
                          uint32_t ulSize, void* pvBuffer) {
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  return fread(pvBuffer, 1, ulSize, pvFile);
}

/*****************************************************************************/
/*! Close open file
*     \param pvFile    Handle to the file (acquired by OS_FileOpen)          */
/*****************************************************************************/
void OS_FileClose(void* pvFile) {
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  if( fclose(pvFile) != 0 )
    perror("FileClose failed");
}

/*****************************************************************************/
/*! Get Millisecond counter value (used for timeout handling)
*     \return Counter value with a resolution of 1ms                         */
/*****************************************************************************/
uint32_t OS_GetMilliSecCounter(void) {
  struct timespec ts_get_milli;
  unsigned int    msec_count;

#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  if( clock_gettime( CLOCK_MONOTONIC, &ts_get_milli ) != 0 )
  {
    perror("gettime failed");
    return 0;
  }
  msec_count = ts_get_milli.tv_sec * 1000;
  msec_count += ts_get_milli.tv_nsec / 1000 / 1000;

  return msec_count;
}

/*****************************************************************************/
/*! Sleep for the given time
*     \param ulSleepTimeMs Time in ms to sleep (0 will sleep for 50us)       */
/*****************************************************************************/
void OS_Sleep(uint32_t ulSleepTimeMs) {
  struct timespec sleeptime;
  struct timespec RemainingTime;
  struct timespec *pRemainingTime = &RemainingTime;
  int    iRet;
  int    iTmpErrno;

  if(ulSleepTimeMs == 0)
  {
#ifdef NO_MIN_SLEEP
    /* do not sleep and return immediately */
    return;
#else
    sleeptime.tv_sec = 0;
    sleeptime.tv_nsec = 50000; // 50 usecs
#endif
  } else
  {
    sleeptime.tv_sec = ulSleepTimeMs / 1000;
    ulSleepTimeMs -= sleeptime.tv_sec * 1000;
    sleeptime.tv_nsec = ulSleepTimeMs * 1000 * 1000;
  }

  iTmpErrno = errno;
  errno = 0;
  while((iRet = nanosleep(&sleeptime, pRemainingTime)))
  {
    if ((errno == EINTR) && (pRemainingTime != NULL) )
    {
      sleeptime.tv_sec  = RemainingTime.tv_sec;
      sleeptime.tv_nsec = RemainingTime.tv_nsec;
    } else
    {
      perror("OS_Sleep failed");
    }
  }
  errno = iTmpErrno;
}

/*****************************************************************************/
/*! Create mutex
*     \return Handle to new created mutex                                    */
/*****************************************************************************/
void* OS_CreateMutex(void) {
  pthread_mutex_t     *mut = malloc(sizeof(pthread_mutex_t));
  pthread_mutexattr_t attr;
  int                 iRet;
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

  if( mut == NULL )
  {
    perror("allocating memory for mutex failed");
    return NULL;
  }
  if( (iRet = pthread_mutexattr_init(&attr)) != 0 )
  {
    fprintf( stderr, "Mutex init attr: %s\n", strerror(iRet));
    goto err_out;
  }
  if( (iRet = pthread_mutex_init(mut, &attr)) != 0 )
  {
    fprintf( stderr, "Mutex init: %s\n", strerror(iRet));
    goto err_out;
  }
  return (void*) mut;

err_out:
  free(mut);
  return NULL;
}

/*****************************************************************************/
/*! Add timeout (given in ms) to timespec struct. On success the new value is
 * returned in time_val.
*     \param time_val  (in/out) time on which msec should be added
*     \param msec      time to add (ms)
*     \return 0 on success                                                   */
/*****************************************************************************/
int add_msec_to_timespec( struct timespec* time_val, uint32_t msec)
{
  if (time_val == NULL)
    return -1;

  time_val->tv_sec += msec / 1000;                 /* integer part in seconds  */
  msec              = (msec % 1000) * 1000 * 1000; /* reminder in nano seconds */

  time_val->tv_nsec += msec;                       /* add nano seconds         */
  if (time_val->tv_nsec >= 1000000000)
  {
    time_val->tv_sec++;
    time_val->tv_nsec = time_val->tv_nsec - 1000000000;
  }
  return 0;
}

/*****************************************************************************/
/*! Try to acquire mutex with timeout
*     \param pvMutex   Handle to mutex
*     \param ulTimeout Timeout in ms to wait for mutex
*     \return !=0 if mutex was acquired                                      */
/*****************************************************************************/
int OS_WaitMutex(void* pvMutex, uint32_t ulTimeout) {
  struct timespec lock_ts;
  pthread_mutex_t *mut = (pthread_mutex_t*) pvMutex;
  int             iRet;

#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  clock_gettime( CLOCK_REALTIME, &lock_ts );

  if (add_msec_to_timespec( &lock_ts, ulTimeout))
  {
    fprintf( stderr, "OS_WaitMutex(): Faild to calculate time to block\n");
    return 0;
  } else
  {
    if( (iRet = pthread_mutex_timedlock(mut, &lock_ts)) != 0 )
    {
      if (iRet != ETIMEDOUT)
        fprintf( stderr, "Mutex wait: %s\n", strerror(iRet));

      return 0;
    }
  }
  return 1;
}

/*****************************************************************************/
/*! Release previously acquired mutex
*     \param pvMutex   Handle to mutex                                       */
/*****************************************************************************/
void OS_ReleaseMutex(void* pvMutex) {
  pthread_mutex_t *mut = (pthread_mutex_t*) pvMutex;
  int             iRet;

#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

  if( (iRet = pthread_mutex_unlock(mut)) != 0 )
  {
    fprintf( stderr, "Mutex unlock: %s\n", strerror(iRet));
  }
}

/*****************************************************************************/
/*! Delete mutex
*     \param pvMutex   Handle to mutex                                       */
/*****************************************************************************/
void OS_DeleteMutex(void* pvMutex) {
  pthread_mutex_t *mut = (pthread_mutex_t*) pvMutex;
  int             iRet;

#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

  if( (iRet = pthread_mutex_destroy(mut)) != 0 )
    fprintf( stderr, "Delete mutex: %s\n", strerror(iRet));

  free(mut);
}

/*****************************************************************************/
/*! Create Lock (Usually same as mutex, but does not support timed waiting)
*     \return Handle to created lock                                         */
/*****************************************************************************/
void* OS_CreateLock(void) {
  pthread_mutexattr_t mta;
  pthread_mutex_t     *mutex;
  int                 iRet;

#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

  pthread_mutexattr_init(&mta);
  if( (iRet = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE)) != 0 )
  {
    fprintf( stderr, "Mutex set attr: %s\n", strerror(iRet));
    return NULL;
  }
  mutex = malloc( sizeof(pthread_mutex_t) );
  if( mutex == NULL )
  {
    perror("allocating memory for mutex");
    return NULL;
  }
  if( (iRet = pthread_mutex_init(mutex, &mta)) != 0 )
  {
    fprintf( stderr, "Mutex init: %s\n", strerror(iRet));
    goto err_out;
  }
  return mutex;

err_out:
  free(mutex);
  return NULL;
}

/*****************************************************************************/
/*! Acquire a lock
*     \param pvLock Handle to lock                                           */
/*****************************************************************************/
void OS_EnterLock(void* pvLock) {
  pthread_mutex_t *mutex = (pthread_mutex_t *) pvLock;
  int             iRet;

#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

  if( (iRet = pthread_mutex_lock(mutex)) != 0)
  {
    fprintf( stderr, "EnterLock failed: %s\n", strerror(iRet));
  }
}

/*****************************************************************************/
/*! Release a lock
*     \param pvLock Handle to lock                                           */
/*****************************************************************************/
void OS_LeaveLock(void* pvLock) {
  pthread_mutex_t *mutex = (pthread_mutex_t *) pvLock;
  int             iRet;

#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

  if( (iRet = pthread_mutex_unlock(mutex)) != 0)
  {
    fprintf( stderr, "Mutex unlock: %s\n", strerror(iRet));
  }
}

/*****************************************************************************/
/*! Delete a lock
*     \param pvLock Handle to lock                                           */
/*****************************************************************************/
void OS_DeleteLock(void* pvLock) {
  pthread_mutex_t *mutex = (pthread_mutex_t *) pvLock;
  int             iRet;

#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

  if( (iRet = pthread_mutex_destroy(mutex)) != 0 )
    fprintf( stderr, "Delete lock: %s\n", strerror(iRet));

  free(mutex);
}

/*****************************************************************************/
/*! Compare strings
*     \param pszBuf1  String buffer 1
*     \param pszBuf2  String buffer 2
*     \return 0 if strings are equal                                         */
/*****************************************************************************/
int OS_Strcmp(const char* pszBuf1, const char* pszBuf2) {
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  return strcmp(pszBuf1, pszBuf2);
}

/*****************************************************************************/
/*! Compare strings case insensitive
*     \param pszBuf1  String buffer 1
*     \param pszBuf2  String buffer 2
*     \param ulLen    Maximum length to compare
*     \return 0 if strings are equal                                         */
/*****************************************************************************/
int OS_Strnicmp(const char* pszBuf1, const char* pszBuf2, uint32_t ulLen) {
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  return strncasecmp(pszBuf1, pszBuf2, ulLen);
}

/*****************************************************************************/
/*! Get length of string
*     \param szText  Text buffer
*     \return Length of given string                                         */
/*****************************************************************************/
int OS_Strlen(const char* szText) {
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  return strlen(szText);
}

/*****************************************************************************/
/*! Copy string to destination buffer
*     \param szText   Destination string
*     \param szSource Source string
*     \param ulLen    Maximum length to copy
*     \return Pointer to szDest                                              */
/*****************************************************************************/
char* OS_Strncpy(char* szDest, const char* szSource, uint32_t ulLen) {
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  return strncpy(szDest, szSource, ulLen);
}

/*****************************************************************************/
/*! Map driver pointer to user space
*     \param pvDriverMem  Pointer to driver memory
*     \param ulMemSize    Size of the memory to map
*     \param ppvMappedMem Returned mapped pointer
*     \param os_dependent OS Dependent parameter in DEVICEINSTANCE
*     \param fCached      Caching option (0=do not use) -> currently ignored
*     \return Handle to mapping, NULL on error                               */
/*****************************************************************************/
void* OS_MapUserPointer(void* pvDriverMem, uint32_t ulMemSize,
                void** ppvMappedMem, void *os_dependent, unsigned char fCached) {
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  /* We don't need to do any mapping, as we are already in user space */
  *ppvMappedMem = pvDriverMem;

  return pvDriverMem;
}

/*****************************************************************************/
/*! Unmap previously mapped user space pointer
*     \param phMapping  Handle returned from OS_MapUserPointer
*     \param os_dependent OS Dependent parameter in DEVICEINSTANCE
*     \return 0 on error                                                     */
/*****************************************************************************/
int OS_UnmapUserPointer(void* phMapping, void *os_dependent) {
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  return 1;
}

/*****************************************************************************/
/*! This function invalidates a cache buffer to be refreshed by
*   the physical memory.
*   \param pvMem      Pointer to the cached memory
*   \param ulMemSize  Length of the cached memory area                       */
/*****************************************************************************/
void OS_InvalidateCacheMemory_FromDevice(void* pvCachedMemPtr, unsigned long ulMemSize) {
  /* not implemented yet */
}

/*****************************************************************************/
/*! This function flushes a cached memory area to the device buffer
*   \param pvMem      Pointer to the cached memory
*   \param ulMemSize  Length of the cached memory area                   */
/*****************************************************************************/
void OS_FlushCacheMemory_ToDevice(void* pvMem, unsigned long ulMemSize)
{
  /* not implemented yet */
}

/*****************************************************************************/
/*! Structure for event handling                                             */
/*****************************************************************************/
struct os_event {
  pthread_mutex_t mutex;           /*!< Mutex to lock access to set, waiting_threads
                                        and cond */
  int             set;             /*!< Protected by mutex. !=0 if event is set */
  int             waiting_threads; /*!< Number of waiting threads on this event */
  pthread_cond_t  cond;            /*!< Condition to signal, if event state has changed,
                                        and a thread is waiting */
};

/*****************************************************************************/
/*! Create event
*     \return Handle to created event                                        */
/*****************************************************************************/
void* OS_CreateEvent(void) {
  struct os_event     *ev       = malloc( sizeof(*ev) );
  pthread_condattr_t  ev_attr;
  pthread_mutexattr_t ev_mutattr;
  int                 iRet;

#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

  if( ev == NULL )
  {
    perror("allocating memory for OS_Event");
    return NULL;
  }

  if( (iRet = pthread_condattr_init( &ev_attr )) != 0 )
  {
    fprintf( stderr, "Cond init attr: %s\n", strerror(iRet));
    goto free_out;
  }

  pthread_condattr_setclock(&ev_attr, CLOCK_MONOTONIC);

  if( (iRet = pthread_cond_init( &(ev->cond), &ev_attr )) != 0 )
  {
    fprintf( stderr, "Cond init: %s\n", strerror(iRet));
    goto free_out;
  }
  if( (iRet = pthread_mutexattr_init(&ev_mutattr)) != 0 )
  {
    fprintf( stderr, "Mutex init attr: %s\n", strerror(iRet));
    goto free_out;
  }
  if( (iRet = pthread_mutexattr_setprotocol(&ev_mutattr, PTHREAD_PRIO_INHERIT)) != 0 )
  {
    fprintf( stderr, "Mutex set attr: %s\n", strerror(iRet));
    goto free_out;
  }
  if( (iRet = pthread_mutex_init(&(ev->mutex), &ev_mutattr)) != 0 )
  {
    fprintf( stderr, "Mutex init: %s\n", strerror(iRet));
    goto free_out;
  }

  ev->set             = 0;
  ev->waiting_threads = 0;

  return ev;

free_out:
  free(ev);
  return NULL;
}

/*****************************************************************************/
/*! Signal event
*     \param pvEvent Handle to event                                         */
/*****************************************************************************/
void OS_SetEvent(void* pvEvent) {
  struct os_event *ev = (struct os_event *) pvEvent;
  int             iRet;

#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

  if( ev == NULL )
  {
    fprintf(stderr, "SetEvent, no event given\n");
  } else
  {
    pthread_mutex_lock(&ev->mutex);

    if (!ev->set)
    {
      ev->set = 1;

      /* Check if there are any waiters, and release them appropriately */
      if(ev->waiting_threads > 0)
      {
        if( (iRet = pthread_cond_signal(&(ev->cond))) != 0 )
          fprintf( stderr, "SetEvent: %s\n", strerror(iRet));

      }
    }

    pthread_mutex_unlock(&ev->mutex);
  }
}

/*****************************************************************************/
/*! Reset event
*     \param pvEvent Handle to event                                         */
/*****************************************************************************/
void OS_ResetEvent(void* pvEvent) {
  struct os_event *ev = (struct os_event *) pvEvent;
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif
  if( ev == NULL )
  {
    fprintf(stderr, "ResetEvent, no event given\n");
  } else
  {
    pthread_mutex_lock(&ev->mutex);

    ev->set = 0;

    pthread_mutex_unlock(&ev->mutex);
  }
}

/*****************************************************************************/
/*! Delete event
*     \param pvEvent Handle to event                                         */
/*****************************************************************************/
void OS_DeleteEvent(void* pvEvent) {
  struct os_event *ev = (struct os_event *) pvEvent;
  int             iRet;
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

  if( (iRet = pthread_cond_destroy(&(ev->cond)) ) != 0 )
    fprintf( stderr, "Delete event cond: %s\n", strerror(iRet));

  if( (iRet = pthread_mutex_destroy(&(ev->mutex)) ) != 0 )
    fprintf( stderr, "Delete mutex: %s\n", strerror(iRet));

  free(ev);
}

/*****************************************************************************/
/*! Wait for event
*     \param pvEvent   Handle to event
*     \param ulTimeout Timeout in ms to wait for event
*     \return CIFX_EVENT_SIGNALLED if event was set, CIFX_EVENT_TIMEOUT otherwise */
/*****************************************************************************/
uint32_t OS_WaitEvent(void* pvEvent, uint32_t ulTimeout) {
  struct os_event *ev = (struct os_event *) pvEvent;
  struct timespec timeout;
  unsigned long   ret = CIFX_EVENT_TIMEOUT;
#ifdef VERBOSE_1
  printf("%s() called\n", __FUNCTION__);
#endif

  if( clock_gettime(CLOCK_MONOTONIC, &timeout) != 0 )
  {
    perror("WaitEvent gettime failed");
  } else
  {
    if (add_msec_to_timespec( &timeout, ulTimeout))
    {
      fprintf( stderr, "OS_WaitEvent(): Faild to calculate time to block\n");
      return ret;
    }

    pthread_mutex_lock(&ev->mutex);

    if(!ev->set)
    {
      /* We need to wait for event */
      ++ev->waiting_threads;

      pthread_cond_timedwait(&ev->cond, &ev->mutex, &timeout);

      --ev->waiting_threads;
    }

    if(ev->set)
    {
      /* We got the event, now reset it */
      ev->set = 0;
      ret = CIFX_EVENT_SIGNALLED;
    }

    pthread_mutex_unlock(&ev->mutex);
  }

  return ret;
}

#ifdef CIFX_TOOLKIT_TIME
/*****************************************************************************/
/*! Get the system time since 1970/01/01
*     \param ptTime Pointer to store  the time value
*     \return actual time vlaue */
/*****************************************************************************/
uint32_t OS_Time( uint32_t *ptTime)
{
  struct timeval tCurrentTime = {0};
  time_t         tSecSince1970 = 0;

  /* get local time */
  if (0 == gettimeofday( &tCurrentTime, NULL))
    tSecSince1970 = tCurrentTime.tv_sec;

  if (ptTime)
    *ptTime = (uint32_t)tSecSince1970;

  return (uint32_t)tSecSince1970;
}
#endif
