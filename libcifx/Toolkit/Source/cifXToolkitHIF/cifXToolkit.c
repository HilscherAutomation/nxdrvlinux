/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXToolkit.c 15355 2025-11-28 09:28:41Z AMinor $:

  Description:
    cifX Toolkit Initialization function implementation. This file contains all functions
    that need to be called by the application which wants to use the toolkit, to pass the
    cards that need to be handled and initialize them all. This file also includes the
    functions for downloading the firmware/configuration on startup and bring the card to live.

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-05-26  Update to new DPMv2 handling, removed handling for older chips
    2023-04-27  Added cifXReadHardwareIdent() function, to read netX "ChipType"
    2022-06-14  Added new user function to read IO buffer caching option

    2021-08-31  - Reworked device type and chip typ detection in cifXStartDevice() / cifXHardwareReset()
                - Adapted OS_Time() call to new parameter type definition

    2021-08-13  - Fixed missing return of ulState packet errors in cifXReadFirmwareIdent()
                - Removed "\r\n" from trace strings, now generally handled in USER_Trace()

    2019-10-11  Propagate prototype changes of endianess conversion function

    2019-03-01  Do not write PCIe configuration space during hardware reset of netX4000
                based PCI hardware

    2019-02-12  Skip update handling for flash based netX90/4000 devices

    2018-12-10  Integrate detection function for netX90/4000 for ROMloader and while
                firmware is running (the latter expects register block at end of DPM)

    2018-10-10  - Updated header and definitions to new Hilscher defines
                - Derived from cifX Toolkit V1.6.0.0

    2018-09-24  Reworked startup structure, moved netx500 and netX51 hboot functions
                to own source modules

**************************************************************************************/

#include "cifXToolkit.h"
#include "cifXErrors.h"
#include "cifXEndianess.h"
#include "cifXHWFunctions.h"
#include "cifXHWFunctionsWrapper.h"
#include "USER_Dependent.h"

#include "Hil_Packet.h"
#include "Hil_SystemCmd.h"
#include "Hil_Results.h"

#ifdef CIFX_TOOLKIT_TIME
extern void cifXInitTime(PDEVICEINSTANCE ptDevInstance);
#endif
extern int32_t cifXReadFirmwareIdent(PDEVICEINSTANCE       ptDevInstance,
                                     uint32_t              ulChannel,
                                     PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                     void*                 pvUser);

/*****************************************************************************/
/*! Definitions                                                              */
/*****************************************************************************/

/*****************************************************************************/
/*! DPM layout configuration structure                                       */
/*****************************************************************************/
typedef struct DPM_LAYOUT_CFG_Ttag
{
  uint32_t ulSysChSize;
  uint32_t ulSysMbxSize;
  uint32_t ulComMbxSize;
  uint32_t ulComMbxElements;
  uint32_t ulComIoInputStart;
  uint32_t ulComIoOutputStart;
  uint32_t ulHsOffset;
} DPM_LAYOUT_CFG_T;

static DPM_LAYOUT_CFG_T s_DpmLayout[] = {
  { /* HIL_HIF_LAYOUT_16K */
    sizeof(HIL_HIF_SYSTEM_CHANNEL_16K_T),
    sizeof(HIL_HIF_SYSTEM_MAILBOX_16K_T),
    sizeof(HIL_HIF_MAILBOX_BLOCK_16K_T),
    sizeof(HIL_HIF_MAILBOX_BLOCK_16K_T) / HIL_MEMBERSIZE(HIL_HIF_MAILBOX_BLOCK_16K_T, aabMailbox[0]),
    HIL_OFFSETOF(HIL_HIF_16K_T, tCommChannel.tPdToHost),
    HIL_OFFSETOF(HIL_HIF_16K_T, tCommChannel.tPdFromHost),
    HIL_OFFSETOF(HIL_HIF_16K_T, tChipControlChannel),
  },
  { /* HIL_HIF_LAYOUT_32K */
    sizeof(HIL_HIF_SYSTEM_CHANNEL_T),
    sizeof(HIL_HIF_SYSTEM_MAILBOX_T),
    sizeof(HIL_HIF_MAILBOX_BLOCK_32K_T),
    sizeof(HIL_HIF_MAILBOX_BLOCK_32K_T) / HIL_MEMBERSIZE(HIL_HIF_MAILBOX_BLOCK_32K_T, aabMailbox[0]),
    HIL_OFFSETOF(HIL_HIF_32K_T, tCommChannel.tPdToHost),
    HIL_OFFSETOF(HIL_HIF_32K_T, tCommChannel.tPdFromHost),
    HIL_OFFSETOF(HIL_HIF_32K_T, tChipControlChannel),
  },
  { /* HIL_HIF_LAYOUT_64K */
    sizeof(HIL_HIF_SYSTEM_CHANNEL_T),
    sizeof(HIL_HIF_SYSTEM_MAILBOX_T),
    sizeof(HIL_HIF_MAILBOX_BLOCK_64K_T),
    sizeof(HIL_HIF_MAILBOX_BLOCK_64K_T) / HIL_MEMBERSIZE(HIL_HIF_MAILBOX_BLOCK_64K_T, aabMailbox[0]),
    HIL_OFFSETOF(HIL_HIF_64K_T, tCommChannel.tPdToHost),
    HIL_OFFSETOF(HIL_HIF_64K_T, tCommChannel.tPdFromHost),
    HIL_OFFSETOF(HIL_HIF_64K_T, tChipControlChannel),
  },
  { /* HIL_HIF_LAYOUT_256K */
    sizeof(HIL_HIF_SYSTEM_CHANNEL_T),
    sizeof(HIL_HIF_SYSTEM_MAILBOX_T),
    sizeof(HIL_HIF_MAILBOX_BLOCK_256K_T),
    sizeof(HIL_HIF_MAILBOX_BLOCK_256K_T) / HIL_MEMBERSIZE(HIL_HIF_MAILBOX_BLOCK_256K_T, aabMailbox[0]),
    HIL_OFFSETOF(HIL_HIF_256K_T, tCommChannel.tPdToHost),
    HIL_OFFSETOF(HIL_HIF_256K_T, tCommChannel.tPdFromHost),
    HIL_OFFSETOF(HIL_HIF_256K_T, tChipControlChannel),
  },
};

/*****************************************************************************/
/*!  \addtogroup CIFX_TOOLKIT_FUNCS cifX DPM Toolkit specific functions
*    \{                                                                      */
/*****************************************************************************/

extern uint32_t                g_ulDeviceCount;
extern PDEVICEINSTANCE*        g_pptDevices;
extern TKIT_DRIVER_INFORMATION g_tDriverInfo;
extern void*                   g_pvTkitLock;

/*****************************************************************************/
/*! Delete a channel instance structure and all contained allocated data
*   \param  ptChannel  Channel instance to delete                            */
/*****************************************************************************/
static void cifXDeleteChannelInstance(PCHANNELINSTANCE ptChannel)
{
  uint32_t ulIdx;

  /*-------------------------------------------------*/
  /* Free dynamic objects created for the interrupt  */
  /*-------------------------------------------------*/
  for(ulIdx = 0; ulIdx < HIL_CNT_ELEMENT(ptChannel->apvHsBitEvent); ++ulIdx)
  {
    if(NULL != ptChannel->apvHsBitEvent[ulIdx])
    {
      OS_DeleteEvent(ptChannel->apvHsBitEvent[ulIdx]);
      ptChannel->apvHsBitEvent[ulIdx] = NULL;
    }
  }

  /*-------------------------------------------------*/
  /* Free all dynamically allocated I/O Input Areas  */
  /*-------------------------------------------------*/
  for(ulIdx = 0; ulIdx < ptChannel->tIoArea.ulIOInputAreas; ++ulIdx)
  {
    if(NULL != ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tBlock.pvMutex)
      OS_DeleteMutex(ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tBlock.pvMutex);
    if (NULL != ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tIoCtl.pvEvent)
      OS_DeleteEvent(ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tIoCtl.pvEvent);
    OS_Memfree(ptChannel->tIoArea.aptIOInputAreas[ulIdx]);
  }

  /*-------------------------------------------------*/
  /* Free all dynamically allocated I/O Output Areas */
  /*-------------------------------------------------*/
  for(ulIdx = 0; ulIdx < ptChannel->tIoArea.ulIOOutputAreas; ++ulIdx)
  {
    if(NULL != ptChannel->tIoArea.aptIOOutputAreas[ulIdx]->tBlock.pvMutex)
      OS_DeleteMutex(ptChannel->tIoArea.aptIOOutputAreas[ulIdx]->tBlock.pvMutex);
    if (NULL != ptChannel->tIoArea.aptIOOutputAreas[ulIdx]->tIoCtl.pvEvent)
      OS_DeleteEvent(ptChannel->tIoArea.aptIOOutputAreas[ulIdx]->tIoCtl.pvEvent);
    OS_Memfree(ptChannel->tIoArea.aptIOOutputAreas[ulIdx]);
  }

  /*-------------------------------------------------*/
  /* Delete Mailbox synchronization objects (Mutex)  */
  /*-------------------------------------------------*/
  if(NULL != ptChannel->tFromHostMbx.tSys.pvMutex)
    OS_DeleteMutex(ptChannel->tFromHostMbx.tSys.pvMutex);
  if(NULL != ptChannel->tToHostMbx.tSys.pvMutex)
    OS_DeleteMutex(ptChannel->tToHostMbx.tSys.pvMutex);

  if (NULL != ptChannel->tToHostMbx.tCom.tCtl.pvEvent)
  {
    OS_DeleteEvent(ptChannel->tToHostMbx.tCom.tCtl.pvEvent);
    ptChannel->tToHostMbx.tCom.tCtl.pvEvent = NULL;
  }

  if (NULL != ptChannel->tFromHostMbx.tCom.tCtl.pvEvent)
  {
    OS_DeleteEvent(ptChannel->tFromHostMbx.tCom.tCtl.pvEvent);
    ptChannel->tFromHostMbx.tCom.tCtl.pvEvent = NULL;
  }

#if 0 // TODO
  /* Remove sync resources */
  for(ulIdx = 0; ulIdx < HIL_CNT_ELEMENT(ptDevInstance->tSyncData.ahSyncBitEvents); ++ulIdx)
  {
    if(NULL != ptDevInstance->tSyncData.ahSyncBitEvents[ulIdx])
    {
      OS_DeleteEvent(ptDevInstance->tSyncData.ahSyncBitEvents[ulIdx]);
      ptDevInstance->tSyncData.ahSyncBitEvents[ulIdx] = NULL;
    }
  }

  OS_DeleteLock(ptDevInstance->tSyncData.pvLock);
  ptDevInstance->tSyncData.pvLock = NULL;
#endif

  /*-------------------------------------------------*/
  /* Delete lock object                              */
  /*-------------------------------------------------*/
  if(NULL != ptChannel->pvLock)
    OS_DeleteLock(ptChannel->pvLock);
  if(NULL != ptChannel->pvInitMutex)
    OS_DeleteMutex(ptChannel->pvInitMutex);

  /* Free channel instance */
  OS_Memfree(ptChannel);
}

/*****************************************************************************/
/*! Create Process Data instances for the given channel
*   \param  ptDevInstance  Device Instance
*   \param  ulType         Input or Output area
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXCreateChannelPdInstance(PDEVICEINSTANCE ptDevInstance,
                                           uint32_t        ulType)
{
  HIL_HIF_PROCESS_DATA_BLOCK_T* ptIOInfo = (HIL_HIF_PROCESS_DATA_BLOCK_T*) &ptDevInstance->pbDPM[HIL_OFFSETOF(HIL_HIF_SYSTEM_CHANNEL_T, tProcessDataInfo)];
  PCHANNELINSTANCE ptChannelInst = ptDevInstance->pptCommChannels[0];
  NETX_IO_BLOCK_T** aptIOAreas = &ptChannelInst->tIoArea.aptIOInputAreas[0];
  DPM_LAYOUT_CFG_T* ptLayout = &s_DpmLayout[ptDevInstance->bDPMLayout - 1];
  HIL_HIF_CHIP_CONTROL_CHANNEL_T* ptGlobalRegisters = (HIL_HIF_CHIP_CONTROL_CHANNEL_T*)ptDevInstance->pvGlobalRegisters;
  uint32_t* pulIOAreaCount = &ptChannelInst->tIoArea.ulIOInputAreas;
  uint32_t ulIoAreaOffset = ptLayout->ulComIoInputStart;
  uint32_t aulNotifyEvents[] = {CIFX_NOTIFY_PD0_IN,  CIFX_NOTIFY_PD1_IN,  CIFX_NOTIFY_PD2_IN,  CIFX_NOTIFY_PD3_IN,
                                CIFX_NOTIFY_PD0_OUT, CIFX_NOTIFY_PD1_OUT, CIFX_NOTIFY_PD2_OUT, CIFX_NOTIFY_PD3_OUT};
  uint32_t ulPdIdx = 0;
  uint8_t bIdx;
  uint8_t bBitoffset = 8;
  int32_t lRet = CIFX_NO_ERROR;

  if (HIL_HIF_DIRECTION_OUT == ulType)
  {
    aptIOAreas = &ptChannelInst->tIoArea.aptIOOutputAreas[0];
    pulIOAreaCount = &ptChannelInst->tIoArea.ulIOOutputAreas;
    ulIoAreaOffset = ptLayout->ulComIoOutputStart;
    bBitoffset = 12;
    ulPdIdx = 4;
  }

  for (bIdx = 0; bIdx < HIL_HIF_MAX_TBUF_CHANNELS; bIdx++)
  {
    if (0 != LE16_TO_HOST(HWIF_READ16(ptDevInstance, ptIOInfo->atProcessDataInfo[bIdx + ulPdIdx].usSize)))
    {
      NETX_IO_BLOCK_T* ptIOInst = NULL;
      void* pvIoMutex = NULL;

      if (NULL == (ptIOInst  = OS_Memalloc(sizeof(*ptIOInst))) ||
          NULL == (pvIoMutex = OS_CreateMutex()))
      {
        OS_DeleteMutex(pvIoMutex);
        OS_DeleteMutex(ptIOInst);

        if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    CIFX_TRACE_LEVEL_ERROR,
                    "Error creating IO instance buffer!");
        }

        lRet = CIFX_INVALID_POINTER;

      } else
      {
        uint8_t* pbDpm = ptDevInstance->pbDPM;

        OS_Memset(ptIOInst, 0, sizeof(*ptIOInst));

        /* Create Input area */
        ptIOInst->tBlock.pvMutex         = pvIoMutex;
        ptIOInst->tBlock.bBitoffset      = 8 + bIdx;
        ptIOInst->tBlock.ulBitmask       = 1 << bBitoffset;
        ptIOInst->tBlock.pbBlockStart    = (uint8_t*) pbDpm + ulIoAreaOffset;
        ptIOInst->tBlock.ulBlockLength   = ((uint32_t) LE16_TO_HOST(HWIF_READ16(ptDevInstance, ptIOInfo->atProcessDataInfo[bIdx + ulPdIdx].usSize))) + 8; /* add 8 because of action cells */
        ptIOInst->tBlock.ulElementCnt    = 3;
        ptIOInst->tBlock.ulElementSize   = ptIOInst->tBlock.ulBlockLength & 0xFFFFFF8;
        ptIOInst->tIoCtl.pbAction        = &ptIOInst->tBlock.pbBlockStart[0];
        ptIOInst->tIoCtl.pbStatus        = &ptIOInst->tBlock.pbBlockStart[1];
        ptIOInst->tIoCtl.ulNotifyEvent   = aulNotifyEvents[bIdx + ulPdIdx];
        *pulIOAreaCount                 += 1;
        ulIoAreaOffset                  += ptIOInst->tBlock.ulBlockLength;
        aptIOAreas[bIdx]                 = ptIOInst;

        if(g_ulTraceLevel & CIFX_TRACE_LEVEL_DEBUG)
        {
          USER_Trace(ptDevInstance,
                    CIFX_TRACE_LEVEL_DEBUG,
                    "I/O Subblock found    (Channel=%d, Block=%d, Offset=0x%08X, Len=0x%04X)",
                    ptChannelInst->ulChannelNumber,
                    *pulIOAreaCount,
                    ulIoAreaOffset,
                    ptIOInst->tBlock.ulBlockLength);
        }
      }
    }
  }

  if(ptDevInstance->bDPMLayout >= HIL_HIF_LAYOUT_16K &&
     ptDevInstance->bDPMLayout <= HIL_HIF_LAYOUT_64K)
  {
    /*TODO: workaround for netX900 MPW (use tunnel of dpm0 config to map host registers into dpm)
            -> delete later on final chip */
    ptChannelInst->tIoArea.tTlbCtl.pulTlbHostStatus = &(((HIL_HIF_CHIP_CONTROL_CHANNEL_PDPM_T*)(ptDevInstance->pvGlobalRegisters))->tHostIrq.ulIrqMaskSet);
    ptChannelInst->tIoArea.tTlbCtl.pulTlbNetxStatus = &(((HIL_HIF_CHIP_CONTROL_CHANNEL_PDPM_T*)(ptDevInstance->pvGlobalRegisters))->tHostIrq.ulIrqPending);
  }
  else
  {
    ptChannelInst->tIoArea.tTlbCtl.pulTlbHostStatus = &ptGlobalRegisters->tHostIrq.ulIrqMaskSet;
    ptChannelInst->tIoArea.tTlbCtl.pulTlbNetxStatus = &ptGlobalRegisters->tHostIrq.ulIrqPending;
  }

  return lRet;
}

/*****************************************************************************/
/*! Create all channel instances for the given device
*   \param  ptDevInstance  Instance to start up
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXCreateChannelInstance(PDEVICEINSTANCE ptDevInstance)
{
  PCHANNELINSTANCE ptChannelInst = NULL;
  void* pvInitMutex = NULL;
  void* pvLock = NULL;
  void* pvSendMBXMutex = NULL;
  void* pvRecvMBXMutex = NULL;
  int32_t lRet = CIFX_NO_ERROR;

  if (NULL == (ptChannelInst                  = (PCHANNELINSTANCE)OS_Memalloc(sizeof(*ptChannelInst))) ||
      NULL == (ptDevInstance->pptCommChannels = (PCHANNELINSTANCE*)OS_Memalloc(sizeof(*ptDevInstance->pptCommChannels))) ||
      NULL == (pvInitMutex                    = OS_CreateMutex()) ||
      NULL == (pvLock                         = OS_CreateLock())  ||
      NULL == (pvSendMBXMutex                 = OS_CreateMutex()) ||
      NULL == (pvRecvMBXMutex                 = OS_CreateMutex()) )
  {
    OS_Memfree(ptChannelInst);
    OS_Memfree(ptDevInstance->pptCommChannels);
    OS_DeleteMutex(pvInitMutex);
    OS_DeleteLock(pvLock);
    OS_DeleteMutex(pvSendMBXMutex);
    OS_DeleteMutex(pvRecvMBXMutex);

    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_ERROR,
                "Error creating buffers for COM channel!");
    }

    lRet = CIFX_INVALID_POINTER;

  } else
  {
    DPM_LAYOUT_CFG_T* ptLayout = &s_DpmLayout[ptDevInstance->bDPMLayout - 1];
    uint8_t* pbDpm = ptDevInstance->pbDPM;

    OS_Memset(ptChannelInst, 0, sizeof(*ptChannelInst));

    ptChannelInst->pbDPMChannelStart                       = pbDpm + ptLayout->ulSysChSize;
    ptChannelInst->ulDPMChannelLength                      = ptLayout->ulHsOffset - ptLayout->ulSysChSize;

    /* These Locks/Mutexes are needed during initialization as we want to send packets, etc.
       They need to be removed if the channel is not being created (e.g. wrong channel type) */
    ptChannelInst->pvLock                                  = pvLock;
    ptChannelInst->pvInitMutex                             = pvInitMutex;
    ptChannelInst->pvDeviceInstance                        = (void*)ptDevInstance;

    /* Handshake block and cells */
    ptChannelInst->tHsCtrl.pulHostFlags                    = &ptDevInstance->pulHandshakeBlock[HIL_HIF_HSC_HOST_COM];
    ptChannelInst->tHsCtrl.pulNetxFlags                    = &ptDevInstance->pulHandshakeBlock[HIL_HIF_HSC_NETX_COM];

    /* Create send mailbox */
    ptChannelInst->tFromHostMbx.tCom.tBlock.pvMutex        = pvSendMBXMutex;
    ptChannelInst->tFromHostMbx.tCom.tBlock.bBitoffset     = 0;
    ptChannelInst->tFromHostMbx.tCom.tBlock.ulBitmask      = HIL_HIF_MBX_IDX_MASK | HIL_HIF_MBX_WRAPAROUND;
    ptChannelInst->tFromHostMbx.tCom.tBlock.pbBlockStart   = &pbDpm[ptLayout->ulSysChSize];
    ptChannelInst->tFromHostMbx.tCom.tBlock.ulBlockLength  = ptLayout->ulComMbxSize;
    ptChannelInst->tFromHostMbx.tCom.tBlock.ulElementCnt   = ptLayout->ulComMbxElements;
    ptChannelInst->tFromHostMbx.tCom.tBlock.ulElementSize  = ptChannelInst->tFromHostMbx.tCom.tBlock.ulBlockLength / ptChannelInst->tFromHostMbx.tCom.tBlock.ulElementCnt;
    ptChannelInst->tFromHostMbx.tCom.tCtl.pulHostFlags     = &ptDevInstance->pulHandshakeBlock[HIL_HIF_HSC_HOST_SNDMBX];
    ptChannelInst->tFromHostMbx.tCom.tCtl.pulNetxFlags     = &ptDevInstance->pulHandshakeBlock[HIL_HIF_HSC_NETX_SNDMBX];

    /* Create receive mailbox */
    ptChannelInst->tToHostMbx.tCom.tBlock.pvMutex          = pvRecvMBXMutex;
    ptChannelInst->tToHostMbx.tCom.tBlock.bBitoffset       = 0;
    ptChannelInst->tToHostMbx.tCom.tBlock.ulBitmask        = HIL_HIF_MBX_IDX_MASK | HIL_HIF_MBX_WRAPAROUND;
    ptChannelInst->tToHostMbx.tCom.tBlock.pbBlockStart     = ptChannelInst->tFromHostMbx.tCom.tBlock.pbBlockStart + ptChannelInst->tFromHostMbx.tCom.tBlock.ulBlockLength;
    ptChannelInst->tToHostMbx.tCom.tBlock.ulBlockLength    = ptLayout->ulComMbxSize;
    ptChannelInst->tToHostMbx.tCom.tBlock.ulElementCnt     = ptLayout->ulComMbxElements;
    ptChannelInst->tToHostMbx.tCom.tBlock.ulElementSize    = ptChannelInst->tToHostMbx.tCom.tBlock.ulBlockLength / ptChannelInst->tToHostMbx.tCom.tBlock.ulElementCnt;
    ptChannelInst->tToHostMbx.tCom.tCtl.pulHostFlags       = &ptDevInstance->pulHandshakeBlock[HIL_HIF_HSC_HOST_RCVMBX];
    ptChannelInst->tToHostMbx.tCom.tCtl.pulNetxFlags       = &ptDevInstance->pulHandshakeBlock[HIL_HIF_HSC_NETX_RCVMBX];

    ptChannelInst->ptCommunicationStatusBlock              = (HIL_HIF_COMMUNICATION_STATUS_BLOCK_T*) &pbDpm[HIL_OFFSETOF(HIL_HIF_SYSTEM_CHANNEL_T, tCommunicationState)];
    ptDevInstance->ulCommChannelCount                      = 1;
    ptDevInstance->pptCommChannels[0]                      = ptChannelInst;

    /* Create Input & Output areas */
    lRet = cifXCreateChannelPdInstance(ptDevInstance, HIL_HIF_DIRECTION_IN);
    if (CIFX_NO_ERROR == lRet)
      lRet = cifXCreateChannelPdInstance(ptDevInstance, HIL_HIF_DIRECTION_OUT);

    DEV_ReadHostFlags(ptChannelInst, 1);
    DEV_ReadHandshakeFlags(ptChannelInst, 0, 0);

    /* Check READY again including COS flag handling, because we have to handle the COS flags */
    if(DEV_WaitForReady_Poll(ptChannelInst, 20))
    {
      int32_t lTempError = CIFX_NO_ERROR;
      if ( CIFX_NO_ERROR != (lTempError = cifXReadFirmwareIdent( ptDevInstance,
                                                                 ptChannelInst->ulChannelNumber,
                                                                 NULL,
                                                                 NULL)))
      {
        if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    CIFX_TRACE_LEVEL_ERROR,
                    "Failed to read firmware identification for channel = %d, error: 0x%08X", ptChannelInst->ulChannelNumber, lTempError);
        }
      }

      if(g_ulTraceLevel & CIFX_TRACE_LEVEL_INFO)
      {
        USER_Trace(ptDevInstance,
                  CIFX_TRACE_LEVEL_INFO,
                  "Device successfully created for channel = %d", ptChannelInst->ulChannelNumber);
      }
    }
  }

  if( (g_ulTraceLevel & CIFX_TRACE_LEVEL_INFO) &&
      (0 == ptDevInstance->ulCommChannelCount) )
  {
    USER_Trace(ptDevInstance,
               CIFX_TRACE_LEVEL_INFO,
               "NO CHANNEL INFORMATION FOUND, No devices created!");
  }

  return lRet;
}

/*****************************************************************************/
/*! Check the DPM/HIF for a compatible layout.
*   \param ptDevInstance Instance to start up
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXCheckDpmLayout(PDEVICEINSTANCE ptDevInstance)
{
  uint8_t*                  pbDpm        = ptDevInstance->pbDPM;
  HIL_HIF_SYSTEM_CHANNEL_T* ptSysChannel = (HIL_HIF_SYSTEM_CHANNEL_T*)pbDpm;
  int32_t                   lRet         = CIFX_NO_ERROR;
  uint8_t                   bDpmLayout   = HWIF_READ8(ptDevInstance, ptSysChannel->tSystemInfo.bHifLayout);

  if (bDpmLayout < HIL_HIF_LAYOUT_16K || bDpmLayout > HIL_HIF_LAYOUT_256K)
    lRet = CIFX_DEV_DPM_LAYOUT_UNKNOWN;

  return lRet;
}

/*****************************************************************************/
/*! Starts a device from ground up
*   \param ptDevInstance Instance to start up
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXCreateSystemDevice(PDEVICEINSTANCE ptDevInstance)
{
  int32_t lRet = CIFX_NO_ERROR;

  /* Initialize System Channel which must be present */
  uint8_t*                    pbDpm           = ptDevInstance->pbDPM;
  HIL_HIF_SYSTEM_CHANNEL_T*   ptSysChannel    = (HIL_HIF_SYSTEM_CHANNEL_T*)pbDpm;
  PCHANNELINSTANCE            ptSystemDevice  = &ptDevInstance->tSystemDevice;
  DPM_LAYOUT_CFG_T*           ptLayout        = NULL;

  CIFX_DEVICE_INFORMATION   tDevInfo;
  uint32_t                  ulDeviceIdx      = 0;
  void*                     pvSendMBXMutex   = NULL;
  void*                     pvRecvMBXMutex   = NULL;
  void*                     pvInitMutex      = NULL;
  void*                     pvLock           = NULL;

  if (CIFX_NO_ERROR != (lRet = cifXCheckDpmLayout(ptDevInstance)))
    return lRet;

  if (NULL == (pvSendMBXMutex = OS_CreateMutex()) ||
      NULL == (pvRecvMBXMutex = OS_CreateMutex()) ||
      NULL == (pvInitMutex    = OS_CreateMutex()) ||
      NULL == (pvLock         = OS_CreateLock())  )
  {
    lRet = CIFX_INVALID_POINTER;

    OS_DeleteMutex(pvSendMBXMutex);
    OS_DeleteMutex(pvRecvMBXMutex);
    OS_DeleteMutex(pvInitMutex);
    OS_DeleteLock(pvLock);
    pvSendMBXMutex = NULL;
    pvRecvMBXMutex = NULL;
    pvInitMutex    = NULL;
    pvLock         = NULL;

    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_ERROR,
                "Error creating buffers for system device!");
    }

  } else
  {
    OS_Memset(&tDevInfo, 0, sizeof(tDevInfo));

    /* Initialize DEVICEINSTANCE */
    ptDevInstance->ulDeviceNumber = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemInfo.ulDeviceNumber));
    ptDevInstance->ulSerialNumber = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemInfo.ulSerialNumber));
    ptDevInstance->ulSlotNumber   = HWIF_READ8(ptDevInstance, ptSysChannel->tSystemInfo.bDevIdNumber);
    ptDevInstance->bDPMLayout     = HWIF_READ8(ptDevInstance, ptSysChannel->tSystemInfo.bHifLayout);
    ptLayout                      = &s_DpmLayout[ptDevInstance->bDPMLayout - 1];

    /* Setup pointer to global netX register block. Assume every card has the register block at the end of the DPM. */
    ptDevInstance->pvGlobalRegisters = (void*) (ptDevInstance->pbDPM + ptLayout->ulHsOffset);

    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_DEBUG)
    {
      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_DEBUG,
                "Device Info:");

      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_DEBUG,
                " - Device Number : %u",
                ptDevInstance->ulDeviceNumber);

      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_DEBUG,
                " - Serial Number : %u",
                ptDevInstance->ulSerialNumber);

      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_DEBUG,
                " - Slot Number   : %u",
                ptDevInstance->ulSlotNumber);
    }

    tDevInfo.ulDeviceNumber   = ptDevInstance->ulDeviceNumber;
    tDevInfo.ulSerialNumber   = ptDevInstance->ulSerialNumber;
    tDevInfo.ptDeviceInstance = ptDevInstance;

    /* Get the user alias name */
    USER_GetAliasName(&tDevInfo, sizeof(ptDevInstance->szAlias), ptDevInstance->szAlias);

    /* Check if alias is unique */
    if(OS_Strlen(ptDevInstance->szAlias) > 0)
    {
      OS_EnterLock(g_pvTkitLock);

      for(ulDeviceIdx = 0; ulDeviceIdx < g_ulDeviceCount; ++ulDeviceIdx)
      {
        if(OS_Strcmp(g_pptDevices[ulDeviceIdx]->szAlias, ptDevInstance->szAlias) == 0)
        {
          /* Duplicate alias found */
          if(g_ulTraceLevel & CIFX_TRACE_LEVEL_WARNING)
          {
            USER_Trace(ptDevInstance,
                      CIFX_TRACE_LEVEL_WARNING,
                      "Duplicate alias '%s' passed (DevNr=%u, SerNr=%u), Alias will be removed!",
                      ptDevInstance->szAlias,
                      ptDevInstance->ulDeviceNumber,
                      ptDevInstance->ulSerialNumber);
          }

          OS_Memset(ptDevInstance->szAlias, 0, sizeof(ptDevInstance->szAlias));
        }
      }
      OS_LeaveLock(g_pvTkitLock);
    }

    ptDevInstance->pulHandshakeBlock                = (uint32_t*) &pbDpm[ptLayout->ulHsOffset];
    ptSystemDevice->tHsCtrl.pulHostFlags            = &ptDevInstance->pulHandshakeBlock[HIL_HIF_HSC_HOST_SYS];
    ptSystemDevice->tHsCtrl.pulNetxFlags            = &ptDevInstance->pulHandshakeBlock[HIL_HIF_HSC_NETX_SYS];
    ptSystemDevice->pbDPMChannelStart               = pbDpm;

    /* Create send mailbox */
    ptSystemDevice->tFromHostMbx.tSys.pvMutex       = pvSendMBXMutex;
    ptSystemDevice->tFromHostMbx.tSys.bBitoffset    = HIL_HIF_HSF_MBX_FROM_HOST_CMD_BIT_NO;
    ptSystemDevice->tFromHostMbx.tSys.ulBitmask     = 1 << HIL_HIF_HSF_MBX_FROM_HOST_CMD_BIT_NO;
    ptSystemDevice->tFromHostMbx.tSys.pbBlockStart  = &pbDpm[ptLayout->ulSysChSize - (2 * ptLayout->ulSysMbxSize)];
    ptSystemDevice->tFromHostMbx.tSys.ulBlockLength = ptLayout->ulSysMbxSize;
    ptSystemDevice->tFromHostMbx.tSys.ulElementCnt  = 1;
    ptSystemDevice->tFromHostMbx.tSys.ulElementSize = ptSystemDevice->tFromHostMbx.tSys.ulBlockLength / ptSystemDevice->tFromHostMbx.tSys.ulElementCnt;

    /* Create receive mailbox */
    ptSystemDevice->tToHostMbx.tSys.pvMutex         = pvRecvMBXMutex;
    ptSystemDevice->tToHostMbx.tSys.bBitoffset      = HIL_HIF_HSF_MBX_TO_HOST_ACK_BIT_NO;
    ptSystemDevice->tToHostMbx.tSys.ulBitmask       = (1 << HIL_HIF_HSF_MBX_TO_HOST_ACK_BIT_NO);
    ptSystemDevice->tToHostMbx.tSys.pbBlockStart    = &pbDpm[ptLayout->ulSysChSize - ptLayout->ulSysMbxSize];
    ptSystemDevice->tToHostMbx.tSys.ulBlockLength   = ptLayout->ulSysMbxSize;
    ptSystemDevice->tToHostMbx.tSys.ulElementCnt    = 1;
    ptSystemDevice->tToHostMbx.tSys.ulElementSize   = ptSystemDevice->tToHostMbx.tSys.ulBlockLength / ptSystemDevice->tToHostMbx.tSys.ulElementCnt;

    ptSystemDevice->ulDPMChannelLength              = ptLayout->ulSysChSize;
    ptSystemDevice->pvLock                          = pvLock;
    ptSystemDevice->pvInitMutex                     = pvInitMutex;
    ptSystemDevice->pvDeviceInstance                = (void*)ptDevInstance;
    ptSystemDevice->fIsSysDevice                    = 1;

    /* Read actual Host state */
    DEV_ReadHostFlags(ptSystemDevice, 1);
    DEV_ReadHandshakeFlags(ptSystemDevice, 1, 0);

    /*--------------------------------------------
      Check if READY is available
    --------------------------------------------*/
    /* Check if system channel is READY before executing additional functions on it */
    if (!DEV_WaitForReady_Poll(ptSystemDevice, CIFX_TO_FIRMWARE_START))
    {
      lRet = CIFX_DEV_NOT_READY;

      /* READY state not reached */
      if (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
          CIFX_TRACE_LEVEL_ERROR,
          "Error device does not reach READY state! (lRet=0x%08X)",
          lRet);
      }
    }

    /* Display actual system state if available */
    if (ptSystemDevice->tHsCtrl.ulNetxFlags & NSF_ERROR)
    {
      /* Trace system error */
      if (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
      {
        uint32_t                  ulError = 0;
        HIL_HIF_SYSTEM_CHANNEL_T* ptSysCh = (HIL_HIF_SYSTEM_CHANNEL_T*)ptSystemDevice->pbDPMChannelStart;

        if (0 != (ulError = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysCh->tSystemState.ulSystemError))))
        {
          USER_Trace(ptDevInstance,
            CIFX_TRACE_LEVEL_ERROR,
            "System error information, (SystemError=0x%08X)!",
            ulError);
        }

        if (0 != (ulError = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysCh->tSystemState.ulSystemStatus))))
        {
          USER_Trace(ptDevInstance,
            CIFX_TRACE_LEVEL_ERROR,
            "System state information, (SystemState=0x%08X)!",
            ulError);
        }
      }
    }
  }

  return lRet;  /*lint !e438 : Last value assigned not used */
}

/*****************************************************************************/
/*! Check for IRQ enable
*   \param ptDevInstance Instance to start up
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXCheckIRQEnable(PDEVICEINSTANCE ptDevInstance)
{
  CIFX_DEVICE_INFORMATION tDevInfo;
  int32_t lRet = CIFX_NO_ERROR;

  OS_Memset(&tDevInfo, 0, sizeof(tDevInfo));

  /* Initialize file information structure */
  tDevInfo.ulDeviceNumber   = ptDevInstance->ulDeviceNumber;
  tDevInfo.ulSerialNumber   = ptDevInstance->ulSerialNumber;
  tDevInfo.ulChannel        = CIFX_SYSTEM_DEVICE;
  tDevInfo.ptDeviceInstance = ptDevInstance;

  /* Ask for interrupt handling */
  if(0 != (ptDevInstance->fIrqEnabled = USER_GetInterruptEnable(&tDevInfo)))
  {
    PCHANNELINSTANCE ptChannel = &ptDevInstance->tSystemDevice;
    uint32_t ulChannel = 0;
    uint32_t ulIdx;

#if 0 // TODO
    /* Create interrupt events for sync handling */
    for (ulIdx = 0; ulIdx < HIL_CNT_ELEMENT(ptDevInstance->tSyncData.ahSyncBitEvents); ++ulIdx)
    {
      if (NULL == (ptDevInstance->tSyncData.ahSyncBitEvents[ulIdx] = OS_CreateEvent()))
      {
        lRet = CIFX_INVALID_POINTER;

        if (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    CIFX_TRACE_LEVEL_ERROR,
                    "Error creating sync event buffer!");
        }

        break;
      }
    }
#endif

    if (CIFX_NO_ERROR == lRet)
    {
      /* Create events for all channels */
      do
      {
        /* Create interrupt events if we are working in interrupt mode */
        for (ulIdx = 0; ulIdx < HIL_CNT_ELEMENT(ptChannel->apvHsBitEvent); ++ulIdx)
        {
          if (NULL == (ptChannel->apvHsBitEvent[ulIdx] = OS_CreateEvent()))
          {
            lRet = CIFX_INVALID_POINTER;

            if (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        CIFX_TRACE_LEVEL_ERROR,
                        "Error creating interrupt event buffer!");
            }

            break;
          }
        }

        if (CIFX_NO_ERROR == lRet && !ptChannel->fIsSysDevice)
        {
          /* Create interrupt events for the communication mailbox */
          if (NULL == (ptChannel->tToHostMbx.tCom.tCtl.pvEvent   = OS_CreateEvent()) ||
              NULL == (ptChannel->tFromHostMbx.tCom.tCtl.pvEvent = OS_CreateEvent())) // TODO
          {
            lRet = CIFX_INVALID_POINTER;

            if (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        CIFX_TRACE_LEVEL_ERROR,
                        "Error creating interrupt event buffer for communication mailbox!");
            }
          }

          /* Create interrupt events for IO input area */
          for (ulIdx = 0; ulIdx < ptChannel->tIoArea.ulIOInputAreas; ++ulIdx)
          {
            if (NULL == (ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tIoCtl.pvEvent  = OS_CreateEvent()))
            {
              lRet = CIFX_INVALID_POINTER;

              if (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
              {
                USER_Trace(ptDevInstance,
                          CIFX_TRACE_LEVEL_ERROR,
                          "Error creating interrupt event buffer for IO input area!");
              }
            }
          }

          /* Create interrupt events for IO output area */
          for (ulIdx = 0; ulIdx < ptChannel->tIoArea.ulIOOutputAreas; ++ulIdx)
          {
            if (NULL == (ptChannel->tIoArea.aptIOOutputAreas[ulIdx]->tIoCtl.pvEvent = OS_CreateEvent()))
            {
              lRet = CIFX_INVALID_POINTER;

              if (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
              {
                USER_Trace(ptDevInstance,
                          CIFX_TRACE_LEVEL_ERROR,
                          "Error creating interrupt event buffer for IO output area!");
              }
            }
          }
        }

        /* Stop processing of further channels if lRet is set. */
        if (CIFX_NO_ERROR != lRet)
          break;

        /* Check if we have such a channel */
        if(ulChannel < ptDevInstance->ulCommChannelCount)
          ptChannel = ptDevInstance->pptCommChannels[ulChannel];

      } while(ulChannel++ < ptDevInstance->ulCommChannelCount);
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Check for enable cached IO buffer access
*   \param ptDevInstance Instance to start up
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXCheckCachedBufferEnable(PDEVICEINSTANCE ptDevInstance)
{
  CIFX_DEVICE_INFORMATION tDevInfo;
  int32_t                 lRet = CIFX_NO_ERROR;
  int                     iCachedState = 0;

  OS_Memset(&tDevInfo, 0, sizeof(tDevInfo));

  /* Initialize file information structure */
  tDevInfo.ulDeviceNumber   = ptDevInstance->ulDeviceNumber;
  tDevInfo.ulSerialNumber   = ptDevInstance->ulSerialNumber;
  tDevInfo.ulChannel        = CIFX_SYSTEM_DEVICE;
  tDevInfo.ptDeviceInstance = ptDevInstance;

  /* Ask for cached IO buffer access */
  iCachedState = USER_GetCachedIOBufferMode(&tDevInfo);
  switch (iCachedState)
  {
    case eCACHED_MODE_ON:
    case eCACHED_MODE_OFF:
      /* Store the information in the device structure */
      ptDevInstance->fCachedMemAccess = iCachedState;
    break;

    default:
      lRet = CIFX_INVALID_PARAMETER;
      if (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                   CIFX_TRACE_LEVEL_ERROR,
                   "USER_GetCachedIOBufferMode() returned invalid caching mode");
      }
    break;
  }

  return lRet;
}

#if 0 // TODO #ifdef CIFX_TOOLKIT_DMA
/*****************************************************************************/
/*! Check for DMA enable
*   \param ptDevInstance Instance to start up
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXCheckDMAEnable(PDEVICEINSTANCE ptDevInstance)
{
  CIFX_DEVICE_INFORMATION tDevInfo;
  int                     iDMAState = 0;

  OS_Memset(&tDevInfo, 0, sizeof(tDevInfo));

  /* If we don't have DMA buffers, we cannot enable them,
     but we need to return OK, to let the toolkit continue
     initialization */
  if(0 == ptDevInstance->ulDMABufferCount)
    return CIFX_NO_ERROR;

  /* Initialize file information structure */
  tDevInfo.ulDeviceNumber   = ptDevInstance->ulDeviceNumber;
  tDevInfo.ulSerialNumber   = ptDevInstance->ulSerialNumber;
  tDevInfo.ulChannel        = CIFX_SYSTEM_DEVICE;
  tDevInfo.ptDeviceInstance = ptDevInstance;

  /* Ask for interrupt handling */
  iDMAState = USER_GetDMAMode(&tDevInfo);
  switch(iDMAState)
  {
    case eDMA_MODE_LEAVE:
    {
      /* Check all channels if they have an active DMA flag and setup the DMA buffers for these cahnnels */
      uint32_t ulChannelIdx = ptDevInstance->ulCommChannelCount;
      for( ulChannelIdx = 0; ulChannelIdx < ptDevInstance->ulCommChannelCount; ulChannelIdx++)
      {
        PCHANNELINSTANCE ptChannel = ptDevInstance->pptCommChannels[ulChannelIdx];
        if(ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_DMA)
        {
          /* This channel has DMA activated, setup DMA buffers */
          (void)DEV_SetupDMABuffers( ptChannel);
        }
      }
    }
    break;

    case eDMA_MODE_ON:
    {
      /* Switch ON DMA handling on all communication channels which supporting DMA */
      uint32_t ulTemp        = 0;
      uint32_t ulChannelIdx = ptDevInstance->ulCommChannelCount;
      for( ulChannelIdx = 0; ulChannelIdx < ptDevInstance->ulCommChannelCount; ulChannelIdx++)
      {
        PCHANNELINSTANCE ptChannel = ptDevInstance->pptCommChannels[ulChannelIdx];

        /* Check if channel supports DMA */
        /* TODO: Check DMA capability of the channel */

        /* This channel has DMA activated, setup DMA buffers */
        (void)DEV_SetupDMABuffers( ptChannel);

        /* Activate DMA on all channels which are available */
        if ( CIFX_NO_ERROR != DEV_DMAState( ptChannel, CIFX_DMA_STATE_ON, &ulTemp))
        {
          if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                       CIFX_TRACE_LEVEL_ERROR,
                       "Failed to activate DMA handling (Channel=%u)",
                       ulChannelIdx);
          }
        }
      }
    }
    break;

    case eDMA_MODE_OFF:
    {
      /* Switch OFF DMA handling on all communication channels which enabled DMA */
      uint32_t ulTemp        = 0;
      uint32_t ulChannelIdx = ptDevInstance->ulCommChannelCount;
      for( ulChannelIdx = 0; ulChannelIdx < ptDevInstance->ulCommChannelCount; ulChannelIdx++)
      {
        PCHANNELINSTANCE ptChannel = ptDevInstance->pptCommChannels[ulChannelIdx];
        if(ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_DMA)
        {
          /* This channel has DMA active, switch OFF */
          (void)DEV_DMAState( ptChannel, CIFX_DMA_STATE_OFF, &ulTemp);
        }
      }
    }
    break;

    default:
    break;

  }

  return CIFX_NO_ERROR;
}
#endif

/*****************************************************************************/
/*! Basic netX device start-up
*   \param ptDevInstance Instance to start up
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXStartDevice(PDEVICEINSTANCE ptDevInstance)
{
  int32_t lRet = CIFX_DRV_INIT_STATE_ERROR;

  ptDevInstance->lInitError = CIFX_NO_ERROR;

  ptDevInstance->eDeviceType = eCIFX_DEVICE_FLASH_BASED;
  ptDevInstance->eChipType = eCHIP_TYPE_NETX900;

  if (CIFX_NO_ERROR != (lRet = cifXCreateSystemDevice( ptDevInstance)))
  {
    USER_Trace(ptDevInstance,
              CIFX_TRACE_LEVEL_ERROR,
              "Unable to access the hardware. Aborting device handling!");
  }

  if(CIFX_NO_ERROR == lRet)
  {
#if 0 // TODO
    /* Create sync resources  */
    if (NULL == (ptDevInstance->tSyncData.pvLock = OS_CreateLock()))
    {
      lRet = CIFX_INVALID_POINTER;

      if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                  CIFX_TRACE_LEVEL_ERROR,
                  "Error creating sync resources!");
      }
    } else
#endif
    {
      /* Read the channel layouts, and build the CHANNELINSTANCES for this device */
      lRet = cifXCreateChannelInstance(ptDevInstance);
    }
  }

#ifdef CIFX_TOOLKIT_TIME
  if(CIFX_NO_ERROR == lRet)
  {
    /* Update the system time of the target if a RTC is available */
    cifXInitTime(ptDevInstance);
  }
#endif

#if 0 // TODO #ifdef CIFX_TOOLKIT_DMA
  if(CIFX_NO_ERROR == lRet)
  {
    /* Check DMA enable */
    lRet = cifXCheckDMAEnable(ptDevInstance);
  }
#endif

  if (CIFX_NO_ERROR == lRet)
  {
    /* Check for cached IO buffer handling */
    lRet = cifXCheckCachedBufferEnable(ptDevInstance);
  }

  if(CIFX_NO_ERROR == lRet)
  {
    /* Check IRQ enable */
    lRet = cifXCheckIRQEnable(ptDevInstance);
  }

  /* Store error in device instance */
  if(CIFX_NO_ERROR != lRet)
    ptDevInstance->lInitError = lRet;

  return lRet;
}

/*****************************************************************************/
/*! Stops Handling the device and removes all associated memory
* ATTENTION: If any application has still opened a connection this will result
*            in an access violation/undefined behavious
*   \param ptDevInstance Instance to clean up
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXStopDevice(PDEVICEINSTANCE ptDevInstance)
{
  int32_t          lRet           = CIFX_NO_ERROR;
  uint32_t         ulIdx          = 0;
  PCHANNELINSTANCE ptSystemDevice = &ptDevInstance->tSystemDevice;

  if (HIL_HIF_LAYOUT_NA == ptDevInstance->bDPMLayout)
    return CIFX_DEV_DPM_LAYOUT_UNKNOWN;

  if (ptDevInstance->ulCommChannelCount > 0)
  {
    cifXDeleteChannelInstance(ptDevInstance->pptCommChannels[0]);
    ptDevInstance->ulCommChannelCount = 0;
    ptDevInstance->pptCommChannels[0] = NULL;
  }

  /*-------------------------------------------------*/
  /* Delete event objects                            */
  /*-------------------------------------------------*/
  for(ulIdx = 0; ulIdx < HIL_CNT_ELEMENT(ptSystemDevice->apvHsBitEvent); ++ulIdx)
  {
    if(NULL != ptSystemDevice->apvHsBitEvent[ulIdx])
    {
      OS_DeleteEvent(ptSystemDevice->apvHsBitEvent[ulIdx]);
      ptSystemDevice->apvHsBitEvent[ulIdx] = NULL;
    }
  }

  /*-------------------------------------------------*/
  /* Delete system channel objects                   */
  /*-------------------------------------------------*/
  OS_DeleteLock(ptSystemDevice->pvLock);
  ptSystemDevice->pvLock = NULL;
  OS_DeleteMutex(ptSystemDevice->pvInitMutex);
  ptSystemDevice->pvInitMutex = NULL;
  OS_DeleteMutex(ptSystemDevice->tToHostMbx.tSys.pvMutex);
  ptSystemDevice->tToHostMbx.tSys.pvMutex = NULL;
  OS_DeleteMutex(ptSystemDevice->tFromHostMbx.tSys.pvMutex);
  ptSystemDevice->tFromHostMbx.tSys.pvMutex = NULL;

  /*-------------------------------------------------*/
  /* Delete Communication channel array              */
  /*-------------------------------------------------*/
  OS_Memfree(ptDevInstance->pptCommChannels);
  ptDevInstance->pptCommChannels    = NULL;
  ptDevInstance->ulCommChannelCount = 0;

  /*-------------------------------------------------*/
  /* Remove Device instance from active devices list */
  /*-------------------------------------------------*/
  for(ulIdx = 0; ulIdx < g_ulDeviceCount; ++ulIdx)
  {
    if(g_pptDevices[ulIdx] == ptDevInstance)
    {
      OS_Memmove(&g_pptDevices[ulIdx],
                 &g_pptDevices[ulIdx + 1],
                 (g_ulDeviceCount - ulIdx - 1) * (uint32_t)sizeof(*g_pptDevices));
      --g_ulDeviceCount;
      break;
    }
  }

  /*-------------------------------------------------*/
  /* Check if we have removed the last device        */
  /*-------------------------------------------------*/
  if(0 == g_ulDeviceCount)
  {
    /* No more devices available */
    OS_Memfree(g_pptDevices);
    g_pptDevices = NULL;

  } else
  {
    /* More device existing, shrink memory */
    g_pptDevices = (PDEVICEINSTANCE*)OS_Memrealloc(g_pptDevices, g_ulDeviceCount * (uint32_t)sizeof(*g_pptDevices));

    if (NULL == g_pptDevices)
    {
      lRet = CIFX_INVALID_POINTER;

      if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                  CIFX_TRACE_LEVEL_ERROR,
                  "Error creating device buffer!");
      }
    }
  }

  return lRet;
}

#if 0 // TODO #ifdef CIFX_TOOLKIT_DMA
/*****************************************************************************/
/*! Check DMA buffer configuration.
*   \param ptDevInstance Holding the DMA buffer configuration
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXTKitCheckDMABufferConfig(PDEVICEINSTANCE ptDevInstance)
{
  int32_t  lRet = CIFX_NO_ERROR;
  uint32_t ulBufferIdx;

  /* If we don't have DMA buffers, user does not want DMA on this
     device and xChannelDMAState, etc. will return an error,
     but we need to return OK, to let the toolkit continue
     initialization */
  if(0 == ptDevInstance->ulDMABufferCount)
    return CIFX_NO_ERROR;

  /* Check DMA channel count */
  if( ptDevInstance->ulDMABufferCount < CIFX_DMA_BUFFER_COUNT)
    return CIFX_DEV_DMA_INSUFF_BUFFER_COUNT;

  /* Check DMA buffer sizes */
  for( ulBufferIdx = 0; ulBufferIdx < ptDevInstance->ulDMABufferCount; ulBufferIdx++)
  {
    if( ptDevInstance->atDmaBuffers[ulBufferIdx].ulSize < CIFX_DMA_MODULO_SIZE)
      lRet = CIFX_DEV_DMA_BUFFER_TOO_SMALL;
    else if( ptDevInstance->atDmaBuffers[ulBufferIdx].ulSize > CIFX_DMA_MAX_BUFFER_SIZE)
      lRet = CIFX_DEV_DMA_BUFFER_TOO_BIG;
    else if( 0 != (ptDevInstance->atDmaBuffers[ulBufferIdx].ulSize % CIFX_DMA_MODULO_SIZE))
      lRet = CIFX_DEV_DMA_BUFFER_NOT_ALIGNED;

    if( CIFX_NO_ERROR != lRet)
      break;
  }

  return lRet;
}
#endif

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
static int HIFcifXTKitISRHandler(PDEVICEINSTANCE ptDevInstance, int fPCIIgnoreGlobalIntFlag)
{
  int iRet;

  /* Check if DPM is available, if not, it cannot be our card, that caused the interrupt */
  if (HWIF_READ32(ptDevInstance, *(uint32_t*)ptDevInstance->pbDPM) == CIFX_DPM_INVALID_CONTENT)
    return CIFX_TKIT_IRQ_OTHERDEVICE;

  if (!ptDevInstance->fIrqEnabled)
  {
    /* Irq is disabled on device, so we assume the user activated the interrupts,
       but wants to poll the card. */

    USER_Trace(ptDevInstance,
      CIFX_TRACE_LEVEL_ERROR,
      "cifXTKitISRHandler() : We received an interrupt, but IRQs are disabled!");

    iRet = CIFX_TKIT_IRQ_OTHERDEVICE;

  } else
  {
    /* We are working in interrupt mode */
    int                             iIrqToDsrBuffer   = ptDevInstance->iIrqToDsrBuffer;
    IRQ_TO_DSR_BUFFER_T*            ptIsrToDsrBuffer  = &ptDevInstance->atIrqToDsrBuffer[iIrqToDsrBuffer];
    HIL_HIF_CHIP_CONTROL_CHANNEL_T* ptGlobalRegisters =
        (HIL_HIF_CHIP_CONTROL_CHANNEL_T*) ptDevInstance->pvGlobalRegisters;

    uint32_t ulIrqStatus = LE32_TO_HOST(HWIF_READ32(ptDevInstance, *ptDevInstance->pptCommChannels[0]->tIoArea.tTlbCtl.pulTlbNetxStatus));

    /* on a DPM module every handshake cell can be read individually,
       on a PCI module the complete handshake register block must be read sequentially. */
    if (!ptDevInstance->fPCICard)
    {
      /* DPM card */

      ++ptDevInstance->ulIrqCounter;
      ptIsrToDsrBuffer->fValid = 1;

      /* IO input and output use the same TLB status/control register.
       * First acknowledge the interrupts, then read the handshake cells. */
      ptIsrToDsrBuffer->ulTlbStatus = ulIrqStatus;

      /* acknowledge HSC IRQs via sms_hsc_irq_clr register */
      HWIF_WRITE32(ptDevInstance, ptGlobalRegisters->tHandshakeIrq.ulIrqClr, HOST_TO_LE32(ptIsrToDsrBuffer->ulTlbStatus) & 0x000000ff);

      /* disable IO-Buffer IRQs if set */
      HWIF_WRITE32(ptDevInstance, ptGlobalRegisters->tHostIrq.ulIrqMaskReset, HOST_TO_LE32(ptIsrToDsrBuffer->ulTlbStatus) & 0x0000ff00);

      /* Read the complete handshake block on DPM hardwares to make sure illegally activated
       * handshake cells don't cause interrupts. */
      HWIF_READN(ptDevInstance,
        ptIsrToDsrBuffer->aulHsk,
        ptDevInstance->pulHandshakeBlock,
        sizeof(ptIsrToDsrBuffer->aulHsk));

      /* Check in DSR which handshake bits have changed. */
      iRet = CIFX_TKIT_IRQ_DSR_REQUESTED;

    } else
    {
      /* PCI card */

      /* First check if we have generated this interrupt. */
      if (0 == ulIrqStatus)
      {
        /* We have not generated this interrupt, so it must be another device on shared IRQ. */
        iRet = CIFX_TKIT_IRQ_OTHERDEVICE;

      } else
      {
        ++ptDevInstance->ulIrqCounter;
        ptIsrToDsrBuffer->fValid = 1;

        /* IO input and output use the same TLB status/control register.
         * First acknowledge the interrupts, then read the handshake cells. */
        ptIsrToDsrBuffer->ulTlbStatus = ulIrqStatus;

        /* acknowledge HSC IRQs via sms_hsc_irq_clr register */
        HWIF_WRITE32(ptDevInstance, ptGlobalRegisters->tHandshakeIrq.ulIrqClr, HOST_TO_LE32(ptIsrToDsrBuffer->ulTlbStatus) & 0x000000ff);

        /* disable IO-Buffer IRQs if set */
        HWIF_WRITE32(ptDevInstance, ptGlobalRegisters->tHostIrq.ulIrqMaskReset, HOST_TO_LE32(ptIsrToDsrBuffer->ulTlbStatus) & 0x0000ff00);

        /* Read the complete handshake block on DPM hardwares to make sure illegally activated
         * handshake cells don't cause interrupts. */
        HWIF_READN(ptDevInstance,
          ptIsrToDsrBuffer->aulHsk,
          ptDevInstance->pulHandshakeBlock,
          sizeof(ptIsrToDsrBuffer->aulHsk));

        /* Check in DSR which handshake bits have changed. */
        iRet = CIFX_TKIT_IRQ_DSR_REQUESTED;
      }
    }
  }

  return iRet;
}

/*****************************************************************************/
/*! Process Input Areas for changes / callbacks
*   \param  ptChannel         Channel Instance
*   \param  ptIsrToDsrBuffer  Current ISR to DSR buffer                      */
/*****************************************************************************/
static void ProcessInputAreas(PCHANNELINSTANCE ptChannel, IRQ_TO_DSR_BUFFER_T* ptIsrToDsrBuffer)
{
  uint32_t ulIdx;

  /* Check IO - Input Areas */
  for (ulIdx = 0; ulIdx < ptChannel->tIoArea.ulIOInputAreas; ++ulIdx)
  {
    if (0 != (ptIsrToDsrBuffer->ulTlbStatus &
              ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tBlock.ulBitmask) &&
        0 == ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tIoCtl.bIrqState)
    {
      ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tIoCtl.bIrqState = 1;
      if (NULL != ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tIoCtl.pfnCallback)
      {
        ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tIoCtl.pfnCallback(
          ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tIoCtl.ulNotifyEvent,
          0,
          NULL,
          ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tIoCtl.pvUser);
      }
      OS_SetEvent(ptChannel->tIoArea.aptIOInputAreas[ulIdx]->tIoCtl.pvEvent);
    }
  }
}

/*****************************************************************************/
/*! Process Output Areas for changes / callbacks
*   \param  ptChannel  Channel Instance                                      */
/*****************************************************************************/
static void ProcessOutputAreas(PCHANNELINSTANCE ptChannel)
{
  uint32_t ulIdx;
  uint8_t bStatus;

  /* Check IO - Output Areas */
  for (ulIdx = 0; ulIdx < ptChannel->tIoArea.ulIOOutputAreas; ++ulIdx)
  {
    bStatus = HWIF_READ8(ptChannel->pvDeviceInstance, *ptChannel->tIoArea.aptIOOutputAreas[ulIdx]->tIoCtl.pbStatus);
    bStatus = (bStatus & NETX_IO_STATUS_LOCKSTATE_MSK);

    /* Check IO - Output Area */
    if (0 == bStatus)
    {
      if (NULL != ptChannel->tIoArea.aptIOOutputAreas[ulIdx]->tIoCtl.pfnCallback)
      {
        ptChannel->tIoArea.aptIOOutputAreas[ulIdx]->tIoCtl.pfnCallback(
          ptChannel->tIoArea.aptIOOutputAreas[ulIdx]->tIoCtl.ulNotifyEvent,
          0,
          NULL,
          ptChannel->tIoArea.aptIOOutputAreas[ulIdx]->tIoCtl.pvUser);
      }
      OS_SetEvent(ptChannel->tIoArea.aptIOOutputAreas[ulIdx]->tIoCtl.pvEvent);
    }
  }
}

/*****************************************************************************/
/*! Deferred interrupt handler
*   \param ptDevInstance Instance the DSR is requested for                   */
/*****************************************************************************/
static void HIFcifXTKitDSRHandler(PDEVICEINSTANCE ptDevInstance)
{
  if (!ptDevInstance->fResetActive)
  {
    /* Get actual data buffer index */
    PCHANNELINSTANCE      ptChannel = &ptDevInstance->tSystemDevice;
    uint32_t              ulChannel = 0;
    int                   iIrqToDsrBuffer = 0;
    IRQ_TO_DSR_BUFFER_T*  ptIsrToDsrBuffer = NULL;

#ifdef CIFX_TOOLKIT_ENABLE_DSR_LOCK
    /* Lock against ISR */
    OS_IrqLock(ptDevInstance->pvOSDependent);
#else

    /* ATTENTION: The IrqToDsr Buffer handling implies a "always" higher priority */
    /*            of the ISR function. This does usually happens on physical ISR functions */
    /*            but does not work if the ISR and DSR are handled as a threads! */

#endif

    iIrqToDsrBuffer = ptDevInstance->iIrqToDsrBuffer;
    ptIsrToDsrBuffer = &ptDevInstance->atIrqToDsrBuffer[iIrqToDsrBuffer];

    if (!ptIsrToDsrBuffer->fValid)
    {
      /* Interrupt did not provide data yet */

#ifdef CIFX_TOOLKIT_ENABLE_DSR_LOCK
      /* Release lock against ISR */
      OS_IrqUnlock(ptDevInstance->pvOSDependent);
#endif

      return;
    }
    else
    {
      /* Flip data buffer so IRQ uses the other buffer */
      ptDevInstance->iIrqToDsrBuffer ^= 0x01;

      /* Invalidate the buffer, we are now handling */
      ptIsrToDsrBuffer->fValid = 0;
    }

#ifdef CIFX_TOOLKIT_ENABLE_DSR_LOCK
    /* Release lock against ISR */
    OS_IrqUnlock(ptDevInstance->pvOSDependent);
#endif

    /* Only process rest of flags if HIL_HIF_NSF_READY is set. This must be done to prevent
       confusion of the toolkit during a system start (xSysdeviceReset) */
    if (ptIsrToDsrBuffer->aulHsk[HIL_HIF_HSC_NETX_SYS] & HIL_HIF_NSF_READY)
    {
#if 0 // TODO sync handling
      /*--------------------------------------------------------------------*/
      /* Evaluate device synchronization flags, the flags are fixed 16 Bit  */
      /*--------------------------------------------------------------------*/
      uint16_t  usChangedSyncBits;
      uint16_t  usOldNSyncFlags = ptDevInstance->tSyncData.usNSyncFlags; /* Remember last known netX flags */

      /* Get pointer to the new flag data from ISR */
      HIL_DPM_HANDSHAKE_CELL_T* ptSyncCell = &ptIsrToDsrBuffer->tHandshakeBuffer.atHsk[NETX_HSK_SYNCH_FLAG_POS];

      /* Get the actual flags */
      ptDevInstance->tSyncData.usNSyncFlags = LE16_TO_HOST(ptSyncCell->t16Bit.ulNetxFlags);

      /* Check if there are changed bits since last interrupt from netX side,  */
      /* and only process sync if bits have chanded! */
      if (0 != (usChangedSyncBits = usOldNSyncFlags ^ ptDevInstance->tSyncData.usNSyncFlags))
      {
        uint32_t  ulBitPos;
        uint16_t  usUnequalSyncBits;

        /* Create unequal bit mask */
        usUnequalSyncBits = ptDevInstance->tSyncData.usNSyncFlags ^ ptDevInstance->tSyncData.usHSyncFlags;

        /* Signal sync events */
        for (ulBitPos = 0; ulBitPos < NETX_NUM_OF_SYNCH_FLAGS; ++ulBitPos)
        {
          /* There is a valid channel */
          uint16_t          usBitMask = (uint16_t)(1 << ulBitPos);
          PCHANNELINSTANCE  ptSyncChannel = NULL;

          if (ulBitPos >= ptDevInstance->ulCommChannelCount)
            break;

          ptSyncChannel = (PCHANNELINSTANCE)ptDevInstance->pptCommChannels[ulBitPos];

          if (usChangedSyncBits & usBitMask)
          {
            uint8_t bState = HIL_FLAGS_NOT_EQUAL;
            int     fProcess = 0;

            /* Handle Sync interrupts, read actual state and set bState accordingly */
            if (HIL_SYNC_MODE_HST_CTRL == HWIF_READ8(ptDevInstance, ptSyncChannel->ptCommunicationStatusBlock->bSyncHskMode))
              bState = HIL_FLAGS_EQUAL;

            /* Check which mode to handle */
            /* HIL_FLAGS_NOT_EQUAL corresponds to DEVICE_CONTROLLED */
            if ((bState == HIL_FLAGS_NOT_EQUAL) &&
              (usUnequalSyncBits & usBitMask))
            {
              fProcess = 1;

            }
            else if ((bState == HIL_FLAGS_EQUAL) &&
              (0 == (usUnequalSyncBits & usBitMask)))
            {
              fProcess = 1;
            }

            if (fProcess)
            {
              /* There is a valid channel */
              /* Check if we have a callback assigned */
              if (ptSyncChannel->tSynch.pfnCallback)
                ptSyncChannel->tSynch.pfnCallback(CIFX_NOTIFY_SYNC, 0, NULL, ptSyncChannel->tSynch.pvUser);

              /* Signal event to allow waiting for sync state without callback */
              if (ptDevInstance->tSyncData.ahSyncBitEvents[ulBitPos])
                OS_SetEvent(ptDevInstance->tSyncData.ahSyncBitEvents[ulBitPos]);
            }
          }
        }
      }
#endif

      /*-----------------------------------------------------*/
      /* Evaluate all changed handshake bits on all cells    */
      /*-----------------------------------------------------*/

      do {
        if (!ptChannel->fIsSysDevice)
        {
          uint32_t ulChangedBits;
          uint32_t ulOldNetxFlags;

          /* COMMUNICATION flags */

          ulOldNetxFlags = ptChannel->tHsCtrl.ulNetxFlags; /* Remember last known netX flags */
          ptChannel->tHsCtrl.ulNetxFlags = LE32_TO_HOST(ptIsrToDsrBuffer->aulHsk[HIL_HIF_HSC_NETX_COM]);

          /* Check which bits have changed since last interrupt from netX side */
          ulChangedBits = ulOldNetxFlags ^ ptChannel->tHsCtrl.ulNetxFlags;

          /* Check COMM state */
          if (ulChangedBits & HIL_HIF_NCF_COMMUNICATING)
          {
            OS_SetEvent(ptChannel->apvHsBitEvent[HIL_HIF_NCF_COMMUNICATING_BIT_NO]);

            /* check if notification is registered */
            if (NULL != ptChannel->tComState.pfnCallback)
            {
              CIFX_NOTIFY_COM_STATE_T tData = {
                  .ulComState = (ptChannel->tHsCtrl.ulNetxFlags & HIL_HIF_NCF_COMMUNICATING) >> HIL_HIF_NCF_COMMUNICATING_BIT_NO };
              ptChannel->tComState.pfnCallback(CIFX_NOTIFY_COM_STATE,
                sizeof(tData),
                &tData,
                ptChannel->tComState.pvUser);
            }
          }

          /* Store current TlbStatus and reset the changes. */
          ptChannel->tIoArea.tTlbCtl.ulTlbNetxStatus = ptIsrToDsrBuffer->ulTlbStatus;
          ProcessInputAreas(ptChannel, ptIsrToDsrBuffer);
          ProcessOutputAreas(ptChannel);

          /* Check COMM Send Mailbox */
          ulOldNetxFlags = ptChannel->tFromHostMbx.tCom.tCtl.ulNetxFlags; /* Remember last known netX flags */
          ptChannel->tFromHostMbx.tCom.tCtl.ulNetxFlags = LE32_TO_HOST(ptIsrToDsrBuffer->aulHsk[HIL_HIF_HSC_NETX_SNDMBX]);

          if ((ulOldNetxFlags & (HIL_HIF_MBX_IDX_MASK | HIL_HIF_MBX_WRAPAROUND)) !=
            (ptChannel->tFromHostMbx.tCom.tCtl.ulNetxFlags & (HIL_HIF_MBX_IDX_MASK | HIL_HIF_MBX_WRAPAROUND)))
          {
            uint32_t ulFillLevel = DEV_GetMBXFillLevel(&ptChannel->tFromHostMbx.tCom);
            if (ulFillLevel < ptChannel->tFromHostMbx.tCom.tBlock.ulElementCnt)
            {
              if (NULL != ptChannel->tFromHostMbx.tCom.tCtl.pfnCallback)
              {
                CIFX_NOTIFY_TX_MBX_EMPTY_DATA_T tTxData = { .ulMaxSendCount = ptChannel->tFromHostMbx.tCom.tBlock.ulElementCnt - ulFillLevel };
                ptChannel->tFromHostMbx.tCom.tCtl.pfnCallback(CIFX_NOTIFY_TX_MBX_EMPTY,
                  sizeof(tTxData),
                  &tTxData,
                  ptChannel->tFromHostMbx.tCom.tCtl.pvUser);
              }
              OS_SetEvent(ptChannel->tFromHostMbx.tCom.tCtl.pvEvent);
            }
          }

          /* Check COMM Receive Mailbox */
          ulOldNetxFlags = ptChannel->tToHostMbx.tCom.tCtl.ulNetxFlags; /* Remember last known netX flags */
          ptChannel->tToHostMbx.tCom.tCtl.ulNetxFlags = LE32_TO_HOST(ptIsrToDsrBuffer->aulHsk[HIL_HIF_HSC_NETX_RCVMBX]);

          if (ulOldNetxFlags != ptChannel->tToHostMbx.tCom.tCtl.ulNetxFlags)
          {
            uint32_t ulFillLevel = DEV_GetMBXFillLevel(&ptChannel->tToHostMbx.tCom);
            if (ulFillLevel > 0)
            {
              if (NULL != ptChannel->tToHostMbx.tCom.tCtl.pfnCallback)
              {
                CIFX_NOTIFY_RX_MBX_FULL_DATA_T tRxData = { .ulRecvCount = ulFillLevel };
                ptChannel->tToHostMbx.tCom.tCtl.pfnCallback(CIFX_NOTIFY_RX_MBX_FULL,
                  sizeof(tRxData),
                  &tRxData,
                  ptChannel->tToHostMbx.tCom.tCtl.pvUser);
              }
              OS_SetEvent(ptChannel->tToHostMbx.tCom.tCtl.pvEvent);
            }
          }
        }
        else
        {
          uint32_t ulChangedBits;
          uint32_t ulOldNetxFlags;

          /* SYSTEM flags */

          ulOldNetxFlags = ptChannel->tHsCtrl.ulNetxFlags; /* Remember last known netX flags */
          ptChannel->tHsCtrl.ulNetxFlags = LE32_TO_HOST(ptIsrToDsrBuffer->aulHsk[HIL_HIF_HSC_NETX_SYS]);

          /* Check which bits have changed since last interrupt from netX side */
          ulChangedBits = ulOldNetxFlags ^ ptChannel->tHsCtrl.ulNetxFlags;

          /*------------------------------------------*/
          /* Process the send receive MBX flags       */
          /*------------------------------------------*/
          /* Check Receive Mailbox */
          if (ulChangedBits & ptChannel->tToHostMbx.tSys.ulBitmask)
          {
            OS_SetEvent(ptChannel->apvHsBitEvent[ptChannel->tToHostMbx.tSys.bBitoffset]);
          }

          /* Check Send Mailbox */
          if (ulChangedBits & ptChannel->tFromHostMbx.tSys.ulBitmask)
          {
            OS_SetEvent(ptChannel->apvHsBitEvent[ptChannel->tFromHostMbx.tSys.bBitoffset]);
          }
        }

        if (ulChannel < ptDevInstance->ulCommChannelCount)
          ptChannel = ptDevInstance->pptCommChannels[ulChannel];

      } while (ulChannel++ < ptDevInstance->ulCommChannelCount);
    }
  }
}

/*****************************************************************************/
/*! Physically Enable Interrupts on hardware
*   \param ptDevInstance Device instance                                     */
/*****************************************************************************/
static void HIFcifXTKitEnableHWInterrupt(PDEVICEINSTANCE ptDevInstance)
{
  HIL_HIF_CHIP_CONTROL_CHANNEL_T* ptGlobalRegisters =
      (HIL_HIF_CHIP_CONTROL_CHANNEL_T*)ptDevInstance->pvGlobalRegisters;
  HWIF_WRITE32(ptDevInstance, ptGlobalRegisters->tHostIrq.ulIrqMaskSet, 0x0000FFFF);
}

/*****************************************************************************/
/*! Physically Disable Interrupts on hardware
*   \param ptDevInstance Device instance                                     */
/*****************************************************************************/
static void HIFcifXTKitDisableHWInterrupt(PDEVICEINSTANCE ptDevInstance)
{
  HIL_HIF_CHIP_CONTROL_CHANNEL_T* ptGlobalRegisters =
      (HIL_HIF_CHIP_CONTROL_CHANNEL_T*)ptDevInstance->pvGlobalRegisters;
  HWIF_WRITE32(ptDevInstance, ptGlobalRegisters->tHostIrq.ulIrqMaskReset, 0x0000FFFF);
}

/*****************************************************************************/
/*! Adds a newly found device to the list of handled device
*   \param ptDevInstance Device to add (must at least include the pointer to
*                        the DPM)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t HIFcifXTKitAddDevice(PDEVICEINSTANCE ptDevInstance)
{
  int32_t lRet;

  /* Check if we have a pointer */
  if(NULL == ptDevInstance)
    return CIFX_INVALID_POINTER;

  /* Disable interrupts during startup phase. Just in case the user has set this flag! */
  ptDevInstance->fIrqEnabled = 0;

  ptDevInstance->ptTkitFun = cifXTkitGetHifTkitFunctionList();
  ptDevInstance->ptCifxFun = cifXTkitGetHifApiFunctionList();
  ptDevInstance->ptDevFun  = cifXTkitGetHifDevFunctionList();

#ifdef CIFX_TOOLKIT_HWIF
  /* Validate hardware access function pointers != NULL */
  if ( (ptDevInstance->pfnHwIfRead    == NULL) ||
       (ptDevInstance->pfnHwIfWrite   == NULL)   )
    return CIFX_INVALID_PARAMETER;
#endif /* CIFX_TOOLKIT_HWIF */

#if 0 // TODO #ifdef CIFX_TOOLKIT_DMA
  /* Check DMA handling just for PCI hardware */
  if( ptDevInstance->fPCICard)
  {
    /* Check DMA buffer configuration */
    if(CIFX_NO_ERROR != (lRet = cifXTKitCheckDMABufferConfig( ptDevInstance)))
      return lRet;
  }
#endif

  /* Run the toolkit start device functions */
  lRet = cifXStartDevice(ptDevInstance);
  if(CIFX_NO_ERROR == lRet)
  {
    /* Lock tkit global data access against reentrancy*/
    OS_EnterLock(g_pvTkitLock);

    /* Increment device count */
    ++g_ulDeviceCount;

    /* Create new list entry */
    g_pptDevices = (PDEVICEINSTANCE*)OS_Memrealloc(g_pptDevices, g_ulDeviceCount * (uint32_t)sizeof(*g_pptDevices));

    if (NULL == g_pptDevices)
    {
      lRet = CIFX_INVALID_POINTER;

      if(g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                  CIFX_TRACE_LEVEL_ERROR,
                  "Error creating device list buffer!");
      }

    } else
    {
      /* Add the new entry to the device list */
      g_pptDevices[g_ulDeviceCount - 1] = ptDevInstance;

      /* Setup interrupts if as given during cifXStartDevice() */
      if(0 != (ptDevInstance->fIrqEnabled))
      {
        /* Perform a dummy interrupt cycle to get handshake flags in Sync for proper operation */
        if(CIFX_TKIT_IRQ_DSR_REQUESTED == HIFcifXTKitISRHandler(ptDevInstance, 1))
          HIFcifXTKitDSRHandler(ptDevInstance);

#ifndef CIFX_TOOLKIT_MANUAL_IRQ_ENABLE
        OS_EnableInterrupts(ptDevInstance->pvOSDependent);
        HIFcifXTKitEnableHWInterrupt(ptDevInstance);
#endif /* CIFX_TOOLKIT_MANUAL_IRQ_ENABLE */
      }
    }

    /* Done with the initialisation */
    OS_LeaveLock(g_pvTkitLock);
  }

  return lRet;
}

/*****************************************************************************/
/*! This functions removes a device from being handled by the toolkit.
*   \param szBoard        Name or Alias of the board to remove
*   \param fForceRemove   !=0 to force the release of the device, even if
*                         any references to the device are open
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t HIFcifXTKitRemoveDevice(char* szBoard, int fForceRemove)
{
  int32_t  lRet   = CIFX_INVALID_BOARD;
  int      fFound = 0;
  uint32_t ulIdx  = 0;

  OS_EnterLock(g_pvTkitLock);

  /* Check if a device with the given name still exists */
  for(ulIdx = 0; ulIdx < g_ulDeviceCount; ++ulIdx)
  {
    if( (OS_Strcmp(g_pptDevices[ulIdx]->szName,  szBoard) == 0) ||
        (OS_Strcmp(g_pptDevices[ulIdx]->szAlias, szBoard) == 0) )
    {
      fFound = 1;
      break;
    }
  }

  /* Remove only devices which are available */
  if(fFound)
  {
    PDEVICEINSTANCE ptDevInstance = g_pptDevices[ulIdx];
    int             fStop         = 0;
    int             fIrqEnabled   = ptDevInstance->fIrqEnabled;

    if(ptDevInstance->fIrqEnabled)
    {
#ifndef CIFX_TOOLKIT_MANUAL_IRQ_ENABLE
      HIFcifXTKitDisableHWInterrupt(ptDevInstance);
      OS_DisableInterrupts(ptDevInstance->pvOSDependent);
#endif /* CIFX_TOOLKIT_MANUAL_IRQ_ENABLE */

      /* mark IRQ as disabled, as the device is now in polling mode */
      ptDevInstance->fIrqEnabled = 0;
    }

    if(fForceRemove)
    {
      /* user requested to force the remove, so don't check for open connections */
      fStop = 1;
    } else
    {
      uint32_t ulChannel = 0;

      if(ptDevInstance->tSystemDevice.ulOpenCount != 0)
      {
        /* system channel is in use, so deny device removal */
        lRet = CIFX_DEV_HW_PORT_IS_USED;
      } else
      {
        fStop = 1;
        /* we need to check if any channel has an open reference */
        for(ulChannel = 0; ulChannel < ptDevInstance->ulCommChannelCount; ++ulChannel)
        {
          if(ptDevInstance->pptCommChannels[ulChannel]->ulOpenCount > 0)
          {
            /* at least one channel has an open reference */
            fStop = 0;
            lRet = CIFX_DEV_HW_PORT_IS_USED;
            break;
          }
        }
      }
    }

    if(fStop)
      lRet = cifXStopDevice(ptDevInstance);

    /* Restore IRQ mode in case the user wants to reuse this device instance */
    ptDevInstance->fIrqEnabled = fIrqEnabled;
  }

  OS_LeaveLock(g_pvTkitLock);

  return lRet;
}

/*****************************************************************************/
/*! Un-Initializes the cifX Toolkit                                          */
/*****************************************************************************/
static void HIFcifXTKitDeinit( void)
{
  int32_t lIdx = 0;

  if(g_pvTkitLock)
  {
    OS_EnterLock(g_pvTkitLock);
  }

  /* g_ulDeviceCount is decremented inside cifXStopDevice() */
  for(lIdx = g_ulDeviceCount-1; lIdx >= 0; lIdx--)
  {
    (void)cifXStopDevice(g_pptDevices[lIdx]);
  }

  if (0 == g_ulDeviceCount)
  {
    if(g_pptDevices)
    {
      OS_Memfree(g_pptDevices);
      g_pptDevices    = NULL;
    }

    if(g_pvTkitLock)
    {
      OS_LeaveLock(g_pvTkitLock);
      OS_DeleteLock(g_pvTkitLock);
      g_pvTkitLock = NULL;
    }

    /* Uninitialize OS functions */
    OS_Deinit();

    g_tDriverInfo.fInitialized = 0;
    g_tDriverInfo.ulOpenCount  = 0;
  } else
  {
    OS_LeaveLock(g_pvTkitLock);
  }
}

/*****************************************************************************/
/*! Initializes the cifX Toolkit
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t HIFcifXTKitInit(void)
{
  int32_t lRet = CIFX_NO_ERROR;

  /* Uninitialize toolkit, just in case it was not correctly closed before */
  HIFcifXTKitDeinit();

  /* Initialize OS functions */
  lRet = OS_Init();

  /* Create toolkit lock, signal toolkit initialization */
  if(CIFX_NO_ERROR == lRet)
  {
    if( NULL == (g_pvTkitLock = OS_CreateLock()) )
    {
      /* Signal initialization error */
      lRet = CIFX_INVALID_POINTER;

      /* Uninitialize OS functions */
      OS_Deinit();
    } else
    {
      /* Toolkit successfully initialized */
      g_tDriverInfo.fInitialized = 1;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Local structure for cifX Toolkit function pointers                       */
/*****************************************************************************/
static CIFX_TKIT_FUNCTION_LIST_T s_tCifxHifTkitFuns =
{
  HIFcifXTKitInit,
  HIFcifXTKitDeinit,
  HIFcifXTKitAddDevice,
  HIFcifXTKitRemoveDevice,
  HIFcifXTKitEnableHWInterrupt,
  HIFcifXTKitDisableHWInterrupt,
  HIFcifXTKitISRHandler,
  HIFcifXTKitDSRHandler,
  NULL,
};

PCIFX_TKIT_FUNCTION_LIST_T cifXTkitGetHifTkitFunctionList(void)
{
  return &s_tCifxHifTkitFuns;
}

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/
