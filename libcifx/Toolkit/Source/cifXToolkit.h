/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXToolkit.h 14803 2023-05-10 09:50:40Z RMayer $:

  Description:
    cifX toolkit function declaration.

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2023-04-26  - Moved DEV function definitions to cifXHWFunctions.h
                - Check parameter macros from cifXFunctions.c moved here
    2021-06-14  - Added new user function USER_GetCachedIOBufferMode()
    2018-10-10  - Updated header and definitions to new Hilscher defines
                - Added chip type definitions for netX90/netX4000 (eCHIP_TYPE_NETX90 / eCHIP_TYPE_NETX4000)
                - Derived from cifX Toolkit V1.6.0.0

**************************************************************************************/

/*****************************************************************************/
/*!  \file                                                                   *
*    cifX toolkit function declaration                                       */
/*****************************************************************************/

#ifndef CIFX_TOOLKIT__H
#define CIFX_TOOLKIT__H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cifXHWFunctions.h"
#include "Hil_FileHeaderV3.h"

#ifndef min
  #define min(a,b)  ((a > b)? b : a)
#endif

/*****************************************************************************/
/*! \addtogroup CIFX_TK_GLOBAL_API Toolkit global API functions              */
/*! \{                                                                       */
/*****************************************************************************/

#define   TOOLKIT_VERSION      "cifX Toolkit 2.8.0.1"

/* Toolkit Global Functions */
int32_t   cifXTKitInit         (void);
void      cifXTKitDeinit       (void);
int32_t   cifXTKitAddDevice    (PDEVICEINSTANCE ptDevInstance);
int32_t   cifXTKitRemoveDevice (char* szBoard, int fForceRemove);

void      cifXTKitDisableHWInterrupt(PDEVICEINSTANCE ptDevInstance);
void      cifXTKitEnableHWInterrupt(PDEVICEINSTANCE ptDevInstance);

void      cifXTKitCyclicTimer  (void);

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/

/*****************************************************************************/
/*! \addtogroup CIFX_TK_STRUCTURE Toolkit Structure Definitions
*   \{                                                                       */
/*****************************************************************************/
typedef struct DEVICE_CHANNEL_DATAtag
{
  int       fModuleLoaded;                          /* Module loaded */
  int       fCNFLoaded;                             /* CNF file loaded */
  char      szFileName[16];                         /* Module short file name 8.3 */
  uint32_t  ulFileSize;
} DEVICE_CHANNEL_DATA;

typedef struct DEVICE_CHANNEL_CONFIGtag
{
  int                 fFWLoaded;                    /* FW file loaded */
  DEVICE_CHANNEL_DATA atChannelData[CIFX_MAX_NUMBER_OF_CHANNELS];
} DEVICE_CHANNEL_CONFIG, *PDEVICE_CHANNEL_CONFIG;

#define CIFXTKIT_DOWNLOAD_NONE      0x00 /*!< Set when file download was skipped. Only valid if CIFX_NO_ERROR is returned */
#define CIFXTKIT_DOWNLOAD_FIRMWARE  0x01 /*!< Successfully downloaded a firmware */
#define CIFXTKIT_DOWNLOAD_MODULE    0x02 /*!< Successfully downloaded a firmware */
#define CIFXTKIT_DOWNLOAD_EXECUTED  0x80 /*!< Download was executed */

/*****************************************************************************/
/*! Global driver information structure used internally in the toolkit       */
/*****************************************************************************/
typedef struct TKIT_DRIVER_INFORMATIONtag
{
  uint32_t ulOpenCount;  /*!< Number of xDriverOpen calls */
  int      fInitialized; /*!< !=1 if the toolkit was initialized successfully */

} TKIT_DRIVER_INFORMATION;

/*****************************************************************************/
/*! Structure passed to USER implemented function, for reading device        *
* specific configuration options                                             */
/*****************************************************************************/
typedef struct CIFX_DEVICE_INFORMATIONtag
{
  uint32_t   ulDeviceNumber;   /*!< Device number of the cifX card */
  uint32_t   ulSerialNumber;   /*!< Serial number                  */
  uint32_t   ulChannel;        /*!< Channel number (0..6)          */
  PDEVICEINSTANCE ptDeviceInstance; /*!< Pointer to device instance     */

} CIFX_DEVICE_INFORMATION, *PCIFX_DEVICE_INFORMATION;

/*****************************************************************************/
/*! Structure passed to USER implemented function, for getting device        *
*   specific configuration files                                             */
/*****************************************************************************/
typedef struct CIFX_FILE_INFORMATIONtag
{
  char  szShortFileName[16];                        /*!< Short filename (8.3) of the file       */
  char  szFullFileName[CIFX_MAX_FILE_NAME_LENGTH];  /*!< Full filename (including path) to file */
} CIFX_FILE_INFORMATION, *PCIFX_FILE_INFORMATION;

#ifdef CIFX_TOOLKIT_PARAMETER_CHECK
  #define CHECK_POINTER(param) if ((void*)NULL == param) return CIFX_INVALID_POINTER;
  #define CHECK_DRIVERHANDLE(handle) if (&g_tDriverInfo != handle) return CIFX_INVALID_HANDLE;
  #define CHECK_SYSDEVICEHANDLE(handle) if (CIFX_NO_ERROR != CheckSysdeviceHandle(handle)) return CIFX_INVALID_HANDLE;
  #define CHECK_CHANNELHANDLE(handle) if (CIFX_NO_ERROR != CheckChannelHandle(handle)) return CIFX_INVALID_HANDLE;
#else
  #define CHECK_POINTER(param)
  #define CHECK_DRIVERHANDLE(handle) UNREFERENCED_PARAMETER(handle)
  #define CHECK_SYSDEVICEHANDLE(handle)
  #define CHECK_CHANNELHANDLE(handle)
#endif

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/

/******************************************************************************
* Functions to be implemented by USER                                         *
******************************************************************************/

void      USER_GetBootloaderFile        (PDEVICEINSTANCE ptDevInstance, PCIFX_FILE_INFORMATION ptFileInfo);
int       USER_GetOSFile                (PCIFX_DEVICE_INFORMATION ptDevInfo, PCIFX_FILE_INFORMATION ptFileInfo);

uint32_t  USER_GetFirmwareFileCount     (PCIFX_DEVICE_INFORMATION ptDevInfo);
int       USER_GetFirmwareFile          (PCIFX_DEVICE_INFORMATION ptDevInfo, uint32_t ulIdx, PCIFX_FILE_INFORMATION ptFileInfo);
uint32_t  USER_GetConfigurationFileCount(PCIFX_DEVICE_INFORMATION ptDevInfo);
int       USER_GetConfigurationFile     (PCIFX_DEVICE_INFORMATION ptDevInfo, uint32_t ulIdx, PCIFX_FILE_INFORMATION ptFileInfo);

int       USER_GetWarmstartParameters   (PCIFX_DEVICE_INFORMATION ptDevInfo, CIFX_PACKET* ptPacket);
void      USER_GetAliasName             (PCIFX_DEVICE_INFORMATION ptDevInfo, uint32_t ulMaxLen, char* szAlias);
int       USER_GetInterruptEnable       (PCIFX_DEVICE_INFORMATION ptDevInfo);
int       USER_GetDMAMode               (PCIFX_DEVICE_INFORMATION ptDevInfo);
int       USER_GetCachedIOBufferMode    (PCIFX_DEVICE_INFORMATION ptDevInfo);

void      USER_Trace                    (PDEVICEINSTANCE ptDevInstance, uint32_t ulTraceLevel, const char* szFormat, ...);

extern uint32_t g_ulTraceLevel;


#ifdef __cplusplus
}
#endif

#endif /* CIFX_TOOLKIT__H */
