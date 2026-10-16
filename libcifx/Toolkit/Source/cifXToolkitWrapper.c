/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXToolkitWrapper.c 15625 2026-10-06 10:21:13Z AMinor $:

  Description:
    cifX Toolkit Wrapper module

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-11-20  created

**************************************************************************************/
#include "cifXFunctionList.h"

extern TKIT_DRIVER_INFORMATION g_tDriverInfo;
extern PDEVICEINSTANCE* g_pptDevices;
extern uint32_t g_ulDeviceCount;
extern void* g_pvTkitLock;

#if DPM_SUPPORT
static CIFX_TKIT_FUNCTION_LIST_T* s_pDPMTKfun = NULL;
#endif

#if HIF_SUPPORT
static CIFX_TKIT_FUNCTION_LIST_T* s_pHIFTKfun = NULL;
#endif

void cifXTKitDeinit(void)
{
  int32_t lIdx = 0;

  if (!g_tDriverInfo.fInitialized)
    return;

  if(g_pvTkitLock)
  {
    OS_EnterLock(g_pvTkitLock);
  }

  /* g_ulDeviceCount is decremented inside cifXStopDevice() */
  for(lIdx = g_ulDeviceCount-1; lIdx >= 0; lIdx--)
  {
    (void)CALL_TK_FUNC(g_pptDevices[lIdx], cifXTKitStopDevice, g_pptDevices[lIdx]);
  }

  if (0 == g_ulDeviceCount)
  {
    if(g_pptDevices)
    {
      OS_Memfree(g_pptDevices);
      g_pptDevices    = NULL;
    }

#if DPM_SUPPORT
    s_pDPMTKfun = NULL;
#endif
#if HIF_SUPPORT
    s_pHIFTKfun = NULL;
#endif

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

int32_t cifXTKitInit(void)
{
  int32_t lRet = CIFX_NO_ERROR;

  if (g_tDriverInfo.fInitialized)
    return CIFX_NO_ERROR;

  /* Uninitialize toolkit, just in case it was not correctly closed before */
  cifXTKitDeinit();

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
#if DPM_SUPPORT
      s_pDPMTKfun = cifXTkitGetDpmTkitFunctionList(); /* returns valid pointer */
#endif
#if HIF_SUPPORT
      s_pHIFTKfun = cifXTkitGetHifTkitFunctionList(); /* returns valid pointer */
#endif

      /* Toolkit successfully initialized */
      g_tDriverInfo.fInitialized = 1;
    }
  }

  return lRet;
}

int32_t cifXTKitAddDevice(PDEVICEINSTANCE ptDevInstance)
{
  int32_t lRet = CIFX_DEV_DPM_LAYOUT_UNKNOWN;

  if (!g_tDriverInfo.fInitialized)
    return CIFX_DRV_DRIVER_NOT_LOADED;

  CHECK_POINTER(ptDevInstance);

#if DPM_SUPPORT
  lRet = s_pDPMTKfun->pfncifXTKitAddDevice(ptDevInstance);
#endif

#if HIF_SUPPORT
  if (CIFX_DEV_DPM_LAYOUT_UNKNOWN == lRet)
    lRet = s_pHIFTKfun->pfncifXTKitAddDevice(ptDevInstance);
#endif

  return lRet;
}

int32_t cifXTKitRemoveDevice(char* szBoard, int fForceRemove)
{
  int      fFound = 0;
  uint32_t ulIdx  = 0;
  int32_t  lRet   = CIFX_INVALID_BOARD;

  CHECK_POINTER(szBoard);

  if (!g_tDriverInfo.fInitialized)
    return CIFX_DRV_DRIVER_NOT_LOADED;

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
      CALL_TK_FUNC(ptDevInstance, cifXTKitDisableHWInterrupt, ptDevInstance);
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
      lRet = CALL_TK_FUNC(ptDevInstance, cifXTKitStopDevice, ptDevInstance);

    /* Restore IRQ mode in case the user wants to reuse this device instance */
    ptDevInstance->fIrqEnabled = fIrqEnabled;
  }

  OS_LeaveLock(g_pvTkitLock);

  return lRet;
}

void cifXTKitEnableHWInterrupt(PDEVICEINSTANCE ptDevInstance)
{
#ifdef CIFX_TOOLKIT_PARAMETER_CHECK
  if (NULL == ptDevInstance)
    return;
#endif

  CALL_TK_FUNC(ptDevInstance, cifXTKitEnableHWInterrupt, ptDevInstance);
}

void cifXTKitDisableHWInterrupt(PDEVICEINSTANCE ptDevInstance)
{
#ifdef CIFX_TOOLKIT_PARAMETER_CHECK
  if (NULL == ptDevInstance)
    return;
#endif

  CALL_TK_FUNC(ptDevInstance, cifXTKitDisableHWInterrupt, ptDevInstance);
}

int cifXTKitISRHandler(PDEVICEINSTANCE ptDevInstance, int fPCIIgnoreGlobalIntFlag)
{
  CHECK_POINTER(ptDevInstance);
  return CALL_TK_FUNC(ptDevInstance, cifXTKitISRHandler, ptDevInstance, fPCIIgnoreGlobalIntFlag);
}

void cifXTKitDSRHandler(PDEVICEINSTANCE ptDevInstance)
{
#ifdef CIFX_TOOLKIT_PARAMETER_CHECK
  if (NULL == ptDevInstance)
    return;
#endif

  CALL_TK_FUNC(ptDevInstance, cifXTKitDSRHandler, ptDevInstance);
}

#if DPM_SUPPORT
void cifXTKitCyclicTimer(void)
{
  if (!g_tDriverInfo.fInitialized)
    return;

  /* Only the DPM toolkit needs the cyclic timer. */
  s_pDPMTKfun->pfncifXTKitCyclicTimer();
}
#endif
