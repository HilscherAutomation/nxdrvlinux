
#ifdef CIFX_TOOLKIT_FUNCTION_LIST

#include <stdint.h>
#include <stddef.h>

#include "cifXErrors.h"
#include "cifXFunctionList.h"

int g_api_if_initialized = 0;

CIFX_TKIT_FUNCTION_LIST_T* g_pDPMTKfun = NULL;
CIFX_TKIT_FUNCTION_LIST_T* g_pHIFTKfun = NULL;
CIFX_API_FUNCTION_LIST_T*  g_pDPMAPIfun = NULL;
CIFX_API_FUNCTION_LIST_T*  g_pHIFAPIfun = NULL;

int32_t cifXTKitInit(void) {
    int32_t ret = CIFX_NO_ERROR;

    g_pDPMTKfun = cifXTkitGetDpmTkitFunctionList();
    g_pHIFTKfun = cifXTkitGetHifTkitFunctionList();
    g_pDPMAPIfun = cifXTkitGetDpmApiFunctionList();
    g_pHIFAPIfun = cifXTkitGetHifApiFunctionList();

    if ( (g_pDPMTKfun == NULL) || (g_pHIFTKfun == NULL) || (g_pDPMAPIfun == NULL) || (g_pHIFAPIfun == NULL) )
        return CIFX_FUNCTION_FAILED;

    if ( (ret = g_pDPMTKfun->pfncifXTKitInit()) != CIFX_NO_ERROR)
        return ret;

    if ( (ret = g_pHIFTKfun->pfncifXTKitInit()) != CIFX_NO_ERROR) {
        g_pDPMTKfun->pfncifXTKitDeinit();
        return ret;
    }
    g_api_if_initialized = 1;
    return CIFX_NO_ERROR;
}

void cifXTKitDeinit(void) {
    if (g_api_if_initialized != 0) {
        g_api_if_initialized = 0;
        if (g_pDPMTKfun != NULL) {
            g_pDPMTKfun->pfncifXTKitDeinit();
            g_pDPMTKfun = NULL;
        }
        if (g_pHIFTKfun != NULL) {
            g_pHIFTKfun->pfncifXTKitDeinit();
            g_pHIFTKfun = NULL;
        }
        g_pDPMAPIfun = NULL;
        g_pHIFAPIfun = NULL;
    }
}

int32_t cifXTKitAddDevice(PDEVICEINSTANCE ptDevInstance) {
    int32_t ret = CIFX_FUNCTION_FAILED;

    if (g_api_if_initialized != 1)
        return CIFX_FUNCTION_FAILED;

    ret = g_pDPMTKfun->pfncifXTKitAddDevice( ptDevInstance);
    if (ret == CIFX_DEV_DPM_LAYOUT_UNKNOWN) {
        ret = g_pHIFTKfun->pfncifXTKitAddDevice( ptDevInstance);
    }
    return ret;
}

extern uint32_t         g_ulDeviceCount;
extern PDEVICEINSTANCE* g_pptDevices;
extern void*            g_pvTkitLock;

int32_t cifXTKitRemoveDevice(char* szBoard, int fForceRemove) {
    int32_t ret = CIFX_FUNCTION_FAILED;
    uint32_t ulIdx;
    PFN_CIFXTKITREMOVEDEVICE pfncifXTKitRemoveDevice = NULL;

    if (g_api_if_initialized != 1)
        return CIFX_FUNCTION_FAILED;

    OS_EnterLock(g_pvTkitLock);

    /* Check if a device with the given name still exists */
    for(ulIdx = 0; ulIdx < g_ulDeviceCount; ++ulIdx)
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

    if(pfncifXTKitRemoveDevice != NULL) {
        ret = pfncifXTKitRemoveDevice(szBoard, fForceRemove);
    }
    return ret;
}

void cifXTKitCyclicTimer(void) {
    if (g_api_if_initialized != 1)
        return;

    g_pDPMTKfun->pfncifXTKitCyclicTimer();
}

#endif //ifdef CIFX_TOOLKIT_FUNCTION_LIST
