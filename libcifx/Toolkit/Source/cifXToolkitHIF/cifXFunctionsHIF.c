/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXFunctionsHIF.c 15566 2026-06-18 06:56:55Z RHornung $:

  Description:
    cifX API function implementation

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-05-26  Update to new DPMv2 handling
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

/*****************************************************************************/
/*!  \addtogroup CIFX_DRIVER_API cifX Driver API implementation
*    \{                                                                      */
/*****************************************************************************/

extern const CIFX_ENDIANESS_ENTRY_T g_atSystemInfoBlock[];
extern const uint32_t g_ulSystemInfoBlockSize;

/*****************************************************************************/
/*! Structure description of NETX_SYSTEM_STATUS_BLOCK                        */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atSystemStatusBlock[] =
{
  /* Offset, Width, Elements */
  { 0x00, eCIFX_ENDIANESS_WIDTH_32BIT, 4}, /* Status/Error/Error/Time        */
  { 0x10, eCIFX_ENDIANESS_WIDTH_16BIT, 1}, /* usCpuLoad                      */
  { 0x14, eCIFX_ENDIANESS_WIDTH_32BIT, 1}, /* ulHWFeatures                   */
};

/*****************************************************************************/
/*! Structure description of NETX_COMMON_STATUS_BLOCK                        */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atCommonStatusBlock[] =
{
  /* Offset, Width, Elements */
  { 0x00, eCIFX_ENDIANESS_WIDTH_32BIT, 2}, /* ulCommState,ulCommError        */
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
static int32_t APIENTRY HIFxSysdevicePutPacket(CIFXHANDLE hSysdevice, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptSysDevice   = (PCHANNELINSTANCE)hSysdevice;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(ptSendPkt);

  if( !OS_WaitMutex( ptSysDevice->tFromHostMbx.tSys.pvMutex, ulTimeout))
    return CIFX_DRV_CMD_ACTIVE;

  lRet = DEV_PutPacket(ptSysDevice, ptSendPkt, ulTimeout);

  OS_ReleaseMutex( ptSysDevice->tFromHostMbx.tSys.pvMutex);

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
static int32_t APIENTRY HIFxSysdeviceGetPacket(CIFXHANDLE hSysdevice, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptSysDevice   = (PCHANNELINSTANCE)hSysdevice;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(ptRecvPkt);

  if( !OS_WaitMutex( ptSysDevice->tToHostMbx.tSys.pvMutex, ulTimeout))
    return CIFX_DRV_CMD_ACTIVE;

  lRet = DEV_GetPacket(ptSysDevice, ptRecvPkt, ulSize, ulTimeout);

  OS_ReleaseMutex( ptSysDevice->tToHostMbx.tSys.pvMutex);

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
static int32_t APIENTRY HIFxSysdeviceDownload(CIFXHANDLE            hSysdevice,
                                              uint32_t              ulChannel,
                                              uint32_t              ulMode,
                                              char*                 pszFileName,
                                              uint8_t*              pabFileData,
                                              uint32_t              ulFileSize,
                                              PFN_PROGRESS_CALLBACK pfnCallback,
                                              PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                              void*                 pvUser)
{
  PCHANNELINSTANCE ptSysDevice   = (PCHANNELINSTANCE)hSysdevice;
  int32_t lRet                   = CIFX_NO_ERROR;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(pszFileName);
  CHECK_POINTER(pabFileData);

  switch(ulMode)
  {
    case DOWNLOAD_MODE_FIRMWARE:
    case DOWNLOAD_MODE_CONFIG:
    case DOWNLOAD_MODE_FILE:
        lRet = DEV_DownloadFile(
            hSysdevice,
            ulChannel,
            ptSysDevice->tFromHostMbx.tSys.ulElementSize,
            HIL_FILE_XFER_FILE,
            pszFileName,
            ulFileSize,
            pabFileData,
            DEV_TransferPacket,
            pfnCallback,
            pfnRecvPktCallback,
            pvUser);
      break;

    default:
      lRet = CIFX_INVALID_PARAMETER;
      break;
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
static int32_t APIENTRY HIFxSysdeviceUpload(CIFXHANDLE            hSysdevice,
                                            uint32_t              ulChannel,
                                            uint32_t              ulMode,
                                            char*                 pszFileName,
                                            uint8_t*              pabFileData,
                                            uint32_t*             pulFileSize,
                                            PFN_PROGRESS_CALLBACK pfnCallback,
                                            PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                            void*                 pvUser)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hSysdevice;
  int32_t          lRet          = CIFX_NO_ERROR;

#ifdef CIFX_TOOLKIT_PARAMETER_CHECK
  if ( (CIFX_NO_ERROR != CheckSysdeviceHandle(hSysdevice)) &&
       (CIFX_NO_ERROR != CheckChannelHandle(hSysdevice)) )
       return CIFX_INVALID_HANDLE;
#endif

  CHECK_POINTER(pszFileName);
  CHECK_POINTER(pabFileData);
  CHECK_POINTER(pulFileSize);

  switch(ulMode)
  {
    case DOWNLOAD_MODE_FIRMWARE:
    case DOWNLOAD_MODE_CONFIG:
    case DOWNLOAD_MODE_FILE:
      lRet = DEV_UploadFile(
          hSysdevice,
          ulChannel,
          ptChannel->tToHostMbx.tSys.ulElementSize,
          HIL_FILE_XFER_FILE,
          pszFileName,
          pulFileSize,
          pabFileData,
          DEV_TransferPacket,
          pfnCallback,
          pfnRecvPktCallback,
          pvUser);
      break;

    default:
      lRet = CIFX_INVALID_PARAMETER;
      break;
  }

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
static int32_t APIENTRY HIFxSysdeviceInfo(CIFXHANDLE hSysdevice, uint32_t ulCmd, uint32_t ulSize, void* pvInfo)
{
  int32_t                   lRet         = CIFX_NO_ERROR;
  PCHANNELINSTANCE          ptSysDevice  = (PCHANNELINSTANCE)hSysdevice;
  HIL_HIF_SYSTEM_CHANNEL_T* ptSysChannel = NULL;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(pvInfo);

  ptSysChannel = (HIL_HIF_SYSTEM_CHANNEL_T*)ptSysDevice->pbDPMChannelStart;

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

        ptInfo->ulMBXSize       = ptSysDevice->tToHostMbx.tSys.ulElementSize;
        ptInfo->ulOpenCnt       = ptSysDevice->ulOpenCount;
      }
    break;

    case CIFX_INFO_CMD_SYSTEM_INFO_BLOCK:
      if( ulSize < (uint32_t)sizeof(SYSTEM_CHANNEL_SYSTEM_INFO_BLOCK))
      {
        lRet = CIFX_INVALID_BUFFERSIZE;
      } else
      {
        uint32_t ulCopyLen = HIL_MIN( ulSize,
                                      (uint32_t)sizeof(SYSTEM_CHANNEL_SYSTEM_INFO_BLOCK));

        HWIF_READN(ptSysDevice->pvDeviceInstance, pvInfo, &ptSysChannel->tSystemInfo, ulCopyLen);

        (void)cifXConvertEndianess(0,
                                   pvInfo,
                                   ulCopyLen,
                                   g_atSystemInfoBlock,
                                   g_ulSystemInfoBlockSize);
      }
    break;

    case CIFX_INFO_CMD_SYSTEM_CHANNEL_BLOCK:
      lRet = CIFX_INVALID_COMMAND;
            break;

    case CIFX_INFO_CMD_SYSTEM_CONTROL_BLOCK:
      lRet = CIFX_INVALID_COMMAND;
    break;

    case CIFX_INFO_CMD_SYSTEM_STATUS_BLOCK:
      if( ulSize < (uint32_t)sizeof(SYSTEM_CHANNEL_SYSTEM_STATUS_BLOCK))
      {
        lRet = CIFX_INVALID_BUFFERSIZE;
      } else
      {
        SYSTEM_CHANNEL_SYSTEM_STATUS_BLOCK* ptInfo = (SYSTEM_CHANNEL_SYSTEM_STATUS_BLOCK*) pvInfo; /* 64byte */
        HIL_HIF_SYSTEM_STATUS_BLOCK_T tSystemStatus = {0}; /* 48byte */

        uint32_t ulCopyLen = sizeof(tSystemStatus);

        HWIF_READN(ptSysDevice->pvDeviceInstance, &tSystemStatus, &ptSysChannel->tSystemState, ulCopyLen);

        (void)cifXConvertEndianess(0,
                                   &tSystemStatus,
                                   ulCopyLen,
                                   s_atSystemStatusBlock,
                                   HIL_CNT_ELEMENT(s_atSystemStatusBlock));

        /* For compatibility between DPMv2 and the cifX API
         * some fields have to be moved to different positions. */
        OS_Memset(ptInfo, 0, sizeof(*ptInfo));
        ptInfo->ulSystemCOS       = 0;
        ptInfo->ulSystemStatus    = tSystemStatus.ulSystemStatus;
        ptInfo->ulSystemError     = tSystemStatus.ulSystemError;
        ptInfo->ulBootError       = tSystemStatus.ulBootError;
        ptInfo->ulTimeSinceStart  = tSystemStatus.ulTimeSinceStart;
        ptInfo->usCpuLoad         = tSystemStatus.usCpuLoad;
        ptInfo->ulHWFeatures      = tSystemStatus.ulHWFeatures;
      }
    break;

    default:
      lRet = CIFX_INVALID_COMMAND;
    break;

  } /* end switch */

  return lRet;
}


/*****************************************************************************/
/*! Get/Return a memory pointer to an extended board memory if available
*   \param hSysdevice   Handle to system device
*   \param ulCmd        Command for get/free
*   \param ptExtMemInfo Pointer to a user buffer to return the information
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY HIFxSysdeviceExtendedMemory(CIFXHANDLE hSysdevice, uint32_t ulCmd, CIFX_EXTENDED_MEMORY_INFORMATION* ptExtMemInfo)
{
  UNREFERENCED_PARAMETER(hSysdevice);
  UNREFERENCED_PARAMETER(ulCmd);
  UNREFERENCED_PARAMETER(ptExtMemInfo);
  return CIFX_FUNCTION_NOT_AVAILABLE;
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
static int32_t APIENTRY HIFxChannelDownload(CIFXHANDLE            hChannel,
                                            uint32_t              ulMode,
                                            char*                 pszFileName,
                                            uint8_t*              pabFileData,
                                            uint32_t              ulFileSize,
                                            PFN_PROGRESS_CALLBACK pfnCallback,
                                            PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                            void*                 pvUser)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  int32_t          lRet          = CIFX_NO_ERROR;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pszFileName);
  CHECK_POINTER(pabFileData);

  switch(ulMode)
  {
    case DOWNLOAD_MODE_FIRMWARE:
    case DOWNLOAD_MODE_CONFIG:
    case DOWNLOAD_MODE_FILE:
      lRet = DEV_DownloadFile(
          hChannel,
          ptChannel->ulChannelNumber,
          ptChannel->tFromHostMbx.tCom.tBlock.ulElementSize,
          HIL_FILE_XFER_FILE,
          pszFileName,
          ulFileSize,
          pabFileData,
          DEV_TransferPacket,
          pfnCallback,
          pfnRecvPktCallback,
          pvUser);
      break;

    default:
      lRet = CIFX_INVALID_PARAMETER;
      break;
  }


  return lRet;
}

/*****************************************************************************/
/*! Inserts a packet into the channels mailbox
*   \param hChannel   Channel handle acquired by xChannelOpen
*   \param ptSendPkt  Packet to send to channel
*   \param ulTimeout  Time in ms to wait for card to accept the packet
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY HIFxChannelPutPacket(CIFXHANDLE hChannel, CIFX_PACKET*  ptSendPkt, uint32_t ulTimeout)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  /* Check if another command is active */
  if ( 0 == OS_WaitMutex( ptChannel->tFromHostMbx.tCom.tBlock.pvMutex, ulTimeout))
    return CIFX_DRV_CMD_ACTIVE;

  lRet = DEV_PutPacket(ptChannel, ptSendPkt, ulTimeout);

  /* Release command */
  OS_ReleaseMutex(ptChannel->tFromHostMbx.tCom.tBlock.pvMutex);

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
static int32_t APIENTRY HIFxChannelGetPacket(CIFXHANDLE hChannel, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  /* Check if another command is active */
  if ( 0 == OS_WaitMutex( ptChannel->tToHostMbx.tCom.tBlock.pvMutex, ulTimeout))
    return CIFX_DRV_CMD_ACTIVE;

  lRet = DEV_GetPacket(ptChannel, ptRecvPkt, ulSize, ulTimeout);

  /* Release command */
  OS_ReleaseMutex(ptChannel->tToHostMbx.tCom.tBlock.pvMutex);

  return lRet;
}

/*****************************************************************************/
/*! Gets send packet from the channels mailbox
*   \param hChannel   Channel handle acquired by xChannelOpen
*   \param ulSize     Size of the return packet buffer
*   \param ptRecvPkt  Returned packet
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY HIFxChannelGetSendPacket(CIFXHANDLE hChannel, uint32_t ulSize, CIFX_PACKET* ptRecvPkt)
{
  PCHANNELINSTANCE   ptChannel   = (PCHANNELINSTANCE)hChannel;
  NETX_BLOCK_T*      ptMailbox   = &ptChannel->tFromHostMbx.tCom.tBlock;
  NETX_HS_CTL_T*     ptCtl       = &ptChannel->tFromHostMbx.tCom.tCtl;
  uint32_t           ulHostFlags = ptCtl->ulHostFlags & ~HIL_HIF_MBX_WRAPAROUND;
  CIFX_PACKET*       ptPacket    = (CIFX_PACKET*) (ptMailbox->pbBlockStart + ulHostFlags * ptMailbox->ulElementSize);
  uint32_t           ulCopySize  = 0;
  int32_t            lRet        = CIFX_NO_ERROR;

  CHECK_CHANNELHANDLE(hChannel);

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
static int32_t APIENTRY HIFxChannelConfigLock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t *pulState, uint32_t ulTimeout)
{
  UNREFERENCED_PARAMETER(hChannel);
  UNREFERENCED_PARAMETER(ulCmd);
  UNREFERENCED_PARAMETER(pulState);
  UNREFERENCED_PARAMETER(ulTimeout);
  return CIFX_FUNCTION_NOT_AVAILABLE;
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
static int32_t APIENTRY HIFxChannelIOInfo(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulAreaNumber, uint32_t ulSize, void* pvData)
{
  int32_t                 lRet            = CIFX_NO_ERROR;
  PCHANNELINSTANCE        ptChannel       = (PCHANNELINSTANCE)hChannel;
  CHANNEL_IO_INFORMATION* ptIoInformation = (CHANNEL_IO_INFORMATION*)pvData;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvData);

  if(ulSize != sizeof(*ptIoInformation))
    return CIFX_INVALID_BUFFERSIZE;

  if(!DEV_IsRunning(ptChannel))
    lRet = CIFX_DEV_NOT_RUNNING;

  switch(ulCmd)
  {
    case CIFX_IO_INPUT_AREA:
      if(0 == ptChannel->tIoArea.ulIOInputAreas)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;

      } else if(ulAreaNumber >= ptChannel->tIoArea.ulIOInputAreas)
      {
        lRet = CIFX_INVALID_PARAMETER;
      } else
      {
        ptIoInformation->ulIOMode     = HIL_IO_MODE_BUFF_HST_CTRL;
        ptIoInformation->ulTotalSize  = ptChannel->tIoArea.aptIOInputAreas[ulAreaNumber]->tBlock.ulElementSize;
      }
      break;

    case CIFX_IO_OUTPUT_AREA:
      if(0 == ptChannel->tIoArea.ulIOOutputAreas)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;

      } else if(ulAreaNumber >= ptChannel->tIoArea.ulIOOutputAreas)
      {
        lRet = CIFX_INVALID_PARAMETER;
      } else
      {
        ptIoInformation->ulIOMode     = HIL_IO_MODE_BUFF_HST_CTRL;
        ptIoInformation->ulTotalSize  = ptChannel->tIoArea.aptIOOutputAreas[ulAreaNumber]->tBlock.ulElementSize;
      }
      break;

    default:
        lRet = CIFX_INVALID_COMMAND;
      break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Wait for IO events
*   \param hChannel         Channel handle acquired by xChannelOpen
*   \param ulEvents         Events to wait for
*   \param pulActiveEvents  Buffer for actually set events
*   \param ulTimeout        Timeout in ms to wait for
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY HIFxChannelIOWaitEvent(CIFXHANDLE  hChannel, uint32_t ulEvents, uint32_t* pulActiveEvents, uint32_t ulTimeout)
{
  int32_t  lRet       = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pulActiveEvents);

  if (!DEV_WaitForIoBitState(ptChannel, ulEvents, pulActiveEvents, ulTimeout))
  {
    lRet = CIFX_DEV_SYNC_STATE_TIMEOUT;
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
static int32_t APIENTRY HIFxChannelIORead(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData, uint32_t ulTimeout)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  NETX_IO_BLOCK_T* ptIOArea      = NULL;
  int32_t          lRet          = CIFX_NO_ERROR;
  uint8_t          bStatus       = 0;
#if 0 // TODO
  uint8_t          bIOBitState   = HIL_FLAGS_NONE;
#endif

  CHECK_CHANNELHANDLE(hChannel);

  if(!DEV_IsRunning(ptChannel))
    return CIFX_DEV_NOT_RUNNING;

  if(ulAreaNumber >= ptChannel->tIoArea.ulIOInputAreas)
    return CIFX_INVALID_PARAMETER;

  UNREFERENCED_PARAMETER(ptDevInstance); /* in case CIFX_TOOLKIT_DMA is not set */

  ptIOArea    = ptChannel->tIoArea.aptIOInputAreas[ulAreaNumber];
#if 0 // TODO
  bIOBitState = DEV_GetIOBitstate(ptChannel, ptIOArea, 0);  // TODO this returns HIL_FLAGS_NOT_EQUAL
#endif

#if 0 // TODO #ifdef CIFX_TOOLKIT_DMA
  /* Check for DMA transfer */
  if( ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_DMA)
  {
    /*------------------------------*/
    /* This is DMA IO data transfer */
    /*------------------------------*/
    /* This is DMACh n */
    PDEVICEINSTANCE   ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
    uint32_t          ulDMChIdx     = ptChannel->ulChannelNumber * 2 + eDMA_INPUT_BUFFER_IDX;
    PCIFX_DMABUFFER_T ptDmaInfo     = &ptDevInstance->atDmaBuffers[ulDMChIdx];

    if(0 != ulAreaNumber )                                  /* Only support for area 0 in DMA mode */
      return CIFX_DEV_DMA_IO_AREA_NOT_SUPPORTED;

    if( (ulOffset + ulDataLen) > ptDmaInfo->ulSize)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Check if another command is active */
    if ( !OS_WaitMutex( ptIOArea->pvMutex, ulTimeout))
      return CIFX_DRV_CMD_ACTIVE;

    /* Read data */
    if(!DEV_WaitForBitState(ptChannel, ptIOArea->bHandshakeBit, bIOBitState, ulTimeout))
    {
      lRet = CIFX_DEV_EXCHANGE_FAILED;
    } else
    {
      /* Read data done */
      DEV_ToggleBit(ptChannel, (uint32_t)(1UL << ptIOArea->bHandshakeBit));

      /* Read data */
      OS_Memcpy(  pvData,
                  ((uint8_t*)(ptDmaInfo->pvBuffer)) + ulOffset,
                  ulDataLen);

      /* Check COMM Flag for return value */
      (void)DEV_IsCommunicating(ptChannel, &lRet);
    }

    /* Release command */
    OS_ReleaseMutex( ptIOArea->pvMutex);

  } else
#endif
  {
    /*---------------------------*/
    /* This is DPM data transfer */
    /*---------------------------*/
    ulOffset = ulOffset + HIL_HIF_IO_DATA_OFFSET; /* skip reserved area */
    if( (ulOffset + ulDataLen) > ptIOArea->tBlock.ulElementSize)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Check if another command is active */
    if ( !OS_WaitMutex( ptIOArea->tBlock.pvMutex, ulTimeout))
      return CIFX_DRV_CMD_ACTIVE;

    /* Read data done */
    DEV_ToggleIoAction(ptChannel, ptIOArea);

    /* Read data */
    HWIF_READN( ptChannel->pvDeviceInstance,
                pvData,
                &ptIOArea->tBlock.pbBlockStart[ulOffset],
                ulDataLen);

    bStatus = HWIF_READ8(ptChannel->pvDeviceInstance, *ptIOArea->tIoCtl.pbStatus);
    bStatus = (bStatus & NETX_IO_STATUS_BALANCECNT_MSK)>>1;
    ptIOArea->tStat.ulExchangeCnt++;
    /* Balance count 1 -> Buffer was exchanged once */
    if (1 != bStatus )
    {
      /* Balance count 0, 7,6,5,4 -> Buffer was not exchanged */
      if ((0 == bStatus ) || (4 > bStatus ))
      {
        ptIOArea->tStat.ulErrCnt++;
        ptIOArea->tStat.ulNoUpateCnt++;
      }
      else /* Balance count 2,3 -> Buffer was exchanged 2 or multiple times */
      {
        ptIOArea->tStat.ulErrCnt++;
        ptIOArea->tStat.ulMissDataCnt++;
      }
    }

    /* Check COMM Flag for return value */
    (void)DEV_IsCommunicating(ptChannel, &lRet);

    /* Release command */
    OS_ReleaseMutex( ptIOArea->tBlock.pvMutex);
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
static int32_t APIENTRY HIFxChannelIOWrite(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData, uint32_t ulTimeout)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PNETX_IO_BLOCK_T ptIOArea      = NULL;
  int32_t          lRet          = CIFX_NO_ERROR;
  uint8_t          bStatus       = 0;
#if 0 // TODO
  uint8_t          bIOBitState   = HIL_FLAGS_NONE;
#endif

  CHECK_CHANNELHANDLE(hChannel);

  if(!DEV_IsRunning(ptChannel))
    return CIFX_DEV_NOT_RUNNING;

  if(ulAreaNumber >= ptChannel->tIoArea.ulIOOutputAreas)
    return CIFX_INVALID_PARAMETER;

  ptIOArea    = ptChannel->tIoArea.aptIOOutputAreas[ulAreaNumber];
#if 0 // TODO
  bIOBitState = DEV_GetIOBitstate(ptChannel, ptIOArea, 1);  // TODO this returns HIL_FLAGS_NOT_EQUAL
#endif

#if 0 // TODO #ifdef CIFX_TOOLKIT_DMA
  /* Check for DMA transfer */
  if( ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_DMA)
  {
    /*------------------------------*/
    /* This is DMA IO data transfer */
    /*------------------------------*/
    /* This is DMACh n+1 */
    PDEVICEINSTANCE   ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
    uint32_t          ulDMChIdx     = ptChannel->ulChannelNumber * 2 + eDMA_OUTPUT_BUFFER_IDX;
    PCIFX_DMABUFFER_T ptDmaInfo     = &ptDevInstance->atDmaBuffers[ulDMChIdx];

    if(0 != ulAreaNumber )                                  /* Only support for area 0 in DMA mode */
      return CIFX_DEV_DMA_IO_AREA_NOT_SUPPORTED;

    if( (ulOffset + ulDataLen) > ptDmaInfo->ulSize)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Check if another command is active */
    if ( !OS_WaitMutex( ptIOArea->pvMutex, ulTimeout))
      return CIFX_DRV_CMD_ACTIVE;

    if(!DEV_WaitForBitState(ptChannel, ptIOArea->bHandshakeBit, bIOBitState, ulTimeout))
    {
      lRet = CIFX_DEV_EXCHANGE_FAILED;
    } else
    {
      /* Read data */
      OS_Memcpy( (((uint8_t*)(ptDmaInfo->pvBuffer)) + ulOffset),
                 pvData,
                 ulDataLen);

      /* Read data done */
      DEV_ToggleBit(ptChannel, (uint32_t)(1UL << ptIOArea->bHandshakeBit));

      /* Check COMM Flag for return value */
      (void)DEV_IsCommunicating(ptChannel, &lRet);
    }

    /* Release command */
    OS_ReleaseMutex( ptIOArea->pvMutex);

  } else
#endif
  {
    /*---------------------------*/
    /* This is DPM data transfer */
    /*---------------------------*/
    ulOffset = ulOffset + HIL_HIF_IO_DATA_OFFSET; /* skip reserved area */
    if( (ulOffset + ulDataLen) > ptIOArea->tBlock.ulElementSize)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Check if another command is active */
    if ( !OS_WaitMutex( ptIOArea->tBlock.pvMutex, ulTimeout))
      return CIFX_DRV_CMD_ACTIVE;

    /* Check if buffer is still locked by running DMA (incremental update mode only) */
    if(!DEV_WaitForLock_Poll(ptChannel, ptIOArea, ulTimeout))
    {
      lRet = CIFX_DEV_EXCHANGE_FAILED;
    } else
    {
      /* Write data */
      HWIF_WRITEN(  ptChannel->pvDeviceInstance,
                   &ptIOArea->tBlock.pbBlockStart[ulOffset],
                    pvData,
                    ulDataLen);

      /* Write data done */
      DEV_ToggleIoAction(ptChannel, ptIOArea);

      bStatus = HWIF_READ8(ptChannel->pvDeviceInstance, *ptIOArea->tIoCtl.pbStatus);
      bStatus = (bStatus & NETX_IO_STATUS_BALANCECNT_MSK) >> 1;
      ptIOArea->tStat.ulExchangeCnt++;
      /* Balance count 0 -> previous buffer was consumed */
      if (0 != bStatus)
      {
          /* Balance count 3, 2, 1 -> previous buffer was not consumed */
          if ((3 > bStatus))
          {
              ptIOArea->tStat.ulErrCnt++;
              ptIOArea->tStat.ulMissDataCnt++;
          }
          else /* Balance count 4,5,6,7 -> new buffer was requested without update */
          {
              ptIOArea->tStat.ulErrCnt++;
              ptIOArea->tStat.ulNoUpateCnt++;
          }
      }

      /* Check COMM Flag for return value */
      (void)DEV_IsCommunicating(ptChannel, &lRet);

      /* Release command */
      OS_ReleaseMutex( ptIOArea->tBlock.pvMutex);
    }
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
static int32_t APIENTRY HIFxChannelIOReadSendData(CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hChannel;
  NETX_IO_BLOCK_T* ptIOArea  = NULL;
  int32_t          lRet      = CIFX_NO_ERROR;

  CHECK_CHANNELHANDLE(hChannel);

#if 0 // TODO #ifdef CIFX_TOOLKIT_DMA
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
    if(ulAreaNumber >= ptChannel->tIoArea.ulIOOutputAreas)
      return CIFX_INVALID_PARAMETER;

    ptIOArea = ptChannel->tIoArea.aptIOOutputAreas[ulAreaNumber];

    ulOffset = ulOffset + HIL_HIF_IO_DATA_OFFSET; /* skip reserved area */
    if( (ulOffset + ulDataLen) > ptIOArea->tBlock.ulElementSize)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Read data */
    HWIF_READN(ptChannel->pvDeviceInstance,
               pvData,
               &ptIOArea->tBlock.pbBlockStart[ulOffset],
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
static int32_t APIENTRY HIFxChannelControlBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  UNREFERENCED_PARAMETER(hChannel);
  UNREFERENCED_PARAMETER(ulCmd);
  UNREFERENCED_PARAMETER(ulOffset);
  UNREFERENCED_PARAMETER(ulDataLen);
  UNREFERENCED_PARAMETER(pvData);
  return CIFX_FUNCTION_NOT_AVAILABLE;
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
static int32_t APIENTRY HIFxChannelCommonStatusBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
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
  } else if(NULL == ptChannel->ptCommunicationStatusBlock)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else
  {
    lRet = DEV_ReadWriteBlock(
        ptChannel,
        (void*)ptChannel->ptCommunicationStatusBlock,
        ulOffset,
        sizeof(*ptChannel->ptCommunicationStatusBlock),
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
static int32_t APIENTRY HIFxChannelExtendedStatusBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  UNREFERENCED_PARAMETER(hChannel);
  UNREFERENCED_PARAMETER(ulCmd);
  UNREFERENCED_PARAMETER(ulOffset);
  UNREFERENCED_PARAMETER(ulDataLen);
  UNREFERENCED_PARAMETER(pvData);
  return CIFX_FUNCTION_NOT_AVAILABLE;
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
static int32_t APIENTRY HIFxChannelUserBlock(CIFXHANDLE  hChannel,
                                             uint32_t    ulAreaNumber,
                                             uint32_t    ulCmd,
                                             uint32_t    ulOffset,
                                             uint32_t    ulDataLen,
                                             void*       pvData)
{
  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvData);

  UNREFERENCED_PARAMETER(hChannel);
  UNREFERENCED_PARAMETER(ulAreaNumber);
  UNREFERENCED_PARAMETER(ulCmd);
  UNREFERENCED_PARAMETER(ulOffset);
  UNREFERENCED_PARAMETER(ulDataLen);
  UNREFERENCED_PARAMETER(pvData);

  return CIFX_FUNCTION_NOT_AVAILABLE;
}

/*****************************************************************************/
/*! Gets a pointer to an IO Area
*   \param hChannel       Handle to the channel
*   \param ulCmd          CIFX_MEM_PTR_OPEN/CIFX_MEM_PTR_CLOSE
*   \param pvMemoryInfo   Pointer to requested memory structure
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY HIFxChannelPLCMemoryPtr(CIFXHANDLE hChannel, uint32_t ulCmd, void* pvMemoryInfo)
{
  PLC_MEMORY_INFORMATION* ptMemory          = (PLC_MEMORY_INFORMATION*)pvMemoryInfo;
  PCHANNELINSTANCE        ptChannel         = (PCHANNELINSTANCE)hChannel;
  int                     fUseCaching       = 0;
  unsigned long           ulAreaDefinition  = 0;
  PDEVICEINSTANCE         ptDevInstance     = NULL;
  int32_t                 lRet              = CIFX_NO_ERROR;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvMemoryInfo);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
    return CIFX_DRV_CHANNEL_NOT_INITIALIZED;

  /* No IO blocks defined */
  if( (0 == ptChannel->tIoArea.ulIOInputAreas) ||
      (0 == ptChannel->tIoArea.ulIOOutputAreas)  )
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

#if 0 // #ifdef CIFX_TOOLKIT_DMA

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
    NETX_IO_BLOCK_T**     ptIoBlock       = NULL;
    uint32_t              ulAreaCount     = 0;
    CACHED_MEMORY_AREA_T* ptCachedMemInfo = NULL;

    if(ulAreaDefinition == CIFX_IO_INPUT_AREA)
    {
      ulAreaCount     = ptChannel->tIoArea.ulIOInputAreas;
      ptIoBlock       = ptChannel->tIoArea.aptIOInputAreas;
      ptCachedMemInfo = &ptChannel->tCachedIOInputArea;

    } else
    {
      ulAreaCount     = ptChannel->tIoArea.ulIOOutputAreas;
      ptIoBlock       = ptChannel->tIoArea.aptIOOutputAreas;
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
          void*    pvDPM        = ptIoBlock[ptMemory->ulAreaNumber]->tBlock.pbBlockStart;
          uint32_t ulDPMSize    = ptIoBlock[ptMemory->ulAreaNumber]->tBlock.ulElementSize;

          /* Check for caching option */
          if ((0 != fUseCaching) && (NULL != ptCachedMemInfo->pvMemPtr))
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
              uint32_t ulOffset = HIL_HIF_IO_DATA_OFFSET; /* skip reserved area */

              *(ptMemory->ppvMemoryPtr)         = (void*)(((char*)pvMappedDPM) + ulOffset);
              *(ptMemory->pulIOAreaStartOffset) = (uint32_t)(ptIoBlock[ptMemory->ulAreaNumber]->tBlock.pbBlockStart - ptChannel->pbDPMChannelStart);
              *(ptMemory->pulIOAreaSize)        = ulDPMSize;

              if (0 != fUseCaching)
              {
                /* Remove any stored cache buffer information */
                ptCachedMemInfo->pvMemPtr     = (void*)(((char*)pvMappedDPM) + ulOffset);
                ptCachedMemInfo->ulAreaSize   = ulDPMSize;
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
              ptCachedMemInfo->pvMemPtr       = NULL;
              ptCachedMemInfo->ulAreaSize     = 0;
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
static int32_t APIENTRY HIFxChannelPLCIsReadReady(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t* pulReadState)
{
  int32_t           lRet         = CIFX_NO_ERROR;
  PCHANNELINSTANCE  ptChannel    = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

  } else if(0 == ptChannel->tIoArea.ulIOInputAreas)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else if(ulAreaNumber >= ptChannel->tIoArea.ulIOInputAreas)
  {
    lRet = CIFX_INVALID_PARAMETER;
  } else
  {
    /* Check if the device is communicating. If only the COM flag is missing, we return the current bit state */
    (void)DEV_IsCommunicating(ptChannel, &lRet);

    if( (lRet != CIFX_DEV_NOT_READY) &&
        (lRet != CIFX_DEV_NOT_RUNNING) )
    {
      /* Read back the send data area */
      *pulReadState = 1;

      /* Invalidate the IO input buffer, for cache refresh */
      if (NULL != ptChannel->tCachedIOInputArea.pvMemPtr)
      {
        OS_InvalidateCacheMemory_FromDevice(ptChannel->tCachedIOInputArea.pvMemPtr, ptChannel->tCachedIOInputArea.ulAreaSize);
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
static int32_t APIENTRY HIFxChannelPLCIsWriteReady(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t* pulWriteState)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

  } else if(ulAreaNumber >= 1)
  {
    lRet = CIFX_INVALID_PARAMETER;
  } else
  {
    /* Check if the device is communication. If only the COM flag is missing, we return the current bit state */
    (void)DEV_IsCommunicating(ptChannel, &lRet);

    if( (lRet != CIFX_DEV_NOT_READY) &&
        (lRet != CIFX_DEV_NOT_RUNNING) )
    {
      /* Check if buffer is still locked by running DMA (incremental update mode only) */
      if(DEV_WaitForLock_Poll(ptChannel, ptChannel->tIoArea.aptIOOutputAreas[ulAreaNumber], 0))
      {
        *pulWriteState = 1;
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
static int32_t APIENTRY HIFxChannelPLCActivateWrite(CIFXHANDLE hChannel, uint32_t ulAreaNumber)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

  } else if(0 == ptChannel->tIoArea.ulIOOutputAreas)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else if(ulAreaNumber >= ptChannel->tIoArea.ulIOOutputAreas)
  {
    lRet = CIFX_INVALID_PARAMETER;
  } else
  {
    /* Check if the device is communicating. If only the COM flag is missing, we toggle the bits */
    (void)DEV_IsCommunicating(ptChannel, &lRet);

    if( (lRet != CIFX_DEV_NOT_READY) &&
        (lRet != CIFX_DEV_NOT_RUNNING) )
    {
      /* Flush the IO output cache to output buffer */
      if (NULL != ptChannel->tCachedIOOutputArea.pvMemPtr)
      {
        OS_FlushCacheMemory_ToDevice(ptChannel->tCachedIOOutputArea.pvMemPtr, ptChannel->tCachedIOOutputArea.ulAreaSize);
      }

      /* Write data done */
      DEV_ToggleIoAction(ptChannel, ptChannel->tIoArea.aptIOOutputAreas[ulAreaNumber]);

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
static int32_t APIENTRY HIFxChannelPLCActivateRead(CIFXHANDLE hChannel, uint32_t ulAreaNumber)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

  } else if(0 == ptChannel->tIoArea.ulIOInputAreas)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else if(ulAreaNumber >= ptChannel->tIoArea.ulIOInputAreas)
  {
    lRet = CIFX_INVALID_PARAMETER;
  } else
  {
    /* Check if the device is communicating. If only the COM flag is missing, we toggle the bits */
    (void)DEV_IsCommunicating(ptChannel, &lRet);

    if( (lRet != CIFX_DEV_NOT_READY) &&
        (lRet != CIFX_DEV_NOT_RUNNING) )
    {
      /* Read data done */
      DEV_ToggleIoAction(ptChannel, ptChannel->tIoArea.aptIOInputAreas[ulAreaNumber]);
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
static int32_t APIENTRY HIFxChannelInfo(CIFXHANDLE hChannel, uint32_t ulSize, void* pvChannelInfo)
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
  if(0 != ptChannel->ptCommunicationStatusBlock)
  {
    ptChannelInfo->ulChannelError = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->ulCommunicationError));
  }

  ptChannelInfo->ulOpenCnt        = ptChannel->ulOpenCount;
  ptChannelInfo->ulPutPacketCnt   = ptChannel->tFromHostMbx.tCom.tBlock.ulTransmissionCnt;
  ptChannelInfo->ulGetPacketCnt   = ptChannel->tToHostMbx.tCom.tBlock.ulTransmissionCnt;
  ptChannelInfo->ulMailboxSize    = ptChannel->tFromHostMbx.tCom.tBlock.ulElementSize;
  ptChannelInfo->ulIOInAreaCnt    = ptChannel->tIoArea.ulIOInputAreas;
  ptChannelInfo->ulIOOutAreaCnt   = ptChannel->tIoArea.ulIOOutputAreas;
  ptChannelInfo->ulHskSize        = HIL_HANDSHAKE_SIZE_32BIT;

  /* Check if we are in interrupt mode */
  if(!((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    DEV_ReadHandshakeFlags(ptChannel, 0, 1);

  ptChannelInfo->ulNetxFlags      = ptChannel->tHsCtrl.ulNetxFlags;
  ptChannelInfo->ulHostFlags      = ptChannel->tHsCtrl.ulHostFlags;
  ptChannelInfo->ulHostCOSFlags   = 0;  // TODO get values from commonstate field?
  ptChannelInfo->ulDeviceCOSFlags = 0;  // TODO get values from commonstate field?

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
static int32_t APIENTRY HIFxChannelHostState(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout)
{
  UNREFERENCED_PARAMETER(hChannel);
  UNREFERENCED_PARAMETER(ulCmd);
  UNREFERENCED_PARAMETER(pulState);
  UNREFERENCED_PARAMETER(ulTimeout);
  return CIFX_FUNCTION_NOT_AVAILABLE;
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
static int32_t APIENTRY HIFxChannelUpload(CIFXHANDLE            hChannel,
                                          uint32_t              ulMode,
                                          char*                 pszFileName,
                                          uint8_t*              pabFileData,
                                          uint32_t*             pulFileSize,
                                          PFN_PROGRESS_CALLBACK pfnCallback,
                                          PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                          void*                 pvUser)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  int32_t          lRet          = CIFX_NO_ERROR;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pszFileName);
  CHECK_POINTER(pabFileData);
  CHECK_POINTER(pulFileSize);

  switch(ulMode)
  {
    case DOWNLOAD_MODE_FIRMWARE:
    case DOWNLOAD_MODE_CONFIG:
    case DOWNLOAD_MODE_FILE:
      lRet = DEV_UploadFile(
          ptChannel,
          ptChannel->ulChannelNumber,
          ptChannel->tToHostMbx.tCom.tBlock.ulElementSize,
          HIL_FILE_XFER_FILE,
          pszFileName,
          pulFileSize,
          pabFileData,
          DEV_TransferPacket,
          pfnCallback,
          pfnRecvPktCallback,
          pvUser);
      break;

    default:
      lRet = CIFX_INVALID_PARAMETER;
      break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Set DMA state of a communication channel
*   \param hChannel         Channel handle
*   \param ulCmd            Command CIFX_DMA_STATE_xxx
*   \param pulState         Return actual state on CIFX_GET_DMA_STATE
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY HIFxChannelDMAState(CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState)
{
#if 0 // TODO #ifdef CIFX_TOOLKIT_DMA

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
  /*if( !ptChannel->ptCommunicationStatusBlock->ulCommunicationCOS->fPCICard) */
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
static int32_t APIENTRY HIFxChannelRegisterNotification(CIFXHANDLE           hChannel,
                                                        uint32_t             ulNotification,
                                                        PFN_NOTIFY_CALLBACK  pfnCallback,
                                                        void*                pvUser)
{
  int32_t          lRet             = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel        = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance    = NULL;
  uint32_t         ulBitState       = 0;
  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pfnCallback);

  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  if(!ptDevInstance->fIrqEnabled)
    return CIFX_INTERRUPT_DISABLED;

  switch (ulNotification)
  {
    case CIFX_NOTIFY_RX_MBX_FULL:
      /* Check if already registered */
      if( NULL != ptChannel->tToHostMbx.tCom.tCtl.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        ptChannel->tToHostMbx.tCom.tCtl.pvUser      = pvUser;
        ptChannel->tToHostMbx.tCom.tCtl.pfnCallback = pfnCallback;

        if(DEV_WaitForMbxState(ptChannel, &ptChannel->tToHostMbx.tCom, NETX_MBX_COM_STATE_FULL, 0))
        {
          CIFX_NOTIFY_RX_MBX_FULL_DATA_T tData = {
            .ulRecvCount = DEV_GetMBXFillLevel(&ptChannel->tToHostMbx.tCom)
          };
          pfnCallback(CIFX_NOTIFY_RX_MBX_FULL, sizeof(tData), &tData, pvUser);
        }
      }
    break;

    case CIFX_NOTIFY_TX_MBX_EMPTY:
      /* Check if already registered */
      if( NULL != ptChannel->tFromHostMbx.tCom.tCtl.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        ptChannel->tFromHostMbx.tCom.tCtl.pvUser      = pvUser;
        ptChannel->tFromHostMbx.tCom.tCtl.pfnCallback = pfnCallback;

        if(DEV_WaitForMbxState(ptChannel, &ptChannel->tFromHostMbx.tCom, NETX_MBX_COM_STATE_EMPTY, 0))
        {
          CIFX_NOTIFY_TX_MBX_EMPTY_DATA_T tData = {
            .ulMaxSendCount = ptChannel->tFromHostMbx.tCom.tBlock.ulElementCnt - DEV_GetMBXFillLevel(&ptChannel->tFromHostMbx.tCom)
          };
          pfnCallback(CIFX_NOTIFY_TX_MBX_EMPTY, sizeof(tData), &tData, pvUser);
        }
      }
    break;

    case CIFX_NOTIFY_PD0_IN:
    case CIFX_NOTIFY_PD0_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[0];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD0_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[0];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have one area */
      if( 0 == ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL != ptBlock->tIoCtl.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        /* Add the callback */
        ptBlock->tIoCtl.pvUser      = pvUser;
        ptBlock->tIoCtl.pfnCallback = pfnCallback;

        if(DEV_WaitForIoBitState(ptChannel, ptBlock->tBlock.ulBitmask, &ulBitState, 0))
        {
          pfnCallback(ulNotification, 0, NULL, pvUser);
        }
      }
    }
    break;

    case CIFX_NOTIFY_PD1_IN:
    case CIFX_NOTIFY_PD1_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[1];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD1_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[1];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have two areas */
      if( 2 > ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL != ptBlock->tIoCtl.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        /* Add the callback */
        ptBlock->tIoCtl.pvUser      = pvUser;
        ptBlock->tIoCtl.pfnCallback = pfnCallback;

        if(DEV_WaitForIoBitState(ptChannel, ptBlock->tBlock.ulBitmask, &ulBitState, 0))
        {
          pfnCallback(ulNotification, 0, NULL, pvUser);
        }
      }
    }
    break;

    case CIFX_NOTIFY_PD2_IN:
    case CIFX_NOTIFY_PD2_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[2];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD2_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[2];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have three areas */
      if( 3 > ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL != ptBlock->tIoCtl.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        /* Add the callback */
        ptBlock->tIoCtl.pvUser      = pvUser;
        ptBlock->tIoCtl.pfnCallback = pfnCallback;

        if(DEV_WaitForIoBitState(ptChannel, ptBlock->tBlock.ulBitmask, &ulBitState, 0))
        {
          pfnCallback(ulNotification, 0, NULL, pvUser);
        }
      }
    }
    break;

    case CIFX_NOTIFY_PD3_IN:
    case CIFX_NOTIFY_PD3_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[3];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD3_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[3];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have four areas */
      if( 4 > ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL != ptBlock->tIoCtl.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        /* Add the callback */
        ptBlock->tIoCtl.pvUser      = pvUser;
        ptBlock->tIoCtl.pfnCallback = pfnCallback;

        if(DEV_WaitForIoBitState(ptChannel, ptBlock->tBlock.ulBitmask, &ulBitState, 0))
        {
          pfnCallback(ulNotification, 0, NULL, pvUser);
        }
      }
    }
    break;

    case CIFX_NOTIFY_SYNC:
      if( NULL != ptChannel->atSynch[0].pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        /* Add the callback */
        ptChannel->atSynch[0].pvUser      = pvUser;
        ptChannel->atSynch[0].pfnCallback = pfnCallback;
        if(DEV_WaitForIoBitState(ptChannel, ptChannel->atSynch[0].ulBitmask, &ulBitState, 0))
        {
          pfnCallback(ulNotification, 0, NULL, pvUser);
        }
      }
    break;

    case CIFX_NOTIFY_SYNC1:
      if( NULL != ptChannel->atSynch[1].pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        /* Add the callback */
        ptChannel->atSynch[1].pvUser      = pvUser;
        ptChannel->atSynch[1].pfnCallback = pfnCallback;
        if(DEV_WaitForIoBitState(ptChannel, ptChannel->atSynch[1].ulBitmask, &ulBitState, 0))
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
        (void)DEV_WaitForBitState(
            ptChannel,
            NCF_COMMUNICATING_BIT_NO,
            HIL_FLAGS_SET,
            0);

        tData.ulComState = ptChannel->tHsCtrl.ulNetxFlags & NCF_COMMUNICATING;
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
static int32_t APIENTRY HIFxChannelUnregisterNotification(CIFXHANDLE hChannel,
                                                          uint32_t   ulNotification)
{
  int32_t lRet = CIFX_NO_ERROR;

  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  switch (ulNotification)
  {
    case CIFX_NOTIFY_RX_MBX_FULL:
      /* Check if already registered */
      if( NULL == ptChannel->tToHostMbx.tCom.tCtl.pfnCallback)
      {
        /* Not registered */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        ptChannel->tToHostMbx.tCom.tCtl.pfnCallback = NULL;
      }
    break;

    case CIFX_NOTIFY_TX_MBX_EMPTY:
      /* Check if already registered */
      if( NULL == ptChannel->tFromHostMbx.tCom.tCtl.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        ptChannel->tFromHostMbx.tCom.tCtl.pfnCallback = NULL;
      }
    break;

    case CIFX_NOTIFY_PD0_IN:
    case CIFX_NOTIFY_PD0_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[0];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD0_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[0];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have one area */
      if( 0 == ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL == ptBlock->tIoCtl.pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        ptBlock->tIoCtl.pfnCallback = NULL;
        ptBlock->tIoCtl.pvUser      = NULL;
        lRet = CIFX_NO_ERROR;
      }
    }
    break;

    case CIFX_NOTIFY_PD1_IN:
    case CIFX_NOTIFY_PD1_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[1];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD1_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[1];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have two areas */
      if( 2 > ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL == ptBlock->tIoCtl.pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        ptBlock->tIoCtl.pfnCallback = NULL;
        ptBlock->tIoCtl.pvUser      = NULL;
        lRet = CIFX_NO_ERROR;
      }
    }
    break;

    case CIFX_NOTIFY_PD2_IN:
    case CIFX_NOTIFY_PD2_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[2];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD2_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[2];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have three areas */
      if( 3 > ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL == ptBlock->tIoCtl.pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        ptBlock->tIoCtl.pfnCallback = NULL;
        ptBlock->tIoCtl.pvUser      = NULL;
        lRet = CIFX_NO_ERROR;
      }
    }
    break;

    case CIFX_NOTIFY_PD3_IN:
    case CIFX_NOTIFY_PD3_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[3];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD3_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[3];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have four areas */
      if( 4 > ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL == ptBlock->tIoCtl.pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        ptBlock->tIoCtl.pfnCallback = NULL;
        ptBlock->tIoCtl.pvUser      = NULL;
        lRet = CIFX_NO_ERROR;
      }
    }
    break;

    case CIFX_NOTIFY_SYNC:
      if( NULL == ptChannel->atSynch[0].pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        /* Add the callback */
        ptChannel->atSynch[0].pfnCallback = NULL;
        ptChannel->atSynch[0].pvUser      = NULL;
      }
    break;

    case CIFX_NOTIFY_SYNC1:
        if (NULL == ptChannel->atSynch[1].pfnCallback)
        {
            /* Not registered before */
            lRet = CIFX_CALLBACK_NOT_REGISTERED;
        }
        else
        {
            /* Add the callback */
            ptChannel->atSynch[1].pfnCallback = NULL;
            ptChannel->atSynch[1].pvUser = NULL;
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
static int32_t APIENTRY HIFxChannelSyncState(CIFXHANDLE  hChannel,
                                             uint32_t    ulCmd,
                                             uint32_t    ulTimeout,
                                             uint32_t*   pulErrorCount)
{
  // TODO: function need to be adapt to netX9x2
  int32_t           lRet      = CIFX_NO_ERROR;
  PCHANNELINSTANCE  ptChannel = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pulErrorCount);

  UNREFERENCED_PARAMETER(ulTimeout);
  UNREFERENCED_PARAMETER(pulErrorCount);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;
  } else
  {
    switch (ulCmd)
    {
      case CIFX_SYNC_SIGNAL_CMD:
        /* fall through */
      case CIFX_SYNC_ACKNOWLEDGE_CMD:
        /* fall through */
      case CIFX_SYNC_WAIT_CMD:
        /* fall through */
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
static CIFX_API_FUNCTION_LIST_T s_tCifxHifApiFuns =
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
  HIFxSysdevicePutPacket,
  HIFxSysdeviceGetPacket,
  HIFxSysdeviceInfo,
  xSysdeviceFindFirstFile,
  xSysdeviceFindNextFile,
  HIFxSysdeviceDownload,
  HIFxSysdeviceUpload,
  xSysdeviceReset,
  xSysdeviceResetEx,
  xSysdeviceBootstart,
  HIFxSysdeviceExtendedMemory,
  xChannelOpen,
  xChannelClose,
  xChannelFindFirstFile,
  xChannelFindNextFile,
  HIFxChannelDownload,
  HIFxChannelUpload,
  xChannelGetMBXState,
  HIFxChannelPutPacket,
  HIFxChannelGetPacket,
  HIFxChannelGetSendPacket,
  HIFxChannelConfigLock,
  xChannelReset,
  HIFxChannelInfo,
  xChannelWatchdog,
  HIFxChannelHostState,
  xChannelBusState,
  HIFxChannelDMAState,
  HIFxChannelIOInfo,
  HIFxChannelIOWaitEvent,
  HIFxChannelIORead,
  HIFxChannelIOWrite,
  HIFxChannelIOReadSendData,
  HIFxChannelControlBlock,
  HIFxChannelCommonStatusBlock,
  HIFxChannelExtendedStatusBlock,
  HIFxChannelUserBlock,
  HIFxChannelPLCMemoryPtr,
  HIFxChannelPLCIsReadReady,
  HIFxChannelPLCIsWriteReady,
  HIFxChannelPLCActivateWrite,
  HIFxChannelPLCActivateRead,
  HIFxChannelRegisterNotification,
  HIFxChannelUnregisterNotification,
  HIFxChannelSyncState,
};

PCIFX_API_FUNCTION_LIST_T cifXTkitGetHifApiFunctionList(void)
{
  return &s_tCifxHifApiFuns;
}

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/
