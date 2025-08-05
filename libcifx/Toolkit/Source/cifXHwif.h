/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXHwif.h 15171 2025-08-05 08:18:45Z AMinor $:

  Description:
    cifX API Hardware handling functions declaration

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-07-09  Created

**************************************************************************************/
#ifndef CIFX_HWIF__H
#define CIFX_HWIF__H

#include "OS_Dependent.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef CIFX_TOOLKIT_HWIF
  typedef void* (*PFN_HWIF_MEMCPY) (uint32_t ulOpt, void* pvDevInstance, void* pvAddr, void* pvData, uint32_t ulLen);

  /*lint -emacro(534, HWIF_READN)  : ignore return value */
  /*lint -emacro(534, HWIF_WRITE*) : ignore return value */
  #define HWIF_READ8(ptDev,  Src) HwIfRead8(ptDev,  (void*)&(Src))
  #define HWIF_READ16(ptDev, Src) HwIfRead16(ptDev, (void*)&(Src))
  #define HWIF_READ32(ptDev, Src) HwIfRead32(ptDev, (void*)&(Src))
  #define HWIF_READN(ptDev,  Dst, Src, Len) ((struct DEVICEINSTANCEtag*)ptDev)->pfnHwIfRead(0, ptDev, (void*)(Src), Dst, Len)
  #define HWIF_WRITE8(ptDev, Dst, Src)                                                           \
  do {                                                                                            \
    uint8_t bData = Src;                                                                          \
    ((struct DEVICEINSTANCEtag*)ptDev)->pfnHwIfWrite(1, ptDev, (void*)&(Dst), (void*)&bData, 1);  \
  } while (0);
  #define HWIF_WRITE16(ptDev, Dst, Src)                                                           \
  do {                                                                                            \
    uint16_t uiData = Src;                                                                        \
    ((struct DEVICEINSTANCEtag*)ptDev)->pfnHwIfWrite(1, ptDev, (void*)&(Dst), (void*)&uiData, 2); \
  } while (0);
  #define HWIF_WRITE32(ptDev, Dst, Src)                                                           \
  do {                                                                                            \
    uint32_t ulData = Src;                                                                        \
    ((struct DEVICEINSTANCEtag*)ptDev)->pfnHwIfWrite(1, ptDev, (void*)&(Dst), (void*)&ulData, 4); \
  } while (0);
  #define HWIF_WRITEN(ptDev, Dst, Src, Len) ((struct DEVICEINSTANCEtag*)ptDev)->pfnHwIfWrite(0, ptDev, (void*)(Dst), Src, Len)

  struct DEVICEINSTANCEtag;
  uint8_t  HwIfRead8  (struct DEVICEINSTANCEtag* ptDev, void* pvSrc);
  uint16_t HwIfRead16 (struct DEVICEINSTANCEtag* ptDev, void* pvSrc);
  uint32_t HwIfRead32 (struct DEVICEINSTANCEtag* ptDev, void* pvSrc);

#else
  #define HWIF_READ8(ptDev,   Src) Src
  #define HWIF_READ16(ptDev,  Src) Src
  #define HWIF_READ32(ptDev,  Src) Src
  #define HWIF_READN(ptDev,   Dst, Src, Len) OS_Memcpy(Dst, Src, Len)
  #define HWIF_WRITE8(ptDev,  Dst, Src) (Dst) = (Src);
  #define HWIF_WRITE16(ptDev, Dst, Src) (Dst) = (Src);
  #define HWIF_WRITE32(ptDev, Dst, Src) (Dst) = (Src);
  #define HWIF_WRITEN(ptDev,  Dst, Src, Len) OS_Memcpy(Dst, Src, Len)
#endif /* CIFX_TOOLKIT_HWIF */

#ifdef __cplusplus
}
#endif

#endif /* CIFX_HWIF__H */
