/**************************************************************************************
 *
 * Copyright (c) 2024, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 * Description: This plugin library can be used to initialize and use a spidev as a "common"
 *              cifX device. The access is based on the cifX toolkit's 'HW-access' functions.
 *
 * Changes:
 *
 *   Version   Date        Author   Description
 *   ----------------------------------------------------------------------------------
 *   1        02.01.24    SD        changed licensing terms
 *
 **************************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <errno.h>
#include <pthread.h>
#include "cifxlinux.h"

/******************************************************************************/
/*** GLOBAL DEFINITIONS *******************************************************/
/******************************************************************************/
#if defined(VERBOSE) || defined(DEBUG)
#define DBG(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#else
#define DBG(fmt, ...)
#endif

#define CHECK_STATE /* enable error message in case of dpm status changes to != 0x11 (printed to stderr) */

/******************************************************************************/
/*** STRUCTURE DEFINITIONS ****************************************************/
/******************************************************************************/
/* states of the SPI interface */
typedef enum ESPI_IF_STATE_E {
  eNotInitialized,
  eInitialized,
} ESPI_IF_STATE;

/* Example structure containing information of the SPI interface (passed as user parameter -> see tSPIDev.userparam) */
struct SPI_PARAM_T {
  char          szName[256];  /* Name of the SPI interface (e.g. /dev/spidev1.0) */
  int           iSPIFD;       /* File desc of the SPI interface                  */
  void*         pvSerDPMLock; /* lock required to synchronize SPI access         */
  int           iError;       /* Error status of SPI interface                   */
  ESPI_IF_STATE eState;       /* Current interface status                        */
  /* The following parameter are related to the linux kernel 'spidev' interface  */
  uint32_t      ulUDelay;     /* SPI param:  If nonzero, how long to delay after the last bit transfer         */
  /*             before optionally deselecting the device before the next transfer */
  uint32_t      ulSpeed;      /* SPI param: see SPI_IOC_RD_MAX_SPEED_HZ, SPI_IOC_WR_MAX_SPEED_HZ    */
  uint8_t       bBits;        /* SPI param: see SPI_IOC_RD_BITS_PER_WORD, SPI_IOC_WR_BITS_PER_WORD */
  uint8_t       bMode;        /* SPI param: see SPI_IOC_RD_MODE, SPI_IOC_WR_MODE */
  uint8_t*      pabTXBuffer;
  uint8_t*      pabRXBuffer;
  uint32_t      ulChunkSize;
  uint8_t       bCSChange;
};

/* Internal structures required for read/write transactions */
/* SPI Read Transfer Structure */
struct SPI_RD_MSG_T {
  uint8_t  abSPIHeader[4];
  uint8_t  abData[1];
};
/* SPI Write Transfer Structure */
struct SPI_WR_MSG_T {
  uint8_t  abSPIHeader[3];
  uint8_t  abData[1];
};

/******************************************************************************/
/*** Global variables *********************************************************/
/******************************************************************************/
#define DEFAULT_BUFFER_SIZE  (8*1024)

/*****************************************************************************/
/*! Create a lock
*     \return Lock Handle                                                    */
/*****************************************************************************/
static void* CreateLock(void) {
  pthread_mutexattr_t mta;
  pthread_mutex_t     *mutex;
  int                 iRet;

  pthread_mutexattr_init(&mta);
  if( (iRet = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE)) != 0 )
    return NULL;

  mutex = malloc( sizeof(pthread_mutex_t) );
  if( mutex == NULL )
    return NULL;

  if( (iRet = pthread_mutex_init(mutex, &mta)) != 0 )
    goto err_out;

  return mutex;

err_out:
  free(mutex);
  return NULL;
}

/*****************************************************************************/
/*! Acquire a lock
*     \param pvLock Handle to lock                                           */
/*****************************************************************************/
static void EnterLock(void* pvLock) {
  pthread_mutex_t *mutex = (pthread_mutex_t *) pvLock;
  int             iRet;

  if( (iRet = pthread_mutex_lock(mutex)) != 0)
  {
    fprintf( stderr, "Locking failed: %s\n", strerror(iRet));
  }
}

/*****************************************************************************/
/*! Release a lock
*     \param pvLock Handle to lock                                           */
/*****************************************************************************/
static void LeaveLock(void* pvLock) {
  pthread_mutex_t *mutex = (pthread_mutex_t *) pvLock;
  int             iRet;

  if( (iRet = pthread_mutex_unlock(mutex)) != 0)
  {
    fprintf( stderr, "Unlock failed: %s\n", strerror(iRet));
  }
}

/*****************************************************************************/
/*! Delete a lock
*     \param pvLock Handle to lock                                           */
/*****************************************************************************/
static void DeleteLock(void* pvLock) {
  pthread_mutex_t *mutex = (pthread_mutex_t *) pvLock;

  pthread_mutex_destroy(mutex);
  free(mutex);
}

/******************************************************************************/
/*! Helper function, transfers SPI message.
 *   \param ptSPIParam Pointer to interface specific parameter
 *   \param pbData     Pointer to SPI message
 *   \param pbData     length of SPI message
 *   \param fCSChange                                                         */
/******************************************************************************/
static int SPITransferMessage(struct SPI_PARAM_T* ptSPIParam, uint8_t *pbData, uint32_t ulLen, uint8_t fCSChange)
{
  int ret = 0;
  struct spi_ioc_transfer tSPITransfer = {0};

  DBG("++SPITransferMessage\n");

  tSPITransfer.tx_buf    = (uint64_t)pbData;
  tSPITransfer.rx_buf    = (uint64_t)pbData;
  tSPITransfer.len       = ulLen;
  tSPITransfer.cs_change = fCSChange;

  if(0 > (ret = ioctl( ptSPIParam->iSPIFD, SPI_IOC_MESSAGE(1), &tSPITransfer)))
    fprintf(stderr, "SPITransferMessage: Failed to transfer message on SPI device '%s' - '%s'.\n", ptSPIParam->szName, strerror(errno));

#ifdef CHECK_STATE
  if (0x11 != pbData[0]) {
    ret = -EAGAIN;
    fprintf(stderr, "DPM status changed 0x%X (OK => 0x11)!\n", pbData[0]);
  }
#endif

  DBG("--SPITransferMessage\n");

  return ret;
}

/******************************************************************************/
/*! Helper function, reading data via SPI interface.
 *  Translate read request into SPI read message.
 *   \param ptSPIParam Pointer to interface specific parameter
 *   \param ulDpmAddr Address offset in DPM to read data from
 *   \param pbData    Pointer to Buffer to store read data
 *   \param ulLen     Number of bytes to read                                 */
/******************************************************************************/
static void SPIReadChunk( struct SPI_PARAM_T* ptSPIParam, uint32_t ulDpmAddr, uint8_t *pbData, uint32_t ulLen)
{
  struct SPI_RD_MSG_T* ptSPIRDMsg = NULL;
  int ret = 0;

  DBG("++SPIReadChunk\n");

  /* Check buffer size */
  if(*(uint16_t*)ptSPIParam->pabRXBuffer < ulLen) {
    fprintf(stderr, "SPIReadChunk: RX SPI buffer is too small\n");
    return;
  }

  ptSPIRDMsg = (struct SPI_RD_MSG_T*)(ptSPIParam->pabRXBuffer+sizeof(uint16_t));

  /* prepare SPI message */
  ptSPIRDMsg->abSPIHeader[0] = 0x80 + (uint8_t)((ulDpmAddr >> 16) & 0x0F);
  ptSPIRDMsg->abSPIHeader[1] = (uint8_t)((ulDpmAddr >> 8) & 0xFF);
  ptSPIRDMsg->abSPIHeader[2] = (uint8_t)((ulDpmAddr) & 0xFF);
  ptSPIRDMsg->abSPIHeader[3] = 0x00;

  /* transfer message */

  ret = SPITransferMessage(ptSPIParam, (uint8_t*)ptSPIRDMsg, ulLen+4, ptSPIParam->bCSChange);
#ifdef CHECK_STATE
  if (0>ret) {
    fprintf(stderr, "Error SPIReadChunk: DPM Addr=0x%X / Len=0x%X\n", ulDpmAddr, ulLen) ;
  }
#endif

  /* return read data */
  memcpy(pbData, ptSPIRDMsg->abData, ulLen);

  DBG("--SPIReadChunk\n");
}

/******************************************************************************/
/*! Helper function, writing data via SPI interface
 *  Translate write request into SPI write message
 *   \param ptSPIParam Pointer to interface specific parameter
 *   \param ulDpmAddr  Offset in DPM where data to write to
 *   \param pbData     Pointer to Buffer pointing to write data
 *   \param ulLen      Number of bytes to write                               */
/******************************************************************************/
static void SPIWriteChunk(struct SPI_PARAM_T* ptSPIParam, uint32_t ulDpmAddr, uint8_t *pbData, uint32_t ulLen)
{
  struct SPI_WR_MSG_T* ptSPIWRMsg = NULL;
  int ret = 0;

  DBG("++SPIWriteChunk\n");

  /* Check buffer size */
  if(*(uint16_t*)ptSPIParam->pabTXBuffer < ulLen) {
    fprintf(stderr, "SPIWriteChunk: TX SPI buffer is too small\n");
    return;
  }

  ptSPIWRMsg = (struct SPI_WR_MSG_T*)(ptSPIParam->pabTXBuffer+sizeof(uint16_t));

  /* prepare SPI message */
  ptSPIWRMsg->abSPIHeader[0] = (uint8_t)((ulDpmAddr >> 16) & 0x0F);
  ptSPIWRMsg->abSPIHeader[1] = (uint8_t)((ulDpmAddr >> 8) & 0xFF);
  ptSPIWRMsg->abSPIHeader[2] = (uint8_t)((ulDpmAddr) & 0xFF);

  memcpy(ptSPIWRMsg->abData, pbData, ulLen);

  /* transfer message */
  ret = SPITransferMessage(ptSPIParam, (uint8_t*)ptSPIWRMsg, ulLen+3, ptSPIParam->bCSChange);
#ifdef CHECK_STATE
  if (0>ret) {
    fprintf(stderr, "Error SPIWriteChunk: DPM Addr=0x%X / Len=0x%X\n", ulDpmAddr, ulLen) ;
  }
#endif

  DBG("--SPIWriteChunk\n");
}

/******************************************************************************/
/*! Read a number of bytes via the custom hardware interface function
 *   \param ptDevice  Pointer to the custom device
 *   \param pvDpmAddr Address offset in DPM to read data from
 *   \param pvDst     Buffer to store read data
 *   \param ulLen     Number of bytes to read                                 */
/******************************************************************************/
static void* SPIHWIFRead(struct CIFX_DEVICE_T* ptDevice, void* pvDpmAddr, void* pvDst, uint32_t ulLen)
{
  struct SPI_PARAM_T* ptSPIParam = (struct SPI_PARAM_T*)ptDevice->userparam;

  DBG("++SPIHWIFRead\n");

  /* check if interface is correctly configured */
  if ((NULL != ptSPIParam) && (ptSPIParam->eState == eInitialized)) {
    /* enter SPI access lock */
    EnterLock(ptSPIParam->pvSerDPMLock);
    /* read data from DPM */
    SPIReadChunk(ptSPIParam, (uint32_t)(uintptr_t)pvDpmAddr, (uint8_t*)pvDst, ulLen);
    LeaveLock(ptSPIParam->pvSerDPMLock);
  }
  DBG("--SPIHWIFRead\n");
  return pvDst;
}

static void* SPIHWIFRead_ext(struct CIFX_DEVICE_T* ptDevice, void* pvDpmAddr, void* pvDst, uint32_t ulLen)
{
  struct SPI_PARAM_T* ptSPIParam = (struct SPI_PARAM_T*)ptDevice->userparam;
  int   reads  = ulLen/ptSPIParam->ulChunkSize;
  void* ret    = NULL;

  while(reads>0) {
    ret       = SPIHWIFRead(ptDevice,pvDpmAddr,pvDst,ptSPIParam->ulChunkSize);
    pvDpmAddr = (void*)((uintptr_t)pvDpmAddr + ptSPIParam->ulChunkSize);
    pvDst     = (void*)((uintptr_t)pvDst     + ptSPIParam->ulChunkSize);
    reads--;
  }
  if (ulLen%ptSPIParam->ulChunkSize)
    ret = SPIHWIFRead(ptDevice,pvDpmAddr,pvDst,ulLen%ptSPIParam->ulChunkSize);

  return ret;
}

/******************************************************************************/
/*! Read a number of bytes via the custom hardware interface function
 *   \param ptDevice  Pointer to the custom device
 *   \param pvDpmAddr Address offset in DPM to write data to
 *   \param pvSrc     Buffer to data to be written
 *   \param ulLen     Number of bytes to writ                                 */
/******************************************************************************/
static void* SPIHWIFWrite(struct CIFX_DEVICE_T* ptDevice, void* pvDpmAddr, void* pvSrc, uint32_t ulLen)
{
  struct SPI_PARAM_T* ptSPIParam = (struct SPI_PARAM_T*)ptDevice->userparam;

  DBG("++SPIHWIFWrite\n");

  /* check if interface is correctly configured */
  if ((NULL != ptSPIParam) && (ptSPIParam->eState == eInitialized)) {
    /* enter SPI access lock */
    EnterLock(ptSPIParam->pvSerDPMLock);
    /* write data to DPM */
    SPIWriteChunk(ptSPIParam, (uint32_t)(uintptr_t)pvDpmAddr, (uint8_t*)pvSrc, ulLen);
    LeaveLock(ptSPIParam->pvSerDPMLock);
  }
  DBG("--SPIHWIFWrite\n");
  return pvDpmAddr;
}

/*  */
static void* SPIHWIFWrite_ext(struct CIFX_DEVICE_T* ptDevice, void* pvDpmAddr, void* pvSrc, uint32_t ulLen)
{
  struct SPI_PARAM_T* ptSPIParam = (struct SPI_PARAM_T*)ptDevice->userparam;
  int writes = ulLen/ptSPIParam->ulChunkSize;
  void* ret = NULL;

  while(writes>0) {
    ret = SPIHWIFWrite(ptDevice,pvDpmAddr,pvSrc,ptSPIParam->ulChunkSize);
    pvDpmAddr = (void*)((uintptr_t)pvDpmAddr + ptSPIParam->ulChunkSize);
    pvSrc     = (void*)((uintptr_t)pvSrc     + ptSPIParam->ulChunkSize);
    writes--;
  }
  if (ulLen%ptSPIParam->ulChunkSize)
    ret = SPIHWIFWrite(ptDevice,pvDpmAddr,pvSrc,ulLen%ptSPIParam->ulChunkSize);

  return ret;
}

/******************************************************************************/
/*! De-initializes SPI interface of device pointed by ptDevice.
 *   \param ptDevice  Pointer to the custom device                            */
/******************************************************************************/
static void SPIHWIFDeInit(struct CIFX_DEVICE_T* ptDevice)
{
  struct SPI_PARAM_T* ptSPIParam = (struct SPI_PARAM_T*)ptDevice->userparam;

  DBG("++SPIHWIFDeInit\n");
  if(ptSPIParam->pvSerDPMLock) {
    DeleteLock(ptSPIParam->pvSerDPMLock);
    ptSPIParam->pvSerDPMLock = NULL;
  }
  close(ptSPIParam->iSPIFD);

  ptSPIParam->eState = eNotInitialized;
  DBG("--SPIHWIFDeInit\n");
}

int32_t DoDummyRead( struct SPI_PARAM_T* ptSPIParam)
{
  struct spi_ioc_transfer tSPITransfer = {0};
  uint8_t                 abData[8]    = {0};
  uint8_t                 bRetry       = 0;
  int32_t                 lRet         = 0;

  /* Do dummy read (required on netX51) */
  tSPITransfer.tx_buf    = (unsigned long)abData;
  tSPITransfer.rx_buf    = (unsigned long)abData;
  tSPITransfer.len       = sizeof(abData);
  tSPITransfer.cs_change = ptSPIParam->bCSChange;

  do {
    memset(abData, 0, sizeof(abData));
    abData[0] = 0x80; /* set read request, cmd, A19:16 */
    abData[1] = 0xff; /* A15:8 */
    abData[2] = 0xfc; /* A7:0 */
    abData[3] = 0;    /* length */

    /* issue dummy transfer */
    if(0 > (lRet = ioctl(ptSPIParam->iSPIFD, SPI_IOC_MESSAGE(1), &tSPITransfer))) {
      fprintf( stderr, "SPIHWIFInit: Failed to send message on SPI device '%s' - '%s'.\n", ptSPIParam->szName, strerror(errno));
      ptSPIParam->iError = errno;
    } else {
      if(bRetry == 0) {
        /* skip first result since this is not valid on netX51 */
        DBG("SPI dummy read:  Header(0x%02x 0x%02x 0x%02x 0x%02x) Data(0x%02x%02x%02x%02x)\n"
            , abData[0], abData[1], abData[2], abData[3], abData[4], abData[5],abData[6], abData[7]);
      }
      else if(bRetry == 1) {
        DBG("SPI read access: Header(0x%02x 0x%02x 0x%02x 0x%02x) Data(0x%02x%02x%02x%02x)\n"
            , abData[0], abData[1], abData[2], abData[3], abData[4], abData[5],abData[6], abData[7]);
        /* check if DPM is in the correct status (serial DPM enabled and unlocked) */
        if(abData[0] != 0x11) {
          lRet = CIFX_FUNCTION_FAILED;
          fprintf( stderr, "SPIHWIFInit: Failed to read from serial DPM. Incorrect DPM status of SPI device '%s' (0x%X).\n", ptSPIParam->szName, abData[0]);
          fprintf( stderr, "SPIHWIFInit: Check the SPI connection and the serial DPM configuration of the device connected on '%s'.\n", ptSPIParam->szName);
        } else {
          ptSPIParam->ulUDelay = 0;
          ptSPIParam->eState   = eInitialized;
        }
        break;
      }
      bRetry++;
    }
  } while(0 <= lRet);

  return lRet;
}

/******************************************************************************/
/*! Initializes SPI interface of device pointed by ptDevice.
 *   \param ptDevice  Pointer to the custom device                            */
/******************************************************************************/
static int32_t SPIHWIFInit(struct CIFX_DEVICE_T* ptDevice)
{
  int32_t lRet = 0;
  struct SPI_PARAM_T* ptSPIParam = (struct SPI_PARAM_T*)ptDevice->userparam;

  DBG("++SPIHWIFInit\n");

  if (NULL == ptSPIParam) {
    fprintf(stderr, "SPIHWIFInit: Invalid initialization parameter for SPI device '%s'\n", ptSPIParam->szName);
    DBG("--SPIHWIFInit\n");
    return CIFX_INVALID_PARAMETER;
  }

  DBG("SPIHWIFInit: SPI Setup: %s, mode %d, %d bits/w, %d Hz\n",ptSPIParam->szName, ptSPIParam->bMode, ptSPIParam->bBits, ptSPIParam->ulSpeed);

  if (0 > (ptSPIParam->iSPIFD = open(ptSPIParam->szName, O_RDWR))) {
    ptSPIParam->iError = errno;
    fprintf( stderr, "SPIHWIFInit: Failed to open SPI device '%s' - '%s'.\n", ptSPIParam->szName, strerror(errno));
    DBG("--SPIHWIFInit\n");
    return CIFX_FILE_OPEN_FAILED;

  } else if (0 > (lRet = ioctl(ptSPIParam->iSPIFD, SPI_IOC_WR_MODE, &ptSPIParam->bMode))) {
    ptSPIParam->iError = errno;
    fprintf( stderr, "SPIHWIFInit: Failed to set 'SPI_IOC_WR_MODE' on SPI device '%s' - '%s'.\n", ptSPIParam->szName, strerror(errno));

  } else if (0 > (lRet = ioctl(ptSPIParam->iSPIFD, SPI_IOC_RD_MODE, &ptSPIParam->bMode))) {
    ptSPIParam->iError = errno;
    fprintf( stderr, "SPIHWIFInit: Failed to set 'SPI_IOC_RD_MODE' on SPI device '%s' - '%s'.\n", ptSPIParam->szName, strerror(errno));

  } else if (0 > (lRet = ioctl(ptSPIParam->iSPIFD, SPI_IOC_WR_BITS_PER_WORD, &ptSPIParam->bBits))) {
    ptSPIParam->iError = errno;
    fprintf( stderr, "SPIHWIFInit: Failed to set 'SPI_IOC_WR_BITS_PER_WORD' on SPI device '%s' - '%s'.\n", ptSPIParam->szName, strerror(errno));

  } else if (0 > (lRet = ioctl(ptSPIParam->iSPIFD, SPI_IOC_RD_BITS_PER_WORD, &ptSPIParam->bBits))) {
    ptSPIParam->iError = errno;
    fprintf( stderr, "SPIHWIFInit: Failed to set 'SPI_IOC_RD_BITS_PER_WORD' on SPI device '%s' - '%s'.\n", ptSPIParam->szName, strerror(errno));

  } else if (0 > (lRet = ioctl(ptSPIParam->iSPIFD, SPI_IOC_WR_MAX_SPEED_HZ, &ptSPIParam->ulSpeed))) {
    ptSPIParam->iError = errno;
    fprintf( stderr, "SPIHWIFInit: Failed to set 'SPI_IOC_WR_MAX_SPEED_HZ' on SPI device '%s' - '%s'.\n", ptSPIParam->szName, strerror(errno));

  } else if (0 > (lRet = ioctl(ptSPIParam->iSPIFD, SPI_IOC_RD_MAX_SPEED_HZ, &ptSPIParam->ulSpeed))) {
    ptSPIParam->iError = errno;
    fprintf( stderr, "SPIHWIFInit: Failed to set 'SPI_IOC_RD_MAX_SPEED_HZ' on SPI device '%s' - '%s'.\n", ptSPIParam->szName, strerror(errno));

  } else {/* successfuly initialized SPI interface */
    /* create SPI access lock */
    if (NULL != (ptSPIParam->pvSerDPMLock = (void*)CreateLock())) {
      /* Do dummy read (required on netX51) */
      lRet = DoDummyRead(ptSPIParam);
    }
  }
  if(0 > lRet)
    SPIHWIFDeInit(ptDevice);

  DBG("--SPIHWIFInit\n");

  if(0 > lRet) {
    return CIFX_FUNCTION_FAILED;
  }
  else {
    ptSPIParam->iError = 0;
    return CIFX_NO_ERROR;
  }
}

void StatusCB(struct CIFX_DEVICE_T* ptDevice, CIFX_NOTIFY_E eEvent)
{
  struct SPI_PARAM_T* ptSPIParam = ptDevice->userparam;

  if (eCIFX_EVENT_POSTRESET == eEvent) {
    /* we have to do a dummy read since netX device was reset */
    DoDummyRead(ptSPIParam);
  }
}

void SDPMDeInit(struct CIFX_DEVICE_T* ptDevice)
{
  struct SPI_PARAM_T* ptSPIParam = ptDevice->userparam;

  free(ptSPIParam->pabTXBuffer);
  free(ptSPIParam->pabRXBuffer);
  free(ptSPIParam);
  free(ptDevice);
}

/******************************************************************************/
/*! Initializes a cifX device for SDPM (usage of SPI).
 *   \param pszSPIDevice  Name of spidev device file (optional)
 *   \param bMode         SPI mode
 *   \param bBits         Bits per SPI word
 *   \param ulFrequency   SPI frequency
 *   \param pszIRQFile    Name of IRQ trigger file (optional)
 *   \return  Pointer to initialized cifX device on success
 *            NULL on error                                                   */
/******************************************************************************/
struct CIFX_DEVICE_T* SDPMInit(uint8_t *pszSPIDevice, uint8_t bMode, uint8_t bBits, uint32_t ulFrequency, uint8_t *pszIRQFile, uint32_t ulChunkSize, uint8_t bCSChange)
{
  struct CIFX_DEVICE_T* ptSPIDev = NULL;
  struct SPI_PARAM_T*   ptSPIParam = NULL;
  uint8_t szSPIDevice[261];
  int32_t lFd;

  DBG("++SDPMInit\n");

  DBG("Running SPI plugin on \"%s\" (mode=%d,bits=%d,freq=%u,chunk=%d,cs-change=%d)!\n", pszSPIDevice, bMode, bBits, ulFrequency, ulChunkSize, bCSChange);

  /* Allocate memory for the cifX device */
  ptSPIDev = calloc( 1, sizeof(*ptSPIDev));
  if(ptSPIDev == NULL) {
    fprintf(stderr, "SDPMInit: Allocate memory for the cifX device\n");
    goto error_out;
  }

  /* Allocate zero initialized memory for the SPI device */
  ptSPIParam = calloc(1, sizeof(*ptSPIParam));
  if(ptSPIParam == NULL) {
    fprintf(stderr, "SDPMInit: Allocate memory for SPI parameters\n");
    goto error_out;
  }

  /* Check for a valid SPI device */
  if(pszSPIDevice == NULL) {
    struct dirent*  entry;
    DIR*            dir = opendir("/sys/class/spidev");

    if(NULL != dir) {
      while((entry = readdir(dir)) != NULL) {
        if((0 == strncmp(entry->d_name, "spidev", 6))) {
          snprintf(szSPIDevice, sizeof(szSPIDevice), "/dev/%s", entry->d_name);
          pszSPIDevice = szSPIDevice;
          break;
        }
      }
    }
  }
  lFd = open(pszSPIDevice, O_RDWR);
  if (lFd < 0) {
    fprintf(stderr, "SDPMInit: Invalid or missing SPI device(%s)\n", pszSPIDevice);
    goto error_out;
  }
  close(lFd);

  /* Check for a valid SPI mode */
  if((bMode < SPI_MODE_0) || (SPI_MODE_3 < bMode)) {
    fprintf(stderr, "SDPMInit: Invalid SPI mode (%d <= x <= %u)\n", SPI_MODE_0, SPI_MODE_3);
    goto error_out;
  }

  /* Check for a valid IRQ file (optional) */
  lFd = -1;
  if(pszIRQFile != NULL) {
    lFd = open(pszIRQFile, O_RDWR|O_NONBLOCK);
    if(lFd < 0) {
      fprintf(stderr, "SDPMInit: Invalid SPI IRQ file (%s). Fallback to polling!\n", pszIRQFile);
    }
  }

  /* Configure the SPI device parameter */
  strncpy(ptSPIParam->szName, pszSPIDevice, sizeof(ptSPIParam->szName));
  ptSPIParam->ulUDelay    = 0;
  ptSPIParam->bMode       = bMode;
  ptSPIParam->bBits       = bBits;
  ptSPIParam->ulSpeed     = ulFrequency;
  ptSPIParam->ulChunkSize = ulChunkSize;
  ptSPIParam->bCSChange   = bCSChange;

  /* Configuration of a SPI device */
  ptSPIDev->dpm         = NULL;    /*!< set to 'NULL' since the SPI device is not memory mapped */
  ptSPIDev->dpmaddr     = 0x0;     /*!< set to '0x00' since there is no direct address to the physical DPM */
  ptSPIDev->dpmlen      = 0x10000; /*!< set to length of dpm (depends on the device) */

  /* Since device is not a uio device and no pci card invalidate all parameter */
  ptSPIDev->uio_num     = -1;      /*!< set to '-1' since it is no 'uio-devices' */
  ptSPIDev->uio_fd      = (lFd >= 0) ? lFd : -1; /*!< set to '-1' since it is no 'uio-devices' */
  ptSPIDev->pci_card    = 0;       /*!< set to 0 since it is no pci card */
  ptSPIDev->force_ram   = 0;       /*!< Force usage of RAM instead of flash. Card will always be reset and all
                                        files are downloaded again (0 => autodetect) */

  /* In this example we are not interrested in any configuration state, so set parameter to NULL */
  ptSPIDev->notify      = StatusCB;    /*!< Function to call, after the card has passed several stages (usually needed on RAM based
                                        devices, that change DPM configuration during initialization) */
  /* Append any custom parameter */
  ptSPIDev->userparam   = ptSPIParam; /*!< Use this parameter to pass SPI interface specific information */

  /* There is no extra memory */
  ptSPIDev->extmem      = NULL;   /*!< virtual pointer to extended memory  */
  ptSPIDev->extmemaddr  = 0x00;   /*!< physical address to extended memory */
  ptSPIDev->extmemlen   = 0;      /*!< Length of extended memory in bytes  */

  /* NOTE: The following parameter are SPI specific */

  /* Hardware access functions, required to read or write to the hardware */
  ptSPIDev->hwif_init   = SPIHWIFInit;   /*!< Function initializes SPI hw-function interface (need to be implemented) */
  ptSPIDev->hwif_deinit = SPIHWIFDeInit; /*!< Function de-initializes SPI hw-function interface (need to be implemented) */
  if (0 != ptSPIParam->ulChunkSize) {
    ptSPIDev->hwif_read   = SPIHWIFRead_ext;   /*!< Function realize DPM read access via SPI transfers (need to be implemented) */
    ptSPIDev->hwif_write  = SPIHWIFWrite_ext;  /*!< Function realize DPM write access via SPI transfers (need to be implemented) */
  } else {
    ptSPIDev->hwif_read   = SPIHWIFRead;   /*!< Function realize DPM read access via SPI transfers (need to be implemented) */
    ptSPIDev->hwif_write  = SPIHWIFWrite;  /*!< Function realize DPM write access via SPI transfers (need to be implemented) */
  }
  /* Allocate buffer for TX SPI-transfers */
  ptSPIParam->pabTXBuffer = malloc(2+DEFAULT_BUFFER_SIZE+3); /* Length field (required by SPIWriteChunk) and SPI header are included */
  if(ptSPIParam->pabTXBuffer == NULL) {
    fprintf(stderr, "SDPMInit: Allocate memory for the TX buffer\n");
    goto error_out;
  }
  *(uint16_t*)ptSPIParam->pabTXBuffer = DEFAULT_BUFFER_SIZE; /* store buffer length */

  /* Allocate buffer for RX SPI-transfers */
  ptSPIParam->pabRXBuffer = malloc(2+DEFAULT_BUFFER_SIZE+4); /* Length field (required by SPIReadChunk) and SPI header are included */
  if(ptSPIParam->pabRXBuffer == NULL) {
    fprintf(stderr, "SDPMInit: Allocate memory for the RX buffer\n");
    goto error_out;
  }
  *(uint16_t*)ptSPIParam->pabRXBuffer = DEFAULT_BUFFER_SIZE; /* store buffer length */

  DBG("--SDPMInit\n");
  return ptSPIDev;

error_out:
  if (ptSPIParam) {
    free(ptSPIParam->pabTXBuffer);
    free(ptSPIParam->pabRXBuffer);
  }
  free(ptSPIParam);
  free(ptSPIDev);
  close(lFd);
  DBG("--SDPMInit\n");
  return NULL;
}

