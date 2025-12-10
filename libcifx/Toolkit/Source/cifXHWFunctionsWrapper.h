/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXHWFunctionsWrapper.h 15329 2025-11-24 13:34:32Z AMinor $:

  Description:
    cifX HW API function wrapper

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-11-20  created

**************************************************************************************/
#ifndef CIFX_HWFUNCTIONS_WRAPPER__H
#define CIFX_HWFUNCTIONS_WRAPPER__H

#define DEV_WriteHandshakeFlags(_ch, ...)              CALL_DEV_FUNC(_ch, DEV_WriteHandshakeFlags, _ch, __VA_ARGS__)
#define DEV_ReadHostFlags(_ch, ...)                    CALL_DEV_FUNC(_ch, DEV_ReadHostFlags, _ch, __VA_ARGS__)
#define DEV_ReadHandshakeFlags(_ch, ...)               CALL_DEV_FUNC(_ch, DEV_ReadHandshakeFlags, _ch, __VA_ARGS__)
#define DEV_GetIOBitstate(_ch, ...)                    CALL_DEV_FUNC(_ch, DEV_GetIOBitstate, _ch, __VA_ARGS__)
#define DEV_WaitForBitState(_ch, ...)                  CALL_DEV_FUNC(_ch, DEV_WaitForBitState, _ch, __VA_ARGS__)
#define DEV_WaitForIoBitState(_ch, ...)                CALL_DEV_FUNC(_ch, DEV_WaitForIoBitState, _ch, __VA_ARGS__)
#define DEV_WaitForMbxState(_ch, ...)                  CALL_DEV_FUNC(_ch, DEV_WaitForMbxState, _ch, __VA_ARGS__)
#define DEV_ToggleBit(_ch, ...)                        CALL_DEV_FUNC(_ch, DEV_ToggleBit, _ch, __VA_ARGS__)
#define DEV_ToggleIoAction(_ch, ...)                   CALL_DEV_FUNC(_ch, DEV_ToggleIoAction, _ch, __VA_ARGS__)
#define DEV_WriteCell(_ch, ...)                        CALL_DEV_FUNC(_ch, DEV_WriteCell, _ch, __VA_ARGS__)
#define DEV_WaitForSyncState(_ch, ...)                 CALL_DEV_FUNC(_ch, DEV_WaitForSyncState, _ch, __VA_ARGS__)
#define DEV_ToggleSyncBit(_dev, ...)                   CALL_FUNC(((PDEVICEINSTANCE)_dev), ptDevFun, DEV_ToggleSyncBit, _dev, __VA_ARGS__)
#define DEV_PutPacket(_ch, ...)                        CALL_DEV_FUNC(_ch, DEV_PutPacket, _ch, __VA_ARGS__)
#define DEV_GetPacket(_ch, ...)                        CALL_DEV_FUNC(_ch, DEV_GetPacket, _ch, __VA_ARGS__)
#define DEV_GetMBXState(_ch, ...)                      CALL_DEV_FUNC(_ch, DEV_GetMBXState, _ch, __VA_ARGS__)
#define DEV_IsReady(_ch)                               CALL_DEV_FUNC(_ch, DEV_IsReady, _ch)
#define DEV_IsRunning(_ch)                             CALL_DEV_FUNC(_ch, DEV_IsRunning, _ch)
#define DEV_IsCommunicating(_ch, ...)                  CALL_DEV_FUNC(_ch, DEV_IsCommunicating, _ch, __VA_ARGS__)
#define DEV_WaitForReady_Poll(_ch, ...)                CALL_DEV_FUNC(_ch, DEV_WaitForReady_Poll, _ch, __VA_ARGS__)
#define DEV_WaitForNotReady_Poll(_ch, ...)             CALL_DEV_FUNC(_ch, DEV_WaitForNotReady_Poll, _ch, __VA_ARGS__)
#define DEV_WaitForLock_Poll(_ch, ...)                 CALL_DEV_FUNC(_ch, DEV_WaitForLock_Poll, _ch, __VA_ARGS__)
#define DEV_TriggerWatchdog(_ch, ...)                  CALL_DEV_FUNC(_ch, DEV_TriggerWatchdog, _ch, __VA_ARGS__)
#define DEV_GetHostState(_ch, ...)                     CALL_DEV_FUNC(_ch, DEV_GetHostState, _ch, __VA_ARGS__)
#define DEV_SetHostState(_ch, ...)                     CALL_DEV_FUNC(_ch, DEV_SetHostState, _ch, __VA_ARGS__)
#define DEV_DoChannelInit(_ch, ...)                    CALL_DEV_FUNC(_ch, DEV_DoChannelInit, _ch, __VA_ARGS__)
#define DEV_DoSystemStart(_ch, ...)                    CALL_DEV_FUNC(_ch, DEV_DoSystemStart, _ch, __VA_ARGS__)
#define DEV_DoSystemBootstart(_ch, ...)                CALL_DEV_FUNC(_ch, DEV_DoSystemBootstart, _ch, __VA_ARGS__)
#define DEV_DoUpdateStart(_ch, ...)                    CALL_DEV_FUNC(_ch, DEV_DoUpdateStart, _ch, __VA_ARGS__)
#define DEV_BusState(_ch, ...)                         CALL_DEV_FUNC(_ch, DEV_BusState, _ch, __VA_ARGS__)
#define DEV_DoHostCOSChange(_ch, ...)                  CALL_DEV_FUNC(_ch, DEV_DoHostCOSChange, _ch, __VA_ARGS__)
#define DEV_CheckCOSFlags(_dev)                        (((PDEVICEINSTANCE)_dev)->ptDevFun->pfnDEV_CheckCOSFlags(_dev))
#define DEV_GetHandshakeBitState(_ch, ...)             CALL_DEV_FUNC(_ch, DEV_GetHandshakeBitState, _ch, __VA_ARGS__)
#define DEV_RemoveChannelFiles(_ch, ...)               CALL_DEV_FUNC(_ch, DEV_RemoveChannelFiles, _ch, __VA_ARGS__)
#define DEV_DeleteFile(_ch, ...)                       CALL_DEV_FUNC(_ch, DEV_DeleteFile, _ch, __VA_ARGS__)
#define DEV_CheckForDownload(_ch, ...)                 CALL_DEV_FUNC(_ch, DEV_CheckForDownload, _ch, __VA_ARGS__)
#define DEV_ProcessFWDownload(_dev, ...)               CALL_FUNC(((PDEVICEINSTANCE)_dev), ptDevFun, DEV_ProcessFWDownload, _dev, __VA_ARGS__)
#ifdef CIFX_TOOLKIT_DMA
    #define DEV_DMAState(_ch, ...)                     CALL_DEV_FUNC(_ch, DEV_DMAState, _ch, __VA_ARGS__)
    #define DEV_SetupDMABuffers(_ch)                   CALL_DEV_FUNC(_ch, DEV_SetupDMABuffers, _ch)
#endif

#endif /* CIFX_HWFUNCTIONS_WRAPPER__H */
