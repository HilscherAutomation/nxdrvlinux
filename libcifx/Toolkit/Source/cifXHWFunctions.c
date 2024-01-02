/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXHWFunctions.c 14802 2023-05-10 09:39:47Z RMayer $:

  Description:
    cifX API Hardware handling functions implementation

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2023-04-18  Added new option parameter for HWIF_READN / WRITEN function, to be able to
                recognize single HWIF_READ16/WRITE32 and HWIF_READ32/WRITE32 accesses
    2023-02-07  Added wait flag in DEV_Reset_Execute()
                Used DEV_Reset_Execute() to signal reset functions on APP_CPU/iDPM handling
    2022-01-04  Using new reset definition mask HIL_SYS_CONTROL_RESET_PARAM_FLAG_MASK in reset function
    2021-10-15  Added ulHostCOSFlagsSaved variable handling
    2020-08-18  After reset, fResetActive needs to be cleared before Handshake Cells
                are re-evaluated
    2019-11-26  Use CIFX_DMA_STATE_* defines in DEV_DMAState()
    2019-11-13  Locking in DEV_ReadHandshakeFlags() includes Handshake Cell accesses
    2019-10-30  Increase timeout during updatestart to firmware (e.g. initial startup
                may take longer than subsequent starts)
    2019-10-16  Reworked reset function handling and parameter passing
    2019-10-08  - Fix reset handling for use case IDPM & APP CPU
                - Split Dev_DoResetEx(), offer separate updatestart function
    2019-03-21  Add timeout during resets for netX4000/4100 based PCI(e) devices to
                prevent DPM accesses during reset.
    2018-11-06  Add reset handling for IDPM and APP CPUs.
    2018-10-10  - Updated header and definitions to new Hilscher defines
                - Derived from cifX Toolkit V1.6.0.0

**************************************************************************************/

/*****************************************************************************/
/*! \file cifXHWFunctions.c
*    cifX API Hardware handling functions implementation                     */
/*****************************************************************************/

#include "cifXHWFunctions.h"
#include "cifXErrors.h"
#include "cifXEndianess.h"

#include "Hil_Packet.h"
#include "Hil_SystemCmd.h"

/*****************************************************************************/
/*!  \addtogroup CIFX_TK_HARDWARE Hardware Access
*    \{                                                                      */
/*****************************************************************************/

/*****************************************************************************/
/*! Sends a Packet to the device/channel
*   \param ptChannel    Channel instance to send a packet
*   \param ptSendPkt    Packet to send
*   \param ulTimeout    Maximum time in ms to wait for an empty mailbox
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_PutPacket(PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout)
{
  int32_t lRet = CIFX_DEV_MAILBOX_FULL;

  if(!DEV_IsReady(ptChannel))
    return CIFX_DEV_NOT_READY;

  /* Check if packet fits into the mailbox */
  if( (LE32_TO_HOST(ptSendPkt->tHeader.ulLen) + HIL_PACKET_HEADER_SIZE) > ptChannel->tSendMbx.ulSendMailboxLength)
    return CIFX_DEV_MAILBOX_TOO_SHORT;

  if(DEV_WaitForBitState(ptChannel, ptChannel->tSendMbx.bSendCMDBitoffset, HIL_FLAGS_EQUAL, ulTimeout))
  {
    /* Copy packet to mailbox */
    ++ptChannel->tSendMbx.ulSendPacketCnt;
    HWIF_WRITEN(ptChannel->pvDeviceInstance,
                ptChannel->tSendMbx.ptSendMailboxStart->abSendMailbox,
                ptSendPkt,
                LE32_TO_HOST(ptSendPkt->tHeader.ulLen) + HIL_PACKET_HEADER_SIZE);

    /* Lock flag access */
    OS_EnterLock(ptChannel->pvLock);

    /* Signal new packet */
    DEV_ToggleBit(ptChannel, ptChannel->tSendMbx.ulSendCMDBitmask);

    /* Unlock flag access */
    OS_LeaveLock(ptChannel->pvLock);

    lRet = CIFX_NO_ERROR;
  }

  return lRet;
}

/*****************************************************************************/
/*! Retrieves a Packet from the device/channel
*   \param ptChannel        Channel instance to receive a packet from
*   \param ptRecvPkt        Pointer to place received Packet in
*   \param ulRecvBufferSize Length of the receive buffer
*   \param ulTimeout        Maximum time in ms to wait for an empty mailbox
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_GetPacket( PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptRecvPkt, uint32_t ulRecvBufferSize, uint32_t ulTimeout)
{
  int32_t       lRet        = CIFX_NO_ERROR;
  uint32_t      ulCopySize  = 0;
  CIFX_PACKET*  ptPacket    = NULL;

  if(!DEV_IsReady(ptChannel))
    return CIFX_DEV_NOT_READY;

  if(!DEV_WaitForBitState(ptChannel, ptChannel->tRecvMbx.bRecvACKBitoffset, HIL_FLAGS_NOT_EQUAL, ulTimeout))
    return CIFX_DEV_GET_NO_PACKET;

  ++ptChannel->tRecvMbx.ulRecvPacketCnt;

  ptPacket   = (CIFX_PACKET*)ptChannel->tRecvMbx.ptRecvMailboxStart->abRecvMailbox;
  ulCopySize = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptPacket->tHeader.ulLen)) + HIL_PACKET_HEADER_SIZE;
  if(ulCopySize > ulRecvBufferSize)
  {
    /* We have to free the mailbox, read as much as possible */
    ulCopySize = ulRecvBufferSize;
    lRet = CIFX_BUFFER_TOO_SHORT;
  }

  HWIF_READN(ptChannel->pvDeviceInstance, ptRecvPkt, ptPacket, ulCopySize);

  /* Lock flag access */
  OS_EnterLock(ptChannel->pvLock);

  /* Signal read packet done */
  DEV_ToggleBit(ptChannel, ptChannel->tRecvMbx.ulRecvACKBitmask);

  /* Unlock flag access */
  OS_LeaveLock(ptChannel->pvLock);

  return lRet;
}

/*****************************************************************************/
/*! Exchanges a packet with the device
*   ATTENTION: This function will poll for receive packet, and will discard
*              any packets that do not match the send packet. So don't use
*              it during active data transfers
*   \param pvChannel        Channel instance to exchange a packet
*   \param ptSendPkt        Send packet pointer
*   \param ptRecvPkt        Pointer to place received Packet in
*   \param ulRecvBufferSize Length of the receive buffer
*   \param ulTimeout        Maximum time in ms to wait for an empty mailbox
*   \param pvPktCallback    Packet callback for unhandled receive packets
*   \param pvUser           User data for callback function
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_TransferPacket( void*                  pvChannel,        CIFX_PACKET*  ptSendPkt, CIFX_PACKET* ptRecvPkt,
                            uint32_t               ulRecvBufferSize, uint32_t      ulTimeout,
                            PFN_RECV_PKT_CALLBACK  pvPktCallback,    void*         pvUser)
{
  int32_t          lCount     = 0;
  int32_t          lRet       = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel  = (PCHANNELINSTANCE)pvChannel;

  if( (lRet = DEV_PutPacket(ptChannel, ptSendPkt, ulTimeout)) == CIFX_NO_ERROR)
  {
    do
    {
      if( (lRet = DEV_GetPacket(ptChannel, ptRecvPkt, ulRecvBufferSize, ulTimeout)) == CIFX_NO_ERROR)
      {
        /* Check if we got the answer */
        if(  ((LE32_TO_HOST(ptRecvPkt->tHeader.ulCmd) & ~HIL_MSK_PACKET_ANSWER) == LE32_TO_HOST(ptSendPkt->tHeader.ulCmd))  &&
             (ptRecvPkt->tHeader.ulSrc   == ptSendPkt->tHeader.ulSrc)    &&
             (ptRecvPkt->tHeader.ulId    == ptSendPkt->tHeader.ulId)     &&
             (ptRecvPkt->tHeader.ulSrcId == ptSendPkt->tHeader.ulSrcId)  )
        {
          /* We got the answer message */
          /* lRet = ptRecvPkt->tHeader.ulState; */ /* Do not deliver back this information */
          break;
        } else
        {
          /* This is not our packet, check if the user wants it */
          if( NULL != pvPktCallback)
          {
            pvPktCallback(ptRecvPkt, pvUser);
          }
        }
        /* Reset error, in case we might drop out of the loop, with no proper answer,
           returning a "good" state */
        lRet = CIFX_DEV_GET_TIMEOUT;
        lCount++;
      } else
      {
        /* Error during packet receive */
        break;
      }
    } while ( lCount < 10);
  }

  return lRet;
}

/*****************************************************************************/
/*! Waits for a given handshake bit state on the channel (polling mode)
*   \param ptChannel    Channel instance to wait for bitstate
*   \param ulBitNumber  BitNumber to wait for (Bitnumber is used for
*                       indexing the event array in IRQ mode)
*   \param bState       State the handshake bit should be in after returning
*                       from this function
*   \param ulTimeout    Maximum time in ms to wait for the desired bit state
*   \return 0 on error/timeout, 1 on success                                 */
/*****************************************************************************/
static int DEV_WaitForBitState_Poll(PCHANNELINSTANCE ptChannel, uint32_t ulBitNumber, uint8_t bState, uint32_t ulTimeout)
{
  uint8_t   bActualState;
  int       iRet        = 0;
  uint32_t  ulBitMask   = 1 << ulBitNumber;
  int32_t   lStartTime  = 0;

  DEV_ReadHandshakeFlags(ptChannel, 0, 1);

  if( (HIL_FLAGS_CLEAR == bState) ||
      (HIL_FLAGS_SET == bState) )
  {
    bActualState = (ptChannel->usNetxFlags & ulBitMask)? HIL_FLAGS_SET : HIL_FLAGS_CLEAR;

  } else
  {
    if((ptChannel->usHostFlags ^ ptChannel->usNetxFlags) & ulBitMask)
      bActualState = HIL_FLAGS_NOT_EQUAL;
    else
      bActualState = HIL_FLAGS_EQUAL;
  }

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

    DEV_ReadHandshakeFlags(ptChannel, 0, 1);

    if( (HIL_FLAGS_CLEAR == bState) ||
        (HIL_FLAGS_SET == bState) )
    {
      bActualState = (ptChannel->usNetxFlags & ulBitMask)? HIL_FLAGS_SET : HIL_FLAGS_CLEAR;

    } else
    {
      if((ptChannel->usHostFlags ^ ptChannel->usNetxFlags) & ulBitMask)
        bActualState = HIL_FLAGS_NOT_EQUAL;
      else
        bActualState = HIL_FLAGS_EQUAL;
    }

    if(bActualState == bState)
    {
      iRet = 1;
      break;
    }

    /* Check for timeout */
    ulDiffTime = OS_GetMilliSecCounter() - lStartTime;
    if ( ulDiffTime > ulTimeout)
    {
      break;
    }

    OS_Sleep(0);
  }

  return iRet;
}

/*****************************************************************************/
/*! Waits for a given handshake bit state on the channel (irq mode)
*   \param ptChannel    Channel instance to wait for bitstate
*   \param ulBitNumber  BitNumber to wait for (Bitnumber is used for
*                       indexing the event array in IRQ mode)
*   \param bState       State the handshake bit should be in after returning
*                       from this function
*   \param ulTimeout    Maximum time in ms to wait for the desired bit state
*   \return 0 on error/timeout, 1 on success                                 */
/*****************************************************************************/
static int DEV_WaitForBitState_Irq(PCHANNELINSTANCE ptChannel, uint32_t ulBitNumber, uint8_t bState, uint32_t ulTimeout)
{
  uint8_t  bActualState;
  int      iRet              = 0;
  uint32_t ulBitMask         = 1 << ulBitNumber;
  int32_t  lStartTime        = 0;
  uint32_t ulInternalTimeout = ulTimeout;

  if( (HIL_FLAGS_CLEAR == bState) ||
      (HIL_FLAGS_SET == bState) )
  {
    bActualState = (ptChannel->usNetxFlags & ulBitMask)? HIL_FLAGS_SET : HIL_FLAGS_CLEAR;

  } else
  {
    if((ptChannel->usHostFlags ^ ptChannel->usNetxFlags) & ulBitMask)
      bActualState = HIL_FLAGS_NOT_EQUAL;
    else
      bActualState = HIL_FLAGS_EQUAL;
  }

  /* The desired state is already there, so just return true */
  if(bActualState == bState)
    return 1;

  /* If no timeout is given, don't try to wait for the Bit change */
  if(0 == ulTimeout)
    return 0;

  /* Just wait for the Interrupt event to be signalled. This bit was toggled if the interrupt
     is executed, so we don't need to check bit state afterwards
     Note: Wait first time with timeout 0 and check if the state is the expected one.
           If not it was a previously set event and we need to wait with the user supplied time out */

  lStartTime = (int32_t)OS_GetMilliSecCounter();

  do
  {
    uint32_t ulCurrentTime;
    uint32_t ulDiffTime;

    /* Wait for DSR to signal Handshake bit change event */
    (void)OS_WaitEvent(ptChannel->ahHandshakeBitEvents[ulBitNumber], ulInternalTimeout);

    ulCurrentTime = OS_GetMilliSecCounter();
    ulDiffTime    = ulCurrentTime - lStartTime;

    /* Adjust timeout for next run */
    ulInternalTimeout = ulTimeout - ulDiffTime;

    /* Check bit state */
    if( (HIL_FLAGS_CLEAR == bState) ||
        (HIL_FLAGS_SET == bState) )
    {
      bActualState = (ptChannel->usNetxFlags & ulBitMask)? HIL_FLAGS_SET : HIL_FLAGS_CLEAR;

    } else
    {
      if((ptChannel->usHostFlags ^ ptChannel->usNetxFlags) & ulBitMask)
        bActualState = HIL_FLAGS_NOT_EQUAL;
      else
        bActualState = HIL_FLAGS_EQUAL;
    }

    if(bActualState == bState)
    {
      iRet = 1;
      break;
    }

    if( ulDiffTime >= ulTimeout)
    {
      /* Timeout expired */
      break;
    }

  } while(iRet == 0);

  return iRet;
}

/*****************************************************************************/
/*! Waits for a given handshake bit state on the channel
*   (IRQ/Polling Wrapper function)
*   \param ptChannel    Channel instance to wait for bitstate
*   \param ulBitNumber  BitNumber to wait for (Bitnumber is used for
*                       indexing the event array in IRQ mode)
*   \param bState       State the handshake bit should be in after returning
*                       from this function
*   \param ulTimeout    Maximum time in ms to wait for the desired bit state
*   \return 0 on error/timeout, 1 on success                                 */
/*****************************************************************************/
int DEV_WaitForBitState(PCHANNELINSTANCE ptChannel, uint32_t ulBitNumber, uint8_t bState, uint32_t ulTimeout)
{
  if( ((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    return DEV_WaitForBitState_Irq(ptChannel, ulBitNumber, bState, ulTimeout);
  else
    return DEV_WaitForBitState_Poll(ptChannel, ulBitNumber, bState, ulTimeout);
}

/*****************************************************************************/
/*! Get expected handshake bit state from IOArea
*   \param ptChannel    Channel instance
*   \param ptIOInstance Pointer to IOInstance
*   \param fOutput      !=0 for output areas
*   \return Expected handshake bit state                                     */
/*****************************************************************************/
uint8_t DEV_GetIOBitstate(PCHANNELINSTANCE ptChannel, PIOINSTANCE ptIOInstance, int fOutput)
{
  uint8_t  bRet        = ptIOInstance->bHandshakeBitState;
  uint8_t* pbIOHskMode = NULL;

  if(fOutput)
    pbIOHskMode = &ptChannel->ptCommonStatusBlock->bPDOutHskMode;
  else
    pbIOHskMode = &ptChannel->ptCommonStatusBlock->bPDInHskMode;

  switch(HWIF_READ8(ptChannel->pvDeviceInstance, *pbIOHskMode))
  {
    case HIL_IO_MODE_BUFF_DEV_CTRL:
      bRet = HIL_FLAGS_NOT_EQUAL;
      break;

    case HIL_IO_MODE_UNCONTROLLED:
      bRet = HIL_FLAGS_NONE;
      break;

    case HIL_IO_MODE_BUFF_HST_CTRL:
      bRet = HIL_FLAGS_EQUAL;
      break;

    case HIL_IO_MODE_DEFAULT:
    default:
      /* Use data from channel information read on startup,
         as I/O Mode is not provided in DPM */
      break;
  }

  return bRet;
}

/*****************************************************************************/
/*! Toggles the given command handshake bit
*   \param ptChannel    Channel instance to change for bit for
*   \param ulBitMask    Bitmask to eXOR into command bits                    */
/*****************************************************************************/
void DEV_ToggleBit(PCHANNELINSTANCE ptChannel, uint32_t ulBitMask)
{
  ptChannel->usHostFlags ^= (uint16_t)ulBitMask;

  if( ptChannel->bHandshakeWidth == HIL_HANDSHAKE_SIZE_8BIT)
  {
    HWIF_WRITE8(ptChannel->pvDeviceInstance, ptChannel->ptHandshakeCell->t8Bit.bHostFlags, (uint8_t)ptChannel->usHostFlags);
  } else
  {
    /* Write 16 Bit handshake */
    HWIF_WRITE16(ptChannel->pvDeviceInstance, ptChannel->ptHandshakeCell->t16Bit.usHostFlags, HOST_TO_LE16(ptChannel->usHostFlags));
  }
}

/*****************************************************************************/
/*! Waits for Sync state on the channel (polling mode)
*   \param ptChannel    Channel instance to wait for bitstate
*   \param bState       State the handshake bit should be in after returning
*                       from this function
*   \param ulTimeout    Maximum time in ms to wait for the desired bit state
*   \return 0 on error/timeout, 1 on success                                 */
/*****************************************************************************/
static int DEV_WaitForSyncState_Poll(PCHANNELINSTANCE ptChannel, uint8_t bState, uint32_t ulTimeout)
{
  uint8_t         bActualState;
  int             iRet        = 0;
  uint32_t        ulBitMask   = 1 << ptChannel->ulChannelNumber;
  int32_t         lStartTime  = 0;
  PDEVICEINSTANCE ptDevInst   = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  DEV_ReadHandshakeFlags(ptChannel, 1, 1);

  if((ptDevInst->tSyncData.usHSyncFlags ^ ptDevInst->tSyncData.usNSyncFlags) & ulBitMask)
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
    uint32_t ulDiffTime  = 0L;

    DEV_ReadHandshakeFlags(ptChannel, 1, 1);

    if((ptDevInst->tSyncData.usHSyncFlags ^ ptDevInst->tSyncData.usNSyncFlags) & ulBitMask)
      bActualState = HIL_FLAGS_NOT_EQUAL;
    else
      bActualState = HIL_FLAGS_EQUAL;

    if(bActualState == bState)
    {
      iRet = 1;
      break;
    }

    /* Check for timeout */
    ulDiffTime = OS_GetMilliSecCounter() - lStartTime;
    if ( ulDiffTime > ulTimeout)
    {
      break;
    }

    OS_Sleep(0);
  }

  return iRet;
}

/*****************************************************************************/
/*! Waits for sync state on the channel (irq mode)
*   \param ptChannel    Channel instance to wait for bitstate
*   \param bState       State the handshake bit should be in after returning
*                       from this function
*   \param ulTimeout    Maximum time in ms to wait for the desired bit state
*   \return 0 on error/timeout, 1 on success                                 */
/*****************************************************************************/
static int DEV_WaitForSyncState_Irq(PCHANNELINSTANCE ptChannel, uint8_t bState, uint32_t ulTimeout)
{
  uint8_t         bActualState;
  int             iRet              = 0;
  uint32_t        ulBitMask         = 1 << ptChannel->ulChannelNumber;
  int32_t         lStartTime        = 0;
  uint32_t        ulInternalTimeout = ulTimeout;
  PDEVICEINSTANCE ptDevInstance     = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  if((ptDevInstance->tSyncData.usHSyncFlags ^ ptDevInstance->tSyncData.usNSyncFlags) & ulBitMask)
    bActualState = HIL_FLAGS_NOT_EQUAL;
  else
    bActualState = HIL_FLAGS_EQUAL;

  /* The desired state is already there, so just return true */
  if(bActualState == bState)
    return 1;

  /* If no timeout is given, don't try to wait for the Bit change */
  if(0 == ulTimeout)
    return 0;

  /* Just wait for the Interrupt event to be signalled. This bit was toggled if the interrupt
     is executed, so we don't need to check bit state afterwards.*/

  lStartTime = (int32_t)OS_GetMilliSecCounter();

  do
  {
    uint32_t ulCurrentTime;
    uint32_t ulDiffTime;

    /* Wait for DSR to signal Handshake bit change event */
    (void)OS_WaitEvent(ptDevInstance->tSyncData.ahSyncBitEvents[ptChannel->ulChannelNumber], ulInternalTimeout);

    ulCurrentTime = OS_GetMilliSecCounter();
    ulDiffTime    = ulCurrentTime - lStartTime;

    /* Adjust timeout for next run */
    ulInternalTimeout = ulTimeout - ulDiffTime;

    /* Check bit state */
    if((ptDevInstance->tSyncData.usHSyncFlags ^ ptDevInstance->tSyncData.usNSyncFlags) & ulBitMask)
      bActualState = HIL_FLAGS_NOT_EQUAL;
    else
      bActualState = HIL_FLAGS_EQUAL;

    if(bActualState == bState)
    {
      iRet = 1;
      break;
    }

    if( ulDiffTime >= ulTimeout)
    {
      /* Timeout expired */
      break;
    }

  } while(iRet == 0);

  return iRet;
}

/*****************************************************************************/
/*! Waits for sync state
*   (IRQ/Polling Wrapper function)
*   \param ptChannel    Channel instance to wait for bitstate
*   \param bState       State the handshake bit should be in after returning
*                       from this function
*   \param ulTimeout    Maximum time in ms to wait for the desired bit state
*   \return 0 on error/timeout, 1 on success                                 */
/*****************************************************************************/
int DEV_WaitForSyncState(PCHANNELINSTANCE ptChannel, uint8_t bState, uint32_t ulTimeout)
{
  if( ((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    return DEV_WaitForSyncState_Irq(ptChannel, bState, ulTimeout);
  else
    return DEV_WaitForSyncState_Poll(ptChannel, bState, ulTimeout);
}

/*****************************************************************************/
/*! Toggles the given sync bit
*   \param ptDevInstance  Device instance
*   \param ulBitMask      Bitmask to eXOR into command bits                  */
/*****************************************************************************/
void DEV_ToggleSyncBit(PDEVICEINSTANCE ptDevInstance, uint32_t ulBitMask)
{
  /* Write 16 Bit handshake */
  HIL_DPM_HANDSHAKE_ARRAY_T* ptHandshakeBlock = (HIL_DPM_HANDSHAKE_ARRAY_T*)ptDevInstance->pbHandshakeBlock;

  OS_EnterLock(ptDevInstance->tSyncData.pvLock);

  ptDevInstance->tSyncData.usHSyncFlags ^= (uint16_t)ulBitMask;
  HWIF_WRITE16(ptDevInstance, ptHandshakeBlock->atHsk[1].t16Bit.usHostFlags, HOST_TO_LE16(ptDevInstance->tSyncData.usHSyncFlags));

  OS_LeaveLock(ptDevInstance->tSyncData.pvLock);
}

/*****************************************************************************/
/*! Reads the actual state of the host handshake bits for the given channel
*   \param ptChannel    Channel instance to change for bit for
*   \param fReadHostCOS !=0 if Application COS should be read                */
/*****************************************************************************/
void DEV_ReadHostFlags(PCHANNELINSTANCE ptChannel, int fReadHostCOS)
{
  PDEVICEINSTANCE ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  if(ptChannel->bHandshakeWidth == HIL_HANDSHAKE_SIZE_8BIT)
    ptChannel->usHostFlags = HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptHandshakeCell->t8Bit.bHostFlags);
  else
    ptChannel->usHostFlags = LE16_TO_HOST(HWIF_READ16(ptChannel->pvDeviceInstance, ptChannel->ptHandshakeCell->t16Bit.usHostFlags));

  /* Also read host sync flags, as they might not be set to zero on flash based devices */
  if( (ptDevInstance->pbHandshakeBlock != NULL) &&
      (ptChannel->fIsSysDevice) )
  {
    HIL_DPM_HANDSHAKE_ARRAY_T* ptHandshakeBlock = (HIL_DPM_HANDSHAKE_ARRAY_T*)ptDevInstance->pbHandshakeBlock;
    ptDevInstance->tSyncData.usHSyncFlags       = LE16_TO_HOST(HWIF_READ16(ptChannel->pvDeviceInstance, ptHandshakeBlock->atHsk[1].t16Bit.usHostFlags));
  }

  if(NULL != ptChannel->ptCommonStatusBlock)
    ptChannel->ulDeviceCOSFlags = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->ulCommunicationCOS));

  if( fReadHostCOS)
  {
    if(NULL != ptChannel->ptControlBlock)
    {
      ptChannel->ulHostCOSFlags       = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptControlBlock->ulApplicationCOS));
      ptChannel->ulHostCOSFlagsSaved  = ptChannel->ulHostCOSFlags;
  }
  }
}

/*****************************************************************************/
/*! Reads the actual state of the handshake bits for the given channel
*   \param ptChannel      Channel instance to change for bit for
*   \param fReadSyncFlags !=0 if sync flags should be updated
*   \param fLockNeeded    !=0 if flag access lock is needed.                 */
/*****************************************************************************/
void DEV_ReadHandshakeFlags(PCHANNELINSTANCE ptChannel, int fReadSyncFlags, int fLockNeeded)
{
  uint16_t  usCOSAckBitMask = 0;
  uint32_t  ulNewCOSFlags   = 0;

  /* Read sync flags */
  PDEVICEINSTANCE ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  /* Lock Handshake Cell and COS flag accesses */
  if(fLockNeeded)
    OS_EnterLock(ptChannel->pvLock);

  if( (ptDevInstance->pbHandshakeBlock != NULL) &&
      fReadSyncFlags )
  {
    HIL_DPM_HANDSHAKE_ARRAY_T* ptHandshakeBlock = (HIL_DPM_HANDSHAKE_ARRAY_T*)ptDevInstance->pbHandshakeBlock;
    ptDevInstance->tSyncData.usNSyncFlags       = LE16_TO_HOST(HWIF_READ16(ptDevInstance, ptHandshakeBlock->atHsk[1].t16Bit.usNetxFlags));
  }

  if(ptChannel->bHandshakeWidth == HIL_HANDSHAKE_SIZE_8BIT)
  {
    /* Read 8 Bit handshake */
    ptChannel->usNetxFlags = HWIF_READ8(ptDevInstance, ptChannel->ptHandshakeCell->t8Bit.bNetxFlags);
  } else
  {
    /* Read 16 Bit handshake */
    ptChannel->usNetxFlags = LE16_TO_HOST(HWIF_READ16(ptDevInstance, ptChannel->ptHandshakeCell->t16Bit.usNetxFlags));
  }

  /* Read device COS command state two times */
  if(ptChannel->fIsSysDevice)
  {
    /* This is the system device */
    HIL_DPM_SYSTEM_CHANNEL_T* ptSysChannel = (HIL_DPM_SYSTEM_CHANNEL_T*)ptChannel->pbDPMChannelStart;
    if ((ptChannel->usNetxFlags ^ ptChannel->usHostFlags) & NSF_NETX_COS_CMD)
    {
      ulNewCOSFlags   = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemCOS)); /* Read actual COS flags */
      usCOSAckBitMask = HSF_NETX_COS_ACK;
    }
  } else if(NULL != ptChannel->ptCommonStatusBlock)
  {
    /* This is a communication channel */
    if ((ptChannel->usNetxFlags ^ ptChannel->usHostFlags) & NCF_NETX_COS_CMD)
    {
      ulNewCOSFlags   = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptChannel->ptCommonStatusBlock->ulCommunicationCOS)); /* Read actual COS flags */
      usCOSAckBitMask = HCF_NETX_COS_ACK;
    }
  }

  if (usCOSAckBitMask)
  {
    /* Read the flags and acknowledge */
    if(ptChannel->ulDeviceCOSFlags != ulNewCOSFlags)
    {
      ptChannel->ulDeviceCOSFlagsChanged  = ptChannel->ulDeviceCOSFlags ^ ulNewCOSFlags;
      ptChannel->ulDeviceCOSFlags         = ulNewCOSFlags;
    }

    DEV_ToggleBit(ptChannel, usCOSAckBitMask);
  }

  /* Unlock Handshake Cell and COS flag accesses */
  if(fLockNeeded)
    OS_LeaveLock(ptChannel->pvLock);
}

/*****************************************************************************/
/*! Wait for NOT READY in poll mode
*   \param ptChannel Channel instance to check
*   \param ulTimeout Wait time
*   \return 1 if channel is NOT ready                                            */
/*****************************************************************************/
int DEV_WaitForNotReady_Poll(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout)
{
  /* Poll for Ready bit */
  int      iActualState = 0;
  uint32_t ulDiffTime   = 0L;
  int32_t  lStartTime   = (int32_t)OS_GetMilliSecCounter();

  /* We do nothing without a timeout */
  if( ulTimeout == 0)
    return iActualState;

  /* Check which READY to use */
  if(ptChannel->fIsSysDevice)
  {
    /* This is the system channel which will reset the whole card */
    do
    {
      /* Check if firmware is READY because we need the DPM Layout */
      DEVICEINSTANCE* ptDevInstance  = (DEVICEINSTANCE*)ptChannel->pvDeviceInstance;

      if( (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptDevInstance->tSystemDevice.ptHandshakeCell->ulValue)) == CIFX_DPM_INVALID_CONTENT)     ||
          (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptDevInstance->tSystemDevice.ptHandshakeCell->ulValue)) == CIFX_DPM_NO_MEMORY_ASSIGNED)  ||
          (0 == (HWIF_READ8(ptChannel->pvDeviceInstance, ptDevInstance->tSystemDevice.ptHandshakeCell->t8Bit.bNetxFlags) & NSF_READY))     )
      {
        /* Card is not ready anymore */
        iActualState = 1;
        break;
      }
      /* Check time */
      ulDiffTime = OS_GetMilliSecCounter() - lStartTime;

      OS_Sleep(0);

    } while (ulDiffTime < ulTimeout);

  } else
  {
    /* This is a communication channel which is restarted */
    do
    {
      if( (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->ulCommunicationCOS)) == CIFX_DPM_INVALID_CONTENT)     ||
          (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->ulCommunicationCOS)) == CIFX_DPM_NO_MEMORY_ASSIGNED)  ||
          (!DEV_IsReady(ptChannel))                                                            )
      {
        /* Channel is not READY anymore */
        iActualState = 1;
        break;
      }

      /* Check time */
      ulDiffTime = OS_GetMilliSecCounter() - lStartTime;

      /* Wait until firmware is down */
      OS_Sleep(1);

    } while (ulDiffTime < ulTimeout);
  }

  return iActualState;
}

/*****************************************************************************/
/*! Wait for READY in poll mode
*   \param ptChannel Channel instance to check
*   \param ulTimeout Wait time
*   \return 1 if channel is ready                                            */
/*****************************************************************************/
int DEV_WaitForReady_Poll(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout)
{
  /* Poll for Ready bit */
  int      iActualState = 0;
  uint32_t ulDiffTime   = 0L;
  int32_t  lStartTime   = (int32_t)OS_GetMilliSecCounter();

  /* We do nothing without a timeout */
  if( ulTimeout == 0)
    return iActualState;

  /* Check which READY to use */
  if(ptChannel->fIsSysDevice)
  {
    /* This is the system channel of the whole card */
    /* Wait until firmware is running */
    OS_Sleep( 10);

    do
    {
      DEVICEINSTANCE* ptDevInstance  = (DEVICEINSTANCE*)ptChannel->pvDeviceInstance;
      char            szCookie[5]    = {0};

      /* Read the DPM cookie */
      HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);

      /* We need to check for a valid cookie */
      if ( (0 == OS_Strcmp( szCookie, CIFX_DPMSIGNATURE_BSL_STR)) ||
           (0 == OS_Strcmp( szCookie, CIFX_DPMSIGNATURE_FW_STR)) )
      {
        /* Check if firmware is READY because we need the DPM Layout */
        if( (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptDevInstance->tSystemDevice.ptHandshakeCell->ulValue)) != CIFX_DPM_INVALID_CONTENT) &&
            (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptDevInstance->tSystemDevice.ptHandshakeCell->ulValue)) != CIFX_DPM_NO_MEMORY_ASSIGNED)  )
        {
          /* Check if firmware is READY because we need the DPM Layout */
          if(HWIF_READ8(ptChannel->pvDeviceInstance, ptDevInstance->tSystemDevice.ptHandshakeCell->t8Bit.bNetxFlags) & NSF_READY)
          {
            DEV_ReadHostFlags(&ptDevInstance->tSystemDevice, 0);

            iActualState = 1;
            break;
          }
        }
      }
      ulDiffTime = OS_GetMilliSecCounter() - lStartTime;

      /* Wait until firmware is running */
      OS_Sleep(1);

    } while ( ulDiffTime < ulTimeout);
  } else
  {
    /* This is a communication channel which is restarted */
    /* Check if this is a real channel (not for bootloader */
    if( ptChannel->fIsChannel)
    {
      do
      {
        /* Wait until the channel is running */
        OS_Sleep( 1);

        /* Wait for READY */
        if( DEV_IsReady(ptChannel))
        {
          DEV_ReadHostFlags(ptChannel, 0);
          iActualState = 1;
          break;
        }
        ulDiffTime = OS_GetMilliSecCounter() - lStartTime;

      } while ( ulDiffTime < ulTimeout);
    }
  }

  return iActualState;
}

/*****************************************************************************/
/*! Wait for NOT RUNNING in poll mode
*   \param ptChannel Channel instance to check
*   \param ulTimeout Wait time
*   \return 1 if channel is NOT running                                            */
/*****************************************************************************/
int DEV_WaitForNotRunning_Poll(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout)
{
  /* Poll for Ready bit */
  int      iActualState = 1;
  uint32_t ulDiffTime   = 0L;
  int32_t  lStartTime   = (int32_t)OS_GetMilliSecCounter();

  /* We not processing a system channel */
  if(ptChannel->fIsSysDevice)
    return iActualState;

  /* Check user timeout */
  if( 0 == ulTimeout)
  {
    if( DEV_IsRunning(ptChannel))
      iActualState = 0;
  } else
  {
    /* User wants to wait */
    while(DEV_IsRunning(ptChannel))
    {
      /* Check for timeout */
      ulDiffTime = OS_GetMilliSecCounter() - lStartTime;

      if(ulDiffTime > ulTimeout)
      {
        iActualState = 0;
        break;
      }

      OS_Sleep(1);
    }
  }

  return iActualState;
}

/*****************************************************************************/
/*! Wait for RUNNING in poll mode
*   \param ptChannel Channel instance to check
*   \param ulTimeout Wait time
*   \return 1 if channel is RUNNING                                            */
/*****************************************************************************/
int DEV_WaitForRunning_Poll(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout)
{
  /* Poll for Ready bit */
  int      iActualState = 1;
  uint32_t ulDiffTime   = 0L;
  int32_t  lStartTime   = (int32_t)OS_GetMilliSecCounter();

  /* We not processing a system channel, so always return a valid state */
  if(ptChannel->fIsSysDevice)
    return iActualState;

  /* Check user timeout */
  if( 0 == ulTimeout)
  {
    /* Just return the actual state */
    iActualState = DEV_IsRunning(ptChannel);
  } else
  {
    /* User wants to wait */
    while(!DEV_IsRunning(ptChannel))
    {
      /* Check for timeout */
      ulDiffTime = OS_GetMilliSecCounter() - lStartTime;

      if(ulDiffTime > ulTimeout)
      {
        iActualState = 0;
        break;
      }

      OS_Sleep(1);
    }
  }

  return iActualState;
}

/*****************************************************************************/
/*! Writes the saved state of the handshake bits to the given channel
*   \param ptChannel Channel instance to write bits to                       */
/*****************************************************************************/
void DEV_WriteHandshakeFlags(PCHANNELINSTANCE ptChannel)
{
  if(ptChannel->bHandshakeWidth == HIL_HANDSHAKE_SIZE_8BIT)
  {
    /* Read 8 Bit handshake */
    HWIF_WRITE8(ptChannel->pvDeviceInstance, ptChannel->ptHandshakeCell->t8Bit.bHostFlags, (uint8_t)ptChannel->usHostFlags);
  } else
  {
    /* Read 16 Bit handshake */
    HWIF_WRITE16(ptChannel->pvDeviceInstance, ptChannel->ptHandshakeCell->t16Bit.usHostFlags, HOST_TO_LE16(ptChannel->usHostFlags));
  }
}

/*****************************************************************************/
/*! Checks if the channel is ready
*   \param ptChannel Channel instance to check
*   \return 1 if channel is ready                                            */
/*****************************************************************************/
int DEV_IsReady(PCHANNELINSTANCE ptChannel)
{
  int iRet = 0;

  /* Handshake flags are read on interrupt, so no need to read them here */
  if(!((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    DEV_ReadHandshakeFlags(ptChannel, 0, 1);

  if(ptChannel->fIsSysDevice)
  {
    if(ptChannel->usNetxFlags & NSF_READY)
    {
      iRet = 1;
    }
  } else
  {
    if(ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_READY)
    {
      iRet = 1;
    }
  }

  return iRet;
}

/*****************************************************************************/
/*! Checks if the channel is running
*   \param ptChannel Channel instance to check
*   \return 1 if channel is ready and running                                */
/*****************************************************************************/
int DEV_IsRunning(PCHANNELINSTANCE ptChannel)
{
  int iRet = 0;

  /* Handshake flags are read on interrupt, so no need to read them here */
  if(!((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    DEV_ReadHandshakeFlags(ptChannel, 0, 1);

  /* only a Communication channel can be running */
  if(!ptChannel->fIsSysDevice)
  {
    if( (ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_READY) &&
        (ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_RUN) )
    {
      iRet = 1;
    }
  }

  return iRet;
}

/*****************************************************************************/
/*! Checks if the channel is communicating
*   \param ptChannel Channel instance to check
*   \param plError   CIFX_NO_ERROR on successful read
*   \return 1 if channel is communicating                                    */
/*****************************************************************************/
int DEV_IsCommunicating(PCHANNELINSTANCE ptChannel, int32_t* plError)
{
  int iRet = 0;

  /* Only communication channels are allowed */
  if( ptChannel->fIsSysDevice)
  {
    *plError = CIFX_INVALID_HANDLE;

  /* Handshake flags are read during DEV_IsReady() */
  }else if( !DEV_IsReady(ptChannel))
  {
    *plError = CIFX_DEV_NOT_READY;

  } else if( !(ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_RUN))
  {
    *plError = CIFX_DEV_NOT_RUNNING;

  } else if ( ptChannel->usNetxFlags & NCF_COMMUNICATING)
  {
    iRet = 1;
    *plError = CIFX_NO_ERROR;
  } else
  {
    *plError = CIFX_DEV_NO_COM_FLAG;
  }

  return iRet;
}

/*****************************************************************************/
/*! Returns the actual state of the device mailbox
*   \param ptChannel      Channel instance to check
*   \param pulRecvPktCnt  Number of pending packets to receive
*   \param pulSendPktCnt  Number of packets that can be sent to the device
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_GetMBXState(PCHANNELINSTANCE ptChannel, uint32_t* pulRecvPktCnt, uint32_t* pulSendPktCnt)
{
  int32_t lRet = CIFX_NO_ERROR;

  /* Is Device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

  /* Check if mailbox is available */
  } else if(ptChannel->tRecvMbx.ulRecvMailboxLength == 0)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;

  /* Check if device is READY */
  } else if(!DEV_IsReady(ptChannel))
  {
    lRet = CIFX_DEV_NOT_READY;
  } else
  {
    /* Get receive MBX state */
    *pulRecvPktCnt = LE16_TO_HOST(HWIF_READ16(ptChannel->pvDeviceInstance, ptChannel->tRecvMbx.ptRecvMailboxStart->usWaitingPackages));

    /* Get send MBX state */
    *pulSendPktCnt = LE16_TO_HOST(HWIF_READ16(ptChannel->pvDeviceInstance, ptChannel->tSendMbx.ptSendMailboxStart->usPackagesAccepted));
  }

  return lRet;
}

/*****************************************************************************/
/*! Triggers/Disables the cifX application Watchdog
*   \param ptChannel        Channel instance to trigger watchdog on
*   \param ulTriggerCmd     CIFX_WATCHDOG_START to start/trigger watchdog,
*                           CIFX_WATCHDOG_STOP to stop watchdog
*   \param pulTriggerValue  Last watchdog trigger value
*                           (informational use only)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_TriggerWatchdog(PCHANNELINSTANCE ptChannel, uint32_t ulTriggerCmd, uint32_t* pulTriggerValue)
{
  int32_t lRet = CIFX_DEV_NOT_RUNNING;

  if( (NULL == ptChannel)       ||
      (NULL == pulTriggerValue) )
    return CIFX_INVALID_POINTER;

  /* Is Device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    /* Init error occurred */
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

  /* Check if device is running */
  } else if(DEV_IsRunning(ptChannel))
  {
    lRet = CIFX_NO_ERROR;

    /* Process command */
    if(ulTriggerCmd == CIFX_WATCHDOG_START)
    {
      /* Copy host value to device value */
      *pulTriggerValue = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->ulHostWatchdog));
      HWIF_WRITE32(ptChannel->pvDeviceInstance, ptChannel->ptControlBlock->ulDeviceWatchdog, HOST_TO_LE32(*pulTriggerValue));

    } else if(ulTriggerCmd == CIFX_WATCHDOG_STOP)
    {
      /* Stop watchdog function */
      HWIF_WRITE32(ptChannel->pvDeviceInstance, ptChannel->ptControlBlock->ulDeviceWatchdog, 0);
      *pulTriggerValue = 0;

    } else
    {
      /* Unknown command */
      lRet = CIFX_INVALID_COMMAND;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Handle the application BUS state COS flag
*   \param ptChannel        Channel instance
*   \param ulCmd            new state to set (CIFX_BUS_STATE_ON / CIFX_BUS_STATE_OFF)
*   \param pulState         Buffer to store actual state
*   \param ulTimeout        timeout to wait for communication to start/stop
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_BusState(PCHANNELINSTANCE ptChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout)
{
  int32_t lRet = CIFX_NO_ERROR;

  if( NULL == pulState) return CIFX_INVALID_POINTER;

  /* Read actual BUS state */
  *pulState = (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->ulCommunicationCOS)) & HIL_COMM_COS_BUS_ON) ? CIFX_BUS_STATE_ON : CIFX_BUS_STATE_OFF;

  switch (ulCmd)
  {
    case CIFX_BUS_STATE_ON:
    {
      /* Check if the BUS is already ON */
      (void)DEV_IsCommunicating(ptChannel, &lRet); /* lRet evaluated */

      if( !*pulState &&
          (CIFX_DEV_NO_COM_FLAG == lRet) )
      {
        /* BUS is OFF */
        int32_t lTemp = DEV_DoHostCOSChange(ptChannel,
                                         HIL_APP_COS_BUS_ON | HIL_APP_COS_BUS_ON_ENABLE, /* set mask        */
                                         0,                                              /* clear mask      */
                                         HIL_APP_COS_BUS_ON_ENABLE,                      /* post clear mask */
                                         CIFX_DEV_BUS_STATE_ON_TIMEOUT,
                                         ulTimeout);
        /* Only update return value, if handshaking did not succeed, so
           we can wait for COM_BIT below */
        if(lTemp != CIFX_NO_ERROR)
          lRet = lTemp;
      }

      if(ulTimeout && (CIFX_DEV_NO_COM_FLAG == lRet))
      {
        /* Wait for Bus is active if user want it */
        if (DEV_WaitForBitState( ptChannel, NCF_COMMUNICATING_BIT_NO, HIL_FLAGS_SET, ulTimeout))
        {
          lRet = CIFX_NO_ERROR;
        } else
        {
          /* Return Error */
          lRet = CIFX_DEV_NO_COM_FLAG;
        }

        *pulState = (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->ulCommunicationCOS)) & HIL_COMM_COS_BUS_ON) ?
                     CIFX_BUS_STATE_ON : CIFX_BUS_STATE_OFF;
      }
    }
    break;

    case CIFX_BUS_STATE_OFF:
    {
      int fWaitCommFlag = 1;

      /* Check if the BUS is off */
      if(!DEV_IsReady(ptChannel))
      {
        lRet = CIFX_DEV_NOT_READY;
        fWaitCommFlag = 0;

      } else if(*pulState || DEV_IsCommunicating(ptChannel, &lRet))
      {
        /* BUS is ON */
        lRet = DEV_DoHostCOSChange(ptChannel,
                                   HIL_APP_COS_BUS_ON_ENABLE,        /* set mask        */
                                   HIL_APP_COS_BUS_ON,               /* clear mask      */
                                   HIL_APP_COS_BUS_ON_ENABLE,        /* post clear mask */
                                   CIFX_DEV_BUS_STATE_OFF_TIMEOUT,
                                   ulTimeout);


        if(CIFX_DEV_FUNCTION_FAILED == lRet)
        {
          fWaitCommFlag = 0;
        }
      }

      /* Check if user wants to wait for the BUS state */
      if(ulTimeout && fWaitCommFlag)
      {
        /* Wait until BUS is OFF */
        if(DEV_WaitForBitState(ptChannel, NCF_COMMUNICATING_BIT_NO, HIL_FLAGS_CLEAR, ulTimeout))
        {
          /* Set actual state */
          lRet = CIFX_NO_ERROR;
        } else
        {
          /* Return error */
          lRet = CIFX_DEV_BUS_STATE_OFF_TIMEOUT;
        }

        *pulState = (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->ulCommunicationCOS)) & HIL_COMM_COS_BUS_ON) ?
                     CIFX_BUS_STATE_ON : CIFX_BUS_STATE_OFF;
      }
    }
    break;

    case CIFX_BUS_STATE_GETSTATE:
    {
      /* Update the COS flags */
      if (0 == DEV_IsRunning(ptChannel))
        lRet = CIFX_DEV_NOT_RUNNING;
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
/*! Read the application COS flag state
*   \param ptChannel        Channel instance
*   \param pulState         returned host state (CIFX_HOST_STATE_READY /
*                           CIFX_HOST_STATE_NOT_READY)
*   \return CIFX_NO_ERROR on success, or CIFX_DEV_NOT_READY                  */
/*****************************************************************************/
int32_t DEV_GetHostState(PCHANNELINSTANCE ptChannel, uint32_t* pulState)
{
  /* Don't return any state if card is not ready */
  if(!DEV_IsReady(ptChannel))
    return CIFX_DEV_NOT_READY;

  *pulState = (ptChannel->ulHostCOSFlags & HIL_APP_COS_APPLICATION_READY)? CIFX_HOST_STATE_READY : CIFX_HOST_STATE_NOT_READY;

  return CIFX_NO_ERROR;
}

/*****************************************************************************/
/*! Set the application ready COS flag
*   \param ptChannel        Channel instance
*   \param ulNewState       new state to set (CIFX_HOST_STATE_READY /
*                           CIFX_HOST_STATE_NOT_READY)
*   \param ulTimeout        timeout to wait for communication to start/stop
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_SetHostState(PCHANNELINSTANCE ptChannel, uint32_t ulNewState, uint32_t ulTimeout)
{
  int32_t lRet = CIFX_NO_ERROR;

  UNREFERENCED_PARAMETER(ulTimeout);    /* prevent compiler warnings */

  /* Don't set host state if card is not configured */
  if(!DEV_IsReady(ptChannel))
    return CIFX_DEV_NOT_READY;

  switch(ulNewState)
  {
  case CIFX_HOST_STATE_NOT_READY:

    /* Check user timeout */
    if( 0 == ulTimeout)
    {
      /* Just set the state */
      /* Lock flag access */
      OS_EnterLock(ptChannel->pvLock);

      /* Clear the application ready flag */
      ptChannel->ulHostCOSFlags &= ~HIL_APP_COS_APPLICATION_READY;

      /* Unlock flag access */
      OS_LeaveLock(ptChannel->pvLock);
    } else
    {
      lRet = DEV_DoHostCOSChange(ptChannel,
                                 0,                              /* set mask        */
                                 HIL_APP_COS_APPLICATION_READY,  /* clear mask      */
                                 0,                              /* post clear mask */
                                 CIFX_DEV_HOST_STATE_CLEAR_TIMEOUT,
                                 ulTimeout);
    }
    break;

  case CIFX_HOST_STATE_READY:
    /* Check user timeout */
    if( 0 == ulTimeout)
    {
      /* Just set the state */
      /* Lock flag access */
      OS_EnterLock(ptChannel->pvLock);

      /* Clear the application ready flag */
      ptChannel->ulHostCOSFlags |= HIL_APP_COS_APPLICATION_READY;

      /* Unlock flag access */
      OS_LeaveLock(ptChannel->pvLock);
    } else
    {
      lRet = DEV_DoHostCOSChange(ptChannel,
                                 HIL_APP_COS_APPLICATION_READY,  /* set mask        */
                                 0,                              /* clear mask      */
                                 0,                              /* post clear mask */
                                 CIFX_DEV_HOST_STATE_SET_TIMEOUT,
                                 ulTimeout);
    }
    break;

  default:
    lRet = CIFX_INVALID_COMMAND;
    break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Read/Write Block
*   \param ptChannel      Channel Instance
*   \param pvBlock        Pointer to the block to copy
*   \param ulOffset       Start offset to copy from/to
*   \param ulBlockLen     Total Length of the Block
*   \param pvDest         Source/Destination buffer
*   \param ulDestLen      Length of the Source/Destination Buffer
*   \param ulCmd          CIFX_CMD_READ_DATA/CIFX_CMD_WRITE_DATA
*   \param fWriteAllowed  !=0 if Write is allowed to the Block
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_ReadWriteBlock(PCHANNELINSTANCE ptChannel, void* pvBlock, uint32_t ulOffset, uint32_t ulBlockLen, void* pvDest, uint32_t ulDestLen, uint32_t ulCmd, int fWriteAllowed)
{
  int32_t lRet = CIFX_NO_ERROR;

  if( (ulOffset + ulDestLen) > ulBlockLen)
    return CIFX_INVALID_ACCESS_SIZE; /* Size too long */

  /* Process the state block area command */
  switch (ulCmd)
  {
    case  CIFX_CMD_WRITE_DATA:
      if(fWriteAllowed)
      {
        /* Write control block */
        HWIF_WRITEN( ptChannel->pvDeviceInstance,
                     ((uint8_t*)pvBlock) + ulOffset,
                      (uint8_t *)pvDest,
                      ulDestLen);
      } else
      {
        lRet = CIFX_INVALID_COMMAND;
      }
      break;

    case CIFX_CMD_READ_DATA:
      /* It is allowed to read the control block back */
      HWIF_READN( ptChannel->pvDeviceInstance,
                  (uint8_t *)pvDest,
                  ((uint8_t*)pvBlock) + ulOffset,
                  ulDestLen);
      break;

    default:
      /* Unknown command */
      lRet = CIFX_INVALID_COMMAND;
      break;
  } /* end switch */

  /* Always deliver back system errors */
  if( (CIFX_NO_ERROR == lRet) &&
      !DEV_IsRunning(ptChannel) )
    lRet = CIFX_DEV_NOT_RUNNING;

  return lRet;
}

/*****************************************************************************/
/*! Returns the state of the given handshake bit/mask
*   \param ptChannel        Channel instance
*   \param ulBitMsk         Bitmask to check for
*   \return HIL_FLAGS_EQUAL/HIL_FLAGS_NOT_EQUAL                              */
/*****************************************************************************/
uint8_t DEV_GetHandshakeBitState(PCHANNELINSTANCE ptChannel, uint32_t ulBitMsk)
{
  uint8_t  bRet        = HIL_FLAGS_EQUAL;

  /* Handshake flags are read on interrupt, so no need to read them here */
  if(!((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    DEV_ReadHandshakeFlags(ptChannel, 0, 1);

  if((ptChannel->usHostFlags ^ ptChannel->usNetxFlags) & ulBitMsk)
    bRet = HIL_FLAGS_NOT_EQUAL;

  return bRet;
}

/*****************************************************************************/
/*! Check the COS flags on this device
*   \param ptDevInstance  Device instance                                    */
/*****************************************************************************/
void DEV_CheckCOSFlags(PDEVICEINSTANCE ptDevInstance)
{
  /* Note: We assume, we only get here in polling mode */
  uint32_t ulChannel;

  if(!OS_WaitMutex(ptDevInstance->tSystemDevice.pvInitMutex, 0))
  {
    if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_DEBUG,
                "DEV_CheckCOSFlags(): Skipping COS Flag handling. Device is in system reset!");
    }
  } else
  {
    /* Evaluate COS bits on Communication channels */
    for(ulChannel = 0; ulChannel < ptDevInstance->ulCommChannelCount; ulChannel++)
    {
      PCHANNELINSTANCE ptChannel    = ptDevInstance->pptCommChannels[ulChannel];
      uint32_t    ulCOSChanged = 0;

      /* Check if we have an valid channel (not for the bootloader) */
      if( (0 == ptChannel->ptControlBlock)      ||
          (0 == ptChannel->ptCommonStatusBlock) )
        return;

      if(!OS_WaitMutex(ptChannel->pvInitMutex, 0))
      {
        if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
        {
          USER_Trace(ptDevInstance,
                    TRACE_LEVEL_DEBUG,
                    "DEV_CheckCOSFlags(): Skipping Channel #%d, which is currently initializing!",
                    ulChannel);
        }

      } else
      {
        /*------------------------------------------*/
        /* Process our own COS flags                */
        /*------------------------------------------*/
        if( ptChannel->ulHostCOSFlags != LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptChannel->ptControlBlock->ulApplicationCOS)))
        {
          /* We have to update our COS flags */
          /* Check if we can signal a new COS state */
          if( DEV_WaitForBitState(ptChannel, HCF_HOST_COS_CMD_BIT_NO, HIL_FLAGS_EQUAL, 0))
          {
            /* Lock flag access */
            OS_EnterLock(ptChannel->pvLock);

            /* Update flags */
            HWIF_WRITE32(ptDevInstance, ptChannel->ptControlBlock->ulApplicationCOS, HOST_TO_LE32(ptChannel->ulHostCOSFlags));
            ptChannel->ulHostCOSFlagsSaved = ptChannel->ulHostCOSFlags;

            /* Signal new COS flags */
            DEV_ToggleBit(ptChannel, HCF_HOST_COS_CMD);

            /* Remove all enable flags from the local COS flags */
            ptChannel->ulHostCOSFlags &= ~(HIL_APP_COS_BUS_ON_ENABLE | HIL_APP_COS_INITIALIZATION_ENABLE | HIL_APP_COS_LOCK_CONFIGURATION_ENABLE);

            OS_LeaveLock(ptChannel->pvLock);
          }
        }
        /*------------------------------------------*/
        /* Process now Hardware COS flags           */
        /*------------------------------------------*/
        /* Handshake flags are read on interrupt, so no need to read them here */
        if(!((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
          DEV_ReadHandshakeFlags(ptChannel, 0, 1);

        /* Get the changed COS flags bitmask */
        ulCOSChanged = ptChannel->ulDeviceCOSFlagsChanged;

        if(ulCOSChanged != 0)
        {

          /* TODO: Signal change event */
        }

  #if 0
        if(ulCOSChanged & HIL_COMM_COS_RESTART_REQUIRED)
        {
          /* Firmware requests a restart */

        }

        if(ulCOSChanged & HIL_COMM_COS_CONFIG_AVAIL)
        {
          /* Configuration changed state */

        }

        if(ulCOSChanged & HIL_COMM_COS_CONFIG_LOCKED)
        {
          /* Configuration locked */

        }
  #endif

        /* We've processed all pending COS flags on this channel */
        ptChannel->ulDeviceCOSFlagsChanged &= ~ulCOSChanged;

        OS_ReleaseMutex(ptChannel->pvInitMutex);
      }
    }
    OS_ReleaseMutex(ptDevInstance->tSystemDevice.pvInitMutex);
  }
}

/*****************************************************************************/
/*! Performs a channel initialization
*   \param ptChannel Channel instance
*   \param ulTimeout Timeout to wait for channel to become READY
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_DoChannelInit(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout)
{
  int32_t         lRet        = CIFX_NO_ERROR;
  PDEVICEINSTANCE ptDevInst   = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  int             fRunning    = DEV_IsRunning(ptChannel);

  if(!OS_WaitMutex(ptChannel->pvInitMutex, CIFX_TO_WAIT_COS_CMD))
  {
    /* This should only happen, if the DEV_CheckCOSFlags function is still busy checking
       for COS changed on this channel */
    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInst,
                TRACE_LEVEL_ERROR,
                "DEV_DoChannelInit(): Error getting Mutex. Access to device COS flags is locked!");
    }

    lRet = CIFX_DRV_CMD_ACTIVE;

  } else
  {
    lRet = DEV_DoHostCOSChange(ptChannel,
                               HIL_APP_COS_INITIALIZATION | HIL_APP_COS_INITIALIZATION_ENABLE, /* set mask        */
                               0,                                                              /* clear mask      */
                               HIL_APP_COS_INITIALIZATION_ENABLE,                              /* post clear mask */
                               CIFX_DEV_FUNCTION_FAILED,
                               CIFX_TO_WAIT_COS_CMD);

    /* Signal Initialisation */
    if(CIFX_NO_ERROR == lRet)
    {
      /* The card has recognized the initialisation, so we can wait until the card has processed it*/
      /* Card was running before, so wait for running flag to vanish */
      if(fRunning)
      {
        /* Check if the Firmware has removed it's running flag,
            or if it's set now, and it was changed during last COS */
        if( (0 == (ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_RUN)) ||
            ( (ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_RUN) &&
              (ptChannel->ulDeviceCOSFlagsChanged & HIL_COMM_COS_RUN) ) )
        {
          /* FW already removed it's RUN Flag during Channel Init command sequence. No need to
              wait for running flag to vanish */
          if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
          {
            USER_Trace(ptDevInst,
                      TRACE_LEVEL_DEBUG,
                      "DEV_DoChannelInit(): Firmware removed HIL_COMM_COS_RUN early! Skipping wait for NotRunning-State");
          }

        } else if( !DEV_WaitForNotRunning_Poll( ptChannel, CIFX_TO_WAIT_HW_RESET_ACTIVE))
        {
          lRet = CIFX_DEV_RESET_TIMEOUT;
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInst,
                      TRACE_LEVEL_ERROR,
                      "DEV_DoChannelInit(): Error waiting for channel to leave running state!");
          }
        }
      }

      /* Card is in restart */
      if(CIFX_NO_ERROR == lRet)
      {
        /* Check if user wants to wait until the card is READY again */
        /* now wait for the channel and it must be at least READY */
        uint32_t ulTempTimeout = ( CIFX_TO_WAIT_HW > ulTimeout) ? ulTimeout : CIFX_TO_WAIT_HW;
        if( DEV_WaitForNotReady_Poll( ptChannel, ulTempTimeout) )
        {
          /* Firmware started after warm start process */
          if( 0 != ulTimeout)
          {
            /* now wait for the channel and it must be at least READY */
            if( !DEV_WaitForReady_Poll( ptChannel, ulTimeout) )
            {
              lRet = CIFX_DEV_NOT_READY;
              if(g_ulTraceLevel & TRACE_LEVEL_WARNING)
              {
                USER_Trace((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance),
                           TRACE_LEVEL_WARNING,
                          "DEV_DoChannelInit(): Channel did not enter READY state during timeout!");
              }
            }
          }
        }
      }
    }

    OS_ReleaseMutex(ptChannel->pvInitMutex);
  }

  return lRet;
}

/*****************************************************************************/
/*! Mark device to be in reset state and clear device internal structure for
*   reset preparation
*   \param ptDevInstance Device instance                                     */
/*****************************************************************************/
static void DEV_Reset_Prepare(PDEVICEINSTANCE ptDevInstance)
{
  uint32_t ulIdx = 0;

  /* Reset is now active and DSR will ignore all incoming interrupts from now on */
  ptDevInstance->fResetActive = 1;

  /* Zero out all internal flags */
  OS_EnterLock(ptDevInstance->tSystemDevice.pvLock);
  ptDevInstance->tSystemDevice.usHostFlags = 0;
  ptDevInstance->tSystemDevice.usNetxFlags = 0;
  OS_LeaveLock(ptDevInstance->tSystemDevice.pvLock);

  for ( ulIdx = 0; ulIdx < ptDevInstance->ulCommChannelCount; ulIdx++)
  {
    OS_EnterLock(ptDevInstance->pptCommChannels[ulIdx]->pvLock);
    ptDevInstance->pptCommChannels[ulIdx]->usHostFlags      = 0;
    ptDevInstance->pptCommChannels[ulIdx]->usNetxFlags      = 0;
    ptDevInstance->pptCommChannels[ulIdx]->ulDeviceCOSFlags = 0;
    ptDevInstance->pptCommChannels[ulIdx]->ulHostCOSFlags   = 0;
    OS_LeaveLock(ptDevInstance->pptCommChannels[ulIdx]->pvLock);
  }
}

/*****************************************************************************/
/*! After reset, re-read the device flags to continue communication
*   \param ptDevInstance Device instance                                     */
/*****************************************************************************/
static void DEV_Reset_Finish(PDEVICEINSTANCE ptDevInstance)
{
  uint32_t ulIdx = 0;

  /* Reset not active anymore */
  ptDevInstance->fResetActive = 0;

  /* Reset is finished, so we can now update our internal states */
  if(ptDevInstance->fIrqEnabled)
  {
    (void)cifXTKitISRHandler(ptDevInstance,1);
    cifXTKitDSRHandler(ptDevInstance);

  } else
  {
    /* Re-Read all handshake flags, as they will have reset */
    OS_EnterLock(ptDevInstance->tSystemDevice.pvLock);
    DEV_ReadHostFlags( &ptDevInstance->tSystemDevice, 0);
    DEV_ReadHandshakeFlags(&ptDevInstance->tSystemDevice, 1, 0);
    OS_LeaveLock(ptDevInstance->tSystemDevice.pvLock);

    for ( ulIdx = 0; ulIdx < ptDevInstance->ulCommChannelCount; ulIdx++)
    {
      OS_EnterLock(ptDevInstance->pptCommChannels[ulIdx]->pvLock);
      DEV_ReadHostFlags( ptDevInstance->pptCommChannels[ulIdx], 1);
      DEV_ReadHandshakeFlags(ptDevInstance->pptCommChannels[ulIdx], 0, 0);
      OS_LeaveLock(ptDevInstance->pptCommChannels[ulIdx]->pvLock);
    }
  }

}

/*****************************************************************************/
/*! Setup device reset and wait until firmware removes READY bit
*   \param ptDevInstance     Device instance
*   \param bHostFlagsChange  Host Flags to be set in SystemChannel
*   \param fWaitOnDevice     Wait on device state if != 0
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t DEV_Reset_Execute(PDEVICEINSTANCE ptDevInstance, uint8_t bHostFlagsChange, uint8_t fWaitOnDevice)
{
  PCHANNELINSTANCE          ptSysDevice  = &ptDevInstance->tSystemDevice;
  HIL_DPM_SYSTEM_CHANNEL_T* ptSysChannel = (HIL_DPM_SYSTEM_CHANNEL_T*)ptSysDevice->pbDPMChannelStart;
  int32_t                   lRet         = CIFX_NO_ERROR;
  uint8_t                   bHostFlags   = HWIF_READ8(ptDevInstance, ptSysDevice->ptHandshakeCell->t8Bit.bHostFlags);

  /* Lock flag access */
  OS_EnterLock(ptSysDevice->pvLock);

  /* Insert the reset cookie */
  HWIF_WRITE32(ptDevInstance, ptSysChannel->tSystemControl.ulSystemCommandCOS, HOST_TO_LE32(HIL_SYS_RESET_COOKIE));

  /* Activate the Reset */
  HWIF_WRITE8(ptDevInstance, ptSysDevice->ptHandshakeCell->t8Bit.bHostFlags, (bHostFlags | bHostFlagsChange));

  /* Leave flag access */
  OS_LeaveLock(ptSysDevice->pvLock);

  if( fWaitOnDevice)
  {
    /* Wait until card has recognized the reset */
    if( !DEV_WaitForNotReady_Poll( ptSysDevice, CIFX_TO_WAIT_HW_RESET_ACTIVE))
      lRet = CIFX_DEV_RESET_TIMEOUT;
  }

  return lRet;
}

/*****************************************************************************/
/*! Performs a system restart on a device and waits for card to get ready again
*   \param ptChannel Channel instance (ALWAYS the system channel)
*   \param ulTimeout Timeout to wait for device to become ready
*   \param ulParam   Reset parameter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_DoSystemStart(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam )
{
  PDEVICEINSTANCE           ptDevInstance   = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  PCHANNELINSTANCE          ptSysDevice     = &ptDevInstance->tSystemDevice;
  HIL_DPM_SYSTEM_CHANNEL_T* ptSysChannel    = (HIL_DPM_SYSTEM_CHANNEL_T*)ptSysDevice->pbDPMChannelStart;
  uint32_t                  ulSystemStatus  = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemStatus));
  int32_t                   lRet            = CIFX_NO_ERROR;

  /* Card was running before, so wait for running flag to vanish */
  if(!DEV_IsReady(ptSysDevice))
    return CIFX_DEV_NOT_READY;

  if(!OS_WaitMutex(ptDevInstance->tSystemDevice.pvInitMutex, CIFX_TO_WAIT_COS_CMD))
  {
    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_ERROR,
                "DEV_DoSystemStart(): Error locking access to device!");
    }

    lRet = CIFX_DRV_INIT_STATE_ERROR;
  }else
  {
    /* Set SystemControl value in the DPM to signal RESET "ONLY SYSTEM CHANNEL" */
    /* this is a coldstart and does not use any parameters                      */
    uint32_t ulSystemControl = HIL_SYS_CONTROL_RESET_MODE_COLDSTART | (ulParam & HIL_SYS_CONTROL_RESET_PARAM_FLAG_MASK);
    HWIF_WRITE32(ptDevInstance, ptSysChannel->tSystemControl.ulSystemControl, HOST_TO_LE32((ulSystemControl)));

    if ( HIL_SYS_STATUS_IDPM == (HIL_SYS_STATUS_IDPM & ulSystemStatus) &&
         HIL_SYS_STATUS_APP  == (HIL_SYS_STATUS_APP  & ulSystemStatus) )
    {
      /* If we're running with an enabled IDPM and APP CPU, no reset
       * will be executed. We just signal the reset state to the COM CPU by using the HSF_RESET flag */

      /* Activate the Reset */
      /* ATTENTION: Do not wait on the device, because the reset will be handled by the COM-CPU, */
      /*            and tehrefore the APP CPU has to remove its MCP_CPU_ID_APP0 bit (see netX MCP register). */
      lRet = DEV_Reset_Execute(ptDevInstance, HSF_RESET, 0 );

    } else
    {
      /* Prepare reset */
      DEV_Reset_Prepare(ptDevInstance);

      /* Perform the Reset */
      lRet = DEV_Reset_Execute(ptDevInstance, HSF_RESET, 1);

      if((CIFX_NO_ERROR != lRet) && (g_ulTraceLevel & TRACE_LEVEL_ERROR))
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_ERROR,
                  "DEV_DoSystemStart(): Error waiting for device to leave READY state!");
      }

      /* Now wait for the card to come back */
      if(CIFX_NO_ERROR == lRet)
      {
        /* Prohibit access to possibly uninitialized PCI memory during reset of netX4000 based PCI devices.
           Timeout of 1s was communicated to be the upper boundary. */
        if( (ptDevInstance->fPCICard) &&
            (( eCHIP_TYPE_NETX4000 == ptDevInstance->eChipType) ||
             ( eCHIP_TYPE_NETX4100 == ptDevInstance->eChipType)  )  )
          OS_Sleep(1000);

        /* now wait for card to become READY */
        if( !DEV_WaitForReady_Poll( ptSysDevice, ( 0 == ulTimeout) ? CIFX_TO_WAIT_HW : ulTimeout) )
        {
          lRet = CIFX_DEV_NOT_READY;

          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "DEV_DoSystemStart(): Error waiting for device to become ready!");
          }
        }

        /* Re-read device handshake flags */
        DEV_Reset_Finish(ptDevInstance);
      }

      /* it is not possible to distinguish between success and failure since do not know the correct state after reset */
      /* so write meaningful dpm content into log file, to let the user verify current system state                    */
      if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
      {
        char szCookie[5];

        HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);
        /* split messages for better readability */
        USER_Trace(ptDevInstance, TRACE_LEVEL_DEBUG, "DEV_DoSystemStart(): (system status after reset)");
        USER_Trace(ptDevInstance, TRACE_LEVEL_DEBUG, " -DPM-Cookie    : '%02X','%02X','%02X','%02X'",
                   szCookie[0],
                   szCookie[1],
                   szCookie[2],
                   szCookie[3]);
        USER_Trace(ptDevInstance, TRACE_LEVEL_DEBUG, " -System Status : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemStatus)) );
        USER_Trace(ptDevInstance, TRACE_LEVEL_DEBUG, " -System Error  : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemError)) );
        USER_Trace(ptDevInstance, TRACE_LEVEL_DEBUG, " -Boot Error    : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulBootError)) );
      }
    }

    OS_ReleaseMutex(ptDevInstance->tSystemDevice.pvInitMutex);
  }

  return lRet;
}

/*****************************************************************************/
/*! Performs a system bootstart on a device and wait for card to get ready again
*   \param ptChannel Channel instance (ALWAYS the system channel)
*   \param ulTimeout Timeout to wait for device to become ready
*   \param ulParam   Reset parameter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_DoSystemBootstart(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam)
{
  PDEVICEINSTANCE           ptDevInstance  = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  PCHANNELINSTANCE          ptSysDevice    = &ptDevInstance->tSystemDevice;
  HIL_DPM_SYSTEM_CHANNEL_T* ptSysChannel   = (HIL_DPM_SYSTEM_CHANNEL_T*)ptSysDevice->pbDPMChannelStart;
  uint32_t                  ulSystemStatus = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemStatus));
  int32_t                   lRet           = CIFX_NO_ERROR;

  /* Card was running before, so wait for running flag to vanish */
  if(!DEV_IsReady(ptSysDevice))
    return CIFX_DEV_NOT_READY;

  if(!OS_WaitMutex(ptDevInstance->tSystemDevice.pvInitMutex, CIFX_TO_WAIT_COS_CMD))
  {
    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_ERROR,
                "DEV_DoSystemBootstart(): Error locking access to device!");
    }

    lRet = CIFX_DRV_INIT_STATE_ERROR;
  } else
  {
    /* Signal BOOTSTART RESET "ONLY SYSTEM CHANNEL" */
    /* Write the reset mode to DPM */
    uint32_t ulSystemControl = HIL_SYS_CONTROL_RESET_MODE_BOOTSTART | (ulParam & HIL_SYS_CONTROL_RESET_PARAM_FLAG_MASK);
    HWIF_WRITE32(ptDevInstance, ptSysChannel->tSystemControl.ulSystemControl, HOST_TO_LE32((ulSystemControl)));

    if ( HIL_SYS_STATUS_IDPM == (HIL_SYS_STATUS_IDPM & ulSystemStatus) &&
         HIL_SYS_STATUS_APP  == (HIL_SYS_STATUS_APP  & ulSystemStatus) )
    {
      /* If we're running with an enabled IDPM and APP CPU, no reset
       * will be executed. We just signal the reset state to the COM CPU by using the HSF_RESET flag */

      /* Activate the Reset (including BOOTSTART bit) */
      /* ATTENTION: Do not wait on the device, because the reset will be handled by the COM-CPU, */
      /*            and tehrefore the APP CPU has to remove its MCP_CPU_ID_APP0 bit (see netX MCP register). */
      lRet = DEV_Reset_Execute(ptDevInstance, (uint8_t)(HSF_RESET | HSF_BOOTSTART), 0);

    } else
    {
      /* Prepare reset */
      DEV_Reset_Prepare(ptDevInstance);

      /* Perform the Reset (including BOOTSTART bit) */
      lRet = DEV_Reset_Execute(ptDevInstance, (uint8_t)(HSF_RESET | HSF_BOOTSTART), 1);

      if((CIFX_NO_ERROR != lRet) && (g_ulTraceLevel & TRACE_LEVEL_ERROR))
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_ERROR,
                  "DEV_DoSystemBootstart(): Error waiting for device to leave READY state!");
      }

      /* Now wait for the card to come back */
      if(CIFX_NO_ERROR == lRet)
      {
        /* Prohibit access to possibly uninitialized PCI memory during reset of netX4000 based PCI devices.
           Timeout of 1s was communicated to be the upper boundary. */
        if( (ptDevInstance->fPCICard) &&
            (( eCHIP_TYPE_NETX4000 == ptDevInstance->eChipType) ||
             ( eCHIP_TYPE_NETX4100 == ptDevInstance->eChipType)  )  )
          OS_Sleep(1000);

        /* now wait for card to become READY */
        if( !DEV_WaitForReady_Poll( ptSysDevice, ( 0 == ulTimeout) ? CIFX_TO_WAIT_HW : ulTimeout) )
        {
          lRet = CIFX_DEV_NOT_READY;

          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "DEV_DoSystemBootstart(): Error waiting for device to become ready!");
          }
        } else
        {
          /* Check if the Bootloader is running */
          char     szCookie[5] = {0};

          /* Read the DPM cookie */
          HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);

          /* on DPM cards we need to check the for a valid cookie */
          if (0 != OS_Strcmp( szCookie, CIFX_DPMSIGNATURE_BSL_STR))
          {
            /* Failed to set the device into boot mode */
            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        TRACE_LEVEL_ERROR,
                        "DEV_DoSystemBootstart(): Error setting card into boot mode!");
            }

            lRet = CIFX_DEV_FUNCTION_FAILED;
          }
        }

        /* Re-read device handshake flags */
        DEV_Reset_Finish(ptDevInstance);
      }
    }
    OS_ReleaseMutex(ptDevInstance->tSystemDevice.pvInitMutex);
  }

  return lRet;
}

/*****************************************************************************/
/*! Performs update start on a device and waits for card to get ready again
*   \param ptChannel Channel instance (ALWAYS the system channel)
*   \param ulTimeout Timeout to wait for device to become ready
*   \param ulParam   Reset parameter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_DoUpdateStart(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam)
{
  PDEVICEINSTANCE           ptDevInstance  = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  PCHANNELINSTANCE          ptSysDevice    = &ptDevInstance->tSystemDevice;
  HIL_DPM_SYSTEM_CHANNEL_T* ptSysChannel   = (HIL_DPM_SYSTEM_CHANNEL_T*)ptSysDevice->pbDPMChannelStart;
  uint32_t                  ulSystemStatus = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemStatus));
  int32_t                   lRet           = CIFX_NO_ERROR;

  /* Card was running before, so wait for running flag to vanish */
  if(!DEV_IsReady(ptSysDevice))
    return CIFX_DEV_NOT_READY;

  if(!OS_WaitMutex(ptDevInstance->tSystemDevice.pvInitMutex, CIFX_TO_WAIT_COS_CMD))
  {
    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_ERROR,
                "DEV_DoUpdateStart(): Error locking access to device!");
    }

    lRet = CIFX_DRV_INIT_STATE_ERROR;
  }else
  {
    /* Signal UPDATESTART RESET "ONLY SYSTEM CHANNEL" */
    /* Write the reset mode to DPM */
    uint32_t ulSystemControl = HIL_SYS_CONTROL_RESET_MODE_UPDATESTART | (ulParam & HIL_SYS_CONTROL_RESET_PARAM_FLAG_MASK);
    HWIF_WRITE32(ptDevInstance, ptSysChannel->tSystemControl.ulSystemControl, HOST_TO_LE32((ulSystemControl)));

    if ( HIL_SYS_STATUS_IDPM == (HIL_SYS_STATUS_IDPM & ulSystemStatus) &&
         HIL_SYS_STATUS_APP  == (HIL_SYS_STATUS_APP  & ulSystemStatus) )
    {
      /* If we're running with an enabled IDPM and APP CPU, no reset
       * will be executed. We just signal the reset state to the COM CPU by using the HSF_RESET flag */

      /* Activate the Reset */
      /* ATTENTION: Do not wait on the device, because the reset will be handled by the COM-CPU, */
      /*            and tehrefore the APP CPU has to remove its MCP_CPU_ID_APP0 bit (see netX MCP register). */
      lRet = DEV_Reset_Execute(ptDevInstance, HSF_RESET, 0);

    } else
    {
      char szCookie[5] = {0};

      /* Prepare reset */
      DEV_Reset_Prepare(ptDevInstance);

      /* Perform the Reset */
      lRet = DEV_Reset_Execute(ptDevInstance, HSF_RESET, 1);

      if(((CIFX_NO_ERROR != lRet)) && (g_ulTraceLevel & TRACE_LEVEL_ERROR))
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_ERROR,
                  "DEV_DoUpdateStart(): Error waiting for device to leave READY state!");
      }

      /* Now wait for the card to come back */
      if(CIFX_NO_ERROR == lRet)
      {
        /* Prohibit access to possibly uninitialized PCI memory during reset of netX4000 based PCI devices.
           Timeout of 1s was communicated to be the upper boundary. */
        if( (ptDevInstance->fPCICard) &&
            (( eCHIP_TYPE_NETX4000 == ptDevInstance->eChipType) ||
             ( eCHIP_TYPE_NETX4100 == ptDevInstance->eChipType)  )  )
          OS_Sleep(1000);

        /* now wait for card to become READY */
        if( !DEV_WaitForReady_Poll( ptSysDevice, CIFX_TO_WAIT_HW ) )
        {
          lRet = CIFX_DEV_NOT_READY;

          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "DEV_DoUpdateStart(): Error waiting for device to become ready!");
          }
        } else
        {
          /* Check if MFW is running */
          HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);

          if (0 != OS_Strcmp( szCookie, CIFX_DPMSIGNATURE_BSL_STR))
          {
            /* Failed to set the device into boot mode */
            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        TRACE_LEVEL_ERROR,
                        "DEV_DoUpdateStart(): Error setting card into update mode!");
            }

            lRet = CIFX_DEV_FUNCTION_FAILED;
          }else
          {
            /* This is an updatestart, expected one additional reset to be performed by MFW */
            if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
            {
              USER_Trace(ptDevInstance,
                        TRACE_LEVEL_DEBUG,
                        "DEV_DoUpdateStart(): Waiting for update being applied.");
            }

            /* Wait until card has recognized the reset back to firmware */
            if( !DEV_WaitForNotReady_Poll( ptSysDevice, ( 0 == ulTimeout) ? CIFX_TO_FIRMWARE_UPDATE : ulTimeout))
            {
              lRet = CIFX_DEV_RESET_TIMEOUT;

              if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
              {
                USER_Trace(ptDevInstance,
                          TRACE_LEVEL_ERROR,
                          "DEV_DoUpdateStart(): Error waiting for device to leave READY state during update!");
              }
              /* 2nd reset not triggered in time. Possible causes:
                 - timeout too short
                 - hardware doesn't support updatestart */
            }

            /* Only continue if update timeout was sufficient */
            if(CIFX_NO_ERROR == lRet)
            {
              /* Prohibit access to possibly uninitialized PCI memory during reset of netX4000 based PCI devices.
                 Timeout of 1s was communicated to be the upper boundary. */
              if( (ptDevInstance->fPCICard) &&
                  (( eCHIP_TYPE_NETX4000 == ptDevInstance->eChipType) ||
                   ( eCHIP_TYPE_NETX4100 == ptDevInstance->eChipType)  )  )
                OS_Sleep(1000);

              /* now wait for card to become READY again */
              if( !DEV_WaitForReady_Poll( ptSysDevice, CIFX_TO_FIRMWARE_START) )
              {
                lRet = CIFX_DEV_NOT_READY;

                if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
                {
                  USER_Trace(ptDevInstance,
                            TRACE_LEVEL_ERROR,
                            "DEV_DoUpdateStart(): Error waiting for device to become ready!");
                }
              }
            }
          }
        }

        /* Re-read device handshake flags */
        DEV_Reset_Finish(ptDevInstance);
      }

      /* Log the current DPM state */
      if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
      {
        HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);
        /* split messages for better readability */
        USER_Trace(ptDevInstance, TRACE_LEVEL_DEBUG, "DEV_DoUpdateStart(): (system status after reset)");
        USER_Trace(ptDevInstance, TRACE_LEVEL_DEBUG, " -DPM-Cookie    : '%02X','%02X','%02X','%02X'",
                   szCookie[0],
                   szCookie[1],
                   szCookie[2],
                   szCookie[3]);
        USER_Trace(ptDevInstance, TRACE_LEVEL_DEBUG, " -System Status : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemStatus)) );
        USER_Trace(ptDevInstance, TRACE_LEVEL_DEBUG, " -System Error  : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemError)) );
        USER_Trace(ptDevInstance, TRACE_LEVEL_DEBUG, " -Boot Error    : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulBootError)) );
      }
    }

    OS_ReleaseMutex(ptDevInstance->tSystemDevice.pvInitMutex);
  }

  return lRet;
}

/*****************************************************************************/
/*! Do a handshake for the ulApplicationCOS bits in DPM. This function will
*   wait for access to ulApplicationCOS (via HSF_HOST_COS_CMD), toggle bits and
*   wait for firmware to acknowledge COS. After handshaking is completed this
*   function will clear bits in internal HostCOS flags defined in PostClearCOSMask
*   \param ptChannel          Channel instance
*   \param ulSetCOSMask       Host COS Bits to set
*   \param ulClearCOSMask     Host COS Bits to clear
*   \param ulPostClearCOSMask Host COS Bits to clear after handshaking has completed
*   \param lSignallingError   Error to return if signalling was not acknowledged
*   \param ulTimeout          Timeout to wait handshake complete
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_DoHostCOSChange(PCHANNELINSTANCE ptChannel,
                         uint32_t ulSetCOSMask,       uint32_t ulClearCOSMask,
                         uint32_t ulPostClearCOSMask, int32_t lSignallingError,
                         uint32_t ulTimeout)
{
  int32_t lRet = CIFX_NO_ERROR;

  /* Check if we are able to send a COS command */
  if( !DEV_WaitForBitState( ptChannel, HCF_HOST_COS_CMD_BIT_NO, HIL_FLAGS_EQUAL, ulTimeout))
  {
    /* Wait for access to COS bits failed */

    if(0 == ulTimeout)
    {
      /* User did not want to wait, so remember his flags, and update them with
         next COS handshake. PostClearMask will be cleared by DSR or DEV_CheckCOSFlags() */
      OS_EnterLock(ptChannel->pvLock);

      ptChannel->ulHostCOSFlags |= ulSetCOSMask;
      ptChannel->ulHostCOSFlags &= ~ulClearCOSMask;

      OS_LeaveLock(ptChannel->pvLock);

      lRet = CIFX_NO_ERROR;

    } else
    {
      lRet = CIFX_DEV_FUNCTION_FAILED;
    }

  } else
  {
    /* Lock flag access */
    OS_EnterLock(ptChannel->pvLock);

    ptChannel->ulHostCOSFlags |= ulSetCOSMask;
    ptChannel->ulHostCOSFlags &= ~ulClearCOSMask;

    HWIF_WRITE32(ptChannel->pvDeviceInstance, ptChannel->ptControlBlock->ulApplicationCOS, HOST_TO_LE32(ptChannel->ulHostCOSFlags));
    ptChannel->ulHostCOSFlagsSaved = ptChannel->ulHostCOSFlags;

    DEV_ToggleBit(ptChannel, HCF_HOST_COS_CMD);

    /* Reset the enable bit in the local flags */
    ptChannel->ulHostCOSFlags &= ~ulPostClearCOSMask;

    /* Unlock flag access */
    OS_LeaveLock(ptChannel->pvLock);

    /* Wait until card has acknowledged the COS flag */
    if( !DEV_WaitForBitState( ptChannel, HCF_HOST_COS_CMD_BIT_NO, HIL_FLAGS_EQUAL, ulTimeout))
    {
      /* Wait for acknowledge from FW to COS handshake failed */
      if(0 == ulTimeout)
      {
        /* User did not want to wait, so tell him everything is OK */
        lRet = CIFX_NO_ERROR;
      } else
      {
        lRet = lSignallingError;
      }
    } else
    {
      lRet = CIFX_NO_ERROR;
    }
  }

  return lRet;
}


#ifdef CIFX_TOOLKIT_DMA
/*****************************************************************************/
/*! Setup DMA buffers
*   \param ptChannel   Channel instance
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_SetupDMABuffers( PCHANNELINSTANCE ptChannel)
{
  int32_t                   lRet            = CIFX_NO_ERROR;
  uint32_t                  ulChannelNumber = 0;
  uint32_t                  ulDMAChIdx      = 0;
  uint32_t                  ulBaseBuffer    = 0;
  NETX_DMA_CHANNEL_CONFIG*  pDMACtrl_1      = NULL;
  NETX_DMA_CHANNEL_CONFIG*  pDMACtrl_2      = NULL;
  CIFX_DMABUFFER_T*         ptDMABuffer_1   = NULL;
  CIFX_DMABUFFER_T*         ptDMABuffer_2   = NULL;

  /*
      netX Buffer Layout
      ----------------------------------  0
      | MemBaseBuffer                    |
      | used for (Host-->netX)           |
      |----------------------------------| BufferSize
      | MemBaseBuffer + BufferSize       |
      | used for (netX-->Host)           |
      ----------------------------------  BufferSize * 2

      DMA channel layout:
        n         = Communication channel number
        DMACh n   = Input data
        DMACh n+1 = Output data
  */

  /* Get the device instance from the channel instance */
  PDEVICEINSTANCE ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  /* Get the DMA control registers */
  ulChannelNumber = ptChannel->ulChannelNumber ;
  ulDMAChIdx      = ulChannelNumber * 2; /* 2 DMA channels per communication channel */

  /* Get the corresponding netX DMA control register */
  pDMACtrl_1      = &ptDevInstance->ptGlobalRegisters->atDmaCtrl[ulDMAChIdx + eDMA_INPUT_BUFFER_IDX]; /* Input channel */
  pDMACtrl_2      = &ptDevInstance->ptGlobalRegisters->atDmaCtrl[ulDMAChIdx + eDMA_OUTPUT_BUFFER_IDX];/* Output channel */

  /* Get the user created DMA buffers */
  ptDMABuffer_1   = &ptDevInstance->atDmaBuffers[ulDMAChIdx + eDMA_INPUT_BUFFER_IDX];                 /* Input buffer */
  ptDMABuffer_2   = &ptDevInstance->atDmaBuffers[ulDMAChIdx + eDMA_OUTPUT_BUFFER_IDX];                /* Output buffer */

  /*------------------------------------*/
  /* Setup INPUT DMA channel and buffer */
  /*------------------------------------*/
  /* Insert the physical buffer address */
  /* Channel N is used as direction netX->Host, so we need to substract "BufferSize" from the pointer to get the DMA at proper location */
  /* Switch to ONE buffer operation!!!!! */
  ulBaseBuffer = ptDMABuffer_1->ulPhysicalAddress - ptDMABuffer_1->ulSize;
  pDMACtrl_1->aulMemBaseBuffer[0] = HOST_TO_LE32(ulBaseBuffer);
  pDMACtrl_1->aulMemBaseBuffer[1] = HOST_TO_LE32(ulBaseBuffer);
  pDMACtrl_1->aulMemBaseBuffer[2] = HOST_TO_LE32(ulBaseBuffer);
  pDMACtrl_1->ulBufCtrl           = HOST_TO_LE32((ptDMABuffer_1->ulSize / 256) << 24); /* Setup buffer size */


  /*-------------------------------------*/
  /* Setup OUTPUT DMA channel and buffer */
  /*-------------------------------------*/
  /* Insert the physical buffer address */
  /* Channel N+1 is used as direction Host->netX so we can use the given pointer to get the DMA buffer*/
  /* Switch to ONE buffer operation!!!!! */
  ulBaseBuffer = ptDMABuffer_2->ulPhysicalAddress;
  pDMACtrl_2->aulMemBaseBuffer[0] = HOST_TO_LE32(ulBaseBuffer);
  pDMACtrl_2->aulMemBaseBuffer[1] = HOST_TO_LE32(ulBaseBuffer);
  pDMACtrl_2->aulMemBaseBuffer[2] = HOST_TO_LE32(ulBaseBuffer);
  pDMACtrl_2->ulBufCtrl           = HOST_TO_LE32((ptDMABuffer_2->ulSize / 256) << 24); /* Setup buffer size */

  return lRet;
}

/*****************************************************************************/
/*! Handle the application DMA state COS flag
*   \param ptChannel        Channel instance
*   \param ulCmd            new state to set (CIFX_DMA_STATE_ON / CIFX_DMA_STATE_OFF)
*   \param pulState         Buffer to store actual state
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_DMAState(PCHANNELINSTANCE ptChannel, uint32_t ulCmd, uint32_t* pulState)
{
  int32_t lRet = CIFX_NO_ERROR;

  if( NULL == pulState)
    return CIFX_INVALID_POINTER;

  /* Check if device is READY */
  if(!DEV_IsReady(ptChannel))
    return CIFX_DEV_NOT_READY;

  /* Read actual DMA state */
  *pulState = (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->ulCommunicationCOS)) & HIL_COMM_COS_DMA) ?
               CIFX_DMA_STATE_ON : CIFX_DMA_STATE_OFF;

  switch (ulCmd)
  {
    case CIFX_DMA_STATE_ON:
    {
      /* Check if the DMA is already ON */
      if(CIFX_DMA_STATE_ON != *pulState)
      {
        /* Setup DMA buffers always, to make sure HIL_COMM_COS_DMA state is handled correctly */
        (void)DEV_SetupDMABuffers(ptChannel);

        /* DMA is OFF, signal new DMA state */
        lRet = DEV_DoHostCOSChange(ptChannel,
                                   HIL_APP_COS_DMA | HIL_APP_COS_DMA_ENABLE,  /* set mask        */
                                   0,                                         /* clear mask      */
                                   HIL_APP_COS_DMA_ENABLE,                    /* post clear mask */
                                   CIFX_DEV_DMA_STATE_ON_TIMEOUT,
                                   CIFX_TO_WAIT_COS_ACK);                     /* Alwas wait for the card ACK */

        /* Read actual state */
        *pulState = (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->ulCommunicationCOS)) & HIL_COMM_COS_DMA) ?
                     CIFX_DMA_STATE_ON : CIFX_DMA_STATE_OFF;
      }
    }
    break;

    case CIFX_DMA_STATE_OFF:
    {
      /* Check if the DMA is already OFF */
      if(CIFX_DMA_STATE_OFF != *pulState)
      {
        /* DMA is ON, signal new DMA state */
        lRet = DEV_DoHostCOSChange(ptChannel,
                                   HIL_APP_COS_DMA_ENABLE,            /* set mask        */
                                   HIL_APP_COS_DMA,                   /* clear mask      */
                                   HIL_APP_COS_DMA_ENABLE,            /* post clear mask */
                                   CIFX_DEV_DMA_STATE_OFF_TIMEOUT,
                                   CIFX_TO_WAIT_COS_ACK);             /* Alwas wait for the card ACK */

        /* Read actual state */
        *pulState = (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommonStatusBlock->ulCommunicationCOS)) & HIL_COMM_COS_DMA) ?
                     CIFX_BUS_STATE_ON : CIFX_BUS_STATE_OFF;
      }
    }
    break;

    case CIFX_DMA_STATE_GETSTATE:
    break;

    default:
      /* Unknown command */
      lRet = CIFX_INVALID_COMMAND;
    break;

  }

  return lRet;
}

/*****************************************************************************/
/*! Get actual DMA input buffer
*   !!!! This function will be needed if "Buffer switch" is supported !!!
*   !!!! Currently we only using "One Buffer" operation in the Toolkit!!!
*   \param ptChannel          Channel instance
*   \param ulDirection        Direction value
*   \return actual DMA buffer number                                         */
/*****************************************************************************/
uint32_t GetActualDMABuffer( PCHANNELINSTANCE ptChannel, uint32_t ulDirection)
{
  uint32_t        ulTemp        = 0;
  uint32_t        ulDMAChIdx    = (ptChannel->ulChannelNumber * 2) + ulDirection; /* This is the input buffer */
  PDEVICEINSTANCE ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  /* Setup pointer to global netX register block */
  NETX_DMA_CHANNEL_CONFIG* pDMACtrl = &ptDevInstance->ptGlobalRegisters->atDmaCtrl[ulDMAChIdx];

  /* Acknowledge the buffer */
  pDMACtrl->ulBufCtrl |= HOST_TO_LE32((1 << 19));
  ulTemp = (LE32_TO_HOST(pDMACtrl->ulBufCtrl)) & (7 << 16);

  ulTemp = (ulTemp >> 17) & 0x00000003;

  return ulTemp;
}
#endif

#ifdef CIFX_TOOLKIT_HWIF
/*****************************************************************************/
/*! Wrapper function to read byte from DPM
*   \param ptDev Device instance
*   \param pvSrc DPM address to read from
*   \return Byte read from DPM                                               */
/*****************************************************************************/
uint8_t HwIfRead8(PDEVICEINSTANCE ptDev, void* pvSrc)
{
  uint8_t bData = 0;
  (void)ptDev->pfnHwIfRead(1, ptDev, pvSrc, &bData, sizeof(bData));
  return bData;
}

/*****************************************************************************/
/*! Wrapper function to read word from DPM
*   \param ptDev Device instance
*   \param pvSrc DPM address to read from
*   \return Word read from DPM                                               */
/*****************************************************************************/
uint16_t HwIfRead16(PDEVICEINSTANCE ptDev, void* pvSrc)
{
  uint16_t usData = 0;
  (void)ptDev->pfnHwIfRead(1, ptDev, pvSrc, &usData, sizeof(usData));
  return usData;
}

/*****************************************************************************/
/*! Wrapper function to read double word from DPM
*   \param ptDev Device instance
*   \param pvSrc DPM address to read from
*   \return Double word read from DPM                                        */
/*****************************************************************************/
uint32_t HwIfRead32(PDEVICEINSTANCE ptDev, void* pvSrc)
{
  uint32_t ulData = 0;
  (void)ptDev->pfnHwIfRead(1, ptDev, pvSrc, &ulData, sizeof(ulData));
  return ulData;
}

#endif /* CIFX_TOOLKIT_HWIF */
/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/
