/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXCommon.c 15335 2025-11-26 09:42:58Z AMinor $:

  Description:
    Common cifX functions and variables shared used by DPM and HIF

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-07-14  created

**************************************************************************************/

/*****************************************************************************/
/*! \file cifXCommon.c
*   Common cifX functions                                                    */
/*****************************************************************************/

#include "cifXToolkit.h"
#include "cifXEndianess.h"
#include "cifXErrors.h"
#include "cifXHWFunctions.h"

#include "Hil_Results.h"
#include "Hil_SystemCmd.h"

uint32_t                g_ulTraceLevel  = CIFX_TRACE_LEVEL_ERROR; /*!< Tracelevel used by the cifX Toolkit */
uint32_t                g_ulDeviceCount = 0;                      /*!< Number of devices handled by the cifX Toolkit */
PDEVICEINSTANCE*        g_pptDevices    = NULL;                   /*!< Array of device informations */
TKIT_DRIVER_INFORMATION g_tDriverInfo   = {0};                    /*!< Global driver information */
void*                   g_pvTkitLock    = NULL;                   /*!< Global cifX Toolkit lock */

/*****************************************************************************/
/*! Structure description of NETX_FW_IDENTIFY_CNF_DATA_T                     */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atFWIdentifyConv[] =
{
  /* Offset, Width,                       Elements */
  { 0x00, eCIFX_ENDIANESS_WIDTH_16BIT, 4}, /* tFwVersion.Maj/Min/Build/Rev   */
  { 0x48, eCIFX_ENDIANESS_WIDTH_16BIT, 1}, /* tFwDate.usYear                 */
};

#ifdef CIFX_TOOLKIT_PARAMETER_CHECK
/*****************************************************************************/
/*! Checks if the given sysdevice handle is valid
*   \param hChannel      Sysdevice handle
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t CheckSysdeviceHandle(CIFXHANDLE hChannel)
{
  int32_t  lRet  = CIFX_INVALID_HANDLE;

  if ( NULL != hChannel)
  {
    PCHANNELINSTANCE ptSysDevice = (PCHANNELINSTANCE)hChannel;
    if ( 1 == ptSysDevice->fIsSysDevice)
    {
      if( 0 == ptSysDevice->ulOpenCount)
      {
        /* We are probably in the initialization phase without an opened device handle */
        lRet = CIFX_NO_ERROR;

      }else if ( (0    == g_ulDeviceCount) ||
                 (NULL == g_pptDevices)    )
      {
        /* We can't search for the handle in the device list, because the list is not available. */
        /* Maybe we are in the initialization phase and the list is not created yet. */
        lRet = CIFX_NO_ERROR;

      }else
      {
        /* Try to find the ptSysDevice pointer in one of the device instances */
        uint32_t ulDev = 0;

        for(ulDev = 0; ulDev < g_ulDeviceCount; ++ulDev)
        {
          if (ptSysDevice == &g_pptDevices[ulDev]->tSystemDevice)
          {
            lRet = CIFX_NO_ERROR;
            break;
          }
        }
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Checks if the given channel handle is valid
*   \param hChannel      Channel handle
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t CheckChannelHandle(CIFXHANDLE hChannel)
{
  int32_t  lRet  = CIFX_INVALID_HANDLE;

  if ( NULL != hChannel)
  {
    PCHANNELINSTANCE ptDevice = (PCHANNELINSTANCE)hChannel;
    if ( 0 == ptDevice->fIsSysDevice)
    {
      /* Check if we can find our device in the device table */
      if ( 0 == g_ulDeviceCount)
      {
        /* We are in the initialization phase without an device entry in the g_pptDevices table */
        lRet = CIFX_NO_ERROR;

      }else if ( (0    == g_ulDeviceCount) ||
                 (NULL == g_pptDevices)    )
      {
        /* We can't search for the handle in the device list, because the list is not available. */
        /* Maybe we are in the initialization phase and the list is not created yet. */
        lRet = CIFX_NO_ERROR;

      }else
      {
        /* Try to find the ptSysDevice pointer in one of the device instances */
        uint32_t ulDev = 0;

        /* Check if we can find our channel inside a device */
        for(ulDev = 0; ulDev < g_ulDeviceCount; ++ulDev)
        {
          uint32_t ulChannel = 0;
          for( ulChannel = 0; ulChannel < g_pptDevices[ulDev]->ulCommChannelCount; ++ulChannel)
          {
            if ( ptDevice == g_pptDevices[ulDev]->pptCommChannels[ulChannel])
            {
              lRet = CIFX_NO_ERROR;
              break;
            }
          }
        }
      }
    }
  }

  return lRet;
}
#endif

#ifdef CIFX_TOOLKIT_TIME
/*****************************************************************************/
/*! Initialize RTC
*   \param ptDevInstance Instance to start up                                */
/*****************************************************************************/
void cifXInitTime(PDEVICEINSTANCE ptDevInstance)
{
  uint32_t ulRTCInfo;

  if (HIL_HIF_LAYOUT_NA == ptDevInstance->bDPMLayout)
  {
    HIL_DPM_SYSTEM_CHANNEL_T* ptSystemChannel = (HIL_DPM_SYSTEM_CHANNEL_T*)(ptDevInstance->tSystemDevice.pbDPMChannelStart);
    ulRTCInfo = (HIL_SYSTEM_HW_RTC_MSK & LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSystemChannel->tSystemState.ulHWFeatures)));
  } else
  {
    HIL_HIF_SYSTEM_CHANNEL_T* ptSystemChannel = (HIL_HIF_SYSTEM_CHANNEL_T*)(ptDevInstance->tSystemDevice.pbDPMChannelStart);
    ulRTCInfo = (HIL_SYSTEM_HW_RTC_MSK & LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSystemChannel->tSystemState.ulHWFeatures)));
  }

  /* Check if RTC is available and not already set */
  if( 0 != (HIL_SYSTEM_HW_RTC_TYPE_MSK & ulRTCInfo))
  {
    /* Check if it is already set */
    if( 0 == (HIL_SYSTEM_HW_RTC_STATE & ulRTCInfo))
    {
      int32_t lRet = 0;

      /* Create a time request*/
      HIL_TIME64_CMD_REQ_T  tSendPkt;
      HIL_TIME64_CMD_CNF_T  tRecvPkt;

      OS_Memset(&tSendPkt, 0, sizeof(tSendPkt));
      OS_Memset(&tRecvPkt, 0, sizeof(tRecvPkt));

      /* Set the time on the device */
      tSendPkt.tHead.ulDest = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
      tSendPkt.tHead.ulSrc  = HOST_TO_LE32(ptDevInstance->ulPhysicalAddress);
      tSendPkt.tHead.ulCmd  = HOST_TO_LE32(HIL_TIME64_COMMAND_REQ);
      tSendPkt.tHead.ulLen  = HOST_TO_LE32(sizeof(tSendPkt.tData));

      tSendPkt.tData.ulTimeCmd = HIL_TIME64_CMD_SETTIME;
      tSendPkt.tData.ullData   = (uint32_t)OS_Time(NULL);

      /* Transfer packet */
      lRet = DEV_TransferPacket(
          &ptDevInstance->tSystemDevice,
          (CIFX_PACKET*)&tSendPkt,
          (CIFX_PACKET*)&tRecvPkt,
          sizeof(tRecvPkt),
          CIFX_TO_SEND_PACKET,
          NULL,
          NULL);

      if( (CIFX_NO_ERROR  != lRet) ||
          (SUCCESS_HIL_OK != LE32_TO_HOST(tRecvPkt.tHead.ulSta)) )
      {
        if(g_ulTraceLevel & CIFX_TRACE_LEVEL_WARNING)
        {
          USER_Trace(ptDevInstance,
                     CIFX_TRACE_LEVEL_WARNING,
                     "Error setting device time! (lRet=0x%08X, ulState=0x%08X)",
                     lRet,
                     LE32_TO_HOST(tRecvPkt.tHead.ulSta));
        }
      }else
      {
        if(g_ulTraceLevel & CIFX_TRACE_LEVEL_INFO)
        {
          USER_Trace(ptDevInstance,
                     CIFX_TRACE_LEVEL_INFO,
                     "Setting RTC done: 0x%08X (%u)",
                     tSendPkt.tData.ullData,
                     tSendPkt.tData.ullData);
        }
      }
    }
  }
}
#endif

/*****************************************************************************/
/*! Read firmware identification
*   \param ptDevInstance      Device Instance
*   \param ulChannel          Channel number
*   \param pfnRecvPktCallback Callback for unexpected packets
*   \param pvUser             Callback user parameter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t cifXReadFirmwareIdent(PDEVICEINSTANCE       ptDevInstance,
                              uint32_t              ulChannel,
                              PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                              void*                 pvUser)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannelInst = ptDevInstance->pptCommChannels[ulChannel];

  HIL_FIRMWARE_IDENTIFY_REQ_T tSendPkt;
  HIL_FIRMWARE_IDENTIFY_CNF_T tRecvPkt;

  OS_Memset(&tSendPkt, 0, sizeof(tSendPkt));
  OS_Memset(&tRecvPkt, 0, sizeof(tRecvPkt));

  /* Read firmware information */
  tSendPkt.tHead.ulDest       = HOST_TO_LE32(HIL_PACKET_DEST_DEFAULT_CHANNEL);
  tSendPkt.tHead.ulSrc        = HOST_TO_LE32(ptDevInstance->ulPhysicalAddress);
  tSendPkt.tHead.ulCmd        = HOST_TO_LE32(HIL_FIRMWARE_IDENTIFY_REQ);
  tSendPkt.tHead.ulLen        = HOST_TO_LE32(sizeof(tSendPkt.tData));
  tSendPkt.tData.ulChannelId  = HOST_TO_LE32(ulChannel);

  /* Transfer packet */
  lRet = DEV_TransferPacket(
      &ptDevInstance->tSystemDevice,
      (CIFX_PACKET*)&tSendPkt,
      (CIFX_PACKET*)&tRecvPkt,
      sizeof(tRecvPkt),
      CIFX_TO_SEND_PACKET,
      pfnRecvPktCallback,
      pvUser);

  if( (CIFX_NO_ERROR  != lRet) ||
      (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(tRecvPkt.tHead.ulSta))) )
  {
    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_WARNING)
    {
      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_WARNING,
                "Error querying firmware information! (lRet=0x%08X)",
                lRet);
    }
  } else
  {
    OS_Memcpy( &ptChannelInst->tFirmwareIdent,
               &tRecvPkt.tData.tFirmwareIdentification,
               sizeof(ptChannelInst->tFirmwareIdent));

    (void)cifXConvertEndianess(0,
                               &ptChannelInst->tFirmwareIdent,
                               sizeof(ptChannelInst->tFirmwareIdent),
                               s_atFWIdentifyConv,
                               HIL_CNT_ELEMENT(s_atFWIdentifyConv));
  }

  return lRet;
}
