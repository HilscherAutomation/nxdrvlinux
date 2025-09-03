
#ifndef APITRANSLATE__H
#define APITRANSLATE__H

#include <string.h>

#include "cifXErrors.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CALL_FUNC(dev, type, _function, ...) (dev->type->pfn##_function (__VA_ARGS__))

#define CALL_TK_FUNC( dev, _function, ...) CALL_FUNC(dev, ptTkitFun, _function, __VA_ARGS__)
#define CALL_DEV_FUNC(dev, _function, ...) CALL_FUNC(dev, ptDevFun,  _function, __VA_ARGS__)
#define CALL_API_FUNC(dev, _function, ...) (dev == NULL) ? CIFX_INVALID_PARAMETER : CALL_FUNC(dev, ptCifxFun, _function, __VA_ARGS__)

/* use a fake structure here not to expose the whole content to user space */
struct FAKE_CHANNELINSTANCE {
  void* pvDeviceInstance;
};

struct FAKE_DEVICEINSTANCE {
  struct CIFX_API_FUNCTION_LIST_Ttag*   ptCifxFun;  /*!< Function pointer to CIFX API functions */
  struct CIFX_DEV_FUNCTION_LIST_Ttag*   ptDevFun;   /*!< Function pointer to DEV API functions  */
  struct CIFX_TKIT_FUNCTION_LIST_Ttag*  ptTkitFun;
};

#define HSYS_TO_DEVINST(x)  (x == NULL ? NULL : ((struct FAKE_DEVICEINSTANCE*)(((struct FAKE_CHANNELINSTANCE*)(x))->pvDeviceInstance)))
#define HCHAN_TO_DEVINST(x) (x == NULL ? NULL : ((struct FAKE_DEVICEINSTANCE*)(((struct FAKE_CHANNELINSTANCE*)(x))->pvDeviceInstance)))

#ifdef __cplusplus
}
#endif

#endif //APITRANSLATE__H
