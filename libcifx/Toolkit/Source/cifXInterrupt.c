/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXInterrupt.c 14244 2021-10-18 12:30:23Z RMayer $:

  Description:
    cifX Toolkit Interrupt handling routines (ISR/DSR)

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2021-10-15  - Rework handling in DSR function, added ulHostCOSFlagsSaved variable
    2018-10-10  - Updated header and definitions to new Hilscher defines
                - Derived from cifX Toolkit V1.6.0.0

**************************************************************************************/

#include "cifXHWFunctions.h"
#include "cifXErrors.h"
#include "cifXEndianess.h"

/*****************************************************************************/
/*!  \addtogroup CIFX_TOOLKIT_FUNCS cifX DPM Toolkit specific functions
*    \{                                                                      */
/*****************************************************************************/

/*****************************************************************************/
/*! Low-Level interrupt handler
*   \param ptDevInstance Instance that probably generated an IRQ (on PCI devices
*                        the routine decides if it was an IRQ for shared interrupt lines)
*   \param fPCIIgnoreGlobalIntFlag  Ignore the global interrupt flag on PCI cards,
*                                   to detect shared interrupts. This might be necessary
*                                   if the user has already filtered out all shared IRQs
*   \return CIFX_TKIT_IRQ_DSR_REQUESTED/CIFX_TKIT_IRQ_HANDLED on success
*           CIFX_TKIT_IRQ_OTHERDEVICE if the IRQ is not from the device      */
/*****************************************************************************/
int cifXTKitISRHandler(PDEVICEINSTANCE ptDevInstance, int fPCIIgnoreGlobalIntFlag)
{
  int iRet;

  /* Check if DPM is available, if not, it cannot be our card, that caused the interrupt */
  if( HWIF_READ32(ptDevInstance, *(uint32_t*)ptDevInstance->pbDPM) == CIFX_DPM_INVALID_CONTENT)
    return CIFX_TKIT_IRQ_OTHERDEVICE;

  if(!ptDevInstance->fIrqEnabled)
  {
    /* Irq is disabled on device, so we assume the user activated the interrupts,
       but wants to poll the card. */

    USER_Trace(ptDevInstance,
               TRACE_LEVEL_ERROR,
               "cifXTKitISRHandler() : We received an interrupt, but IRQs are disabled!");

    iRet = CIFX_TKIT_IRQ_OTHERDEVICE;

  } else
  {
    /* We are working in interrupt mode */
    uint32_t                    ulChannel;
    int                         iIrqToDsrBuffer   = ptDevInstance->iIrqToDsrBuffer;
    IRQ_TO_DSR_BUFFER_T*        ptIsrToDsrBuffer  = &ptDevInstance->atIrqToDsrBuffer[iIrqToDsrBuffer];
    HIL_DPM_HANDSHAKE_ARRAY_T*  ptHandshakeBuffer = &ptIsrToDsrBuffer->tHandshakeBuffer;

    /* on a DPM module every handshake cell can be read individually,
       on a PCI module the complete handshake register block must be read sequentially */
    if( (!ptDevInstance->fPCICard) ||
        (!ptDevInstance->pbHandshakeBlock) )
    {
      /* DPM card */

      ++ptDevInstance->ulIrqCounter;
      ptIsrToDsrBuffer->fValid = 1;

      /* Check if we have a handshake block, if so, we read it completely on DPM hardwares
         to make sure, illegally activated handshake cells, don't cause interrupts */
      if (NULL != ptDevInstance->pbHandshakeBlock)
      {
        HWIF_READN( ptDevInstance,
                    ptHandshakeBuffer,
                    ptDevInstance->pbHandshakeBlock,
                    sizeof(*ptHandshakeBuffer));
      } else
      {
        /* We do not have a handshake block, so we have to read them one by one */
        /* and only for the available channels */
        ptHandshakeBuffer->atHsk[0].ulValue = HWIF_READ32(ptDevInstance, ptDevInstance->tSystemDevice.ptHandshakeCell->ulValue);

        for(ulChannel = 0; ulChannel < ptDevInstance->ulCommChannelCount; ++ulChannel)
        {
          PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)ptDevInstance->pptCommChannels[ulChannel];
          uint32_t         ulBlockID = ptChannel->ulBlockID;

          ptHandshakeBuffer->atHsk[ulBlockID].ulValue = HWIF_READ32(ptDevInstance, ptChannel->ptHandshakeCell->ulValue);
        }
      }

      /* we need to check in DSR which handshake bits have changed */
      iRet = CIFX_TKIT_IRQ_DSR_REQUESTED;

    } else
    {
      /* PCI card */

      uint32_t ulIrqState0 = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptDevInstance->ptGlobalRegisters->ulIRQState_0));
      uint32_t ulIrqState1 = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptDevInstance->ptGlobalRegisters->ulIRQState_1));

      /* First check if we have generated this interrupt by reading the global IRQ status bit */
      if(  !fPCIIgnoreGlobalIntFlag &&
          (0 == (ulIrqState0 & MSK_IRQ_STA0_INT_REQ)) )
      {
        /* we have not generated this interrupt, so it must be another device on shared IRQ */
        iRet = CIFX_TKIT_IRQ_OTHERDEVICE;

      } else
      {
        HIL_DPM_HANDSHAKE_ARRAY_T* ptHandshakeBlock = (HIL_DPM_HANDSHAKE_ARRAY_T*)ptDevInstance->pbHandshakeBlock;

        /* confirm all interrupts.
           We can safely clear handshake interrupts here, as we are reading the handshake flags below,
           so we won't miss an IRQ.*/
        HWIF_WRITE32(ptDevInstance, ptDevInstance->ptGlobalRegisters->ulIRQState_0, HOST_TO_LE32(ulIrqState0));
        HWIF_WRITE32(ptDevInstance, ptDevInstance->ptGlobalRegisters->ulIRQState_1, HOST_TO_LE32(ulIrqState1));

        ++ptDevInstance->ulIrqCounter;
        ptIsrToDsrBuffer->fValid = 1;

        /* Only read first 8 Handshake cells, due to a netX hardware issue. Reading flags 8-15 may
           also confirm IRQs for Handshake cell 0-7 due to an netX internal readahead buffer */
        ptHandshakeBuffer->atHsk[HIL_DPM_SYSTEM_CHANNEL_INDEX].ulValue    = HWIF_READ32( ptDevInstance, ptHandshakeBlock->atHsk[HIL_DPM_SYSTEM_CHANNEL_INDEX].ulValue);
        ptHandshakeBuffer->atHsk[HIL_DPM_HANDSHAKE_CHANNEL_INDEX].ulValue = HWIF_READ32( ptDevInstance, ptHandshakeBlock->atHsk[HIL_DPM_HANDSHAKE_CHANNEL_INDEX].ulValue);

        for(ulChannel = 0; ulChannel < ptDevInstance->ulCommChannelCount; ++ulChannel)
          ptHandshakeBuffer->atHsk[HIL_DPM_COM_CHANNEL_START_INDEX + ulChannel].ulValue = HWIF_READ32(ptDevInstance, ptHandshakeBlock->atHsk[HIL_DPM_COM_CHANNEL_START_INDEX + ulChannel].ulValue);

        /* we need to check in DSR which handshake bits have changed */
        iRet = CIFX_TKIT_IRQ_DSR_REQUESTED;
      }
    }
  }

  return iRet;
}

/*****************************************************************************/
/*! Process IO Areas for changes / callbacks
*   \param ptChannel      Channel Instance
*   \param ptIoArea       IO Area
*   \param usChangedBits  Bits that have changed since last IRQ
*   \param usUnequalBits  Bits that are unequal between host and netX
*   \param fOutput        !=0 if an output area is processed                 */
/*****************************************************************************/
static void ProcessIOArea(PCHANNELINSTANCE  ptChannel,
                          PIOINSTANCE       ptIoArea,
                          uint16_t          usChangedBits,
                          uint16_t          usUnequalBits,
                          int               fOutput)
{
  uint16_t usBitMask = (uint16_t)(1 << ptIoArea->bHandshakeBit);

  if(usChangedBits & usBitMask)
  {
    PFN_NOTIFY_CALLBACK pfnCallback = NULL;
    uint8_t             bIOBitState = DEV_GetIOBitstate(ptChannel, ptIoArea, fOutput);

    switch(bIOBitState)
    {
    case HIL_FLAGS_EQUAL:
      if(0 == (usUnequalBits & usBitMask))
        pfnCallback = ptIoArea->pfnCallback;
      break;

    case HIL_FLAGS_NOT_EQUAL:
      if(usUnequalBits & usBitMask)
        pfnCallback = ptIoArea->pfnCallback;
      break;

    case HIL_FLAGS_CLEAR:
      if(0 == (ptChannel->usNetxFlags & usBitMask))
        pfnCallback = ptIoArea->pfnCallback;
      break;

    case HIL_FLAGS_SET:
      if(ptChannel->usNetxFlags & usBitMask)
        pfnCallback = ptIoArea->pfnCallback;
      break;
    }

    if(pfnCallback)
      pfnCallback(ptIoArea->ulNotifyEvent, 0, NULL, ptIoArea->pvUser);

    OS_SetEvent(ptChannel->ahHandshakeBitEvents[ptIoArea->bHandshakeBit]);
  }
}

/*****************************************************************************/
/*! Deferred interrupt handler
*   \param ptDevInstance Instance the DSR is requested for                   */
/*****************************************************************************/
void cifXTKitDSRHandler(PDEVICEINSTANCE ptDevInstance)
{
  if(!ptDevInstance->fResetActive)
  {
    /* Get actual data buffer index */
    uint32_t              ulChannel        = 0;
    PCHANNELINSTANCE      ptChannel        = &ptDevInstance->tSystemDevice;
    int                   iIrqToDsrBuffer  = 0;
    IRQ_TO_DSR_BUFFER_T*  ptIrqToDsrBuffer = NULL;

#ifdef CIFX_TOOLKIT_ENABLE_DSR_LOCK
    /* Lock against ISR */
    OS_IrqLock(ptDevInstance->pvOSDependent);
#else

    /* ATTENTION: The IrqToDsr Buffer handling implies a "always" higher priority */
    /*            of the ISR function. This does usually happens on physical ISR functions */
    /*            but does not work if the ISR and DSR are handled as a threads! */

#endif

    iIrqToDsrBuffer  = ptDevInstance->iIrqToDsrBuffer;
    ptIrqToDsrBuffer = &ptDevInstance->atIrqToDsrBuffer[iIrqToDsrBuffer];

    if(!ptIrqToDsrBuffer->fValid)
    {
      /* Interrupt did not provide data yet */

#ifdef CIFX_TOOLKIT_ENABLE_DSR_LOCK
      /* Release lock against ISR */
      OS_IrqUnlock(ptDevInstance->pvOSDependent);
#endif

      return;
    } else
    {
      /* Flip data buffer so IRQ uses the other buffer */
      ptDevInstance->iIrqToDsrBuffer ^= 0x01;

      /* Invalidate the buffer, we are now handling */
      ptIrqToDsrBuffer->fValid        = 0;
    }

#ifdef CIFX_TOOLKIT_ENABLE_DSR_LOCK
    /* Release lock against ISR */
    OS_IrqUnlock(ptDevInstance->pvOSDependent);
#endif

    /* Only process rest of flags if NSF_READY is set. This must be done to prevent
       confusion of the toolkit during a system start (xSysdeviceReset) */
    if(ptIrqToDsrBuffer->tHandshakeBuffer.atHsk[0].t8Bit.bNetxFlags & NSF_READY)
    {
      /*--------------------------------------------------------------------*/
      /* Evaluate device synchronisation flags, the flags are fixed 16 Bit  */
      /*--------------------------------------------------------------------*/
      uint16_t  usChangedSyncBits;
      uint16_t  usOldNSyncFlags = ptDevInstance->tSyncData.usNSyncFlags; /* Remember last known netX flags */

      /* Get pointer to the new flag data from ISR */
      HIL_DPM_HANDSHAKE_CELL_T*  ptSyncCell  = &ptIrqToDsrBuffer->tHandshakeBuffer.atHsk[NETX_HSK_SYNCH_FLAG_POS];

      /* Get the actual flags */
      ptDevInstance->tSyncData.usNSyncFlags = LE16_TO_HOST(ptSyncCell->t16Bit.usNetxFlags);

      /* Check if there are changed bits since last interrupt from netX side,  */
      /* and only process sync if bits have chanded! */
      if ( 0 != (usChangedSyncBits = usOldNSyncFlags ^ ptDevInstance->tSyncData.usNSyncFlags))
      {
        uint32_t  ulBitPos;
        uint16_t  usUnequalSyncBits;

        /* Create unequal bit mask */
        usUnequalSyncBits = ptDevInstance->tSyncData.usNSyncFlags ^ ptDevInstance->tSyncData.usHSyncFlags;

        /* Signal sync events */
        for(ulBitPos = 0; ulBitPos < NETX_NUM_OF_SYNCH_FLAGS; ++ulBitPos)
        {
          /* There is a valid channel */
          uint16_t          usBitMask     = (uint16_t)(1 << ulBitPos);
          PCHANNELINSTANCE  ptSyncChannel = NULL;

          if (ulBitPos >= ptDevInstance->ulCommChannelCount)
            break;

          ptSyncChannel = (PCHANNELINSTANCE)ptDevInstance->pptCommChannels[ulBitPos];

          if ( usChangedSyncBits & usBitMask)
          {
            uint8_t bState   = HIL_FLAGS_NOT_EQUAL;
            int     fProcess = 0;

            /* Handle Sync interrupts, read actual state and set bState accordingly */
            if( HIL_SYNC_MODE_HST_CTRL == HWIF_READ8(ptDevInstance, ptSyncChannel->ptCommonStatusBlock->bSyncHskMode))
              bState = HIL_FLAGS_EQUAL;

            /* Check which mode to handle */
            /* HIL_FLAGS_NOT_EQUAL corresponds to DEVICE_CONTROLLED */
            if( (bState == HIL_FLAGS_NOT_EQUAL) &&
                (usUnequalSyncBits & usBitMask) )
            {
              fProcess = 1;

            } else if( (bState == HIL_FLAGS_EQUAL) &&
                       (0 == (usUnequalSyncBits & usBitMask)) )
            {
              fProcess = 1;
            }

            if(fProcess)
            {
              /* There is a valid channel */
              /* Check if we have a callback assigned */
              if (ptSyncChannel->tSynch.pfnCallback)
                ptSyncChannel->tSynch.pfnCallback( CIFX_NOTIFY_SYNC, 0, NULL, ptSyncChannel->tSynch.pvUser);

              /* Signal event to allow waiting for sync state without callback */
              if( ptDevInstance->tSyncData.ahSyncBitEvents[ulBitPos])
                OS_SetEvent(ptDevInstance->tSyncData.ahSyncBitEvents[ulBitPos]);
            }
          }
        }
      }

      /*-----------------------------------------------------*/
      /* Evaluate all changed handshake bits on all channels */
      /* Start with SYSTEM channel                           */
      do
      {
        uint16_t usChangedBits;
        uint16_t usUnequalBits;
        uint16_t usOldNetxFlags = ptChannel->usNetxFlags; /* Remember last known netX flags */
        uint32_t ulIdx;

        /* Address the handshake cell */
        HIL_DPM_HANDSHAKE_CELL_T* ptHskCell = &ptIrqToDsrBuffer->tHandshakeBuffer.atHsk[ptChannel->ulBlockID];

        if(ptChannel->bHandshakeWidth == HIL_HANDSHAKE_SIZE_8BIT)
        {
          ptChannel->usNetxFlags = ptHskCell->t8Bit.bNetxFlags;
        } else
        {
          ptChannel->usNetxFlags = LE16_TO_HOST(ptHskCell->t16Bit.usNetxFlags);
        }

        /* Check which bits have changed since last interrupt from netX side */
        usChangedBits = usOldNetxFlags ^ ptChannel->usNetxFlags;
        usUnequalBits = ptChannel->usNetxFlags ^ ptChannel->usHostFlags;

        /* Check if we have a valid channel (not for the bootloader) */
        if(ptChannel->fIsChannel)
        {
          /*------------------------------------------*/
          /* Process CHANNEL flags                    */
          /*------------------------------------------*/

          /* -----------------------------------------*/
          /* Check COM Flag and I/O areas             */
          /* -----------------------------------------*/
          if(usChangedBits & NCF_COMMUNICATING)
          {
            OS_SetEvent(ptChannel->ahHandshakeBitEvents[NCF_COMMUNICATING_BIT_NO]);

            /* check if notification is registered */
            if (NULL != ptChannel->tComState.pfnCallback)
            {
              CIFX_NOTIFY_COM_STATE_T tData;

              tData.ulComState = (ptChannel->usNetxFlags & NCF_COMMUNICATING);

              ptChannel->tComState.pfnCallback( CIFX_NOTIFY_COM_STATE,
                                                sizeof(tData),
                                                &tData,
                                                ptChannel->tComState.pvUser);
            }
          }

          /* Check IO - Input Areas */
          for(ulIdx = 0; ulIdx < ptChannel->ulIOInputAreas; ++ulIdx)
          {
            ProcessIOArea(ptChannel,
                          ptChannel->pptIOInputAreas[ulIdx],
                          usChangedBits,
                          usUnequalBits,
                          0);
          }

          /* Check IO - Output Areas */
          for(ulIdx = 0; ulIdx < ptChannel->ulIOOutputAreas; ++ulIdx)
          {
            ProcessIOArea(ptChannel,
                          ptChannel->pptIOOutputAreas[ulIdx],
                          usChangedBits,
                          usUnequalBits,
                          1);
          }

          /* -----------------------------------------*/
          /* Check COS Flags                          */
          /* -----------------------------------------*/
          /* Check netX for new COS flags             */
          /* -----------------------------------------*/
          if( usUnequalBits & NCF_NETX_COS_CMD)
          {
            uint32_t ulNewCOSFlags = 0;

            /* Lock flag access */
            OS_EnterLock(ptChannel->pvLock);

            /* Read the flags and acknowledge them */
            ulNewCOSFlags = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptChannel->ptCommonStatusBlock->ulCommunicationCOS));

            /* Check if they have changed */
            if(ptChannel->ulDeviceCOSFlags != ulNewCOSFlags)
            {
              ptChannel->ulDeviceCOSFlagsChanged  = ptChannel->ulDeviceCOSFlags ^ ulNewCOSFlags;
              ptChannel->ulDeviceCOSFlags         = ulNewCOSFlags;
            }

            DEV_ToggleBit(ptChannel, HCF_NETX_COS_ACK);

            /* Unlock flag access */
            OS_LeaveLock(ptChannel->pvLock);
          }

          /* Signal netX COS command flag change */
          if( usChangedBits & NCF_NETX_COS_CMD)
            OS_SetEvent(ptChannel->ahHandshakeBitEvents[NCF_NETX_COS_CMD_BIT_NO]);

          /* --------------------------------------------------*/
          /* Process our own COS flags (Write them to device)  */
          /* --------------------------------------------------*/
          /* Check if we have new COS flags to write */
          if ( ptChannel->ulHostCOSFlagsSaved != ptChannel->ulHostCOSFlags)
          {
            /* Check if it is allowed to write new flags */
            if( !(usUnequalBits & NCF_HOST_COS_ACK))
            {
              /* Lock flag access */
              OS_EnterLock(ptChannel->pvLock);

              /* Update COS flags */
              HWIF_WRITE32(ptDevInstance, ptChannel->ptControlBlock->ulApplicationCOS, HOST_TO_LE32(ptChannel->ulHostCOSFlags));

              /* Store the written values */
              ptChannel->ulHostCOSFlagsSaved = ptChannel->ulHostCOSFlags;

              /* Signal new COS flags */
              DEV_ToggleBit(ptChannel, HCF_HOST_COS_CMD);

              /* Remove all enable flags from the local COS flags */
              ptChannel->ulHostCOSFlags &= ~(HIL_APP_COS_BUS_ON_ENABLE | HIL_APP_COS_INITIALIZATION_ENABLE | HIL_APP_COS_LOCK_CONFIGURATION_ENABLE);

              /* Unlock flag access */
              OS_LeaveLock(ptChannel->pvLock);
            }
          }

          /* Signal host COS acknowledge flag change */
          if( usChangedBits & NCF_HOST_COS_ACK)
            OS_SetEvent(ptChannel->ahHandshakeBitEvents[NCF_HOST_COS_ACK_BIT_NO]);

        } else
        {
          /*------------------------------------------*/
          /* Process SYSTEM DEVICE Hardware COS flags */
          /*------------------------------------------*/
          /*----------------------------------------------------*/
          /* Check if the hardware signals new system COS flags */
          /*----------------------------------------------------*/
          if( usUnequalBits & NSF_NETX_COS_CMD)
          {
            /* Read the flags and acknowledge them */
            HIL_DPM_SYSTEM_CHANNEL_T* ptSyschannel   = (HIL_DPM_SYSTEM_CHANNEL_T*)ptChannel->pbDPMChannelStart;
            uint32_t                  ulNewCOSFlags  = 0;

            /* Lock flag access */
            OS_EnterLock(ptChannel->pvLock);

            /* Read the actual "System COS" flags */
            ulNewCOSFlags  = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSyschannel->tSystemState.ulSystemCOS));

            /* Read the flags and acknowledge them */
            if(ptChannel->ulDeviceCOSFlags != ulNewCOSFlags)
            {
              ptChannel->ulDeviceCOSFlagsChanged  = ptChannel->ulDeviceCOSFlags ^ ulNewCOSFlags;
              ptChannel->ulDeviceCOSFlags         = ulNewCOSFlags;
            }

            DEV_ToggleBit(ptChannel, HSF_NETX_COS_ACK);

            /* Unlock flag access */
            OS_LeaveLock(ptChannel->pvLock);
          }

            /* Signal COS CMD bits */
          if(usChangedBits & NSF_NETX_COS_CMD)
            OS_SetEvent(ptChannel->ahHandshakeBitEvents[NSF_NETX_COS_CMD_BIT_NO]);

          /* Signal COS ACK bits */
          if(usChangedBits & NSF_HOST_COS_ACK)
            OS_SetEvent(ptChannel->ahHandshakeBitEvents[NSF_HOST_COS_ACK_BIT_NO]);
        }

        /*------------------------------------------*/
        /* Process the send receive MBX flags       */
        /*------------------------------------------*/
        /* Check Receive Mailbox */
        if( usChangedBits & ptChannel->tRecvMbx.ulRecvACKBitmask)
        {
          if( (usUnequalBits & ptChannel->tRecvMbx.ulRecvACKBitmask) &&
              (NULL != ptChannel->tRecvMbx.pfnCallback) )
          {
            CIFX_NOTIFY_RX_MBX_FULL_DATA_T tRxData;

            tRxData.ulRecvCount = LE16_TO_HOST(HWIF_READ16(ptDevInstance, ptChannel->tRecvMbx.ptRecvMailboxStart->usWaitingPackages));

            ptChannel->tRecvMbx.pfnCallback(CIFX_NOTIFY_RX_MBX_FULL,
                                            sizeof(tRxData),
                                            &tRxData,
                                            ptChannel->tRecvMbx.pvUser);
          }
          OS_SetEvent(ptChannel->ahHandshakeBitEvents[ptChannel->tRecvMbx.bRecvACKBitoffset]);
        }

        /* Check Send Mailbox */
        if( usChangedBits & ptChannel->tSendMbx.ulSendCMDBitmask)
        {
          if( (0    == (usUnequalBits & ptChannel->tSendMbx.ulSendCMDBitmask)) &&
              (NULL != ptChannel->tSendMbx.pfnCallback) )
          {
            CIFX_NOTIFY_TX_MBX_EMPTY_DATA_T tTxData;

            tTxData.ulMaxSendCount = LE16_TO_HOST(HWIF_READ16(ptDevInstance, ptChannel->tSendMbx.ptSendMailboxStart->usPackagesAccepted));

            ptChannel->tSendMbx.pfnCallback(CIFX_NOTIFY_TX_MBX_EMPTY,
                                            sizeof(tTxData),
                                            &tTxData,
                                            ptChannel->tSendMbx.pvUser);
          }
          OS_SetEvent(ptChannel->ahHandshakeBitEvents[ptChannel->tSendMbx.bSendCMDBitoffset]);
        }

        /* Next channel */
        if(ulChannel < ptDevInstance->ulCommChannelCount)
          ptChannel = ptDevInstance->pptCommChannels[ulChannel];

        ulChannel++;

      } while(ulChannel <= ptDevInstance->ulCommChannelCount);
    }
  }
}

/*****************************************************************************/
/*!  \}                                                                      */
/*****************************************************************************/
