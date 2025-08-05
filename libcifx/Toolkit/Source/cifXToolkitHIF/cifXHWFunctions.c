/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXHWFunctions.c 15191 2025-08-05 13:33:12Z AMinor $:

  Description:
    cifX API Hardware handling functions implementation

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-05-26  Update to new DPMv2 handling
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
#include "Hil_SystemCmd.h"
#include "Hil_Results.h"

#include <stdlib.h>

/*****************************************************************************/
/*!  \addtogroup CIFX_TK_HARDWARE Hardware Access
*    \{                                                                      */
/*****************************************************************************/

/*****************************************************************************/
/*! Reads the actual state of the handshake bits for the given channel
*   \param ptChannel      Channel instance to change for bit for
*   \param fReadSyncFlags !=0 if sync flags should be updated
*   \param fLockNeeded    !=0 if flag access lock is needed.                 */
/*****************************************************************************/
CIFX_STATIC void DEV_ReadHandshakeFlags(PCHANNELINSTANCE ptChannel, int fReadSyncFlags, int fLockNeeded)
{
  /* Lock Handshake Cell and COS flag accesses */
  if (fLockNeeded)
    OS_EnterLock(ptChannel->pvLock);

#if 0 // TODO
  if ((ptDevInstance->pulHandshakeBlock != NULL) && fReadSyncFlags)
  {
    HIL_DPM_HANDSHAKE_ARRAY_T* ptHandshakeBlock = (HIL_DPM_HANDSHAKE_ARRAY_T*)ptDevInstance->pulHandshakeBlock;
    ptDevInstance->tSyncData.usNSyncFlags = LE16_TO_HOST(HWIF_READ16(ptDevInstance, ptHandshakeBlock->atHsk[1].t16Bit.ulNetxFlags));
  }
#endif

  ptChannel->tHsCtrl.ulNetxFlags = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, *ptChannel->tHsCtrl.pulNetxFlags));

  if (!ptChannel->fIsSysDevice)
  {
    ptChannel->tToHostMbx.tCom.tCtl.ulNetxFlags = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, *ptChannel->tToHostMbx.tCom.tCtl.pulNetxFlags));
    ptChannel->tFromHostMbx.tCom.tCtl.ulNetxFlags = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, *ptChannel->tFromHostMbx.tCom.tCtl.pulNetxFlags));
    ptChannel->tIoArea.tTlbCtl.ulTlbNetxStatus = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, *ptChannel->tIoArea.tTlbCtl.pulTlbNetxStatus));
  }

  /* Unlock Handshake Cell and COS flag accesses */
  if (fLockNeeded)
    OS_LeaveLock(ptChannel->pvLock);
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
    bActualState = (ptChannel->tHsCtrl.ulNetxFlags & ulBitMask)? HIL_FLAGS_SET : HIL_FLAGS_CLEAR;

  } else
  {
    if((ptChannel->tHsCtrl.ulHostFlags ^ ptChannel->tHsCtrl.ulNetxFlags) & ulBitMask)
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
      bActualState = (ptChannel->tHsCtrl.ulNetxFlags & ulBitMask)? HIL_FLAGS_SET : HIL_FLAGS_CLEAR;

    } else
    {
      if((ptChannel->tHsCtrl.ulHostFlags ^ ptChannel->tHsCtrl.ulNetxFlags) & ulBitMask)
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
/*! Waits until the DMA of SMS Tbuf has finished (incremental update mode only)
*   \param ptChannel    Channel instance to wait for
*   \param ptInst       Block instance to wait for
*   \param ulTimeout    Maximum time in ms to wait for the desired bit state
*   \return 0 on error/timeout, 1 on success                                 */
/*****************************************************************************/
CIFX_STATIC int DEV_WaitForLock_Poll(PCHANNELINSTANCE ptChannel, PNETX_IO_BLOCK_T ptInst, uint32_t ulTimeout)
{
  int32_t lStartTime = 0;
  uint8_t bActualState;
  int iRet = 0;

  bActualState = HWIF_READ8(ptChannel->pvDeviceInstance, *ptInst->tIoCtl.pbStatus);
  bActualState = (bActualState & NETX_IO_STATUS_LOCKSTATE_MSK);

  /* The desired state is already there, so just return true */
  if (0 == bActualState)
    return 1;

  /* If no timeout is given, don't try to wait for the Bit change */
  if (0 == ulTimeout)
    return 0;

  lStartTime = (int32_t)OS_GetMilliSecCounter();

  /* Poll for desired bit state */
  while (0 != bActualState)
  {
    uint32_t ulDiffTime  = 0L;

    bActualState = HWIF_READ8(ptChannel->pvDeviceInstance, *ptInst->tIoCtl.pbStatus);
    bActualState = (bActualState & NETX_IO_STATUS_LOCKSTATE_MSK);

    /* The desired state is already there, so just return true */
    if (0 == bActualState)
    {
      iRet = 1;
      break;
    }

    /* Check for timeout */
    ulDiffTime = OS_GetMilliSecCounter() - lStartTime;
    if (ulDiffTime > ulTimeout)
    {
      break;
    }

    OS_Sleep(0);
  }

  return iRet;

}

/*****************************************************************************/
/*! Waits for a given handshake bit state on the I/O area (polling mode)
*   \param ptChannel    Channel instance to wait for
*   \param ptInst       Block instance on which to wait for
*   \param ulTimeout    Maximum time in ms to wait for the desired bit state
*   \return 0 on error/timeout, 1 on success                                 */
/*****************************************************************************/
CIFX_STATIC int DEV_WaitForIoBitState_Poll(PCHANNELINSTANCE ptChannel, NETX_IO_BLOCK_T* ptInst, uint32_t ulTimeout)
{
  uint32_t ulActualState;
  int32_t lStartTime = 0;
  int iRet = 0;

  DEV_ReadHandshakeFlags(ptChannel, 0, 1);

  ulActualState = (ptChannel->tIoArea.tTlbCtl.ulTlbNetxStatus & ptInst->tBlock.ulBitmask);

  /* The desired state is already there, so just return true */
  if (ulActualState == ptInst->tBlock.ulBitmask)
    return 1;

  /* If no timeout is given, don't try to wait for the Bit change */
  if (0 == ulTimeout)
    return 0;

  lStartTime = (int32_t)OS_GetMilliSecCounter();

  /* Poll for desired bit state */
  while (ulActualState != ptInst->tBlock.ulBitmask)
  {
    uint32_t ulDiffTime  = 0L;

    DEV_ReadHandshakeFlags(ptChannel, 0, 1);

    ulActualState = (ptChannel->tIoArea.tTlbCtl.ulTlbNetxStatus & ptInst->tBlock.ulBitmask);

    /* The desired state is already there, so just return true */
    if (ulActualState == ptInst->tBlock.ulBitmask)
    {
      iRet = 1;
      break;
    }

    /* Check for timeout */
    ulDiffTime = OS_GetMilliSecCounter() - lStartTime;
    if (ulDiffTime > ulTimeout)
    {
      break;
    }

    OS_Sleep(0);
  }

  return iRet;
}

/*****************************************************************************/
/*! Waits for a given handshake bit state on the I/O area (irq mode)
*   \param ptChannel    Channel instance to wait for
*   \param ptInst       Block instance on which to wait for
*   \param ulTimeout    Maximum time in ms to wait for the desired bit state
*   \return 0 on error/timeout, 1 on success                                 */
/*****************************************************************************/
CIFX_STATIC int DEV_WaitForIoBitState_Irq(PCHANNELINSTANCE ptChannel, NETX_IO_BLOCK_T* ptInst, uint32_t ulTimeout)
{
  uint32_t ulActualState;
  int32_t lStartTime = 0;
  uint32_t ulInternalTimeout = ulTimeout;
  int iRet = 0;

  ulActualState = (ptChannel->tIoArea.tTlbCtl.ulTlbNetxStatus ^ ptChannel->tIoArea.tTlbCtl.ulTlbHostStatus) & ptInst->tBlock.ulBitmask;

  /* The desired state is already there, so just return true */
  if (ulActualState == ptInst->tBlock.ulBitmask)
    return 1;

  /* If no timeout is given, don't try to wait for the Bit change */
  if (0 == ulTimeout)
    return 0;

  lStartTime = (int32_t)OS_GetMilliSecCounter();

  do {
    uint32_t ulCurrentTime;
    uint32_t ulDiffTime;

    /* Wait for DSR to signal Handshake bit change event */
    (void)OS_WaitEvent(ptInst->tIoCtl.pvEvent, ulInternalTimeout);

    ulCurrentTime = OS_GetMilliSecCounter();
    ulDiffTime    = ulCurrentTime - lStartTime;

    /* Adjust timeout for next run */
    ulInternalTimeout = ulTimeout - ulDiffTime;

    ulActualState = (ptChannel->tIoArea.tTlbCtl.ulTlbNetxStatus ^ ptChannel->tIoArea.tTlbCtl.ulTlbHostStatus) & ptInst->tBlock.ulBitmask;

    /* The desired state is already there, so just return true */
    if (ulActualState == ptInst->tBlock.ulBitmask)
    {
      iRet = 1;
      break;
    }

    if (ulDiffTime >= ulTimeout)
    {
      /* Timeout expired */
      break;
    }

  } while (iRet == 0);

  return iRet;
}

/*****************************************************************************/
/*! Waits for a given handshake bit state on the I/O area (irq mode)
*   \param ptChannel    Channel instance to wait for
*   \param ptInst       Block instance on which to wait for
*   \param ulTimeout    Maximum time in ms to wait for the desired bit state
*   \return 0 on error/timeout, 1 on success                                 */
/*****************************************************************************/
CIFX_STATIC int DEV_WaitForIoBitState(PCHANNELINSTANCE ptChannel, NETX_IO_BLOCK_T* ptInst, uint32_t ulTimeout)
{
  if( ((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    return DEV_WaitForIoBitState_Irq(ptChannel, ptInst, ulTimeout);
  else
    return DEV_WaitForIoBitState_Poll(ptChannel, ptInst, ulTimeout);
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
    bActualState = (ptChannel->tHsCtrl.ulNetxFlags & ulBitMask)? HIL_FLAGS_SET : HIL_FLAGS_CLEAR;

  } else
  {
    if((ptChannel->tHsCtrl.ulHostFlags ^ ptChannel->tHsCtrl.ulNetxFlags) & ulBitMask)
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
    (void)OS_WaitEvent(ptChannel->apvHsBitEvent[ulBitNumber], ulInternalTimeout);

    ulCurrentTime = OS_GetMilliSecCounter();
    ulDiffTime    = ulCurrentTime - lStartTime;

    /* Adjust timeout for next run */
    ulInternalTimeout = ulTimeout - ulDiffTime;

    /* Check bit state */
    if( (HIL_FLAGS_CLEAR == bState) ||
        (HIL_FLAGS_SET == bState) )
    {
      bActualState = (ptChannel->tHsCtrl.ulNetxFlags & ulBitMask)? HIL_FLAGS_SET : HIL_FLAGS_CLEAR;

    } else
    {
      if((ptChannel->tHsCtrl.ulHostFlags ^ ptChannel->tHsCtrl.ulNetxFlags) & ulBitMask)
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
/*! Waits for a given state on the communication mailbox (polling mode)
*   (IRQ/Polling Wrapper function)
*   \param ptChannel    Channel instance to wait for
*   \param ptInst       Block instance on which to wait for
*   \param ulState      State to wait for
*   \param ulTimeout    Maximum time in ms to wait for the desired bit state
*   \return 0 on error/timeout, 1 on success                                 */
/*****************************************************************************/
static int DEV_WaitForMbxState_Poll(PCHANNELINSTANCE ptChannel, NETX_MAILBOX_BLOCK_T* ptInst, uint32_t ulState, uint32_t ulTimeout)
{
  uint32_t ulNetxFlags = 0;
  int32_t  lStartTime  = 0;
  int      iRet        = 0;

  DEV_ReadHandshakeFlags(ptChannel, 0, 1);
  ulNetxFlags = ptInst->tCtl.ulNetxFlags & ptInst->tBlock.ulBitmask;
  if (NETX_MBX_COM_STATE_EMPTY == ulState)
    ulNetxFlags ^= HIL_HIF_MBX_WRAPAROUND;

  if ((ptInst->tCtl.ulHostFlags & ptInst->tBlock.ulBitmask) != ulNetxFlags)
    return 1;

  /* If no timeout is given, don't try to wait for the Bit change */
  if(0 == ulTimeout)
    return 0;

  lStartTime = (int32_t)OS_GetMilliSecCounter();

  /* Poll for desired bit state */
  do {
    uint32_t ulDiffTime = 0;

    DEV_ReadHandshakeFlags(ptChannel, 0, 1);
    ulNetxFlags = ptInst->tCtl.ulNetxFlags & ptInst->tBlock.ulBitmask;
    if (NETX_MBX_COM_STATE_EMPTY == ulState)
      ulNetxFlags ^= HIL_HIF_MBX_WRAPAROUND;

    if ((ptInst->tCtl.ulHostFlags & ptInst->tBlock.ulBitmask) != ulNetxFlags)
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

  } while (ptInst->tCtl.ulHostFlags == ulNetxFlags);

  return iRet;
}

/*****************************************************************************/
/*! Waits for a given state on the communication mailbox (irq mode)
*   (IRQ/Polling Wrapper function)
*   \param ptChannel    Channel instance to wait for
*   \param ptInst       Block instance on which to wait for
*   \param ulState      State to wait for
*   \param ulTimeout    Maximum time in ms to wait for the desired bit state
*   \return 0 on error/timeout, 1 on success                                 */
/*****************************************************************************/
static int DEV_WaitForMbxState_Irq(PCHANNELINSTANCE ptChannel, NETX_MAILBOX_BLOCK_T* ptInst, uint32_t ulState, uint32_t ulTimeout)
{
  uint32_t ulNetxFlags       = 0;
  int32_t  lStartTime        = 0;
  int      iRet              = 0;
  uint32_t ulInternalTimeout = ulTimeout;

  ulNetxFlags = ptInst->tCtl.ulNetxFlags & ptInst->tBlock.ulBitmask;
  if (NETX_MBX_COM_STATE_EMPTY == ulState)
    ulNetxFlags ^= HIL_HIF_MBX_WRAPAROUND;

  if ((ptInst->tCtl.ulHostFlags & ptInst->tBlock.ulBitmask) != ulNetxFlags)
    return 1;

  /* If no timeout is given, don't try to wait for the Bit change */
  if(0 == ulTimeout)
    return 0;

  /* Just wait for the Interrupt event to be signaled. This bit was toggled if the interrupt
     is executed, so we don't need to check bit state afterwards
     Note: Wait first time with timeout 0 and check if the state is the expected one.
           If not it was a previously set event and we need to wait with the user supplied time out */

  lStartTime = (int32_t)OS_GetMilliSecCounter();

  do {
    uint32_t ulCurrentTime;
    uint32_t ulDiffTime;

    /* Wait for DSR to signal Handshake bit change event */
    (void)OS_WaitEvent(ptInst->tCtl.pvEvent, ulInternalTimeout);

    ulCurrentTime = OS_GetMilliSecCounter();
    ulDiffTime    = ulCurrentTime - lStartTime;

    /* Adjust timeout for next run */
    ulInternalTimeout = ulTimeout - ulDiffTime;

    ulNetxFlags = ptInst->tCtl.ulNetxFlags & ptInst->tBlock.ulBitmask;
    if (NETX_MBX_COM_STATE_EMPTY == ulState)
      ulNetxFlags ^= HIL_HIF_MBX_WRAPAROUND;

    if ((ptInst->tCtl.ulHostFlags & ptInst->tBlock.ulBitmask) != ulNetxFlags)
    {
      iRet = 1;
      break;
    }

    if( ulDiffTime >= ulTimeout)
    {
      /* Timeout expired */
      break;
    }

  } while (iRet == 0);

  return iRet;
}

/*****************************************************************************/
/*! Waits for a given state on the communication mailbox
*   (IRQ/Polling Wrapper function)
*   \param ptChannel    Channel instance to wait for
*   \param ptInst       Block instance on which to wait for
*   \param ulState      State to wait for
*   \param ulTimeout    Maximum time in ms to wait for the desired bit state
*   \return 0 on error/timeout, 1 on success                                 */
/*****************************************************************************/
CIFX_STATIC int DEV_WaitForMbxState(PCHANNELINSTANCE ptChannel, NETX_MAILBOX_BLOCK_T* ptInst, uint32_t ulState, uint32_t ulTimeout)
{
  if( ((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    return DEV_WaitForMbxState_Irq(ptChannel, ptInst, ulState, ulTimeout);
  else
    return DEV_WaitForMbxState_Poll(ptChannel, ptInst, ulState, ulTimeout);
}

/*****************************************************************************/
/*! Toggles the given command handshake bit
*   \param ptChannel    Channel instance to change for bit for
*   \param ulBitMask    Bitmask to eXOR into command bits                    */
/*****************************************************************************/
CIFX_STATIC void DEV_ToggleBit(PCHANNELINSTANCE ptChannel, uint32_t ulBitMask)
{
  ptChannel->tHsCtrl.ulHostFlags ^= ulBitMask;
  HWIF_WRITE32(ptChannel->pvDeviceInstance, *ptChannel->tHsCtrl.pulHostFlags, ptChannel->tHsCtrl.ulHostFlags);
}

/*****************************************************************************/
/*! Toggles the given I/O action bit
*   \param ptChannel    Channel instance to change for bit for
*   \param ptInst       Block instance to toggle                             */
/*****************************************************************************/
CIFX_STATIC void DEV_ToggleIoAction(PCHANNELINSTANCE ptChannel, NETX_IO_BLOCK_T* ptInst)
{
  ptChannel->tIoArea.tTlbCtl.ulTlbHostStatus ^= ptInst->tBlock.ulBitmask;
  ptInst->tIoCtl.bAction ^= NETX_IO_STATUS_TOGGLEBIT_MSK;
  HWIF_WRITE8(ptChannel->pvDeviceInstance, *ptInst->tIoCtl.pbAction, ptInst->tIoCtl.bAction);
}

/*****************************************************************************/
/*! Toggles the given handshake cell
*   \param ptChannel  Channel instance to change for bit for
*   \param ptInst     Block instance to change the cell for
*   \param ulValue    Value to change the cell to                            */
/*****************************************************************************/
CIFX_STATIC void DEV_WriteCell(PCHANNELINSTANCE ptChannel, NETX_MAILBOX_BLOCK_T* ptInst, uint32_t ulValue)
{
  ptInst->tCtl.ulHostFlags = ulValue;
  HWIF_WRITE32(ptChannel->pvDeviceInstance, *ptInst->tCtl.pulHostFlags, ptInst->tCtl.ulHostFlags);
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
  int             iRet          = 0;
  uint32_t        ulBitMask     = 1 << ptChannel->ulChannelNumber;
  int32_t         lStartTime    = 0;
  PDEVICEINSTANCE ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  DEV_ReadHandshakeFlags(ptChannel, 1, 1);

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

  lStartTime = (int32_t)OS_GetMilliSecCounter();

  /* Poll for desired bit state */
  while(bActualState != bState)
  {
    uint32_t ulDiffTime  = 0L;

    DEV_ReadHandshakeFlags(ptChannel, 1, 1);

    if((ptDevInstance->tSyncData.usHSyncFlags ^ ptDevInstance->tSyncData.usNSyncFlags) & ulBitMask)
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

#if 0 // TODO
    /* Wait for DSR to signal Handshake bit change event */
    (void)OS_WaitEvent(ptDevInstance->tSyncData.ahSyncBitEvents[ptChannel->ulChannelNumber], ulInternalTimeout);
#endif

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
/*! Toggles the given sync bit
*   \param ptDevInstance  Device instance
*   \param ulBitMask      Bitmask to eXOR into command bits                  */
/*****************************************************************************/
CIFX_STATIC void DEV_ToggleSyncBit(PDEVICEINSTANCE ptDevInstance, uint32_t ulBitMask)
{
#if 0 // TODO
  /* Write 16 Bit handshake */
  HIL_DPM_HANDSHAKE_ARRAY_T* ptHandshakeBlock = (HIL_DPM_HANDSHAKE_ARRAY_T*)ptDevInstance->pulHandshakeBlock;

  OS_EnterLock(ptDevInstance->tSyncData.pvLock);

  ptDevInstance->tSyncData.usHSyncFlags ^= (uint16_t)ulBitMask;
  HWIF_WRITE16(ptDevInstance, ptHandshakeBlock->atHsk[1].t16Bit.ulHostFlags, HOST_TO_LE16(ptDevInstance->tSyncData.usHSyncFlags));

  OS_LeaveLock(ptDevInstance->tSyncData.pvLock);
#endif
}

/*****************************************************************************/
/*! Reads the actual state of the host handshake bits for the given channel
*   \param ptChannel    Channel instance to change for bit for
*   \param fReadHostCOS !=0 if Application COS should be read                */
/*****************************************************************************/
CIFX_STATIC void DEV_ReadHostFlags(PCHANNELINSTANCE ptChannel, int fReadHostCOS)
{
  uint32_t ulIdx;

  ptChannel->tHsCtrl.ulHostFlags = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, *ptChannel->tHsCtrl.pulHostFlags));

  if (!ptChannel->fIsSysDevice)
  {
    ptChannel->tToHostMbx.tCom.tCtl.ulHostFlags = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, *ptChannel->tToHostMbx.tCom.tCtl.pulHostFlags));
    ptChannel->tFromHostMbx.tCom.tCtl.ulHostFlags = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, *ptChannel->tFromHostMbx.tCom.tCtl.pulHostFlags));
    ptChannel->tIoArea.tTlbCtl.ulTlbHostStatus = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, *ptChannel->tIoArea.tTlbCtl.pulTlbNetxStatus));

    for (ulIdx = 0; ulIdx < ptChannel->tIoArea.ulIOInputAreas; ulIdx++)
    {
      ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tIoCtl.bAction =
          HWIF_READ8(ptChannel->pvDeviceInstance, *ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tIoCtl.pbStatus)  & NETX_IO_STATUS_TOGGLEBIT_MSK;
    }
    for (ulIdx = 0; ulIdx < ptChannel->tIoArea.ulIOOutputAreas; ulIdx++)
    {
      ptChannel->tIoArea.aptIOOutputAreas[ulIdx]->tIoCtl.bAction =
          HWIF_READ8(ptChannel->pvDeviceInstance, *ptChannel->tIoArea.aptIOOutputAreas[ulIdx]->tIoCtl.pbStatus) & NETX_IO_STATUS_TOGGLEBIT_MSK;
    }
  }

#if 0 // TODO
  /* Also read host sync flags, as they might not be set to zero on flash based devices */
  if( (ptDevInstance->pulHandshakeBlock != NULL) &&
      (ptChannel->fIsSysDevice) )
  {
    HIL_DPM_HANDSHAKE_ARRAY_T* ptHandshakeBlock = (HIL_DPM_HANDSHAKE_ARRAY_T*)ptDevInstance->pulHandshakeBlock;
    ptDevInstance->tSyncData.usHSyncFlags       = LE16_TO_HOST(HWIF_READ16(ptChannel->pvDeviceInstance, ptHandshakeBlock->atHsk[1].t16Bit.ulHostFlags));
  }
#endif
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
  if (!((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    DEV_ReadHandshakeFlags(ptChannel, 0, 1);

  /* only a Communication channel can be running */
  if (!ptChannel->fIsSysDevice)
  {
    if ((ptChannel->tHsCtrl.ulNetxFlags & HIL_HIF_NCF_READY) &&
      (ptChannel->tHsCtrl.ulNetxFlags & HIL_HIF_NCF_RUN))
    {
      iRet = 1;
    }
  }

  return iRet;
}

/*****************************************************************************/
/*! Checks if the channel is ready
*   \param ptChannel  Channel instance to check
*   \return 1 if channel is ready                                            */
/*****************************************************************************/
CIFX_STATIC int DEV_IsReady(PCHANNELINSTANCE ptChannel)
{
  int iRet = 0;

  /* Handshake flags are read on interrupt, so no need to read them here */
  if (!((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    DEV_ReadHandshakeFlags(ptChannel, 0, 1);

  if (ptChannel->fIsSysDevice)
  {
    if (ptChannel->tHsCtrl.ulNetxFlags & NSF_READY)
    {
      iRet = 1;
    }
  }
  else
  {
    if (ptChannel->tHsCtrl.ulNetxFlags & HIL_HIF_NCF_READY)
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
      if( (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, *ptChannel->tHsCtrl.pulNetxFlags)) == CIFX_DPM_INVALID_CONTENT)     ||
          (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, *ptChannel->tHsCtrl.pulNetxFlags)) == CIFX_DPM_NO_MEMORY_ASSIGNED)  ||
          (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, *ptChannel->tHsCtrl.pulNetxFlags)) & NSF_READY) == 0 )
      {
        /* Card is not ready anymore */
        iActualState = 1;
        break;
      }

      ulDiffTime = OS_GetMilliSecCounter() - lStartTime;

      OS_Sleep(0);

    } while (ulDiffTime < ulTimeout);
  } else
  {
    /* This is a communication channel which is restarted */
    do
    {
      if( (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->ulCommunicationState)) == CIFX_DPM_INVALID_CONTENT)     ||
          (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->ulCommunicationState)) == CIFX_DPM_NO_MEMORY_ASSIGNED)  ||
          (!DEV_IsReady(ptChannel)) )
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
    /* Wait until firmware is running */
    OS_Sleep( 10);

    do
    {
      DEVICEINSTANCE* ptDevInstance  = (DEVICEINSTANCE*)ptChannel->pvDeviceInstance;
      char            szCookie[5]    = {0};

      /* Read the DPM cookie */
      HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);

      /* We need to check for a valid cookie */
      if (0 == OS_Strcmp(szCookie, CIFX_DPMSIGNATURE_FW_STR))
      {
        /* Check if firmware is READY because we need the DPM Layout */
        if( (LE32_TO_HOST(HWIF_READ32(ptDevInstance, *ptChannel->tHsCtrl.pulNetxFlags)) != CIFX_DPM_INVALID_CONTENT) &&
            (LE32_TO_HOST(HWIF_READ32(ptDevInstance, *ptChannel->tHsCtrl.pulNetxFlags)) != CIFX_DPM_NO_MEMORY_ASSIGNED)  )
        {
          /* Check if firmware is READY because we need the DPM Layout */
          if(HWIF_READ32(ptChannel->pvDeviceInstance, *ptChannel->tHsCtrl.pulNetxFlags) & NSF_READY)
          {
            DEV_ReadHostFlags(ptChannel, 0);

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
  HWIF_WRITE32(ptChannel->pvDeviceInstance, *ptChannel->tHsCtrl.pulHostFlags, ptChannel->tHsCtrl.ulHostFlags);
}

/*****************************************************************************/
/*! Sends a Packet using the system channel mailbox
*   \param ptChannel    Channel instance
*   \param ptSendPkt    Packet to send
*   \param ulTimeout    Maximum time in ms to wait for an empty mailbox
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_PutPacketSys(PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout)
{
  NETX_MAILBOX_AREA_U* ptMailbox = &ptChannel->tFromHostMbx;
  int32_t              lRet = CIFX_DEV_MAILBOX_FULL;

  if (!DEV_IsReady(ptChannel))
    return CIFX_DEV_NOT_READY;

  /* Check if packet fits into the mailbox */
  if ((LE32_TO_HOST(ptSendPkt->tHeader.ulLen) + HIL_PACKET_HEADER_SIZE) > ptMailbox->tSys.ulElementSize)
    return CIFX_DEV_MAILBOX_TOO_SHORT;

  if (DEV_WaitForBitState(ptChannel, ptMailbox->tSys.bBitoffset, HIL_FLAGS_EQUAL, ulTimeout))
  {
    /* Copy packet to mailbox */
    ++ptMailbox->tSys.ulTransmissionCnt;
    HWIF_WRITEN(ptChannel->pvDeviceInstance,
      ptMailbox->tSys.pbBlockStart,
      ptSendPkt,
      LE32_TO_HOST(ptSendPkt->tHeader.ulLen) + HIL_PACKET_HEADER_SIZE);

    /* Lock flag access */
    OS_EnterLock(ptChannel->pvLock);

    /* Signal new packet */
    DEV_ToggleBit(ptChannel, ptMailbox->tSys.ulBitmask);

    /* Unlock flag access */
    OS_LeaveLock(ptChannel->pvLock);

    lRet = CIFX_NO_ERROR;
  }

  return lRet;
}

/*****************************************************************************/
/*! Sends a Packet using the communication mailbox
*   \param ptChannel    Channel instance
*   \param ptSendPkt    Packet to send
*   \param ulTimeout    Maximum time in ms to wait for an empty mailbox
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_PutPacketCom(PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout)
{
  NETX_BLOCK_T* ptMailbox = &ptChannel->tFromHostMbx.tCom.tBlock;
  NETX_HS_CTL_T* ptCtl = &ptChannel->tFromHostMbx.tCom.tCtl;
  int32_t        lRet = CIFX_DEV_MAILBOX_FULL;

  if (!DEV_IsReady(ptChannel))
    return CIFX_DEV_NOT_READY;

  /* Check if packet fits into the mailbox */
  if ((LE32_TO_HOST(ptSendPkt->tHeader.ulLen) + HIL_PACKET_HEADER_SIZE) > ptMailbox->ulElementSize)
    return CIFX_DEV_MAILBOX_TOO_SHORT;

  if (DEV_WaitForMbxState(ptChannel, &ptChannel->tFromHostMbx.tCom, NETX_MBX_COM_STATE_EMPTY, ulTimeout))
  {
    uint32_t ulHostFlags = (ptCtl->ulHostFlags + 1) & ptMailbox->ulBitmask;
    uint32_t ulElementIdx = ulHostFlags & ~HIL_HIF_MBX_WRAPAROUND;

    if (ulElementIdx >= ptMailbox->ulElementCnt)
    {
      ulHostFlags = ulHostFlags ^ HIL_HIF_MBX_WRAPAROUND;
      ulHostFlags = ulHostFlags & HIL_HIF_MBX_WRAPAROUND;
      ulElementIdx = 0;
    }

    /* Copy packet to mailbox */
    ++ptMailbox->ulTransmissionCnt;
    HWIF_WRITEN(ptChannel->pvDeviceInstance,
      ptMailbox->pbBlockStart + ulElementIdx * ptMailbox->ulElementSize,
      ptSendPkt,
      LE32_TO_HOST(ptSendPkt->tHeader.ulLen) + HIL_PACKET_HEADER_SIZE);

    /* Lock flag access */
    OS_EnterLock(ptChannel->pvLock);

    /* Signal new packet */
    DEV_WriteCell(ptChannel, &ptChannel->tFromHostMbx.tCom, ulHostFlags);

    /* Unlock flag access */
    OS_LeaveLock(ptChannel->pvLock);

    lRet = CIFX_NO_ERROR;
  }

  return lRet;
}

/*****************************************************************************/
/*! Sends a Packet to the device/channel
*   \param ptChannel    Channel instance
*   \param ptSendPkt    Packet to send
*   \param ulTimeout    Maximum time in ms to wait for an empty mailbox
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_PutPacket(PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout)
{
  int32_t lRet = CIFX_INVALID_PARAMETER;

  if (ptChannel->fIsSysDevice)
    lRet = DEV_PutPacketSys(ptChannel, ptSendPkt, ulTimeout);
  else
    lRet = DEV_PutPacketCom(ptChannel, ptSendPkt, ulTimeout);

  return lRet;
}

/*****************************************************************************/
/*! Retrieves a Packet from the system channel mailbox
*   \param ptChannel        Channel instance to receive a packet from
*   \param ptRecvPkt        Pointer to place received Packet in
*   \param ulRecvBufferSize Length of the receive buffer
*   \param ulTimeout        Maximum time in ms to wait for an empty mailbox
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_GetPacketSys(PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptRecvPkt, uint32_t ulRecvBufferSize, uint32_t ulTimeout)
{
  int32_t       lRet = CIFX_NO_ERROR;
  uint32_t      ulCopySize = 0;
  CIFX_PACKET* ptPacket = NULL;

  if (!DEV_IsReady(ptChannel))
    return CIFX_DEV_NOT_READY;

  if (!DEV_WaitForBitState(ptChannel, ptChannel->tToHostMbx.tSys.bBitoffset, HIL_FLAGS_NOT_EQUAL, ulTimeout))
    return CIFX_DEV_GET_NO_PACKET;

  ++ptChannel->tToHostMbx.tSys.ulTransmissionCnt;

  ptPacket = (CIFX_PACKET*)ptChannel->tToHostMbx.tSys.pbBlockStart;
  ulCopySize = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptPacket->tHeader.ulLen)) + HIL_PACKET_HEADER_SIZE;
  if (ulCopySize > ulRecvBufferSize)
  {
    /* We have to free the mailbox, read as much as possible */
    ulCopySize = ulRecvBufferSize;
    lRet = CIFX_BUFFER_TOO_SHORT;
  }

  HWIF_READN(ptChannel->pvDeviceInstance, ptRecvPkt, ptPacket, ulCopySize);

  /* Lock flag access */
  OS_EnterLock(ptChannel->pvLock);

  /* Signal read packet done */
  DEV_ToggleBit(ptChannel, ptChannel->tToHostMbx.tSys.ulBitmask);

  /* Unlock flag access */
  OS_LeaveLock(ptChannel->pvLock);

  return lRet;
}

/*****************************************************************************/
/*! Retrieves a Packet from the communication mailbox
*   \param ptChannel        Channel instance to receive a packet from
*   \param ptRecvPkt        Pointer to place received Packet in
*   \param ulRecvBufferSize Length of the receive buffer
*   \param ulTimeout        Maximum time in ms to wait for an empty mailbox
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_GetPacketCom(PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptRecvPkt, uint32_t ulRecvBufferSize, uint32_t ulTimeout)
{
  NETX_BLOCK_T* ptMailbox = &ptChannel->tToHostMbx.tCom.tBlock;
  NETX_HS_CTL_T* ptCtl = &ptChannel->tToHostMbx.tCom.tCtl;
  CIFX_PACKET* ptPacket = NULL;
  uint32_t ulHostFlags = 0;
  uint32_t ulElementIdx = 0;
  uint32_t ulCopySize = 0;
  int32_t lRet = CIFX_NO_ERROR;

  if (!DEV_IsReady(ptChannel))
    return CIFX_DEV_NOT_READY;

  if (!DEV_WaitForMbxState(ptChannel, &ptChannel->tToHostMbx.tCom, NETX_MBX_COM_STATE_FULL, ulTimeout))
    return CIFX_DEV_GET_NO_PACKET;

  ulHostFlags = (ptCtl->ulHostFlags + 1) & ptMailbox->ulBitmask;
  ulElementIdx = ulHostFlags & ~HIL_HIF_MBX_WRAPAROUND;

  if (ulElementIdx >= ptMailbox->ulElementCnt)
  {
    ulHostFlags = ulHostFlags ^ HIL_HIF_MBX_WRAPAROUND;
    ulHostFlags = ulHostFlags & HIL_HIF_MBX_WRAPAROUND;
    ulElementIdx = 0;
  }

  ptPacket = (CIFX_PACKET*)(ptMailbox->pbBlockStart + ulElementIdx * ptMailbox->ulElementSize);
  ulCopySize = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptPacket->tHeader.ulLen)) + HIL_PACKET_HEADER_SIZE;
  if (ulCopySize > ulRecvBufferSize)
  {
    /* We have to free the mailbox, read as much as possible */
    ulCopySize = ulRecvBufferSize;
    lRet = CIFX_BUFFER_TOO_SHORT;
  }
  HWIF_READN(ptChannel->pvDeviceInstance, ptRecvPkt, ptPacket, ulCopySize);

  /* Lock flag access */
  OS_EnterLock(ptChannel->pvLock);

  /* Signal read packet done */
  DEV_WriteCell(ptChannel, &ptChannel->tToHostMbx.tCom, ulHostFlags);

  /* Unlock flag access */
  OS_LeaveLock(ptChannel->pvLock);

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
CIFX_STATIC int32_t DEV_GetPacket(PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptRecvPkt, uint32_t ulRecvBufferSize, uint32_t ulTimeout)
{
  int32_t lRet = CIFX_INVALID_PARAMETER;

  if (ptChannel->fIsSysDevice)
    lRet = DEV_GetPacketSys(ptChannel, ptRecvPkt, ulRecvBufferSize, ulTimeout);
  else
    lRet = DEV_GetPacketCom(ptChannel, ptRecvPkt, ulRecvBufferSize, ulTimeout);

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
CIFX_STATIC int32_t DEV_TransferPacket(void*                 pvChannel,
                                       CIFX_PACKET*          ptSendPkt,
                                       CIFX_PACKET*          ptRecvPkt,
                                       uint32_t              ulRecvBufferSize,
                                       uint32_t              ulTimeout,
                                       PFN_RECV_PKT_CALLBACK pvPktCallback,
                                       void*                 pvUser)
{
  PCHANNELINSTANCE  ptChannel = (PCHANNELINSTANCE)pvChannel;
  int32_t           lCount = 0;
  int32_t           lRet = CIFX_NO_ERROR;

  if ((lRet = DEV_PutPacket(ptChannel, ptSendPkt, ulTimeout)) == CIFX_NO_ERROR)
  {
    do
    {
      if ((lRet = DEV_GetPacket(ptChannel, ptRecvPkt, ulRecvBufferSize, ulTimeout)) == CIFX_NO_ERROR)
      {
        /* Check if we got the answer */
        if (((LE32_TO_HOST(ptRecvPkt->tHeader.ulCmd) & ~HIL_MSK_PACKET_ANSWER) == LE32_TO_HOST(ptSendPkt->tHeader.ulCmd)) &&
          (ptRecvPkt->tHeader.ulSrc == ptSendPkt->tHeader.ulSrc) &&
          (ptRecvPkt->tHeader.ulId == ptSendPkt->tHeader.ulId) &&
          (ptRecvPkt->tHeader.ulSrcId == ptSendPkt->tHeader.ulSrcId))
        {
          /* We got the answer message */
          /* lRet = ptRecvPkt->tHeader.ulState; */ /* Do not deliver back this information */
          break;
        }
        else
        {
          /* This is not our packet, check if the user wants it */
          if (NULL != pvPktCallback)
          {
            pvPktCallback(ptRecvPkt, pvUser);
          }
        }
        /* Reset error, in case we might drop out of the loop, with no proper answer,
           returning a "good" state */
        lRet = CIFX_DEV_GET_TIMEOUT;
        lCount++;
      }
      else
      {
        /* Error during packet receive */
        break;
      }
    } while (lCount < 10);
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

  } else if( !(ptChannel->tHsCtrl.ulNetxFlags & HIL_HIF_NCF_RUN))
  {
    *plError = CIFX_DEV_NOT_RUNNING;

  } else if ( ptChannel->tHsCtrl.ulNetxFlags & HIL_HIF_NCF_COMMUNICATING)
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
/*! Returns the fill level of the communication mailbox
*   \param ptInst  Pointer to the mailbox instance
*   \return Fill level of the mailbox                                        */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_GetMBXFillLevel(NETX_MAILBOX_BLOCK_T* ptInst)
{
  uint32_t ulReqHsk = ptInst->tCtl.ulHostFlags & ptInst->tBlock.ulBitmask;
  uint32_t ulCnfHsk = ptInst->tCtl.ulNetxFlags & ptInst->tBlock.ulBitmask;
  uint32_t ulLevel  = 0;

  if (ulReqHsk != ulCnfHsk)
  {
    uint32_t ulReqIdx = ulReqHsk & ~HIL_HIF_MBX_WRAPAROUND;
    uint32_t ulCnfIdx = ulCnfHsk & ~HIL_HIF_MBX_WRAPAROUND;

    if (ulReqIdx != ulCnfIdx)
      ulLevel = abs(ulReqIdx - ulCnfIdx);
    else
      ulLevel = ptInst->tBlock.ulElementCnt;
  }

  return ulLevel;
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

  } else if (ptChannel->fIsSysDevice)
  {
    /* Check if device is READY */
    if(!DEV_IsReady(ptChannel))
    {
      lRet = CIFX_DEV_NOT_READY;
    } else
    {
      NETX_MAILBOX_AREA_U* ptRecvBlock = &ptChannel->tToHostMbx;
      NETX_MAILBOX_AREA_U* ptSendBlock = &ptChannel->tFromHostMbx;
      uint32_t ulHostFlags = 0;
      uint32_t ulNetxFlags = 0;

      DEV_ReadHandshakeFlags(ptChannel, 0, 0);

      /* Get receive MBX state */
      ulHostFlags    = ptChannel->tHsCtrl.ulHostFlags & ptRecvBlock->tSys.ulBitmask;
      ulNetxFlags    = ptChannel->tHsCtrl.ulNetxFlags & ptRecvBlock->tSys.ulBitmask;
      *pulRecvPktCnt = ulHostFlags != ulNetxFlags;

      /* Get send MBX state */
      ulHostFlags    = ptChannel->tHsCtrl.ulHostFlags & ptSendBlock->tSys.ulBitmask;
      ulNetxFlags    = ptChannel->tHsCtrl.ulNetxFlags & ptSendBlock->tSys.ulBitmask;
      *pulSendPktCnt = ulHostFlags == ulNetxFlags;
    }
  } else
  {
    /* Check if device is READY */
    if(!DEV_IsReady(ptChannel))
    {
      lRet = CIFX_DEV_NOT_READY;
    } else
    {
      NETX_MAILBOX_AREA_U* ptRecvBlock = &ptChannel->tToHostMbx;
      NETX_MAILBOX_AREA_U* ptSendBlock = &ptChannel->tFromHostMbx;

      DEV_ReadHandshakeFlags(ptChannel, 0, 0);

      *pulRecvPktCnt = DEV_GetMBXFillLevel(&ptRecvBlock->tCom);
      *pulSendPktCnt = ptSendBlock->tCom.tBlock.ulElementCnt - DEV_GetMBXFillLevel(&ptSendBlock->tCom);
    }
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
    PDEVICEINSTANCE ptDevInstance = (PDEVICEINSTANCE) ptChannel->pvDeviceInstance;
    HIL_HIF_SYSTEM_WATCHDOG_BLOCK_T* ptWatchdog =
        (HIL_HIF_SYSTEM_WATCHDOG_BLOCK_T*) &ptDevInstance->pbDPM[HIL_OFFSETOF(HIL_HIF_SYSTEM_CHANNEL_T, tSystemWatchdog)];
    lRet = CIFX_NO_ERROR;

    /* Process command */
    if(ulTriggerCmd == CIFX_WATCHDOG_START)
    {
      /* Copy host value to device value */
      *pulTriggerValue = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptWatchdog->ulHostWatchdog));
      HWIF_WRITE32(ptChannel->pvDeviceInstance, ptWatchdog->ulDeviceWatchdog, HOST_TO_LE32(*pulTriggerValue));

    } else if(ulTriggerCmd == CIFX_WATCHDOG_STOP)
    {
      /* Stop watchdog function */
      HWIF_WRITE32(ptChannel->pvDeviceInstance, ptWatchdog->ulDeviceWatchdog, 0);
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
/*! Send a packet to the device to change the bus state
*   \param ptChannel  Channel instance
*   \param ulState    State to change to
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t DEV_DoBusStateChange(PCHANNELINSTANCE ptChannel, uint32_t ulState)
{
  PDEVICEINSTANCE           ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  HIL_START_STOP_COMM_REQ_T tSendPkt;
  HIL_START_STOP_COMM_CNF_T tRecvPkt;
  int32_t                   lRet          = CIFX_NO_ERROR;

  OS_Memset(&tSendPkt, 0, sizeof(tSendPkt));
  OS_Memset(&tRecvPkt, 0, sizeof(tRecvPkt));

  /* Read firmware information */
  tSendPkt.tHead.ulDest   = HOST_TO_LE32(HIL_PACKET_DEST_DEFAULT_CHANNEL);
  tSendPkt.tHead.ulSrc    = HOST_TO_LE32(ptDevInstance->ulPhysicalAddress);
  tSendPkt.tHead.ulCmd    = HOST_TO_LE32(HIL_START_STOP_COMM_REQ);
  tSendPkt.tHead.ulLen    = HOST_TO_LE32(sizeof(tSendPkt.tData));
  tSendPkt.tData.ulParam  = HOST_TO_LE32(ulState);

  /* Transfer packet */
  lRet = DEV_TransferPacket(ptChannel,
                            (CIFX_PACKET*)&tSendPkt,
                            (CIFX_PACKET*)&tRecvPkt,
                            sizeof(tRecvPkt),
                            CIFX_TO_SEND_PACKET,
                            NULL,
                            NULL);

  if( (CIFX_NO_ERROR  != lRet) ||
      (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(tRecvPkt.tHead.ulSta))) )
  {
    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_WARNING)
    {
      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_WARNING,
                "Error changing bus state! (lRet=0x%08X)",
                lRet);
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Handle the application BUS state COS flag
*   \param ptChannel        Channel instance
*   \param ulCmd            New state to set (CIFX_BUS_STATE_ON / CIFX_BUS_STATE_OFF)
*   \param pulState         Buffer to store actual state
*   \param ulTimeout        Timeout to wait for communication to start/stop
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_BusState(PCHANNELINSTANCE ptChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout)
{
  int32_t lRet = CIFX_NO_ERROR;

  UNREFERENCED_PARAMETER(ulTimeout);

  if( NULL == pulState) return CIFX_INVALID_POINTER;

  /* Read actual BUS state */
  *pulState = (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->ulCommunicationState)) & HIL_HIF_COMM_STATE_BUS_ON) ?
              CIFX_BUS_STATE_ON : CIFX_BUS_STATE_OFF;

  switch (ulCmd)
  {
    case CIFX_BUS_STATE_ON:
    {
      /* Check if the BUS is already ON */
      (void)DEV_IsCommunicating(ptChannel, &lRet); /* lRet evaluated */

      if( !*pulState &&
          (CIFX_DEV_NO_COM_FLAG == lRet) )
      {
        int32_t lTemp = DEV_DoBusStateChange(ptChannel, HIL_START_STOP_COMM_PARAM_START);

        /* Only update return value, if packet transfer did not succeed, so
           we can wait for COM_BIT below */
        if(lTemp != CIFX_NO_ERROR)
          lRet = lTemp;
      }

      *pulState = (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->ulCommunicationState)) & HIL_HIF_COMM_STATE_BUS_ON) ?
                  CIFX_BUS_STATE_ON : CIFX_BUS_STATE_OFF;
    }
    break;

    case CIFX_BUS_STATE_OFF:
    {
      /* Check if the BUS is off */
      if(!DEV_IsReady(ptChannel))
      {
        lRet = CIFX_DEV_NOT_READY;

      } else if(*pulState || DEV_IsCommunicating(ptChannel, &lRet))
      {
        lRet = DEV_DoBusStateChange(ptChannel, HIL_START_STOP_COMM_PARAM_STOP);

        *pulState = (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->ulCommunicationState)) & HIL_HIF_COMM_STATE_BUS_ON) ?
                    CIFX_BUS_STATE_ON : CIFX_BUS_STATE_OFF;
      }
    }
    break;

    case CIFX_BUS_STATE_GETSTATE:
    {
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
/*! Performs a channel initialization
*   \param ptChannel Channel instance
*   \param ulTimeout Timeout to wait for channel to become READY
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t DEV_DoChannelInit(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout)
{
  int32_t                lRet          = CIFX_NO_ERROR;
  PDEVICEINSTANCE        ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  int                    fRunning      = DEV_IsRunning(ptChannel);
  HIL_CHANNEL_INIT_REQ_T tSendPkt;
  HIL_CHANNEL_INIT_CNF_T tRecvPkt;

  OS_Memset(&tSendPkt, 0, sizeof(tSendPkt));
  OS_Memset(&tRecvPkt, 0, sizeof(tRecvPkt));

  /* Read firmware information */
  tSendPkt.tHead.ulDest = HOST_TO_LE32(HIL_PACKET_DEST_DEFAULT_CHANNEL);
  tSendPkt.tHead.ulSrc  = HOST_TO_LE32(ptDevInstance->ulPhysicalAddress);
  tSendPkt.tHead.ulCmd  = HOST_TO_LE32(HIL_CHANNEL_INIT_REQ);
  tSendPkt.tHead.ulLen  = 0;

  /* Transfer packet */
  lRet = DEV_TransferPacket(ptChannel,
                            (CIFX_PACKET*)&tSendPkt,
                            (CIFX_PACKET*)&tRecvPkt,
                            sizeof(tRecvPkt),
                            CIFX_TO_SEND_PACKET,
                            NULL,
                            NULL);

  if( (CIFX_NO_ERROR  != lRet) ||
      (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(tRecvPkt.tHead.ulSta))) )
  {
    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_WARNING)
    {
      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_WARNING,
                "Error executing channel initialization! (lRet=0x%08X)",
                lRet);
    }
  }

  if(CIFX_NO_ERROR == lRet)
  {
    /* The card has recognized the initialization, so we can wait until the card has processed it*/
    /* Card was running before, so wait for running flag to vanish */
    if(fRunning)
    {
      /* Check if the Firmware has removed it's running flag,
         or if it's set now.*/
      if ( 0 == (ptChannel->tHsCtrl.ulNetxFlags & HIL_HIF_NCF_RUN) )
      {
        /* FW already removed it's RUN Flag during Channel Init command sequence. No need to
            wait for running flag to vanish */
        if(g_ulTraceLevel & CIFX_TRACE_LEVEL_DEBUG)
        {
          USER_Trace(ptDevInstance,
                    CIFX_TRACE_LEVEL_DEBUG,
                    "DEV_DoChannelInit(): Firmware removed HIL_COMM_COS_RUN early! Skipping wait for NotRunning-State");
        }
      } else if( !DEV_WaitForNotRunning_Poll( ptChannel, CIFX_TO_WAIT_HW_RESET_ACTIVE))
      {
        lRet = CIFX_DEV_RESET_TIMEOUT;
        if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
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
              USER_Trace(ptDevInstance,
                         CIFX_TRACE_LEVEL_WARNING,
                        "DEV_DoChannelInit(): Channel did not enter READY state during timeout!");
            }
          }
        }
      }
    }
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
  ptDevInstance->tSystemDevice.tHsCtrl.ulHostFlags = 0;
  ptDevInstance->tSystemDevice.tHsCtrl.ulNetxFlags = 0;
  OS_LeaveLock(ptDevInstance->tSystemDevice.pvLock);

  for ( ulIdx = 0; ulIdx < ptDevInstance->ulCommChannelCount; ulIdx++)
  {
    OS_EnterLock(ptDevInstance->pptCommChannels[ulIdx]->pvLock);
    ptDevInstance->pptCommChannels[ulIdx]->tHsCtrl.ulHostFlags      = 0;
    ptDevInstance->pptCommChannels[ulIdx]->tHsCtrl.ulNetxFlags      = 0;
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
*   \param ulParam           Reset parameter
*   \param fWaitOnDevice     Wait on device state if != 0
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t DEV_Reset_Execute(PDEVICEINSTANCE ptDevInstance, uint32_t ulParam, uint8_t fWaitOnDevice)
{
  HIL_FIRMWARE_RESET_REQ_T tSendPkt;
  HIL_FIRMWARE_RESET_CNF_T tRecvPkt;
  int32_t lRet = CIFX_NO_ERROR;

  OS_Memset(&tSendPkt, 0, sizeof(tSendPkt));
  OS_Memset(&tRecvPkt, 0, sizeof(tRecvPkt));

  tSendPkt.tHead.ulDest         = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
  tSendPkt.tHead.ulSrc          = HOST_TO_LE32(ptDevInstance->ulPhysicalAddress);
  tSendPkt.tHead.ulCmd          = HOST_TO_LE32(HIL_FIRMWARE_RESET_REQ);
  tSendPkt.tHead.ulLen          = HOST_TO_LE32(sizeof(tSendPkt.tData));
  tSendPkt.tData.ulResetMode    = HOST_TO_LE32(ulParam);
  tSendPkt.tData.ulTimeToReset  = 0; /* unused */

  /* Transfer packet */
  lRet = DEV_TransferPacket(&ptDevInstance->tSystemDevice,
                            (CIFX_PACKET*)&tSendPkt,
                            (CIFX_PACKET*)&tRecvPkt,
                            sizeof(tRecvPkt),
                            CIFX_TO_SEND_PACKET,
                            NULL,
                            NULL);

  if( (CIFX_NO_ERROR  != lRet) ||
      (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(tRecvPkt.tHead.ulSta))) )
  {
    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_WARNING)
    {
      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_WARNING,
                "Error requesting system reset! (lRet=0x%08X)",
                lRet);
    }
  } else
  {
    if (fWaitOnDevice)
    {
      /* Wait until card has recognized the reset */
      if (!DEV_WaitForNotReady_Poll(&ptDevInstance->tSystemDevice, CIFX_TO_WAIT_HW_RESET_ACTIVE))
        lRet = CIFX_DEV_RESET_TIMEOUT;
    }
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
  PDEVICEINSTANCE             ptDevInstance   = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  PCHANNELINSTANCE            ptSysDevice     = &ptDevInstance->tSystemDevice;
  HIL_HIF_SYSTEM_CHANNEL_T* ptSysChannel    = (HIL_HIF_SYSTEM_CHANNEL_T*)ptSysDevice->pbDPMChannelStart;
  uint32_t                    ulSystemStatus  = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemStatus));
  int32_t                     lRet            = CIFX_NO_ERROR;

  /* Card was running before, so wait for running flag to vanish */
  if(!DEV_IsReady(ptSysDevice))
    return CIFX_DEV_NOT_READY;

  if(!OS_WaitMutex(ptSysDevice->pvInitMutex, CIFX_TO_WAIT_COS_CMD))
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
    uint32_t ulResetParam = HIL_RESET_MODE_COLDSTART | (ulParam & (HIL_RESET_PARAM_MSK|HIL_RESET_FLAG_MSK));

    if ( HIL_SYS_STATUS_IDPM == (HIL_SYS_STATUS_IDPM & ulSystemStatus) &&
         HIL_SYS_STATUS_APP  == (HIL_SYS_STATUS_APP  & ulSystemStatus) )
    {
      /* If we're running with an enabled IDPM and APP CPU, no reset will be executed.
       * We just signal the reset state to the COM CPU by using the HIL_FIRMWARE_RESET_REQ. */

      /* Activate the Reset */
      /* ATTENTION: Do not wait on the device, because the reset will be handled by the secure enclave, */
      /*            and therefore the APP CPU has to signal its readiness to the secure enclave. */
      lRet = DEV_Reset_Execute(ptDevInstance, ulResetParam, 0);

    } else
    {
      /* Prepare reset */
      DEV_Reset_Prepare(ptDevInstance);

      /* Perform the Reset */
      lRet = DEV_Reset_Execute(ptDevInstance, ulResetParam, 1);

      if (CIFX_NO_ERROR != lRet)
      {
        ptDevInstance->fResetActive = 0;

        if (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    CIFX_TRACE_LEVEL_ERROR,
                    "DEV_DoSystemStart(): Error waiting for device to leave READY state!");
        }
      }

      /* Now wait for the card to come back */
      if(CIFX_NO_ERROR == lRet)
      {
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
      /* so write meaningful DPM content into log file, to let the user verify current system state                    */
      if(g_ulTraceLevel & CIFX_TRACE_LEVEL_DEBUG)
      {
        char szCookie[5] = {0};

        HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);
        /* split messages for better readability */
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, "DEV_DoSystemStart(): (system status after reset)");
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -DPM-Cookie    : '%02X','%02X','%02X','%02X'",
                   szCookie[0], szCookie[1], szCookie[2], szCookie[3]);
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -System Status : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemStatus)) );
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -System Error  : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemError)) );
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -Boot Error    : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulBootError)) );
      }
    }

    OS_ReleaseMutex(ptSysDevice->pvInitMutex);
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
  PDEVICEINSTANCE             ptDevInstance  = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  PCHANNELINSTANCE            ptSysDevice    = &ptDevInstance->tSystemDevice;
  HIL_HIF_SYSTEM_CHANNEL_T* ptSysChannel   = (HIL_HIF_SYSTEM_CHANNEL_T*)ptSysDevice->pbDPMChannelStart;
  uint32_t                    ulSystemStatus = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemStatus));
  int32_t                     lRet           = CIFX_NO_ERROR;

  /* Card was running before, so wait for running flag to vanish */
  if(!DEV_IsReady(ptSysDevice))
    return CIFX_DEV_NOT_READY;

  if(!OS_WaitMutex(ptSysDevice->pvInitMutex, CIFX_TO_WAIT_COS_CMD))
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
    uint32_t ulResetParam = HIL_RESET_MODE_BOOTSTART | (ulParam & (HIL_RESET_PARAM_MSK|HIL_RESET_FLAG_MSK));

    if ( HIL_SYS_STATUS_IDPM == (HIL_SYS_STATUS_IDPM & ulSystemStatus) &&
         HIL_SYS_STATUS_APP  == (HIL_SYS_STATUS_APP  & ulSystemStatus) )
    {
      /* If we're running with an enabled IDPM and APP CPU, no reset will be executed.
       * We just signal the reset state to the COM CPU by using the HIL_FIRMWARE_RESET_REQ. */

      /* Activate the Reset (including BOOTSTART bit) */
      /* ATTENTION: Do not wait on the device, because the reset will be handled by the secure enclave, */
      /*            and therefore the APP CPU has to signal its readiness to the secure enclave. */
      lRet = DEV_Reset_Execute(ptDevInstance, ulResetParam, 0);

    } else
    {
      /* Prepare reset */
      DEV_Reset_Prepare(ptDevInstance);

      /* Perform the Reset (including BOOTSTART bit) */
      lRet = DEV_Reset_Execute(ptDevInstance, (uint8_t)(HSF_RESET | HSF_BOOTSTART), 1);

      if (CIFX_NO_ERROR != lRet)
      {
        ptDevInstance->fResetActive = 0;

        if (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    CIFX_TRACE_LEVEL_ERROR,
                    "DEV_DoSystemBootstart(): Error waiting for device to leave READY state!");
        }
      }

      /* Now wait for the card to come back */
      if(CIFX_NO_ERROR == lRet)
      {
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
          char szCookie[5] = {0};

          /* Read the DPM cookie */
          HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);

          /* On DPM cards we need to check for a valid cookie */
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
    OS_ReleaseMutex(ptSysDevice->pvInitMutex);
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
  PDEVICEINSTANCE             ptDevInstance  = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  PCHANNELINSTANCE            ptSysDevice    = &ptDevInstance->tSystemDevice;
  HIL_HIF_SYSTEM_CHANNEL_T*   ptSysChannel   = (HIL_HIF_SYSTEM_CHANNEL_T*)ptSysDevice->pbDPMChannelStart;
  uint32_t                    ulSystemStatus = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemStatus));
  int32_t                     lRet           = CIFX_NO_ERROR;

  /* Card was running before, so wait for running flag to vanish */
  if(!DEV_IsReady(ptSysDevice))
    return CIFX_DEV_NOT_READY;

  if(!OS_WaitMutex(ptSysDevice->pvInitMutex, CIFX_TO_WAIT_COS_CMD))
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
    uint32_t ulResetParam = HIL_RESET_MODE_UPDATESTART | (ulParam & (HIL_RESET_PARAM_MSK|HIL_RESET_FLAG_MSK));

    if ( HIL_SYS_STATUS_IDPM == (HIL_SYS_STATUS_IDPM & ulSystemStatus) &&
         HIL_SYS_STATUS_APP  == (HIL_SYS_STATUS_APP  & ulSystemStatus) )
    {
      /* If we're running with an enabled IDPM and APP CPU, no reset will be executed.
       * We just signal the reset state to the COM CPU by using the HIL_FIRMWARE_RESET_REQ. */

      /* Activate the Reset (including BOOTSTART bit) */
      /* ATTENTION: Do not wait on the device, because the reset will be handled by the secure enclave, */
      /*            and therefore the APP CPU has to signal its readiness to the secure enclave. */
      lRet = DEV_Reset_Execute(ptDevInstance, ulResetParam, 0);

    } else
    {
      char szCookie[5] = {0};

      /* Prepare reset */
      DEV_Reset_Prepare(ptDevInstance);

      /* Perform the Reset */
      lRet = DEV_Reset_Execute(ptDevInstance, ulResetParam, 1);

      if (CIFX_NO_ERROR != lRet)
      {
        ptDevInstance->fResetActive = 0;

        if (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    CIFX_TRACE_LEVEL_ERROR,
                    "DEV_DoUpdateStart(): Error waiting for device to leave READY state!");
        }
      }

      /* Now wait for the card to come back */
      if(CIFX_NO_ERROR == lRet)
      {
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
          /* Check if the firmware is running */
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
            /* This is an updatestart, expected one additional reset to be performed by the firmware */
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
                   szCookie[0], szCookie[1], szCookie[2], szCookie[3]);
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -System Status : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemStatus)) );
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -System Error  : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemError)) );
        USER_Trace(ptDevInstance, CIFX_TRACE_LEVEL_DEBUG, " -Boot Error    : 0x%X",
                   LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulBootError)) );
      }
    }

    OS_ReleaseMutex(ptSysDevice->pvInitMutex);
  }

  return lRet;
}

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
        uint32_t ulFileNameLength = HIL_MIN( ((uint32_t)OS_Strlen(szFileName) + 1),
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
        ulCopySize  = HIL_MIN( (sizeof(uSendPkt.tPacket.abData) - sizeof(uSendPkt.tDownloadReq.tData)), uSendPkt.tDownloadReq.tData.usFileNameLength);

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
*   \param ulMailbox          Mailbox to send a packet (NETX_MBX_SYS/COM)
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
  }                               uSendPkt;

  union
  {
    CIFX_PACKET                   tPacket;
    HIL_FILE_UPLOAD_CNF_T         tUploadCnf;
    HIL_FILE_UPLOAD_DATA_CNF_T    tUploadDataCnf;
  }                               uRecvPkt;

  char*            pbCopyPtr      = NULL;
  uint32_t         ulCopySize     = 0;
  uint32_t         ulFileLength   = 0;
  uint16_t         usFilenameLen  = (uint16_t)(OS_Strlen(szFileName) + 1); /*Firmware expects length including terminating NULL */
  uint32_t         ulBlockSize    = ulMailboxSize -
                                    (uint32_t)sizeof(uRecvPkt.tUploadDataCnf); /* maximum size of each file block */
  int              fSendAbort     = 0;
  int32_t          lRetAbort      = CIFX_NO_ERROR;
  int32_t          lRet           = CIFX_NO_ERROR;
  uint32_t         ulCurrentId    = 0;
  uint32_t         ulSrc          = OS_GetMilliSecCounter(); /* Early versions used pvChannel as ulSrc,
                                                                but this won't work on 64 Bit machines.
                                                                As we need something unique we use the current system time */

  OS_Memset(&uSendPkt, 0, sizeof(uSendPkt));
  OS_Memset(&uRecvPkt, 0, sizeof(uRecvPkt));

  /* Check parameters */
  if( (NULL == pvData) || (NULL == pulDataBufferLen) )
    return CIFX_INVALID_POINTER;

  if( ulMailboxSize < HIL_HIF_SYSTEM_MAILBOX_MIN_SIZE)
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
  ulCopySize  = HIL_MIN( (sizeof(uSendPkt.tPacket.abData) - sizeof(uSendPkt.tUploadReq.tData)), uSendPkt.tUploadReq.tData.usFileNameLength);

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

  /* Get the DMA control registers */
  ulChannelNumber = ptChannel->ulChannelNumber ;
  ulDMAChIdx      = ulChannelNumber * 2; /* 2 DMA channels per communication channel */

  /* Get the corresponding netX DMA control register */
  pDMACtrl_1      = &ptDevInstance->pvGlobalRegisters->atDmaCtrl[ulDMAChIdx + eDMA_INPUT_BUFFER_IDX]; /* Input channel */
  pDMACtrl_2      = &ptDevInstance->pvGlobalRegisters->atDmaCtrl[ulDMAChIdx + eDMA_OUTPUT_BUFFER_IDX];/* Output channel */

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
  *pulState = (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->ulCommunicationCOS)) & HIL_COMM_COS_DMA) ?
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

#if 0 // TODO
        /* DMA is OFF, signal new DMA state */
        lRet = DEV_DoHostCOSChange(ptChannel,
                                   HIL_APP_COS_DMA | HIL_APP_COS_DMA_ENABLE,  /* set mask        */
                                   0,                                         /* clear mask      */
                                   HIL_APP_COS_DMA_ENABLE,                    /* post clear mask */
                                   CIFX_DEV_DMA_STATE_ON_TIMEOUT,
                                   CIFX_TO_WAIT_COS_ACK);                     /* Alwas wait for the card ACK */

        /* Read actual state */
        *pulState = (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->ulCommunicationCOS)) & HIL_COMM_COS_DMA) ?
                     CIFX_DMA_STATE_ON : CIFX_DMA_STATE_OFF;
#endif
      }
    }
    break;

    case CIFX_DMA_STATE_OFF:
    {
      /* Check if the DMA is already OFF */
      if(CIFX_DMA_STATE_OFF != *pulState)
      {
#if 0 // TODO
        /* DMA is ON, signal new DMA state */
        lRet = DEV_DoHostCOSChange(ptChannel,
                                   HIL_APP_COS_DMA_ENABLE,            /* set mask        */
                                   HIL_APP_COS_DMA,                   /* clear mask      */
                                   HIL_APP_COS_DMA_ENABLE,            /* post clear mask */
                                   CIFX_DEV_DMA_STATE_OFF_TIMEOUT,
                                   CIFX_TO_WAIT_COS_ACK);             /* Alwas wait for the card ACK */

        /* Read actual state */
        *pulState = (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->ulCommunicationCOS)) & HIL_COMM_COS_DMA) ?
                     CIFX_BUS_STATE_ON : CIFX_BUS_STATE_OFF;
#endif
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
static CIFX_DEV_FUNCTION_LIST_T s_tCifxHifDevFuns =
{
  DEV_WriteHandshakeFlags,
  DEV_ReadHostFlags,
  DEV_ReadHandshakeFlags,
  NULL,
  DEV_WaitForBitState,
  DEV_WaitForIoBitState,
  DEV_WaitForMbxState,
  DEV_ToggleBit,
  DEV_ToggleIoAction,
  DEV_WriteCell,
  DEV_WaitForSyncState,
  DEV_ToggleSyncBit,
  DEV_PutPacket,
  DEV_GetPacket,
  DEV_GetMBXState,
  DEV_GetMBXFillLevel,
  DEV_TransferPacket,
  DEV_IsReady,
  DEV_IsRunning,
  DEV_IsCommunicating,
  DEV_WaitForReady_Poll,
  DEV_WaitForNotReady_Poll,
  DEV_WaitForRunning_Poll,
  DEV_WaitForNotRunning_Poll,
  DEV_WaitForLock_Poll,
  DEV_TriggerWatchdog,
  NULL,
  NULL,
  DEV_ReadWriteBlock,
  DEV_DoChannelInit,
  DEV_DoSystemStart,
  DEV_DoSystemBootstart,
  DEV_DoUpdateStart,
  DEV_BusState,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  DEV_DownloadFile,
  DEV_UploadFile,
#ifdef CIFX_TOOLKIT_DMA
  DEV_DMAState,
  DEV_SetupDMABuffers,
#endif
};

PCIFX_DEV_FUNCTION_LIST_T cifXTkitGetHifDevFunctionList(void)
{
  return &s_tCifxHifDevFuns;
}

#endif

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/
