#ifndef INTAPI2POINTER__H
#define INTAPI2POINTER__H

#include "cifXHWFunctions.h"
#include "apitranslate.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef CIFX_TOOLKIT_FUNCTION_LIST

struct driver_handle {
    CIFXHANDLE hDriverDPM;
    CIFXHANDLE hDriverHIF;
    uint32_t open_count;
};

/* currently double declared see CIFX_API_PREVENT_DECL_ERROR) */
int32_t cifXTKitInit(void);
void    cifXTKitDeinit( void);
int32_t cifXTKitAddDevice(PDEVICEINSTANCE ptDevInstance);
int32_t cifXTKitRemoveDevice(char* szBoard, int fForceRemove);
void    cifXTKitCyclicTimer(void);

#define cifXTKitEnableHWInterrupt(x)  CALL_TK_FUNC(x, cifXTKitEnableHWInterrupt, x)
#define cifXTKitDisableHWInterrupt(x) CALL_TK_FUNC(x, cifXTKitEnableHWInterrupt, x)
#define cifXTKitISRHandler(x,...)     CALL_TK_FUNC(x, cifXTKitISRHandler, x, __VA_ARGS__)
#define cifXTKitDSRHandler(x)         CALL_TK_FUNC(x, cifXTKitDSRHandler, x)

#define DEV_WriteHandshakeFlags(x, ...)              CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_WriteHandshakeFlags, x, __VA_ARGS__)
#define DEV_ReadHostFlags(x, ...)                    CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_ReadHostFlags, x, __VA_ARGS__)
#define DEV_ReadHandshakeFlags(x, ...)               CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_ReadHandshakeFlags, x, __VA_ARGS__)
#define DEV_GetIOBitstate(x, ...)                    CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_GetIOBitstate, x, __VA_ARGS__)
#define DEV_WaitForBitState(x, ...)                  CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_WaitForBitState, x, __VA_ARGS__)
#define DEV_WaitForIoBitState(x, ...)                CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_WaitForIoBitState, x, __VA_ARGS__)
#define DEV_WaitForMbxState(x, ...)                  CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_WaitForMbxState, x, __VA_ARGS__)
#define DEV_ToggleBit(x, ...)                        CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_ToggleBit, x, __VA_ARGS__)
#define DEV_ToggleIoAction(x, ...)                   CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_ToggleIoAction, x, __VA_ARGS__)
#define DEV_WriteCell(x, ...)                        CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_WriteCell, x, __VA_ARGS__)
#define DEV_WaitForSyncState(x, ...)                 CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_WaitForSyncState, x, __VA_ARGS__)
#define DEV_ToggleSyncBit(x, ...)                    CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_ToggleSyncBit, x, __VA_ARGS__)
#define DEV_PutPacket(x, ...)                        CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_PutPacket, x, __VA_ARGS__)
#define DEV_GetPacket(x, ...)                        CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_GetPacket, x, __VA_ARGS__)
#define DEV_GetMBXState(x, ...)                      CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_GetMBXState, x, __VA_ARGS__)
#define DEV_GetMBXFillLevel(x, ...)                  CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_GetMBXFillLevel, x, __VA_ARGS__)
#define DEV_TransferPacket(x, ...)                   CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_TransferPacket, x, __VA_ARGS__)
#define DEV_IsReady(x)                               CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_IsReady, x)
#define DEV_IsRunning(x, ...)                        CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_IsRunning, x, __VA_ARGS__)
#define DEV_IsCommunicating(x, ...)                  CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_IsCommunicating, x, __VA_ARGS__)
#define DEV_WaitForReady_Poll(x, ...)                CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_WaitForReady_Poll, x, __VA_ARGS__)
#define DEV_WaitForNotReady_Poll(x, ...)             CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_WaitForNotReady_Poll, x, __VA_ARGS__)
#define DEV_WaitForRunning_Poll(x, ...)              CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_WaitForRunning_Poll, x, __VA_ARGS__)
#define DEV_WaitForNotRunning_Poll(x, ...)           CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_WaitForNotRunning_Poll, x, __VA_ARGS__)
#define DEV_WaitForLock_Poll(x, ...)                 CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_WaitForLock_Poll, x, __VA_ARGS__)
#define DEV_TriggerWatchdog(x, ...)                  CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_TriggerWatchdog, x, __VA_ARGS__)
#define DEV_GetHostState(x, ...)                     CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_GetHostState, x, __VA_ARGS__)
#define DEV_SetHostState(x, ...)                     CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_SetHostState, x, __VA_ARGS__)
#define DEV_ReadWriteBlock(x, ...)                   CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_ReadWriteBlock, x, __VA_ARGS__)
#define DEV_DoChannelInit(x, ...)                    CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_DoChannelInit, x, __VA_ARGS__)
#define DEV_DoSystemStart(x, ...)                    CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_DoSystemStart, x, __VA_ARGS__)
#define DEV_DoSystemBootstart(x, ...)                CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_DoSystemBootstart, x, __VA_ARGS__)
#define DEV_DoUpdateStart(x, ...)                    CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_DoUpdateStart, x, __VA_ARGS__)
#define DEV_BusState(x, ...)                         CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_BusState, x, __VA_ARGS__)
#define DEV_DoHostCOSChange(x, ...)                  CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_DoHostCOSChange, x, __VA_ARGS__)
#define DEV_CheckCOSFlags(x, ...)                    CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_CheckCOSFlags, x, __VA_ARGS__)
#define DEV_GetHandshakeBitState(x, ...)             CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_GetHandshakeBitState, x, __VA_ARGS__)
#define DEV_RemoveChannelFiles(x, ...)               CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_RemoveChannelFiles, x, __VA_ARGS__)
#define DEV_DeleteFile(x, ...)                       CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_DeleteFile, x, __VA_ARGS__)
#define DEV_CheckForDownload(x, ...)                 CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_CheckForDownload, x, __VA_ARGS__)
#define DEV_IsFWFile(x, ...)                         CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_IsFWFile, x, __VA_ARGS__)
#define DEV_IsNXFFile(x, ...)                        CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_IsNXFFile, x, __VA_ARGS__)
#define DEV_IsNXOFile(x, ...)                        CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_IsNXOFile, x, __VA_ARGS__)
#define DEV_GetFWTransferTypeFromFileName(x, ...)    CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_GetFWTransferTypeFromFileName, x, __VA_ARGS__)
#define DEV_ProcessFWDownload(x, ...)                CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_ProcessFWDownload, x, __VA_ARGS__)
#define DEV_DownloadFile(x, ...)                     CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_DownloadFile, x, __VA_ARGS__)
#define DEV_UploadFile(x, ...)                       CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_UploadFile, x, __VA_ARGS__)
#ifdef CIFX_TOOLKIT_DMA
    #define DEV_DMAState(x, ...)                         CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_DMAState, __VA_ARGS__)
    #define DEV_SetupDMABuffers(x, ...)                  CALL_DEV_FUNC(HSYS_TO_DEVINST(x), DEV_SetupDMABuffers, __VA_ARGS__)
#endif

#endif //CIFX_TOOLKIT_FUNCTION_LIST

#ifdef __cplusplus
}
#endif

#endif //INTAPI2POINTER
