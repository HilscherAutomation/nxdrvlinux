/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXDMA.h 15171 2025-08-05 08:18:45Z AMinor $:

  Description:
    DMA related information

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-07-24  Created

**************************************************************************************/

#ifndef CIFX_DMA__H
#define CIFX_DMA__H

#include <stdint.h>

/* TODO: this is required so the structure in cifXFunctionList.h is the
 *       same for the DPM and HIF Toolkit */
#define CIFX_DMA_BUFFER_COUNT            8                              /*!< Number of DMA buffers */
typedef struct CIFX_DMABUFFER_Ttag
{
  uint32_t   ulSize;                      /*!< DMA buffer size  */
  uint32_t   ulPhysicalAddress;           /*!< Physical address of the buffer */
  void*      pvBuffer;                    /*!< Pointer to the buffer */
  void*      pvUser;                      /*!< User parameter */
} CIFX_DMABUFFER_T, *PCIFX_DMABUFFER_T;

// #ifdef CIFX_TOOLKIT_DMA
//
// TODO
//
// #endif /* CIFX_TOOLKIT_DMA */

#endif /* CIFX_DMA__H */
