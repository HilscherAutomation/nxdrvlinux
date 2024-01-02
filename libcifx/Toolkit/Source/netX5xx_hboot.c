/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: $:

  Description:
    cifX Toolkit implementation of the netX100/500 boot functions

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2018-09-24  moved hboot functions to separate module


**************************************************************************************/

#include "cifXToolkit.h"
#include "NetX_ROMLoader.h"
#include "cifXErrors.h"
#include "cifXEndianess.h"

/*****************************************************************************/
/*! Downloads and starts the bootloader on netX100
*   \param ptDevInstance Instance to download the bootloader to (needs a reset
*                        before downloading)
*   \param pbFileData    Pointer to bootloader file data
*   \param ulFileDataLen Length of bootloader file
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t cifXStartBootloader_netX100( PDEVICEINSTANCE ptDevInstance,
                                     uint8_t*        pbFileData,
                                     uint32_t        ulFileDataLen)
{
  int32_t  lRet  = CIFX_DRV_INIT_STATE_ERROR;
  uint8_t* pbTmp = (uint8_t*)OS_Memalloc(ulFileDataLen);

  if(NULL == pbTmp)
  {
    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_ERROR,
                "Error allocating memory for bootloader verification!");
    }
    lRet = CIFX_FILE_LOAD_INSUFF_MEM;
  } else
  {
    /* Startup 2nd stage Loader */
    HWIF_WRITEN(ptDevInstance, ptDevInstance->pbDPM, pbFileData, ulFileDataLen);

    HWIF_READN(ptDevInstance, pbTmp, ptDevInstance->pbDPM, ulFileDataLen);

    if(OS_Memcmp(pbTmp, pbFileData, ulFileDataLen) != 0)
    {
      lRet = CIFX_DRV_DOWNLOAD_FAILED;

      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_ERROR,
                  "Downloading of bootloader to DPM failed!");
      }

    } else
    {
      /* Toggle Start bit to let the second stage loader get started by netX ROMloader
        Set bit 7 (Host) equal to Bit 3 (netX) */
      uint32_t ulState = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptDevInstance->ptGlobalRegisters->ulSystemState));

      if( ulState & MSK_SYSSTA_BOOT_ACTIVE)
        /* Bit 3 is set, set Bit 7 */
        ulState |= (uint32_t)MSK_SYSSTA_BOOT_START;
      else
        /* Bit 3 is 0, clear Bit 7 */
        ulState &= (uint32_t)~MSK_SYSSTA_BOOT_START;

      HWIF_WRITE32(ptDevInstance, ptDevInstance->ptGlobalRegisters->ulSystemState, HOST_TO_LE32(ulState));

      /* We are done with starting the netX */
      lRet = CIFX_NO_ERROR;
    }

    OS_Memfree(pbTmp);
  }

  return lRet;
}