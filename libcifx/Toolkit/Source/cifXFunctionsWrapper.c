/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXFunctionsWrapper.c 15554 2026-06-12 13:59:57Z MNoll $:

  Description:
    cifX API Wrapper module

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-11-20  created

**************************************************************************************/
#include "cifXFunctionList.h"

int32_t APIENTRY xSysdevicePutPacket(CIFXHANDLE hSysdevice, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout)
{
  CHECK_SYSDEVICEHANDLE(hSysdevice);
  return CALL_API_FUNC(hSysdevice, xSysdevicePutPacket, hSysdevice, ptSendPkt, ulTimeout);
}

int32_t APIENTRY xSysdeviceGetPacket(CIFXHANDLE hSysdevice, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout)
{
  CHECK_SYSDEVICEHANDLE(hSysdevice);
  return CALL_API_FUNC(hSysdevice, xSysdeviceGetPacket, hSysdevice, ulSize, ptRecvPkt, ulTimeout);
}

int32_t APIENTRY xSysdeviceInfo(CIFXHANDLE hSysdevice, uint32_t ulCmd, uint32_t ulSize, void* pvInfo)
{
  CHECK_SYSDEVICEHANDLE(hSysdevice);
  return CALL_API_FUNC(hSysdevice, xSysdeviceInfo, hSysdevice, ulCmd, ulSize, pvInfo);
}

int32_t APIENTRY xSysdeviceExtendedMemory(CIFXHANDLE hSysdevice, uint32_t ulCmd, CIFX_EXTENDED_MEMORY_INFORMATION* ptExtMemInfo)
{
  CHECK_SYSDEVICEHANDLE(hSysdevice);
  return CALL_API_FUNC(hSysdevice, xSysdeviceExtendedMemory, hSysdevice, ulCmd, ptExtMemInfo);
}

int32_t APIENTRY xSysdeviceDownload(CIFXHANDLE            hSysdevice,
                                    uint32_t              ulChannel,
                                    uint32_t              ulMode,
                                    char*                 pszFileName,
                                    uint8_t*              pabFileData,
                                    uint32_t              ulFileSize,
                                    PFN_PROGRESS_CALLBACK pfnCallback,
                                    PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                    void*                 pvUser)
{
  CHECK_SYSDEVICEHANDLE(hSysdevice);
  return CALL_API_FUNC(hSysdevice, xSysdeviceDownload, hSysdevice,
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
                                  void*                 pvUser)
{
#ifdef CIFX_TOOLKIT_PARAMETER_CHECK
  if ( (CIFX_NO_ERROR != CheckSysdeviceHandle(hSysdevice)) &&
       (CIFX_NO_ERROR != CheckChannelHandle(hSysdevice)) )
    return CIFX_INVALID_HANDLE;
#endif
  return CALL_API_FUNC(hSysdevice, xSysdeviceUpload, hSysdevice,
                       ulChannel,
                       ulMode,
                       pszFileName,
                       pabFileData,
                       pulFileSize,
                       pfnCallback,
                       pfnRecvPktCallback,
                       pvUser);
}

int32_t APIENTRY xChannelDownload(CIFXHANDLE            hChannel,
                                  uint32_t              ulMode,
                                  char*                 pszFileName,
                                  uint8_t*              pabFileData,
                                  uint32_t              ulFileSize,
                                  PFN_PROGRESS_CALLBACK pfnCallback,
                                  PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                  void*                 pvUser)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelDownload, hChannel,
                       ulMode,
                       pszFileName,
                       pabFileData,
                       ulFileSize,
                       pfnCallback,
                       pfnRecvPktCallback,
                       pvUser);
}

int32_t APIENTRY xChannelUpload(CIFXHANDLE            hChannel,
                                uint32_t              ulMode,
                                char*                 pszFileName,
                                uint8_t*              pabFileData,
                                uint32_t*             pulFileSize,
                                PFN_PROGRESS_CALLBACK pfnCallback,
                                PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                void*                 pvUser)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelUpload, hChannel,
                       ulMode,
                       pszFileName,
                       pabFileData,
                       pulFileSize,
                       pfnCallback,
                       pfnRecvPktCallback,
                       pvUser);
}

int32_t APIENTRY xChannelPutPacket(CIFXHANDLE hChannel, CIFX_PACKET*  ptSendPkt, uint32_t ulTimeout)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelPutPacket, hChannel, ptSendPkt, ulTimeout);
}

int32_t APIENTRY xChannelGetPacket(CIFXHANDLE hChannel, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelGetPacket, hChannel, ulSize, ptRecvPkt, ulTimeout);
}

int32_t APIENTRY xChannelGetSendPacket(CIFXHANDLE hChannel, uint32_t ulSize, CIFX_PACKET* ptRecvPkt)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelGetSendPacket, hChannel, ulSize, ptRecvPkt);
}

int32_t APIENTRY xChannelConfigLock(CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelConfigLock, hChannel, ulCmd, pulState, ulTimeout);
}

int32_t APIENTRY xChannelInfo(CIFXHANDLE hChannel, uint32_t ulSize, void* pvChannelInfo)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelInfo, hChannel, ulSize, pvChannelInfo);
}

int32_t APIENTRY xChannelHostState(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelHostState, hChannel, ulCmd, pulState, ulTimeout);
}

int32_t APIENTRY xChannelDMAState(CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelDMAState, hChannel, ulCmd, pulState);
}

int32_t APIENTRY xChannelIOInfo(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulAreaNumber, uint32_t ulSize, void* pvData)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelIOInfo, hChannel, ulCmd, ulAreaNumber, ulSize, pvData);
}

int32_t APIENTRY xChannelIOWaitEvent(CIFXHANDLE hChannel, uint32_t ulEvents, uint32_t* pulActiveEvents, uint32_t ulTimeout)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelIOWaitEvent, hChannel, ulEvents, pulActiveEvents, ulTimeout);
}

int32_t APIENTRY xChannelIORead(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData, uint32_t ulTimeout)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelIORead, hChannel, ulAreaNumber, ulOffset, ulDataLen, pvData, ulTimeout);
}

int32_t APIENTRY xChannelIOWrite(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData, uint32_t ulTimeout)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelIOWrite, hChannel, ulAreaNumber, ulOffset, ulDataLen, pvData, ulTimeout);
}

int32_t APIENTRY xChannelIOReadSendData(CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelIOReadSendData, hChannel, ulAreaNumber, ulOffset, ulDataLen, pvData);
}

int32_t APIENTRY xChannelControlBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelControlBlock, hChannel, ulCmd, ulOffset, ulDataLen, pvData);
}

int32_t APIENTRY xChannelCommonStatusBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelCommonStatusBlock, hChannel, ulCmd, ulOffset, ulDataLen, pvData);
}

int32_t APIENTRY xChannelExtendedStatusBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelExtendedStatusBlock, hChannel, ulCmd, ulOffset, ulDataLen, pvData);
}

int32_t APIENTRY xChannelUserBlock(CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelUserBlock, hChannel, ulAreaNumber, ulCmd, ulOffset, ulDataLen, pvData);
}

int32_t APIENTRY xChannelPLCMemoryPtr(CIFXHANDLE hChannel, uint32_t ulCmd, void* pvMemoryInfo)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelPLCMemoryPtr, hChannel, ulCmd, pvMemoryInfo);
}

int32_t APIENTRY xChannelPLCIsReadReady(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t* pulReadState)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelPLCIsReadReady, hChannel, ulAreaNumber, pulReadState);
}

int32_t APIENTRY xChannelPLCIsWriteReady(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t* pulWriteState)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelPLCIsWriteReady, hChannel, ulAreaNumber, pulWriteState);
}

int32_t APIENTRY xChannelPLCActivateWrite(CIFXHANDLE hChannel, uint32_t ulAreaNumber)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelPLCActivateWrite, hChannel, ulAreaNumber);
}

int32_t APIENTRY xChannelPLCActivateRead(CIFXHANDLE hChannel, uint32_t ulAreaNumber)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelPLCActivateRead, hChannel, ulAreaNumber);
}

int32_t APIENTRY xChannelRegisterNotification(CIFXHANDLE          hChannel,
                                              uint32_t            ulNotification,
                                              PFN_NOTIFY_CALLBACK pfnCallback,
                                              void*               pvUser)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelRegisterNotification, hChannel,
                       ulNotification,
                       pfnCallback,
                       pvUser);
}

int32_t APIENTRY xChannelUnregisterNotification(CIFXHANDLE hChannel,
                                                uint32_t   ulNotification)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelUnregisterNotification, hChannel,
                       ulNotification);
}

int32_t APIENTRY xChannelSyncState(CIFXHANDLE hChannel,
                                   uint32_t   ulCmd,
                                   uint32_t   ulTimeout,
                                   uint32_t*  pulErrorCount)
{
  CHECK_CHANNELHANDLE(hChannel);
  return CALL_API_FUNC(hChannel, xChannelSyncState, hChannel,
                       ulCmd,
                       ulTimeout,
                       pulErrorCount);
}
