/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXFunctions.c 15405 2025-12-12 07:33:32Z AMinor $:

  Description:
    cifX API function implementation

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2023-04-26  - Added new compiler option CIFX_TOOLKIT_USE_CUSTOM_DRV_FUNCS
                - Moved check parameter macros to cifXtoolkit.h
    2022-06-14  - Added option and handling for cached PLC memory pointers
                - Reorganized xChannelPLCMemoryPtr() to handle caching option for IO area 0
    2022-01-18  Fixed xSysdeviceResetEx() to allow additional flags passing to underlying functions
    2021-09-13  Propagate changes of cifXErrors.h (spelling of name
                CIFX_DEV_DMA_HANDSHAKEMODE_NOT_SUPPORTED)
    2020-02-06  xDriverEnumBoards() should return CIFX_NO_MORE_ENTRIES
                if ulBoard exceeds actual board count, instead of invalid board
    2019-10-11  Use internal buffer for endian conversion in xChannelControlBlock()
                Propagate prototype changes of endian conversion function
    2018-11-06  Added new function xSysdeviceResetEx()
    2018-10-10  - Updated header and definitions to new Hilscher defines
                - Derived from cifX Toolkit V1.6.0.0

**************************************************************************************/

/*****************************************************************************/
/*! \file cifXFunctions.c
*   cifX API function implementation                                         */
/*****************************************************************************/

#include "cifXToolkit.h"
#include "cifXErrors.h"
#include "cifXEndianess.h"
#include "cifXFunctionList.h"
#include "cifXHWFunctions.h"
#include "cifXHWFunctionsWrapper.h"
#include "USER_Dependent.h"

#include "Hil_Results.h"
#include "Hil_Packet.h"
#include "Hil_ApplicationCmd.h"
#include "Hil_SystemCmd.h"

/* Commonly used function, not exposed to the user interface */
int32_t cifXStartModule      ( PDEVICEINSTANCE ptDevInstance, uint32_t ulChannelNumber, char* pszModuleName,
                               uint32_t ulFileSize, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);
int32_t cifXReadFirmwareIdent( PDEVICEINSTANCE ptDevInstance, uint32_t ulChannel,
                               PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser);

#ifdef CIFX_TOOLKIT_TIME
void    cifXInitTime         ( PDEVICEINSTANCE ptDevInstance);
#endif

/*****************************************************************************/
/*!  \addtogroup CIFX_DRIVER_API cifX Driver API implementation
*    \{                                                                      */
/*****************************************************************************/

extern const CIFX_ENDIANESS_ENTRY_T g_atSystemInfoBlock[];
extern const uint32_t g_ulSystemInfoBlockSize;

/*****************************************************************************/
/*! Structure description of NETX_SYSTEM_CHANNEL_INFO                        */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atSystemChannelInfo[] =
{
  /* Offset, Width, Elements */
  { 0x04, eCIFX_ENDIANESS_WIDTH_32BIT, 1}, /* ulSizeOfChannel                */
  { 0x08, eCIFX_ENDIANESS_WIDTH_16BIT, 2}, /* usSizeOfMailbox
                                              usMailboxStartOffset           */
};

/*****************************************************************************/
/*! Structure description of NETX_HANDSHAKE_CHANNEL_INFO                     */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atHandshakeChannelInfo[] =
{
  /* Offset, Width, Elements */
  { 0x04, eCIFX_ENDIANESS_WIDTH_32BIT, 1}, /* ulSizeOfChannel                */
};

/*****************************************************************************/
/*! Structure description of NETX_COMMUNICATION_CHANNEL_INFO                 */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atCommChannelInfo[] =
{
  /* Offset, Width, Elements */
  { 0x04, eCIFX_ENDIANESS_WIDTH_32BIT, 1}, /* ulSizeOfChannel                */
  { 0x08, eCIFX_ENDIANESS_WIDTH_16BIT, 3}, /* usCommunicationClass
                                              usProtocolClass
                                              usProtocolConformanceClass     */
};

/*****************************************************************************/
/*! Structure description of NETX_APPLICATION_CHANNEL_INFO                   */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atAppChannelInfo[] =
{
  /* Offset, Width, Elements */
  { 0x04, eCIFX_ENDIANESS_WIDTH_32BIT, 1}, /* ulSizeOfChannel                */
};

/*****************************************************************************/
/*! Structure description of NETX_SYSTEM_CONTROL_BLOCK                       */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atSystemControlBlock[] =
{
  /* Offset, Width, Elements */
  { 0x00, eCIFX_ENDIANESS_WIDTH_32BIT, 2}, /* ulSystemCmdCOS, ulReserved     */
};

/*****************************************************************************/
/*! Structure description of NETX_SYSTEM_STATUS_BLOCK                        */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atSystemStatusBlock[] =
{
  /* Offset, Width, Elements */
  { 0x00, eCIFX_ENDIANESS_WIDTH_32BIT, 5}, /* ulSystem-COS/Status/Error      */
  { 0x14, eCIFX_ENDIANESS_WIDTH_16BIT, 1}, /* usCpuLoad                      */
  { 0x18, eCIFX_ENDIANESS_WIDTH_32BIT, 1}, /* ulHWFeatures                   */
};

/*****************************************************************************/
/*! Structure description of NETX_CONTROL_BLOCK                              */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atControlBlock[] =
{
  /* Offset, Width, Elements */
  { 0x00, eCIFX_ENDIANESS_WIDTH_32BIT, 2}, /* ulApplCos, ulWatchdog          */
};

/*****************************************************************************/
/*! Structure description of NETX_COMMON_STATUS_BLOCK                        */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atCommonStatusBlock[] =
{
  /* Offset, Width, Elements */
  { 0x00, eCIFX_ENDIANESS_WIDTH_32BIT, 3}, /* ulCommCos,ulCommState,
                                              ulCommError                    */
  { 0x0C, eCIFX_ENDIANESS_WIDTH_16BIT, 2}, /* usVersion,usWatchDogTime
                                                                             */
  { 0x14, eCIFX_ENDIANESS_WIDTH_32BIT, 2}, /* ulHostWatchDog, ulErrorCount,
                                                                             */
  { 0x22, eCIFX_ENDIANESS_WIDTH_16BIT, 3}, /* ausReserved[3]
                                                                             */
  { 0x28, eCIFX_ENDIANESS_WIDTH_32BIT, 6}, /* ulSlaveState, ulSlaveErrLogInd
                                              ulNumOfConfigSlaves,ulNumOfActiveSlaves
                                              ulNumOfDiagSlaves, ulReserved  */
};

extern uint32_t                g_ulDeviceCount; /*!< Number of available device (Array size of g_pptDevices) */
extern PDEVICEINSTANCE*        g_pptDevices;    /*!< Array containing all handled device instances           */
extern TKIT_DRIVER_INFORMATION g_tDriverInfo;   /*!< Global driver information                               */


/*****************************************************************************/
/*! Inserts a packet into the System Mailbox
*   \param hSysdevice      Handle to the System device
*   \param ptSendPkt       Packet to send to device
*   \param ulTimeout       maximum time to wait for packet to be accepted
*                          by device (in ms)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxSysdevicePutPacket(CIFXHANDLE hSysdevice, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptSysDevice   = (PCHANNELINSTANCE)hSysdevice;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(ptSendPkt);

  if( !OS_WaitMutex( ptSysDevice->tSendMbx.pvSendMBXMutex, ulTimeout))
    return CIFX_DRV_CMD_ACTIVE;

  lRet = DEV_PutPacket(ptSysDevice, ptSendPkt, ulTimeout);

  OS_ReleaseMutex( ptSysDevice->tSendMbx.pvSendMBXMutex);

  return lRet;
}

/*****************************************************************************/
/*! Retrieves a packet from the System Mailbox
*   \param hSysdevice      Handle to the System device
*   \param ulSize          Size of the buffer to retrieve the packet
*   \param ptRecvPkt       Pointer to buffer for received packet
*   \param ulTimeout       maximum time to wait for packet to be delivered
*                          by device (in ms)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxSysdeviceGetPacket(CIFXHANDLE hSysdevice, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptSysDevice   = (PCHANNELINSTANCE)hSysdevice;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(ptRecvPkt);

  if( !OS_WaitMutex( ptSysDevice->tRecvMbx.pvRecvMBXMutex, ulTimeout))
    return CIFX_DRV_CMD_ACTIVE;

  lRet = DEV_GetPacket(ptSysDevice, ptRecvPkt, ulSize, ulTimeout);

  OS_ReleaseMutex( ptSysDevice->tRecvMbx.pvRecvMBXMutex);

  return lRet;
}

/*****************************************************************************/
/*! Download a file (Firmware, Configuration, etc) to the device
*   \param hSysdevice         Handle to the system device
*   \param ulChannel          Channel number to load the file to
*   \param ulMode             Download mode (DOWNLOAD_MODE_FIRMWARE, etc)
*   \param pszFileName        Name of the file
*   \param pabFileData        Pointer to the file data
*   \param ulFileSize         Length of the file data
*   \param pfnCallback        Callback for progress indication
*                             (NULL for no callback)
*   \param pfnRecvPktCallback Callback Callback pointer for unsolicited receive packets
*                             (NULL for no callback)
*   \param pvUser             User parameter passed to callback
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxSysdeviceDownload(CIFXHANDLE            hSysdevice,
                                              uint32_t              ulChannel,
                                              uint32_t              ulMode,
                                              char*                 pszFileName,
                                              uint8_t*              pabFileData,
                                              uint32_t              ulFileSize,
                                              PFN_PROGRESS_CALLBACK pfnCallback,
                                              PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                              void*                 pvUser)
{
  PCHANNELINSTANCE ptSysDevice    = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE  ptDevInstance  = (PDEVICEINSTANCE)ptSysDevice->pvDeviceInstance;
  uint32_t         ulTransferType = 0;
  int32_t          lRet           = CIFX_NO_ERROR;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(pszFileName);
  CHECK_POINTER(pabFileData);

  switch(ulMode)
  {
  case DOWNLOAD_MODE_FIRMWARE:
    lRet = DEV_GetFWTransferTypeFromFileName(
        ((PDEVICEINSTANCE)(ptSysDevice->pvDeviceInstance))->eChipType,
        pszFileName, &ulTransferType);
    if( CIFX_NO_ERROR != lRet)
      return lRet;

    lRet = DEV_DownloadFile(ptSysDevice,
                            ulChannel,
                            ptSysDevice->tSendMbx.ulSendMailboxLength,
                            ulTransferType,
                            pszFileName,
                            ulFileSize,
                            pabFileData,
                            DEV_TransferPacket,
                            pfnCallback,
                            pfnRecvPktCallback,
                            pvUser);

    break;

  case DOWNLOAD_MODE_CONFIG:
  case DOWNLOAD_MODE_FILE:
      ulTransferType = HIL_FILE_XFER_FILE;
      lRet = DEV_DownloadFile(ptSysDevice,
                              ulChannel,
                              ptSysDevice->tSendMbx.ulSendMailboxLength,
                              ulTransferType,
                              pszFileName,
                              ulFileSize,
                              pabFileData,
                              DEV_TransferPacket,
                              pfnCallback,
                              pfnRecvPktCallback,
                              pvUser);
    break;

  case DOWNLOAD_MODE_LICENSECODE:
      ulTransferType = HIL_FILE_XFER_LICENSE_CODE;

      lRet = DEV_DownloadFile(ptSysDevice,
                              ulChannel,
                              ptSysDevice->tSendMbx.ulSendMailboxLength,
                              ulTransferType,
                              pszFileName,
                              ulFileSize,
                              pabFileData,
                              DEV_TransferPacket,
                              pfnCallback,
                              pfnRecvPktCallback,
                              pvUser);

    break;

  case DOWNLOAD_MODE_MODULE:
    {
      /* We downloading a NXO file */
      PCHANNELINSTANCE  ptChannelInst = NULL;

      /* Check if we have a NXO module file */
      if ( !DEV_IsNXOFile(pszFileName))
      {
        lRet = CIFX_FILE_NAME_INVALID;
      } else if(ulChannel >= ptDevInstance->ulCommChannelCount)
      {
        /* Invalid channel number */
        lRet = CIFX_INVALID_CHANNEL;
      } else
      {
        /* Check if the channel is READY and something is already loaded */
        ptChannelInst = ptDevInstance->pptCommChannels[ulChannel];

        /* Check if we are supporting modules */
        if( !ptDevInstance->fModuleLoad)
        {
          lRet = CIFX_DRV_DOWNLOAD_MODULE_NO_BASEOS;
        } else if( DEV_IsReady(ptChannelInst))
        {
          /* Channel already READY */
          lRet = CIFX_DEV_MODULE_ALREADY_RUNNING;
        } else
        {
          uint8_t bLoadState = CIFXTKIT_DOWNLOAD_NONE;
          if ( CIFX_NO_ERROR == (lRet = DEV_ProcessFWDownload(ptDevInstance,
                                                               ulChannel,
                                                               NULL,
                                                               pszFileName,
                                                               ulFileSize,
                                                               pabFileData,
                                                               &bLoadState,
                                                               DEV_TransferPacket,
                                                               pfnCallback,
                                                               pfnRecvPktCallback,
                                                               pvUser)))
          {
            /* Start module */
            if (CIFX_NO_ERROR == (lRet = cifXStartModule( ptDevInstance, ulChannel, pszFileName,
                                         ulFileSize, pfnRecvPktCallback, pvUser)))
            {
              if ( CIFX_NO_ERROR == (lRet = cifXReadFirmwareIdent( ptDevInstance, ulChannel,
                                                                   pfnRecvPktCallback, pvUser)))
              {
                if ( 0 == (bLoadState & CIFXTKIT_DOWNLOAD_EXECUTED))
                {
                  /* Return download skipped, file exists */
                  lRet = CIFX_DEV_MODULE_ALREADY_EXISTS;
                }
              }
            }
          }
        }
      }
    }
    break;

  default:
    return CIFX_INVALID_PARAMETER;
  }

  return lRet;
}

/*****************************************************************************/
/*! Uploads a file via system channel
*   \param hSysdevice         Handle to the System device
*   \param ulChannel          Channel number to get directory from
*   \param ulMode             Transfer Mode
*   \param pszFileName        Filename to upload
*   \param pabFileData        Pointer to buffer receiving upload
*   \param pulFileSize        [in]Length of buffer, [out] Bytes copied to buffer
*   \param pfnCallback        Callback pointer for progress
*                             (NULL for no callback)
*   \param pfnRecvPktCallback Callback pointer for unsolicited receive packets
*                             (NULL for no callback)
*   \param pvUser             User parameter on callback.
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxSysdeviceUpload(CIFXHANDLE            hSysdevice,
                                            uint32_t              ulChannel,
                                            uint32_t              ulMode,
                                            char*                 pszFileName,
                                            uint8_t*              pabFileData,
                                            uint32_t*             pulFileSize,
                                            PFN_PROGRESS_CALLBACK pfnCallback,
                                            PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                            void*                 pvUser)
{
  PCHANNELINSTANCE ptSysDevice    = (PCHANNELINSTANCE)hSysdevice;
  uint32_t         ulTransferType = 0;
  int32_t          lRet           = CIFX_NO_ERROR;

#ifdef CIFX_TOOLKIT_PARAMETER_CHECK
  if ( (CIFX_NO_ERROR != CheckSysdeviceHandle(hSysdevice)) &&
       (CIFX_NO_ERROR != CheckChannelHandle(hSysdevice))     )
       return CIFX_INVALID_HANDLE;
#endif

  CHECK_POINTER(pszFileName);
  CHECK_POINTER(pabFileData);
  CHECK_POINTER(pulFileSize);

  switch(ulMode)
  {
  case DOWNLOAD_MODE_FIRMWARE:
    lRet = DEV_GetFWTransferTypeFromFileName(
        ((PDEVICEINSTANCE)(ptSysDevice->pvDeviceInstance))->eChipType,
        pszFileName, &ulTransferType);
    if( CIFX_NO_ERROR != lRet)
      return lRet;
    break;

  case DOWNLOAD_MODE_CONFIG:
  case DOWNLOAD_MODE_FILE:
    ulTransferType = HIL_FILE_XFER_FILE;
    break;

  default:
    return CIFX_INVALID_PARAMETER;
  }

  lRet = DEV_UploadFile(ptSysDevice,
                        ulChannel,
                        ptSysDevice->tRecvMbx.ulRecvMailboxLength,
                        ulTransferType,
                        pszFileName,
                        pulFileSize,
                        pabFileData,
                        DEV_TransferPacket,
                        pfnCallback,
                        pfnRecvPktCallback,
                        pvUser);

  return lRet;
}

/*****************************************************************************/
/*! Gets the information of a system device
*   \param hSysdevice   Handle to the system device
*   \param ulCmd        Information to fetch (see defines CIFX_INFO_CMD_SYSTEM_XXX)
*   \param ulSize       Size of the passed structure
*   \param pvInfo       Pointer to the structure for returned data
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxSysdeviceInfo(CIFXHANDLE hSysdevice, uint32_t ulCmd, uint32_t ulSize, void* pvInfo)
{
  int32_t                   lRet         = CIFX_NO_ERROR;
  PCHANNELINSTANCE          ptSysDevice  = (PCHANNELINSTANCE)hSysdevice;
  HIL_DPM_SYSTEM_CHANNEL_T* ptSysChannel = NULL;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(pvInfo);

  ptSysChannel = (HIL_DPM_SYSTEM_CHANNEL_T*)ptSysDevice->pbDPMChannelStart;

  switch(ulCmd)
  {
    case CIFX_INFO_CMD_SYSTEM_INFORMATION:
      if( ulSize < (uint32_t)sizeof(SYSTEM_CHANNEL_SYSTEM_INFORMATION))
      {
        lRet = CIFX_INVALID_BUFFERSIZE;
      } else
      {
        /* Insert global system channel information */
        SYSTEM_CHANNEL_SYSTEM_INFORMATION* ptInfo = (SYSTEM_CHANNEL_SYSTEM_INFORMATION*)pvInfo;

        /* These values are directly read from DPM, so they need to be converted to host endianess */
        ptInfo->ulSystemError   = LE32_TO_HOST(HWIF_READ32(ptSysDevice->pvDeviceInstance, ptSysChannel->tSystemState.ulSystemError));
        ptInfo->ulDpmTotalSize  = LE32_TO_HOST(HWIF_READ32(ptSysDevice->pvDeviceInstance, ptSysChannel->tSystemInfo.ulDpmTotalSize));
        ptInfo->ulDeviceNumber  = LE32_TO_HOST(HWIF_READ32(ptSysDevice->pvDeviceInstance, ptSysChannel->tSystemInfo.ulDeviceNumber));
        ptInfo->ulSerialNumber  = LE32_TO_HOST(HWIF_READ32(ptSysDevice->pvDeviceInstance, ptSysChannel->tSystemInfo.ulSerialNumber));

        ptInfo->ulMBXSize       = ptSysDevice->tRecvMbx.ulRecvMailboxLength;
        ptInfo->ulOpenCnt       = ptSysDevice->ulOpenCount;
      }
    break;

    case CIFX_INFO_CMD_SYSTEM_INFO_BLOCK:
      if( ulSize < (uint32_t)sizeof(SYSTEM_CHANNEL_SYSTEM_INFO_BLOCK))
      {
        lRet = CIFX_INVALID_BUFFERSIZE;
      } else
      {
        uint32_t ulCopyLen = HIL_MIN(ulSize, (uint32_t)sizeof(SYSTEM_CHANNEL_SYSTEM_INFO_BLOCK));

        HWIF_READN(ptSysDevice->pvDeviceInstance, pvInfo, &ptSysChannel->tSystemInfo, ulCopyLen);

        (void)cifXConvertEndianess(0,
                                   pvInfo,
                                   ulCopyLen,
                                   g_atSystemInfoBlock,
                                   g_ulSystemInfoBlockSize);
      }
    break;

    case CIFX_INFO_CMD_SYSTEM_CHANNEL_BLOCK:
      if( ulSize < (uint32_t)sizeof(SYSTEM_CHANNEL_CHANNEL_INFO_BLOCK))
      {
        lRet = CIFX_INVALID_BUFFERSIZE;
      } else
      {
        SYSTEM_CHANNEL_CHANNEL_INFO_BLOCK* ptInfoBuffer = (SYSTEM_CHANNEL_CHANNEL_INFO_BLOCK*) pvInfo; /* use the read buffer for data conversion */

        uint32_t ulCopyLen = HIL_MIN(ulSize, (uint32_t)sizeof(SYSTEM_CHANNEL_CHANNEL_INFO_BLOCK));
        int iChannel;

        HWIF_READN(ptSysDevice->pvDeviceInstance, pvInfo, &ptSysChannel->atChannelInfo[0], ulCopyLen);

        /* Convert channel information structure. This depends on the first byte (bChannelType),
           so we need to parse the whole array */
        for(iChannel = 0;
            iChannel < (int) HIL_CNT_ELEMENT(ptSysChannel->atChannelInfo);
            ++iChannel)
        {
          uint32_t ulBlockLength = (uint32_t)sizeof(ptSysChannel->atChannelInfo[0]);
          uint32_t ulOffset      = (uint32_t)(iChannel * ulBlockLength);

          if( ulOffset > ulCopyLen)
            break;

          if( (ulOffset + sizeof(ptSysChannel->atChannelInfo[0])) > ulCopyLen)
          {
            /* part of block copied, so calculate restlen */
            ulBlockLength = ulCopyLen - ulOffset;
          }

          /* Convert endianess */
          switch(ptInfoBuffer->abInfoBlock[iChannel][0])
          {
          case HIL_CHANNEL_TYPE_SYSTEM:
            (void)cifXConvertEndianess(0,
                                       &ptInfoBuffer->abInfoBlock[iChannel], /*lint !e545 */
                                       ulBlockLength,
                                       s_atSystemChannelInfo,
                                       HIL_CNT_ELEMENT(s_atSystemChannelInfo));
            break;

          case HIL_CHANNEL_TYPE_HANDSHAKE:
            (void)cifXConvertEndianess(0,
                                       &ptInfoBuffer->abInfoBlock[iChannel], /*lint !e545 */
                                       ulBlockLength,
                                       s_atHandshakeChannelInfo,
                                       HIL_CNT_ELEMENT(s_atHandshakeChannelInfo));
            break;

          case HIL_CHANNEL_TYPE_COMMUNICATION:
            (void)cifXConvertEndianess(0,
                                       &ptInfoBuffer->abInfoBlock[iChannel], /*lint !e545 */
                                       ulBlockLength,
                                       s_atCommChannelInfo,
                                       HIL_CNT_ELEMENT(s_atCommChannelInfo));
            break;

          case HIL_CHANNEL_TYPE_APPLICATION:
            (void)cifXConvertEndianess(0,
                                       &ptInfoBuffer->abInfoBlock[iChannel], /*lint !e545 */
                                       ulBlockLength,
                                       s_atAppChannelInfo,
                                       HIL_CNT_ELEMENT(s_atAppChannelInfo));
            break;

          default:
            /* This should never happen */
            break;
          }
        }
      }
    break;

    case CIFX_INFO_CMD_SYSTEM_CONTROL_BLOCK:
      if( ulSize < (uint32_t)sizeof(SYSTEM_CHANNEL_SYSTEM_CONTROL_BLOCK))
      {
        lRet = CIFX_INVALID_BUFFERSIZE;
      } else
      {
        uint32_t ulCopyLen = HIL_MIN(ulSize, (uint32_t)sizeof(SYSTEM_CHANNEL_SYSTEM_CONTROL_BLOCK));
        HWIF_READN(ptSysDevice->pvDeviceInstance, pvInfo, &ptSysChannel->tSystemControl, ulCopyLen);

        (void)cifXConvertEndianess(0,
                                   pvInfo,
                                   ulCopyLen,
                                   s_atSystemControlBlock,
                                   HIL_CNT_ELEMENT(s_atSystemControlBlock));
      }
    break;

    case CIFX_INFO_CMD_SYSTEM_STATUS_BLOCK:
      if( ulSize < (uint32_t)sizeof(SYSTEM_CHANNEL_SYSTEM_STATUS_BLOCK))
      {
        lRet = CIFX_INVALID_BUFFERSIZE;
      } else
      {
        uint32_t ulCopyLen = HIL_MIN(ulSize, (uint32_t)sizeof(SYSTEM_CHANNEL_SYSTEM_STATUS_BLOCK));

        HWIF_READN(ptSysDevice->pvDeviceInstance, pvInfo, &ptSysChannel->tSystemState, ulCopyLen);

        (void)cifXConvertEndianess(0,
                                   pvInfo,
                                   ulCopyLen,
                                   s_atSystemStatusBlock,
                                   HIL_CNT_ELEMENT(s_atSystemStatusBlock));
      }
    break;

    default:
      lRet = CIFX_INVALID_COMMAND;
    break;

  } /* end switch */

  return lRet;
}

/*****************************************************************************/
/*! Download a file (Firmware, Configuration, etc) to the device
*   \param hChannel           Handle to the channel
*   \param ulMode             Download mode (DOWNLOAD_MODE_FIRMWARE, etc)
*   \param pszFileName        Name of the file
*   \param pabFileData        Pointer to the file data
*   \param ulFileSize         Length of the file data
*   \param pfnCallback        Callback for progress indication
*                             (NULL for no callback)
*   \param pfnRecvPktCallback Callback Callback pointer for unsolicited receive packets
*                             (NULL for no callback)
*   \param pvUser             User parameter passed to callback
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelDownload(CIFXHANDLE hChannel,    uint32_t   ulMode,
                                            char*      pszFileName, uint8_t*  pabFileData, uint32_t ulFileSize,
                                            PFN_PROGRESS_CALLBACK pfnCallback, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser)
{
  PCHANNELINSTANCE ptChannel      = (PCHANNELINSTANCE)hChannel;
  uint32_t         ulTransferType = 0;
  int32_t          lRet           = CIFX_NO_ERROR;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pszFileName);
  CHECK_POINTER(pabFileData);

  switch(ulMode)
  {
  case DOWNLOAD_MODE_FIRMWARE:
    lRet = DEV_GetFWTransferTypeFromFileName(
        ((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->eChipType,
        pszFileName, &ulTransferType);
    if( CIFX_NO_ERROR != lRet)
      return lRet;
    break;

  case DOWNLOAD_MODE_CONFIG:
  case DOWNLOAD_MODE_FILE:
    ulTransferType = HIL_FILE_XFER_FILE;
    break;

  case DOWNLOAD_MODE_LICENSECODE:
    ulTransferType = HIL_FILE_XFER_LICENSE_CODE;
    break;

  default:
    return CIFX_INVALID_PARAMETER;
  }

  lRet = DEV_DownloadFile(ptChannel,
                          ptChannel->ulChannelNumber,
                          ptChannel->tSendMbx.ulSendMailboxLength,
                          ulTransferType,
                          pszFileName,
                          ulFileSize,
                          pabFileData,
                          DEV_TransferPacket,
                          pfnCallback,
                          pfnRecvPktCallback,
                          pvUser);

  return lRet;
}

/*****************************************************************************/
/*! Inserts a packet into the channels mailbox
*   \param hChannel   Channel handle acquired by xChannelOpen
*   \param ptSendPkt  Packet to send to channel
*   \param ulTimeout  Time in ms to wait for card to accept the packet
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelPutPacket(CIFXHANDLE hChannel, CIFX_PACKET*  ptSendPkt, uint32_t ulTimeout)
{
  int32_t          lRet      = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hChannel;

  /* Check if another command is active */
  if ( 0 == OS_WaitMutex( ptChannel->tSendMbx.pvSendMBXMutex, ulTimeout))
    return CIFX_DRV_CMD_ACTIVE;

  lRet = DEV_PutPacket(ptChannel, ptSendPkt, ulTimeout);

  /* Release command */
  OS_ReleaseMutex(ptChannel->tSendMbx.pvSendMBXMutex);

  return lRet;
}

/*****************************************************************************/
/*! Gets a packet from the channels mailbox
*   \param hChannel   Channel handle acquired by xChannelOpen
*   \param ulSize     Size of the return packet buffer
*   \param ptRecvPkt  Returned packet
*   \param ulTimeout  Time in ms to wait for available message
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelGetPacket(CIFXHANDLE hChannel, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  /* Check if another command is active */
  if ( 0 == OS_WaitMutex( ptChannel->tRecvMbx.pvRecvMBXMutex, ulTimeout))
    return CIFX_DRV_CMD_ACTIVE;

  lRet = DEV_GetPacket(ptChannel, ptRecvPkt, ulSize, ulTimeout);

  /* Release command */
  OS_ReleaseMutex(ptChannel->tRecvMbx.pvRecvMBXMutex);

  return lRet;
}

/*****************************************************************************/
/*! Gets send packet from the channels mailbox
*   \param hChannel   Channel handle acquired by xChannelOpen
*   \param ulSize     Size of the return packet buffer
*   \param ptRecvPkt  Returned packet
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelGetSendPacket(CIFXHANDLE hChannel, uint32_t ulSize, CIFX_PACKET* ptRecvPkt)
{
  int32_t           lRet        = CIFX_NO_ERROR;
  PCHANNELINSTANCE  ptChannel   = (PCHANNELINSTANCE)hChannel;
  CIFX_PACKET*      ptPacket    = (CIFX_PACKET*)ptChannel->tSendMbx.ptSendMailboxStart->abSendMailbox;
  uint32_t          ulCopySize  = 0;

  ulCopySize  = HWIF_READ32(ptChannel->pvDeviceInstance, ptPacket->tHeader.ulLen) + HIL_PACKET_HEADER_SIZE;

  if( ulCopySize > ulSize)
  {
    /* Use the user buffer length if packet does not fit in the user buffer */
    ulCopySize = ulSize;
    lRet = CIFX_BUFFER_TOO_SHORT;
  }

  /* Just copy the available data into the user buffer */
  HWIF_READN(ptChannel->pvDeviceInstance, ptRecvPkt, ptPacket, ulCopySize);

  return lRet;
}

/*****************************************************************************/
/*! Lock the configuration on a communication channel
*   \param hChannel         Channel handle
*   \param ulCmd            CIFX_CONFIGURATION_XXX defines
*   \param pulState         Return locking state
*   \param ulTimeout        Timeout in [ms]
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelConfigLock(CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pulState);

  /* Check if we are in interrupt mode */
  if(!((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    DEV_ReadHandshakeFlags(ptChannel, 0, 1);

  /* TODO: WAIT until card has recognized the LOCK command */
  UNREFERENCED_PARAMETER(ulTimeout);

  switch (ulCmd)
  {
    case CIFX_CONFIGURATION_LOCK:
    {
      /* Check if the configuration is already LOCKED */
      if( ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_CONFIG_LOCKED)
      {
        /* Configuration already locked */
        *pulState = CIFX_CONFIGURATION_LOCK;
      } else
      {
        lRet = DEV_DoHostCOSChange(ptChannel,
                                   HIL_APP_COS_LOCK_CONFIGURATION | HIL_APP_COS_LOCK_CONFIGURATION_ENABLE,
                                   0,
                                   HIL_APP_COS_LOCK_CONFIGURATION_ENABLE,
                                   CIFX_DEV_CONFIG_LOCK_TIMEOUT,
                                   ulTimeout);

        if(CIFX_NO_ERROR == lRet)
        {
          /* Set actual state */
          *pulState = CIFX_CONFIGURATION_LOCK;
        }
      }
    }
    break;

    case CIFX_CONFIGURATION_UNLOCK:
    {
      /* Check if the configuration is NOT LOCKED  */
      if( !(ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_CONFIG_LOCKED))
      {
        /* Configuration is NOT locked */
        *pulState = CIFX_CONFIGURATION_UNLOCK;
      } else
      {
        lRet = DEV_DoHostCOSChange(ptChannel,
                                   HIL_APP_COS_LOCK_CONFIGURATION_ENABLE,
                                   HIL_APP_COS_LOCK_CONFIGURATION,
                                   HIL_APP_COS_LOCK_CONFIGURATION_ENABLE,
                                   CIFX_DEV_CONFIG_UNLOCK_TIMEOUT,
                                   ulTimeout);

        if(CIFX_NO_ERROR == lRet)
        {
          /* Set actual state */
          *pulState = CIFX_CONFIGURATION_UNLOCK;
        }
      }
    }
    break;

    case CIFX_CONFIGURATION_GETLOCKSTATE:
      /* Get the actual state of the config lock bit */
      if( 0 == (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->ulCommunicationCOS)) & HIL_COMM_COS_CONFIG_LOCKED))
      {
        /* Configuration is not locked */
        *pulState = CIFX_CONFIGURATION_UNLOCK;
      } else
      {
        /* Configuration is locked */
        *pulState = CIFX_CONFIGURATION_LOCK;
      }
    break;

    default:
      /* Unknown command */
      lRet = CIFX_INVALID_COMMAND;
    break;

  }

  return lRet;
}

/*****************************************************************************/
/*! Reads I/O Area information for the given Channel
*   \param hChannel         Channel handle
*   \param ulCmd            CIFX_IO_INPUT_AREA/CIFX_IO_OUTPUT_AREA
*   \param ulAreaNumber     Number of area to get information for
*   \param ulSize           Size of returned data structure
*   \param pvData           Pointer to returned data
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelIOInfo(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulAreaNumber, uint32_t ulSize, void* pvData)
{
  int32_t                 lRet            = CIFX_NO_ERROR;
  PCHANNELINSTANCE        ptChannel       = (PCHANNELINSTANCE)hChannel;
  CHANNEL_IO_INFORMATION* ptIoInformation = (CHANNEL_IO_INFORMATION*)pvData;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvData);

  if(ulSize != sizeof(*ptIoInformation))
    return CIFX_INVALID_BUFFERSIZE;

  if(!DEV_IsRunning(ptChannel))
  {
    lRet = CIFX_DEV_NOT_RUNNING;
  }

  switch(ulCmd)
  {
    case CIFX_IO_INPUT_AREA:
      {
        if(0 == ptChannel->ulIOInputAreas)
        {
          lRet = CIFX_FUNCTION_NOT_AVAILABLE;

        } else if(ulAreaNumber >= ptChannel->ulIOInputAreas)
        {
          lRet = CIFX_INVALID_PARAMETER;
        } else
        {
          PIOINSTANCE ptIoArea = ptChannel->pptIOInputAreas[ulAreaNumber];

          if(HIL_IO_MODE_DEFAULT != HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->bPDInHskMode))
            ptIoInformation->ulIOMode    = HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->bPDInHskMode);
          else
            ptIoInformation->ulIOMode    = ptIoArea->usHandshakeMode;

          ptIoInformation->ulTotalSize   = ptIoArea->ulDPMAreaLength;
        }
      }
      break;

    case CIFX_IO_OUTPUT_AREA:
      {
        if(0 == ptChannel->ulIOOutputAreas)
        {
          lRet = CIFX_FUNCTION_NOT_AVAILABLE;

        } else if(ulAreaNumber >= ptChannel->ulIOOutputAreas)
        {
          lRet = CIFX_INVALID_PARAMETER;
        } else
        {
          PIOINSTANCE ptIoArea = ptChannel->pptIOOutputAreas[ulAreaNumber];

          if(HIL_IO_MODE_DEFAULT != HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->bPDOutHskMode))
            ptIoInformation->ulIOMode    = HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->bPDOutHskMode);
          else
            ptIoInformation->ulIOMode    = ptIoArea->usHandshakeMode;
          ptIoInformation->ulTotalSize   = ptIoArea->ulDPMAreaLength;
        }
      }
      break;

    default:
        lRet = CIFX_INVALID_COMMAND;
      break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Reads the Input data from the channel
*   \param hChannel     Channel handle acquired by xChannelOpen
*   \param ulAreaNumber Number of the I/O Area (0..n)
*   \param ulOffset     Data offset in Input area
*   \param ulDataLen    Length of data to read
*   \param pvData       Buffer to place returned data
*   \param ulTimeout    Timeout in ms to wait for finished I/O Handshake
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelIORead(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData, uint32_t ulTimeout)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  int32_t          lRet          = CIFX_NO_ERROR;
  PIOINSTANCE      ptIOArea      = NULL;
  uint8_t          bIOBitState   = HIL_FLAGS_NONE;

  if(!DEV_IsRunning(ptChannel))
    return CIFX_DEV_NOT_RUNNING;

  if(ulAreaNumber >= ptChannel->ulIOInputAreas)
    return CIFX_INVALID_PARAMETER;

  UNREFERENCED_PARAMETER(ptDevInstance); /* in case CIFX_TOOLKIT_DMA is not set */

  ptIOArea    = ptChannel->pptIOInputAreas[ulAreaNumber];
  bIOBitState = DEV_GetIOBitstate(ptChannel, ptIOArea, 0);

#ifdef CIFX_TOOLKIT_DMA
  /* Check for DMA transfer */
  if( ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_DMA)
  {
    /*------------------------------*/
    /* This is DMA IO data transfer */
    /*------------------------------*/
    /* This is DMACh n */
    uint32_t          ulDMChIdx     = ptChannel->ulChannelNumber * 2 + eDMA_INPUT_BUFFER_IDX;
    PCIFX_DMABUFFER_T ptDmaInfo     = &ptDevInstance->atDmaBuffers[ulDMChIdx];

    if(0 != ulAreaNumber )                                  /* Only support for area 0 in DMA mode */
      return CIFX_DEV_DMA_IO_AREA_NOT_SUPPORTED;

    if( (ulOffset + ulDataLen) > ptDmaInfo->ulSize)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Check if another command is active */
    if ( !OS_WaitMutex( ptIOArea->pvMutex, ulTimeout))
      return CIFX_DRV_CMD_ACTIVE;

    /* TODO: define read procedure ??Toggle -> Read or READ->Toggle */
    if(HIL_FLAGS_NONE == bIOBitState)
    {
      /* Read data without handshake does not work in DMA operation*/
      lRet = CIFX_DEV_DMA_HANDSHAKEMODE_NOT_SUPPORTED;

    } else
    {
      /* Read data */
      if(!DEV_WaitForBitState(ptChannel, ptIOArea->bHandshakeBit, bIOBitState, ulTimeout))
      {
        lRet = CIFX_DEV_EXCHANGE_FAILED;
      } else
      {
        /* Read data */
        OS_Memcpy(  pvData,
                    ((uint8_t*)(ptDmaInfo->pvBuffer)) + ulOffset,
                    ulDataLen);

        /* Lock flag access */
        OS_EnterLock(ptChannel->pvLock);

        /* Read data done */
        DEV_ToggleBit(ptChannel, (uint32_t)(1UL << ptIOArea->bHandshakeBit));

        /* Unlock flag access */
        OS_LeaveLock(ptChannel->pvLock);

        /* Check COMM Flag for return value */
        (void)DEV_IsCommunicating(ptChannel, &lRet);
      }
    }

    /* Release command */
    OS_ReleaseMutex( ptIOArea->pvMutex);

  } else
#endif
  {
    /*---------------------------*/
    /* This is DPM data transfer */
    /*---------------------------*/
    if( (ulOffset + ulDataLen) > ptIOArea->ulDPMAreaLength)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Check if another command is active */
    if ( !OS_WaitMutex( ptIOArea->pvMutex, ulTimeout))
      return CIFX_DRV_CMD_ACTIVE;

    /* Read data */
    if(HIL_FLAGS_NONE == bIOBitState)
    {
      /* Read data */
      HWIF_READN( ptChannel->pvDeviceInstance,
                  pvData,
                  &ptIOArea->pbDPMAreaStart[ulOffset],
                  ulDataLen);

      /* Check COMM Flag for return value */
      (void)DEV_IsCommunicating(ptChannel, &lRet);

    } else if(!DEV_WaitForBitState(ptChannel, ptIOArea->bHandshakeBit, bIOBitState, ulTimeout))
    {
      lRet = CIFX_DEV_EXCHANGE_FAILED;
    } else
    {
      /* Read data */
      HWIF_READN( ptChannel->pvDeviceInstance,
                  pvData,
                  &ptIOArea->pbDPMAreaStart[ulOffset],
                  ulDataLen);

      /* Lock flag access */
      OS_EnterLock(ptChannel->pvLock);

      /* Read data done */
      DEV_ToggleBit(ptChannel, (uint32_t)(1UL << ptIOArea->bHandshakeBit));

      /* Unlock flag access */
      OS_LeaveLock(ptChannel->pvLock);

      /* Check COMM Flag for return value */
      (void)DEV_IsCommunicating(ptChannel, &lRet);
    }

    /* Release command */
    OS_ReleaseMutex( ptIOArea->pvMutex);
  }

  return lRet;
}

/*****************************************************************************/
/*! Writes the Output data to the channel
*   \param hChannel     Channel handle acquired by xChannelOpen
*   \param ulAreaNumber Number of the I/O Area (0..n)
*   \param ulOffset     Data offset in Output area
*   \param ulDataLen    Length of data to send
*   \param pvData       Buffer containing send data
*   \param ulTimeout    Timeout in ms to wait for handshake completion
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelIOWrite(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData, uint32_t ulTimeout)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  int32_t          lRet          = CIFX_NO_ERROR;
  PIOINSTANCE      ptIOArea      = NULL;
  uint8_t          bIOBitState   = HIL_FLAGS_NONE;

  if(!DEV_IsRunning(ptChannel))
    return CIFX_DEV_NOT_RUNNING;

  if(ulAreaNumber >= ptChannel->ulIOOutputAreas)
    return CIFX_INVALID_PARAMETER;

  UNREFERENCED_PARAMETER(ptDevInstance); /* in case CIFX_TOOLKIT_DMA is not set */

  ptIOArea    = ptChannel->pptIOOutputAreas[ulAreaNumber];
  bIOBitState = DEV_GetIOBitstate(ptChannel, ptIOArea, 1);

#ifdef CIFX_TOOLKIT_DMA
  /* Check for DMA transfer */
  if( ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_DMA)
  {
    /*------------------------------*/
    /* This is DMA IO data transfer */
    /*------------------------------*/
    /* This is DMACh n+1 */
    uint32_t          ulDMChIdx     = ptChannel->ulChannelNumber * 2 + eDMA_OUTPUT_BUFFER_IDX;
    PCIFX_DMABUFFER_T ptDmaInfo     = &ptDevInstance->atDmaBuffers[ulDMChIdx];

    if(0 != ulAreaNumber )                                  /* Only support for area 0 in DMA mode */
      return CIFX_DEV_DMA_IO_AREA_NOT_SUPPORTED;

    if( (ulOffset + ulDataLen) > ptDmaInfo->ulSize)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Check if another command is active */
    if ( !OS_WaitMutex( ptIOArea->pvMutex, ulTimeout))
      return CIFX_DRV_CMD_ACTIVE;

    /* Read data */
    /* TODO: define read procedure ??Toggle -> Read or READ->Toggle */
    if(HIL_FLAGS_NONE == bIOBitState)
    {
      /* Read data without handshake does not work in DMA operation*/
      lRet = CIFX_DEV_DMA_HANDSHAKEMODE_NOT_SUPPORTED;

    } else
    {
      if(!DEV_WaitForBitState(ptChannel, ptIOArea->bHandshakeBit, bIOBitState, ulTimeout))
      {
        lRet = CIFX_DEV_EXCHANGE_FAILED;
      } else
      {
        /* Read data */
        OS_Memcpy( (((uint8_t*)(ptDmaInfo->pvBuffer)) + ulOffset),
                   pvData,
                   ulDataLen);

        /* Lock flag access */
        OS_EnterLock(ptChannel->pvLock);

        /* Read data done */
        DEV_ToggleBit(ptChannel, (uint32_t)(1UL << ptIOArea->bHandshakeBit));

        /* Unlock flag access */
        OS_LeaveLock(ptChannel->pvLock);

        /* Check COMM Flag for return value */
        (void)DEV_IsCommunicating(ptChannel, &lRet);
      }
    }

    /* Release command */
    OS_ReleaseMutex( ptIOArea->pvMutex);

  } else
#endif
  {
    /*---------------------------*/
    /* This is DPM data transfer */
    /*---------------------------*/
    if( (ulOffset + ulDataLen) > ptIOArea->ulDPMAreaLength)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Check if another command is active */
    if ( !OS_WaitMutex( ptIOArea->pvMutex, ulTimeout))
      return CIFX_DRV_CMD_ACTIVE;

    /* Read data */
    /* TODO: define write procedure ??Toggle -> Write or Write->Toggle */
    if(HIL_FLAGS_NONE == bIOBitState)
    {
      /* Read data without handshake */
      HWIF_WRITEN(  ptChannel->pvDeviceInstance,
                   &ptIOArea->pbDPMAreaStart[ulOffset],
                    pvData,
                    ulDataLen);

      /* Check COMM Flag for return value */
      (void)DEV_IsCommunicating(ptChannel, &lRet);

    } else
    {
      if(!DEV_WaitForBitState(ptChannel, ptIOArea->bHandshakeBit, bIOBitState, ulTimeout))
      {
        lRet = CIFX_DEV_EXCHANGE_FAILED;
      } else
      {
        /* Read data */
        HWIF_WRITEN(  ptChannel->pvDeviceInstance,
                     &ptIOArea->pbDPMAreaStart[ulOffset],
                      pvData,
                      ulDataLen);

        /* Lock flag access */
        OS_EnterLock(ptChannel->pvLock);

        /* Read data done */
        DEV_ToggleBit(ptChannel, (uint32_t)(1UL << ptIOArea->bHandshakeBit));

        /* Unlock flag access */
        OS_LeaveLock(ptChannel->pvLock);

        /* Check COMM Flag for return value */
        (void)DEV_IsCommunicating(ptChannel, &lRet);
      }
    }

    /* Release command */
    OS_ReleaseMutex( ptIOArea->pvMutex);
  }

  return lRet;
}

/*****************************************************************************/
/*! Read back Send Data Area from channel
*   \param hChannel     Channel handle acquired by xChannelOpen
*   \param ulAreaNumber Number of the I/O Area (0..n)
*   \param ulOffset     Data start offset
*   \param ulDataLen    Data length to read
*   \param pvData       Data buffer
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelIOReadSendData(CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hChannel;
  int32_t          lRet      = CIFX_NO_ERROR;

#ifdef CIFX_TOOLKIT_DMA

  /* Check for DMA transfer */
  if( ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_DMA)
  {
    /* This is DMACh n */
    PDEVICEINSTANCE   ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
    uint32_t          ulDMChIdx     = ptChannel->ulChannelNumber * 2 + eDMA_OUTPUT_BUFFER_IDX;  /* We reading the Output buffer */
    PCIFX_DMABUFFER_T ptDmaInfo     = &ptDevInstance->atDmaBuffers[ulDMChIdx];

    if(0 != ulAreaNumber )                                  /* Only support for area 0 in DMA mode */
      return CIFX_DEV_DMA_IO_AREA_NOT_SUPPORTED;

    if( (ulOffset + ulDataLen) > ptDmaInfo->ulSize)
      return CIFX_INVALID_ACCESS_SIZE;                      /* read size too long */

    /* Read data */
    OS_Memcpy(  pvData,
                ((uint8_t*)(ptDmaInfo->pvBuffer)) + ulOffset,
                ulDataLen);
  } else

#endif

  {
    PIOINSTANCE ptIOArea  = NULL;

    if(ulAreaNumber >= ptChannel->ulIOOutputAreas)
      return CIFX_INVALID_PARAMETER;

    ptIOArea = ptChannel->pptIOOutputAreas[ulAreaNumber];

    if( (ulOffset + ulDataLen) > ptIOArea->ulDPMAreaLength)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Read data */
    HWIF_READN(ptChannel->pvDeviceInstance,
               pvData,
               &ptIOArea->pbDPMAreaStart[ulOffset],
               ulDataLen);
  }

  return lRet;
}

/*****************************************************************************/
/*! Read/Write Control block
*   \param hChannel       Handle to the channel
*   \param ulCmd          CIFX_CMD_READ_DATA/CIFX_CMD_WRITE_DATA
*   \param ulOffset       Start offset of read/write
*   \param ulDataLen      Length of data to read/write
*   \param pvData         Buffer to copy from/to
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelControlBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  int32_t           lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE  ptChannel     = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvData);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

    /* Check if CONTROL block is available */
  } else if(NULL == ptChannel->ptControlBlock)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else
  {
    if ((ulOffset + ulDataLen) > ptChannel->ulControlBlockSize)
      lRet = CIFX_INVALID_ACCESS_SIZE;
    else
    {
      HIL_DPM_CONTROL_BLOCK_T tChannelControlBlockTmp = {0};
      void*   pvDataInternal = pvData;          /* Default to user buffer */
      int32_t lRetTmp        = CIFX_NO_ERROR;   /* Error from the conversion function */

      if(CIFX_CMD_WRITE_DATA == ulCmd)
      {
        /* To prohibit changes to user supplied buffer on write, use a local copy instead */
        pvDataInternal = (void*)(((uint8_t*)&tChannelControlBlockTmp) + ulOffset);
        OS_Memcpy(pvDataInternal, pvData, ulDataLen);
        lRetTmp = cifXConvertEndianess(ulOffset,
                                       pvDataInternal,
                                       ulDataLen,
                                       s_atControlBlock,
                                       HIL_CNT_ELEMENT(s_atControlBlock));
      }

      if(CIFX_NO_ERROR == lRetTmp)
      {
        lRet = DEV_ReadWriteBlock(ptChannel,
                                  (void*)ptChannel->ptControlBlock,
                                  ulOffset,
                                  ptChannel->ulControlBlockSize,
                                  pvDataInternal,
                                  ulDataLen,
                                  ulCmd,
                                  1);
      }

      /* Note: We accept errors from the DEV_ReadWriteBlock() function because we want to inform the user in any case */
      /*       (e.g. CIFX_DEV_NOT_RUNNING) even if we running the conversion on an not filled / empty buffer! */
      if(CIFX_CMD_READ_DATA == ulCmd)
      {
        lRetTmp = cifXConvertEndianess(ulOffset,
                                       pvDataInternal,
                                       ulDataLen,
                                       s_atControlBlock,
                                       HIL_CNT_ELEMENT(s_atControlBlock));
      }

      /* Error of DEV_ReadWriteBlock() takes priority over (possible) Endianess conversion error */
      if((CIFX_NO_ERROR == lRet) && (CIFX_NO_ERROR != lRetTmp))
        lRet = lRetTmp;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Read/Write Common status block
*   \param hChannel       Handle to the channel
*   \param ulCmd          CIFX_CMD_READ_DATA
*   \param ulOffset       Start offset of read
*   \param ulDataLen      Length of data to read
*   \param pvData         Buffer to copy to
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelCommonStatusBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvData);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

    /* Check if STATUS block is available */
  } else if(NULL == ptChannel->ptCommonStatusBlock)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else
  {
    lRet = DEV_ReadWriteBlock(ptChannel,
                              (void*)ptChannel->ptCommonStatusBlock,
                              ulOffset,
                              ptChannel->ulCommonStatusSize,
                              pvData,
                              ulDataLen,
                              ulCmd,
                              0);

    /* Note: We accept errors from the DEV_ReadWriteBlock() function because we want to inform the user in any case */
    /*       (e.g. CIFX_DEV_NOT_RUNNING) even if we running the conversion on an not filled / empty buffer! */
    if(CIFX_CMD_READ_DATA == ulCmd)
    {
      int32_t lRetTmp = cifXConvertEndianess(ulOffset,
                                             pvData,
                                             ulDataLen,
                                             s_atCommonStatusBlock,
                                             HIL_CNT_ELEMENT(s_atCommonStatusBlock));

      /* Error of DEV_ReadWriteBlock() takes priority over (possible) Endianess conversion error */
      if((CIFX_NO_ERROR == lRet) && (CIFX_NO_ERROR != lRetTmp))
        lRet = lRetTmp;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Read Extended status block
*   \param hChannel       Handle to the channel
*   \param ulCmd          CIFX_CMD_READ_DATA
*   \param ulOffset       Start offset of read
*   \param ulDataLen      Length of data to read
*   \param pvData         Buffer to copy to
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelExtendedStatusBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvData);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

    /* Check if CONTROL block is available */
  } else if(NULL == ptChannel->ptExtendedStatusBlock)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else
  {
    lRet = DEV_ReadWriteBlock(ptChannel,
                              (void*)ptChannel->ptExtendedStatusBlock,
                              ulOffset,
                              ptChannel->ulExtendedStatusSize,
                              pvData,
                              ulDataLen,
                              ulCmd,
                              0);
  }

  return lRet;
}

/*****************************************************************************/
/*! Read a user block
*   \param hChannel       Handle to the channel
*   \param ulAreaNumber   Userblock number
*   \param ulCmd          CIFX_CMD_READ_DATA
*   \param ulOffset       Start offset of read
*   \param ulDataLen      Length of data to read
*   \param pvData         Buffer to copy to
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelUserBlock(CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvData);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

    /* Check if CONTROL block is available */
  } else if( (ulAreaNumber >= ptChannel->ulUserAreas) ||
             (NULL == ptChannel->pptUserAreas[ulAreaNumber]) )
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else
  {
    PUSERINSTANCE ptUserInstance = ptChannel->pptUserAreas[ulAreaNumber];

    lRet = DEV_ReadWriteBlock(ptChannel,
                              (void*)ptUserInstance->pbUserBlockStart,
                              ulOffset,
                              ptUserInstance->ulUserBlockLength,
                              pvData,
                              ulDataLen,
                              ulCmd,
                              1);
  }

  return lRet;
}

/*****************************************************************************/
/*! Gets a pointer to an IO Area
*   \param hChannel       Handle to the channel
*   \param ulCmd          CIFX_MEM_PTR_OPEN/CIFX_MEM_PTR_CLOSE
*   \param pvMemoryInfo   Pointer to requested memory structure
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelPLCMemoryPtr(CIFXHANDLE hChannel, uint32_t ulCmd, void* pvMemoryInfo)
{
  PLC_MEMORY_INFORMATION* ptMemory          = (PLC_MEMORY_INFORMATION*)pvMemoryInfo;
  PCHANNELINSTANCE        ptChannel         = (PCHANNELINSTANCE)hChannel;
  int                     fUseCaching       = 0;
  unsigned long           ulAreaDefinition  = 0;
  PDEVICEINSTANCE         ptDevInstance     = NULL;
  int32_t           lRet      = CIFX_NO_ERROR;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvMemoryInfo);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
    return CIFX_DRV_CHANNEL_NOT_INITIALIZED;

  /* No IO blocks defined */
  if( (0 == ptChannel->ulIOInputAreas) ||
      (0 == ptChannel->ulIOOutputAreas)  )
    return CIFX_FUNCTION_NOT_AVAILABLE;

  ptDevInstance     = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  fUseCaching       = ptDevInstance->fCachedMemAccess;
  ulAreaDefinition  = ptMemory->ulAreaDefinition;

#ifdef CIFX_CACHE_TEST
  /* Check for additional option in area definition */
  /* and overwrite if additional flags are set. */
  if (0 != (ulAreaDefinition & ~CIFX_IO_AREA_MASK))
  {
    /* Override caching option to get uncached pointers */
    ulAreaDefinition &= CIFX_IO_AREA_MASK;
    fUseCaching      = 0;
  }
#endif

#ifdef CIFX_TOOLKIT_DMA

  /* Check for DMA transfer */
  if( ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_DMA)
  {
    /*------------------------------*/
    /* This is DMA IO data transfer */
    /*------------------------------*/
    /* This is DMACh n */
    PCIFX_DMABUFFER_T       ptDmaInfo       = NULL;
    CACHED_MEMORY_AREA_T*   ptCachedMemInfo = NULL;
    uint32_t                ulDMChIdx       = 0;

    if(0 != ptMemory->ulAreaNumber)
    {
      /* Invalid IO area */
      lRet = CIFX_DEV_DMA_IO_AREA_NOT_SUPPORTED;  /* Only support for area 0 in DMA mode */
    } else
    {
      if (ulAreaDefinition == CIFX_IO_INPUT_AREA)
      {
        ulDMChIdx       = ptChannel->ulChannelNumber * 2 + eDMA_INPUT_BUFFER_IDX;
        ptDmaInfo       = &ptDevInstance->atDmaBuffers[ulDMChIdx];
        ptCachedMemInfo = &ptChannel->tCachedIOInputArea;
      } else
      {
        ulDMChIdx       = ptChannel->ulChannelNumber * 2 + eDMA_OUTPUT_BUFFER_IDX;
        ptDmaInfo       = &ptDevInstance->atDmaBuffers[ulDMChIdx];
        ptCachedMemInfo = &ptChannel->tCachedIOOutputArea;
      }

      /* Check user command */
      switch(ulCmd)
      {
        case CIFX_MEM_PTR_OPEN:
        {
          void*     pvMappedDPM = NULL;
          void*     pvDPM       = ptDmaInfo->pvBuffer;
          uint32_t  ulDPMSize   = ptDmaInfo->ulSize;

          /* Check for caching option */
          if ((0 != fUseCaching) &&
              (NULL != ptCachedMemInfo->pvMemPtr))
          {
            /* Mapping already exists */
            lRet = CIFX_NO_MORE_ENTRIES;
          } else
          {
            /* Return global memory information */
            if (NULL == (ptMemory->pvMemoryID = OS_MapUserPointer(pvDPM, ulDPMSize, &pvMappedDPM, ptDevInstance->pvOSDependent, (unsigned char)fUseCaching)))
            {
              lRet = CIFX_MEMORY_MAPPING_FAILED;
            } else
            {
              *(ptMemory->ppvMemoryPtr)         = (void*)pvMappedDPM;
              *(ptMemory->pulIOAreaStartOffset) = 0;                    /* In DMA mode, we don't have a DPM start offeset */
              *(ptMemory->pulIOAreaSize)        = ulDPMSize;

              /* Store the memory pointer for later cache operation (e.g. flush) */
              if (0 != fUseCaching)
              {
                ptCachedMemInfo->pvMemPtr   = (void*)pvMappedDPM;
                ptCachedMemInfo->ulAreaSize = ulDPMSize;
              }
            }
          }
        }
        break;

        case CIFX_MEM_PTR_CLOSE:
        {
          if(!OS_UnmapUserPointer(ptMemory->pvMemoryID, ptDevInstance->pvOSDependent))
          {
            lRet = CIFX_INVALID_HANDLE;
          } else
          {
            ptMemory->pvMemoryID              = NULL;
            *(ptMemory->ppvMemoryPtr)         = NULL;
            *(ptMemory->pulIOAreaStartOffset) = 0;
            *(ptMemory->pulIOAreaSize)        = 0;

            if (0 != fUseCaching)
            {
              /* Remove any stored cache buffer information */
              ptCachedMemInfo->pvMemPtr         = NULL;
              ptCachedMemInfo->ulAreaSize       = 0;
            }
          }
        }
        break;

        default:
          lRet = CIFX_INVALID_COMMAND;
        break;

      } /* end switch */
    }
  } else

#endif

  {
    PIOINSTANCE*            pptIOInstances  = NULL;
    uint32_t                ulAreaCount     = 0;
    CACHED_MEMORY_AREA_T* ptCachedMemInfo = NULL;

    if(ulAreaDefinition == CIFX_IO_INPUT_AREA)
    {
      ulAreaCount = ptChannel->ulIOInputAreas;
      pptIOInstances = ptChannel->pptIOInputAreas;
      ptCachedMemInfo = &ptChannel->tCachedIOInputArea;

    } else
    {
      ulAreaCount = ptChannel->ulIOOutputAreas;
      pptIOInstances = ptChannel->pptIOOutputAreas;
      ptCachedMemInfo = &ptChannel->tCachedIOOutputArea;
    }

    /* Check if IO area is available */
    if (ptMemory->ulAreaNumber >= ulAreaCount)
    {
      /* Invalid IO area */
      lRet = CIFX_INVALID_PARAMETER;
    } else
    {
      /* Check user command */
      switch(ulCmd)
      {
        case CIFX_MEM_PTR_OPEN:
        {
          void*    pvMappedDPM  = NULL;
          void*    pvDPM        = pptIOInstances[ptMemory->ulAreaNumber]->pbDPMAreaStart;
          uint32_t ulDPMSize    = pptIOInstances[ptMemory->ulAreaNumber]->ulDPMAreaLength;

          /* Check for caching option */
          if ((0 != fUseCaching) &&
              (NULL != ptCachedMemInfo->pvMemPtr))
          {
            /* Mapping already exists */
            lRet = CIFX_NO_MORE_ENTRIES;

          }else
          {
          /* Return global memory information */
            if (NULL == (ptMemory->pvMemoryID = OS_MapUserPointer(pvDPM, ulDPMSize, &pvMappedDPM, ptDevInstance->pvOSDependent, (unsigned char)fUseCaching)))
          {
            lRet = CIFX_MEMORY_MAPPING_FAILED;
          } else
          {
            *(ptMemory->ppvMemoryPtr)         = (void*)pvMappedDPM;
            *(ptMemory->pulIOAreaStartOffset) = (uint32_t)(pptIOInstances[ptMemory->ulAreaNumber]->pbDPMAreaStart -
                                                                ptChannel->pbDPMChannelStart);
            *(ptMemory->pulIOAreaSize)        = ulDPMSize;

              if (0 != fUseCaching)
              {
                /* Remove any stored cache buffer information */
                ptCachedMemInfo->pvMemPtr   = (void*)pvMappedDPM;
                ptCachedMemInfo->ulAreaSize = ulDPMSize;
              }
            }
          }
        }
        break;

        case CIFX_MEM_PTR_CLOSE:
        {
          if(!OS_UnmapUserPointer(ptMemory->pvMemoryID, ptDevInstance->pvOSDependent))
          {
            lRet = CIFX_INVALID_HANDLE;
          } else
          {
            ptMemory->pvMemoryID              = NULL;
            *(ptMemory->ppvMemoryPtr)         = NULL;
            *(ptMemory->pulIOAreaStartOffset) = 0;
            *(ptMemory->pulIOAreaSize)        = 0;

            if (0 != fUseCaching)
            {
              /* Remove any stored cache buffer information */
              ptCachedMemInfo->pvMemPtr         = NULL;
              ptCachedMemInfo->ulAreaSize       = 0;
            }
          }
        }
        break;

        default:
          lRet = CIFX_INVALID_COMMAND;
        break;
      } /* end switch */
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Checks if the given IO Area is ready for the next handshake
*   \param hChannel       Handle to the channel
*   \param ulAreaNumber   Area to check
*   \param pulReadState   Returned state of the area (!=0 means area is ready)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelPLCIsReadReady(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t* pulReadState)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;
  } else if(0 == ptChannel->ulIOInputAreas)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else
  {
    /* Check if IO area is available */
    if(ulAreaNumber >= ptChannel->ulIOInputAreas)
    {
      lRet = CIFX_INVALID_PARAMETER;
    } else
    {
      /* Check if the device is communication. If only the COM flag is missing, we return the current bit state */
      (void)DEV_IsCommunicating(ptChannel, &lRet);

      if( (lRet != CIFX_DEV_NOT_READY) &&
          (lRet != CIFX_DEV_NOT_RUNNING) )
      {
        /* Read back the send data area */
        PIOINSTANCE ptIOInst    = ptChannel->pptIOInputAreas[ulAreaNumber];
        uint8_t     bIOBitState = DEV_GetIOBitstate(ptChannel, ptIOInst, 0);

        *pulReadState = 0;

        if( (HIL_FLAGS_NONE == bIOBitState)  ||
            (DEV_GetHandshakeBitState(ptChannel, (uint32_t)(1UL << ptIOInst->bHandshakeBit)) == bIOBitState) )
        {
          *pulReadState = 1;

          /* Invalidate the IO input buffer, for cache refresh */
          if (NULL != ptChannel->tCachedIOInputArea.pvMemPtr)
          {
            OS_InvalidateCacheMemory_FromDevice(ptChannel->tCachedIOInputArea.pvMemPtr, ptChannel->tCachedIOInputArea.ulAreaSize);
          }
        }
      }
    }
  }

  /* Assuming that the request was handled */
  return lRet;
}

/*****************************************************************************/
/*! Checks if the given IO Area is ready for the next handshake
*   \param hChannel       Handle to the channel
*   \param ulAreaNumber   Area to check
*   \param pulWriteState  Returned state of the area (!=0 means area is ready)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelPLCIsWriteReady(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t* pulWriteState)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;
  } else if(0 == ptChannel->ulIOOutputAreas)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else
  {
    /* Check if IO area is available */
    if(ulAreaNumber >= ptChannel->ulIOOutputAreas)
    {
      lRet = CIFX_INVALID_PARAMETER;
    } else
    {
      /* Check if the device is communication. If only the COM flag is missing, we return the current bit state */
      (void)DEV_IsCommunicating(ptChannel, &lRet);

      if( (lRet != CIFX_DEV_NOT_READY) &&
          (lRet != CIFX_DEV_NOT_RUNNING) )
      {
        /* Read back the send data area */
        PIOINSTANCE ptIOInst    = ptChannel->pptIOOutputAreas[ulAreaNumber];
        uint8_t     bIOBitState = DEV_GetIOBitstate(ptChannel, ptIOInst, 1);

        *pulWriteState = 0;

        if( (HIL_FLAGS_NONE == bIOBitState)  ||
            (DEV_GetHandshakeBitState(ptChannel, (uint32_t)(1UL << ptIOInst->bHandshakeBit)) == bIOBitState) )
        {
          *pulWriteState = 1;
        }
      }
    }
  }

  /* Assuming that the request was handled */
  return lRet;
}

/*****************************************************************************/
/*! Toggles the Handshake bit for the given IO Output Area
*   \param hChannel       Handle to the channel
*   \param ulAreaNumber   Areanumber
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelPLCActivateWrite(CIFXHANDLE hChannel, uint32_t ulAreaNumber)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;
  } else if(0 == ptChannel->ulIOOutputAreas)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else
  {
    /* Check if IO area is available */
    if(ulAreaNumber >= ptChannel->ulIOOutputAreas)
    {
      lRet = CIFX_INVALID_PARAMETER;
    } else
    {
      /* Check if the device is communication. If only the COM flag is missing, we toggle the bits */
      (void)DEV_IsCommunicating(ptChannel, &lRet);

      if( (lRet != CIFX_DEV_NOT_READY) &&
          (lRet != CIFX_DEV_NOT_RUNNING) )
      {
        PIOINSTANCE ptIOInst = ptChannel->pptIOOutputAreas[ulAreaNumber];

        /* Flush the IO output cache to output buffer */
        if (NULL != ptChannel->tCachedIOOutputArea.pvMemPtr)
        {
          OS_FlushCacheMemory_ToDevice(ptChannel->tCachedIOOutputArea.pvMemPtr, ptChannel->tCachedIOOutputArea.ulAreaSize);
        }

        /* Lock flag access */
        OS_EnterLock(ptChannel->pvLock);

        DEV_ToggleBit(ptChannel, (uint32_t)(1UL << ptIOInst->bHandshakeBit));

        /* Unlock flag access */
        OS_LeaveLock(ptChannel->pvLock);
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Toggles the Handshake bit for the given IO Input Area
*   \param hChannel       Handle to the channel
*   \param ulAreaNumber   Areanumber
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelPLCActivateRead(CIFXHANDLE hChannel, uint32_t ulAreaNumber)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;
  } else if(0 == ptChannel->ulIOInputAreas)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else
  {
    /* Check if IO area is available */
    if(ulAreaNumber >= ptChannel->ulIOInputAreas)
    {
      lRet = CIFX_INVALID_PARAMETER;
    } else
    {
      /* Check if the device is communication. If only the COM flag is missing, we toggle the bits */
      (void)DEV_IsCommunicating(ptChannel, &lRet);

      if( (lRet != CIFX_DEV_NOT_READY) &&
          (lRet != CIFX_DEV_NOT_RUNNING) )
      {
        PIOINSTANCE ptIOInst = ptChannel->pptIOInputAreas[ulAreaNumber];

        /* Lock flag access */
        OS_EnterLock(ptChannel->pvLock);

        DEV_ToggleBit(ptChannel, (uint32_t)(1UL << ptIOInst->bHandshakeBit));

        /* Unlock flag access */
        OS_LeaveLock(ptChannel->pvLock);
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Get Channel information on an open channel
*   \param hChannel       Handle to the channel
*   \param ulSize         Size of return buffer
*   \param pvChannelInfo  Return buffer (CHANNEL_INFORMATION structure)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelInfo(CIFXHANDLE hChannel, uint32_t ulSize, void* pvChannelInfo)
{
  CHANNEL_INFORMATION*  ptChannelInfo = (CHANNEL_INFORMATION*)pvChannelInfo;
  PCHANNELINSTANCE      ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE       ptDevInstance = NULL;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvChannelInfo);

  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  if(ulSize < (uint32_t)sizeof(*ptChannelInfo))
    return CIFX_INVALID_BUFFERSIZE;

  (void)OS_Strncpy(ptChannelInfo->abBoardName,
                   ptDevInstance->szName,
                   (uint32_t)sizeof(ptChannelInfo->abBoardName));
  (void)OS_Strncpy(ptChannelInfo->abBoardAlias,
                   ptDevInstance->szAlias,
                   (uint32_t)sizeof(ptChannelInfo->abBoardAlias));

  ptChannelInfo->ulDeviceNumber   = ptDevInstance->ulDeviceNumber;
  ptChannelInfo->ulSerialNumber   = ptDevInstance->ulSerialNumber;

  ptChannelInfo->usFWMajor        = ptChannel->tFirmwareIdent.tFwVersion.usMajor;
  ptChannelInfo->usFWMinor        = ptChannel->tFirmwareIdent.tFwVersion.usMinor;
  ptChannelInfo->usFWRevision     = ptChannel->tFirmwareIdent.tFwVersion.usRevision;
  ptChannelInfo->usFWBuild        = ptChannel->tFirmwareIdent.tFwVersion.usBuild;
  ptChannelInfo->bFWNameLength    = ptChannel->tFirmwareIdent.tFwName.bNameLength;

  OS_Memcpy(ptChannelInfo->abFWName,
            ptChannel->tFirmwareIdent.tFwName.abName,
            sizeof(ptChannelInfo->abFWName));

  ptChannelInfo->usFWYear         = ptChannel->tFirmwareIdent.tFwDate.usYear;
  ptChannelInfo->bFWMonth         = ptChannel->tFirmwareIdent.tFwDate.bMonth;
  ptChannelInfo->bFWDay           = ptChannel->tFirmwareIdent.tFwDate.bDay;

  ptChannelInfo->ulChannelError   = 0;
  if(0 != ptChannel->ptCommonStatusBlock)
  {
    ptChannelInfo->ulChannelError = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->ulCommunicationError));
  }

  ptChannelInfo->ulOpenCnt        = ptChannel->ulOpenCount;
  ptChannelInfo->ulPutPacketCnt   = ptChannel->tSendMbx.ulSendPacketCnt;
  ptChannelInfo->ulGetPacketCnt   = ptChannel->tRecvMbx.ulRecvPacketCnt;
  ptChannelInfo->ulMailboxSize    = ptChannel->tSendMbx.ulSendMailboxLength;
  ptChannelInfo->ulIOInAreaCnt    = ptChannel->ulIOInputAreas;
  ptChannelInfo->ulIOOutAreaCnt   = ptChannel->ulIOOutputAreas;
  ptChannelInfo->ulHskSize        = ptChannel->bHandshakeWidth;

  /* Check if we are in interrupt mode */
  if(!((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    DEV_ReadHandshakeFlags(ptChannel, 0, 1);

  ptChannelInfo->ulNetxFlags      = ptChannel->usNetxFlags;
  ptChannelInfo->ulHostFlags      = ptChannel->usHostFlags;
  ptChannelInfo->ulHostCOSFlags   = ptChannel->ulHostCOSFlags;
  ptChannelInfo->ulDeviceCOSFlags = ptChannel->ulDeviceCOSFlags;

  return CIFX_NO_ERROR;
}

/*****************************************************************************/
/*! Set/Get Host state of the card
*   \param hChannel       Handle to the channel
*   \param ulCmd          Host state command (CIFX_HOST_STATE_NOT_READY,
*                         CIFX_HOST_STATE_READY, CIFX_HOST_STATE_READ)
*   \param pulState       Returned state if command is CIFX_HOST_STATE_READ
*   \param ulTimeout      Time in ms to wait for start/stop communication
*                         flag
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelHostState(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout)
{
  int32_t lRet = CIFX_INVALID_PARAMETER;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pulState);

  switch(ulCmd)
  {
  case CIFX_HOST_STATE_READ:
    lRet = DEV_GetHostState((PCHANNELINSTANCE)hChannel,
                             pulState);
    break;

  case CIFX_HOST_STATE_READY:
  case CIFX_HOST_STATE_NOT_READY:
    lRet = DEV_SetHostState((PCHANNELINSTANCE)hChannel,
                             ulCmd,
                             ulTimeout);
    break;

  default:
    lRet = CIFX_INVALID_COMMAND;
    break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Uploads a file via Communication channel
*   \param hChannel           Handle to the Channel
*   \param ulMode             Transfer Mode
*   \param pszFileName        Filename to upload
*   \param pabFileData        Pointer to buffer receiving upload
*   \param pulFileSize        [in]Length of buffer, [out] Bytes copied to buffer
*   \param pfnCallback        Callback pointer for progress
*                             (NULL for no callback)
*   \param pfnRecvPktCallback Callback pointer for unsolicited receive packets
*                             (NULL for no callback)
*   \param pvUser             User parameter on callback.
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelUpload(CIFXHANDLE hChannel, uint32_t ulMode,
                                          char* pszFileName, uint8_t* pabFileData, uint32_t* pulFileSize,
                                          PFN_PROGRESS_CALLBACK pfnCallback, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser)
{
  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  return xSysdeviceUpload(hChannel,
                          ptChannel->ulChannelNumber,
                          ulMode,
                          pszFileName,
                          pabFileData,
                          pulFileSize,
                          pfnCallback,
                          pfnRecvPktCallback,
                          pvUser);

}

/*****************************************************************************/
/*! Set DMA state of a communication channel
*   \param hChannel         Channel handle
*   \param ulCmd            Command CIFX_DMA_STATE_xxx
*   \param pulState         Return actual state on CIFX_GET_DMA_STATE
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelDMAState(CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState)
{
#ifdef CIFX_TOOLKIT_DMA

  int32_t          lRet          = CIFX_INVALID_PARAMETER;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = NULL;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pulState);

  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  /* Only possible on PCI devices */
  if( !ptDevInstance->fPCICard)
    return CIFX_FUNCTION_NOT_AVAILABLE;

  /* Only possible if user provided DMA buffers */
  if(0 == ptDevInstance->ulDMABufferCount)
    return CIFX_DEV_DMA_INSUFF_BUFFER_COUNT;

  /* Check if firmware supports DMA functions */

  /*TODO: Check this */
  /*if( !ptChannel->ptCommonStatusBlock->ulCommunicationCOS->fPCICard) */
  /*  return CIFX_FUNCTION_NOT_AVAILABLE; */

  lRet = DEV_DMAState( (PCHANNELINSTANCE)hChannel,
                                          ulCmd,
                                          pulState);

  return lRet;

#else

  UNREFERENCED_PARAMETER(hChannel);
  UNREFERENCED_PARAMETER(ulCmd);
  UNREFERENCED_PARAMETER(pulState);
  return CIFX_FUNCTION_NOT_AVAILABLE; /*lint !e438 : unused variables */

#endif
}

/*****************************************************************************/
/*! Register a callback notification
*   \param hChannel           Handle to the Channel
*   \param ulNotification     Notification
*   \param pfnCallback        Callback function
*   \param pvUser             User data pointer
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelRegisterNotification(CIFXHANDLE           hChannel,
                                                        uint32_t             ulNotification,
                                                        PFN_NOTIFY_CALLBACK  pfnCallback,
                                                        void*                pvUser)
{
  int32_t          lRet             = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel        = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance    = NULL;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pfnCallback);

  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  if(!ptDevInstance->fIrqEnabled)
    return CIFX_INTERRUPT_DISABLED;

  switch (ulNotification)
  {
    case CIFX_NOTIFY_RX_MBX_FULL:
      /* Check if already registered */
      if( NULL != ptChannel->tRecvMbx.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        ptChannel->tRecvMbx.pvUser      = pvUser;
        ptChannel->tRecvMbx.pfnCallback = pfnCallback;

        if(DEV_WaitForBitState(ptChannel,
                               ptChannel->tRecvMbx.bRecvACKBitoffset,
                               HIL_FLAGS_NOT_EQUAL,
                               0))
        {
          CIFX_NOTIFY_RX_MBX_FULL_DATA_T tData;
          tData.ulRecvCount = LE16_TO_HOST(HWIF_READ16(ptDevInstance, ptChannel->tRecvMbx.ptRecvMailboxStart->usWaitingPackages));

          pfnCallback(CIFX_NOTIFY_RX_MBX_FULL, sizeof(tData), &tData, pvUser);
        }
      }
    break;

    case CIFX_NOTIFY_TX_MBX_EMPTY:
      /* Check if already registered */
      if( NULL != ptChannel->tSendMbx.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        ptChannel->tSendMbx.pvUser      = pvUser;
        ptChannel->tSendMbx.pfnCallback = pfnCallback;

        if(DEV_WaitForBitState(ptChannel,
                               ptChannel->tSendMbx.bSendCMDBitoffset,
                               HIL_FLAGS_EQUAL,
                               0))
        {
          CIFX_NOTIFY_TX_MBX_EMPTY_DATA_T tData;
          tData.ulMaxSendCount = LE16_TO_HOST(HWIF_READ16(ptDevInstance, ptChannel->tSendMbx.ptSendMailboxStart->usPackagesAccepted));

          pfnCallback(CIFX_NOTIFY_TX_MBX_EMPTY, sizeof(tData), &tData, pvUser);
        }

      }
    break;

    case CIFX_NOTIFY_PD0_IN:
    case CIFX_NOTIFY_PD0_OUT:
    {
      IOINSTANCE* ptIOArea    = ptChannel->pptIOInputAreas[0];
      uint32_t    ulAreaCount = ptChannel->ulIOInputAreas;
      uint8_t     bIOBitState = DEV_GetIOBitstate(ptChannel, ptIOArea, 0);

      if( CIFX_NOTIFY_PD0_OUT == ulNotification)
      {
        ptIOArea    = ptChannel->pptIOOutputAreas[0];
        ulAreaCount = ptChannel->ulIOOutputAreas;
        bIOBitState = DEV_GetIOBitstate(ptChannel, ptIOArea, 1);
      }

      /* Check if we have one input area */
      if( 0 == ulAreaCount)
      {
        /* No input area */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL != ptIOArea->pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        /* Add the callback */
        ptIOArea->pvUser      = pvUser;
        ptIOArea->pfnCallback = pfnCallback;

        if(DEV_WaitForBitState(ptChannel,
                               ptIOArea->bHandshakeBit,
                               bIOBitState,
                               0))
        {
          pfnCallback(ulNotification, 0, NULL, pvUser);
        }

        lRet = CIFX_NO_ERROR;
      }
    }
    break;

    case CIFX_NOTIFY_PD1_IN:
    case CIFX_NOTIFY_PD1_OUT:
    {
      IOINSTANCE* ptIOArea    = ptChannel->pptIOInputAreas[1];
      uint32_t    ulAreaCount = ptChannel->ulIOInputAreas;
      uint8_t     bIOBitState = DEV_GetIOBitstate(ptChannel, ptIOArea, 0);

      if( CIFX_NOTIFY_PD1_OUT == ulNotification)
      {
        ptIOArea    = ptChannel->pptIOOutputAreas[1];
        ulAreaCount = ptChannel->ulIOOutputAreas;
        bIOBitState = DEV_GetIOBitstate(ptChannel, ptIOArea, 1);
      }

      /* Check if we have two input areas */
      if( 1 <= ulAreaCount)
      {
        /* No input area */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL != ptIOArea->pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        /* Add the callback */
        ptIOArea->pvUser      = pvUser;
        ptIOArea->pfnCallback = pfnCallback;

        if(DEV_WaitForBitState(ptChannel,
                               ptIOArea->bHandshakeBit,
                               bIOBitState,
                               0))
        {
          pfnCallback(ulNotification, 0, NULL, pvUser);
        }
      }
    }
    break;

    case CIFX_NOTIFY_SYNC:
      if( NULL != ptChannel->tSynch.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        /* Add the callback */
        uint8_t bState = HIL_FLAGS_NOT_EQUAL;

        ptChannel->tSynch.pvUser      = pvUser;
        ptChannel->tSynch.pfnCallback = pfnCallback;

        /* Add callback for sync on startup */
        if( HIL_SYNC_MODE_HST_CTRL == HWIF_READ8(ptDevInstance, ptChannel->ptCommonStatusBlock->bSyncHskMode))
          bState = HIL_FLAGS_EQUAL;

        if(DEV_WaitForSyncState(ptChannel,
                                bState,
                                0))
        {
          pfnCallback(ulNotification, 0, NULL, pvUser);
        }

      }
    break;

    case CIFX_NOTIFY_COM_STATE:
      /* Check if already registered */
      if( NULL != ptChannel->tComState.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        CIFX_NOTIFY_COM_STATE_T tData;

        ptChannel->tComState.pvUser      = pvUser;
        ptChannel->tComState.pfnCallback = pfnCallback;

        /* Just update the actual flag state by reading it once */
        (void)DEV_WaitForBitState(ptChannel,
                                  NCF_COMMUNICATING_BIT_NO,
                                  HIL_FLAGS_SET,
                                  0);

        tData.ulComState = ptChannel->usNetxFlags & NCF_COMMUNICATING;
        pfnCallback(CIFX_NOTIFY_COM_STATE, sizeof(tData), &tData, pvUser);
      }
    break;

    default:
      lRet = CIFX_INVALID_COMMAND;
    break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Unregister a callback notification
*   \param hChannel           Handle to the Channel
*   \param ulNotification     Notification
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelUnregisterNotification(CIFXHANDLE hChannel,
                                                          uint32_t   ulNotification)
{
  int32_t lRet = CIFX_NO_ERROR;

  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  switch (ulNotification)
  {
    case CIFX_NOTIFY_RX_MBX_FULL:
      /* Check if already registered */
      if( NULL == ptChannel->tRecvMbx.pfnCallback)
      {
        /* Not registered */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        ptChannel->tRecvMbx.pfnCallback = NULL;
      }
    break;

    case CIFX_NOTIFY_TX_MBX_EMPTY:
      /* Check if already registered */
      if( NULL == ptChannel->tSendMbx.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        ptChannel->tSendMbx.pfnCallback = NULL;
      }
    break;

    case CIFX_NOTIFY_PD0_IN:
    case CIFX_NOTIFY_PD0_OUT:
    {
      IOINSTANCE* ptIOArea    = ptChannel->pptIOInputAreas[0];
      uint32_t    ulAreaCount = ptChannel->ulIOInputAreas;

      if( CIFX_NOTIFY_PD0_OUT == ulNotification)
      {
        ptIOArea    = ptChannel->pptIOOutputAreas[0];
        ulAreaCount = ptChannel->ulIOOutputAreas;
      }

      /* Check if we have one input area */
      if( 0 == ulAreaCount)
      {
        /* No input area */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL == ptIOArea->pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        /* Add the callback */
        ptIOArea->pfnCallback = NULL;
        ptIOArea->pvUser      = NULL;
        lRet = CIFX_NO_ERROR;
      }
    }
    break;

    case CIFX_NOTIFY_PD1_IN:
    case CIFX_NOTIFY_PD1_OUT:
    {
      IOINSTANCE* ptIOArea    = ptChannel->pptIOInputAreas[1];
      uint32_t    ulAreaCount = ptChannel->ulIOInputAreas;

      if( CIFX_NOTIFY_PD1_OUT == ulNotification)
      {
        ptIOArea    = ptChannel->pptIOOutputAreas[1];
        ulAreaCount = ptChannel->ulIOOutputAreas;
      }

      /* Check if we have two input areas */
      if( 1 <= ulAreaCount)
      {
        /* No input area */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL == ptIOArea->pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        /* Add the callback */
        ptIOArea->pfnCallback = NULL;
        ptIOArea->pvUser      = NULL;
      }
    }
    break;

    case CIFX_NOTIFY_SYNC:
      if( NULL == ptChannel->tSynch.pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        /* Add the callback */
        ptChannel->tSynch.pfnCallback = NULL;
        ptChannel->tSynch.pvUser      = NULL;
      }
    break;

    case CIFX_NOTIFY_COM_STATE:
      if( NULL == ptChannel->tComState.pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        /* delete the callback */
        ptChannel->tComState.pfnCallback = NULL;
        ptChannel->tComState.pvUser      = NULL;
      }
    break;

    default:
      lRet = CIFX_INVALID_COMMAND;
    break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Signal a sync state, either a sync command or acknowledge
*   \param hChannel           Handle to the Channel
*   \param ulCmd              Sync command
*   \param ulTimeout          Timeout to wait for sync / sync signalling
*   \param pulErrorCount      Actual sync error counter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY DPMxChannelSyncState(CIFXHANDLE  hChannel,
                                             uint32_t    ulCmd,
                                             uint32_t    ulTimeout,
                                             uint32_t*   pulErrorCount)
{
  int32_t           lRet      = CIFX_NO_ERROR;
  PCHANNELINSTANCE  ptChannel = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pulErrorCount);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;
  } else
  {
    switch (ulCmd)
    {
      case CIFX_SYNC_SIGNAL_CMD:
        /* Check if SYNC mode is host controlled */
        if(HIL_SYNC_MODE_HST_CTRL != HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->bSyncHskMode))
        {
          /* Invalid Device mode */
          lRet = CIFX_DEV_SYNC_STATE_INVALID_MODE;

        } else if(!DEV_WaitForSyncState(ptChannel, HIL_FLAGS_EQUAL, ulTimeout))
        {
          /* Sync cannot be signalled as bits are in wrong state */
          lRet = CIFX_DEV_SYNC_STATE_TIMEOUT;
        } else
        {
          /* Signal new sync */
          DEV_ToggleSyncBit( (PDEVICEINSTANCE)ptChannel->pvDeviceInstance, (1 << ptChannel->ulChannelNumber));

          /* Return actual error counter */
          *pulErrorCount = HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->bErrorSyncCnt);

          /* Check if the device is communication */
          (void)DEV_IsCommunicating(ptChannel, &lRet);
        }
      break;

      case CIFX_SYNC_ACKNOWLEDGE_CMD:
        /* Check if SYNC mode is device controlled */
        if(HIL_SYNC_MODE_DEV_CTRL != HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->bSyncHskMode))
        {
          /* Invalid Device mode */
          lRet = CIFX_DEV_SYNC_STATE_INVALID_MODE;

        } else if(!DEV_WaitForSyncState(ptChannel, HIL_FLAGS_NOT_EQUAL, ulTimeout))
        {
          /* Sync cannot be signalled as bits are in wrong state */
          lRet = CIFX_DEV_SYNC_STATE_TIMEOUT;
        } else
        {
          /* Acknowledge an device sys */
          DEV_ToggleSyncBit( (PDEVICEINSTANCE)ptChannel->pvDeviceInstance, (1 << ptChannel->ulChannelNumber));

          /* Return actual error counter */
          *pulErrorCount = HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->bErrorSyncCnt);

          /* Check if the device is communication */
          (void)DEV_IsCommunicating(ptChannel, &lRet);
        }
      break;

      case CIFX_SYNC_WAIT_CMD:
        {
          if( (HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->bSyncHskMode) != HIL_SYNC_MODE_HST_CTRL) &&
              (HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->bSyncHskMode) != HIL_SYNC_MODE_DEV_CTRL) )
          {
            /* Invalid Device mode */
            lRet = CIFX_DEV_SYNC_STATE_INVALID_MODE;
          } else
          {
            uint8_t bState = HIL_FLAGS_NOT_EQUAL;

            if( HIL_SYNC_MODE_HST_CTRL == HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->bSyncHskMode))
              bState = HIL_FLAGS_EQUAL;

            /* Wait for sync */
            if(!DEV_WaitForSyncState(ptChannel, bState, ulTimeout))
            {
              /* Sync timeout */
              lRet = CIFX_DEV_SYNC_STATE_TIMEOUT;
            } else
            {
              /* Return actual error counter */
              *pulErrorCount = HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->bErrorSyncCnt);

              /* Check if the device is communication */
              (void)DEV_IsCommunicating(ptChannel, &lRet);
            }
          }
        }
      break;

      default:
        lRet = CIFX_INVALID_COMMAND;
      break;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Local structure for cifX API function pointers                           */
/*****************************************************************************/
static CIFX_API_FUNCTION_LIST_T s_tCifxDpmApiFuns =
{
  xDriverOpen,
  xDriverClose,
  xDriverGetInformation,
  xDriverGetErrorDescription,
  xDriverEnumBoards,
  xDriverEnumChannels,
  xDriverMemoryPointer,
  NULL, /* OS specific, implemented by user. */
  xSysdeviceOpen,
  xSysdeviceClose,
  xSysdeviceGetMBXState,
  DPMxSysdevicePutPacket,
  DPMxSysdeviceGetPacket,
  DPMxSysdeviceInfo,
  xSysdeviceFindFirstFile,
  xSysdeviceFindNextFile,
  DPMxSysdeviceDownload,
  DPMxSysdeviceUpload,
  xSysdeviceReset,
  xSysdeviceResetEx,
  xSysdeviceBootstart,
  xSysdeviceExtendedMemory,
  xChannelOpen,
  xChannelClose,
  xChannelFindFirstFile,
  xChannelFindNextFile,
  DPMxChannelDownload,
  DPMxChannelUpload,
  xChannelGetMBXState,
  DPMxChannelPutPacket,
  DPMxChannelGetPacket,
  DPMxChannelGetSendPacket,
  DPMxChannelConfigLock,
  xChannelReset,
  DPMxChannelInfo,
  xChannelWatchdog,
  DPMxChannelHostState,
  xChannelBusState,
  DPMxChannelDMAState,
  DPMxChannelIOInfo,
  DPMxChannelIORead,
  DPMxChannelIOWrite,
  DPMxChannelIOReadSendData,
  DPMxChannelControlBlock,
  DPMxChannelCommonStatusBlock,
  DPMxChannelExtendedStatusBlock,
  DPMxChannelUserBlock,
  DPMxChannelPLCMemoryPtr,
  DPMxChannelPLCIsReadReady,
  DPMxChannelPLCIsWriteReady,
  DPMxChannelPLCActivateWrite,
  DPMxChannelPLCActivateRead,
  DPMxChannelRegisterNotification,
  DPMxChannelUnregisterNotification,
  DPMxChannelSyncState,
};

PCIFX_API_FUNCTION_LIST_T cifXTkitGetDpmApiFunctionList(void)
{
  return &s_tCifxDpmApiFuns;
}

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/
