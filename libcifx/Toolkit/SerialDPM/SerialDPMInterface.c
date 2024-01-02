/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: SerialDPMInterface.c 14801 2023-05-10 09:36:54Z RMayer $:

  Description:
    Serial DPM Interface

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2023-05-10  Adapted netX Read/Write function definitions to new ulOption parameter
    2019-08-06  Chip detection loop in SerialDPM_Init() reworked
    2018-08-09  fixed pclint warnings
    2014-08-01  initial version

**************************************************************************************/

/*****************************************************************************/
/*! \file SerialDPMInterface.c
*   Serial DPM protocol implementation                                       */
/*****************************************************************************/

#include "OS_Spi.h"
#include "cifXHWFunctions.h"
#include "SerialDPMInterface.h"
#include "cifXErrors.h"

#define MAX_CNT(array)       (sizeof((array))/sizeof((array)[0]))
#define MIN(a,b)             (((a)<(b))?(a):(b))

#define MAX_TRANSFER_LEN     124
#define CMD_READ_NX50(len)   (0x80 | len)
#define CMD_READ_NX10(len)   ((len > 127)? 0x80:(0x80 | len))
#define CMD_WRITE_NX10(len)  ((len > 127)? 0x00:len)
/*lint -emacro(572, CMD_READ_NX51 ) : Excessive shift value */
#define CMD_READ_NX51(addr)  (0x80 | ((addr>>16)&0xF))
/*lint -emacro(572, CMD_WRITE_NX51 ) : Excessive shift value */
#define CMD_WRITE_NX51(addr) ((addr>>16)&0xF)
#define CMD_LEN_NX51(len)    ((len > 255)? 0x00:len)

#ifndef CIFX_TOOLKIT_HWIF
  #error "CIFX_TOOLKIT_HWIF must be explicitly enabled to support serial DPM!"
#endif

/*****************************************************************************/
/*! Read a number of bytes from SPI interface (netX50 Slave)
*   \param ulOption       0 = memcpy / 1 = single transfer
*   \param pvDevInstance  Device Instance
*   \param pvAddr         Address offset in DPM to read data from
*   \param pvData         Buffer to store read data
*   \param ulLen          Number of bytes to read                            */
/*****************************************************************************/
static void* Read_NX50( uint32_t ulOption, void* pvDevInstance, void* pvAddr, void* pvData, uint32_t ulLen)
{
  DEVICEINSTANCE* ptDevice      = (DEVICEINSTANCE*) pvDevInstance;
  uint8_t         bUnused       = 0x00;
  uint32_t        ulByteTimeout = 100;
  uint32_t        ulDpmAddr     = (uint32_t)pvAddr;
  uint8_t*        pabData       = (uint8_t*)pvData;

  OS_SpiLock(ptDevice->pvOSDependent);
  while (ulLen > 0)
  {
    uint8_t  abSend[3];
    uint32_t ulChunkLen = MIN(MAX_TRANSFER_LEN, ulLen);

    ulLen -= ulChunkLen;

    /* Assemble command */
    abSend[0] = (uint8_t)((ulDpmAddr >> 8) & 0xFF);
    abSend[1] = (uint8_t)((ulDpmAddr >> 0) & 0xFF);
    abSend[2] = (uint8_t)(CMD_READ_NX50(ulChunkLen));

    OS_SpiAssert(pvDevInstance);
    OS_SpiTransfer(pvDevInstance, abSend, NULL, MAX_CNT(abSend));

    do
    {
      if(ulByteTimeout == 0)
      {
          OS_SpiDeassert(pvDevInstance);
          return pvData;
      }
      --ulByteTimeout;

      /* get the idle bytes done */
      OS_SpiTransfer(pvDevInstance, NULL, &bUnused, 1);

    } while((bUnused & 0xFF) != 0xA5);

    while(ulChunkLen--)
    {
      OS_SpiTransfer(pvDevInstance, &bUnused, pabData++, 1);
    }

    OS_SpiDeassert(pvDevInstance);

    ulDpmAddr += MAX_TRANSFER_LEN;
  }
  OS_SpiUnlock(ptDevice->pvOSDependent);

  return pvData;
}

/*****************************************************************************/
/*! Write a number of bytes to SPI interface (netX50 Slave)
*   \param ulOption       0 = memcpy / 1 = single transfer
*   \param pvDevInstance  Device Instance
*   \param pvAddr         Address offset in DPM to write data to
*   \param pvData         Data to write to SPI interface
*   \param ulLen          Number of bytes to write                           */
/*****************************************************************************/
static void* Write_NX50( uint32_t ulOption, void* pvDevInstance, void* pvAddr, void* pvData, uint32_t ulLen)
{
  DEVICEINSTANCE* ptDevice  = (DEVICEINSTANCE*) pvDevInstance;
  uint32_t        ulDpmAddr = (uint32_t)pvAddr;
  uint8_t*        pabData   = (uint8_t*)pvData;

  OS_SpiLock(ptDevice->pvOSDependent);
  while (ulLen > 0)
  {
    uint8_t  abSend[3];
    uint32_t ulChunkLen = MIN(MAX_TRANSFER_LEN, ulLen);

    ulLen -= ulChunkLen;

    /* Assemble command */
    abSend[0] = (uint8_t)((ulDpmAddr >> 8) & 0xFF);
    abSend[1] = (uint8_t)((ulDpmAddr >> 0) & 0xFF);
    abSend[2] = (uint8_t)ulChunkLen;

    OS_SpiAssert(pvDevInstance);
    OS_SpiTransfer(pvDevInstance, abSend, NULL, MAX_CNT(abSend));
    OS_SpiTransfer(pvDevInstance, pabData, NULL, ulChunkLen);
    OS_SpiDeassert(pvDevInstance);

    ulDpmAddr += ulChunkLen;
    pabData   += ulChunkLen;      /*lint !e662 */
  }
  OS_SpiUnlock(ptDevice->pvOSDependent);

  return pvAddr;
}

/*****************************************************************************/
/*! Read a number of bytes from SPI interface (netX500 Slave)
*   \param ulOption       0 = memcpy / 1 = single transfer
*   \param pvDevInstance  Device Instance
*   \param pvAddr         Address offset in DPM to read data from
*   \param pvData         Buffer to store read data
*   \param ulLen          Number of bytes to read                            */
/*****************************************************************************/
static void* Read_NX500( uint32_t ulOption, void* pvDevInstance, void* pvAddr, void* pvData, uint32_t ulLen)
{
  DEVICEINSTANCE* ptDevice      = (DEVICEINSTANCE*) pvDevInstance;
  uint8_t         bUnused       = 0x00;
  uint32_t        ulByteTimeout = 100;
  uint32_t        ulDpmAddr     = (uint32_t)pvAddr;
  uint8_t*        pabData       = (uint8_t*)pvData;
  uint32_t        ulPreLen      = ulDpmAddr&0x3;

  /* Align offset and length */
  ulDpmAddr &= ~0x3;

  OS_SpiLock(ptDevice->pvOSDependent);
  while (ulLen > 0)
  {
    uint8_t  abSend[3];
    uint32_t ulChunkLen   = MIN(MAX_TRANSFER_LEN, ulLen);
    uint32_t ulAlignedLen = (ulChunkLen+3)&~0x3;

    ulLen -= ulChunkLen;

    /* Assemble command */
    abSend[0] = (uint8_t)((ulDpmAddr >> 8) & 0xFF);
    abSend[1] = (uint8_t)((ulDpmAddr >> 0) & 0xFF);
    abSend[2] = (uint8_t)(CMD_READ_NX50(ulAlignedLen));

    /* assert chip select */
    OS_SpiAssert(pvDevInstance);
    OS_SpiTransfer(pvDevInstance, abSend, NULL, MAX_CNT(abSend));

    do
    {
      if(ulByteTimeout == 0)
      {
          OS_SpiDeassert(pvDevInstance);
          return pvData;
      }
      --ulByteTimeout;

      /* get the idle bytes done */
      OS_SpiTransfer(pvDevInstance, NULL, &bUnused, 1);

    } while((bUnused & 0xFF) != 0xA5);

    if (ulPreLen)
    {
      OS_SpiTransfer(pvDevInstance, NULL, NULL, ulPreLen);
      ulPreLen = 0;
    }

    OS_SpiTransfer(pvDevInstance, NULL, pabData, ulChunkLen);

    if (0 != (ulAlignedLen - ulChunkLen))
    {
      OS_SpiTransfer(pvDevInstance, NULL, NULL, ulAlignedLen - ulChunkLen);
    }

    OS_SpiDeassert(pvDevInstance);

    ulDpmAddr += ulChunkLen;
    pabData   += ulChunkLen;      /*lint !e662 */
  }
  OS_SpiUnlock(ptDevice->pvOSDependent);

  return pvData;
}

/*****************************************************************************/
/*! Read and modify number of bytes from SPI interface (netX500 Slave)
*   \param ulOption       0 = memcpy / 1 = single transfer
*   \param pvDevInstance  Device Instance
*   \param pvAddr         Address offset in DPM to read/modify data
*   \param pvData         Data to write to SPI interface
*   \param ulLen          Number of bytes to write                           */
/*****************************************************************************/
static void* ReadModifyWrite_NX500( uint32_t ulOption, void* pvDevInstance, void* pvAddr, void* pvData, uint32_t ulLen)
{
  uint32_t ulDpmAddr = (uint32_t)pvAddr;
  uint8_t* pabData   = (uint8_t*)pvData;
  uint8_t  abRead[4];

  if (ulDpmAddr&0x3)
  {
    uint32_t ulAlignedAddr = ulDpmAddr&~0x3;
    uint8_t* pabRead       = &abRead[ulDpmAddr&0x3];
    uint32_t ulPartLen     = MIN(ulLen, (4 - (ulDpmAddr&0x3)));

    ulLen     -= ulPartLen;
    ulDpmAddr += ulPartLen;

    (void) Read_NX50( 0, pvDevInstance, (void*)ulAlignedAddr, abRead, 4);
    while (ulPartLen--)
    {
      *pabRead++ = *pabData++;
    }
    (void) Write_NX50( 0, pvDevInstance, (void*)ulAlignedAddr, abRead, 4);
  }

  if (ulLen&~0x3)
  {
    uint32_t ulAlignedLen = ulLen&~0x3;
    (void) Write_NX50( 0, pvDevInstance, (void*)ulDpmAddr, pabData, ulAlignedLen);
    pabData   += ulAlignedLen;
    ulDpmAddr += ulAlignedLen;
    ulLen     -= ulAlignedLen;
  }

  if (ulLen&0x3)
  {
    uint8_t* pabRead = &abRead[0];

    (void) Read_NX50( 0, pvDevInstance, (void*)ulDpmAddr, abRead, 4);
    while (ulLen--)
    {
      *pabRead++ = *pabData++;
    }
    (void) Write_NX50( 0, pvDevInstance, (void*)ulDpmAddr, abRead, 4);
  }
  return pvAddr;
}

/*****************************************************************************/
/*! Read a number of bytes from SPI interface (netX10 Slave)
*   \param ulOption       0 = memcpy / 1 = single transfer
*   \param pvDevInstance  Device Instance
*   \param pvAddr         Address offset in DPM to read data from
*   \param pvData         Buffer to store read data
*   \param ulLen          Number of bytes to read                            */
/*****************************************************************************/
static void* Read_NX10( uint32_t ulOption, void* pvDevInstance, void* pvAddr, void* pvData, uint32_t ulLen)
{
  DEVICEINSTANCE* ptDevice = (DEVICEINSTANCE*) pvDevInstance;
  uint8_t         abSend[3];

  /* Assemble command */
  abSend[0] = (uint8_t)(((uint32_t)pvAddr >> 8) & 0xFF);
  abSend[1] = (uint8_t)(((uint32_t)pvAddr >> 0) & 0xFF);
  abSend[2] = (uint8_t)(CMD_READ_NX10(ulLen));

  OS_SpiLock(ptDevice->pvOSDependent);
  OS_SpiAssert(pvDevInstance);
  OS_SpiTransfer(pvDevInstance, abSend, NULL, MAX_CNT(abSend));
  OS_SpiTransfer(pvDevInstance, NULL, (uint8_t*)pvData, ulLen);
  OS_SpiDeassert(pvDevInstance);
  OS_SpiUnlock(ptDevice->pvOSDependent);
  return pvData;
}

/*****************************************************************************/
/*! Write a number of bytes to SPI interface (netX10 Slave)
*   \param ulOption       0 = memcpy / 1 = single transfer
*   \param pvDevInstance  Device Instance
*   \param pvAddr         Address offset in DPM to write data to
*   \param pvData         Data to write to SPI interface
*   \param ulLen          Number of bytes to write                           */
/*****************************************************************************/
static void* Write_NX10( uint32_t ulOption, void* pvDevInstance, void* pvAddr, void* pvData, uint32_t ulLen)
{
  DEVICEINSTANCE* ptDevice = (DEVICEINSTANCE*) pvDevInstance;
  uint8_t         abSend[3];

  /* Assemble command */
  abSend[0] = (uint8_t)(((uint32_t)pvAddr >> 8) & 0xFF);
  abSend[1] = (uint8_t)(((uint32_t)pvAddr >> 0) & 0xFF);
  abSend[2] = (uint8_t)(CMD_WRITE_NX10(ulLen));

  OS_SpiLock(ptDevice->pvOSDependent);
  OS_SpiAssert(pvDevInstance);
  OS_SpiTransfer(pvDevInstance, abSend, NULL, MAX_CNT(abSend));
  OS_SpiTransfer(pvDevInstance, (uint8_t*)pvData, NULL, ulLen);
  OS_SpiDeassert(pvDevInstance);
  OS_SpiUnlock(ptDevice->pvOSDependent);
  return pvAddr;
}

/*****************************************************************************/
/*! Read a number of bytes from SPI interface (netX51 Slave)
*   \param ulOption       0 = memcpy / 1 = single transfer
*   \param pvDevInstance  Device Instance
*   \param pvAddr         Address offset in DPM to read data from
*   \param pvData         Buffer to store read data
*   \param ulLen          Number of bytes to read                            */
/*****************************************************************************/
static void* Read_NX51( uint32_t ulOption, void* pvDevInstance, void* pvAddr, void* pvData, uint32_t ulLen)
{
  DEVICEINSTANCE* ptDevice = (DEVICEINSTANCE*) pvDevInstance;
  uint8_t         abSend[4];

  /* Assemble command */
  abSend[0] = (uint8_t)(CMD_READ_NX51((uint32_t)pvAddr));
  abSend[1] = (uint8_t)(((uint32_t)pvAddr >> 8) & 0xFF);
  abSend[2] = (uint8_t)(((uint32_t)pvAddr >> 0) & 0xFF);
  abSend[3] = (uint8_t)(CMD_LEN_NX51(ulLen));

  OS_SpiLock(ptDevice->pvOSDependent);
  OS_SpiAssert(pvDevInstance);
  OS_SpiTransfer(pvDevInstance, abSend, NULL, MAX_CNT(abSend));
  OS_SpiTransfer(pvDevInstance, NULL, (uint8_t*)pvData, ulLen);
  OS_SpiDeassert(pvDevInstance);
  OS_SpiUnlock(ptDevice->pvOSDependent);
  return pvData;
}

/*****************************************************************************/
/*! Write a number of bytes to SPI interface (netX51 Slave)
*   \param ulOption       0 = memcpy / 1 = single transfer
*   \param pvDevInstance  Device Instance
*   \param pvAddr         Address offset in DPM to write data to
*   \param pvData         Data to write to SPI interface
*   \param ulLen          Number of bytes to write                           */
/*****************************************************************************/
static void* Write_NX51( uint32_t ulOption, void* pvDevInstance, void* pvAddr, void* pvData, uint32_t ulLen)
{
  DEVICEINSTANCE* ptDevice = (DEVICEINSTANCE*) pvDevInstance;
  uint8_t         abSend[3];

  /* Assemble command */
  abSend[0] = (uint8_t)(CMD_WRITE_NX51((uint32_t)pvAddr));
  abSend[1] = (uint8_t)(((uint32_t)pvAddr >> 8) & 0xFF);
  abSend[2] = (uint8_t)(((uint32_t)pvAddr >> 0) & 0xFF);

  OS_SpiLock(ptDevice->pvOSDependent);
  OS_SpiAssert(pvDevInstance);
  OS_SpiTransfer(pvDevInstance, abSend, NULL, MAX_CNT(abSend));
  OS_SpiTransfer(pvDevInstance, (uint8_t*)pvData, NULL, ulLen);
  OS_SpiDeassert(pvDevInstance);
  OS_SpiUnlock(ptDevice->pvOSDependent);
  return pvAddr;
}

/*****************************************************************************/
/*! Initialize serial DPM interface
*   \param ptDevice  Device Instance
*   \return protocol type identifier                                         */
/*****************************************************************************/
int SerialDPM_Init(DEVICEINSTANCE* ptDevice)
{
  uint8_t bUnused;
  int     iSerDpmType   = SERDPM_UNKNOWN;
  int     iDummyReadCnt = 0;

  if (CIFX_NO_ERROR == OS_SpiInit(ptDevice->pvOSDependent))
  {
    uint32_t ulDetect  = 0;
    uint8_t  abSend[]  = {CMD_READ_NX51(0xFF), 0xFF, CMD_READ_NX50(4)};

    OS_SpiLock(ptDevice->pvOSDependent);

    /* ATTENTION: Some of the netX chips needs a DUMMY read before delivering valid data */
    /*            If the first access delivers a SERDPM_UNKNOWN result,                  */
    /*            the loop should be processed again a second time                       */
    do
    {
      /* Execute the SPI chip detection */
      OS_SpiAssert(ptDevice);
      OS_SpiTransfer(ptDevice, abSend, (unsigned char*)&ulDetect, MAX_CNT(abSend));
      OS_SpiDeassert(ptDevice);

      if (0 == ulDetect)
      {
        iSerDpmType = SERDPM_NETX10;
      } else if (0x00FFFFFF == ulDetect)
      {
        iSerDpmType = SERDPM_NETX50;
      } else if (0x64 == (0xFF & ulDetect))
      {
        iSerDpmType = SERDPM_NETX100;
      } else if (0x11 == (0x1F & ulDetect))
      {
        iSerDpmType = SERDPM_NETX51;
      } else
      {
        iSerDpmType = SERDPM_UNKNOWN;
      }

      iDummyReadCnt++;

    } while ((iDummyReadCnt < 2) && (iSerDpmType == SERDPM_UNKNOWN));

    OS_SpiUnlock(ptDevice->pvOSDependent);

    switch (iSerDpmType)
    {
      case SERDPM_NETX100:
        ptDevice->pfnHwIfRead  = Read_NX500;
        ptDevice->pfnHwIfWrite = ReadModifyWrite_NX500;
        break;

      case SERDPM_NETX50:
        ptDevice->pfnHwIfRead  = Read_NX50;
        ptDevice->pfnHwIfWrite = Write_NX50;
        break;
      case SERDPM_NETX10:
        ptDevice->pfnHwIfRead  = Read_NX10;
        ptDevice->pfnHwIfWrite = Write_NX10;
        /* Initialize SPI unit of slave by making 2 dummy reads */
        (void) Read_NX10(0, ptDevice, 0, &bUnused, 1);
        (void) Read_NX10(0, ptDevice, 0, &bUnused, 1);
        break;
      case SERDPM_NETX51:
        ptDevice->pfnHwIfRead  = Read_NX51;
        ptDevice->pfnHwIfWrite = Write_NX51;
        /* Initialize SPI unit of slave by making 2 dummy reads */
        (void) Read_NX51(0, ptDevice, 0, &bUnused, 1);
        (void) Read_NX51(0, ptDevice, 0, &bUnused, 1);
        break;
      default:
        /* Protocol type detection failed */
        break;
    }

    /* This is a SPI connection, not PCI! */
    ptDevice->fPCICard       = 0;
    /* The DPM address must be zero, as we only transfer address offsets via the SPI interface. */
    ptDevice->pbDPM          = NULL;
  }

  return iSerDpmType;
}
