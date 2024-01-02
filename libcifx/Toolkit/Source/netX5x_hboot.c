/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: $:

  Description:
    cifX Toolkit implementation of the netX50/51 boot functions

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2021-08-31  IsNetX51or52ROM() sets now eChipType in device instance after checking
    2018-09-24  moved hboot functions to separate module


**************************************************************************************/

#include "cifXToolkit.h"
#include "NetX_ROMLoader.h"
#include "netx50_romloader_dpm.h"
#include "netx51_romloader_dpm.h"
#include "cifXErrors.h"
#include "cifXEndianess.h"

/*****************************************************************************/
/*! Structure describing a single HBOOT DPM mailbox                          */
/*****************************************************************************/
typedef struct HBOOT_MBX_DATA_Ttag
{
  uint8_t            bHskMask;   /*!< Handshake bit to toggle for this mailbox */
  uint32_t           ulSize;     /*!< Total size of mailbox                    */
  void*              pvData;     /*!< Data area of mailbox                     */
  volatile uint32_t* pulDataLen; /*!< Used size of mailbox                     */
} HBOOT_MBX_DATA_T;

/*****************************************************************************/
/*! Structure describing HBOOT DPM                                           */
/*****************************************************************************/
typedef struct HBOOT_DATA_Ttag
{
  HBOOT_HSREGISTER_T* ptHsk;          /*!< Handshake cell                  */
  HBOOT_MBX_DATA_T    tToNetXMailbox; /*!< Mailbox information Host-->netX */
  HBOOT_MBX_DATA_T    tToHostMailbox; /*!< Mailbox information netX-->Host */

} HBOOT_DATA_T;

/*****************************************************************************/
/*! Detect a running netX51/52 ROMLoader via DPM
*   \param ptDevInstance Instance to reset
*   \return !=0 if netX51/52 has been detected                               */
/*****************************************************************************/
int IsNetX51or52ROM(PDEVICEINSTANCE ptDevInstance)
{
  int                       iRet     = 0;
  NETX51_DPM_CONFIG_AREA_T* ptDpmCfg = (NETX51_DPM_CONFIG_AREA_T*)ptDevInstance->pbDPM;

  if( (HWIF_READ8(ptDevInstance, ptDevInstance->pbDPM[NETX51_DETECT_OFFSET1]) == NETX51_DETECT_VALUE1) &&
      (HWIF_READ8(ptDevInstance, ptDevInstance->pbDPM[NETX51_DETECT_OFFSET2]) == NETX51_DETECT_VALUE2) &&
      (HWIF_READ32(ptDevInstance, ptDpmCfg->aulReserved1[0]) == 0) &&
      (HWIF_READ32(ptDevInstance, ptDpmCfg->aulReserved1[1]) == 0) )
  {
    /* We found a valid entry */
    /* Check for netX51 or netX52 */
    uint32_t ulDpmNetxVersion = (MSK_NX56_dpm_netx_version_valid | MSK_NX56_dpm_netx_version_chiptype) &
                                HWIF_READ32(ptDevInstance, ptDpmCfg->ulDpmNetxVersion);

    if ( (MSK_NX56_dpm_netx_version_valid | ( 2 << SRT_NX56_dpm_netx_version_chiptype)) == ulDpmNetxVersion )
    {
      /* This is a netX52 */
      ptDevInstance->eChipType = eCHIP_TYPE_NETX52;
    } else
    {
      /* This is a netX51 */
      ptDevInstance->eChipType = eCHIP_TYPE_NETX51;
    }

    iRet = 1;
  }

  return iRet;
}

/*****************************************************************************/
/*! Wait for bitstate in netX50/51 ROMloader (hboot) DPM
*   \param ptDevInstance Instance to download the bootloader to (needs a reset
*                        before downloading)
*   \param ptHbootData   Romloader boot data structure
*   \param ulBitMask     Bitmask to check
*   \param bState        Required state (HIL_FLAGS_EQUAL/NOT_EQUAL are supported
*   \param ulTimeout     Timeout in ms to wait for packet
*   \return !=0 on success                                                   */
/*****************************************************************************/
static int hboot_waitforbitstate(PDEVICEINSTANCE ptDevInstance,
                                 HBOOT_DATA_T*   ptHbootData,
                                 uint32_t        ulBitMask,
                                 uint8_t         bState,
                                 uint32_t        ulTimeout)
{
  int                   iRet         = 0;
  int32_t               lStartTime   = 0;
  HBOOT_HSREGISTER_T*   ptHsk        = ptHbootData->ptHsk;
  uint8_t               bActualState = 0;
  uint8_t               bHostFlags   = 0;

  UNREFERENCED_PARAMETER(ptDevInstance);

  bHostFlags = HWIF_READ8(ptDevInstance, ptHsk->t8Bit.bHostFlags);
  if((bHostFlags ^ HWIF_READ8(ptDevInstance, ptHsk->t8Bit.bNetXFlags)) & ulBitMask)
    bActualState = HIL_FLAGS_NOT_EQUAL;
  else
    bActualState = HIL_FLAGS_EQUAL;

  /* The desired state is already there, so just return true */
  if(bActualState == bState)
    return 1;

  /* If no timeout is given, don't try to wait for the Bit change */
  if(0 == ulTimeout)
    return 0;

  lStartTime = (int32_t)OS_GetMilliSecCounter();

  /* Poll for desired bit state */
  while(bActualState != bState)
  {
    uint32_t   ulDiffTime  = 0L;

    bHostFlags = HWIF_READ8(ptDevInstance, ptHsk->t8Bit.bHostFlags);
    if((bHostFlags ^ HWIF_READ8(ptDevInstance, ptHsk->t8Bit.bNetXFlags)) & ulBitMask)
      bActualState = HIL_FLAGS_NOT_EQUAL;
    else
      bActualState = HIL_FLAGS_EQUAL;

    /* Check for timeout */
    ulDiffTime = OS_GetMilliSecCounter() - lStartTime;
    if ( ulDiffTime > ulTimeout)
    {
      break;
    }

    OS_Sleep(0);
  }

  if(bActualState == bState)
    iRet = 1;

  return iRet;
}

/*****************************************************************************/
/*! Send a packet to the netX50/51 romloader (hboot)
*   \param ptDevInstance Instance to download the bootloader to (needs a reset
*                        before downloading)
*   \param ptHbootData   Romloader boot data structure
*   \param pbData        Send data buffer
*   \param ulDataLen     Length of send data
*   \param ulTimeout     Timeout in ms to wait for packet
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t hboot_send_packet(PDEVICEINSTANCE ptDevInstance,
                                 HBOOT_DATA_T*   ptHbootData,
                                 uint8_t*        pbData,
                                 uint32_t        ulDataLen,
                                 uint32_t        ulTimeout)
{
  int32_t               lRet          = CIFX_NO_ERROR;
  HBOOT_HSREGISTER_T*   ptHsk         = ptHbootData->ptHsk;
  uint8_t               bToNetXMask   = ptHbootData->tToNetXMailbox.bHskMask;
  uint32_t              ulMailboxSize = ptHbootData->tToNetXMailbox.ulSize;
  void*                 pvMailbox     = ptHbootData->tToNetXMailbox.pvData;
  volatile uint32_t*    pulMbxDataLen = ptHbootData->tToNetXMailbox.pulDataLen;

  if(ulDataLen > ulMailboxSize)
    return CIFX_INVALID_BUFFERSIZE;

  if(!hboot_waitforbitstate(ptDevInstance,
                            ptHbootData,
                            bToNetXMask,
                            HIL_FLAGS_EQUAL,
                            ulTimeout))
  {
    /* The mailbox is busy */
    lRet = CIFX_DEV_PUT_TIMEOUT;

  } else
  {
    uint8_t bHostFlags = 0;

    /* The mailbox is free */
    HWIF_WRITEN(ptDevInstance,
                pvMailbox,
                pbData,
                ulDataLen);

    HWIF_WRITE32(ptDevInstance, pulMbxDataLen[0], HOST_TO_LE32(ulDataLen));

    bHostFlags = HWIF_READ8(ptDevInstance, ptHsk->t8Bit.bHostFlags);
    HWIF_WRITE8(ptDevInstance, ptHsk->t8Bit.bHostFlags, (bHostFlags ^ bToNetXMask));

    lRet = CIFX_NO_ERROR;
  }

  return lRet;
}

/*****************************************************************************/
/*! Get a packet from the netX50/51 romloader (hboot)
*   \param ptDevInstance Instance to download the bootloader to (needs a reset
*                        before downloading)
*   \param ptHBootData   Romloader boot data structure
*   \param pbResult      Buffer for romloader error
*   \param ulTimeout     Timeout in ms to wait for packet
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t hboot_get_packet(PDEVICEINSTANCE ptDevInstance,
                                HBOOT_DATA_T*   ptHBootData,
                                uint8_t*        pbResult,
                                uint32_t        ulTimeout)
{
  int32_t lRet  = CIFX_DEV_GET_NO_PACKET;

  if(hboot_waitforbitstate(ptDevInstance,
                           ptHBootData,
                           NETX50_DPM_TOHOSTMBX_MSK,
                           HIL_FLAGS_NOT_EQUAL,
                           ulTimeout))
  {
    HBOOT_HSREGISTER_T* ptHskReg      = ptHBootData->ptHsk;
    volatile uint8_t*   pbMailbox     = (volatile uint8_t*)ptHBootData->tToHostMailbox.pvData;
    volatile uint32_t*  pulMbxDataLen = ptHBootData->tToHostMailbox.pulDataLen;
    uint8_t             bToHostMask   = ptHBootData->tToHostMailbox.bHskMask;
    uint8_t             bHostFlags    = 0;

    lRet = CIFX_NO_ERROR;

    if( LE32_TO_HOST(HWIF_READ32(ptDevInstance, pulMbxDataLen[0])) != 1)
    {
      lRet = CIFX_DRV_INIT_STATE_ERROR;

    } else
    {
      *pbResult = HWIF_READ8(ptDevInstance, pbMailbox[0]);
    }

    bHostFlags = HWIF_READ8(ptDevInstance, ptHskReg->t8Bit.bHostFlags);
    if((bHostFlags ^ HWIF_READ8(ptDevInstance, ptHskReg->t8Bit.bNetXFlags)) & bToHostMask)
    {
      HWIF_WRITE8(ptDevInstance, ptHskReg->t8Bit.bHostFlags, (bHostFlags ^ bToHostMask));
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Exchanges a packet with the netX50/51 romloader (hboot)
*   \param ptDevInstance Instance to download the bootloader to (needs a reset
*                        before downloading)
*   \param ptHBootData   Romloader boot data structure
*   \param pbSendData    Send data buffer
*   \param ulSendDataLen Send data length
*   \param pbResult      Buffer for romloader error
*   \param ulTimeout     Timeout in ms to wait for packet
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t hboot_transfer_packet(PDEVICEINSTANCE ptDevInstance,
                                     HBOOT_DATA_T*   ptHBootData,
                                     uint8_t*        pbSendData,
                                     uint32_t        ulSendDataLen,
                                     uint8_t*        pbResult,
                                     uint32_t        ulTimeout)
{
  int32_t lRet  = CIFX_NO_ERROR;

  if(CIFX_NO_ERROR == (lRet = hboot_send_packet(ptDevInstance, ptHBootData, pbSendData, ulSendDataLen, ulTimeout)))
  {
    lRet = hboot_get_packet(ptDevInstance, ptHBootData, pbResult, ulTimeout);
  }

  return lRet;
}

/*****************************************************************************/
/*! Downloads and starts the bootloader on netX50/51 (hboot)
*   \param ptDevInstance Instance to download the bootloader to (needs a reset
*                        before downloading)
*   \param pbFileData    Pointer to bootloader file data
*   \param ulFileDataLen Length of bootloader file
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t cifXStartBootloader_hboot(PDEVICEINSTANCE ptDevInstance,
                                  uint8_t*        pbFileData,
                                  uint32_t        ulFileDataLen)
{
  int32_t       lRet       = CIFX_NO_ERROR;
  uint32_t      ulCopyLen  = 0;
  HBOOT_DATA_T  tHBoot;

  OS_Memset(&tHBoot, 0, sizeof(tHBoot));

  /* Check for chip type and initialize boot data structure */
  if((eCHIP_TYPE_NETX51 == ptDevInstance->eChipType) || (eCHIP_TYPE_NETX52 == ptDevInstance->eChipType))
  {
    PNETX51_ROMLOADER_DPM ptDpm = (PNETX51_ROMLOADER_DPM)ptDevInstance->pbDPM;

    tHBoot.ptHsk = (HBOOT_HSREGISTER_T*)&ptDpm->tHandshake.ulHandshakeFlag;

    tHBoot.tToHostMailbox.bHskMask   = NETX51_DPM_TOHOSTMBX_MSK;
    tHBoot.tToHostMailbox.pulDataLen = &ptDpm->tHBootConfig.ulNetXToHostDataSize;
    tHBoot.tToHostMailbox.pvData     = (void*)ptDpm->abNetxToHostData;
    tHBoot.tToHostMailbox.ulSize     = sizeof(ptDpm->abNetxToHostData);

    tHBoot.tToNetXMailbox.bHskMask   = NETX51_DPM_TONETXMBX_MSK;
    tHBoot.tToNetXMailbox.pulDataLen = &ptDpm->tHBootConfig.ulHostToNetxDataSize;
    tHBoot.tToNetXMailbox.pvData     = (void*)ptDpm->abHostToNetxData;
    tHBoot.tToNetXMailbox.ulSize     = sizeof(ptDpm->abHostToNetxData);

    if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
    {
      USER_Trace(ptDevInstance,
                 TRACE_LEVEL_DEBUG,
                 "Found netX51 ROMloader");
    }
  } else
  {
    PNETX50_ROMLOADER_DPM ptDpm = (PNETX50_ROMLOADER_DPM)ptDevInstance->pbDPM;

    tHBoot.ptHsk = &ptDpm->atHandshakeRegs[NETX50_DPM_HANDSHAKE_OFFSET];

    tHBoot.tToHostMailbox.bHskMask   = NETX50_DPM_TOHOSTMBX_MSK;
    tHBoot.tToHostMailbox.pulDataLen = &ptDpm->ulNetxToHostDataSize;
    tHBoot.tToHostMailbox.pvData     = (void*)ptDpm->abNetxToHostData;
    tHBoot.tToHostMailbox.ulSize     = sizeof(ptDpm->abNetxToHostData);

    tHBoot.tToNetXMailbox.bHskMask   = NETX50_DPM_TONETXMBX_MSK;
    tHBoot.tToNetXMailbox.pulDataLen = &ptDpm->ulHostToNetxDataSize;
    tHBoot.tToNetXMailbox.pvData     = (void*)ptDpm->abHostToNetxData;
    tHBoot.tToNetXMailbox.ulSize     = sizeof(ptDpm->abHostToNetxData);

    /* Read romloader version */
    if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
    {
      uint32_t ulLayout = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptDpm->aulDpmHsRegs[NETX50_DPM_BLLAYOUT_OFFSET]));
      ulLayout = (ulLayout & MSK_NETX50_DPM_BLLAYOUT) >> SRT_NETX50_DPM_BLLAYOUT;

      USER_Trace(ptDevInstance,
                 TRACE_LEVEL_DEBUG,
                 "Found netX50 ROMloader, DPM layout type 0x%08X",
                 ulLayout);
    }
  }

  ulCopyLen = tHBoot.tToNetXMailbox.ulSize;

  if(ulFileDataLen < sizeof(NETX_BOOTBLOCK_T))
  {
    USER_Trace(ptDevInstance,
               TRACE_LEVEL_ERROR,
               "Invalid Bootloader file. File must be larger than 64 Bytes. (Detected Size:%u)",
                ulFileDataLen);
    lRet = CIFX_FILE_TYPE_INVALID;
  }

  if(CIFX_NO_ERROR == lRet)
  {
    uint8_t bResult             = 0;
    int     fLastPacketReceived = 0;

    /* Send Bootblock to device */
    if(CIFX_NO_ERROR != (lRet = hboot_transfer_packet(ptDevInstance,
                                                      &tHBoot,
                                                      pbFileData,
                                                      sizeof(NETX_BOOTBLOCK_T),
                                                      &bResult,
                                                      CIFX_TO_SEND_PACKET)))
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_ERROR,
                "Error transfering bootheader to netX50 Bootloader (lRet = 0x%08X)",
                 lRet);

    } else if(0 != bResult)
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_ERROR,
                "netX50 ROMloader rejected bootblock (bResult = %u)",
                 bResult);
      lRet = CIFX_DRV_DOWNLOAD_FAILED;

    } else
    {
      /* Everything ok. start with rest of file */
      pbFileData    += (uint32_t)sizeof(NETX_BOOTBLOCK_T);
      ulFileDataLen -= (uint32_t)sizeof(NETX_BOOTBLOCK_T);
    }

    /* Download whole file and abort if something went wrong during download */
    while( (ulFileDataLen > 0) &&
           (lRet == CIFX_NO_ERROR) )
    {
      /* Last fragment may be shorter */
      if(ulFileDataLen < ulCopyLen)
        ulCopyLen = ulFileDataLen;

      /* Place message in mailbox and wait until message has been processed */
      lRet = hboot_send_packet(ptDevInstance, &tHBoot, pbFileData, ulCopyLen, CIFX_TO_SEND_PACKET);

      if(CIFX_NO_ERROR != lRet)
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_ERROR,
                  "Error transferring data packet from/to netX50 Bootloader (lRet = 0x%08X)",
                   lRet);

      } else if(CIFX_NO_ERROR == (hboot_get_packet(ptDevInstance, &tHBoot, &bResult, 0)))
      {
        /* Download is finished or has been aborted. Check will be done below */
        fLastPacketReceived = 1;
        break;
      }

      pbFileData    += ulCopyLen;
      ulFileDataLen -= ulCopyLen;
    }

    if(CIFX_NO_ERROR == lRet)
    {
      if(!fLastPacketReceived &&
         (CIFX_NO_ERROR != (lRet = hboot_get_packet(ptDevInstance, &tHBoot, &bResult, CIFX_TO_SEND_PACKET))) )
      {
        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Error getting final packet from netX50 ROM Loader. lRet=0x%08X",
                    lRet);
        }

      } else if( 0 != bResult)
      {
        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "netX50 ROM Loader download error. bResult=%u",
                    bResult);
        }
        lRet = CIFX_DRV_DOWNLOAD_FAILED;

      }
    }
  }

  return lRet;
}
