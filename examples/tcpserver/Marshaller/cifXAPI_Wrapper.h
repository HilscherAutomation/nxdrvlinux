/**************************************************************************************
 
   Copyright (c) Hilscher GmbH. All Rights Reserved.
 
 **************************************************************************************
 
   Filename:
    $Workfile: OS_CifXModul.h $
   Last Modification:
    $Author: AlexanderMinor $
    $Modtime: $
    $Revision: 12813 $
   
   Targets:
     Win32/ANSI   : yes
     Win32/Unicode: yes (define _UNICODE)
     WinCE        : yes
 
   Description:
    cifX function pointers for cifX marshalling module
       
   Changes:
 
     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
     3         26.09.2013  SS       Added support for xDriverRestartDevice call
     2         25.06.2010  SD       Change:
                                     - changed types of driver functions (for 64-bit support)
     1         25.05.2009  PL       intitial version
 
**************************************************************************************/

#ifndef __CIFXAPI_WRAPPPER__H
#define __CIFXAPI_WRAPPPER__H

/*****************************************************************************/
/*! \file cifXAPI_Wrapper.h
*   cifX function pointers for cifX marshalling module                       */
/*****************************************************************************/

#ifdef __cplusplus
  extern "C" {
#endif

#include "cifXUser.h"

/*****************************************************************************/
/*! \addtogroup NETX_MARSHALLER_CIFX
*   \{                                                                       */
/*****************************************************************************/
/* Global driver functions */
typedef int32_t(APIENTRY *PFN_xDriverOpen                 )( CIFXHANDLE* phDriver);
typedef int32_t(APIENTRY *PFN_xDriverClose                )( CIFXHANDLE  hDriver);
typedef int32_t(APIENTRY *PFN_xDriverGetInformation       )( CIFXHANDLE  hDriver, uint32_t ulSize, void* pvDriverInfo);
typedef int32_t(APIENTRY *PFN_xDriverGetErrorDescription  )( int32_t        lError, char* szBuffer, uint32_t ulBufferLen);
typedef int32_t(APIENTRY *PFN_xDriverEnumBoards           )( CIFXHANDLE  hDriver, uint32_t ulBoard, uint32_t ulSize, void* pvBoardInfo);
typedef int32_t(APIENTRY *PFN_xDriverEnumChannels         )( CIFXHANDLE  hDriver, uint32_t ulBoard, uint32_t ulChannel, uint32_t ulSize, void* pvChannelInfo);
typedef int32_t(APIENTRY *PFN_xDriverMemoryPointer        )( CIFXHANDLE  hDriver, uint32_t ulBoard, uint32_t ulCmd,void* pvMemoryInfo);
typedef int32_t(APIENTRY *PFN_xDriverRestartDevice        )( CIFXHANDLE  hDriver, char* szBoardName, void* pvData);

/* System device depending functions */
typedef int32_t(APIENTRY *PFN_xSysdeviceOpen              )( CIFXHANDLE  hDriver, char*   szBoard, CIFXHANDLE* phSysdevice);
typedef int32_t(APIENTRY *PFN_xSysdeviceClose             )( CIFXHANDLE  hSysdevice);
typedef int32_t(APIENTRY *PFN_xSysdeviceGetMBXState       )( CIFXHANDLE  hSysdevice, uint32_t* pulRecvPktCount, uint32_t* pulSendPktCount);
typedef int32_t(APIENTRY *PFN_xSysdevicePutPacket         )( CIFXHANDLE  hSysdevice, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout);
typedef int32_t(APIENTRY *PFN_xSysdeviceGetPacket         )( CIFXHANDLE  hSysdevice, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout);
typedef int32_t(APIENTRY *PFN_xSysdeviceInfo              )( CIFXHANDLE  hSysdevice, uint32_t ulCmd, uint32_t ulSize, void* pvInfo);

typedef int32_t(APIENTRY *PFN_xSysdeviceFindFirstFile     )( CIFXHANDLE  hSysdevice, uint32_t ulChannel, CIFX_DIRECTORYENTRY* ptDirectoryInfo, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);
typedef int32_t(APIENTRY *PFN_xSysdeviceFindNextFile      )( CIFXHANDLE  hSysdevice, uint32_t ulChannel, CIFX_DIRECTORYENTRY* ptDirectoryInfo, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);
typedef int32_t(APIENTRY *PFN_xSysdeviceDownload          )( CIFXHANDLE  hSysdevice, uint32_t ulChannel, uint32_t ulMode, char* szFileName, uint8_t* pabFileData, uint32_t ulFileSize, 
                                                                          PFN_PROGRESS_CALLBACK pfnCallback, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);

typedef int32_t(APIENTRY *PFN_xSysdeviceReset             )( CIFXHANDLE  hSysdevice, uint32_t ulTimeout);
typedef int32_t(APIENTRY *PFN_xSysdeviceResetEx           )( CIFXHANDLE  hSysdevice, uint32_t ulTimeout, uint32_t ulMode);

/* Channel depending functions */
typedef int32_t(APIENTRY *PFN_xChannelOpen                )( CIFXHANDLE  hDriver,  char* szBoard, uint32_t ulChannel, CIFXHANDLE* phChannel);
typedef int32_t(APIENTRY *PFN_xChannelClose               )( CIFXHANDLE  hChannel);
typedef int32_t(APIENTRY *PFN_xChannelFindFirstFile       )( CIFXHANDLE  hChannel, CIFX_DIRECTORYENTRY* ptDirectoryInfo, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);
typedef int32_t(APIENTRY *PFN_xChannelFindNextFile        )( CIFXHANDLE  hChannel, CIFX_DIRECTORYENTRY* ptDirectoryInfo, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);

typedef int32_t(APIENTRY *PFN_xChannelDownload            )( CIFXHANDLE  hChannel, uint32_t ulMode, char* szFileName, uint8_t* pabFileData, uint32_t ulFileSize, 
                                                                          PFN_PROGRESS_CALLBACK pfnCallback, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);
                                                                          
typedef int32_t(APIENTRY *PFN_xChannelGetMBXState         )( CIFXHANDLE  hChannel, uint32_t* pulRecvPktCount, uint32_t* pulSendPktCount);
typedef int32_t(APIENTRY *PFN_xChannelPutPacket           )( CIFXHANDLE  hChannel, CIFX_PACKET*  ptSendPkt, uint32_t ulTimeout);
typedef int32_t(APIENTRY *PFN_xChannelGetPacket           )( CIFXHANDLE  hChannel, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout);
typedef int32_t(APIENTRY *PFN_xChannelGetSendPacket       )( CIFXHANDLE  hChannel, uint32_t ulSize, CIFX_PACKET* ptRecvPkt);

typedef int32_t(APIENTRY *PFN_xChannelConfigLock          )( CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout);
typedef int32_t(APIENTRY *PFN_xChannelReset               )( CIFXHANDLE  hChannel, uint32_t ulResetMode, uint32_t ulTimeout);
typedef int32_t(APIENTRY *PFN_xChannelInfo                )( CIFXHANDLE  hChannel, uint32_t ulSize, void* pvChannelInfo);
typedef int32_t(APIENTRY *PFN_xChannelWatchdog            )( CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulTrigger);
typedef int32_t(APIENTRY *PFN_xChannelHostState           )( CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout);
typedef int32_t(APIENTRY *PFN_xChannelBusState            )( CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout);

typedef int32_t(APIENTRY *PFN_xChannelIOInfo              )( CIFXHANDLE  hChannel, uint32_t ulCmd,        uint32_t ulAreaNumber, uint32_t ulSize, void* pvData);
typedef int32_t(APIENTRY *PFN_xChannelIORead              )( CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData, uint32_t ulTimeout);
typedef int32_t(APIENTRY *PFN_xChannelIOWrite             )( CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData, uint32_t ulTimeout);
typedef int32_t(APIENTRY *PFN_xChannelIOReadSendData      )( CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData);

typedef int32_t(APIENTRY *PFN_xChannelControlBlock        )( CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData);
typedef int32_t(APIENTRY *PFN_xChannelCommonStatusBlock   )( CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData);
typedef int32_t(APIENTRY *PFN_xChannelExtendedStatusBlock )( CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData);
typedef int32_t(APIENTRY *PFN_xChannelUserBlock           )( CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData);

typedef int32_t(APIENTRY *PFN_xChannelPLCMemoryPtr        )( CIFXHANDLE  hChannel, uint32_t ulCmd,        void* pvMemoryInfo);
typedef int32_t(APIENTRY *PFN_xChannelPLCIsReadReady      )( CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t * pulReadState);
typedef int32_t(APIENTRY *PFN_xChannelPLCIsWriteReady     )( CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t * pulWriteState);
typedef int32_t(APIENTRY *PFN_xChannelPLCActivateWrite    )( CIFXHANDLE  hChannel, uint32_t ulAreaNumber);
typedef int32_t(APIENTRY *PFN_xChannelPLCActivateRead     )( CIFXHANDLE  hChannel, uint32_t ulAreaNumber);

typedef struct DRIVER_FUNCTIONStag
{
  PFN_xDriverOpen                 pfnxDriverOpen;
  PFN_xDriverClose                pfnxDriverClose;
  PFN_xDriverGetInformation       pfnxDriverGetInformation;
  PFN_xDriverGetErrorDescription  pfnxDriverGetErrorDescription;
  PFN_xDriverEnumBoards           pfnxDriverEnumBoards;
  PFN_xDriverEnumChannels         pfnxDriverEnumChannels;
  PFN_xDriverMemoryPointer        pfnxDriverMemoryPointer;
  PFN_xDriverRestartDevice        pfnxDriverRestartDevice;
  PFN_xSysdeviceOpen              pfnxSysdeviceOpen;
  PFN_xSysdeviceClose             pfnxSysdeviceClose;
  PFN_xSysdeviceReset             pfnxSysdeviceReset;
  PFN_xSysdeviceResetEx           pfnxSysdeviceResetEx;
  PFN_xSysdeviceGetMBXState       pfnxSysdeviceGetMBXState;
  PFN_xSysdevicePutPacket         pfnxSysdevicePutPacket;
  PFN_xSysdeviceGetPacket         pfnxSysdeviceGetPacket;
  PFN_xSysdeviceDownload          pfnxSysdeviceDownload;
  PFN_xSysdeviceInfo              pfnxSysdeviceInfo;
  PFN_xSysdeviceFindFirstFile     pfnxSysdeviceFindFirstFile;
  PFN_xSysdeviceFindNextFile      pfnxSysdeviceFindNextFile;
  PFN_xChannelOpen                pfnxChannelOpen;
  PFN_xChannelClose               pfnxChannelClose;
  PFN_xChannelDownload            pfnxChannelDownload;
  PFN_xChannelGetMBXState         pfnxChannelGetMBXState;
  PFN_xChannelPutPacket           pfnxChannelPutPacket;
  PFN_xChannelGetPacket           pfnxChannelGetPacket;
  PFN_xChannelGetSendPacket       pfnxChannelGetSendPacket;
  PFN_xChannelConfigLock          pfnxChannelConfigLock;
  PFN_xChannelReset               pfnxChannelReset;
  PFN_xChannelInfo                pfnxChannelInfo;
  PFN_xChannelFindFirstFile       pfnxChannelFindFirstFile;
  PFN_xChannelFindNextFile        pfnxChannelFindNextFile;
  PFN_xChannelWatchdog            pfnxChannelWatchdog;
  PFN_xChannelHostState           pfnxChannelHostState;
  PFN_xChannelBusState            pfnxChannelBusState;
  PFN_xChannelIOInfo              pfnxChannelIOInfo;
  PFN_xChannelIORead              pfnxChannelIORead;
  PFN_xChannelIOWrite             pfnxChannelIOWrite;
  PFN_xChannelIOReadSendData      pfnxChannelIOReadSendData;
  PFN_xChannelControlBlock        pfnxChannelControlBlock;
  PFN_xChannelCommonStatusBlock   pfnxChannelCommonStatusBlock;
  PFN_xChannelExtendedStatusBlock pfnxChannelExtendedStatusBlock;
  PFN_xChannelUserBlock           pfnxChannelUserBlock;
  PFN_xChannelPLCMemoryPtr        pfnxChannelPLCMemoryPtr;
  PFN_xChannelPLCIsReadReady      pfnxChannelPLCIsReadReady;
  PFN_xChannelPLCIsWriteReady     pfnxChannelPLCIsWriteReady;
  PFN_xChannelPLCActivateWrite    pfnxChannelPLCActivateWrite;
  PFN_xChannelPLCActivateRead     pfnxChannelPLCActivateRead;
} DRIVER_FUNCTIONS, *PDRIVER_FUNCTIONS;

#ifdef __cplusplus
  }
#endif

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/

#endif /*  __CIFXAPI_WRAPPPER__H */
