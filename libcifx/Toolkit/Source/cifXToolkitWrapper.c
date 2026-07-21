/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXToolkitWrapper.c 15335 2025-11-26 09:42:58Z AMinor $:

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

int32_t cifXTKitInit(void)
{
  int32_t lRet = CIFX_NO_ERROR;

  if (g_tDriverInfo.fInitialized)
    return CIFX_NO_ERROR;

#if DPM_SUPPORT
  s_pDPMTKfun = cifXTkitGetDpmTkitFunctionList(); /* returns valid pointer */
  lRet = s_pDPMTKfun->pfncifXTKitInit();
#endif

#if HIF_SUPPORT
  s_pHIFTKfun = cifXTkitGetHifTkitFunctionList(); /* returns valid pointer */
  if (CIFX_NO_ERROR == lRet)
  {
    lRet = s_pHIFTKfun->pfncifXTKitInit();
#if DPM_SUPPORT
    if (CIFX_NO_ERROR != lRet)
      s_pDPMTKfun->pfncifXTKitDeinit();
#endif
  }
#endif

  return lRet;
}

void cifXTKitDeinit(void)
{
  if (!g_tDriverInfo.fInitialized)
    return;

#if DPM_SUPPORT
  s_pDPMTKfun->pfncifXTKitDeinit();
  s_pDPMTKfun = NULL;
#endif

#if HIF_SUPPORT
  s_pHIFTKfun->pfncifXTKitDeinit();
  s_pHIFTKfun = NULL;
#endif
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
  PFN_CIFXTKITREMOVEDEVICE pfncifXTKitRemoveDevice = NULL;
  int32_t lRet = CIFX_FUNCTION_FAILED;
  uint32_t ulIdx;

  CHECK_POINTER(szBoard);

  if (!g_tDriverInfo.fInitialized)
    return CIFX_DRV_DRIVER_NOT_LOADED;

  OS_EnterLock(g_pvTkitLock);

  /* Check if a device with the given name still exists */
  for (ulIdx = 0; ulIdx < g_ulDeviceCount; ++ulIdx)
  {
    if( (OS_Strcmp(g_pptDevices[ulIdx]->szName,  szBoard) == 0) ||
        (OS_Strcmp(g_pptDevices[ulIdx]->szAlias, szBoard) == 0) )
    {
      /* get function pointer to make sure device is not removed since we can't hold lock */
      pfncifXTKitRemoveDevice = g_pptDevices[ulIdx]->ptTkitFun->pfncifXTKitRemoveDevice;
      break;
    }
  }

  OS_LeaveLock(g_pvTkitLock);

  if (NULL != pfncifXTKitRemoveDevice)
  {
    lRet = pfncifXTKitRemoveDevice(szBoard, fForceRemove);
  }

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
