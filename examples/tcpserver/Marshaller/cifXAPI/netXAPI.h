/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: netXAPI.h 14572 2018-10-15 11:24:27Z Robert $:

  Description:
    Global netX API definition for netX drivers

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2019-10-10  - Added type cast to error definitions for 64Bit systems
    2018-11-06  - Added function pointer PFN_SYSDEVICERESETEX for new function xSysdeviceResetEx()
    2018-10-12  - Changed file header to actual version
                - Using new Hilscher headers and definitions
    2015-07-24  - Reviewed consolidated from several sources
    2013-06-24  - Added APIENTRY to NXAPI callback prototypes
    2011-01-11  - Updated data types to ISOC99
                - Additional functions for netXTransport added
    2009-05-04  - Declaration for nxDrvGetConfig() added
    2008-07-08  - xSysdevice/xChannel download included
    2008-02-19  - New error number included
    2008-02-01  - NXDRV_DEVICE_INFORMATION structure extended by
                  complete system information block
                - New error number included
    2007-11-09  created

**************************************************************************************/

/* prevent multiple inclusion */
#ifndef __NETxAPI_H
#define __NETxAPI_H

#pragma once

#include "cifXUser.h"

#ifdef __cplusplus
  extern "C" {
#endif  /* _cplusplus */


/*****************************************************************************
*! netX API Error Definition
*****************************************************************************/
#define NXAPI_NO_ERROR                      ((int32_t)0x00000000L)

#define NXAPI_NOT_INITIALIZED               ((int32_t)0x00000010L)
#define NXAPI_NO_WORKING_DIRECTORY          ((int32_t)0x00000011L)
#define NXAPI_NO_ENTRIES                    ((int32_t)0x00000012L)
#define NXAPI_NO_ENTRY_FOUND                ((int32_t)0x00000013L)
#define NXAPI_UNKOWN_COMMAND                ((int32_t)0x00000014L)
#define NXAPI_INVALID_POINTER               ((int32_t)0x00000015L)
#define NXAPI_INVALID_PARAMETER             ((int32_t)0x00000016L)
#define NXAPI_BUFFER_TOO_SHORT              ((int32_t)0x00000017L)

#define NXAPI_DRIVER_NOT_INITIALIZED        ((int32_t)0x00000020L)
#define NXAPI_DRIVER_DLL_NOT_LOADED         ((int32_t)0x00000021L)
#define NXAPI_DRIVER_INTERFACE_MISSING      ((int32_t)0x00000022L)
#define NXAPI_DRIVER_FUNCTION_MISSING       ((int32_t)0x00000023L)
#define NXAPI_DRIVER_NO_DEVICE_FOUND        ((int32_t)0x00000024L)
#define NXAPI_DRIVER_DOWNLOAD_FAILED        ((int32_t)0x00000025L)
#define NXAPI_DRIVER_INVALID_COMMAND        ((int32_t)0x00000026L)

#define NXAPI_DRIVER_DIRECTORY_CREATE       ((int32_t)0x00000030L)
#define NXAPI_DRIVER_FILE_CREATE            ((int32_t)0x00000031L)
#define NXAPI_DRIVER_FILE_READ              ((int32_t)0x00000032L)
#define NXAPI_DRIVER_FILE_WRITE             ((int32_t)0x00000032L)
#define NXAPI_DRIVER_FILE_DELETE            ((int32_t)0x00000032L)

/* Registry errors */
#define NXAPI_DRIVER_REG_OPEN               ((int32_t)0x00000040L)
#define NXAPI_DRIVER_REG_CLOSE              ((int32_t)0x00000041L)

#define NXAPI_DRIVER_REG_ENUM_KEY           ((int32_t)0x00000043L)
#define NXAPI_DRIVER_REG_CREATE_KEY         ((int32_t)0x00000044L)
#define NXAPI_DRIVER_REG_DELETE_KEY         ((int32_t)0x00000045L)
#define NXAPI_DRIVER_REG_READ_KEY           ((int32_t)0x00000046L)

#define NXAPI_DRIVER_REG_ENUM_VALUE         ((int32_t)0x00000047L)
#define NXAPI_DRIVER_REG_CREATE_VALUE       ((int32_t)0x00000048L)
#define NXAPI_DRIVER_REG_DELETE_VALUE       ((int32_t)0x00000049L)
#define NXAPI_DRIVER_REG_READ_VALUE         ((int32_t)0x0000004AL)
#define NXAPI_DRIVER_REG_WRITE_VALUE        ((int32_t)0x0000004BL)



/*****************************************************************************
****  nxAPI Download Command definitions                                  ****
*****************************************************************************/
#define NXAPI_CMD_FIRMWARE                  0x00000001L
#define NXAPI_CMD_CONFIGURATION             0x00000002L
#define NXAPI_CMD_WARMSTART                 0x00000003L
#define NXAPI_CMD_BOOTLOADER                0x00000004L

/*****************************************************************************
****  nxAPI Configuration Command definitions                             ****
*****************************************************************************/
#define NXAPI_CMD_READ_DRIVER_CFG           0x00000001L
#define NXAPI_CMD_WRITE_DRIVER_CFG          0x00000002L
#define NXAPI_CMD_READ_CHANNEL_CFG          0x00000003L
#define NXAPI_CMD_WRITE_CHANNEL_CFG         0x00000004L

#define NXAPI_CMD_READ_FILE_CFG             0x00000005L
#define NXAPI_CMD_DELETE_FW_FILE            0x00000006L
#define NXAPI_CMD_DELETE_CFG_FILE           0x00000007L
#define NXAPI_CMD_DELETE_ALL_FILES          0x00000008L

#define NXAPI_CFG_DATATYPE_STRING           0x00000001L   /* Null terminated string */
#define NXAPI_CFG_DATATYPE_BINARY           0x00000003L
#define NXAPI_CFG_DATATYPE_DWORD            0x00000004L


/*****************************************************************************
****  INCLUDE FILES AND CONSTANT DEFINITIONS                              ****
*****************************************************************************/
#define NXDRV_NAME_LENGTH                   64
#define NXDRV_VERSION_LENGTH                64

/* Global hardware driver type */
#define NXDRV_TYPE_DPM                      0x00000001
#define NXDRV_TYPE_USB                      0x00000002
#define NXDRV_TYPE_SERIAL                   0x00000004
#define NXDRV_TYPE_ETH                      0x00000008

/* Global driver Requirements */
#define NXDRV_REQ_STARTUP_SW                0x00000001

/* Command Definitions */
#define NXDRV_FIND_FIRST                    1
#define NXDRV_FIND_NEXT                     2

/* Command definitions of Extended Name */
#define NXCON_GET_FULL_NAME                      1
#define NXCON_GET_SHORT_NAME                     2

/* defines of the nxCon functions */
#define NXCON_MAX_LENGTH_CONNECTOR_IDENTIFIER    6
#define NXCON_UUID_STRING_SIZE                  37
#define NXCON_FILE_NAME_LENGTH                 256
#define NXCON_DESCRIPTION_LENGTH                64

/*****************************************************************************/
/*! typedef struct NXDRV_HW_INFORMATION
** Structure contains information about available drivers                    */
/*****************************************************************************/
typedef struct tagNXDRV_HW_INFORMATION
{
  char                  szDriverName[NXDRV_NAME_LENGTH];            /*!< Name of the driver     */
  char                  szVersion[NXDRV_NAME_LENGTH];               /*!< Driver version         */
  uint32_t              ulDriverType;                               /*!< Driver type            */
  uint32_t              ulDriverRequirements;                       /*!< Driver requirements    */
  uint32_t              ulDeviceClass;                              /*!< Supported device class */
} NXDRV_HW_INFORMATION, *PNXDRV_HW_INFORMATION;

/*****************************************************************************/
/*! typedef struct NXDRV_DEVICE_INFORMATION
** Structure contains the drivers device information.                        */
/*****************************************************************************/
typedef struct tagNXDRV_DEVICE_INFORMATION
{
  CIFXHANDLE                        hDevice;                        /*!< Device handle            */
  char                              szDeviceName[NXDRV_NAME_LENGTH];/*!< Device name              */
  SYSTEM_CHANNEL_SYSTEM_INFO_BLOCK  tSystemInfoBlock;               /*!< Device System Info Block */
} NXDRV_DEVICE_INFORMATION, *PNXDRV_DEVICE_INFORMATION;

/*****************************************************************************/
/*! typedef struct NXDRV_CONFIG_INFORMATION
** Structure contains the driver configuration information.                  */
/*****************************************************************************/
typedef struct tagNXDRV_DRIVER_CFG_DATA_INFO
{
  char                  szValueName[NXDRV_NAME_LENGTH];             /*!< Value name  */
  uint32_t              ulValueIndex;                               /*!< Value index */
  uint32_t              ulValueType;                                /*!< Value type  */
  uint32_t              ulValueSize;                                /*!< Value size  */
} NXDRV_DRIVER_CFG_DATA_INFO, *PNXDRV_DRIVER_CFG_DATA_INFO;


typedef int32_t(APIENTRY *PFN_NXDRV_INFO) (uint32_t ulSize, void* pvInfo);
typedef void(APIENTRY *PFN_NXAPI_PROGRESS_CALLBACK)(uint32_t ulStep,  uint32_t ulMaxStep, void* pvUser, char bFinished, int32_t lError);
typedef void(APIENTRY *PFN_NXAPI_BROWSE_CALLBACK)  (uint32_t  ulBoard,BOARD_INFORMATION* ptBoardInfo, uint32_t ulStep, uint32_t ulMaxStep, void* pvUser, char bFinished, int32_t lError);

/*****************************************************************************/
/*! netX API driver functions                                                */
/*****************************************************************************/
int32_t APIENTRY nxDrvInit            ( void);
int32_t APIENTRY nxDrvExit            ( void);
int32_t APIENTRY nxDrvGetInformation  ( uint32_t              ulSize,
                                        PNXDRV_HW_INFORMATION ptDrvInfo);

int32_t APIENTRY nxDrvFindDevice      ( uint32_t                  ulCmd,        uint32_t  ulInfoSize,
                                        NXDRV_DEVICE_INFORMATION* ptDeviceInfo, uint32_t* pulSearchIdx);

int32_t APIENTRY nxDrvBrowseDevices   ( PFN_NXAPI_BROWSE_CALLBACK pfnCallback, void* pvUser);

int32_t APIENTRY nxDrvDownload        ( CIFXHANDLE    hDevice,      uint32_t  ulChannel,    uint32_t       ulCmd,
                                        uint32_t      ulFileSize,   char*     pszFileName,  unsigned char* pabFileData,
                                        void*         pvUser,       PFN_NXAPI_PROGRESS_CALLBACK pfnCallback);

int32_t APIENTRY nxDrvStart           ( CIFXHANDLE hDevice, uint32_t ulChannel);
int32_t APIENTRY nxDrvStartEx         ( CIFXHANDLE hDevice, uint32_t ulChannel, uint32_t ulResetTimeout, uint32_t ulMode);

int32_t APIENTRY nxDrvGetConfigInfo   ( CIFXHANDLE hDevice, uint32_t ulCmd, int32_t lChannel, uint32_t ulSearchIndex, uint32_t ulBufferSize, NXDRV_DRIVER_CFG_DATA_INFO* ptCfgData);
int32_t APIENTRY nxDrvGetConfig       ( CIFXHANDLE hDevice, uint32_t ulCmd, int32_t lChannel, NXDRV_DRIVER_CFG_DATA_INFO* ptCfgData, uint32_t ulBufferSize, void* pvData);
int32_t APIENTRY nxDrvSetConfig       ( CIFXHANDLE hDevice, uint32_t ulCmd, int32_t lChannel, NXDRV_DRIVER_CFG_DATA_INFO* ptCfgData, uint32_t ulBufferSize, void* pvData);
int32_t APIENTRY nxDrvDeleteConfig    ( CIFXHANDLE hDevice, uint32_t ulCmd, int32_t lChannel, NXDRV_DRIVER_CFG_DATA_INFO* ptCfgData);

/***************************************************************************
* Extension of nxAPI ( Connector extension )
***************************************************************************/
int32_t APIENTRY nxConGetConfig          ( char* szUUID, uint32_t* pulConfigSize, char* pcConfig);
int32_t APIENTRY nxConSetConfig          ( char* szUUID, char*     pcConfig);
int32_t APIENTRY nxConCreateConfigDialog ( char* szUUID, void*     pvParentWnd, void** pvDialogWnd);
int32_t APIENTRY nxConCloseConfigDialog  ( char* szUUID, int32_t   fSaveChanges);
int32_t APIENTRY nxConEnumerate          ( uint32_t ulConnectorIdx, uint32_t ulSize,  void* pvConnectorInfo);
int32_t APIENTRY nxConGetCorrespondName  ( char*    szSourceName,   uint32_t ulCmd,   uint32_t ulCorrespondSize, char* szCorrespondName);

/***************************************************************************
* CIFX Device Driver API Functions
***************************************************************************/
/* Global driver functions */
typedef int32_t (APIENTRY *PFN_DRVOPEN)                   ( CIFXHANDLE* phDriver);
typedef int32_t (APIENTRY *PFN_DRVCLOSE)                  ( CIFXHANDLE  hDriver);
typedef int32_t (APIENTRY *PFN_DRVGETINFORMATION)         ( CIFXHANDLE  hDriver,  uint32_t ulSize,   void* pvDriverInfo);
typedef int32_t (APIENTRY *PFN_DRVGETERRORDESCRIPTION)    ( int32_t     lError,   char*    szBuffer, uint32_t ulBufferLen);
typedef int32_t (APIENTRY *PFN_DRVENUMBOARDS)             ( CIFXHANDLE  hDriver,  uint32_t ulBoard,  uint32_t ulSize,    void*    pvBoardInfo);
typedef int32_t (APIENTRY *PFN_DRVENUMCHANNELS)           ( CIFXHANDLE  hDriver,  uint32_t ulBoard,  uint32_t ulChannel, uint32_t ulSize, void* pvChannelInfo);

/* System device depending functions */
typedef int32_t (APIENTRY *PFN_SYSDEVICEOPEN)             ( CIFXHANDLE  hDriver, char*   szBoard, CIFXHANDLE* phSysdevice);
typedef int32_t (APIENTRY *PFN_SYSDEVICECLOSE)            ( CIFXHANDLE  hSysdevice);
typedef int32_t (APIENTRY *PFN_SYSDEVICEGETMBXSTATE)      ( CIFXHANDLE  hSysdevice, uint32_t* pulRecvPktCount, uint32_t* pulSendPktCount);
typedef int32_t (APIENTRY *PFN_SYSDEVICEPUTPACKET)        ( CIFXHANDLE  hSysdevice, CIFX_PACKET*  ptSendPkt, uint32_t ulTimeout);
typedef int32_t (APIENTRY *PFN_SYSDEVICEGETPACKET)        ( CIFXHANDLE  hSysdevice, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout);
typedef int32_t (APIENTRY *PFN_SYSDEVICEINFO)             ( CIFXHANDLE  hSysdevice, uint32_t ulCmd, uint32_t ulSize, void* pvInfo);

typedef int32_t (APIENTRY *PFN_SYSDEVICEFINDFIRSTFILE)    ( CIFXHANDLE  hSysdevice, uint32_t ulChannel, CIFX_DIRECTORYENTRY* ptDirectoryInfo, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);
typedef int32_t (APIENTRY *PFN_SYSDEVICEFINDNEXTFILE)     ( CIFXHANDLE  hSysdevice, uint32_t ulChannel, CIFX_DIRECTORYENTRY* ptDirectoryInfo, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);

typedef int32_t (APIENTRY *PFN_SYSDEVICEDOWNLOAD)         ( CIFXHANDLE  hSysdevice, uint32_t ulChannel, uint32_t ulMode, char* pszFileName, unsigned char* pabFileData, uint32_t ulFileSize,
                                                            PFN_PROGRESS_CALLBACK pfnCallback, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);
typedef int32_t (APIENTRY *PFN_SYSDEVICEUPLOAD)           ( CIFXHANDLE  hSysdevice, uint32_t ulChannel, uint32_t ulMode, char* pszFileName, unsigned char* pabFileData, uint32_t* pulFileSize,
                                                            PFN_PROGRESS_CALLBACK pfnCallback, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);

typedef int32_t (APIENTRY *PFN_SYSDEVICERESET)            ( CIFXHANDLE  hSysdevice, uint32_t ulTimeout);
typedef int32_t (APIENTRY *PFN_SYSDEVICERESETEX)          ( CIFXHANDLE  hSysdevice, uint32_t ulTimeout, uint32_t ulMode);

typedef int32_t (APIENTRY *PFN_CHANNELOPEN)               ( CIFXHANDLE  hDriver,  char* szBoard, uint32_t ulChannel, CIFXHANDLE* phChannel);
typedef int32_t (APIENTRY *PFN_CHANNELCLOSE)              ( CIFXHANDLE  hChannel);

typedef int32_t (APIENTRY *PFN_CHANNELFINDFIRSTFILE)      ( CIFXHANDLE  hChannel, CIFX_DIRECTORYENTRY* ptDirectoryInfo, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);
typedef int32_t (APIENTRY *PFN_CHANNELFINDNEXTFILE)       ( CIFXHANDLE  hChannel, CIFX_DIRECTORYENTRY* ptDirectoryInfo, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);

typedef int32_t (APIENTRY *PFN_CHANNELDOWNLOAD)           ( CIFXHANDLE  hChannel, uint32_t ulMode, char* pszFileName, unsigned char* pabFileData, uint32_t ulFileSize,
                                                            PFN_PROGRESS_CALLBACK pfnCallback, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);
typedef int32_t (APIENTRY *PFN_CHANNELUPLOAD)             ( CIFXHANDLE  hChannel, uint32_t ulMode, char* pszFileName, unsigned char* pabFileData, uint32_t* pulFileSize,
                                                            PFN_PROGRESS_CALLBACK pfnCallback, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);

typedef int32_t (APIENTRY *PFN_CHANNELGETMBXSTATE)        ( CIFXHANDLE  hChannel, uint32_t* pulRecvPktCount, uint32_t* pulSendPktCount);
typedef int32_t (APIENTRY *PFN_CHANNELPUTPACKET)          ( CIFXHANDLE  hChannel, CIFX_PACKET*  ptSendPkt, uint32_t ulTimeout);
typedef int32_t (APIENTRY *PFN_CHANNELGETPACKET)          ( CIFXHANDLE  hChannel, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout);

typedef int32_t (APIENTRY *PFN_CHANNELCONTROLBLOCK)       ( CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData);
typedef int32_t (APIENTRY *PFN_CHANNELCOMMONSTATUSBLOCK)  ( CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData);
typedef int32_t (APIENTRY *PFN_CHANNELEXTENDEDSTATUSBLOCK)( CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData);
typedef int32_t (APIENTRY *PFN_CHANNELUSERBLOCK)          ( CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData);

typedef int32_t (APIENTRY *PFN_CHANNELCONFIGLOCK)         ( CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout);
typedef int32_t (APIENTRY *PFN_CHANNELRESET)              ( CIFXHANDLE  hChannel, uint32_t ulResetMode, uint32_t ulTimeout);
typedef int32_t (APIENTRY *PFN_CHANNELINFO)               ( CIFXHANDLE  hChannel, uint32_t ulSize, void* pvChannelInfo);
typedef int32_t (APIENTRY *PFN_CHANNELHOSTSTATE)          ( CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout);
typedef int32_t (APIENTRY *PFN_CHANNELBUSSTATE)           ( CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout);
typedef int32_t (APIENTRY *PFN_CHANNELIOINFO)             ( CIFXHANDLE  hChannel, uint32_t ulCmd,        uint32_t ulAreaNumber, uint32_t ulSize, void* pvData);
typedef int32_t (APIENTRY *PFN_CHANNELIOREAD)             ( CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulOffset,     uint32_t ulDataLen, void* pvData, uint32_t ulTimeout);
typedef int32_t (APIENTRY *PFN_CHANNELIOWRITE)            ( CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulOffset,     uint32_t ulDataLen, void* pvData, uint32_t ulTimeout);


typedef struct NXDRV_FUNCTION_TABLEtag
{
  /* The cifX API interface */
  PFN_DRVOPEN                     pfnDriverOpen;                                /*!< xDriverOpen                    */
  PFN_DRVCLOSE                    pfnDriverClose;
  PFN_DRVGETINFORMATION           pfnDriverGetInformation;
  PFN_DRVGETERRORDESCRIPTION      pfnDriverGetErrorDescription;
  PFN_DRVENUMBOARDS               pfnDriverEnumBoards;
  PFN_DRVENUMCHANNELS             pfnDriverEnumChannels;

  /* System channel functions */
  PFN_SYSDEVICEOPEN               pfnSysdeviceOpen;
  PFN_SYSDEVICECLOSE              pfnSysdeviceClose;
  PFN_SYSDEVICEGETMBXSTATE        pfnSysdeviceGetMBXState;
  PFN_SYSDEVICEPUTPACKET          pfnSysdevicePutPacket;
  PFN_SYSDEVICEGETPACKET          pfnSysdeviceGetPacket;
  PFN_SYSDEVICEINFO               pfnSysdeviceInfo;
  PFN_SYSDEVICEFINDFIRSTFILE      pfnSysdeviceFindFirstFile;
  PFN_SYSDEVICEFINDNEXTFILE       pfnSysdeviceFindNextFile;

  PFN_SYSDEVICEDOWNLOAD           pfnSysdeviceDownload;
  PFN_SYSDEVICEUPLOAD             pfnSysdeviceUpload;

  PFN_SYSDEVICERESET              pfnSysdeviceReset;
  PFN_SYSDEVICERESETEX            pfnSysdeviceResetEx;

  /* Communication channel functions */
  PFN_CHANNELOPEN                 pfnChannelOpen;
  PFN_CHANNELCLOSE                pfnChannelClose;
  PFN_CHANNELFINDFIRSTFILE        pfnChannelFindFirstFile;
  PFN_CHANNELFINDNEXTFILE         pfnChannelFindNextFile;

  PFN_CHANNELDOWNLOAD             pfnChannelDownload;
  PFN_CHANNELUPLOAD               pfnChannelUpload;

  PFN_CHANNELGETMBXSTATE          pfnChannelGetMBXState;
  PFN_CHANNELPUTPACKET            pfnChannelPutPacket;
  PFN_CHANNELGETPACKET            pfnChannelGetPacket;

  PFN_CHANNELCONTROLBLOCK         pfnChannelControlBlock;
  PFN_CHANNELCOMMONSTATUSBLOCK    pfnChannelCommonStatusBlock;
  PFN_CHANNELEXTENDEDSTATUSBLOCK  pfnChannelExtendedStatusBlock;
/*  PFN_CHANNELUSERBLOCK            pfnChannelUserBlock; */

  PFN_CHANNELCONFIGLOCK           pfnChannelConfigLock;
  PFN_CHANNELRESET                pfnChannelReset;
  PFN_CHANNELINFO                 pfnChannelInfo;
  PFN_CHANNELHOSTSTATE            pfnChannelHostState;
  PFN_CHANNELBUSSTATE             pfnChannelBusState;
  PFN_CHANNELIOINFO               pfnChannelIOInfo;
  PFN_CHANNELIOREAD               pfnChannelIORead;
  PFN_CHANNELIOWRITE              pfnChannelIOWrite;

} NXDRV_FUNCTION_TABLE, *PNXDRV_FUNCTION_TABLE;

/*****************************************************************************/
/*! typedef struct NXDRV_INFORMATION
** Structure contains the cifX driver information about available driver DLLs*/
/*****************************************************************************/
typedef struct tagNXDRV_INFORMATION
{
  CIFXHANDLE            hDriver;
  int32_t               lError;
  NXDRV_FUNCTION_TABLE* ptFnc;
} NXDRV_INFORMATION, *PNXDRV_INFORMATION;

/***************************************************************************
* Extension of nxAPI ( Connector extension )
***************************************************************************/

/*****************************************************************************/
/*! typedef struct NXCON_CONNECTOR_INFO_T
** Structure contains the connector information of a available connector     */
/*****************************************************************************/
typedef struct NXCON_CONNECTOR_INFO_Ttag
{
  char          szConnectorUUID [NXCON_UUID_STRING_SIZE];                 /*!< UUID of the selected connector             */
  char          szIdentifier    [NXCON_MAX_LENGTH_CONNECTOR_IDENTIFIER];  /*!< Identifier of the connector                */
  char          szFileName      [NXCON_FILE_NAME_LENGTH];                 /*!< File name of the connector                 */
  char          szFullFileName  [NXCON_FILE_NAME_LENGTH];                 /*!< Full file name of the connector            */
  char          szDescription   [NXCON_DESCRIPTION_LENGTH];               /*!< Description of the connector               */
  uint32_t      ulConnectorType;                                          /*!< Supported types of the selected connector  */
} NXCON_CONNECTOR_INFO_T, *PNXCON_CONNECTOR_INFO_T;

/***************************************************************************/

#ifdef __cplusplus
}
#endif

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/

#endif  /* __NETxAPI_H */
