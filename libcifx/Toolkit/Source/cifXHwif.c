/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXHwif.c 15171 2025-08-05 08:18:45Z AMinor $:

  Description:
    cifX API Hardware interface handling functions

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-07-09  Created

**************************************************************************************/
#include "cifXHWFunctions.h"
#include "cifXFunctionList.h"
#include "cifXHwif.h"

#ifdef CIFX_TOOLKIT_HWIF

/*****************************************************************************/
/*! Wrapper function to read byte from DPM
*   \param ptDev Device instance
*   \param pvSrc DPM address to read from
*   \return Byte read from DPM                                               */
/*****************************************************************************/
uint8_t HwIfRead8(struct DEVICEINSTANCEtag* ptDev, void* pvSrc)
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
uint16_t HwIfRead16(struct DEVICEINSTANCEtag* ptDev, void* pvSrc)
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
uint32_t HwIfRead32(struct DEVICEINSTANCEtag* ptDev, void* pvSrc)
{
  uint32_t ulData = 0;
  (void)ptDev->pfnHwIfRead(1, ptDev, pvSrc, &ulData, sizeof(ulData));
  return ulData;
}

#endif /* CIFX_TOOLKIT_HWIF */
