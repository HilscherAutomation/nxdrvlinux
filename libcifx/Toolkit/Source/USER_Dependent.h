/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: USER_Dependent.h 15171 2025-08-05 08:18:45Z AMinor $:

  Description:
    User dependent function declaration. These function must be implemented by the
    user. They are called internally by the cifX Toolkit.

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-05-26  initial version

**************************************************************************************/

#ifndef USER_DEPENDENT__H
#define USER_DEPENDENT__H

#ifdef __cplusplus
extern "C"
{
#endif

#include "cifXToolkit.h"
#include "cifXUser.h"

/* Trace level definitions */
#define CIFX_TRACE_LEVEL_DEBUG    0x00000001
#define CIFX_TRACE_LEVEL_INFO     0x00000002
#define CIFX_TRACE_LEVEL_WARNING  0x00000004
#define CIFX_TRACE_LEVEL_ERROR    0x00000008

/* Actual trace log level */
extern uint32_t g_ulTraceLevel;

/*****************************************************************************/
/*! Structure passed to USER implemented function, for getting device        *
 *  specific configuration files                                             */
/*****************************************************************************/
typedef struct CIFX_FILE_INFORMATIONtag
{
  char  szShortFileName[16];                        /*!< Short filename (8.3) of the file       */
  char  szFullFileName[CIFX_MAX_FILE_NAME_LENGTH];  /*!< Full filename (including path) to file */
} CIFX_FILE_INFORMATION, * PCIFX_FILE_INFORMATION;

/*****************************************************************************/
/*! Structure passed to USER implemented function, for reading device        *
 *  specific configuration options                                           */
/*****************************************************************************/
typedef struct CIFX_DEVICE_INFORMATIONtag
{
  uint32_t        ulDeviceNumber;   /*!< Device number of the cifX card */
  uint32_t        ulSerialNumber;   /*!< Serial number                  */
  uint32_t        ulChannel;        /*!< Channel number (0..6)          */
  PDEVICEINSTANCE ptDeviceInstance; /*!< Pointer to device instance     */
} CIFX_DEVICE_INFORMATION, *PCIFX_DEVICE_INFORMATION;

/******************************************************************************
 * Functions to be implemented by USER                                        *
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

#ifdef __cplusplus
}
#endif

#endif /* USER_DEPENDENT__H */
