/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXFunctionList.h 15329 2025-11-24 13:34:32Z AMinor $:

  Description:
    cifX toolkit function list.

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-07-11  Created

**************************************************************************************/
#ifndef CIFX_FUNCTION_LIST__H
#define CIFX_FUNCTION_LIST__H

#include "cifXUser.h"
#include "cifXHwif.h"
#include "cifXHWResources.h"
#include "OS_Includes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Macros for easier handling. */
#define CALL_FUNC(_dev, _api, _fun, ...) (_dev->_api->pfn##_fun(__VA_ARGS__))
#define CALL_TK_FUNC(_dev, _fun, ...) CALL_FUNC(((PDEVICEINSTANCE)_dev), ptTkitFun, _fun, __VA_ARGS__)
#define CALL_DEV_FUNC(_ch, _fun, ...) CALL_FUNC(CHINST_TO_DEVINST(_ch), ptDevFun, _fun, __VA_ARGS__)
#define CALL_API_FUNC(_ch, _fun, ...) CALL_FUNC(CHINST_TO_DEVINST(_ch), ptCifxFun, _fun, __VA_ARGS__)
#define CHINST_TO_DEVINST(_ch) ((PDEVICEINSTANCE)(((PCHANNELINSTANCE)(_ch))->pvDeviceInstance))

/* Function pointer typedef's for cifX Toolkit API functions. */
typedef int32_t (*PFN_CIFXTKITINIT)               (void);
typedef void    (*PFN_CIFXTKITDEINIT)             (void);
typedef int32_t (*PFN_CIFXTKITADDDEVICE)          (PDEVICEINSTANCE ptDevInstance);
typedef int32_t (*PFN_CIFXTKITREMOVEDEVICE)       (char* szBoard, int fForceRemove);
typedef void    (*PFN_cifXTKITENABLEHWINTERRUPT)  (PDEVICEINSTANCE ptDevInstance);
typedef void    (*PFN_cifXTKITDISABLEHWINTERRUPT) (PDEVICEINSTANCE ptDevInstance);
typedef int     (*PFN_cifXTKITISRHANDLER)         (PDEVICEINSTANCE ptDevInstance, int fPCIIgnoreGlobalIntFlag);
typedef void    (*PFN_cifXTKITDSRHANDLER)         (PDEVICEINSTANCE ptDevInstance);
typedef void    (*PFN_cifXTKITCYCLICTIMER)        (void);

/* Function list structure for the cifX Toolkit API functions. */
typedef struct CIFX_TKIT_FUNCTION_LIST_Ttag
{
  PFN_CIFXTKITINIT                pfncifXTKitInit;
  PFN_CIFXTKITDEINIT              pfncifXTKitDeinit;
  PFN_CIFXTKITADDDEVICE           pfncifXTKitAddDevice;
  PFN_CIFXTKITREMOVEDEVICE        pfncifXTKitRemoveDevice;
  PFN_cifXTKITENABLEHWINTERRUPT   pfncifXTKitEnableHWInterrupt;
  PFN_cifXTKITDISABLEHWINTERRUPT  pfncifXTKitDisableHWInterrupt;
  PFN_cifXTKITISRHANDLER          pfncifXTKitISRHandler;
  PFN_cifXTKITDSRHANDLER          pfncifXTKitDSRHandler;
  PFN_cifXTKITCYCLICTIMER         pfncifXTKitCyclicTimer;
} CIFX_TKIT_FUNCTION_LIST_T, *PCIFX_TKIT_FUNCTION_LIST_T;


/* Function list structure for cifX API functions. The function pointer
 * typedef's are already defined in cifXUser.h. */
typedef struct CIFX_API_FUNCTION_LIST_Ttag
{
  PFN_XDRIVEROPEN                     pfnxDriverOpen;
  PFN_XDRIVERCLOSE                    pfnxDriverClose;
  PFN_XDRIVERGETINFORMATION           pfnxDriverGetInformation;
  PFN_XDRIVERGETERRORDESCRIPTION      pfnxDriverGetErrorDescription;
  PFN_XDRIVERENUMBOARDS               pfnxDriverEnumBoards;
  PFN_XDRIVERENUMCHANNELS             pfnxDriverEnumChannels;
  PFN_XDRIVERMEMORYPOINTER            pfnxDriverMemoryPointer;
  PFN_XDRIVERRESTARTDEVICE            pfnxDriverRestartDevice;
  PFN_XSYSDEVICEOPEN                  pfnxSysdeviceOpen;
  PFN_XSYSDEVICECLOSE                 pfnxSysdeviceClose;
  PFN_XSYSDEVICEGETMBXSTATE           pfnxSysdeviceGetMBXState;
  PFN_XSYSDEVICEPUTPACKET             pfnxSysdevicePutPacket;
  PFN_XSYSDEVICEGETPACKET             pfnxSysdeviceGetPacket;
  PFN_XSYSDEVICEINFO                  pfnxSysdeviceInfo;
  PFN_XSYSDEVICEFINDFIRSTFILE         pfnxSysdeviceFindFirstFile;
  PFN_XSYSDEVICEFINDNEXTFILE          pfnxSysdeviceFindNextFile;
  PFN_XSYSDEVICEDOWNLOAD              pfnxSysdeviceDownload;
  PFN_XSYSDEVICEUPLOAD                pfnxSysdeviceUpload;
  PFN_XSYSDEVICERESET                 pfnxSysdeviceReset;
  PFN_XSYSDEVICERESETEX               pfnxSysdeviceResetEx;
  PFN_XSYSDEVICEBOOTSTART             pfnxSysdeviceBootstart;
  PFN_XSYSDEVICEEXTENDEDMEMORY        pfnxSysdeviceExtendedMemory;
  PFN_XCHANNELOPEN                    pfnxChannelOpen;
  PFN_XCHANNELCLOSE                   pfnxChannelClose;
  PFN_XCHANNELFINDFIRSTFILE           pfnxChannelFindFirstFile;
  PFN_XCHANNELFINDNEXTFILE            pfnxChannelFindNextFile;
  PFN_XCHANNELDOWNLOAD                pfnxChannelDownload;
  PFN_XCHANNELUPLOAD                  pfnxChannelUpload;
  PFN_XCHANNELGETMBXSTATE             pfnxChannelGetMBXState;
  PFN_XCHANNELPUTPACKET               pfnxChannelPutPacket;
  PFN_XCHANNELGETPACKET               pfnxChannelGetPacket;
  PFN_XCHANNELGETSENDPACKET           pfnxChannelGetSendPacket;
  PFN_XCHANNELCONFIGLOCK              pfnxChannelConfigLock;
  PFN_XCHANNELRESET                   pfnxChannelReset;
  PFN_XCHANNELINFO                    pfnxChannelInfo;
  PFN_XCHANNELWATCHDOG                pfnxChannelWatchdog;
  PFN_XCHANNELHOSTSTATE               pfnxChannelHostState;
  PFN_XCHANNELBUSSTATE                pfnxChannelBusState;
  PFN_XCHANNELDMASTATE                pfnxChannelDMAState;
  PFN_XCHANNELIOINFO                  pfnxChannelIOInfo;
  PFN_XCHANNELIOREAD                  pfnxChannelIORead;
  PFN_XCHANNELIOWRITE                 pfnxChannelIOWrite;
  PFN_XCHANNELIOREADSENDDATA          pfnxChannelIOReadSendData;
  PFN_XCHANNELCONTROLBLOCK            pfnxChannelControlBlock;
  PFN_XCHANNELCOMMONSTATUSBLOCK       pfnxChannelCommonStatusBlock;
  PFN_XCHANNELEXTENDEDSTATUSBLOCK     pfnxChannelExtendedStatusBlock;
  PFN_XCHANNELUSERBLOCK               pfnxChannelUserBlock;
  PFN_XCHANNELPLCMEMORYPTR            pfnxChannelPLCMemoryPtr;
  PFN_XCHANNELPLCISREADREADY          pfnxChannelPLCIsReadReady;
  PFN_XCHANNELPLCISWRITEREADY         pfnxChannelPLCIsWriteReady;
  PFN_XCHANNELPLCACTIVATEWRITE        pfnxChannelPLCActivateWrite;
  PFN_XCHANNELPLCACTIVATEREAD         pfnxChannelPLCActivateRead;
  PFN_XCHANNELREGISTERNOTIFICATION    pfnxChannelRegisterNotification;
  PFN_XCHANNELUNREGISTERNOTIFICATION  pfnxChannelUnregisterNotification;
  PFN_XCHANNELSYNCSTATE               pfnxChannelSyncState;
} CIFX_API_FUNCTION_LIST_T, *PCIFX_API_FUNCTION_LIST_T;


/* Function pointer typedef's for DEV API functions. */
typedef void    (*PFN_DEV_WRITEHANDSHAKEFLAGS)            (PCHANNELINSTANCE ptChannel);
typedef void    (*PFN_DEV_READHOSTFLAGS)                  (PCHANNELINSTANCE ptChannel, int fReadHostCOS);
typedef void    (*PFN_DEV_READHANDSHAKEFLAGS)             (PCHANNELINSTANCE ptChannel, int fReadSyncFlags, int fLockNeeded);

typedef uint8_t (*PFN_DEV_GETIOBITSTATE)                  (PCHANNELINSTANCE ptChannel, PIOINSTANCE ptIOInstance, int fOutput);
typedef int     (*PFN_DEV_WAITFORBITSTATE)                (PCHANNELINSTANCE ptChannel, uint32_t ulBitNumber, uint8_t bState, uint32_t ulTimeout);
typedef int     (*PFN_DEV_WAITFORIOBITSTATE)              (PCHANNELINSTANCE ptChannel, NETX_IO_BLOCK_T* ptInst, uint32_t ulTimeout);
typedef int     (*PFN_DEV_WAITFORMBXSTATE)                (PCHANNELINSTANCE ptChannel, PNETX_MAILBOX_BLOCK_T ptInst, uint32_t ulState, uint32_t ulTimeout);
typedef void    (*PFN_DEV_TOGGLEBIT)                      (PCHANNELINSTANCE ptChannel, uint32_t ulBitMask);
typedef void    (*PFN_DEV_TOGGLEIOACTION)                 (PCHANNELINSTANCE ptChannel, NETX_IO_BLOCK_T* ptInst);
typedef void    (*PFN_DEV_WRITECELL)                      (PCHANNELINSTANCE ptChannel, PNETX_MAILBOX_BLOCK_T ptInst, uint32_t ulValue);
typedef int     (*PFN_DEV_WAITFORSYNCSTATE)               (PCHANNELINSTANCE ptChannel, uint8_t bState, uint32_t ulTimeout);

typedef void    (*PFN_DEV_TOGGLESYNCBIT)                  (PDEVICEINSTANCE  ptDevInstance, uint32_t ulBitMask);
typedef int32_t (*PFN_DEV_PUTPACKET)                      (PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout);
typedef int32_t (*PFN_DEV_GETPACKET)                      (PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptRecvPkt, uint32_t ulRecvBufferSize, uint32_t ulTimeout);
typedef int32_t (*PFN_DEV_GETMBXSTATE)                    (PCHANNELINSTANCE ptChannel, uint32_t* pulRecvPktCnt, uint32_t* pulSendPktCnt);
typedef int32_t (*PFN_DEV_GETMBXFILLLEVEL)                (PNETX_MAILBOX_BLOCK_T  ptInst);
typedef int32_t (*PFN_DEV_TRANSFERPACKET)                 (void*                  pvChannel,        CIFX_PACKET* ptSendPkt, CIFX_PACKET* ptRecvPkt,
                                                           uint32_t               ulRecvBufferSize, uint32_t     ulTimeout,
                                                           PFN_RECV_PKT_CALLBACK  pfnRecvPacket,    void*        pvUser);
typedef int     (*PFN_DEV_ISREADY)                        (PCHANNELINSTANCE ptChannel);
typedef int     (*PFN_DEV_ISRUNNING)                      (PCHANNELINSTANCE ptChannel);
typedef int     (*PFN_DEV_ISCOMMUNICATING)                (PCHANNELINSTANCE ptChannel, int32_t* plError);
typedef int     (*PFN_DEV_WAITFORREADY_POLL)              (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);
typedef int     (*PFN_DEV_WAITFORNOTREADY_POLL)           (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);
typedef int     (*PFN_DEV_WAITFORLOCK_POLL)               (PCHANNELINSTANCE ptChannel, PNETX_IO_BLOCK_T ptInst, uint32_t ulTimeout);

typedef int32_t (*PFN_DEV_TRIGGERWATCHDOG)                (PCHANNELINSTANCE ptChannel, uint32_t ulTriggerCmd, uint32_t* pulTriggerValue);
typedef int32_t (*PFN_DEV_GETHOSTSTATE)                   (PCHANNELINSTANCE ptChannel, uint32_t* pulState);
typedef int32_t (*PFN_DEV_SETHOSTSTATE)                   (PCHANNELINSTANCE ptChannel, uint32_t ulNewState, uint32_t ulTimeout);
typedef int32_t (*PFN_DEV_DOCHANNELINIT)                  (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);
typedef int32_t (*PFN_DEV_DOSYSTEMSTART)                  (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam);
typedef int32_t (*PFN_DEV_DOSYSTEMBOOTSTART)              (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam);
typedef int32_t (*PFN_DEV_DOUPDATESTART)                  (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam);
typedef int32_t (*PFN_DEV_BUSSTATE)                       (PCHANNELINSTANCE ptChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout);
typedef int32_t (*PFN_DEV_DOHOSTCOSCHANGE)                (PCHANNELINSTANCE ptChannel,          uint32_t ulSetCOSMask,     uint32_t ulClearCOSMask,
                                                           uint32_t         ulPostClearCOSMask, int32_t  lSignallingError, uint32_t ulTimeout);
typedef void    (*PFN_DEV_CHECKCOSFLAGS)                  (PDEVICEINSTANCE ptDevInstance);
typedef uint8_t (*PFN_DEV_GETHANDSHAKEBITSTATE)           (PCHANNELINSTANCE ptChannel, uint32_t ulBitMsk);

typedef int32_t (*PFN_TRANSFER_PACKET)                    (void*                    pvChannel,
                                                           CIFX_PACKET*             ptSendPkt,
                                                           CIFX_PACKET*             ptRecvPkt,
                                                           uint32_t                 ulRecvBufferSize,
                                                           uint32_t                 ulTimeout,
                                                           PFN_RECV_PKT_CALLBACK    pfnPktCallback,
                                                           void*                    pvUser);
typedef int32_t (*PFN_DEV_REMOVECHANNELFILES)             (PCHANNELINSTANCE         ptChannel,
                                                           uint32_t                 ulChannel,
                                                           PFN_TRANSFER_PACKET      pfnTransferPacket,
                                                           PFN_RECV_PKT_CALLBACK    pfnRecvPacket,
                                                           void*                    pvUser,
                                                           char*                    szExceptFile);
typedef int32_t (*PFN_DEV_DELETEFILE)                     (void*                    pvChannel,
                                                           uint32_t                 ulChannelNumber,
                                                           char*                    pszFileName,
                                                           PFN_TRANSFER_PACKET      pfnTransferPacket,
                                                           PFN_RECV_PKT_CALLBACK    pfnRecvPacket,
                                                           void*                    pvUser);
typedef int32_t (*PFN_DEV_CHECKFORDOWNLOAD)               (void*                    pvChannel,
                                                           uint32_t                 ulChannelNumber,
                                                           int*                     pfDownload,
                                                           char*                    pszFileName,
                                                           void*                    pvFileData,
                                                           uint32_t                 ulFileSize,
                                                           PFN_TRANSFER_PACKET      pfnTransferPacket,
                                                           PFN_RECV_PKT_CALLBACK    pfnRecvPacket,
                                                           void*                    pvUser);
typedef int32_t (*PFN_DEV_PROCESSFWDOWNLOAD)              (PDEVICEINSTANCE          ptDevInstance,
                                                           uint32_t                 ulChannel,
                                                           char*                    pszFullFileName,
                                                           char*                    pszFileName,
                                                           uint32_t                 ulFileLength,
                                                           uint8_t*                 pbBuffer,
                                                           uint8_t*                 pbLoadState,
                                                           PFN_TRANSFER_PACKET      pfnTransferPacket,
                                                           PFN_PROGRESS_CALLBACK    pfnCallback,
                                                           PFN_RECV_PKT_CALLBACK    pfnRecvPktCallback,
                                                           void*                    pvUser);
#ifdef CIFX_TOOLKIT_DMA
typedef int32_t (*PFN_DEV_DMASTATE)                       (PCHANNELINSTANCE ptChannel, uint32_t ulCmd, uint32_t* pulState);
typedef int32_t (*PFN_DEV_SETUPDMABUFFERS)                (PCHANNELINSTANCE ptChannel);
#endif


/* Function list structure for DEV API functions. */
typedef struct CIFX_DEV_FUNCTION_LIST_Ttag
{
  PFN_DEV_WRITEHANDSHAKEFLAGS           pfnDEV_WriteHandshakeFlags;
  PFN_DEV_READHOSTFLAGS                 pfnDEV_ReadHostFlags;
  PFN_DEV_READHANDSHAKEFLAGS            pfnDEV_ReadHandshakeFlags;
  PFN_DEV_GETIOBITSTATE                 pfnDEV_GetIOBitstate;
  PFN_DEV_WAITFORBITSTATE               pfnDEV_WaitForBitState;
  PFN_DEV_WAITFORIOBITSTATE             pfnDEV_WaitForIoBitState;
  PFN_DEV_WAITFORMBXSTATE               pfnDEV_WaitForMbxState;
  PFN_DEV_TOGGLEBIT                     pfnDEV_ToggleBit;
  PFN_DEV_TOGGLEIOACTION                pfnDEV_ToggleIoAction;
  PFN_DEV_WRITECELL                     pfnDEV_WriteCell;
  PFN_DEV_WAITFORSYNCSTATE              pfnDEV_WaitForSyncState;
  PFN_DEV_TOGGLESYNCBIT                 pfnDEV_ToggleSyncBit;
  PFN_DEV_PUTPACKET                     pfnDEV_PutPacket;
  PFN_DEV_GETPACKET                     pfnDEV_GetPacket;
  PFN_DEV_GETMBXSTATE                   pfnDEV_GetMBXState;
  PFN_DEV_GETMBXFILLLEVEL               pfnDEV_GetMBXFillLevel;
  PFN_DEV_TRANSFERPACKET                pfnDEV_TransferPacket;
  PFN_DEV_ISREADY                       pfnDEV_IsReady;
  PFN_DEV_ISRUNNING                     pfnDEV_IsRunning;
  PFN_DEV_ISCOMMUNICATING               pfnDEV_IsCommunicating;
  PFN_DEV_WAITFORREADY_POLL             pfnDEV_WaitForReady_Poll;
  PFN_DEV_WAITFORNOTREADY_POLL          pfnDEV_WaitForNotReady_Poll;
  PFN_DEV_WAITFORLOCK_POLL              pfnDEV_WaitForLock_Poll;
  PFN_DEV_TRIGGERWATCHDOG               pfnDEV_TriggerWatchdog;
  PFN_DEV_GETHOSTSTATE                  pfnDEV_GetHostState;
  PFN_DEV_SETHOSTSTATE                  pfnDEV_SetHostState;
  PFN_DEV_DOCHANNELINIT                 pfnDEV_DoChannelInit;
  PFN_DEV_DOSYSTEMSTART                 pfnDEV_DoSystemStart;
  PFN_DEV_DOSYSTEMBOOTSTART             pfnDEV_DoSystemBootstart;
  PFN_DEV_DOUPDATESTART                 pfnDEV_DoUpdateStart;
  PFN_DEV_BUSSTATE                      pfnDEV_BusState;
  PFN_DEV_DOHOSTCOSCHANGE               pfnDEV_DoHostCOSChange;
  PFN_DEV_CHECKCOSFLAGS                 pfnDEV_CheckCOSFlags;
  PFN_DEV_GETHANDSHAKEBITSTATE          pfnDEV_GetHandshakeBitState;
  PFN_DEV_REMOVECHANNELFILES            pfnDEV_RemoveChannelFiles;
  PFN_DEV_DELETEFILE                    pfnDEV_DeleteFile;
  PFN_DEV_CHECKFORDOWNLOAD              pfnDEV_CheckForDownload;
  PFN_DEV_PROCESSFWDOWNLOAD             pfnDEV_ProcessFWDownload;
#ifdef CIFX_TOOLKIT_DMA
  PFN_DEV_DMASTATE                      pfnDEV_DMAState;
  PFN_DEV_SETUPDMABUFFERS               pfnDEV_SetupDMABuffers;
#endif
} CIFX_DEV_FUNCTION_LIST_T, *PCIFX_DEV_FUNCTION_LIST_T;


/* Functions for retrieving the corresponding API function structure. */
#if DPM_SUPPORT
  PCIFX_API_FUNCTION_LIST_T  cifXTkitGetDpmApiFunctionList (void);
  PCIFX_DEV_FUNCTION_LIST_T  cifXTkitGetDpmDevFunctionList (void);
  PCIFX_TKIT_FUNCTION_LIST_T cifXTkitGetDpmTkitFunctionList(void);
#endif

#if HIF_SUPPORT
  PCIFX_API_FUNCTION_LIST_T  cifXTkitGetHifApiFunctionList (void);
  PCIFX_DEV_FUNCTION_LIST_T  cifXTkitGetHifDevFunctionList (void);
  PCIFX_TKIT_FUNCTION_LIST_T cifXTkitGetHifTkitFunctionList(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* CIFX_FUNCTION_LIST__H */
