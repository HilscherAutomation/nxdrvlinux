
#include <stdint.h>

#include "cifXUser.h"
#include "apitranslate.h"
#include "cifXFunctionList.h"
#include "intapi2pointer.h"

//TODO: option to select of only one toolkit? -> may skip wrapper call

extern int g_api_if_initialized;

extern CIFX_TKIT_FUNCTION_LIST_T* g_pDPMTKfun;
extern CIFX_TKIT_FUNCTION_LIST_T* g_pHIFTKfun;
extern CIFX_API_FUNCTION_LIST_T*  g_pDPMAPIfun;
extern CIFX_API_FUNCTION_LIST_T*  g_pHIFAPIfun;

static struct driver_handle s_pdrvhandle;

int32_t APIENTRY xDriverOpen( CIFXHANDLE* phDriver) {
    int32_t ret = CIFX_INVALID_PARAMETER;

    if (g_api_if_initialized != 1)
        return CIFX_FUNCTION_FAILED;

    if (phDriver == NULL)
        return CIFX_INVALID_PARAMETER;

    ret = CIFX_FUNCTION_FAILED;/* there is no general memory error */

    *phDriver = NULL;
    if (s_pdrvhandle.open_count == 0) {
        s_pdrvhandle.hDriverDPM = NULL;
        s_pdrvhandle.hDriverHIF = NULL;

        ret = g_pDPMAPIfun->pfnxDriverOpen( &s_pdrvhandle.hDriverDPM);
        if (ret == CIFX_NO_ERROR) {
            ret = g_pHIFAPIfun->pfnxDriverOpen( &s_pdrvhandle.hDriverHIF);
        }
        if (ret != CIFX_NO_ERROR)
            goto err;
    }
    /* currently both toolkits need to be always present */
    if ( (s_pdrvhandle.hDriverDPM != NULL) && (s_pdrvhandle.hDriverHIF != NULL) ) {
        *phDriver = &s_pdrvhandle;
        s_pdrvhandle.open_count++;
        return CIFX_NO_ERROR;
    }
err:
    if (s_pdrvhandle.hDriverDPM != NULL)
        g_pDPMAPIfun->pfnxDriverClose( s_pdrvhandle.hDriverDPM);
    if (s_pdrvhandle.hDriverHIF != NULL)
        g_pHIFAPIfun->pfnxDriverClose( s_pdrvhandle.hDriverHIF);

    return ret;
}

int32_t APIENTRY xDriverClose( CIFXHANDLE hDriver) {
    int32_t ret = CIFX_INVALID_HANDLE;

    if (&s_pdrvhandle != hDriver)
        return CIFX_INVALID_HANDLE;

    if (s_pdrvhandle.open_count > 0) {
        if (s_pdrvhandle.open_count == 1) {
            if (s_pdrvhandle.hDriverDPM != NULL) {
                if ( (ret = g_pDPMAPIfun->pfnxDriverClose( s_pdrvhandle.hDriverDPM)) != CIFX_NO_ERROR)
                    goto err;
            }
            s_pdrvhandle.hDriverDPM = NULL;
            if (s_pdrvhandle.hDriverHIF != NULL) {
                if ( (ret = g_pHIFAPIfun->pfnxDriverClose( s_pdrvhandle.hDriverHIF)) != CIFX_NO_ERROR)
                    goto err;
            }
            s_pdrvhandle.hDriverHIF = NULL;
        }
        s_pdrvhandle.open_count--;
        return CIFX_NO_ERROR;
    }
err:
    return ret;
}

int32_t APIENTRY xDriverGetInformation( CIFXHANDLE hDriver, uint32_t ulSize, void* pvDriverInfo) {

    if (&s_pdrvhandle != hDriver)
        return CIFX_INVALID_HANDLE;

    //TODO: common function?
    return g_pDPMAPIfun->pfnxDriverGetInformation( s_pdrvhandle.hDriverDPM, ulSize, pvDriverInfo);
}

int32_t APIENTRY xDriverGetErrorDescription( int32_t lError,  char* szBuffer, uint32_t ulBufferLen) {

    if (g_api_if_initialized != 1)
        return CIFX_FUNCTION_FAILED;

    //TODO: common function?
    return g_pDPMAPIfun->pfnxDriverGetErrorDescription( lError, szBuffer, ulBufferLen);
}

int32_t APIENTRY xDriverEnumBoards( CIFXHANDLE hDriver, uint32_t ulBoard, uint32_t ulSize, void* pvBoardInfo) {

    if (&s_pdrvhandle != hDriver)
        return CIFX_INVALID_HANDLE;

    //TODO: common function?
    return g_pDPMAPIfun->pfnxDriverEnumBoards( s_pdrvhandle.hDriverDPM, ulBoard, ulSize, pvBoardInfo);
}

int32_t APIENTRY xDriverEnumChannels( CIFXHANDLE hDriver, uint32_t ulBoard, uint32_t ulChannel, uint32_t ulSize, void* pvChannelInfo) {

    if (&s_pdrvhandle != hDriver)
        return CIFX_INVALID_HANDLE;

    //TODO: common function?
    return g_pDPMAPIfun->pfnxDriverEnumChannels( s_pdrvhandle.hDriverDPM, ulBoard, ulChannel, ulSize, pvChannelInfo);
}

int32_t APIENTRY xDriverMemoryPointer( CIFXHANDLE  hDriver, uint32_t ulBoard, uint32_t ulCmd, void* pvMemoryInfo) {

    if (&s_pdrvhandle != hDriver)
        return CIFX_INVALID_HANDLE;

    return g_pDPMAPIfun->pfnxDriverMemoryPointer( s_pdrvhandle.hDriverDPM, ulBoard, ulCmd, pvMemoryInfo);
}

#if 0
int32_t APIENTRY xDriverRestartDevice( CIFXHANDLE hDriver, char* szBoardName, void* pvData) {

    if (&s_pdrvhandle != hDriver)
        return CIFX_INVALID_HANDLE;

    return g_pDPMAPIfun->pfnxDriverRestartDevice( s_pdrvhandle.hDriverDPM, szBoardName, pvData);
}
#endif

int32_t APIENTRY xSysdeviceOpen( CIFXHANDLE hDriver, char* szBoard, CIFXHANDLE* phSysdevice) {

    if (&s_pdrvhandle != hDriver)
        return CIFX_INVALID_HANDLE;

    /* NOTE/TODO: Currently there is no need to differentiate both (DPM/HIF) functions do the same (should be moved in common?) */
    return g_pDPMAPIfun->pfnxSysdeviceOpen( s_pdrvhandle.hDriverDPM, szBoard, phSysdevice);
}

int32_t APIENTRY xSysdeviceClose(CIFXHANDLE hSysdevice) {
   return CALL_API_FUNC(HSYS_TO_DEVINST(hSysdevice), xSysdeviceClose, hSysdevice);
}

int32_t APIENTRY xSysdeviceGetMBXState(CIFXHANDLE hSysdevice, uint32_t* pulRecvPktCount, uint32_t* pulSendPktCount) {
    return CALL_API_FUNC(HSYS_TO_DEVINST(hSysdevice), xSysdeviceGetMBXState, hSysdevice, pulRecvPktCount, pulSendPktCount);
}

int32_t APIENTRY xSysdevicePutPacket(CIFXHANDLE hSysdevice, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout) {
    return CALL_API_FUNC(HSYS_TO_DEVINST(hSysdevice), xSysdevicePutPacket, hSysdevice, ptSendPkt, ulTimeout);
}

int32_t APIENTRY xSysdeviceGetPacket(CIFXHANDLE hSysdevice, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout) {
    return CALL_API_FUNC(HSYS_TO_DEVINST(hSysdevice), xSysdeviceGetPacket, hSysdevice, ulSize, ptRecvPkt, ulTimeout);
}

int32_t APIENTRY xSysdeviceInfo(CIFXHANDLE hSysdevice, uint32_t ulCmd, uint32_t ulSize, void* pvInfo) {
    return CALL_API_FUNC(HSYS_TO_DEVINST(hSysdevice), xSysdeviceInfo, hSysdevice, ulCmd, ulSize, pvInfo);
}

int32_t APIENTRY xSysdeviceFindFirstFile(CIFXHANDLE hSysdevice, uint32_t ulChannel, CIFX_DIRECTORYENTRY* ptDirectoryInfo,
                                                     PFN_RECV_PKT_CALLBACK  pfnRecvPktCallback, void* pvUser) {
    return CALL_API_FUNC(HSYS_TO_DEVINST(hSysdevice), xSysdeviceFindFirstFile, hSysdevice, ulChannel, ptDirectoryInfo,
                                                     pfnRecvPktCallback, pvUser);
}

int32_t APIENTRY xSysdeviceFindNextFile(CIFXHANDLE hSysdevice, uint32_t ulChannel, CIFX_DIRECTORYENTRY* ptDirectoryInfo,
                                                    PFN_RECV_PKT_CALLBACK  pfnRecvPktCallback, void* pvUser) {
    return CALL_API_FUNC(HSYS_TO_DEVINST(hSysdevice), xSysdeviceFindNextFile, hSysdevice, ulChannel, ptDirectoryInfo,
                                                    pfnRecvPktCallback, pvUser);
}

int32_t APIENTRY xSysdeviceDownload( CIFXHANDLE            hSysdevice,
                                     uint32_t              ulChannel,
                                     uint32_t              ulMode,
                                     char*                 pszFileName,
                                     uint8_t*              pabFileData,
                                     uint32_t              ulFileSize,
                                     PFN_PROGRESS_CALLBACK pfnCallback,
                                     PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                     void*                 pvUser) {
    return CALL_API_FUNC(HSYS_TO_DEVINST(hSysdevice), xSysdeviceDownload, hSysdevice,
                                                                          ulChannel,
                                                                          ulMode,
                                                                          pszFileName,
                                                                          pabFileData,
                                                                          ulFileSize,
                                                                          pfnCallback,
                                                                          pfnRecvPktCallback,
                                                                          pvUser);
}

int32_t APIENTRY xSysdeviceUpload(CIFXHANDLE            hSysdevice,
                                  uint32_t              ulChannel,
                                  uint32_t              ulMode,
                                  char*                 pszFileName,
                                  uint8_t*              pabFileData,
                                  uint32_t*             pulFileSize,
                                  PFN_PROGRESS_CALLBACK pfnCallback,
                                  PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                  void*                 pvUser) {
    return CALL_API_FUNC(HSYS_TO_DEVINST(hSysdevice), xSysdeviceUpload, hSysdevice,
                                                                        ulChannel,
                                                                        ulMode,
                                                                        pszFileName,
                                                                        pabFileData,
                                                                        pulFileSize,
                                                                        pfnCallback,
                                                                        pfnRecvPktCallback,
                                                                        pvUser);
}

int32_t APIENTRY xSysdeviceReset(CIFXHANDLE hSysdevice, uint32_t ulTimeout) {
    return CALL_API_FUNC(HSYS_TO_DEVINST(hSysdevice), xSysdeviceReset, hSysdevice, ulTimeout);
}

int32_t APIENTRY xSysdeviceResetEx( CIFXHANDLE  hSysdevice, uint32_t ulTimeout, uint32_t ulMode) {
    return CALL_API_FUNC(HSYS_TO_DEVINST(hSysdevice), xSysdeviceResetEx, hSysdevice, ulTimeout, ulMode);
}

int32_t APIENTRY xSysdeviceBootstart(CIFXHANDLE hSysdevice, uint32_t ulTimeout) {
    return CALL_API_FUNC(HSYS_TO_DEVINST(hSysdevice), xSysdeviceBootstart, hSysdevice, ulTimeout);
}

int32_t APIENTRY xSysdeviceExtendedMemory(CIFXHANDLE hSysdevice, uint32_t ulCmd, CIFX_EXTENDED_MEMORY_INFORMATION* ptExtMemInfo) {
    return CALL_API_FUNC(HSYS_TO_DEVINST(hSysdevice), xSysdeviceExtendedMemory, hSysdevice, ulCmd, ptExtMemInfo);
}

int32_t APIENTRY xChannelOpen( CIFXHANDLE hDriver, char* szBoard, uint32_t ulChannel, CIFXHANDLE* phChannel) {

    if (&s_pdrvhandle != hDriver)
        return CIFX_INVALID_HANDLE;

    /* NOTE/TODO: Currently there is no need to differentiate both (DPM/HIF) functions do the same (should be moved in common?) */
    return g_pDPMAPIfun->pfnxChannelOpen( s_pdrvhandle.hDriverDPM, szBoard, ulChannel, phChannel);
}

int32_t APIENTRY xChannelClose(CIFXHANDLE hChannel) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelClose, hChannel);
}

int32_t APIENTRY xChannelFindFirstFile(CIFXHANDLE hChannel, CIFX_DIRECTORYENTRY* ptDirectoryInfo,
                                                   PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelFindFirstFile, hChannel, ptDirectoryInfo,
                                                               pfnRecvPktCallback, pvUser);
}

int32_t APIENTRY xChannelFindNextFile(CIFXHANDLE hChannel, CIFX_DIRECTORYENTRY* ptDirectoryInfo,
                                                  PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelFindNextFile, hChannel, ptDirectoryInfo,
                                                               pfnRecvPktCallback, pvUser);
}

int32_t APIENTRY xChannelDownload(CIFXHANDLE hChannel,    uint32_t   ulMode,
                                              char*      pszFileName, uint8_t*  pabFileData, uint32_t ulFileSize,
                                              PFN_PROGRESS_CALLBACK pfnCallback, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelDownload, hChannel, ulMode,
                                              pszFileName, pabFileData, ulFileSize,
                                              pfnCallback, pfnRecvPktCallback, pvUser);
}

int32_t APIENTRY xChannelUpload(CIFXHANDLE hChannel, uint32_t ulMode,
                                            char* pszFileName, uint8_t* pabFileData, uint32_t* pulFileSize,
                                            PFN_PROGRESS_CALLBACK pfnCallback, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelUpload, hChannel, ulMode, pszFileName, pabFileData, pulFileSize,
                                            pfnCallback, pfnRecvPktCallback, pvUser);
}

int32_t APIENTRY xChannelGetMBXState(CIFXHANDLE hChannel, uint32_t* pulRecvPktCount, uint32_t* pulSendPktCount) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelGetMBXState, hChannel, pulRecvPktCount, pulSendPktCount);
}

int32_t APIENTRY xChannelPutPacket(CIFXHANDLE hChannel, CIFX_PACKET*  ptSendPkt, uint32_t ulTimeout) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelPutPacket, hChannel, ptSendPkt, ulTimeout);
}

int32_t APIENTRY xChannelGetPacket(CIFXHANDLE hChannel, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelGetPacket, hChannel, ulSize, ptRecvPkt, ulTimeout);
}

int32_t APIENTRY xChannelGetSendPacket(CIFXHANDLE hChannel, uint32_t ulSize, CIFX_PACKET* ptRecvPkt) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelGetSendPacket, hChannel, ulSize, ptRecvPkt);
}

int32_t APIENTRY xChannelConfigLock(CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelConfigLock, hChannel, ulCmd, pulState, ulTimeout);
}

int32_t APIENTRY xChannelReset(CIFXHANDLE  hChannel, uint32_t ulResetMode, uint32_t ulTimeout) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelReset, hChannel, ulResetMode, ulTimeout);
}

int32_t APIENTRY xChannelInfo(CIFXHANDLE hChannel, uint32_t ulSize, void* pvChannelInfo) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelInfo, hChannel, ulSize, pvChannelInfo);
}

int32_t APIENTRY xChannelWatchdog(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t* pulTrigger) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelWatchdog, hChannel, ulCmd, pulTrigger);
}

int32_t APIENTRY xChannelHostState(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelHostState, hChannel, ulCmd, pulState, ulTimeout);
}

int32_t APIENTRY xChannelBusState(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelBusState, hChannel, ulCmd, pulState, ulTimeout);
}

int32_t APIENTRY xChannelDMAState(CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelDMAState, hChannel, ulCmd, pulState);
}

int32_t APIENTRY xChannelIOInfo(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulAreaNumber, uint32_t ulSize, void* pvData) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelIOInfo, hChannel, ulCmd, ulAreaNumber, ulSize, pvData);
}

int32_t APIENTRY xChannelIORead(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData, uint32_t ulTimeout) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelIORead, hChannel, ulAreaNumber, ulOffset, ulDataLen, pvData, ulTimeout);
}

int32_t APIENTRY xChannelIOWrite(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData, uint32_t ulTimeout) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelIOWrite, hChannel, ulAreaNumber, ulOffset, ulDataLen, pvData, ulTimeout);
}

int32_t APIENTRY xChannelIOReadSendData(CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelIOReadSendData, hChannel, ulAreaNumber, ulOffset, ulDataLen, pvData);
}

int32_t APIENTRY xChannelControlBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelControlBlock, hChannel, ulCmd, ulOffset, ulDataLen, pvData);
}

int32_t APIENTRY xChannelCommonStatusBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelCommonStatusBlock, hChannel, ulCmd, ulOffset, ulDataLen, pvData);
}

int32_t APIENTRY xChannelExtendedStatusBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelExtendedStatusBlock, hChannel, ulCmd, ulOffset, ulDataLen, pvData);
}

int32_t APIENTRY xChannelUserBlock(CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelUserBlock, hChannel, ulAreaNumber, ulCmd, ulOffset, ulDataLen, pvData);
}

int32_t APIENTRY xChannelPLCMemoryPtr(CIFXHANDLE hChannel, uint32_t ulCmd, void* pvMemoryInfo) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelPLCMemoryPtr, hChannel, ulCmd, pvMemoryInfo);
}

int32_t APIENTRY xChannelPLCIsReadReady(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t* pulReadState) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelPLCIsReadReady, hChannel, ulAreaNumber, pulReadState);
}

int32_t APIENTRY xChannelPLCIsWriteReady(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t* pulWriteState) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelPLCIsWriteReady, hChannel, ulAreaNumber, pulWriteState);
}

int32_t APIENTRY xChannelPLCActivateWrite(CIFXHANDLE hChannel, uint32_t ulAreaNumber) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelPLCActivateWrite, hChannel, ulAreaNumber);
}

int32_t APIENTRY xChannelPLCActivateRead(CIFXHANDLE hChannel, uint32_t ulAreaNumber) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelPLCActivateRead, hChannel, ulAreaNumber);
}

int32_t APIENTRY xChannelRegisterNotification(CIFXHANDLE           hChannel,
                                              uint32_t             ulNotification,
                                              PFN_NOTIFY_CALLBACK  pfnCallback,
                                              void*                pvUser) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelRegisterNotification, hChannel, ulNotification,
                                              pfnCallback,
                                              pvUser);
}

int32_t APIENTRY xChannelUnregisterNotification( CIFXHANDLE hChannel,
                                                             uint32_t   ulNotification) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelUnregisterNotification, hChannel, ulNotification);
}

int32_t APIENTRY xChannelSyncState( CIFXHANDLE  hChannel,
                                                uint32_t    ulCmd,
                                                uint32_t    ulTimeout,
                                                uint32_t*   pulErrorCount) {
    return CALL_API_FUNC(HCHAN_TO_DEVINST(hChannel), xChannelSyncState, hChannel, ulCmd,
                                                ulTimeout,
                                                pulErrorCount);
}
