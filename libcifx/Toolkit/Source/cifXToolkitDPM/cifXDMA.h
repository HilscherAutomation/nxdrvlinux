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

#ifdef CIFX_TOOLKIT_DMA

  /*****************************************************************************/
  /*! DMA buffer structure.
   *   In DMA mode, passing the physical and virtual pointers to pre-defined
   *   to the toolkit.                                                         */
  /*****************************************************************************/

  /*****************************************************************************/
  /*! Definition of the DMA channel numbers                                    */
  /*****************************************************************************/
  typedef enum
  {
    eDMA_CHANNEL_0 = 0,
    eDMA_CHANNEL_1 = 1,
    eDMA_CHANNEL_2 = 2,
    eDMA_CHANNEL_3 = 3,
    eDMA_CHANNEL_4 = 4,
    eDMA_CHANNEL_5 = 5,
    eDMA_CHANNEL_6 = 6,
    eDMA_CHANNEL_7 = 7
  } CIFX_DMA_CHANNEL;

  /*****************************************************************************/
  /*! Definition of the DMA buffers and direction                              */
  /*****************************************************************************/
  typedef enum
  {
    eDMA_INPUT_BUFFER_IDX   = 0,            /* Input buffer index */
    eDMA_OUTPUT_BUFFER_IDX  = 1             /* Output buffer index */
  } CIFX_DMA_DIRECTION;

  /*****************************************************************************/
  /*! Definition of the DMA                                                    */
  /*****************************************************************************/
  typedef enum CIFX_TOOLKIT_DMA_MODE_Etag
  {
    eDMA_MODE_LEAVE = 0,                    /*!< Leave communication channels in actual mode */
    eDMA_MODE_ON,                           /*!< Switch channels into DMA mode if possible */
    eDMA_MODE_OFF                           /*!< Switch OFF DMA mode for all channels */
  } CIFX_TOOLKIT_DMA_MODE_E;

  /*****************************************************************************/
  /*! Default definitions and DMA buffer structure                             */
  /*****************************************************************************/
  /* ATTENTION: - Buffer size must be a multiple of 256 Byte */
  /*            - Maximum supported DMA buffer size is 63,75 KByte */
  #define CIFX_DMA_MODULO_SIZE             256
  #define CIFX_DMA_MAX_BUFFER_SIZE         (255 * CIFX_DMA_MODULO_SIZE)   /*!< Max configureable DMA buffer size */
  #define CIFX_DMA_BUFFER_COUNT            8                              /*!< Number of DMA buffers */
  #define CIFX_DEFAULT_DMA_BUFFER_SIZE     8*1024                         /*!< DMA buffer size in KByte */

  typedef struct CIFX_DMABUFFER_Ttag
  {
    uint32_t   ulSize;                      /*!< DMA buffer size  */
    uint32_t   ulPhysicalAddress;           /*!< Physical address of the buffer */
    void*      pvBuffer;                    /*!< Pointer to the buffer */
    void*      pvUser;                      /*!< User parameter */
  } CIFX_DMABUFFER_T, *PCIFX_DMABUFFER_T;

#endif /* CIFX_TOOLKIT_DMA */

#endif /* CIFX_DMA__H */
