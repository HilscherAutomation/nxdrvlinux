/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXHWFunctions.c 15171 2025-08-05 08:18:45Z AMinor $:

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

#include "cifXFunctionList.h"
#include "cifXHWFunctions.h"
#include "cifXErrors.h"
#include "cifXEndianess.h"
#include "Hilcrc32.h"
#include "Hilmd5.h"
#include "USER_Dependent.h"

#include "Hil_Packet.h"
#include "Hil_ApplicationCmd.h"
#include "Hil_FileHeaderV3.h"
#include "Hil_SystemCmd.h"
#include "Hil_Results.h"

/*****************************************************************************/
/*!  \addtogroup CIFX_TK_HARDWARE Hardware Access
*    \{                                                                      */
/*****************************************************************************/

/*****************************************************************************/
/*! Get expected handshake bit state from IOArea
*   \param ptChannel    Channel instance
*   \param ptIOInstance Pointer to IOInstance
*   \param fOutput      !=0 for output areas
*   \return Expected handshake bit state                                     */
/*****************************************************************************/
CIFX_STATIC uint8_t DEV_GetIOBitstate(PCHANNELINSTANCE ptChannel, PIOINSTANCE ptIOInstance, int fOutput)
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
CIFX_STATIC void DEV_ToggleBit(PCHANNELINSTANCE ptChannel, uint32_t ulBitMask)
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
/*! Toggles the given sync bit
*   \param ptDevInstance  Device instance
*   \param ulBitMask      Bitmask to eXOR into command bits                  */
/*****************************************************************************/
CIFX_STATIC void DEV_ToggleSyncBit(PDEVICEINSTANCE ptDevInstance, uint32_t ulBitMask)
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
CIFX_STATIC void DEV_ReadHostFlags(PCHANNELINSTANCE ptChannel, int fReadHostCOS)
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
CIFX_STATIC void DEV_ReadHandshakeFlags(PCHANNELINSTANCE ptChannel, int fReadSyncFlags, int fLockNeeded)
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
CIFX_STATIC int DEV_WaitForSyncState(PCHANNELINSTANCE ptChannel, uint8_t bState, uint32_t ulTimeout)
{
  if( ((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    return DEV_WaitForSyncState_Irq(ptChannel, bState, ulTimeout);
  else
    return DEV_WaitForSyncState_Poll(ptChannel, bState, ulTimeout);
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
CIFX_STATIC int DEV_WaitForBitState(PCHANNELINSTANCE ptChannel, uint32_t ulBitNumber, uint8_t bState, uint32_t ulTimeout)
{
  if( ((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    return DEV_WaitForBitState_Irq(ptChannel, ulBitNumber, bState, ulTimeout);
  else
    return DEV_WaitForBitState_Poll(ptChannel, ulBitNumber, bState, ulTimeout);
}

/*****************************************************************************/
/*! Checks if the channel is running
*   \param ptChannel Channel instance to check
*   \return 1 if channel is ready and running                                */
/*****************************************************************************/
CIFX_STATIC int DEV_IsRunning(PCHANNELINSTANCE ptChannel)
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
/*! Checks if the channel is ready
*   \param ptChannel Channel instance to check
*   \return 1 if channel is ready                                            */
/*****************************************************************************/
CIFX_STATIC int DEV_IsReady(PCHANNELINSTANCE ptChannel)
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
/*! Wait for NOT READY in poll mode
*   \param ptChannel Channel instance to check
*   \param ulTimeout Wait time
*   \return 1 if channel is NOT ready                                            */
/*****************************************************************************/
CIFX_STATIC int DEV_WaitForNotReady_Poll(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout)
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
CIFX_STATIC int DEV_WaitForReady_Poll(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout)
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
CIFX_STATIC int DEV_WaitForNotRunning_Poll(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout)
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
CIFX_STATIC int DEV_WaitForRunning_Poll(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout)
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
CIFX_STATIC void DEV_WriteHandshakeFlags(PCHANNELINSTANCE ptChannel)
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
/*! Sends a Packet to the device/channel
*   \param ptChannel    Channel instance to send a packet
*   \param ptSendPkt    Packet to send
*   \param ulTimeout    Maximum time in ms to wait for an empty mailbox
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_PutPacket(PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout)
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
CIFX_STATIC int32_t DEV_GetPacket( PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptRecvPkt, uint32_t ulRecvBufferSize, uint32_t ulTimeout)
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
CIFX_STATIC int32_t DEV_TransferPacket( void*                  pvChannel,        CIFX_PACKET*  ptSendPkt, CIFX_PACKET* ptRecvPkt,
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
/*! Checks if the channel is communicating
*   \param ptChannel Channel instance to check
*   \param plError   CIFX_NO_ERROR on successful read
*   \return 1 if channel is communicating                                    */
/*****************************************************************************/
CIFX_STATIC int DEV_IsCommunicating(PCHANNELINSTANCE ptChannel, int32_t* plError)
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
CIFX_STATIC int32_t DEV_GetMBXState(PCHANNELINSTANCE ptChannel, uint32_t* pulRecvPktCnt, uint32_t* pulSendPktCnt)
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
CIFX_STATIC int32_t DEV_TriggerWatchdog(PCHANNELINSTANCE ptChannel, uint32_t ulTriggerCmd, uint32_t* pulTriggerValue)
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
/*! Read the application COS flag state
*   \param ptChannel        Channel instance
*   \param pulState         returned host state (CIFX_HOST_STATE_READY /
*                           CIFX_HOST_STATE_NOT_READY)
*   \return CIFX_NO_ERROR on success, or CIFX_DEV_NOT_READY                  */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_GetHostState(PCHANNELINSTANCE ptChannel, uint32_t* pulState)
{
  /* Don't return any state if card is not ready */
  if(!DEV_IsReady(ptChannel))
    return CIFX_DEV_NOT_READY;

  *pulState = (ptChannel->ulHostCOSFlags & HIL_APP_COS_APPLICATION_READY)? CIFX_HOST_STATE_READY : CIFX_HOST_STATE_NOT_READY;

  return CIFX_NO_ERROR;
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
CIFX_STATIC int32_t DEV_ReadWriteBlock(PCHANNELINSTANCE ptChannel, void* pvBlock, uint32_t ulOffset, uint32_t ulBlockLen, void* pvDest, uint32_t ulDestLen, uint32_t ulCmd, int fWriteAllowed)
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
CIFX_STATIC uint8_t DEV_GetHandshakeBitState(PCHANNELINSTANCE ptChannel, uint32_t ulBitMsk)
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
CIFX_STATIC void DEV_CheckCOSFlags(PDEVICEINSTANCE ptDevInstance)
{
  /* Note: We assume, we only get here in polling mode */
  uint32_t ulChannel;

  if(!OS_WaitMutex(ptDevInstance->tSystemDevice.pvInitMutex, 0))
  {
    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_DEBUG)
    {
      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_DEBUG,
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
        if(g_ulTraceLevel & CIFX_TRACE_LEVEL_DEBUG)
        {
          USER_Trace(ptDevInstance,
                    CIFX_TRACE_LEVEL_DEBUG,
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
    (void)CIFX_MAKE_TKIT_FUN(cifXTKitISRHandler)(ptDevInstance, 1);
    CIFX_MAKE_TKIT_FUN(cifXTKitDSRHandler)(ptDevInstance);
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
CIFX_STATIC int32_t DEV_DoSystemStart(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam )
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
    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_ERROR,
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

      if((CIFX_NO_ERROR != lRet) && (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR))
      {
        USER_Trace(ptDevInstance,
                  CIFX_TRACE_LEVEL_ERROR,
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

          if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      CIFX_TRACE_LEVEL_ERROR,
                      "DEV_DoSystemStart(): Error waiting for device to become ready!");
          }
        }

        /* Re-read device handshake flags */
        DEV_Reset_Finish(ptDevInstance);
      }

      /* it is not possible to distinguish between success and failure since do not know the correct state after reset */
      /* so write meaningful dpm content into log file, to let the user verify current system state                    */
      if(g_ulTraceLevel & CIFX_TRACE_LEVEL_DEBUG)
      {
        char szCookie[5];

        HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);
        /* split messages for better readability */
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, "DEV_DoSystemStart(): (system status after reset)");
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -DPM-Cookie    : '%02X','%02X','%02X','%02X'",
                   szCookie[0],
                   szCookie[1],
                   szCookie[2],
                   szCookie[3]);
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -System Status : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemStatus)) );
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -System Error  : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemError)) );
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -Boot Error    : 0x%X",
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
CIFX_STATIC int32_t DEV_DoSystemBootstart(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam)
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
    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_ERROR,
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

      if((CIFX_NO_ERROR != lRet) && (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR))
      {
        USER_Trace(ptDevInstance,
                  CIFX_TRACE_LEVEL_ERROR,
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

          if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      CIFX_TRACE_LEVEL_ERROR,
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
            if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        CIFX_TRACE_LEVEL_ERROR,
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
CIFX_STATIC int32_t DEV_DoUpdateStart(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam)
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
    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_ERROR,
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

      if(((CIFX_NO_ERROR != lRet)) && (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR))
      {
        USER_Trace(ptDevInstance,
                  CIFX_TRACE_LEVEL_ERROR,
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

          if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      CIFX_TRACE_LEVEL_ERROR,
                      "DEV_DoUpdateStart(): Error waiting for device to become ready!");
          }
        } else
        {
          /* Check if MFW is running */
          HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);

          if (0 != OS_Strcmp( szCookie, CIFX_DPMSIGNATURE_BSL_STR))
          {
            /* Failed to set the device into boot mode */
            if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        CIFX_TRACE_LEVEL_ERROR,
                        "DEV_DoUpdateStart(): Error setting card into update mode!");
            }

            lRet = CIFX_DEV_FUNCTION_FAILED;
          }else
          {
            /* This is an updatestart, expected one additional reset to be performed by MFW */
            if(g_ulTraceLevel & CIFX_TRACE_LEVEL_DEBUG)
            {
              USER_Trace(ptDevInstance,
                        CIFX_TRACE_LEVEL_DEBUG,
                        "DEV_DoUpdateStart(): Waiting for update being applied.");
            }

            /* Wait until card has recognized the reset back to firmware */
            if( !DEV_WaitForNotReady_Poll( ptSysDevice, ( 0 == ulTimeout) ? CIFX_TO_FIRMWARE_UPDATE : ulTimeout))
            {
              lRet = CIFX_DEV_RESET_TIMEOUT;

              if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
              {
                USER_Trace(ptDevInstance,
                          CIFX_TRACE_LEVEL_ERROR,
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

                if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
                {
                  USER_Trace(ptDevInstance,
                            CIFX_TRACE_LEVEL_ERROR,
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
      if(g_ulTraceLevel & CIFX_TRACE_LEVEL_DEBUG)
      {
        HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);
        /* split messages for better readability */
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, "DEV_DoUpdateStart(): (system status after reset)");
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -DPM-Cookie    : '%02X','%02X','%02X','%02X'",
                   szCookie[0],
                   szCookie[1],
                   szCookie[2],
                   szCookie[3]);
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -System Status : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemStatus)) );
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -System Error  : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemError)) );
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -Boot Error    : 0x%X",
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
CIFX_STATIC int32_t DEV_DoHostCOSChange(PCHANNELINSTANCE ptChannel,
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

/*****************************************************************************/
/*! Performs a channel initialization
*   \param ptChannel Channel instance
*   \param ulTimeout Timeout to wait for channel to become READY
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_DoChannelInit(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout)
{
  int32_t         lRet        = CIFX_NO_ERROR;
  PDEVICEINSTANCE ptDevInst   = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  int             fRunning    = DEV_IsRunning(ptChannel);

  if(!OS_WaitMutex(ptChannel->pvInitMutex, CIFX_TO_WAIT_COS_CMD))
  {
    /* This should only happen, if the DEV_CheckCOSFlags function is still busy checking
       for COS changed on this channel */
    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInst,
                CIFX_TRACE_LEVEL_ERROR,
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
          if(g_ulTraceLevel & CIFX_TRACE_LEVEL_DEBUG)
          {
            USER_Trace(ptDevInst,
                      CIFX_TRACE_LEVEL_DEBUG,
                      "DEV_DoChannelInit(): Firmware removed HIL_COMM_COS_RUN early! Skipping wait for NotRunning-State");
          }

        } else if( !DEV_WaitForNotRunning_Poll( ptChannel, CIFX_TO_WAIT_HW_RESET_ACTIVE))
        {
          lRet = CIFX_DEV_RESET_TIMEOUT;
          if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInst,
                      CIFX_TRACE_LEVEL_ERROR,
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
              if(g_ulTraceLevel & CIFX_TRACE_LEVEL_WARNING)
              {
                USER_Trace((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance),
                           CIFX_TRACE_LEVEL_WARNING,
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
/*! Set the application ready COS flag
*   \param ptChannel        Channel instance
*   \param ulNewState       new state to set (CIFX_HOST_STATE_READY /
*                           CIFX_HOST_STATE_NOT_READY)
*   \param ulTimeout        timeout to wait for communication to start/stop
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_SetHostState(PCHANNELINSTANCE ptChannel, uint32_t ulNewState, uint32_t ulTimeout)
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
/*! Handle the application BUS state COS flag
*   \param ptChannel        Channel instance
*   \param ulCmd            new state to set (CIFX_BUS_STATE_ON / CIFX_BUS_STATE_OFF)
*   \param pulState         Buffer to store actual state
*   \param ulTimeout        timeout to wait for communication to start/stop
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_BusState(PCHANNELINSTANCE ptChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout)
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
/*! Delete all existing files in a channel, from the file system.
*   \param ptChannel          Channel instance
*   \param ulChannel          Channel number
*   \param pfnTransferPacket  Function used for transferring packets
*   \param pfnRecvPacket      User callback for unsolicited receive packets
*   \param pvUser             User parameter passed on callback
*   \param szExceptFile       File extension to ignore while deleting files
*   \return always returns 1                                                 */
/*****************************************************************************/
CIFX_STATIC int DEV_RemoveChannelFiles(PCHANNELINSTANCE       ptChannel,
                                       uint32_t               ulChannel,
                                       PFN_TRANSFER_PACKET    pfnTransferPacket,
                                       PFN_RECV_PKT_CALLBACK  pfnRecvPacket,
                                       void*                  pvUser,
                                       char*                  szExceptFile)
{
  /* Try to find file with the extension *.nxm, *.nxf, *.mod and remove it */
  PDEVICEINSTANCE     ptDevInstance   = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  CIFX_DIRECTORYENTRY tDirectoryEntry;
  int32_t             lRet            = CIFX_NO_ERROR;
  int                 fFindFirst      = 1;

  /* Search for all firmware files. If one is found. delete it an start with find first again, */
  /* because we can't store a directory list in here */
  do
  {
    if ( fFindFirst)
    {
       OS_Memset(&tDirectoryEntry, 0, sizeof(tDirectoryEntry));

      /* Search first file */
      if ( !(CIFX_NO_ERROR == (lRet = CIFX_MAKE_CIFX_FUN(xSysdeviceFindFirstFile)( ptChannel, ulChannel, &tDirectoryEntry, pfnRecvPacket, pvUser))))
      {
        /* No more files, or error during find first */
        break;
      } else
      {
        /* Is this a valid file name */
        int iStrlen = OS_Strlen(tDirectoryEntry.szFilename);
        if( iStrlen >= CIFX_MIN_FILE_NAME_LENGTH)  /* At least x.abc */
        {
          if( !((NULL != szExceptFile)                                                          &&
                (4 == OS_Strlen(szExceptFile))                                                  &&
                (0 == OS_Strnicmp( szExceptFile, &tDirectoryEntry.szFilename[iStrlen - 4], 4)))   )
          {
            /* Delete file and continue with find first file again */
            (void)CIFX_MAKE_DEV_FUN(DEV_DeleteFile)( ptChannel, ulChannel, tDirectoryEntry.szFilename, pfnTransferPacket, pfnRecvPacket, pvUser);
          }
        } else
        {
          /* Not a valid file, search next file */
          fFindFirst = 0;
        }
      }
    } else
    {
      /* Search for more files */
      if ( !(CIFX_NO_ERROR == (lRet = CIFX_MAKE_CIFX_FUN(xSysdeviceFindNextFile)( ptChannel, ulChannel, &tDirectoryEntry, pfnRecvPacket, pvUser))))
      {
        /* No more files, or error during find next */
        break;
      } else
      {
        /* Is this a valid file name */
        int iStrlen = OS_Strlen(tDirectoryEntry.szFilename);
        if( iStrlen >= CIFX_MIN_FILE_NAME_LENGTH)  /* At least x.abc */
        {
          /* If firmware file, delete it, else search until all files checked */
          if( !((NULL != szExceptFile)                                            &&
                (4 == OS_Strlen(szExceptFile))                                    &&
                (0 == OS_Strnicmp( szExceptFile, &tDirectoryEntry.szFilename[iStrlen - 4], 4)))   )
          {
            /* Delete the file and start with find first again */
            (void)CIFX_MAKE_DEV_FUN(DEV_DeleteFile)( ptChannel, ulChannel, tDirectoryEntry.szFilename, pfnTransferPacket, pfnRecvPacket, pvUser);
            fFindFirst = 1;
          }
        }
      }
    }
  } while ( CIFX_NO_ERROR == lRet);

  return 1;
}

/*****************************************************************************/
/*! Check if we have a firmware file
*   \param pszFileName      Input file name
*   \return 1 on success                                                     */
/*****************************************************************************/
CIFX_STATIC int DEV_IsFWFile( char* pszFileName)
{
  /* Check if we have a .NXO, .NXF,.NXM or .MOD extension */
  int fRet    = 0;
  int iStrlen = OS_Strlen(pszFileName);

  if( iStrlen >= CIFX_MIN_FILE_NAME_LENGTH)  /* At least x.abc */
  {
    if ( (0 == OS_Strnicmp( HIL_FILE_EXTENSION_FIRMWARE,     &pszFileName[iStrlen - 4], 4) ) ||
         (0 == OS_Strnicmp( HIL_FILE_EXTENSION_NXM_FIRMWARE, &pszFileName[iStrlen - 4], 4) ) ||
         (0 == OS_Strnicmp( HIL_FILE_EXTENSION_OPTION,       &pszFileName[iStrlen - 4], 4) ) ||
         (0 == OS_Strnicmp( ".MOD", &pszFileName[iStrlen - 4], 4) )  )
    {
      fRet = 1;
    }
  }

  return fRet;
}

/*****************************************************************************/
/*! Check if we have a firmware (update) file for netX90 and netX4000
*   \param pszFileName      Input file name
*   \return 1 on success                                                     */
/*****************************************************************************/
CIFX_STATIC int DEV_IsFWFileNetX90or4000( char* pszFileName)
{
  /* Check if we have a .NXI or .NAI extension, or FWUPDATE.ZIP / FWUPDATE.NXS
     Note: NXE or NAE should be downloaded in update container.
  */
  int fRet    = 0;
  int iStrlen = OS_Strlen(pszFileName);

  if( iStrlen >= CIFX_MIN_FILE_NAME_LENGTH)  /* At least x.abc */
  {
    if ( (0 == OS_Strnicmp( HIL_FILE_EXTENSION_NXI_FIRMWARE, &pszFileName[iStrlen - 4], 4) ) ||
         (0 == OS_Strnicmp( HIL_FILE_EXTENSION_NAI_FIRMWARE, &pszFileName[iStrlen - 4], 4) )  )
    {
      fRet = 1;

    /* Check for ZIP container */
    } else if( (iStrlen == OS_Strlen("FWUPDATE.ZIP"))                         &&
               (0       == OS_Strnicmp( "FWUPDATE.ZIP", &pszFileName[0], iStrlen)) )
    {
      fRet = 1;

    /* Check for NXS container */
    } else if( (iStrlen == OS_Strlen("FWUPDATE.NXS"))                         &&
               (0       == OS_Strnicmp( "FWUPDATE.NXS", &pszFileName[0], iStrlen)) )
    {
      fRet = 1;
    }
  }

  return fRet;
}

/*****************************************************************************/
/*! Check if we have a NXO file
*   \param pszFileName      Input file name
*   \return 1 on success                                                     */
/*****************************************************************************/
CIFX_STATIC int DEV_IsNXOFile( char* pszFileName)
{
  /* Check if we have a .NXO, .NXF,.NXM or .MOD extension */
  int fRet    = 0;
  int iStrlen = OS_Strlen(pszFileName);

  if( iStrlen >= CIFX_MIN_FILE_NAME_LENGTH)  /* At least x.abc */
  {
    if ( 0 == OS_Strnicmp( HIL_FILE_EXTENSION_OPTION, &pszFileName[iStrlen - 4], 4) )
    {
      fRet = 1;
    }
  }

  return fRet;
}

/*****************************************************************************/
/*! Check if we have a NXF file
*   \param pszFileName      Input file name
*   \return 1 on success                                                     */
/*****************************************************************************/
CIFX_STATIC int DEV_IsNXFFile( char* pszFileName)
{
  /* Check if we have a .NXO, .NXF,.NXM or .MOD extension */
  int fRet    = 0;
  int iStrlen = OS_Strlen(pszFileName);

  if( iStrlen >= CIFX_MIN_FILE_NAME_LENGTH)  /* At least x.abc */
  {
    if ( 0 == OS_Strnicmp( HIL_FILE_EXTENSION_FIRMWARE, &pszFileName[iStrlen - 4], 4) )
    {
      fRet = 1;
    }
  }

  return fRet;
}

/*****************************************************************************/
/*! Delete the given file
*   \param pvChannel          Channel instance
*   \param ulChannelNumber    Channel number
*   \param pszFileName        Input file name
*   \param pfnTransferPacket  Function used for transferring packets
*   \param pfnRecvPacket      User callback for unsolicited receive packets
*   \param pvUser             User parameter passed on callback
*   \return 1 on success                                                     */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_DeleteFile(void* pvChannel, uint32_t ulChannelNumber, char* pszFileName,
                                   PFN_TRANSFER_PACKET    pfnTransferPacket,
                                   PFN_RECV_PKT_CALLBACK  pfnRecvPacket,
                                   void*                  pvUser)
{
  /* Create delete packet */
  union
  {
    CIFX_PACKET           tPacket;
    HIL_FILE_DELETE_REQ_T tFileDelete;

  }                       uSendPkt;
  CIFX_PACKET             tConf;
  char*                   pbCopyPtr     = NULL;
  uint32_t                ulCopySize    = 0;
  uint16_t                usFileNameLen = (uint16_t)OS_Strlen(pszFileName);
  int32_t                 lRet          = CIFX_NO_ERROR;
  uint32_t                ulSrc         = OS_GetMilliSecCounter(); /* Early versions used pvChannel as ulSrc,
                                                                      but this won't work on 64 Bit machines.
                                                                      As we need something unique we use the current system time */

  OS_Memset(&uSendPkt, 0, sizeof(uSendPkt));
  OS_Memset(&tConf,    0, sizeof(tConf));

  /* Initialize the message */
  uSendPkt.tFileDelete.tHead.ulSrc    = HOST_TO_LE32(ulSrc);
  uSendPkt.tFileDelete.tHead.ulDest   = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
  uSendPkt.tFileDelete.tHead.ulCmd    = HOST_TO_LE32(HIL_FILE_DELETE_REQ);
  uSendPkt.tFileDelete.tHead.ulExt    = HOST_TO_LE32(HIL_PACKET_SEQ_NONE);
  uSendPkt.tFileDelete.tHead.ulLen    = HOST_TO_LE32((uint32_t)(sizeof(uSendPkt.tFileDelete.tData) +
                                                                usFileNameLen + 1));

  /* Insert file data */
  uSendPkt.tFileDelete.tData.ulChannelNo      = HOST_TO_LE32(ulChannelNumber);
  uSendPkt.tFileDelete.tData.usFileNameLength = HOST_TO_LE16( (uint16_t)(usFileNameLen + 1) );

  /* Setup copy buffer and copy size */
  pbCopyPtr   = ((char*)(&uSendPkt.tPacket.abData[0])) + sizeof(uSendPkt.tFileDelete.tData);
  ulCopySize  = HIL_MIN((sizeof(uSendPkt.tPacket.abData) - sizeof(uSendPkt.tFileDelete.tData)), uSendPkt.tFileDelete.tData.usFileNameLength);

  /* Insert file name */
  (void)OS_Strncpy( pbCopyPtr, pszFileName, ulCopySize);

  /* Send delete packet */
  lRet = pfnTransferPacket( pvChannel,
                            &uSendPkt.tPacket,
                            (CIFX_PACKET*)&tConf,
                            (uint32_t)sizeof(tConf),
                            CIFX_TO_FIRMWARE_START,       /* Could take a little while */
                            pfnRecvPacket,
                            pvUser);

  if(CIFX_NO_ERROR == lRet)
    lRet = LE32_TO_HOST(tConf.tHeader.ulState);

  return lRet;
}

/*****************************************************************************/
/*! Get the firmware transfer type from file name
*   \param eChipType        netC chip type working on
*   \param pszFileName      Input file name
*   \param pulTransferType  Buffer for transfer type
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_GetFWTransferTypeFromFileName( CIFX_TOOLKIT_CHIPTYPE_E eChipType,
                                                       char*                   pszFileName,
                                                       uint32_t*               pulTransferType)
{
  /* Check if we have a NXF or .NXM / .MOD extension */
  int32_t lRet = CIFX_NO_ERROR;

  int iStrlen = (int)OS_Strlen(pszFileName);
  if( iStrlen < CIFX_MIN_FILE_NAME_LENGTH)  /* At least x.abc */
  {
    lRet = CIFX_FILE_NAME_INVALID;
  } else
  {
    /* Check if we have a valid firmware file */
    lRet = CIFX_FILE_TYPE_INVALID;

    /* netX90/netX4000 files */
    if( (eCHIP_TYPE_NETX90   == eChipType) ||
        (eCHIP_TYPE_NETX4000 == eChipType) )
    {
      if (DEV_IsFWFileNetX90or4000( pszFileName))
      {
        /* Use file transfer type for netX90/4000 updates */
        *pulTransferType = HIL_FILE_XFER_FILE;
        lRet = CIFX_NO_ERROR;
      }
    } else if (DEV_IsFWFile( pszFileName))
    {
      /* other firmware files */
      /* We have a firmware file, choose the correct download type */
      if ( (0 == OS_Strnicmp( HIL_FILE_EXTENSION_NXM_FIRMWARE, &pszFileName[iStrlen - 4], 4) ) ||
           (0 == OS_Strnicmp( HIL_FILE_EXTENSION_OPTION,       &pszFileName[iStrlen - 4], 4) ) ||
           (0 == OS_Strnicmp( ".MOD", &pszFileName[iStrlen - 4], 4) )  )
      {
        /* We are using the module file transfer type */
        *pulTransferType = HIL_FILE_XFER_MODULE;
      } else
      {
        /* All other files are downloaded via the file transfer type */
        *pulTransferType = HIL_FILE_XFER_FILE;
      }
      lRet = CIFX_NO_ERROR;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Check if we have to download a file
*   \param pvChannel          Channel instance
*   \param ulChannelNumber    Channel number
*   \param pfDownload         Download flag
*   \param pszFileName        File name
*   \param pvFileData         File data buffer
*   \param ulFileSize         File size
*   \param pfnTransferPacket  Transfer packet function
*   \param pfnRecvPacket      Receive packet callback for unhandled packets
*   \param pvUser             User data for callback functions
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_CheckForDownload( void* pvChannel, uint32_t ulChannelNumber, int* pfDownload,
                                          char* pszFileName, void* pvFileData, uint32_t ulFileSize,
                                          PFN_TRANSFER_PACKET   pfnTransferPacket,
                                          PFN_RECV_PKT_CALLBACK pfnRecvPacket,
                                          void*                 pvUser)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)pvChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  /* Read the MD5 from the system */
  union
  {
    CIFX_PACKET             tPacket;
    HIL_FILE_GET_MD5_REQ_T  tRequest;
  }                         uSendPkt;
  union
  {
    CIFX_PACKET             tPacket;
    HIL_FILE_GET_MD5_CNF_T  tConf;
  }                         uConf;
  char*                     pbCopyPtr     = NULL;
  uint32_t                  ulCopySize    = 0;
  uint16_t                  usFileNameLen = (uint16_t)OS_Strlen(pszFileName);
  uint32_t                  ulSrc         = OS_GetMilliSecCounter(); /* Early versions used pvChannel as ulSrc,
                                                                        but this won't work on 64 Bit machines.
                                                                        As we need something unique we use the current system time */

  OS_Memset(&uSendPkt, 0, sizeof(uSendPkt));
  OS_Memset(&uConf,    0, sizeof(uConf));

  /* Set flag to download always necessary */
  *pfDownload = 1;

  /* Initialize the message */
  uSendPkt.tRequest.tHead.ulSrc              = HOST_TO_LE32(ulSrc);
  uSendPkt.tRequest.tHead.ulDest             = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
  uSendPkt.tRequest.tHead.ulCmd              = HOST_TO_LE32(HIL_FILE_GET_MD5_REQ);
  uSendPkt.tRequest.tHead.ulExt              = HOST_TO_LE32(HIL_PACKET_SEQ_NONE);
  uSendPkt.tRequest.tHead.ulLen              = HOST_TO_LE32((uint32_t)(sizeof(uSendPkt.tRequest.tData) + usFileNameLen + 1));
  uSendPkt.tRequest.tData.usFileNameLength   = HOST_TO_LE16( (uint16_t)(usFileNameLen + 1) );
  uSendPkt.tRequest.tData.ulChannelNo        = HOST_TO_LE32(ulChannelNumber);

  /* Setup copy buffer and copy size */
  pbCopyPtr   = ((char*)(&uSendPkt.tPacket.abData[0])) + sizeof(uSendPkt.tRequest.tData);
  ulCopySize  = HIL_MIN((sizeof(uSendPkt.tPacket.abData) - sizeof(uSendPkt.tRequest.tData)), uSendPkt.tRequest.tData.usFileNameLength);

  /* Insert file name */
  (void)OS_Strncpy( pbCopyPtr, pszFileName, ulCopySize);

  /* Read the MD5 from the system */
  lRet = pfnTransferPacket( pvChannel,
                            &uSendPkt.tPacket,
                            &uConf.tPacket,
                            (uint32_t)sizeof(uConf.tPacket),
                            CIFX_TO_FIRMWARE_START,       /* Could take a little while */
                            pfnRecvPacket,
                            pvUser);

  if(CIFX_NO_ERROR != lRet)
  {
    /* Error reading MD5 checksum */
    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                 CIFX_TRACE_LEVEL_ERROR,
                 "Failed to send MD5 request, lRet = 0x%08x", lRet);
    }
  } else if(SUCCESS_HIL_OK != LE32_TO_HOST(uConf.tConf.tHead.ulSta))
  {
    /* Error reading MD5 checksum */
    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_INFO)
    {
      USER_Trace(ptDevInstance,
                 CIFX_TRACE_LEVEL_INFO,
                 "No MD5 Information available. Probably the file does not exist on device. (ulState = 0x%08x)",
                 uConf.tConf.tHead.ulSta);
    }
  } else
  {
    /* We got an MD5 from the rcX, test it */
    /* Calculate MD5 */
    md5_state_t tMd5State;
    md5_byte_t  abMd5[16];

    OS_Memset(abMd5, 0, sizeof(abMd5));

    md5_init(&tMd5State);
    md5_append(&tMd5State, (md5_byte_t*)pvFileData, ulFileSize);
    md5_finish(&tMd5State, abMd5);

    if(OS_Memcmp(abMd5, uConf.tConf.tData.abMD5, sizeof(abMd5)) == 0)
    {
      /* same file already on device, suppress download */
      *pfDownload = 0;

      /* MD5 checksum is equal, no download necessary */
      if(g_ulTraceLevel & CIFX_TRACE_LEVEL_INFO)
      {
        USER_Trace(ptDevInstance,
                  CIFX_TRACE_LEVEL_INFO,
                  "MD5 checksum is identical, download not necessary");
      }

    } else
    {
      /* MD5 checksum is not identical, download necessary */
      if(g_ulTraceLevel & CIFX_TRACE_LEVEL_INFO)
      {
        USER_Trace(ptDevInstance,
                  CIFX_TRACE_LEVEL_INFO,
                  "MD5 not identical, process file download");
      }
    }
  }

  return lRet;
} /*lint !e429 : pvFileData not freed or returned */

/*****************************************************************************/
/*! Process firmware download
*   \param ptDevInstance      Instance to start up
*   \param ulChannel          Channel number
*   \param pszFullFileName    Full file name (used for opening file)
*   \param pszFileName        Short file name (used on device)
*   \param ulFileLength       Length of the file
*   \param pbBuffer           File buffer
*   \param pbLoadState        Returned action of download (see CIFXTKIT_DOWNLOAD_XXX)
*   \param pfnTransferPacket  Function to used for exchanging packets
*   \param pfnCallback        Progress callback
*   \param pfnRecvPktCallback Callback for unexpected packets
*   \param pvUser             Callback user parameter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_ProcessFWDownload( PDEVICEINSTANCE       ptDevInstance,
                                           uint32_t              ulChannel,
                                           char*                 pszFullFileName,
                                           char*                 pszFileName,
                                           uint32_t              ulFileLength,
                                           uint8_t*              pbBuffer,
                                           uint8_t*              pbLoadState,
                                           PFN_TRANSFER_PACKET   pfnTransferPacket,
                                           PFN_PROGRESS_CALLBACK pfnCallback,
                                           PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                           void*                 pvUser)
{
  PCHANNELINSTANCE ptSysDevice   = &ptDevInstance->tSystemDevice;
  int32_t          lRet          = CIFX_NO_ERROR;

  *pbLoadState  =  CIFXTKIT_DOWNLOAD_NONE;

  /*------------------------------------------------------------*/
  /* Process the firmware download depending on the eDeviceType */
  /*------------------------------------------------------------*/
  switch (ptDevInstance->eDeviceType)
  {
    /*----------------------------*/
    /* This is a RAM based device */
    /*----------------------------*/
    /* - RAM based devices are started by a RESET and therefore it is not necessary to delete a file */
    /* - Firmware files (NXF) and/or Modules (NXO) are loaded into RAM and not into the file system */
    case eCIFX_DEVICE_RAM_BASED:
    {
      /* We have not to delete files but we have to change the "transfer type" of the file */
      uint32_t ulTransfertype = HIL_FILE_XFER_MODULE;

      /* Check if we have a NXF*/
      if( CIFX_MAKE_DEV_FUN(DEV_IsNXFFile)(pszFileName) &&
          (0 != ulChannel) )
      {
        /* Downloading an NXF to a channel other than 0 is not supported */
        if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                      CIFX_TRACE_LEVEL_ERROR,
                      "Error channel number %u for a firmware is not supported",
                      ulChannel);
        }

      /* Check if we have an NXO file */
      } else if( CIFX_MAKE_DEV_FUN(DEV_IsNXOFile)(pszFileName) &&
                 (!ptDevInstance->fModuleLoad))
      {
        /* Downloading an NXO without a running Base OS is not allowed */
        if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                      CIFX_TRACE_LEVEL_ERROR,
                      "Error NXO files are not allowed without a Base OS firmware");
        }
      } else
      {
        /* Download the file stored in the buffer */
        lRet = CIFX_MAKE_DEV_FUN(DEV_DownloadFile)(
                                ptSysDevice,
                                ulChannel,
                                ptDevInstance->tSystemDevice.tSendMbx.ulSendMailboxLength,
                                ulTransfertype,
                                pszFileName,
                                ulFileLength,
                                pbBuffer,
                                pfnTransferPacket,
                                pfnCallback,
                                pfnRecvPktCallback,
                                pvUser);

        if(CIFX_NO_ERROR != lRet)
        {
          if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      CIFX_TRACE_LEVEL_ERROR,
                      "Error downloading firmware to device '%s'"\
                      " - (lRet=0x%08X)!",
                      pszFullFileName,
                      lRet);
          }
        } else
        {
          /*-----------------------*/
          /* We have loaded a file */
          /*-----------------------*/

          /* Check if we have a NXF */
          if ( CIFX_MAKE_DEV_FUN(DEV_IsNXFFile)( pszFileName))
          {
            /* NXF loaded, store information for startup handling */
            *pbLoadState = CIFXTKIT_DOWNLOAD_FIRMWARE | CIFXTKIT_DOWNLOAD_EXECUTED; /* we have a firmware loaded */
          }

          /* Check if we have a NXO */
          if ( CIFX_MAKE_DEV_FUN(DEV_IsNXOFile)( pszFileName))
          {
            /* NXO loaded, store information for startup handling */
            *pbLoadState = CIFXTKIT_DOWNLOAD_MODULE  | CIFXTKIT_DOWNLOAD_EXECUTED;  /* we have a module loaded */
          }

          if(g_ulTraceLevel & CIFX_TRACE_LEVEL_DEBUG)
          {
            USER_Trace(ptDevInstance,
                      CIFX_TRACE_LEVEL_DEBUG,
                      "Successfully downloaded the firmware to device '%s'!",
                      pszFullFileName);
          }
        }
      }
    }
    break;

    /*------------------------------*/
    /* This is a FLASH based device */
    /*------------------------------*/
    /* - FLASH based devices are not reseted on the beginning */
    /* - Files are checked if they are already existing to prevent a download into FLASH */
    /* - If a new firmware files (NXF) is loaded, all other files (NXD/NXO etc) are deleted in all "PORTs" */
    /* - Firmware files are only allowed for PORT0 */
    /* - If an NXO is downloaded, all files (NXO/NXD) are deleted first. An existing NXF must be protected, because it is the base module! */
    case eCIFX_DEVICE_FLASH_BASED:
    {
      /* We have not to delete files but we have to change the "transfer type" of the file */
      uint32_t ulTransfertype  = HIL_FILE_XFER_FILE;
      int      fDownload       = 0;

      /* Does the file exist on the hardware, if so, skip the download */
      if ( CIFX_NO_ERROR != (lRet = CIFX_MAKE_DEV_FUN(DEV_CheckForDownload)(
                                                          ptSysDevice,
                                                          ulChannel,
                                                          &fDownload,
                                                          pszFileName,
                                                          pbBuffer,
                                                          ulFileLength,
                                                          pfnTransferPacket,
                                                          NULL,
                                                          NULL)))
      {
        /* Display an error */
        if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                      CIFX_TRACE_LEVEL_ERROR,
                      "Error checking for download '%s'!",
                      pszFullFileName);
        }

      /* Check if we have to download the file */
      } else if(!fDownload)
      {
        /*-----------------------------------*/
        /* Download not necessary            */
        /*-----------------------------------*/
        /* Store NXO Information for startup */
        if(CIFX_MAKE_DEV_FUN(DEV_IsNXOFile)(pszFileName))
        {
          if( !ptDevInstance->fModuleLoad)
          {
            /* Downloading an NXO without a running Base OS is not allowed */
            if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                          CIFX_TRACE_LEVEL_ERROR,
                          "Error NXO files are not allowed without a Base OS firmware");
            }

            lRet = CIFX_FILE_TYPE_INVALID;

          } else
          {
            if(g_ulTraceLevel & CIFX_TRACE_LEVEL_DEBUG)
            {
              USER_Trace(ptDevInstance,
                        CIFX_TRACE_LEVEL_DEBUG,
                        "Skipping download for file '%s'" \
                        "[checksum identical]!",
                        pszFullFileName);
            }

            *pbLoadState = CIFXTKIT_DOWNLOAD_MODULE;
          }
        } else if(CIFX_MAKE_DEV_FUN(DEV_IsNXFFile)(pszFileName))
        {
          *pbLoadState = CIFXTKIT_DOWNLOAD_FIRMWARE;
        } else
        {
          lRet = CIFX_FILE_TYPE_INVALID;
        }
      } else
      {

        /*-----------------------------------*/
        /* Download is necessary             */
        /*-----------------------------------*/
        /* Check if we have a NXF*/
        if( CIFX_MAKE_DEV_FUN(DEV_IsNXFFile)(pszFileName))
        {
          if (0 != ulChannel)
          {
            /* Downloading an NXF to a channel other than 0 is not supported */
            if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                          CIFX_TRACE_LEVEL_ERROR,
                          "Error channel number %u for a firmware is not supported",
                          ulChannel);
            }
            lRet = CIFX_INVALID_PARAMETER;
            fDownload = 0;

          } else
          {
            /* ATTENTION: If we are downloading an "NXF" file, we have a complete firmware.     */
            /*            In this case all other files should be deleted!                       */
            /*            NXF are stored under channel 0                                        */

            /* Files for a flash based device are always transfered into the FLASH file system */
            /* We have to delete existing files, depending of a NXF/NXO */
            uint32_t ulChNum = 0;

            /* Remove ALL files */
            for ( ulChNum = 0; ulChNum < CIFX_MAX_NUMBER_OF_CHANNELS; ulChNum++)
            {
              (void)CIFX_MAKE_DEV_FUN(DEV_RemoveChannelFiles)(ptSysDevice, ulChNum, pfnTransferPacket, NULL, NULL, NULL);
            }

            /* We have loaded a new firmware file */
            *pbLoadState = CIFXTKIT_DOWNLOAD_FIRMWARE;
          }

         /* Check if we have an NXO file */
        } else if( CIFX_MAKE_DEV_FUN(DEV_IsNXOFile)(pszFileName))
        {
          if( !ptDevInstance->fModuleLoad)
          {
            /* Downloading an NXO without a running Base OS is not allowed */
            if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                          CIFX_TRACE_LEVEL_ERROR,
                          "Error NXO files are not allowed without a Base OS firmware");
            }

            lRet = CIFX_FILE_TYPE_INVALID;
            fDownload = 0;

          } else
          {
            /* ATTENTION: If the file is an "NXO", we have to delete all files EXCEPT the "NXF" */
            /*            because this is our BASE OS file!                                     */
            /*            NXF are stored under channel 0, NXOs are storeable in each channel    */

            /* Files for a flash based device are always transfered into the FLASH file system */
            /* We have to delete existing files, depending of a NXF/NXO */
            /* Leave NXF file */
            (void)CIFX_MAKE_DEV_FUN(DEV_RemoveChannelFiles)( ptSysDevice, ulChannel, pfnTransferPacket, NULL, NULL, HIL_FILE_EXTENSION_FIRMWARE);

            /* We have loaded a new module */
            *pbLoadState = CIFXTKIT_DOWNLOAD_MODULE;
          }
        } else
        {
          /* TODO: Unsupported file , do we need to check this???*/
          fDownload = 0;
        }

        if(fDownload)
        {
          /* Download the file stored in the buffer */
          lRet = CIFX_MAKE_DEV_FUN(DEV_DownloadFile)(
                                  ptSysDevice,
                                  ulChannel,
                                  ptDevInstance->tSystemDevice.tSendMbx.ulSendMailboxLength,
                                  ulTransfertype,
                                  pszFileName,
                                  ulFileLength,
                                  pbBuffer,
                                  pfnTransferPacket,
                                  NULL,
                                  NULL,
                                  NULL);

          if(CIFX_NO_ERROR != lRet)
          {
            if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        CIFX_TRACE_LEVEL_ERROR,
                        "Error downloading firmware to device '%s'"\
                        " - (lRet=0x%08X)!",
                        pszFullFileName,
                        lRet);
            }

          } else
          {
            if(g_ulTraceLevel & CIFX_TRACE_LEVEL_DEBUG)
            {
              USER_Trace(ptDevInstance,
                        CIFX_TRACE_LEVEL_DEBUG,
                        "Successfully downloaded the firmware to device '%s'!",
                        pszFullFileName);
            }

            *pbLoadState |= CIFXTKIT_DOWNLOAD_EXECUTED;
          }
        }
      }
    }
    break;

    default:
      /* Unknown device type */
      if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                    CIFX_TRACE_LEVEL_ERROR,
                    "Error unsupported device type %u found for download handling!",
                    ptDevInstance->eDeviceType);
      }
    break;

  } /* end switch */

  return lRet;
} /*lint !e429 : pbBuffer not freed or returned */

/*****************************************************************************/
/*! Download a file to the hardware
*   \param pvChannel          Channel instance the download is performed on
*   \param ulChannel          Channel number the download is for
*   \param ulMailboxSize      Size of the mailbox
*   \param ulTransferType     Type of transfer (see HIL_FILE_XFER_XXX defines)
*   \param szFileName         Short file name (needed by firmware to create the file by name)
*   \param ulFileLength       Length of the file to download
*   \param pvData             File data being downloaded
*   \param pfnTransferPacket  Function used for transferring packets
*   \param pfnCallback        User callback for download progress indications
*   \param pfnRecvPktCallback User callback for unsolicited receive packets
*   \param pvUser             User parameter passed on callback
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_DownloadFile(void*                 pvChannel,
                                     uint32_t              ulChannel,
                                     uint32_t              ulMailboxSize,
                                     uint32_t              ulTransferType,
                                     char*                 szFileName,
                                     uint32_t              ulFileLength,
                                     void*                 pvData,
                                     PFN_TRANSFER_PACKET   pfnTransferPacket,
                                     PFN_PROGRESS_CALLBACK pfnCallback,
                                     PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                     void*                 pvUser)
{
  union
  {
    CIFX_PACKET                   tPacket;
    HIL_FILE_DOWNLOAD_REQ_T       tDownloadReq;
    HIL_FILE_DOWNLOAD_DATA_REQ_T  tDownloadDataReq;
    HIL_FILE_DOWNLOAD_ABORT_REQ_T tAbortReq;
  }          uSendPkt;
  union
  {
    CIFX_PACKET                   tPacket;
    HIL_FILE_DOWNLOAD_CNF_T       tDownloadCnf;
    HIL_FILE_DOWNLOAD_DATA_CNF_T  tDownloadDataCnf;
    HIL_FILE_DOWNLOAD_ABORT_CNF_T tAbortCnf;
  }          uRecvPkt;

  /* Set download state informations */
  uint32_t   ulMaxDataLength     = ulMailboxSize -  /* Maximum possible user data length */
                                   (uint32_t)sizeof(HIL_FILE_DOWNLOAD_DATA_REQ_T);

  char*      pbCopyPtr           = NULL;
  uint32_t   ulCopySize          = 0;
  uint32_t   ulSendLen           = 0;
  uint32_t   ulTransferedLength  = 0;
  uint8_t*   pabActData          = NULL;
  uint32_t   ulCRC               = 0;
  uint32_t   ulBlockNumber       = 0;
  uint32_t   ulState             = HIL_FILE_DOWNLOAD_REQ;
  uint32_t   ulCmdDataState      = HIL_PACKET_SEQ_NONE;
  int        fStopDownload       = 0;
  int32_t    lRetAbort           = CIFX_NO_ERROR;
  int32_t    lRet                = CIFX_NO_ERROR;
  uint32_t   ulCurrentId         = 0;
  uint32_t   ulSrc               = OS_GetMilliSecCounter(); /* Early versions used pvChannel as ulSrc,
                                                               but this won't work on 64 Bit machines.
                                                               As we need something unique we use the current system time */
  uint32_t   ulTransferTimeout   = CIFX_TO_SEND_PACKET;

  OS_Memset(&uSendPkt, 0, sizeof(uSendPkt));
  OS_Memset(&uRecvPkt, 0, sizeof(uRecvPkt));

  /* Check parameters */
  if( NULL == pvData)
    return CIFX_INVALID_POINTER;

  if( 0 == ulFileLength)
    return CIFX_INVALID_PARAMETER;

  pabActData = (uint8_t*)pvData;

  /* Performce download */
  do
  {
    switch (ulState)
    {
      /* Send download request */
      case HIL_FILE_DOWNLOAD_REQ:
      {
        /* Validate filename length to fit mailbox/packet */
        uint32_t ulFileNameLength = HIL_MIN(((uint32_t)OS_Strlen(szFileName) + 1),
                                            (ulMailboxSize - (uint32_t)sizeof(HIL_FILE_DOWNLOAD_REQ_T))); /*lint !e666 : function call OS_Strlen() */

        /* Insert packet data */
        ++ulCurrentId;
        uSendPkt.tDownloadReq.tHead.ulDest   = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
        uSendPkt.tDownloadReq.tHead.ulSrc    = HOST_TO_LE32(ulSrc);
        uSendPkt.tDownloadReq.tHead.ulDestId = HOST_TO_LE32(0);
        uSendPkt.tDownloadReq.tHead.ulSrcId  = HOST_TO_LE32(0);
        uSendPkt.tDownloadReq.tHead.ulLen    = HOST_TO_LE32((uint32_t)(sizeof(HIL_FILE_DOWNLOAD_REQ_DATA_T) +
                                                                       ulFileNameLength));
        uSendPkt.tDownloadReq.tHead.ulId     = HOST_TO_LE32(ulCurrentId);
        uSendPkt.tDownloadReq.tHead.ulSta    = HOST_TO_LE32(0);
        uSendPkt.tDownloadReq.tHead.ulCmd    = HOST_TO_LE32(HIL_FILE_DOWNLOAD_REQ);
        uSendPkt.tDownloadReq.tHead.ulExt    = HOST_TO_LE32(ulCmdDataState);
        uSendPkt.tDownloadReq.tHead.ulRout   = HOST_TO_LE32(0);

        /* Insert command data (extended data) */
        uSendPkt.tDownloadReq.tData.ulFileLength     = HOST_TO_LE32(ulFileLength);
        uSendPkt.tDownloadReq.tData.ulMaxBlockSize   = HOST_TO_LE32(ulMaxDataLength);
        uSendPkt.tDownloadReq.tData.ulXferType       = HOST_TO_LE32(ulTransferType);
        uSendPkt.tDownloadReq.tData.ulChannelNo      = HOST_TO_LE32(ulChannel);
        uSendPkt.tDownloadReq.tData.usFileNameLength = HOST_TO_LE16((uint16_t)ulFileNameLength);

        /* Setup copy buffer and copy size */
        pbCopyPtr   = ((char*)(&uSendPkt.tPacket.abData[0])) + sizeof(uSendPkt.tDownloadReq.tData);
        ulCopySize  = HIL_MIN((sizeof(uSendPkt.tPacket.abData) - sizeof(uSendPkt.tDownloadReq.tData)), uSendPkt.tDownloadReq.tData.usFileNameLength);

        /* Insert file name */
        (void)OS_Strncpy( pbCopyPtr, szFileName, ulCopySize);

        /* Usually a file system is used for file storage, but there are configurations
         * that use RAW FLASH instead (e.g. netX90 usecase A/B). Deleting a file inside
         * the file system is usually quite quick, but erasing a FLASH area can take a
         * considerable amount of time (depending on the area size and the erase time
         * of the FLASH chip). That's why we try to calculate the necessary timeout
         * value for a successful first download packet. If the netX requires less time,
         * the function will return sooner. The following assumptions are made here:
         * Erase sector size: 4096, Erase sector time: 150ms.
         */

        /* Transfer packet */
        lRet = pfnTransferPacket(pvChannel,
                                 &uSendPkt.tPacket,
                                 &uRecvPkt.tPacket,
                                 (uint32_t)sizeof(uRecvPkt.tPacket),
                                 ulTransferTimeout + ((ulFileLength / 4096) * 150),
                                 pfnRecvPktCallback,
                                 pvUser);

        if( (CIFX_NO_ERROR  != lRet)                                 ||
            (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST((int32_t)uRecvPkt.tDownloadCnf.tHead.ulSta))) )
        {
          /* Error during first packet, end download */
          /* Send progress notification */
          if(pfnCallback)
            pfnCallback(ulTransferedLength, ulFileLength, pvUser, CIFX_CALLBACK_FINISHED, lRet);

          /* Send abort request on unusable data */
          ulState = HIL_FILE_DOWNLOAD_ABORT_REQ;
        } else if( LE32_TO_HOST(uRecvPkt.tDownloadCnf.tData.ulMaxBlockSize) == 0)
        {
          /* Error in device information, stop download (Device returned illegal block size */
          lRet = CIFX_INVALID_ACCESS_SIZE;

          /* Send progress notification */
          if(pfnCallback)
            pfnCallback(ulTransferedLength, ulFileLength, pvUser, CIFX_CALLBACK_FINISHED, lRet);

          /* Send abort request on unusable data */
          ulState = HIL_FILE_DOWNLOAD_ABORT_REQ;
        } else
        {
          /* Everything went ok, so start transmitting file data now */
          /* Get download packet size from the device confirmation.
             If the devices packet size is smaller than our size, use the length from the device.
             Otherwise use our length. */
          if( ulMaxDataLength > LE32_TO_HOST(uRecvPkt.tDownloadCnf.tData.ulMaxBlockSize))
            ulMaxDataLength = LE32_TO_HOST(uRecvPkt.tDownloadCnf.tData.ulMaxBlockSize);

          /* Check if the file fits into one packet or if we have to send multiple packets */
          ulSendLen = ulMaxDataLength;
          if(ulFileLength <= ulSendLen)
          {
            /* We have only one packet to send */
            ulSendLen       = ulFileLength;
            ulCmdDataState  = HIL_PACKET_SEQ_NONE;
          } else
          {
            /* We have to send multiple packets */
            ulCmdDataState  = HIL_PACKET_SEQ_FIRST;
          }

          /* Goto next state */
          ulState = HIL_FILE_DOWNLOAD_DATA_REQ;
        }
      }
      break;

      /* Data download packets */
      case HIL_FILE_DOWNLOAD_DATA_REQ:
      {
        ++ulCurrentId;
        uSendPkt.tDownloadDataReq.tHead.ulDest     = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
        uSendPkt.tDownloadDataReq.tHead.ulSrc      = HOST_TO_LE32(ulSrc);
        uSendPkt.tDownloadDataReq.tHead.ulCmd      = HOST_TO_LE32(HIL_FILE_DOWNLOAD_DATA_REQ);
        uSendPkt.tDownloadDataReq.tHead.ulId       = HOST_TO_LE32(ulCurrentId);
        uSendPkt.tDownloadDataReq.tHead.ulExt      = HOST_TO_LE32(ulCmdDataState);

        /* Copy file data to packet */
        OS_Memcpy( &uSendPkt.tDownloadDataReq.tData + 1, pabActData, ulSendLen);

        /* Adjust packet length */
        uSendPkt.tDownloadDataReq.tHead.ulLen      = HOST_TO_LE32((uint32_t)(sizeof(HIL_FILE_DOWNLOAD_DATA_REQ_DATA_T) +
                                                                             ulSendLen));

        /* Create continued CRC */
        ulCRC = CreateCRC32( ulCRC, pabActData, ulSendLen);
        uSendPkt.tDownloadDataReq.tData.ulChksum   = HOST_TO_LE32(ulCRC);
        uSendPkt.tDownloadDataReq.tData.ulBlockNo  = HOST_TO_LE32(ulBlockNumber);
        ++ulBlockNumber;

        /* Transfer packet */
        lRet = pfnTransferPacket(pvChannel,
                                &uSendPkt.tPacket,
                                &uRecvPkt.tPacket,
                                (uint32_t)sizeof(uRecvPkt.tPacket),
                                ulTransferTimeout,
                                pfnRecvPktCallback,
                                pvUser);

        if( (CIFX_NO_ERROR  != lRet)                                   ||
            (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST((int32_t)(uRecvPkt.tDownloadDataCnf.tHead.ulSta)))) )
        {
          /* Driver error during transfer packet, end download */
          /* Always try to send an abort request */
          if(pfnCallback)
            pfnCallback(ulTransferedLength, ulFileLength, pvUser, CIFX_CALLBACK_FINISHED, lRet);

          ulState = HIL_FILE_DOWNLOAD_ABORT_REQ;
        } else
        {
          /* Add send size to transferred size */
          ulTransferedLength += ulSendLen;

          /* Indicate progress, if user wants a notification */
          if(pfnCallback)
            pfnCallback(ulTransferedLength, ulFileLength, pvUser,
                        (ulTransferedLength == ulFileLength) ? CIFX_CALLBACK_FINISHED : CIFX_CALLBACK_ACTIVE,
                        lRet);

          /* Check if we are done with the download */
          if( (HIL_PACKET_SEQ_LAST == ulCmdDataState) ||
              (HIL_PACKET_SEQ_NONE == ulCmdDataState) )
          {
            /* No more packets to send, end download */
            fStopDownload = 1;
          } else
          {
            /* Move data pointer to next data */
            pabActData += ulSendLen;

            /* Calculate next message length */
            if ( ulFileLength <= (ulSendLen + ulTransferedLength))
            {
              /* Set the send length to rest of data,
                 This will be the last packet */
              ulSendLen = ulFileLength - ulTransferedLength;
              ulCmdDataState = HIL_PACKET_SEQ_LAST;

              /* ATTENTION: Check the transfer type */
              if ( HIL_FILE_XFER_MODULE == ulTransferType)
              {
                /* Module loading will relocate the module with the last packet.
                   So the confirmation packet takes longer, depending on the
                   file size (and contained firmware).
                   Measurements showed that for every 100kB the module needs
                   one additional second for relocation */
                ulTransferTimeout += (ulFileLength / (100 * 1024)) * 1000;
              }
            } else
            {
              ulCmdDataState = HIL_PACKET_SEQ_MIDDLE;
            }

            /* Goto next state */
            ulState = HIL_FILE_DOWNLOAD_DATA_REQ;
          }
        }
      }
      break;

      /* Abort active download */
      case HIL_FILE_DOWNLOAD_ABORT_REQ:
      {
        ++ulCurrentId;
        uSendPkt.tAbortReq.tHead.ulDest   = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
        uSendPkt.tAbortReq.tHead.ulSrc    = HOST_TO_LE32(ulSrc);
        uSendPkt.tAbortReq.tHead.ulDestId = HOST_TO_LE32(0);
        uSendPkt.tAbortReq.tHead.ulSrcId  = HOST_TO_LE32(0);
        uSendPkt.tAbortReq.tHead.ulLen    = HOST_TO_LE32(0);
        uSendPkt.tAbortReq.tHead.ulId     = HOST_TO_LE32(ulCurrentId);
        uSendPkt.tAbortReq.tHead.ulSta    = HOST_TO_LE32(0);
        uSendPkt.tAbortReq.tHead.ulCmd    = HOST_TO_LE32(HIL_FILE_DOWNLOAD_ABORT_REQ);
        uSendPkt.tAbortReq.tHead.ulExt    = HOST_TO_LE32(HIL_PACKET_SEQ_NONE);
        uSendPkt.tAbortReq.tHead.ulRout   = HOST_TO_LE32(0);

        /* Transfer packet */
        lRetAbort = pfnTransferPacket(pvChannel,
                                      &uSendPkt.tPacket,
                                      &uRecvPkt.tPacket,
                                      (uint32_t)sizeof(uRecvPkt.tPacket),
                                      ulTransferTimeout,
                                      pfnRecvPktCallback,
                                      pvUser);

        if( lRetAbort == CIFX_NO_ERROR)
        {
          /* Return packet state if function succeeded */
          lRetAbort = LE32_TO_HOST((int32_t)uRecvPkt.tAbortCnf.tHead.ulSta);
        }

        /* End download */
        fStopDownload = 1;
      }
      break;

      default:
        /* unknown, leave command */
        lRet = CIFX_FUNCTION_FAILED;

        /* End download */
        fStopDownload = 1;
        break;
    }

  } while(!fStopDownload);

  /* Always return lRet first, then abort error */
  if( CIFX_NO_ERROR != lRet)
    return lRet;
  else if( CIFX_NO_ERROR != lRetAbort)
    return lRetAbort;
  else
    return CIFX_NO_ERROR;
} /*lint !e429 : pvData not freed or returned */

/*****************************************************************************/
/*! Uploads a file from the hardware. It is required to list the files
* on the hardware, to know the file length for creating the buffer.
*   \param pvChannel          Channel instance the upload is performed on
*   \param ulChannel          Channel number the upload made is for
*   \param ulMailboxSize      Size of the mailbox
*   \param ulTransferType     Type of transfer (see HIL_FILE_XFER_XXX defines)
*   \param szFileName         Short file name
*   \param pulDataBufferLen   Length of the provided buffer, returned length of data
*   \param pvData             Buffer for storing upload. This buffer must be allocated by the caller.
*   \param pfnTransferPacket  Function used for transferring packets
*   \param pfnCallback        User callback for upload progress indications
*   \param pfnRecvPktCallback User callback for unsolicited receive packets
*   \param pvUser             User parameter passed on callback
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_UploadFile(void*                   pvChannel,
                                   uint32_t                ulChannel,
                                   uint32_t                ulMailboxSize,
                                   uint32_t                ulTransferType,
                                   char*                   szFileName,
                                   uint32_t*               pulDataBufferLen,
                                   void*                   pvData,
                                   PFN_TRANSFER_PACKET     pfnTransferPacket,
                                   PFN_PROGRESS_CALLBACK   pfnCallback,
                                   PFN_RECV_PKT_CALLBACK   pfnRecvPktCallback,
                                   void*                   pvUser)
{
  /* Usually one brace should be enough, but GNU wants to have a second brace
     to initialize the structure. On GCC 4.0.3 the whole structure is initialized
     as described in ISOC90 */
  union
  {
    CIFX_PACKET                   tPacket;
    HIL_FILE_UPLOAD_REQ_T         tUploadReq;
    HIL_FILE_UPLOAD_DATA_REQ_T    tUploadDataReq;
    HIL_FILE_DOWNLOAD_ABORT_REQ_T tAbortReq;
  }                             uSendPkt;

  union
  {
    CIFX_PACKET                   tPacket;
    HIL_FILE_UPLOAD_CNF_T         tUploadCnf;
    HIL_FILE_UPLOAD_DATA_CNF_T    tUploadDataCnf;
  }                             uRecvPkt;

  char*                         pbCopyPtr         = NULL;
  uint32_t                      ulCopySize        = 0;
  uint32_t                      ulFileLength      = 0;
  uint16_t                      usFilenameLen     = (uint16_t)(OS_Strlen(szFileName) + 1); /*Firmware expects length including terminating NULL */
  uint32_t                      ulBlockSize       = ulMailboxSize -
                                                    (uint32_t)sizeof(uRecvPkt.tUploadDataCnf); /* maximum size of each file block */
  int                           fSendAbort        = 0;
  int32_t                       lRetAbort         = CIFX_NO_ERROR;
  int32_t                       lRet              = CIFX_NO_ERROR;
  uint32_t                      ulCurrentId       = 0;
  uint32_t                      ulSrc             = OS_GetMilliSecCounter(); /* Early versions used pvChannel as ulSrc,
                                                                                but this won't work on 64 Bit machines.
                                                                                As we need something unique we use the current system time */

  OS_Memset(&uSendPkt, 0, sizeof(uSendPkt));
  OS_Memset(&uRecvPkt, 0, sizeof(uRecvPkt));

  /* Check parameters */
  if( (NULL == pvData) || (NULL == pulDataBufferLen) )
    return CIFX_INVALID_POINTER;

  if( ulMailboxSize < HIL_DPM_SYSTEM_MAILBOX_MIN_SIZE)
    return CIFX_DEV_MAILBOX_TOO_SHORT;

  ++ulCurrentId;
  uSendPkt.tUploadReq.tHead.ulDest             = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
  uSendPkt.tUploadReq.tHead.ulSrc              = HOST_TO_LE32(ulSrc);
  uSendPkt.tUploadReq.tHead.ulDestId           = HOST_TO_LE32(0);
  uSendPkt.tUploadReq.tHead.ulSrcId            = HOST_TO_LE32(0);
  uSendPkt.tUploadReq.tHead.ulLen              = HOST_TO_LE32((uint32_t)(sizeof(uSendPkt.tUploadReq.tData) + usFilenameLen));
  uSendPkt.tUploadReq.tHead.ulId               = HOST_TO_LE32(ulCurrentId);
  uSendPkt.tUploadReq.tHead.ulSta              = HOST_TO_LE32(0);
  uSendPkt.tUploadReq.tHead.ulCmd              = HOST_TO_LE32(HIL_FILE_UPLOAD_REQ);
  uSendPkt.tUploadReq.tHead.ulExt              = HOST_TO_LE32(HIL_PACKET_SEQ_NONE);
  uSendPkt.tUploadReq.tHead.ulRout             = HOST_TO_LE32(0);

  uSendPkt.tUploadReq.tData.usFileNameLength   = HOST_TO_LE16(usFilenameLen);
  uSendPkt.tUploadReq.tData.ulXferType         = HOST_TO_LE32(ulTransferType);
  uSendPkt.tUploadReq.tData.ulMaxBlockSize     = HOST_TO_LE32(ulBlockSize);
  uSendPkt.tUploadReq.tData.ulChannelNo        = HOST_TO_LE32(ulChannel);

  /* Setup copy buffer and copy size */
  pbCopyPtr   = ((char*)(&uSendPkt.tPacket.abData[0])) + sizeof(uSendPkt.tUploadReq.tData);
  ulCopySize  = HIL_MIN((sizeof(uSendPkt.tPacket.abData) - sizeof(uSendPkt.tUploadReq.tData)), uSendPkt.tUploadReq.tData.usFileNameLength);

  (void)OS_Strncpy( pbCopyPtr, szFileName, ulCopySize);

  lRet = pfnTransferPacket(pvChannel,
                           &uSendPkt.tPacket,
                           &uRecvPkt.tPacket,
                           (uint32_t)sizeof(uRecvPkt.tPacket),
                           CIFX_TO_SEND_PACKET,
                           pfnRecvPktCallback,
                           pvUser);

  /* Read file length */
  ulFileLength = LE32_TO_HOST(uRecvPkt.tUploadCnf.tData.ulFileLength);

  /* ATTENTION: We have to send an "Abort" to the system if:                              */
  /*  1. Command or File Error occured                                                    */
  /*  2. If the file exists but the length is 0                                           */
  /* In both cases, it is possible the system has activated a data transfer and waits     */
  /* on data requests commands.                                                           */
  /* It is necessary to send a "Abort" command, otherwise the next file access will fail  */
  /* with an error "COMMAND_ACTIVE".                                                      */

  if( (CIFX_NO_ERROR  != lRet)                                                      ||
      (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(uRecvPkt.tPacket.tHeader.ulState)))   ||
      (0              == ulFileLength)                                              )
  {
    /* Set return of read file length to 0 */
    *pulDataBufferLen = 0;

    /* Send progress notification */
    if(pfnCallback)
      pfnCallback( 0, 0, pvUser, CIFX_CALLBACK_FINISHED, lRet);

    /* Execute an abort command */
    fSendAbort = 1;
  } else
  {
    /* Check file length against user buffer length */
    if(ulFileLength > *pulDataBufferLen)
    {
      fSendAbort = 1;
      lRet = CIFX_INVALID_BUFFERSIZE;
    } else
    {
      uint32_t  ulCRC              = 0;
      uint8_t*  pbData             = (uint8_t*)pvData; /* pointer to return buffer */
      uint32_t  ulTransferredBytes = 0;
      uint32_t  ulTotalBytes       = ulFileLength;

      /* Set return of read file length to 0 */
      *pulDataBufferLen = 0;

      /* Create upload data packet */
      ++ulCurrentId;
      OS_Memset( &uSendPkt.tUploadDataReq, 0, sizeof(uSendPkt.tUploadDataReq));
      uSendPkt.tUploadDataReq.tHead.ulDest     = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
      uSendPkt.tUploadDataReq.tHead.ulSrc      = HOST_TO_LE32(ulSrc);
      uSendPkt.tUploadDataReq.tHead.ulDestId   = HOST_TO_LE32(0);
      uSendPkt.tUploadDataReq.tHead.ulSrcId    = HOST_TO_LE32(0);
      uSendPkt.tUploadDataReq.tHead.ulLen      = HOST_TO_LE32(0);
      uSendPkt.tUploadDataReq.tHead.ulId       = HOST_TO_LE32(ulCurrentId);
      uSendPkt.tUploadDataReq.tHead.ulSta      = HOST_TO_LE32(0);
      uSendPkt.tUploadDataReq.tHead.ulCmd      = HOST_TO_LE32(HIL_FILE_UPLOAD_DATA_REQ);
      uSendPkt.tUploadDataReq.tHead.ulExt      = HOST_TO_LE32(HIL_PACKET_SEQ_NONE);
      uSendPkt.tUploadDataReq.tHead.ulRout     = HOST_TO_LE32(0);

      /* Adjust block size to the size of the system */
      if( LE32_TO_HOST(uRecvPkt.tUploadCnf.tData.ulMaxBlockSize) < ulBlockSize)
         ulBlockSize = LE32_TO_HOST(uRecvPkt.tUploadCnf.tData.ulMaxBlockSize);

      /* Check size we have to send */
      /* If this is only one packet, set extension to NONE */
      uSendPkt.tUploadDataReq.tHead.ulExt = HOST_TO_LE32(HIL_PACKET_SEQ_FIRST);
      if( ulTotalBytes <= ulBlockSize)
        uSendPkt.tUploadDataReq.tHead.ulExt = HOST_TO_LE32(HIL_PACKET_SEQ_NONE);    /* We can send all in one packet */

      /* Perform upload */
      while( (ulFileLength > 0) && (CIFX_NO_ERROR == lRet) )
      {
        /* Send and receive data */
        lRet = pfnTransferPacket(pvChannel,
                                 &uSendPkt.tPacket,
                                 &uRecvPkt.tPacket,
                                 (uint32_t)sizeof(uRecvPkt.tPacket),
                                 CIFX_TO_SEND_PACKET,
                                 pfnRecvPktCallback,
                                 pvUser);
        /* Check for errors */
        if( (CIFX_NO_ERROR  != lRet)                                ||
            (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(uRecvPkt.tPacket.tHeader.ulState))) )
        {
          /* This is a packet error from the hardware */
          /* - Inform application */
          /* - Leave upload and send abort */
          if(pfnCallback)
            pfnCallback(ulTransferredBytes, ulTotalBytes, pvUser, CIFX_CALLBACK_FINISHED, lRet);

          fSendAbort = 1;
          break;
        } else
        {
          uint32_t  ulCurrentDataLen = LE32_TO_HOST(uRecvPkt.tUploadDataCnf.tHead.ulLen) -
                                                    (uint32_t)sizeof(uRecvPkt.tUploadDataCnf.tData);
          uint8_t*  pbRecvData       = (uint8_t*)(&uRecvPkt.tUploadDataCnf.tData + 1);
          uint32_t  ulPacketCrc      = LE32_TO_HOST(uRecvPkt.tUploadDataCnf.tData.ulChksum);

          /* Create own checksum and compare with it */
          ulCRC = CreateCRC32( ulCRC, pbRecvData, ulCurrentDataLen);

          if(ulCRC != ulPacketCrc)
          {
            /* Abort, as a CRC32 error occurred */
            lRet = CIFX_FILE_CHECKSUM_ERROR;

            /* Send progress notification */
            if(pfnCallback)
              pfnCallback(ulTransferredBytes, ulTotalBytes, pvUser, CIFX_CALLBACK_FINISHED, lRet);

            fSendAbort = 1;
            break;
          } else
          {
            /* Next packet */
            ++ulCurrentId;
            uSendPkt.tUploadDataReq.tHead.ulId = HOST_TO_LE32(ulCurrentId);

            /* Calculate outstanding size */
            ulFileLength        -= ulCurrentDataLen;
            OS_Memcpy(pbData, pbRecvData, ulCurrentDataLen);
            pbData              += ulCurrentDataLen;
            ulTransferredBytes  += ulCurrentDataLen;
            *pulDataBufferLen   = ulTransferredBytes;

            /* Send progress notification */
            if(pfnCallback)
              pfnCallback(ulTransferredBytes, ulTotalBytes, pvUser,
                          (ulTransferredBytes == ulTotalBytes)? CIFX_CALLBACK_FINISHED : CIFX_CALLBACK_ACTIVE,
                          lRet);

            /* Calculate next packet length and packet extension */
            if(ulFileLength != 0)
            {
              if(ulFileLength <= ulBlockSize)
                uSendPkt.tUploadDataReq.tHead.ulExt = HOST_TO_LE32(HIL_PACKET_SEQ_LAST);
              else
                uSendPkt.tUploadDataReq.tHead.ulExt = HOST_TO_LE32(HIL_PACKET_SEQ_MIDDLE);
            }
          }
        }
      }
    }
  }

  /* If anything failed during upload, send an abort request */
  if( fSendAbort)
  {
    ++ulCurrentId;
    uSendPkt.tAbortReq.tHead.ulDest   = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
    uSendPkt.tAbortReq.tHead.ulSrc    = HOST_TO_LE32(ulSrc);
    uSendPkt.tAbortReq.tHead.ulDestId = HOST_TO_LE32(0);
    uSendPkt.tAbortReq.tHead.ulSrcId  = HOST_TO_LE32(0);
    uSendPkt.tAbortReq.tHead.ulLen    = HOST_TO_LE32(0);
    uSendPkt.tAbortReq.tHead.ulId     = HOST_TO_LE32(ulCurrentId);
    uSendPkt.tAbortReq.tHead.ulSta    = HOST_TO_LE32(0);
    uSendPkt.tAbortReq.tHead.ulCmd    = HOST_TO_LE32(HIL_FILE_UPLOAD_ABORT_REQ);
    uSendPkt.tAbortReq.tHead.ulExt    = HOST_TO_LE32(HIL_PACKET_SEQ_NONE);
    uSendPkt.tAbortReq.tHead.ulRout   = HOST_TO_LE32(0);

    /* Transfer packet */
    lRetAbort = pfnTransferPacket(pvChannel,
                                  &uSendPkt.tPacket,
                                  &uRecvPkt.tPacket,
                                  (uint32_t)sizeof(uRecvPkt.tPacket),
                                  CIFX_TO_SEND_PACKET,
                                  pfnRecvPktCallback,
                                  pvUser);

    if( lRetAbort == CIFX_NO_ERROR)
    {
      /* Return packet state if function succeeded */
      lRetAbort = LE32_TO_HOST((int32_t)uRecvPkt.tPacket.tHeader.ulState);
    }
  }

  /* Always return lRet first, then abort error */
  if( CIFX_NO_ERROR != lRet)
    return lRet;
  else if( CIFX_NO_ERROR != lRetAbort)
    return lRetAbort;
  else
    return CIFX_NO_ERROR;
}

#ifdef CIFX_TOOLKIT_DMA
/*****************************************************************************/
/*! Setup DMA buffers
*   \param ptChannel   Channel instance
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_SetupDMABuffers( PCHANNELINSTANCE ptChannel)
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
  PNETX_GLOBAL_REG_BLOCK ptGlobalRegisters = (PNETX_GLOBAL_REG_BLOCK)ptDevInstance->pvGlobalRegisters;

  /* Get the DMA control registers */
  ulChannelNumber = ptChannel->ulChannelNumber ;
  ulDMAChIdx      = ulChannelNumber * 2; /* 2 DMA channels per communication channel */

  /* Get the corresponding netX DMA control register */
  pDMACtrl_1      = &ptGlobalRegisters->atDmaCtrl[ulDMAChIdx + eDMA_INPUT_BUFFER_IDX]; /* Input channel */
  pDMACtrl_2      = &ptGlobalRegisters->atDmaCtrl[ulDMAChIdx + eDMA_OUTPUT_BUFFER_IDX];/* Output channel */

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
CIFX_STATIC int32_t DEV_DMAState(PCHANNELINSTANCE ptChannel, uint32_t ulCmd, uint32_t* pulState)
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
#endif

#ifdef CIFX_TOOLKIT_FUNCTION_LIST
/*****************************************************************************/
/*! Local structure for cifX DEV function pointers                           */
/*****************************************************************************/
static CIFX_DEV_FUNCTION_LIST_T s_tCifxDpmDevFuns =
{
  DEV_WriteHandshakeFlags,
  DEV_ReadHostFlags,
  DEV_ReadHandshakeFlags,
  NULL,
  DEV_WaitForBitState,
  NULL,
  NULL,
  DEV_ToggleBit,
  NULL,
  NULL,
  DEV_WaitForSyncState,
  DEV_ToggleSyncBit,
  DEV_PutPacket,
  DEV_GetPacket,
  DEV_GetMBXState,
  NULL,
  DEV_TransferPacket,
  DEV_IsReady,
  DEV_IsRunning,
  DEV_IsCommunicating,
  DEV_WaitForReady_Poll,
  DEV_WaitForNotReady_Poll,
  DEV_WaitForRunning_Poll,
  DEV_WaitForNotRunning_Poll,
  NULL,
  DEV_TriggerWatchdog,
  DEV_GetHostState,
  DEV_SetHostState,
  DEV_ReadWriteBlock,
  DEV_DoChannelInit,
  DEV_DoSystemStart,
  DEV_DoSystemBootstart,
  DEV_DoUpdateStart,
  DEV_BusState,
  DEV_DoHostCOSChange,
  DEV_CheckCOSFlags,
  DEV_GetHandshakeBitState,
  DEV_RemoveChannelFiles,
  DEV_DeleteFile,
  DEV_CheckForDownload,
  DEV_IsFWFile,
  DEV_IsNXFFile,
  DEV_IsNXOFile,
  DEV_GetFWTransferTypeFromFileName,
  DEV_ProcessFWDownload,
  DEV_DownloadFile,
  DEV_UploadFile,
#ifdef CIFX_TOOLKIT_DMA
  DEV_DMAState,
  DEV_SetupDMABuffers,
#endif
};

PCIFX_DEV_FUNCTION_LIST_T cifXTkitGetDpmDevFunctionList(void)
{
  return &s_tCifxDpmDevFuns;
}

#endif

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/
