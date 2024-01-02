/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXInit.c 14703 2023-04-27 12:25:19Z RMayer $:

  Description:
    cifX Toolkit Initialization function implementation. This file contains all functions
    that need to be called by the application which wants to use the toolkit, to pass the
    cards that need to be handled and initialize them all. This file also includes the
    functions for downloading the firmware/configuration on startup and bring the card to live.

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
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

#include "Hil_Packet.h"
#include "Hil_ModuleLoader.h"
#include "Hil_SystemCmd.h"
#include "Hil_Results.h"

#include "NetX_ROMLoader.h"
#include "netx50_romloader_dpm.h"
#include "netx51_romloader_dpm.h"


/* Use external definitions for the netX specific functions */
/* This keeps the netX chip headers independent of toolkit definitions. */
extern int32_t  cifXStartBootloader_netX100 ( PDEVICEINSTANCE ptDevInstance,
                                              uint8_t*        pbFileData,
                                              uint32_t        ulFileDataLen);

extern int32_t  cifXStartBootloader_hboot   ( PDEVICEINSTANCE ptDevInstance,
                                              uint8_t*        pbFileData,
                                              uint32_t        ulFileDataLen);

extern int      IsNetX51or52ROM             ( PDEVICEINSTANCE ptDevInstance);
extern int      IsNetX4x00FLASH             ( PDEVICEINSTANCE ptDevInstance);
extern int      IsNetX4x00ROM               ( PDEVICEINSTANCE ptDevInstance);
extern int      IsNetX90FLASH               ( PDEVICEINSTANCE ptDevInstance);
extern int      IsNetX90ROM               ( PDEVICEINSTANCE ptDevInstance);

/*****************************************************************************/
/*! Structure description of NETX_FW_IDENTIFY_CNF_DATA_T                     */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atFWIdentifyConv[] =
{
  /* Offset, Width,                       Elements */
  { 0x00, eCIFX_ENDIANESS_WIDTH_16BIT, 4}, /* tFwVersion.Maj/Min/Build/Rev   */
  { 0x48, eCIFX_ENDIANESS_WIDTH_16BIT, 1}, /* tFwDate.usYear                 */
};

uint32_t g_ulTraceLevel = TRACE_LEVEL_ERROR;  /*!< Tracelevel used by the toolkit */

/*****************************************************************************/
/*!  \addtogroup CIFX_TOOLKIT_FUNCS cifX DPM Toolkit specific functions
*    \{                                                                      */
/*****************************************************************************/

uint32_t                g_ulDeviceCount = 0;      /*!< Number of devices handled by toolkit */
PDEVICEINSTANCE*        g_pptDevices    = NULL;   /*!< Array of device informations         */

TKIT_DRIVER_INFORMATION g_tDriverInfo   = {0};    /*!< Global driver information            */

void* g_pvTkitLock = NULL;

/*****************************************************************************/
/*! Cyclic timer for COS bit checking, if we are running in polling mode     */
/*****************************************************************************/
void cifXTKitCyclicTimer(void)
{
  uint32_t ulIdx;

  OS_EnterLock(g_pvTkitLock);
  for(ulIdx = 0; ulIdx < g_ulDeviceCount; ulIdx++)
  {
    if(!g_pptDevices[ulIdx]->fIrqEnabled)
    {
      /* Device is not running in IRQ mode, so we need to check COS */
      DEV_CheckCOSFlags(g_pptDevices[ulIdx]);
    }
  }
  OS_LeaveLock(g_pvTkitLock);
}

/*****************************************************************************/
/*! Delete a channel instance structure and all contained allocated data
*   \param   ptChannelInst Channel instance to delete (will also be free'd)  */
/*****************************************************************************/
static void cifXDeleteChannelInstance(PCHANNELINSTANCE ptChannelInst)
{
  uint32_t ulTemp;

  /*-------------------------------------------------*/
  /* Free dynamic objects created for the interrupt  */
  /*-------------------------------------------------*/
  /* Clean up all interrupt events */
  for(ulTemp = 0; ulTemp < sizeof(ptChannelInst->ahHandshakeBitEvents) / sizeof(ptChannelInst->ahHandshakeBitEvents[0]); ++ulTemp)
  {
    if(NULL != ptChannelInst->ahHandshakeBitEvents[ulTemp])
    {
      OS_DeleteEvent(ptChannelInst->ahHandshakeBitEvents[ulTemp]);
      ptChannelInst->ahHandshakeBitEvents[ulTemp] = NULL;
    }
  }

  /*-------------------------------------------------*/
  /* Free all dynamically allocated I/O Input Areas  */
  /*-------------------------------------------------*/
  if(NULL != ptChannelInst->pptIOInputAreas)
  {
    for(ulTemp = 0; ulTemp < ptChannelInst->ulIOInputAreas; ++ulTemp)
    {
      PIOINSTANCE ptIoInst = ptChannelInst->pptIOInputAreas[ulTemp];

      if(NULL != ptIoInst)
      {
        /* Delete synchronisation object */
        OS_DeleteMutex(ptIoInst->pvMutex);

        OS_Memfree(ptIoInst);
        ptChannelInst->pptIOInputAreas[ulTemp] = NULL;
      }
    }

    OS_Memfree(ptChannelInst->pptIOInputAreas);
    ptChannelInst->pptIOInputAreas = NULL;
  }

  /*-------------------------------------------------*/
  /* Free all dynamically allocated I/O Output Areas */
  /*-------------------------------------------------*/
  if(NULL != ptChannelInst->pptIOOutputAreas)
  {
    for(ulTemp = 0; ulTemp < ptChannelInst->ulIOOutputAreas; ++ulTemp)
    {
      PIOINSTANCE ptIoInst = ptChannelInst->pptIOOutputAreas[ulTemp];

      if(NULL!= ptIoInst)
      {
        /* Delete synchronisation object */
        OS_DeleteMutex(ptIoInst->pvMutex);

        OS_Memfree(ptIoInst);
        ptChannelInst->pptIOOutputAreas[ulTemp] = NULL;
      }
    }

    OS_Memfree(ptChannelInst->pptIOOutputAreas);
    ptChannelInst->pptIOOutputAreas = NULL;
  }

  /*-------------------------------------------------*/
  /* Free all dynamically allocated User Areas       */
  /*-------------------------------------------------*/
  if(NULL != ptChannelInst->pptUserAreas)
  {
    for(ulTemp = 0; ulTemp < ptChannelInst->ulUserAreas; ++ulTemp)
    {
      OS_Memfree(ptChannelInst->pptUserAreas[ulTemp]);
      ptChannelInst->pptUserAreas[ulTemp] = NULL;
    }

    OS_Memfree(ptChannelInst->pptUserAreas);
    ptChannelInst->pptUserAreas = NULL;
  }

  /*-------------------------------------------------*/
  /* Delete Mailbox synchronisation objects (Mutex)  */
  /*-------------------------------------------------*/
  if(NULL != ptChannelInst->tSendMbx.pvSendMBXMutex)
    OS_DeleteMutex(ptChannelInst->tSendMbx.pvSendMBXMutex);
  if(NULL != ptChannelInst->tRecvMbx.pvRecvMBXMutex)
    OS_DeleteMutex(ptChannelInst->tRecvMbx.pvRecvMBXMutex);

  /*-------------------------------------------------*/
  /* Delete lock object                              */
  /*-------------------------------------------------*/
  if(NULL != ptChannelInst->pvLock)
    OS_DeleteLock(ptChannelInst->pvLock);
  if(NULL != ptChannelInst->pvInitMutex)
    OS_DeleteMutex(ptChannelInst->pvInitMutex);

  /* Free channel instance */
  OS_Memfree(ptChannelInst);
}

/*****************************************************************************/
/*! Evaluate the netX chip of the device (when running in ROMloader) and sets
*   value in supplied device instance
*
*   Global register block only evaluated for netX90/4000 based devices.
*
*   \param ptDevInstance        Device instance
*   \param ulActDPMState        Actual DPM state
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t cifXDetectChipTypebyROMLoader(PDEVICEINSTANCE ptDevInstance, uint32_t ulActDPMState)
{
  int32_t  lRet     = CIFX_DRV_INIT_STATE_ERROR;
  uint32_t ulCookie = 0;

  ptDevInstance->eChipType = eCHIP_TYPE_UNKNOWN;

  HWIF_READN(ptDevInstance, &ulCookie, ptDevInstance->pbDPM, sizeof(ulCookie));

  /*-----------------------------*/
  /* Start with netX50           */
  /*-----------------------------*/
  if(ulCookie == HOST_TO_LE32(NETX50_BOOTID_DPM))
  {
    /* This is a netX50 */
    ptDevInstance->eChipType = eCHIP_TYPE_NETX50;
    lRet = CIFX_NO_ERROR;

  /*-----------------------------*/
  /* Check for netX51 / netX52   */
  /*-----------------------------*/
  } else if(IsNetX51or52ROM(ptDevInstance))
  {
    /* eChipType already set */
    lRet = CIFX_NO_ERROR;

  /*-----------------------------*/
  /* Check for netX4000 / 4100   */
  /*-----------------------------*/
  } else if(IsNetX4x00ROM(ptDevInstance))
  {
    /* eChipType already set */
    lRet = CIFX_NO_ERROR;

  /*-----------------------------*/
  /* Check for netX90            */
  /*-----------------------------*/
  } else if(IsNetX90ROM(ptDevInstance))
  {
    /* eChipType already set */
    lRet = CIFX_NO_ERROR;

  /*-----------------------------*/
  /* Check for netX100 / netX500 */
  /*-----------------------------*/
  } else if( (ulActDPMState & (MSK_SYSSTA_BOOT_ACTIVE | MSK_SYSSTA_LED_READY)) == (MSK_SYSSTA_BOOT_ACTIVE | MSK_SYSSTA_LED_READY) )
  {
    /* This must be a netX100/500. Currently we are not able to
       detect netX100 / netX500 independently */
    ptDevInstance->eChipType = eCHIP_TYPE_NETX500;
    lRet = CIFX_NO_ERROR;
  }

  if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
  {
    USER_Trace( ptDevInstance,
                TRACE_LEVEL_DEBUG,
                "Chiptype detected: %d",
                ptDevInstance->eChipType);
  }

  return lRet;
}

/*****************************************************************************/
/*! Performs a hardware reset on the given device
*   \param ptDevInstance Instance to reset
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXHardwareReset(PDEVICEINSTANCE ptDevInstance)
{
  static const uint32_t s_aulResetSequence[] =
  {
    0x00000000, 0x00000001,0x00000003, 0x00000007,
    0x0000000F, 0x0000001F,0x0000003F, 0x0000007F,
    0x000000FF
  };

  void*                   pvPCIConfig    = NULL;
  uint32_t                ulIdx          = 0;
  int32_t                 lRet           = CIFX_DRV_INIT_STATE_ERROR;
  volatile uint32_t*      pulHostReset   = &ptDevInstance->ptGlobalRegisters->ulHostReset;
  volatile uint32_t*      pulSystemState = &ptDevInstance->ptGlobalRegisters->ulSystemState;

  /* Read PCI config */
  if(ptDevInstance->fPCICard)
  {
    /* Only RAM based PCI devices without netX4x00 needs to save the PCI Configuration before a reset */
    /* Note: At this point we are not able to detect netX4000 chips, which does not loose
             PCI Config information during a reset. But we don't know the chip state before a reset
             and therefore we can't be sure to find a DPM and the global register block. */
    pvPCIConfig = OS_ReadPCIConfig(ptDevInstance->pvOSDependent);
  }

  /* Check for netX5x and load pointer to reset and system state in DPM */
  if( IsNetX51or52ROM(ptDevInstance))
  {
    NETX51_DPM_CONFIG_AREA_T* ptDpmConfig = (NETX51_DPM_CONFIG_AREA_T*)ptDevInstance->pbDPM;
    pulHostReset   = &ptDpmConfig->ulDpmResetRequest;
    pulSystemState = &ptDpmConfig->ulDpmSysSta;
  }

  /* Perform netX Hardware Reset */
  for(ulIdx = 0; ulIdx < sizeof(s_aulResetSequence) / sizeof(s_aulResetSequence[0]); ++ulIdx)
    HWIF_WRITE32(ptDevInstance, *pulHostReset, HOST_TO_LE32(s_aulResetSequence[ulIdx]));

  /* Wait until netX is in reset */
  OS_Sleep(NET_BOOTLOADER_RESET_TIME);

  /* Write PCI config */
  if( ptDevInstance->fPCICard)
  {
    /* Only RAM based PCI devices without netX4x00 needs to restore the PCI Configuration after a reset */
    /* Note: But we don't know the chip type at this point and for any further access to a DPM
             we have to restore the PCI config information, otherwise it is possible we accessinhg
             device memory which is not correctly mapped by the host system. */
      OS_WritePCIConfig(ptDevInstance->pvOSDependent, pvPCIConfig);
  }

  /* Call user, to allow setting up DPM, HW etc, */
  if(ptDevInstance->pfnNotify)
  {
    if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_DEBUG,
                "Calling supplied function (0x%08X) after resetting the card!",
                ptDevInstance->pfnNotify);
    }
    ptDevInstance->pfnNotify(ptDevInstance, eCIFX_TOOLKIT_EVENT_POSTRESET);
  }

  /* Wait for romloader to signal PCI Boot State */
  for(ulIdx = 0; (ulIdx < NET_BOOTLOADER_STARTUP_CYCLES) && (lRet != CIFX_NO_ERROR); ++ulIdx)
  {
    uint32_t ulState;
    OS_Sleep(NET_BOOTLOADER_STARTUP_WAIT);

    ulState = LE32_TO_HOST(HWIF_READ32(ptDevInstance, *pulSystemState)); /*lint !e564 */

    /* Check if state not 0xFFFFFFFF. This happens if memory is not available. */
    if( (ulState == CIFX_DPM_INVALID_CONTENT)   ||
        (ulState == CIFX_DPM_NO_MEMORY_ASSIGNED)  )
    {
      /* Error, register block not available */
      lRet = CIFX_MEMORY_MAPPING_FAILED;

      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                   TRACE_LEVEL_ERROR,
                   "DPM Content invalid after Reset (Data=0x%08X)!",
                   ulState);
      }
      break;

    } else
    {
      /* Detect chip type after hardware reset */
      lRet = cifXDetectChipTypebyROMLoader( ptDevInstance, ulState);
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Download the 2nd Stage Bootloader to the card, starts it and checks if
* it is running on the card
*   \param ptDevInstance Instance to download the bootloader to (needs a reset
*                        before downloading)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXRunBootloader(PDEVICEINSTANCE ptDevInstance)
{
  uint32_t              ulFileSize  = 0;
  void*                 pvFile      = NULL;
  int32_t               lRet        = CIFX_DRV_INIT_STATE_ERROR;

  CIFX_FILE_INFORMATION tFileInfo;
  OS_Memset(&tFileInfo, 0, sizeof(tFileInfo));

  /* Read boot loader file name depending to the chip type */
  USER_GetBootloaderFile( ptDevInstance, &tFileInfo);

  /* Try to open the file */
  if(NULL == (pvFile = OS_FileOpen(tFileInfo.szFullFileName, &ulFileSize)))
  {
    lRet = CIFX_FILE_OPEN_FAILED;

    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_ERROR,
                "Error opening bootloader file '%s'!",
                tFileInfo.szFullFileName);
    }
  } else
  {
    /* Read bootloader file data */
    uint8_t* pbBuffer = (uint8_t*)OS_Memalloc(ulFileSize);

    if(g_ulTraceLevel & TRACE_LEVEL_INFO)
    {
      USER_Trace(ptDevInstance,
                 TRACE_LEVEL_INFO,
                 "Downloading bootloder '%s'",
                 tFileInfo.szFullFileName);
    }

    if( NULL == pbBuffer)
    {
      lRet = CIFX_FILE_LOAD_INSUFF_MEM;

      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_ERROR,
                  "Error creating file buffer!");
      }
    } else
    {
      if(ulFileSize != OS_FileRead(pvFile, 0, ulFileSize, pbBuffer))
      {
        lRet = CIFX_FILE_READ_ERROR;

        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Error reading bootloader file '%s'!",
                    tFileInfo.szFullFileName);
        }

      } else
      {

        /* Call user, to allow setting up DPM, HW etc, */
        if(ptDevInstance->pfnNotify)
        {
          if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_DEBUG,
                      "Calling supplied function (0x%08X) before starting bootloader!",
                      ptDevInstance->pfnNotify);
          }
          ptDevInstance->pfnNotify(ptDevInstance, eCIFX_TOOLKIT_EVENT_PRE_BOOTLOADER);
        }

        switch(ptDevInstance->eChipType)
        {
        case eCHIP_TYPE_NETX500:
        case eCHIP_TYPE_NETX100:
          lRet = cifXStartBootloader_netX100(ptDevInstance,
                                             pbBuffer,
                                             ulFileSize);
          break;

        case eCHIP_TYPE_NETX50:
        case eCHIP_TYPE_NETX51:
        case eCHIP_TYPE_NETX52:
          lRet = cifXStartBootloader_hboot(ptDevInstance,
                                           pbBuffer,
                                           ulFileSize);
          break;

        default:
          lRet = CIFX_DRV_INIT_STATE_ERROR;
          break;
        }

        if(CIFX_NO_ERROR == lRet)
        {
          uint32_t  ulIdx  = 0;

          /* Wait until 2nd Stage loader or firmware is running */
          for(ulIdx = 0; ulIdx < NET_BOOTLOADER_STARTUP_CYCLES; ++ulIdx)
          {
            volatile uint32_t* pulDpmStart = (volatile uint32_t*)ptDevInstance->pbDPM;

            /* Wait until bootloader is active */
            OS_Sleep(NET_BOOTLOADER_STARTUP_WAIT);

            /* Call user, to setup DPM, in case the bootloader uses
                other timings/bit width than the original ROM loader settings */
            if( ptDevInstance->pfnNotify && (0 == ulIdx))
            {
              if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
              {
                USER_Trace(ptDevInstance,
                          TRACE_LEVEL_DEBUG,
                          "Calling supplied function (0x%08X) after starting bootloader!",
                          ptDevInstance->pfnNotify);
              }
              ptDevInstance->pfnNotify(ptDevInstance, eCIFX_TOOLKIT_EVENT_POST_BOOTLOADER);
            }

            /* Check if state not 0xFFFFFFFF. This happens if memory is not available. */
            if( (HWIF_READ32(ptDevInstance, *pulDpmStart) == HOST_TO_LE32(CIFX_DPM_INVALID_CONTENT))   ||
                (HWIF_READ32(ptDevInstance, *pulDpmStart) == HOST_TO_LE32(CIFX_DPM_NO_MEMORY_ASSIGNED))  )
            {
              /* Set temporary error, device is not yet back on bus */
              lRet = CIFX_MEMORY_MAPPING_FAILED;

            } else
            {
              if( (HWIF_READ32(ptDevInstance, *pulDpmStart) != HOST_TO_LE32(CIFX_DPMSIGNATURE_BSL_VAL)) &&
                  (HWIF_READ32(ptDevInstance, *pulDpmStart) != HOST_TO_LE32(CIFX_DPMSIGNATURE_FW_VAL)) )
              {
                /* no 'netX' or 'BOOT' signature found */
                lRet = CIFX_DRV_INIT_STATE_ERROR;
              } else
              {
                /* All states are OK */
                lRet = CIFX_NO_ERROR;

                if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
                {
                  USER_Trace(ptDevInstance,
                            TRACE_LEVEL_DEBUG,
                            "Bootloader was downloaded and started successfully!");
                }

                break;
              }
            }
          }

          if(CIFX_NO_ERROR != lRet)
          {
            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        TRACE_LEVEL_ERROR,
                        "DPM not accessible after starting Bootloader! (lRet=0x%08X)",
                        lRet);
            }
          }
        }
      }

      /* Free file buffer */
      OS_Memfree(pbBuffer);
    }

    /* Close file */
    OS_FileClose(pvFile);

  } /*lint !e593 : pbBuffer possible not freed */

  return lRet;
}

/*****************************************************************************/
/*! Start RAM based device
*   Device will execute a complet hardware reset here
*   \param ptDevInstance Instance to start up
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXStartRAMDevice(PDEVICEINSTANCE ptDevInstance)
{
  int32_t lRet = CIFX_NO_ERROR;

  /*--------------------------------------*/
  /* Check DPM size which must be 64KByte */
  /*--------------------------------------*/
  if(ptDevInstance->ulDPMSize < NETX_DPM_MEMORY_SIZE)
  {
    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      USER_Trace( ptDevInstance,
                  TRACE_LEVEL_ERROR,
                  "RAM based device needs a DPM >= 64kB to work (DPMSize=%u). Device cannot be handled!",
                  ptDevInstance->ulDPMSize);
    }

    lRet = CIFX_INVALID_BOARD;

  } else
  {
    if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
    {
      USER_Trace( ptDevInstance,
                  TRACE_LEVEL_DEBUG,
                  "New RAM based device found, device will be reset!");
    }

    /*-------------------------------------------------------------*/
    /* Call user notify function to allow setting up DPM, HW etc.  */
    /*-------------------------------------------------------------*/
    if(ptDevInstance->pfnNotify)
    {
      if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
      {
        USER_Trace( ptDevInstance,
                    TRACE_LEVEL_DEBUG,
                    "Calling supplied function (0x%08X) before resetting the card (HWReset)!",
                    ptDevInstance->pfnNotify);
      }
      ptDevInstance->pfnNotify(ptDevInstance, eCIFX_TOOLKIT_EVENT_PRERESET);
    }

    /*-------------------------------------------------------------*/
    /* Reset card                                                  */
    /*-------------------------------------------------------------*/
    if(CIFX_NO_ERROR != (lRet = cifXHardwareReset(ptDevInstance)))
    {
      /* HW reset failed */
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace( ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Hardware reset failed, check if the card is correctly configured (PCI/DPM bootmode) (lRet=0x%08X)!",
                    lRet);
      }

    /*-------------------------------------------------------------*/
    /* Load Bootloader to card and start it.                       */
    /*-------------------------------------------------------------*/
    } else if( CIFX_NO_ERROR != (lRet = cifXRunBootloader(ptDevInstance)))
    {
      /* Bootloader could not be started */
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace( ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Bootloader could not be started! (lRet=0x%08X)",
                    lRet);
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Handle BASE OS Module for RAM based devices
*   \param ptDevInstance Instance to start up
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXHandleRAMBaseOSModule(PDEVICEINSTANCE ptDevInstance)
{
  /* Check if we have a BASE OS image (cifXrcX.nxf) ready for download (new download mechanism).
     If not, we will try to continue the "old" style that expects a module containing
     the whole rcX */

  int32_t                 lRet      = CIFX_NO_ERROR;
  CIFX_DEVICE_INFORMATION tDevInfo;
  CIFX_FILE_INFORMATION   tFileInfo;

  OS_Memset(&tDevInfo,  0, sizeof(tDevInfo));
  OS_Memset(&tFileInfo, 0, sizeof(tFileInfo));

  /* Initialize file information structure */
  tDevInfo.ulDeviceNumber   = ptDevInstance->ulDeviceNumber;
  tDevInfo.ulSerialNumber   = ptDevInstance->ulSerialNumber;
  tDevInfo.ulChannel        = CIFX_SYSTEM_DEVICE;
  tDevInfo.ptDeviceInstance = ptDevInstance;

  /*-----------------------------------------*/
  /* Ask user about an BASE OS Module        */
  /*-----------------------------------------*/
  if( USER_GetOSFile(&tDevInfo, &tFileInfo))
  {
    uint32_t    ulFileLength  = 0;
    void*       pvFile        = NULL;
    CIFXHANDLE  hSysDevice    = (CIFXHANDLE)&ptDevInstance->tSystemDevice;

    if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
    {
      USER_Trace( ptDevInstance,
                  TRACE_LEVEL_WARNING,
                  "O/S file (%s) found. Download and start base rcX System!", tFileInfo.szShortFileName);
    }

    /*-----------------------------------------*/
    /* Open the file                           */
    /*-----------------------------------------*/
    if(NULL == (pvFile  = OS_FileOpen(tFileInfo.szFullFileName, &ulFileLength)))
    {
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace( ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Error opening OS file '%s'!",
                    tFileInfo.szFullFileName);
      }

      lRet = CIFX_FILE_OPEN_FAILED;

    } else
    {
      /*-------------------------------------------------------*/
      /* Create local buffer and read the file into the buffer */
      /*-------------------------------------------------------*/
      void* pbBuffer = OS_Memalloc(ulFileLength);
      if (NULL == pbBuffer)
      {
        lRet = CIFX_FILE_LOAD_INSUFF_MEM;

        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Error creating file buffer!");
        }

      } else
      {
        /*-------------------------------------------------------*/
        /* Read the file into the buffer                         */
        /*-------------------------------------------------------*/
        if(ulFileLength != OS_FileRead(pvFile, 0, ulFileLength, pbBuffer))
        {
          /* Error reading file */
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace( ptDevInstance,
                        TRACE_LEVEL_ERROR,
                        "Error reading OS file from disk '%s'!",
                        tFileInfo.szFullFileName);
          }

          lRet = CIFX_FILE_READ_ERROR;

        /*---------------------------------------------------------------------------------------------*/
        /* based devices are not using a FLASH file system, the BASE OS file must be always downloaded */
        /*---------------------------------------------------------------------------------------------*/
        } else if(CIFX_NO_ERROR != (lRet = xSysdeviceDownload(hSysDevice,
                                                              0,
                                                              DOWNLOAD_MODE_FIRMWARE,
                                                              tFileInfo.szShortFileName,
                                                              (uint8_t*)pbBuffer,
                                                              ulFileLength,
                                                              NULL,
                                                              NULL,
                                                              NULL)))
        {
          /* Error during download */
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace( ptDevInstance,
                        TRACE_LEVEL_ERROR,
                        "Error downloading OS file to device '%s'"\
                        " - (lRet=0x%08X)!",
                        tFileInfo.szFullFileName,
                        lRet);
          }
        } else
        {
          /* Start the Base OS
              - We need to send a channel instantiate request, so the bootloader
                starts up the base module */

          HIL_CHANNEL_INSTANTIATE_REQ_T tSendPkt;
          HIL_CHANNEL_INSTANTIATE_CNF_T tRecvPkt;

          OS_Memset(&tSendPkt, 0, sizeof(tSendPkt));
          OS_Memset(&tRecvPkt, 0, sizeof(tRecvPkt));

          /* Download successfull */
          if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
          {
            USER_Trace( ptDevInstance,
                        TRACE_LEVEL_DEBUG,
                        "OS file was downloaded successfully '%s'", tFileInfo.szFullFileName);
          }

          /* Create start request */
          tSendPkt.tHead.ulDest      = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
          tSendPkt.tHead.ulLen       = HOST_TO_LE32(sizeof(HIL_CHANNEL_INSTANTIATE_REQ_DATA_T));
          tSendPkt.tHead.ulCmd       = HOST_TO_LE32(HIL_CHANNEL_INSTANTIATE_REQ);
          tSendPkt.tData.ulChannelNo = HOST_TO_LE32(CIFX_SYSTEM_DEVICE);

          /* Transfer packet */
          lRet = DEV_TransferPacket( &ptDevInstance->tSystemDevice,
                                    (CIFX_PACKET*)&tSendPkt,
                                    (CIFX_PACKET*)&tRecvPkt,
                                    sizeof(HIL_MODULE_INSTANTIATE_CNF_T),
                                    CIFX_TO_SEND_PACKET,
                                    NULL,
                                    NULL);

          if( (CIFX_NO_ERROR  != lRet) ||
              (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(tRecvPkt.tHead.ulSta))) )
          {
            /* Error starting the firmware */
            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        TRACE_LEVEL_ERROR,
                        "Error sending Start request to start Base OS (lRet=0x%08X)!",
                        lRet);
            }
          }  else
          {
            /*--------------------------------------------
                Wait until READY is gone!!!!!!!!!!!!!!!!!!!
            --------------------------------------------*/
            if (!DEV_WaitForNotReady_Poll( &ptDevInstance->tSystemDevice, CIFX_TO_FIRMWARE_START))
            {
              lRet = CIFX_DEV_RESET_TIMEOUT;

              if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
              {
                USER_Trace(ptDevInstance,
                          TRACE_LEVEL_ERROR,
                          "Error waiting for firmware to leave reset state! (lRet=0x%08X)",
                          lRet);
              }
            } else
            {
              /*--------------------------------------------
                  Wait until READY is back
              --------------------------------------------*/
              /* Check if firmware is READY because we need the DPM Layout */
              if (!DEV_WaitForReady_Poll( &ptDevInstance->tSystemDevice, CIFX_TO_FIRMWARE_START))
              {
                lRet = CIFX_DEV_NOT_READY;

                /* READY state not reached */
                if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
                {
                  USER_Trace(ptDevInstance,
                            TRACE_LEVEL_ERROR,
                            "Error device does not reach READY state! (lRet=0x%08X)",
                            lRet);
                }
              }
            }
          }
        }

        /* Free the file buffer */
        OS_Memfree(pbBuffer);
      }

      /* Close the file */
      OS_FileClose(pvFile);

    } /*lint !e429 : pbBuffer not freed or returned */
  }

  return lRet;
}

/*****************************************************************************/
/*! Start Flash based device
*   \param ptDevInstance Instance to start up
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXStartFlashDevice(PDEVICEINSTANCE ptDevInstance)
{
  /* Check if a FW or Bootloader is running */
  ptDevInstance = ptDevInstance;

  /* Note: Checks already done in cifXEvaluateDeviveType() */

  return CIFX_NO_ERROR;
}

/*****************************************************************************/
/*! Handle Flash based BASE OS Module
*   \param ptDevInstance Instance to start up
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXHandleFlashBaseOSModule(PDEVICEINSTANCE ptDevInstance)
{
  /* Check if we have a BASE OS image (comXrcX.nxf) ready for download (new download mechanism).
     If not, we will try to continue the "old" style that expects a module containing
     the whole rcX */

  /* ATTENTION:
    Base Module Start
    --------------------------
    - On Flash based cards: we have to start the Base Module with a SYSTEM_RESTART
  */

  int32_t                 lRet      = CIFX_NO_ERROR;
  CIFX_DEVICE_INFORMATION tDevInfo;
  CIFX_FILE_INFORMATION   tFileInfo;

  OS_Memset(&tDevInfo,  0, sizeof(tDevInfo));
  OS_Memset(&tFileInfo, 0, sizeof(tFileInfo));

  /* Initialize file information structure */
  tDevInfo.ulDeviceNumber   = ptDevInstance->ulDeviceNumber;
  tDevInfo.ulSerialNumber   = ptDevInstance->ulSerialNumber;
  tDevInfo.ulChannel        = CIFX_SYSTEM_DEVICE;
  tDevInfo.ptDeviceInstance = ptDevInstance;

  /* Ask user about an BASE OS Module */
  if(USER_GetOSFile(&tDevInfo, &tFileInfo))
  {
    uint32_t  ulFileLength  = 0;
    void*     pvFile        = NULL;

    CIFXHANDLE hSysDevice = (CIFXHANDLE)&ptDevInstance->tSystemDevice;

    if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
    {
      USER_Trace(ptDevInstance,
                  TRACE_LEVEL_WARNING,
                  "O/S file (%s) found. Download and start base rcX System!", tFileInfo.szShortFileName);
    }

    /*-----------------------------------------*/
    /* Open the file                           */
    /*-----------------------------------------*/
    if(NULL == (pvFile = OS_FileOpen( tFileInfo.szFullFileName, &ulFileLength)))
    {
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Error opening OS file '%s'!",
                    tFileInfo.szFullFileName);
      }

      lRet = CIFX_FILE_OPEN_FAILED;

    } else
    {
      /*-------------------------------------------------------*/
      /* Create local buffer and read the file into the buffer */
      /*-------------------------------------------------------*/
      uint8_t* pbBuffer = (uint8_t*)OS_Memalloc(ulFileLength);

      if (NULL == pbBuffer)
      {
        lRet = CIFX_FILE_LOAD_INSUFF_MEM;

        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Error creating file buffer!");
        }

      } else
      {
        /*-------------------------------------------------------*/
        /* Read the file into the buffer                         */
        /*-------------------------------------------------------*/
        if(ulFileLength != OS_FileRead(pvFile, 0, ulFileLength, pbBuffer))
        {
          /* Error reading file */
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace( ptDevInstance,
                        TRACE_LEVEL_ERROR,
                        "Error reading OS file from disk '%s'!",
                        tFileInfo.szFullFileName);
          }

          lRet = CIFX_FILE_READ_ERROR;

        } else
        {
          /* On a flash based device we need to check if the file already exists
             We will only download it, if our file is different from that on the
             device, or the device does not have this file                      */
          int fDownload = 0;
          if ( CIFX_NO_ERROR != (lRet = DEV_CheckForDownload( hSysDevice,
                                                              HIL_PACKET_DEST_SYSTEM, /* BASE OS will be found in "PORT_0" */
                                                              &fDownload,
                                                              tFileInfo.szShortFileName,
                                                              pbBuffer,
                                                              ulFileLength,
                                                              DEV_TransferPacket,
                                                              NULL,
                                                              NULL)))
          {
            /* Display an error */
            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace( ptDevInstance,
                          TRACE_LEVEL_ERROR,
                          "Error checking for download '%s'!",
                          tFileInfo.szFullFileName);
            }
          } else if (!fDownload)
          {
            /* File already exists on the hardware, we have not to download it*/
            if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
            {
              USER_Trace( ptDevInstance,
                          TRACE_LEVEL_DEBUG,
                          "Skipping download for file '%s'" \
                          "[checksum identical]!",
                          tFileInfo.szFullFileName);
            }
          } else
          {
            /* Delete all files in all channels */
            uint32_t ulChNum = 0;
            for ( ulChNum = 0; ulChNum < CIFX_MAX_NUMBER_OF_CHANNELS; ulChNum++)
            {
              (void)DEV_RemoveChannelFiles( (PCHANNELINSTANCE)hSysDevice, ulChNum, DEV_TransferPacket, NULL, NULL, NULL);
            }

            /* Download the file stored in the buffer */
            lRet = xSysdeviceDownload(hSysDevice,
                                      0,
                                      DOWNLOAD_MODE_FIRMWARE,
                                      tFileInfo.szShortFileName,
                                      pbBuffer,
                                      ulFileLength,
                                      NULL,
                                      NULL,
                                      NULL);

            if(CIFX_NO_ERROR != lRet)
            {
              /* Error during download */
              if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
              {
                USER_Trace( ptDevInstance,
                            TRACE_LEVEL_ERROR,
                            "Error downloading OS file to device '%s'"\
                            " - (lRet=0x%08X)!",
                            tFileInfo.szFullFileName,
                            lRet);
              }
            } else
            {
              /* Download successful */
              if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
              {
                USER_Trace( ptDevInstance,
                            TRACE_LEVEL_DEBUG,
                            "OS file was downloaded successfully '%s'", tFileInfo.szFullFileName);
              }

              /* Start the Base OS */
              /* We have to do a SYSTEMSTART */
              if ( CIFX_NO_ERROR != (lRet = DEV_DoSystemStart( &ptDevInstance->tSystemDevice, CIFX_TO_FIRMWARE_START, 0)))
              {
                if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
                {
                  USER_Trace(ptDevInstance,
                            TRACE_LEVEL_ERROR,
                            "Error during Flash based Base OS system start! (lRet=0x%08X)",
                            lRet);
                }
              }
            }
          }
        } /*lint !e429 : pbBuffer not freed or returned */

        /* Free the file buffer */
        OS_Memfree(pbBuffer);
      }

      /* Close the file */
      OS_FileClose(pvFile);

    } /*lint !e429 : pbBuffer not freed or returned */
  }

  return lRet;
}


/*****************************************************************************/
/*! Download firmware/module files to a specified device
*   \param ptDevInstance    Instance to download the files to
*   \param ptDevChannelCfg  Channel configuration data (downloaded files, etc.)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXDownloadFWFiles(PDEVICEINSTANCE ptDevInstance, PDEVICE_CHANNEL_CONFIG ptDevChannelCfg)
{
  int32_t  lRet        = CIFX_NO_ERROR;
  uint32_t ulChannel   = 0;

  /* Process all channels */
  for(ulChannel = 0; ulChannel < CIFX_MAX_NUMBER_OF_CHANNELS; ++ulChannel)
  {
    CIFX_DEVICE_INFORMATION tDevInfo;
    uint32_t                ulIdx         = 0;
    uint32_t                ulFirmwareCnt = 0;

    OS_Memset(&tDevInfo, 0, sizeof(tDevInfo));

    tDevInfo.ulDeviceNumber   = ptDevInstance->ulDeviceNumber;
    tDevInfo.ulSerialNumber   = ptDevInstance->ulSerialNumber;
    tDevInfo.ulChannel        = ulChannel;
    tDevInfo.ptDeviceInstance = ptDevInstance;

    /* Get information about the number of firmware files to download */
    ulFirmwareCnt = USER_GetFirmwareFileCount(&tDevInfo);

    /* Show information about the channel and the number of firmware files to download */
    if(g_ulTraceLevel & TRACE_LEVEL_INFO)
    {
      USER_Trace(ptDevInstance,
                  TRACE_LEVEL_INFO,
                  "Firmware download, checking / starting: CHANNEL #%d, %d file(s)",
                  ulChannel,
                  ulFirmwareCnt);
    }

    /*----------------------------*/
    /* Process all firmware files */
    /*----------------------------*/
    for(ulIdx = 0; ulIdx < ulFirmwareCnt; ++ulIdx)
    {
      CIFX_FILE_INFORMATION tFileInfo;

      OS_Memset(&tFileInfo, 0, sizeof(tFileInfo));

      /* Read file information */
      if(!USER_GetFirmwareFile(&tDevInfo, ulIdx, &tFileInfo))
      {
        /* Firmware file not returned by USER */
        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "Error querying Firmware to load from USER_GetFirmwareFile (ulIdx=%u)!",
                      ulIdx);
        }
      } else
      {
        /*-----------------------------------------*/
        /* Open the file                           */
        /*-----------------------------------------*/
        uint32_t  ulFileLength = 0;
        void*     pvFile = OS_FileOpen(tFileInfo.szFullFileName, &ulFileLength);
        if(NULL == pvFile)
        {
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                        TRACE_LEVEL_ERROR,
                        "Error opening Firmware file '%s'!",
                        tFileInfo.szFullFileName);
          }

        } else
        {
          /*-------------------------------------------------------*/
          /* Create local buffer and read the file into the buffer */
          /*-------------------------------------------------------*/
          uint8_t* pbBuffer = (uint8_t*)OS_Memalloc(ulFileLength);

          if (NULL == pbBuffer)
          {
            lRet = CIFX_FILE_LOAD_INSUFF_MEM;

            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        TRACE_LEVEL_ERROR,
                        "Error creating file buffer!");
            }

          } else
          {
            /*-------------------------------------------------------*/
            /* Read the file into the buffer                         */
            /*-------------------------------------------------------*/
            if(ulFileLength != OS_FileRead(pvFile, 0, ulFileLength, pbBuffer))
            {
              /* Error reading file */
              if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
              {
                USER_Trace(ptDevInstance,
                          TRACE_LEVEL_ERROR,
                          "Error reading Firmware file from disk '%s'!",
                          tFileInfo.szFullFileName);
              }
            } else
            {
              uint8_t bLoadState = CIFXTKIT_DOWNLOAD_NONE;
              if( CIFX_NO_ERROR == (lRet = DEV_ProcessFWDownload( ptDevInstance,
                                                                  ulChannel,
                                                                  tFileInfo.szFullFileName,
                                                                  tFileInfo.szShortFileName,
                                                                  ulFileLength,
                                                                  pbBuffer,
                                                                  &bLoadState,
                                                                  DEV_TransferPacket,
                                                                  NULL,
                                                                  NULL,
                                                                  NULL)))
              {
                switch(bLoadState & ~CIFXTKIT_DOWNLOAD_EXECUTED)
                {
                  case CIFXTKIT_DOWNLOAD_MODULE:
                    /* Store name and module state */
                    (void)OS_Strncpy(ptDevChannelCfg->atChannelData[ulChannel].szFileName,
                                     tFileInfo.szShortFileName,
                                     sizeof(ptDevChannelCfg->atChannelData[ulChannel].szFileName));

                    ptDevChannelCfg->atChannelData[ulChannel].fModuleLoaded = 1;
                    ptDevChannelCfg->atChannelData[ulChannel].ulFileSize    = ulFileLength;
                  break;

                  case CIFXTKIT_DOWNLOAD_FIRMWARE:
                    /* Store name and firmware state */
                    /* Unselect all modules, because we have a firmware */
                    OS_Memset(ptDevChannelCfg->atChannelData, 0, sizeof(ptDevChannelCfg->atChannelData));

                    /* Set firmware information */
                    if( bLoadState & CIFXTKIT_DOWNLOAD_EXECUTED)
                      ptDevChannelCfg->fFWLoaded = 1;

                    ptDevChannelCfg->atChannelData[ulChannel].ulFileSize = ulFileLength;
                  break;

                  default:
                    /* nothing to do*/
                  break;
                }
              }
            }

            /* Free the file buffer */
            OS_Memfree(pbBuffer);
          }

          /* Close the file */
          OS_FileClose(pvFile);

        } /*lint !e429 : pbBuffer not freed or returned */
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Download configuration files to a specified device
*   \param ptDevInstance    Instance to download the files to
*   \param ptDevChannelCfg  Channel configuration data (downloaded files, etc.)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXDownloadCNFFiles(PDEVICEINSTANCE ptDevInstance, PDEVICE_CHANNEL_CONFIG ptDevChannelCfg)
{
  CIFXHANDLE hSysDevice  = (CIFXHANDLE)&ptDevInstance->tSystemDevice;
  int32_t    lRet        = CIFX_NO_ERROR;
  uint32_t   ulChannel   = 0;

  /* Process all channels */
  for(ulChannel = 0; ulChannel < CIFX_MAX_NUMBER_OF_CHANNELS; ++ulChannel)
  {
    CIFX_DEVICE_INFORMATION tDevInfo;
    uint32_t                ulIdx       = 0;
    uint32_t                ulConfigCnt = 0;

    OS_Memset(&tDevInfo, 0, sizeof(tDevInfo));

    tDevInfo.ulDeviceNumber   = ptDevInstance->ulDeviceNumber;
    tDevInfo.ulSerialNumber   = ptDevInstance->ulSerialNumber;
    tDevInfo.ulChannel        = ulChannel;
    tDevInfo.ptDeviceInstance = ptDevInstance;

    /* Get information about the number of configuration files to download */
    ulConfigCnt   = USER_GetConfigurationFileCount(&tDevInfo);

    /* Display information about configuration files to download */
    if(g_ulTraceLevel & TRACE_LEVEL_INFO)
    {
      USER_Trace(ptDevInstance,
                  TRACE_LEVEL_INFO,
                  "Configuration download, checking / starting: CHANNEL#%d, %d file(s)!",
                  ulChannel,
                  ulConfigCnt);
    }

    /*---------------------------------*/
    /* Process all configuration files */
    /*---------------------------------*/
    for(ulIdx = 0; ulIdx < ulConfigCnt; ++ulIdx)
    {
      CIFX_FILE_INFORMATION tFileInfo;

      OS_Memset(&tFileInfo, 0, sizeof(tFileInfo));

      /* Read file information */
      if(!USER_GetConfigurationFile(&tDevInfo, ulIdx, &tFileInfo))
      {
        /* Configuration file not returned by USER */
        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "Error querying configuration to load via USER_GetConfigurationFile (ulIdx=%u)!",
                      ulIdx);
        }
      } else
      {
        /*----------------------------*/
        /* Open the file              */
        /*----------------------------*/
        uint32_t   ulFileLength = 0;
        void*      pvFile       = OS_FileOpen(tFileInfo.szFullFileName, &ulFileLength);
        if(NULL == pvFile)
        {
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "Error opening configuration file '%s'!",
                      tFileInfo.szFullFileName);
          }
        } else
        {
          /*-------------------------------------------------------*/
          /* Create local buffer and read the file into the buffer */
          /*-------------------------------------------------------*/
          uint8_t*  pbBuffer = (uint8_t*)OS_Memalloc(ulFileLength);

          if (NULL == pbBuffer)
          {
            lRet = CIFX_FILE_LOAD_INSUFF_MEM;

            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        TRACE_LEVEL_ERROR,
                        "Error creating file buffer!");
            }

          } else
          {
            /*-------------------------------------------------------*/
            /* Read the file into the buffer                         */
            /*-------------------------------------------------------*/
            if( ulFileLength != OS_FileRead(pvFile, 0, ulFileLength, pbBuffer))
            {
              /* Error reading file */
              if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
              {
                USER_Trace(ptDevInstance,
                           TRACE_LEVEL_ERROR,
                           "Error reading configuration file from disk '%s'!",
                           tFileInfo.szFullFileName);
              }
            } else
            {
              /* Check if we have to download the file or if it already exists on the hardware */
              int fDownload = 0;
              if ( CIFX_NO_ERROR != (lRet = DEV_CheckForDownload( hSysDevice,
                                                                  ulChannel,
                                                                  &fDownload,
                                                                  tFileInfo.szShortFileName,
                                                                  pbBuffer,
                                                                  ulFileLength,
                                                                  DEV_TransferPacket,
                                                                  NULL,
                                                                  NULL)))
              {
                /* Display an error */
                if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
                {
                  USER_Trace(ptDevInstance,
                              TRACE_LEVEL_ERROR,
                              "Error checking for download '%s'!",
                              tFileInfo.szFullFileName);
                }
              } else if(!fDownload)
              {
                if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
                {
                  USER_Trace(ptDevInstance,
                              TRACE_LEVEL_DEBUG,
                              "Skipping download for file '%s'" \
                              "[checksum identical]!",
                              tFileInfo.szFullFileName);
                }
              } else
              {
                /* Download the file stored in the buffer */
                lRet = xSysdeviceDownload(hSysDevice,
                                          ulChannel,
                                          DOWNLOAD_MODE_CONFIG,
                                          tFileInfo.szShortFileName,
                                          pbBuffer,
                                          ulFileLength,
                                          NULL,
                                          NULL,
                                          NULL);
                if(CIFX_NO_ERROR != lRet)
                {
                  if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
                  {
                    USER_Trace(ptDevInstance,
                              TRACE_LEVEL_ERROR,
                              "Error downloading configuration to device '%s'"\
                              " - (lRet=0x%08X)!",
                              tFileInfo.szFullFileName,
                              lRet);
                  }
                } else
                {
                  /* We have downloaded a configuration file */
                  ptDevChannelCfg->atChannelData[ulChannel].fCNFLoaded = 1;

                  if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
                  {
                    USER_Trace(ptDevInstance,
                              TRACE_LEVEL_DEBUG,
                              "Successfully downloaded the configuration to device '%s'!",
                              tFileInfo.szFullFileName);
                  }
                }
              }
            }

            /* Free the file buffer */
            OS_Memfree(pbBuffer);
          }

          /* Close the file */
          OS_FileClose(pvFile);

        }  /*lint !e429 : pbBuffer not freed or returned */
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Read hardware identification
*   \param ptDevInstance      Device Instance
*   \param pfnRecvPktCallback Callback for unexpected packets
*   \param pvUser             Callback user parameter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t cifXReadHardwareIdent( PDEVICEINSTANCE ptDevInstance,
                               PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser)
{
  int32_t          lRet           = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptSystemdevice = &ptDevInstance->tSystemDevice;

  HIL_HW_IDENTIFY_REQ_T tSendPkt;
  CIFX_PACKET           tRecvPkt;

  OS_Memset(&tSendPkt, 0, sizeof(tSendPkt));
  OS_Memset(&tRecvPkt, 0, sizeof(tRecvPkt));

  /* Read firmware information */
  tSendPkt.tHead.ulDest       = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
  tSendPkt.tHead.ulSrc        = HOST_TO_LE32(ptDevInstance->ulPhysicalAddress);
  tSendPkt.tHead.ulCmd        = HOST_TO_LE32(HIL_HW_IDENTIFY_REQ);
  tSendPkt.tHead.ulLen        = 0;

  /* Transfer packet */
  lRet = DEV_TransferPacket( ptSystemdevice,
                             (CIFX_PACKET*)&tSendPkt,
                             &tRecvPkt,
                             sizeof(tRecvPkt),
                             CIFX_TO_SEND_PACKET,
                             pfnRecvPktCallback,
                             pvUser);

  if( (CIFX_NO_ERROR  != lRet) ||
      (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(tRecvPkt.tHeader.ulState))) )
  {
    if(g_ulTraceLevel & TRACE_LEVEL_WARNING)
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_WARNING,
                "Error querying hardware information! (lRet=0x%08X)",
                lRet);
    }
  } else
  {
    HIL_HW_IDENTIFY_CNF_T* ptData = (HIL_HW_IDENTIFY_CNF_T*)&tRecvPkt;

    ptDevInstance->eChipType  = (CIFX_TOOLKIT_CHIPTYPE_E)LE32_TO_HOST(ptData->tData.ulChipTyp);
  }

  return lRet;
}

/*****************************************************************************/
/*! Read firmware identification
*   \param ptDevInstance      Device Instance
*   \param ulChannel          Channel number
*   \param pfnRecvPktCallback Callback for unexpected packets
*   \param pvUser             Callback user parameter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t cifXReadFirmwareIdent( PDEVICEINSTANCE ptDevInstance, uint32_t ulChannel,
                               PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannelInst = ptDevInstance->pptCommChannels[ulChannel];

  HIL_FIRMWARE_IDENTIFY_REQ_T tSendPkt;
  CIFX_PACKET                 tRecvPkt;

  OS_Memset(&tSendPkt, 0, sizeof(tSendPkt));
  OS_Memset(&tRecvPkt, 0, sizeof(tRecvPkt));

  /* Read firmware information */
  tSendPkt.tHead.ulDest       = HOST_TO_LE32(HIL_PACKET_DEST_DEFAULT_CHANNEL);
  tSendPkt.tHead.ulSrc        = HOST_TO_LE32(ptDevInstance->ulPhysicalAddress);
  tSendPkt.tHead.ulCmd        = HOST_TO_LE32(HIL_FIRMWARE_IDENTIFY_REQ);
  tSendPkt.tHead.ulLen        = HOST_TO_LE32(sizeof(tSendPkt.tData));
  tSendPkt.tData.ulChannelId  = HOST_TO_LE32(ulChannel);

  /* Transfer packet */
  lRet = DEV_TransferPacket( ptChannelInst,
                             (CIFX_PACKET*)&tSendPkt,
                             &tRecvPkt,
                             sizeof(tRecvPkt),
                             CIFX_TO_SEND_PACKET,
                             pfnRecvPktCallback,
                             pvUser);

  if( (CIFX_NO_ERROR  != lRet) ||
      (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(tRecvPkt.tHeader.ulState))) )
  {
    if(g_ulTraceLevel & TRACE_LEVEL_WARNING)
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_WARNING,
                "Error querying firmware information! (lRet=0x%08X)",
                lRet);
    }
  } else
  {
    HIL_FIRMWARE_IDENTIFY_CNF_T* ptData = (HIL_FIRMWARE_IDENTIFY_CNF_T*)&tRecvPkt;

    OS_Memcpy( &ptChannelInst->tFirmwareIdent,
               &ptData->tData.tFirmwareIdentification,
               sizeof(ptChannelInst->tFirmwareIdent));

    (void)cifXConvertEndianess(0,
                               &ptChannelInst->tFirmwareIdent,
                               sizeof(ptChannelInst->tFirmwareIdent),
                               s_atFWIdentifyConv,
                               sizeof(s_atFWIdentifyConv) / sizeof(s_atFWIdentifyConv[0]));
  }

  return lRet;
}


/*****************************************************************************/
/*! Start a downloaded module
*   \param ptDevInstance      Instance to download the files to
*   \param ulChannelNumber    Channel number
*   \param pszModuleName      Module name
*   \param ulFileSize         Length of the file data
*   \param pfnRecvPktCallback Callback for unexpected packets
*   \param pvUser             Callback user parameter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t cifXStartModule( PDEVICEINSTANCE ptDevInstance, uint32_t ulChannelNumber, char* pszModuleName,
                         uint32_t ulFileSize, PFN_RECV_PKT_CALLBACK pfnRecvPktCallback, void* pvUser)
{
  int32_t             lRet        = CIFX_NO_ERROR;
  union
  {
    CIFX_PACKET                           tPacket;
    HIL_MODLOAD_LOAD_AND_RUN_MODULE_REQ_T tLoadAndRunReq;
    HIL_MODLOAD_RUN_MODULE_REQ_T          tRunReq;
  }                   uSendPacket;

  HIL_PACKET_HEADER_T tRecvPacket;
  uint32_t            ulNameLength  = 0;
  char*               pbCopyPtr     = NULL;
  uint32_t            ulCopySize    = 0;

  OS_Memset(&uSendPacket, 0, sizeof(uSendPacket));
  OS_Memset(&tRecvPacket, 0, sizeof(tRecvPacket));

  ulNameLength = OS_Strlen(pszModuleName) + 1;

  if( eCIFX_DEVICE_FLASH_BASED == ptDevInstance->eDeviceType)
  {
    /* comX Modules are stored in the FLASH file system and must be started by a modul LOAD_AND_RUN command */

    /* Create start command */
    uSendPacket.tPacket.tHeader.ulSrc   = HOST_TO_LE32(ptDevInstance->ulPhysicalAddress);
    uSendPacket.tPacket.tHeader.ulDest  = HOST_TO_LE32(HIL_PACKET_DEST_DEFAULT_CHANNEL);
    uSendPacket.tPacket.tHeader.ulCmd   = HOST_TO_LE32(HIL_MODLOAD_CMD_LOAD_AND_RUN_MODULE_REQ);
    uSendPacket.tPacket.tHeader.ulLen   = 0;
    uSendPacket.tPacket.tHeader.ulState = 0;
    uSendPacket.tPacket.tHeader.ulExt   = 0;

    /* Add request specific data to the packet data area */
    uSendPacket.tLoadAndRunReq.tData.ulChannel = HOST_TO_LE32(ulChannelNumber);

    /* Adjust packet length */
    uSendPacket.tPacket.tHeader.ulLen = HOST_TO_LE32( (uint32_t)(sizeof(uSendPacket.tLoadAndRunReq.tData) + ulNameLength) );

    /* Setup copy buffer and copy size */
    pbCopyPtr   = ((char*)(&uSendPacket.tPacket.abData[0])) + sizeof(uSendPacket.tLoadAndRunReq.tData);
    ulCopySize  = min( (sizeof(uSendPacket.tPacket.abData) - sizeof(uSendPacket.tLoadAndRunReq.tData)), ulNameLength);

    /* Insert file name */
    (void)OS_Strncpy( pbCopyPtr, pszModuleName, ulCopySize);

  } else
  {
    /* cifX card do not have a FLASH, modules are loaded into memory and will be started by a RUN_MODULE command */

    /* Create start command */
    uSendPacket.tPacket.tHeader.ulSrc   = HOST_TO_LE32(ptDevInstance->ulPhysicalAddress);
    uSendPacket.tPacket.tHeader.ulDest  = HOST_TO_LE32(HIL_PACKET_DEST_DEFAULT_CHANNEL);
    uSendPacket.tPacket.tHeader.ulCmd   = HOST_TO_LE32(HIL_MODLOAD_CMD_RUN_MODULE_REQ);
    uSendPacket.tPacket.tHeader.ulLen   = 0;
    uSendPacket.tPacket.tHeader.ulState = 0;
    uSendPacket.tPacket.tHeader.ulExt   = 0;

    /* Add request specific data to the packet data area */
    uSendPacket.tRunReq.tData.ulChannel = HOST_TO_LE32(ulChannelNumber);

    /* Adjust packet length */
    uSendPacket.tPacket.tHeader.ulLen = HOST_TO_LE32( (uint32_t)(sizeof(uSendPacket.tRunReq.tData) + ulNameLength) );

    /* Setup copy buffer and copy size */
    pbCopyPtr   = ((char*)(&uSendPacket.tPacket.abData[0])) + sizeof(uSendPacket.tRunReq.tData);
    ulCopySize  = min( (sizeof(uSendPacket.tPacket.abData) - sizeof(uSendPacket.tRunReq.tData)), ulNameLength);

    /* Insert file name */
    (void)OS_Strncpy( pbCopyPtr, pszModuleName, ulCopySize);

  }

  /* Module loading will relocate the module with the last packet.
     So the confirmation packet takes longer, depending on the
     file size (and contained firmware).
     Measurements showed that for every 100kB the module needs
     one additional second for relocation */
  lRet = DEV_TransferPacket( &ptDevInstance->tSystemDevice,
                             &uSendPacket.tPacket,
                             (CIFX_PACKET*)&tRecvPacket,
                             sizeof(tRecvPacket),
                             (uint32_t)(CIFX_TO_FIRMWARE_START + (ulFileSize / (100 * 1024)) * 1000),
                             pfnRecvPktCallback,
                             pvUser);

  if ( ( CIFX_NO_ERROR  != lRet) ||
       ( SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(tRecvPacket.ulSta))) )
  {
    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                  TRACE_LEVEL_ERROR,
                  "Error starting module '%s' on Channel %d - (lRet=0x%08X)!",
                  pszModuleName,
                  ulChannelNumber,
                  lRet);
    }
  } else
  {
    /* Check if we have an instance for the channel! */
    /* Note: On a system start we probably have no channel because they are created after the start handling */
    uint32_t ulTimeout    = 1000L;

    lRet = CIFX_DEV_NOT_READY;

    if( ulChannelNumber < ptDevInstance->ulCommChannelCount)
    {
      uint32_t ulDiffTime   = 0L;
      uint32_t lStartTime   = (int32_t)OS_GetMilliSecCounter();

      /* We should have such a communication channel */
      do
      {
        if (DEV_IsReady(ptDevInstance->pptCommChannels[ulChannelNumber]))
        {
          lRet = CIFX_NO_ERROR;
          break;
        }

        /* Check time */
        ulDiffTime = OS_GetMilliSecCounter() - lStartTime;

        /* Wait until firmware is down */
        OS_Sleep(1);

      } while (ulDiffTime < ulTimeout);
    } else
    {
      /* We do not have a communication channel number, */
      /* probably the first start and channels created afterwards */
      OS_Sleep (ulTimeout);

    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Start firmware on RAM based devices
*   \param ptDevInstance    Instance to download the files to
*   \param ptDevChannelCfg  Device configuration
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXStartRAMFirmware(PDEVICEINSTANCE ptDevInstance, PDEVICE_CHANNEL_CONFIG ptDevChannelCfg)
{
  /* On a RAM based device all modules are downloaded using HIL_FILE_XFERMODULE
     so we need to start them using
     1. RUN_MODULE for .NXO files
     2. CHANNEL_INSTANTIATE_REQ for .NXF files

     If only a configuration was downloaded, we need to execute a CHANNEL_INIT
     so the stack uses the new configuration */
  int32_t lRet = CIFX_NO_ERROR;

  /* ---------------------------------------------*/
  /* Check if we have to process loadable modules */
  /* ---------------------------------------------*/
  if( ptDevInstance->fModuleLoad)
  {
    /* We have loadable modules, the base OS is already running */
    /* We have not to process CNF files, because module load will also load the actual configuration */
    /* Start modules */
    uint32_t ulChNum = 0;
    for ( ulChNum = 0; ulChNum < CIFX_MAX_NUMBER_OF_CHANNELS; ulChNum++)
    {
      if( ptDevChannelCfg->atChannelData[ulChNum].fModuleLoaded)
      {
        /* Start modules */
        (void)cifXStartModule( ptDevInstance, ulChNum, ptDevChannelCfg->atChannelData[ulChNum].szFileName,
                               ptDevChannelCfg->atChannelData[ulChNum].ulFileSize, NULL, NULL);
      }
    }

  /* ---------------------------------------------*/
  /* Check if we have a firmware loaded           */
  /* ---------------------------------------------*/
  } else if(ptDevChannelCfg->fFWLoaded == 1)
  {
    /* We have to send a CHANNEL_INIT to start the firmware */
    uint32_t ulChannel   = 0;

    /* We doing CHANNELINIT, only possible via a packet on the system channel */
    HIL_CHANNEL_INSTANTIATE_REQ_T tSendPkt;
    HIL_CHANNEL_INSTANTIATE_CNF_T tRecvPkt;

    OS_Memset(&tSendPkt, 0, sizeof(tSendPkt));
    OS_Memset(&tRecvPkt, 0, sizeof(tRecvPkt));

    /* Create start request */
    tSendPkt.tHead.ulDest      = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
    tSendPkt.tHead.ulLen       = HOST_TO_LE32(sizeof(HIL_CHANNEL_INSTANTIATE_REQ_DATA_T));
    tSendPkt.tHead.ulCmd       = HOST_TO_LE32(HIL_CHANNEL_INSTANTIATE_REQ);
    tSendPkt.tData.ulChannelNo = HOST_TO_LE32(ulChannel);

    /* Transfer packet */
    lRet = DEV_TransferPacket( &ptDevInstance->tSystemDevice,
                              (CIFX_PACKET*)&tSendPkt,
                              (CIFX_PACKET*)&tRecvPkt,
                              sizeof(HIL_MODULE_INSTANTIATE_CNF_T),
                              CIFX_TO_SEND_PACKET,
                              NULL,
                              NULL);

    if( (CIFX_NO_ERROR  != lRet)                   ||
        (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(tRecvPkt.tHead.ulSta))) )
    {
      /* Error starting the firmware */
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_ERROR,
                  "Error sending Start request for Firmware (lRet=0x%08X)!",
                  lRet);
      }
    } else
    {
      /*--------------------------------------------
          Wait until READY is gone!!!!!!!!!!!!!!!!!!!
      --------------------------------------------*/
      if (!DEV_WaitForNotReady_Poll( &ptDevInstance->tSystemDevice, CIFX_TO_FIRMWARE_START))
      {
        lRet = CIFX_DEV_RESET_TIMEOUT;

        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Error waiting for firmware to leave reset state! (lRet=0x%08X)",
                    lRet);
        }

      } else
      {
        /*--------------------------------------------
            Wait until READY is back
        --------------------------------------------*/
        /* Check if firmware is READY because we need the DPM Layout */
        if (!DEV_WaitForReady_Poll( &ptDevInstance->tSystemDevice, CIFX_TO_FIRMWARE_START))
        {
          lRet = CIFX_DEV_NOT_READY;

          /* READY state not reached */
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "Error device does not reach READY state! (lRet=0x%08X)",
                      lRet);
          }
        }
      }
    }
  }

  if (CIFX_NO_ERROR == lRet)
  {
    /* Display a system error if NSF_ERROR is set */
    if( ptDevInstance->tSystemDevice.usNetxFlags & NSF_ERROR)
    {
      /* Trace system error */
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        uint32_t                  ulError = 0;
        HIL_DPM_SYSTEM_CHANNEL_T* ptSysCh = (HIL_DPM_SYSTEM_CHANNEL_T*)ptDevInstance->tSystemDevice.pbDPMChannelStart;

        if(0 != (ulError = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysCh->tSystemState.ulSystemError))))
        {
          USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "System error information, (SystemError=0x%08X)!",
                      ulError);
        }

        if( 0 != (ulError = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysCh->tSystemState.ulSystemStatus))))
        {
          USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "System state information, (SystemState=0x%08X)!",
                      ulError);
        }
      }
    }

    /* Display channel READY reached */
    if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_DEBUG,
                "System channel is READY!");
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Start Firmware on flash based devices
*   \param ptDevInstance    Instance to download the files to
*   \param ptDevChannelCfg  Device configuration
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXStartFlashFirmware(PDEVICEINSTANCE ptDevInstance, PDEVICE_CHANNEL_CONFIG ptDevChannelCfg)
{
  /*
    LOADABLE MODULES (.NXO):
    --------------------------------
    - We need a running Base O/S module.
    - Modules must be loaded with LOAD_AND_RUN_MODULE_REQ
    - Available configurations are automatically activated if a module is started

    FIRMWARE (NXF/NXM/MOD) download:
    --------------------------------
    - The new firmware must be started by sending a SYSTEM_RESTART to the card
    - The configuration will be automatically used by the FW

    CONFIGURATION download:
    -----------------------
    - The new configuration must be activated by sending a CHANNEL_INIT to the card
  */
  int32_t lRet = CIFX_NO_ERROR;

  /* Check if we have to process loadable modules */
  if( ptDevInstance->fModuleLoad)
  {
    /* We have loadable modules, the base OS is already running. A RESET is necessary if modules are started */
    /* We have not to process CNF files, because module load will also load the actual configuration */
    int fSystemStartDone = 0;

    /* Start modules */
    uint32_t ulChNum = 0;
    for ( ulChNum = 0; ulChNum < CIFX_MAX_NUMBER_OF_CHANNELS; ulChNum++)
    {
      if( ptDevChannelCfg->atChannelData[ulChNum].fModuleLoaded)
      {
        if( 0 == fSystemStartDone)
        {
          /* We have to do a SYSTEMSTART before loading a Module again! Maybe it is already running */
          if ( CIFX_NO_ERROR != (lRet = DEV_DoSystemStart( &ptDevInstance->tSystemDevice, CIFX_TO_FIRMWARE_START, 0)))
          {
            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        TRACE_LEVEL_ERROR,
                        "Error during system start! (lRet=0x%08X)",
                        lRet);
            }
            break;              /* No more starts possible */
          }
          fSystemStartDone = 1; /* No more starts necessary */
        }

        /* Start modules */
        (void)cifXStartModule( ptDevInstance, ulChNum, ptDevChannelCfg->atChannelData[ulChNum].szFileName,
                               ptDevChannelCfg->atChannelData[ulChNum].ulFileSize, NULL, NULL);
      }
    }
  } else
  {
    /* Check if we have a firmware loaded */
    if(ptDevChannelCfg->fFWLoaded == 1)
    {
      /* We have to do a SYSTEMSTART */
      if ( CIFX_NO_ERROR != (lRet = DEV_DoSystemStart( &ptDevInstance->tSystemDevice, CIFX_TO_FIRMWARE_START, 0)))
      {
        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Error during system start! (lRet=0x%08X)",
                    lRet);
        }
      } else
      {
        if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
        {
          USER_Trace(ptDevInstance,
                    TRACE_LEVEL_DEBUG,
                    "System start done!",
                    lRet);
        }
      }
    }
  }

  if (CIFX_NO_ERROR == lRet)
  {
    /* Display a system error if NSF_ERROR is set */
    if( ptDevInstance->tSystemDevice.usNetxFlags & NSF_ERROR)
    {
      /* Trace system error */
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        uint32_t                  ulError = 0;
        HIL_DPM_SYSTEM_CHANNEL_T* ptSysCh = (HIL_DPM_SYSTEM_CHANNEL_T*)ptDevInstance->tSystemDevice.pbDPMChannelStart;

        if(0 != (ulError = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysCh->tSystemState.ulSystemError))))
        {
          USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "System error information, (SystemError=0x%08X)!",
                      ulError);
        }

        if( 0 != (ulError = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysCh->tSystemState.ulSystemStatus))))
        {
          USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "System state information, (SystemState=0x%08X)!",
                      ulError);
        }
      }
    }

    /* Display channel READY reached */
    if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_DEBUG,
                "System channel is READY!");
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Perform a CHANNEL_INIT on all channels that received a new configuration
*   on flash based devices
*   \param ptDevInstance    Device instance
*   \param ptDevChannelCfg  Device configuration
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXStartFlashConfiguration(PDEVICEINSTANCE ptDevInstance, PDEVICE_CHANNEL_CONFIG ptDevChannelCfg)
{
  if(!ptDevChannelCfg->fFWLoaded)
  {
    /* Only activate configuration if no firmware has been loaded on a flash based
       device, as the systemstart after FW Download will automatically load database */

    /* Process all channels and send a CHANNEL_INIT to each one */
    uint32_t ulChannel   = 0;
    for(ulChannel = 0; ulChannel < ptDevInstance->ulCommChannelCount; ++ulChannel)
    {
      int32_t lChannelRet = CIFX_NO_ERROR;

      if ( ptDevChannelCfg->atChannelData[ulChannel].fCNFLoaded)
      {
        lChannelRet = DEV_DoChannelInit(ptDevInstance->pptCommChannels[ulChannel], CIFX_TO_SEND_PACKET);

        if(CIFX_NO_ERROR == lChannelRet)
        {
          if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_DEBUG,
                      "Successfully performed channel init on channel #%d!",
                      ulChannel);
          }
        } else
        {
          /* Error performing channel init */
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "Error performing channel init on channel #%d (lRet=0x%08X)!",
                      ulChannel,
                      lChannelRet);
          }
        }
      }
    }
  }

  /* Always return OK here, so the user is able to access the device later on.
     Any error during channel init is not fatal (e.g. maybe a missing master license) */
  return CIFX_NO_ERROR;
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
  uint8_t*                        pbDpm            = ptDevInstance->pbDPM;
  HIL_DPM_SYSTEM_CHANNEL_T*       ptSysChannel     = (HIL_DPM_SYSTEM_CHANNEL_T*)pbDpm;
  HIL_DPM_SYSTEM_CHANNEL_INFO_T*  ptSysChannelInfo = (HIL_DPM_SYSTEM_CHANNEL_INFO_T*)&ptSysChannel->atChannelInfo[HIL_DPM_SYSTEM_CHANNEL_INDEX];
  PCHANNELINSTANCE                ptSystemDevice   = &ptDevInstance->tSystemDevice;

  uint32_t                  ulMBXSize        = 0;
  CIFX_DEVICE_INFORMATION   tDevInfo;
  uint32_t                  ulDeviceIdx      = 0;
  uint32_t                  ulSysChannelSize = 0;
  void*                     pvSendMBXMutex   = NULL;
  void*                     pvRecvMBXMutex   = NULL;
  void*                     pvInitMutex      = NULL;
  void*                     pvLock           = NULL;

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

    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_ERROR,
                "Error creating buffers for system device!");
    }

  } else
  {
    OS_Memset(&tDevInfo, 0, sizeof(tDevInfo));

    ulMBXSize        = LE16_TO_HOST(HWIF_READ16(ptDevInstance, ptSysChannelInfo->usSizeOfMailbox)) / 2;
    ulSysChannelSize = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannelInfo->ulSizeOfChannel));

    /* Setup pointer to global netX register block */
    ptDevInstance->ptGlobalRegisters = (PNETX_GLOBAL_REG_BLOCK)(ptDevInstance->pbDPM +
                                                                ptDevInstance->ulDPMSize -
                                                                sizeof(NETX_GLOBAL_REG_BLOCK));

    /* Initialize DEVICEINSTANCE */
    ptDevInstance->ulDeviceNumber = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemInfo.ulDeviceNumber));
    ptDevInstance->ulSerialNumber = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemInfo.ulSerialNumber));
    ptDevInstance->ulSlotNumber   = HWIF_READ8(ptDevInstance, ptSysChannel->tSystemInfo.bDevIdNumber);

    if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
    {
      USER_Trace(ptDevInstance,
                TRACE_LEVEL_DEBUG,
                "Device Info:");

      USER_Trace(ptDevInstance,
                TRACE_LEVEL_DEBUG,
                " - Device Number : %u",
                ptDevInstance->ulDeviceNumber);

      USER_Trace(ptDevInstance,
                TRACE_LEVEL_DEBUG,
                " - Serial Number : %u",
                ptDevInstance->ulSerialNumber);

      USER_Trace(ptDevInstance,
                TRACE_LEVEL_DEBUG,
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
        if(OS_Strcmp(g_pptDevices[ulDeviceIdx]->szAlias,
                    ptDevInstance->szAlias) == 0)
        {
          /* Duplicate alias found */
          if(g_ulTraceLevel & TRACE_LEVEL_WARNING)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_WARNING,
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

    /* TODO: Systemchannel Handshake Flags may be inside Systemchannel in the future */
    ptDevInstance->pbHandshakeBlock     = &pbDpm[ulSysChannelSize];
    ptSystemDevice->ptHandshakeCell     = (HIL_DPM_HANDSHAKE_CELL_T*)ptDevInstance->pbHandshakeBlock;

    ptSystemDevice->pbDPMChannelStart   = pbDpm;
    ptSystemDevice->bHandshakeWidth     = HIL_HANDSHAKE_SIZE_8BIT;

    /* Create send mailbox */
    ptSystemDevice->tSendMbx.pvSendMBXMutex      = pvSendMBXMutex;
    ptSystemDevice->tSendMbx.bSendCMDBitoffset   = HSF_SEND_MBX_CMD_BIT_NO;
    ptSystemDevice->tSendMbx.ulSendCMDBitmask    = 1 << HSF_SEND_MBX_CMD_BIT_NO;
    ptSystemDevice->tSendMbx.ptSendMailboxStart  = (HIL_DPM_SEND_MAILBOX_BLOCK_T*)
                                                   (pbDpm + LE16_TO_HOST(HWIF_READ16(ptDevInstance, ptSysChannelInfo->usMailboxStartOffset)));

    ptSystemDevice->tSendMbx.ulSendMailboxLength = ulMBXSize -
                                                   (uint32_t)(sizeof(*(ptSystemDevice->tSendMbx.ptSendMailboxStart)) -
                                                    sizeof(ptSystemDevice->tSendMbx.ptSendMailboxStart->abSendMailbox));

    /* Create receive mailbox */
    ptSystemDevice->tRecvMbx.pvRecvMBXMutex      = pvRecvMBXMutex;
    ptSystemDevice->tRecvMbx.bRecvACKBitoffset   = HSF_RECV_MBX_ACK_BIT_NO;
    ptSystemDevice->tRecvMbx.ulRecvACKBitmask    = (1 << HSF_RECV_MBX_ACK_BIT_NO);
    ptSystemDevice->tRecvMbx.ptRecvMailboxStart  = (HIL_DPM_RECV_MAILBOX_BLOCK_T*)( (uint8_t*)(ptSystemDevice->tSendMbx.ptSendMailboxStart) + ulMBXSize);

    ptSystemDevice->tRecvMbx.ulRecvMailboxLength = ulMBXSize -
                                                   (uint32_t)(sizeof(*(ptSystemDevice->tRecvMbx.ptRecvMailboxStart)) -
                                                    sizeof(ptSystemDevice->tRecvMbx.ptRecvMailboxStart->abRecvMailbox));

    ptSystemDevice->ulDPMChannelLength  = ulSysChannelSize;

    ptSystemDevice->pvLock              = pvLock;
    ptSystemDevice->pvInitMutex         = pvInitMutex;

    ptSystemDevice->pvDeviceInstance    = (void*)ptDevInstance;

    ptSystemDevice->fIsSysDevice        = 1;

    /* Read actual Host state, in case they differ from 0 */
    DEV_ReadHostFlags(&ptDevInstance->tSystemDevice, 1);
    DEV_ReadHandshakeFlags(&ptDevInstance->tSystemDevice, 1, 0);


    /*--------------------------------------------
      Check if READY is available
    --------------------------------------------*/
    /* Check if system channel is READY before exceuting additional functions on it */
    if (!DEV_WaitForReady_Poll( &ptDevInstance->tSystemDevice, CIFX_TO_FIRMWARE_START))
    {
      lRet = CIFX_DEV_NOT_READY;

      /* READY state not reached */
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_ERROR,
                  "Error device does not reach READY state! (lRet=0x%08X)",
                  lRet);
      }
    }

    /* Display actual system state if available */
    if( ptDevInstance->tSystemDevice.usNetxFlags & NSF_ERROR)
    {
      /* Trace system error */
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        uint32_t                  ulError = 0;
        HIL_DPM_SYSTEM_CHANNEL_T* ptSysCh = (HIL_DPM_SYSTEM_CHANNEL_T*)ptDevInstance->tSystemDevice.pbDPMChannelStart;

        if(0 != (ulError = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysCh->tSystemState.ulSystemError))))
        {
          USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "System error information, (SystemError=0x%08X)!",
                      ulError);
        }

        if( 0 != (ulError = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysCh->tSystemState.ulSystemStatus))))
        {
          USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "System state information, (SystemState=0x%08X)!",
                      ulError);
        }
      }
    }
  }

  return lRet;  /*lint !e438 : Last value assigned not used */
}

/*****************************************************************************/
/*! Reads the channel layouts and the according subblock layouts
*   \param ptDevInstance Instance to configure the layout for
*   \param ptChannel     Instance to channel, the layout it saved to
*   \param ulNumOfBlocks Number of blocks to read from this channel
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXReadChannelLayout(PDEVICEINSTANCE ptDevInstance, PCHANNELINSTANCE ptChannel, uint32_t ulNumOfBlocks)
{
  int32_t  lRet        = CIFX_NO_ERROR;
  uint32_t ulIdx       = 0;
  uint32_t ulPacketIdx = 0;

  HIL_DPM_GET_BLOCK_INFO_REQ_T tSendPkt;
  HIL_DPM_GET_BLOCK_INFO_CNF_T tRecvPkt;

  OS_Memset(&tSendPkt, 0, sizeof(tSendPkt));
  OS_Memset(&tRecvPkt, 0, sizeof(tRecvPkt));

  if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
  {
    USER_Trace(ptDevInstance,
                TRACE_LEVEL_DEBUG,
                "Reading Channel Info on Channel#%d (DPM Start Offset=0x%08X Length=0x%08X)",
                ptChannel->ulChannelNumber,
                (uint32_t)(ptChannel->pbDPMChannelStart - ptDevInstance->pbDPM),
                ptChannel->ulDPMChannelLength);

    USER_Trace(ptDevInstance,
                TRACE_LEVEL_DEBUG,
                "-------------------------------------------------------------------------");
  }

  /* Read the information for the given block ID */
  /* Create subblock read request packet */
  tSendPkt.tHead.ulSrc  = HOST_TO_LE32(ptDevInstance->ulPhysicalAddress);
  tSendPkt.tHead.ulDest = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
  tSendPkt.tHead.ulLen  = HOST_TO_LE32(sizeof(tSendPkt.tData));
  tSendPkt.tHead.ulCmd  = HOST_TO_LE32(HIL_DPM_GET_BLOCK_INFO_REQ);

  /* Read subblock information */
  for ( ulIdx = 0; ulIdx < ulNumOfBlocks; ulIdx++)
  {
    /* Insert subblock request packet data */
    ++ulPacketIdx;
    tSendPkt.tHead.ulId             = HOST_TO_LE32(ulPacketIdx);            /* Insert Packet number */

    /* Insert subblock request packet data */
    tSendPkt.tData.ulAreaIndex      = HOST_TO_LE32(ptChannel->ulBlockID);
    tSendPkt.tData.ulSubblockIndex  = HOST_TO_LE32(ulIdx);                  /* Insert Block index into packet */

    /* Transfer request */
    if ( (lRet = DEV_TransferPacket( &ptDevInstance->tSystemDevice,
                                     (CIFX_PACKET*)&tSendPkt,
                                     (CIFX_PACKET*)&tRecvPkt,
                                     sizeof(HIL_DPM_GET_BLOCK_INFO_CNF_T),
                                     CIFX_TO_SEND_PACKET,
                                     NULL,
                                     NULL)) != CIFX_NO_ERROR)
    {
      /* Display errors */
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_ERROR,
                  "Error reading subblock information, Error: 0x%08X  (AreaIndex=%d, SubblockIndex=%d)",
                  lRet,
                  LE32_TO_HOST(tSendPkt.tData.ulAreaIndex),
                  LE32_TO_HOST(tSendPkt.tData.ulSubblockIndex));
      }
    } else if ( SUCCESS_HIL_OK != LE32_TO_HOST(tRecvPkt.tHead.ulSta))
    {
      /* Display errors */
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_ERROR,
                  "Error reading subblock information, error firmware answer,  Error: 0x%08X  (AreaIndex=%d, SubblockIndex=%d)",
                  LE32_TO_HOST(tRecvPkt.tHead.ulSta),
                  LE32_TO_HOST(tSendPkt.tData.ulAreaIndex),
                  LE32_TO_HOST(tSendPkt.tData.ulSubblockIndex));
      }
    } else
    {
      /*--------------------------------------------------------*/
      /* Check block information and create corresponding areas */
      /*--------------------------------------------------------*/
      switch(LE32_TO_HOST(tRecvPkt.tData.ulType) & HIL_BLOCK_MASK)
      {
        /*---------------------------*/
        /* Block types not supported */
        /*---------------------------*/
        case HIL_BLOCK_UNDEFINED:
        case HIL_BLOCK_UNKNOWN:

        if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
        {
          USER_Trace(ptDevInstance,
                    TRACE_LEVEL_DEBUG,
                    "Undefined/Unknown subblock type (Channel=%d, Block=%d, Type=0x%08X)",
                    ptChannel->ulChannelNumber,
                    LE32_TO_HOST(tSendPkt.tData.ulSubblockIndex),
                    LE32_TO_HOST(tRecvPkt.tData.ulType));
        }
        break;

        /*---------------------------*/
        /* Create DATA image block   */
        /*---------------------------*/
        case HIL_BLOCK_DATA_IMAGE:
        case HIL_BLOCK_DATA_IMAGE_HI_PRIO:
        {
          switch(LE16_TO_HOST(tRecvPkt.tData.usFlags) & HIL_DIRECTION_MASK)
          {
            /* Output Data image */
            case HIL_DIRECTION_OUT:
            {
              PIOINSTANCE ptIOOutputInstance = (PIOINSTANCE)OS_Memalloc(sizeof(*ptIOOutputInstance));
              void*       pvMutex            = NULL;

              if (NULL == ptIOOutputInstance           ||
                  NULL == (pvMutex = OS_CreateMutex()) )
              {
                lRet = CIFX_INVALID_POINTER;

                OS_Memfree(ptIOOutputInstance);
                OS_DeleteMutex(pvMutex);
                ptIOOutputInstance = NULL;
                pvMutex            = NULL;

                if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
                {
                  USER_Trace(ptDevInstance,
                            TRACE_LEVEL_ERROR,
                            "Error creating IO output instance buffer!");
                }

              } else
              {
                OS_Memset(ptIOOutputInstance, 0, sizeof(*ptIOOutputInstance));

                ptIOOutputInstance->pbDPMAreaStart  = ptChannel->pbDPMChannelStart + LE32_TO_HOST(tRecvPkt.tData.ulOffset);
                ptIOOutputInstance->ulDPMAreaLength = LE32_TO_HOST(tRecvPkt.tData.ulSize);
                ptIOOutputInstance->bHandshakeBit   = (uint8_t)LE16_TO_HOST(tRecvPkt.tData.usHandshakeBit);
                ptIOOutputInstance->usHandshakeMode = LE16_TO_HOST(tRecvPkt.tData.usHandshakeMode);

                if((LE32_TO_HOST(tRecvPkt.tData.ulType) & HIL_BLOCK_MASK) == HIL_BLOCK_DATA_IMAGE)
                  ptIOOutputInstance->ulNotifyEvent = CIFX_NOTIFY_PD0_OUT;
                else
                  ptIOOutputInstance->ulNotifyEvent = CIFX_NOTIFY_PD1_OUT;

                /* Create area mutex object */
                ptIOOutputInstance->pvMutex = pvMutex;

                switch(ptIOOutputInstance->usHandshakeMode)
                {
                  case HIL_IO_MODE_BUFF_DEV_CTRL:
                    ptIOOutputInstance->bHandshakeBitState = HIL_FLAGS_NOT_EQUAL;
                  break;

                  case HIL_IO_MODE_BUFF_HST_CTRL:
                    ptIOOutputInstance->bHandshakeBitState = HIL_FLAGS_EQUAL;
                  break;

                  default:
                    /* Unknown or non handshake */
                    ptIOOutputInstance->bHandshakeBitState = HIL_FLAGS_NONE;
                  break;
                }

                ++ptChannel->ulIOOutputAreas;
                ptChannel->pptIOOutputAreas = (PIOINSTANCE*)OS_Memrealloc(ptChannel->pptIOOutputAreas,
                                                                          ptChannel->ulIOOutputAreas * (uint32_t)sizeof(ptIOOutputInstance));

                if (NULL == ptChannel->pptIOOutputAreas)
                {
                  lRet = CIFX_INVALID_POINTER;

                  if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
                  {
                    USER_Trace(ptDevInstance,
                              TRACE_LEVEL_ERROR,
                              "Error creating IO output area buffer!");
                  }

                } else
                {
                  ptChannel->pptIOOutputAreas[ptChannel->ulIOOutputAreas - 1] = ptIOOutputInstance;

                  if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
                  {
                    USER_Trace(ptDevInstance,
                              TRACE_LEVEL_DEBUG,
                              "I/O Output Subblock found    (Channel=%d, Block=%d, Offset=0x%08X, Len=0x%04X)",
                              ptChannel->ulChannelNumber,
                              LE32_TO_HOST(tSendPkt.tData.ulSubblockIndex),
                              LE32_TO_HOST(tRecvPkt.tData.ulOffset),
                              LE32_TO_HOST(tRecvPkt.tData.ulSize));
                  }
                }
              }
            } /*lint !e438 !e593 : Last value assigned not used / ptIOOutputInstance not freed */
            break;

            /* Input Data image          */
            case HIL_DIRECTION_IN:
            {
              PIOINSTANCE ptIOInputInstance = (PIOINSTANCE)OS_Memalloc(sizeof(*ptIOInputInstance));
              void*       pvMutex           = NULL;

              if (NULL == ptIOInputInstance            ||
                  NULL == (pvMutex = OS_CreateMutex()) )
              {
                lRet = CIFX_INVALID_POINTER;

                OS_Memfree(ptIOInputInstance);
                OS_DeleteMutex(pvMutex);
                ptIOInputInstance = NULL;
                pvMutex           = NULL;

                if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
                {
                  USER_Trace(ptDevInstance,
                            TRACE_LEVEL_ERROR,
                            "Error creating IO input instance buffer!");
                }

              } else
              {
                OS_Memset(ptIOInputInstance, 0, sizeof(*ptIOInputInstance));

                ptIOInputInstance->pbDPMAreaStart  = ptChannel->pbDPMChannelStart + LE32_TO_HOST(tRecvPkt.tData.ulOffset);
                ptIOInputInstance->ulDPMAreaLength = LE32_TO_HOST(tRecvPkt.tData.ulSize);
                ptIOInputInstance->bHandshakeBit   = (uint8_t)LE16_TO_HOST(tRecvPkt.tData.usHandshakeBit);
                ptIOInputInstance->usHandshakeMode = LE16_TO_HOST(tRecvPkt.tData.usHandshakeMode);

                if((LE32_TO_HOST(tRecvPkt.tData.ulType) & HIL_BLOCK_MASK) == HIL_BLOCK_DATA_IMAGE)
                  ptIOInputInstance->ulNotifyEvent = CIFX_NOTIFY_PD0_IN;
                else
                  ptIOInputInstance->ulNotifyEvent = CIFX_NOTIFY_PD1_IN;

                /* Create area mutex object */
                ptIOInputInstance->pvMutex = pvMutex;

                switch(ptIOInputInstance->usHandshakeMode)
                {
                  case HIL_IO_MODE_BUFF_DEV_CTRL:
                    ptIOInputInstance->bHandshakeBitState = HIL_FLAGS_NOT_EQUAL;
                  break;

                  case HIL_IO_MODE_BUFF_HST_CTRL:
                    ptIOInputInstance->bHandshakeBitState = HIL_FLAGS_EQUAL;
                  break;

                  default:
                    /* Unknown or non handshake */
                    ptIOInputInstance->bHandshakeBitState = HIL_FLAGS_NONE;
                  break;
                }

                ++ptChannel->ulIOInputAreas;
                ptChannel->pptIOInputAreas = (PIOINSTANCE*)OS_Memrealloc(ptChannel->pptIOInputAreas,
                                                                         ptChannel->ulIOInputAreas * (uint32_t)sizeof(ptIOInputInstance));

                if (NULL == ptChannel->pptIOInputAreas)
                {
                  lRet = CIFX_INVALID_POINTER;

                  if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
                  {
                    USER_Trace(ptDevInstance,
                              TRACE_LEVEL_ERROR,
                              "Error creating IO input area buffer!");
                  }

                } else
                {
                  ptChannel->pptIOInputAreas[ptChannel->ulIOInputAreas- 1] = ptIOInputInstance;

                  if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
                  {
                    USER_Trace(ptDevInstance,
                              TRACE_LEVEL_DEBUG,
                              "I/O Input Subblock found     (Channel=%d, Block=%d, Offset=0x%08X, Len=0x%04X)",
                              ptChannel->ulChannelNumber,
                              LE32_TO_HOST(tSendPkt.tData.ulSubblockIndex),
                              LE32_TO_HOST(tRecvPkt.tData.ulOffset),
                              LE32_TO_HOST(tRecvPkt.tData.ulSize));
                  }
                }
              }
            }  /*lint !e438 !e593 : Last value assigned not used / ptIOOutputInstance not freed */
            break;

            default:
              /* Firmware returned an invalid IO subblock info (neither IN, nor OUT),
                This should never happen */
              if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
              {
                USER_Trace(ptDevInstance,
                          TRACE_LEVEL_ERROR,
                          "Invalid I/O direction found! (Channel=%d, Block=%d,Dir=0x%08X)",
                          ptChannel->ulChannelNumber,
                          LE32_TO_HOST(tSendPkt.tData.ulSubblockIndex),
                          LE16_TO_HOST(tRecvPkt.tData.usFlags) & HIL_DIRECTION_MASK);
              }
            break;
          } /* end creating IO data block */
        } /* end IO sub block handling */
        break;

        /*---------------------------*/
        /* Create MAILBOX block      */
        /*---------------------------*/
        case HIL_BLOCK_MAILBOX:
        {
          switch(LE16_TO_HOST(tRecvPkt.tData.usFlags) & HIL_DIRECTION_MASK)
          {
            /* Create output mailbox */
            case HIL_DIRECTION_OUT:
            {
              /* Create mailbox synchronisation object */
              if (NULL == (ptChannel->tSendMbx.pvSendMBXMutex = OS_CreateMutex()))
              {
                lRet = CIFX_INVALID_POINTER;

                if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
                {
                  USER_Trace(ptDevInstance,
                            TRACE_LEVEL_ERROR,
                            "Error creating output mailbox buffer!");
                }
              } else
              {
                /* Create mailbox */
                ptChannel->tSendMbx.ptSendMailboxStart   = (HIL_DPM_SEND_MAILBOX_BLOCK_T*)( ptChannel->pbDPMChannelStart +
                                                                                            LE32_TO_HOST(tRecvPkt.tData.ulOffset));
                ptChannel->tSendMbx.ulSendMailboxLength  = LE32_TO_HOST(tRecvPkt.tData.ulSize) -
                                                                        (uint32_t)(sizeof(*(ptChannel->tSendMbx.ptSendMailboxStart)) -
                                                                         sizeof(ptChannel->tSendMbx.ptSendMailboxStart->abSendMailbox));

                ptChannel->tSendMbx.bSendCMDBitoffset    = (uint8_t)LE16_TO_HOST(tRecvPkt.tData.usHandshakeBit);
                ptChannel->tSendMbx.ulSendCMDBitmask     = (1 << ptChannel->tSendMbx.bSendCMDBitoffset);

                if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
                {
                  USER_Trace(ptDevInstance,
                            TRACE_LEVEL_DEBUG,
                            "Output Mailbox found         (Channel=%d, Block=%d, Offset=0x%08X, Len=0x%04X)",
                            ptChannel->ulChannelNumber,
                            LE32_TO_HOST(tSendPkt.tData.ulSubblockIndex),
                            LE32_TO_HOST(tRecvPkt.tData.ulOffset),
                            LE32_TO_HOST(tRecvPkt.tData.ulSize));
                }
              }
            }
            break;

            /* Create input mailbox */
            case HIL_DIRECTION_IN:
            {
              /* Create mailbox synchronisation object */
              if (NULL == (ptChannel->tRecvMbx.pvRecvMBXMutex = OS_CreateMutex()))
              {
                lRet = CIFX_INVALID_POINTER;

                if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
                {
                  USER_Trace(ptDevInstance,
                            TRACE_LEVEL_ERROR,
                            "Error creating input mailbox buffer!");
                }

              } else
              {
                /* Create receive mailbox */
                ptChannel->tRecvMbx.ptRecvMailboxStart  = (HIL_DPM_RECV_MAILBOX_BLOCK_T*)( ptChannel->pbDPMChannelStart +
                                                                                           LE32_TO_HOST(tRecvPkt.tData.ulOffset));
                ptChannel->tRecvMbx.ulRecvMailboxLength = LE32_TO_HOST(tRecvPkt.tData.ulSize) -
                                                                       (uint32_t)(sizeof(*(ptChannel->tRecvMbx.ptRecvMailboxStart)) -
                                                                       sizeof(ptChannel->tRecvMbx.ptRecvMailboxStart->abRecvMailbox));
                ptChannel->tRecvMbx.bRecvACKBitoffset   = (uint8_t)LE16_TO_HOST(tRecvPkt.tData.usHandshakeBit);
                ptChannel->tRecvMbx.ulRecvACKBitmask    = (1 << ptChannel->tRecvMbx.bRecvACKBitoffset);

                if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
                {
                  USER_Trace(ptDevInstance,
                            TRACE_LEVEL_DEBUG,
                            "Input Mailbox found          (Channel=%d, Block=%d, Offset=0x%08X, Len=0x%04X)",
                            ptChannel->ulChannelNumber,
                            LE32_TO_HOST(tSendPkt.tData.ulSubblockIndex),
                            LE32_TO_HOST(tRecvPkt.tData.ulOffset),
                            LE32_TO_HOST(tRecvPkt.tData.ulSize));
                }
              }
            }
            break;

            default:
              /* Firmware returned an invalid mailbox subblock info (neither IN, nor OUT)  */
              if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
              {
                USER_Trace(ptDevInstance,
                          TRACE_LEVEL_ERROR,
                          "Invalid mailbox direction found! (Channel=%d, Block=%d,Dir=0x%08X)",
                          ptChannel->ulChannelNumber,
                          LE32_TO_HOST(tSendPkt.tData.ulSubblockIndex),
                          LE16_TO_HOST(tRecvPkt.tData.usFlags) & HIL_DIRECTION_MASK);
              }
            break;
          } /* end creating Send/Receive MAILBOX block */
        } /* end MAILBOX sub block handling */
        break;

        /*-------------------------------*/
        /* Create CONTROL/PARAMTER block */
        /*-------------------------------*/
        case HIL_BLOCK_CTRL_PARAM:
        {
          ptChannel->ptControlBlock     = (HIL_DPM_CONTROL_BLOCK_T*)( ptChannel->pbDPMChannelStart +
                                                                      LE32_TO_HOST(tRecvPkt.tData.ulOffset));
          ptChannel->bControlBlockBit   = (uint8_t)LE16_TO_HOST(tRecvPkt.tData.usHandshakeBit);
          ptChannel->ulControlBlockSize = LE32_TO_HOST(tRecvPkt.tData.ulSize);

          if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_DEBUG,
                      "Control block found          (Channel=%d, Block=%d, Offset=0x%08X, Len=0x%04X)",
                      ptChannel->ulChannelNumber,
                      LE32_TO_HOST(tSendPkt.tData.ulSubblockIndex),
                      LE32_TO_HOST(tRecvPkt.tData.ulOffset),
                      LE32_TO_HOST(tRecvPkt.tData.ulSize));
          }
        }
        break;

        /*-------------------------------*/
        /* Create Common STATE block     */
        /*-------------------------------*/
        case HIL_BLOCK_COMMON_STATE:
        {
          ptChannel->ptCommonStatusBlock = (HIL_DPM_COMMON_STATUS_BLOCK_T*)(ptChannel->pbDPMChannelStart +
                                                                            LE32_TO_HOST(tRecvPkt.tData.ulOffset));
          ptChannel->bCommonStatusBit    = (uint8_t)LE16_TO_HOST(tRecvPkt.tData.usHandshakeBit);
          ptChannel->ulCommonStatusSize  = LE32_TO_HOST(tRecvPkt.tData.ulSize);

          if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_DEBUG,
                      "Common Status block found    (Channel=%d, Block=%d, Offset=0x%08X, Len=0x%04X)",
                      ptChannel->ulChannelNumber,
                      LE32_TO_HOST(tSendPkt.tData.ulSubblockIndex),
                      LE32_TO_HOST(tRecvPkt.tData.ulOffset),
                      LE32_TO_HOST(tRecvPkt.tData.ulSize));
          }
        }
        break;

        /*-------------------------------*/
        /* Create Extended STATE block   */
        /*-------------------------------*/
        case HIL_BLOCK_EXTENDED_STATE:
        {
          ptChannel->ptExtendedStatusBlock = (HIL_DPM_EXTENDED_STATUS_BLOCK_T*)(ptChannel->pbDPMChannelStart +
                                                                                LE32_TO_HOST(tRecvPkt.tData.ulOffset));
          ptChannel->bExtendedStatusBit    = (uint8_t)LE16_TO_HOST(tRecvPkt.tData.usHandshakeBit);
          ptChannel->ulExtendedStatusSize  = LE32_TO_HOST(tRecvPkt.tData.ulSize);

          if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_DEBUG,
                      "Extended Status block found  (Channel=%d, Block=%d, Offset=0x%08X, Len=0x%04X)",
                      ptChannel->ulChannelNumber,
                      LE32_TO_HOST(tSendPkt.tData.ulSubblockIndex),
                      LE32_TO_HOST(tRecvPkt.tData.ulOffset),
                      LE32_TO_HOST(tRecvPkt.tData.ulSize));
          }
        }
        break;

        /*-------------------------------*/
        /* Create User block             */
        /*-------------------------------*/
        case HIL_BLOCK_USER:
        {
          PUSERINSTANCE ptUserInstance = (PUSERINSTANCE)OS_Memalloc(sizeof(*ptUserInstance));

          if (NULL == ptUserInstance)
          {
            lRet = CIFX_INVALID_POINTER;

            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        TRACE_LEVEL_ERROR,
                        "Error creating user block instance buffer!");
            }

          } else
          {
            OS_Memset(ptUserInstance, 0, sizeof(*ptUserInstance));

            ptUserInstance->pbUserBlockStart  = ptChannel->pbDPMChannelStart +
                                                LE32_TO_HOST(tRecvPkt.tData.ulOffset);
            ptUserInstance->ulUserBlockLength = LE32_TO_HOST(tRecvPkt.tData.ulSize);

            ++ptChannel->ulUserAreas;
            ptChannel->pptUserAreas = (PUSERINSTANCE*)OS_Memrealloc(ptChannel->pptUserAreas,
                                                                    ptChannel->ulUserAreas * (uint32_t)sizeof(ptUserInstance));

            if (NULL == ptChannel->pptUserAreas)
            {
              lRet = CIFX_INVALID_POINTER;

              OS_Memfree(ptUserInstance);
              ptUserInstance = NULL;

              if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
              {
                USER_Trace(ptDevInstance,
                          TRACE_LEVEL_ERROR,
                          "Error creating user area buffer!");
              }

            } else
            {
              ptChannel->pptUserAreas[ptChannel->ulUserAreas- 1] = ptUserInstance;

              if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
              {
                USER_Trace(ptDevInstance,
                          TRACE_LEVEL_DEBUG,
                          "User block found             (Channel=%d, Block=%d, Offset=0x%08X, Len=0x%04X)",
                          ptChannel->ulChannelNumber,
                          LE32_TO_HOST(tSendPkt.tData.ulSubblockIndex),
                          LE32_TO_HOST(tRecvPkt.tData.ulOffset),
                          LE32_TO_HOST(tRecvPkt.tData.ulSize));
              }
            }
          }
        } /*lint !e438 : Last value assigned not used */
        break;

        /*-------------------------------*/
        /* DEFAULT / Unknown             */
        /*-------------------------------*/
        default:
          /* Unknown block type, this should never happen */
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "Invalid subblock information found! (Channel=%d, Block=%d, Offset=0x%08X, Len=0x%04X, Type=%u)",
                      ptChannel->ulChannelNumber,
                      LE32_TO_HOST(tSendPkt.tData.ulSubblockIndex),
                      LE32_TO_HOST(tRecvPkt.tData.ulOffset),
                      LE32_TO_HOST(tRecvPkt.tData.ulSize),
                      LE32_TO_HOST(tRecvPkt.tData.ulType) & HIL_BLOCK_MASK);
          }
        break;
      } /* end process subblock information */
    }
  } /* End enumerate subblocks */

  if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
  {
    USER_Trace(ptDevInstance,
                TRACE_LEVEL_DEBUG,
                "-------------------------------------------------------------------------");
  }

  return lRet;
}

/*****************************************************************************/
/*! Handle WARMSTART parameter
*   \param ptDevInstance Device instance
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXHandleWarmstartParameter(PDEVICEINSTANCE ptDevInstance)
{
  CIFX_PACKET   tPacket;
  CIFX_PACKET   tRecvPacket;
  int32_t       lRet        = CIFX_NO_ERROR;
  uint32_t      ulBlockID   = 0;

  OS_Memset(&tPacket,     0, sizeof(tPacket));
  OS_Memset(&tRecvPacket, 0, sizeof(tRecvPacket));

  /*----------------------------------------------------------*/
  /* Process all available channels for warmstart  parameters */
  /*----------------------------------------------------------*/
  for(ulBlockID = 0; ulBlockID < ptDevInstance->ulCommChannelCount; ++ulBlockID)
  {
    /* NOTE: only use a single packet here top save stack usage*/
    CIFX_DEVICE_INFORMATION tDevInfo;

    /* Get channel instance */
    PCHANNELINSTANCE ptChannelInst = (PCHANNELINSTANCE)ptDevInstance->pptCommChannels[ulBlockID];

    OS_Memset(&tDevInfo, 0, sizeof(tDevInfo));

    /* Check for warm start data */
    tDevInfo.ulChannel        = ptChannelInst->ulChannelNumber;
    tDevInfo.ulDeviceNumber   = ptDevInstance->ulDeviceNumber;
    tDevInfo.ulSerialNumber   = ptDevInstance->ulSerialNumber;
    tDevInfo.ptDeviceInstance = ptDevInstance;

    OS_Memset( &tPacket, 0, sizeof(tPacket));

    if ( !USER_GetWarmstartParameters( &tDevInfo, &tPacket))
    {
      /* No warm start parameter available */
      if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_DEBUG,
                  "No warm start parameter found or available!");
      }
    } else
    {
      /* Send warm start parameter to hardware */
      int32_t lChannelError = DEV_TransferPacket( ptChannelInst,
                                                  &tPacket,
                                                  &tRecvPacket,
                                                  sizeof(tRecvPacket),
                                                  CIFX_TO_SEND_PACKET,
                                                  NULL,
                                                  NULL);

      if( (CIFX_NO_ERROR  != lChannelError) ||
          (SUCCESS_HIL_OK != LE32_TO_HOST(tRecvPacket.tHeader.ulState)) )
      {
        /* Error sending warm start parameter */
        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Error sending warm start parameter to hardware! (lSendError=0x%08X,ulSta=0x%08X)",
                    lChannelError,
                    LE32_TO_HOST(tPacket.tHeader.ulState));
        }
      } else
      {
        /*--------------------------------------------*/
        /* Wait until STACK is READY/RUNNING          */
        /*--------------------------------------------*/
        if (DEV_WaitForRunning_Poll( ptChannelInst, CIFX_TO_FIRMWARE_START))
        {
          /* Firmware started after warm start process */
          if(g_ulTraceLevel & TRACE_LEVEL_INFO)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_INFO,
                      "Successfully sent warm start parameters to Channel #%d!",
                      ulBlockID);
          }
        } else
        {
          /* Firmware not started after warm start process */
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "Hardware not Ready/Running after channel warm start!");
          }
        }
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Create all channel instances for the given device
*   \param ptDevInstance    Instance to start up
*   \param ptDevChannelCfg  Channel configuration data (downloaded files, etc.)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXCreateChannels(PDEVICEINSTANCE ptDevInstance, PDEVICE_CHANNEL_CONFIG ptDevChannelCfg)
{
  int32_t         lRet     = CIFX_NO_ERROR;

  /* Process the system channel and create devices */
  HIL_DPM_SYSTEM_CHANNEL_T*         ptSysChannel     = (HIL_DPM_SYSTEM_CHANNEL_T*)ptDevInstance->tSystemDevice.pbDPMChannelStart;
  HIL_DPM_SYSTEM_CHANNEL_INFO_T*    ptSysChannelInfo = (HIL_DPM_SYSTEM_CHANNEL_INFO_T*)&ptSysChannel->atChannelInfo[HIL_DPM_SYSTEM_CHANNEL_INDEX];
  HIL_DPM_HANDSHAKE_CHANNEL_INFO_T* ptHskBlockInfo   = (HIL_DPM_HANDSHAKE_CHANNEL_INFO_T*)&ptSysChannel->atChannelInfo[HIL_DPM_HANDSHAKE_CHANNEL_INDEX];
  HIL_DPM_CHANNEL_INFO_BLOCK_T*     ptChannel        = NULL;
  uint32_t                          ulChannelID      = 0; /* The dedicated communication channel ID */
  uint32_t                          ulBlockID        = 0;
  HIL_DPM_HANDSHAKE_ARRAY_T*        ptHskBlock       = NULL;

  /* Calculate the start address in the DPM */
  uint32_t ulDPMChannelStartAddress  = 0;
  uint32_t ulDPMChannelStartIdx      = HIL_DPM_SYSTEM_CHANNEL_INDEX;         /* Start of the communication channel definitions */

  ulDPMChannelStartAddress  = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannelInfo->ulSizeOfChannel)); /* Start behind the system channel */
  ptHskBlock                = (HIL_DPM_HANDSHAKE_ARRAY_T*)(ptDevInstance->pbDPM +
                              LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannelInfo->ulSizeOfChannel)));

  /* Check if we have a following handshake channel. This will be always at position 1 in the
     information structure */
  if(HWIF_READ8(ptDevInstance, ptHskBlockInfo->bChannelType) == HIL_CHANNEL_TYPE_HANDSHAKE)
  {
    /* There is a handshake block, add the size to the start address */
    ptDevInstance->pbHandshakeBlock = ptDevInstance->pbDPM + ulDPMChannelStartAddress;
    ulDPMChannelStartAddress       += LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptHskBlockInfo->ulSizeOfChannel));
    ulDPMChannelStartIdx            = HIL_DPM_COM_CHANNEL_START_INDEX;
  }

  /* Iterate over all block definitions. */
  ptChannel = &ptSysChannel->atChannelInfo[ulDPMChannelStartIdx];

  /*---------------------------------------------------------*/
  /* Create communication channels depending on the          */
  /* configuration information from the system channel       */
  /*---------------------------------------------------------*/
  for(ulBlockID = ulDPMChannelStartIdx; ulBlockID < HIL_DPM_MAX_SUPPORTED_CHANNELS; ++ulBlockID)
  {
    int fCreateChannel = 0;

    /* Check Block types */
    switch(HWIF_READ8(ptDevInstance, ptChannel->tSystem.bChannelType))
    {
      case HIL_CHANNEL_TYPE_COMMUNICATION:
      case HIL_CHANNEL_TYPE_APPLICATION:
        /* This is a communication or application channel, create a device for this block */
        fCreateChannel = 1;
      break;

      case HIL_CHANNEL_TYPE_HANDSHAKE:
        /* Handshake block start address must be remembered for PCI cards to
           access bits in IRQ mode */
        ptDevInstance->pbHandshakeBlock = ptDevInstance->pbDPM + ulDPMChannelStartAddress;
        break;

      case HIL_CHANNEL_TYPE_UNDEFINED:
      case HIL_CHANNEL_TYPE_RESERVED:
      case HIL_CHANNEL_TYPE_SYSTEM:
      default:
        /* Do not process these types */
      break;

    } /* end switch */

    /* Check if we have to create a channel */
    if(fCreateChannel)
    {
      PCHANNELINSTANCE ptChannelInst = NULL;
      void* pvInitMutex = NULL;
      void* pvLock      = NULL;

      /* Check the new channel is still inside the DPM, to prevent access errors */
      /* if the channel configuration does not match the maximum channel size */
      if( ptDevInstance->ulDPMSize < (LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptChannel->tCom.ulSizeOfChannel)) + ulDPMChannelStartAddress))
      {
        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                     TRACE_LEVEL_ERROR,
                     "Channel (%u) size exceeds the maximum DPM size!",
                     ulChannelID);
        }

        ptDevInstance->lInitError = CIFX_DEV_DPMSIZE_MISMATCH;

        break;    /* Skip further channel creation */
      }

      /* Allocate a channel instance */
      ptChannelInst = (PCHANNELINSTANCE)OS_Memalloc(sizeof(*ptChannelInst));

      if (NULL == ptChannelInst                    ||
          NULL == (pvInitMutex = OS_CreateMutex()) ||
          NULL == (pvLock      = OS_CreateLock())  )
      {
        lRet = CIFX_INVALID_POINTER;

        OS_Memfree(ptChannelInst);
        OS_DeleteMutex(pvInitMutex);
        OS_DeleteLock(pvLock);
        ptChannelInst = NULL;
        pvInitMutex   = NULL;
        pvLock        = NULL;

        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Error creating channel instance buffer!");
        }

      } else
      {
        OS_Memset(ptChannelInst, 0, sizeof(*ptChannelInst));

        ptChannelInst->ulChannelNumber    = ulChannelID;
        ptChannelInst->ulBlockID          = ulBlockID;
        ptChannelInst->pbDPMChannelStart  = ptDevInstance->pbDPM + ulDPMChannelStartAddress;
        ptChannelInst->ulDPMChannelLength = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptChannel->tCom.ulSizeOfChannel));

        /* These Locks/Mutexes are needed during initialization as we want to send packets, etc.
           They need to be removed if the channel is not being created (e.g. wrong channel type) */
        ptChannelInst->pvLock             = pvLock;
        ptChannelInst->pvInitMutex        = pvInitMutex;
        ptChannelInst->pvDeviceInstance   = (void*)ptDevInstance;

        ptChannelInst->ptHandshakeCell    = (HIL_DPM_HANDSHAKE_CELL_T*)&ptHskBlock->atHsk[ulBlockID];
        if((HWIF_READ8(ptDevInstance, ptChannel->tCom.bSizePositionOfHandshake) & HIL_HANDSHAKE_POSITION_MASK) == HIL_HANDSHAKE_POSITION_BEGINNING)
          ptChannelInst->ptHandshakeCell  = (HIL_DPM_HANDSHAKE_CELL_T*)ptChannelInst->pbDPMChannelStart;

        ptChannelInst->bHandshakeWidth    = HWIF_READ8(ptDevInstance, ptChannel->tCom.bSizePositionOfHandshake) & HIL_HANDSHAKE_SIZE_MASK;

        DEV_ReadHostFlags(ptChannelInst, 1);
        DEV_ReadHandshakeFlags(ptChannelInst, 0, 0);

        /* Read channel layout */
        if (CIFX_NO_ERROR != (lRet = cifXReadChannelLayout(ptDevInstance, ptChannelInst, HWIF_READ8(ptDevInstance, ptSysChannel->atChannelInfo[ulBlockID].tCom.bNumberOfBlocks))))
        {
          /* Could not read channel layout, delete the previous allocated channel instance.
           * This will remove all allocated resources of the channel instance. */
          cifXDeleteChannelInstance(ptChannelInst);

        } else
        {
          /* Read the host flag once, to keep them in sync with the actual DPM state */
          DEV_ReadHostFlags(ptChannelInst, 1);
          DEV_ReadHandshakeFlags(ptChannelInst, 0, 0);

          /* Check if we have an communication channel. Than we have to make sure,
             all necessary block are availbale  */

          /* TODO: WHAT HAPPENS IF WE HAVE AN APPLICATION CHANNEL */
          if( HIL_CHANNEL_TYPE_COMMUNICATION != HWIF_READ8(ptDevInstance, ptChannel->tSystem.bChannelType))
          {
            /* We only creating COMMUNICATION Channels at this point */
            fCreateChannel = 0;   /* Skip further processing */
          } else
          {
            /* We have a Communication channel, check it */
            if( (NULL == ptChannelInst->ptCommonStatusBlock) ||
                (NULL == ptChannelInst->ptControlBlock)      ||
                (NULL == ptChannelInst->tSendMbx.ptSendMailboxStart)  ||
                (NULL == ptChannelInst->tRecvMbx.ptRecvMailboxStart) )
            {
              /* Channel does not meet minimum system requirements and is ignored */
              fCreateChannel = 0;   /* Skip further processing */

              if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
              {
                USER_Trace(ptDevInstance,
                          TRACE_LEVEL_ERROR,
                          "Channel (%u) does not meet minimum requirement and is ignored!",
                          ulBlockID);
              }
            } else
            {
              /* This is a real channel */
              ptChannelInst->fIsChannel = 1;
            }
          }

          /* Check if we have to creat a channel */
          if(fCreateChannel)
          {
            /* If a Firmware/Firmware module was loaded.we have to wait until the Stack is READY */
            /* This should prevent the timeout for waiting on CHANNEL-READY! if only a configuration is loaded */
            /* or a channel definition exists without a channel. */
            if( ptDevChannelCfg->fFWLoaded)
            {
              int fWait = 1;
              if( (ptDevInstance->fModuleLoad) &&
                  !(ptDevChannelCfg->atChannelData[ulChannelID].fModuleLoaded))
              {
                fWait = 0;
              }

              if( fWait)
              {
                /*--------------------------------------------------------*/
                /* We created a new channel, now read firmware information */
                /* Wait until STACK is READY before communicating with it */
                /*--------------------------------------------------------*/
                if (!DEV_WaitForReady_Poll(ptChannelInst, CIFX_TO_FIRMWARE_START))
                {
                  /* READY failed */
                  if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
                  {
                    USER_Trace(ptDevInstance,
                              TRACE_LEVEL_WARNING,
                              "Error channel not READY, channel = %d)", ulChannelID);
                  }
                } else
                {
                  /* We need the actual state of all channel flags, including our own one */
                  DEV_ReadHostFlags( ptChannelInst, 1);
                }
              }
            }

            ++ulChannelID;
            ++ptDevInstance->ulCommChannelCount;
            ptDevInstance->pptCommChannels = (PCHANNELINSTANCE*)OS_Memrealloc(ptDevInstance->pptCommChannels, ptDevInstance->ulCommChannelCount * (uint32_t)sizeof(*ptDevInstance->pptCommChannels));

            if (NULL == ptDevInstance->pptCommChannels)
            {
              lRet = CIFX_INVALID_POINTER;

              if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
              {
                USER_Trace(ptDevInstance,
                          TRACE_LEVEL_ERROR,
                          "Error creating communication channel buffer!");
              }

            } else
            {
              ptDevInstance->pptCommChannels[ptDevInstance->ulCommChannelCount - 1] = ptChannelInst;

              /* Check ready again including COS flag handling, because we have to handle the COS flags */
              /* If a module firmware was loaded we are waiting before on the channel ready. */
              /* If we have not downloaded a firmware / module we skipping the prior test but we have to */
              /* make sure the channel is READY and we have also to handle COS flags in this case! */
              if( DEV_WaitForReady_Poll(ptChannelInst, 20))
              {
                int32_t lTempError = CIFX_NO_ERROR;
                if ( CIFX_NO_ERROR != (lTempError = cifXReadFirmwareIdent( ptDevInstance,
                                                                           ptChannelInst->ulChannelNumber,
                                                                           NULL,
                                                                           NULL)))
                {
                  if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
                  {
                    USER_Trace(ptDevInstance,
                              TRACE_LEVEL_ERROR,
                              "Failed to read firmware identification for channel = %d, error: 0x%08X", ptChannelInst->ulChannelNumber, lTempError);
                  }
                }

                if(g_ulTraceLevel & TRACE_LEVEL_INFO)
                {
                  USER_Trace(ptDevInstance,
                            TRACE_LEVEL_INFO,
                            "Device successfully created for channel = %d", ptChannelInst->ulChannelNumber);
                }
              }
            }

          } else if( NULL != ptChannelInst )
          {
            /* We have not created a channel, delete the previous allocated channel instance */
            cifXDeleteChannelInstance(ptChannelInst);
          }
        }
      }
    }

    /* Next Block */
    ulDPMChannelStartAddress += LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptChannel->tCom.ulSizeOfChannel));
    ptChannel++;
  }

  if( (g_ulTraceLevel & TRACE_LEVEL_INFO) &&
      (0 == ptDevInstance->ulCommChannelCount) )
  {
    USER_Trace(ptDevInstance,
               TRACE_LEVEL_INFO,
               "NO CHANNEL INFORMATION FOUND, No devices created!");
  }

  return lRet;
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
    PCHANNELINSTANCE ptChannelInst = &ptDevInstance->tSystemDevice;
    uint32_t         ulChannel     = 0;
    uint32_t         ulSync;

    /* create all synch events */
    for(ulSync = 0; ulSync < sizeof(ptDevInstance->tSyncData.ahSyncBitEvents) / sizeof(ptDevInstance->tSyncData.ahSyncBitEvents[0]); ++ulSync)
    {
      if (NULL == (ptDevInstance->tSyncData.ahSyncBitEvents[ulSync] = OS_CreateEvent()))
      {
        lRet = CIFX_INVALID_POINTER;

        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Error creating sync event buffer!");
        }

        break;
      }
    }

    if (CIFX_NO_ERROR == lRet)
    {
      /* Create events for all channels */
      do
      {
        uint32_t ulHandshakeWidth = 8;
        uint32_t ulIdx            = 0;

        /* Create interrupt events if we are working in interrupt mode */
        if(ptChannelInst->bHandshakeWidth == HIL_HANDSHAKE_SIZE_16BIT)
        {
          ulHandshakeWidth = 16;
        }

        for(ulIdx = 0; ulIdx < ulHandshakeWidth; ++ulIdx)
        {
          if (NULL == (ptChannelInst->ahHandshakeBitEvents[ulIdx] = OS_CreateEvent()))
          {
            lRet = CIFX_INVALID_POINTER;

            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        TRACE_LEVEL_ERROR,
                        "Error creating interrupt event buffer!");
            }

            break;
          }
        }

        /* Stop processing of further channels if lRet is set. */
        if (CIFX_NO_ERROR != lRet)
          break;

        /* Check if we have such a channel */
        if(ulChannel < ptDevInstance->ulCommChannelCount)
          ptChannelInst = ptDevInstance->pptCommChannels[ulChannel];

        /* Note: Check for <= as we are additionally evaluating the system channel */
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
      if (g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                   TRACE_LEVEL_ERROR,
                   "USER_GetCachedIOBufferMode() returned invalid caching mode");
      }
    break;
  }

  return lRet;
}

#ifdef CIFX_TOOLKIT_DMA
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
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                       TRACE_LEVEL_ERROR,
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

#ifdef CIFX_TOOLKIT_TIME
/*****************************************************************************/
/*! Initialize RTC
*   \param ptDevInstance Instance to start up                                */
/*****************************************************************************/
void cifXInitTime(PDEVICEINSTANCE ptDevInstance)
{
  HIL_DPM_SYSTEM_CHANNEL_T*  ptSystemChannel = (HIL_DPM_SYSTEM_CHANNEL_T*)(ptDevInstance->tSystemDevice.pbDPMChannelStart);
  uint32_t ulRTCInfo = (HIL_SYSTEM_HW_RTC_MSK & LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSystemChannel->tSystemState.ulHWFeatures)));

  /* Check if RTC is available and not already set */
  if( 0 != (HIL_SYSTEM_HW_RTC_TYPE_MSK & ulRTCInfo))
  {
    /* Check if it is already set */
    if( 0 == (HIL_SYSTEM_HW_RTC_STATE & ulRTCInfo))
    {
      int32_t lRet = 0;

      /* Create a time request*/
      HIL_TIME_CMD_REQ_T  tSendPkt;
      CIFX_PACKET         tRecvPkt;

      OS_Memset(&tSendPkt, 0, sizeof(tSendPkt));
      OS_Memset(&tRecvPkt, 0, sizeof(tRecvPkt));

      /* Set the time on the device */
      tSendPkt.tHead.ulDest = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
      tSendPkt.tHead.ulSrc  = HOST_TO_LE32(ptDevInstance->ulPhysicalAddress);
      tSendPkt.tHead.ulCmd  = HOST_TO_LE32(HIL_TIME_COMMAND_REQ);
      tSendPkt.tHead.ulLen  = HOST_TO_LE32(sizeof(tSendPkt.tData));

      tSendPkt.tData.ulTimeCmd = TIME_CMD_SETTIME;
      /* Get actual system time */
      tSendPkt.tData.ulData    = (uint32_t)OS_Time(NULL);

      /* Transfer packet */
      lRet = DEV_TransferPacket( &ptDevInstance->tSystemDevice,
                                 (CIFX_PACKET*)&tSendPkt,
                                 &tRecvPkt,
                                 sizeof(tRecvPkt),
                                 CIFX_TO_SEND_PACKET,
                                 NULL,
                                 NULL);

      if( (CIFX_NO_ERROR  != lRet) ||
          (SUCCESS_HIL_OK != LE32_TO_HOST(tRecvPkt.tHeader.ulState)) )
      {
        if(g_ulTraceLevel & TRACE_LEVEL_WARNING)
        {
          USER_Trace(ptDevInstance,
                     TRACE_LEVEL_WARNING,
                     "Error setting device time! (lRet=0x%08X, ulState=0x%08X)",
                     lRet,
                     LE32_TO_HOST(tRecvPkt.tHeader.ulState));
        }
      }else
      {
        if(g_ulTraceLevel & TRACE_LEVEL_INFO)
        {
          USER_Trace(ptDevInstance,
                     TRACE_LEVEL_INFO,
                     "Setting RTC done: 0x%08X (%u)",
                     tSendPkt.tData.ulData,
                     tSendPkt.tData.ulData);
        }
      }
    }
  }
}
#endif

/*****************************************************************************/
/*! Check is BASE OS module running
*   \param ptDevInstance Instance to start up
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXIsBaseOSModule(PDEVICEINSTANCE ptDevInstance)
{
  HIL_DPM_SYSTEM_CHANNEL_T*  ptSystemChannel = (HIL_DPM_SYSTEM_CHANNEL_T*)(ptDevInstance->tSystemDevice.pbDPMChannelStart);

  ptDevInstance->fModuleLoad = 0;
  if( 0 != (0x80000000 & LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSystemChannel->tSystemState.ulSystemStatus))))
    ptDevInstance->fModuleLoad = 1;

  return CIFX_NO_ERROR;
}

/*****************************************************************************/
/*! Evaluate device type
*   \param ptDevInstance Instance to start up
*   \return SUCCESS_HIL_OK if device type is OK                              */
/*****************************************************************************/
static int32_t cifXEvaluateDeviceType(PDEVICEINSTANCE ptDevInstance)
{
  /* Evaluate the defined device type to process basic hardware setup handling including
     firmware / module / configuration file download and hardware startup.

     Possible device types:

     eCIFX_DEVICE_AUTODETECT:
     Try to find out if a ROM loader or firmware is running
     => Hardware defined as PCI hardware is generally a RAM based device
     => A known cookie in the DPM tells us if a firmware is running and the device is FLASH based
     => If the cookie is unknown and the DPM size is 64 KByte we expect a RAM based device
     => If the DPM is smaller than 64 Kbyte, than the device detection fails

     eCIFX_DEVICE_RAM_BASED:
     Configuration has defined a RAM based device

     eCIFX_DEVICE_FLASH_BASED:
     Configuration has defined a FLASH based device

     eCIFX_DEVICE_DONT_TOUCH:
     Evaluate the current state of the device without changing anything

  */

  int32_t lRet = CIFX_INVALID_BOARD;

  /*-----------------------------------------------------------*/
  /* Check for FLASH based device                              */
  /* and                                                       */
  /* Check for DON'T TOUCH device                              */
  /*-----------------------------------------------------------*/
  if( (eCIFX_DEVICE_FLASH_BASED == ptDevInstance->eDeviceType) ||
      (eCIFX_DEVICE_DONT_TOUCH  == ptDevInstance->eDeviceType))
  {
    /* In both cases we expect to have a valid cookie on the beginning of the DPM */
    char szCookie[5];

    OS_Memset(szCookie, 0, sizeof(szCookie));

    HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);

    if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
    {
      if(eCIFX_DEVICE_FLASH_BASED == ptDevInstance->eDeviceType)
      {
        USER_Trace(ptDevInstance,
                   TRACE_LEVEL_DEBUG,
                   "Device Type is fix defined to: eCIFX_DEVICE_FLASH_BASED");
      }else
      {
        USER_Trace(ptDevInstance,
                   TRACE_LEVEL_DEBUG,
                   "Device Type is fix defined to: eCIFX_DEVICE_DONT_TOUCH");
      }
    }

    /* Check for a valid cookie */
    if( (0 == OS_Strcmp( szCookie, CIFX_DPMSIGNATURE_BSL_STR)) ||
        (0 == OS_Strcmp( szCookie, CIFX_DPMSIGNATURE_FW_STR)) )
    {
      /* Return the configured device type device */
      lRet = CIFX_NO_ERROR;
    }else
    {
      USER_Trace( ptDevInstance,
            TRACE_LEVEL_ERROR,
            "Detect device type, invalid cookie found! (cookie='%02X','%02X','%02X','%02X')",
            szCookie[0],
            szCookie[1],
            szCookie[2],
            szCookie[3]);
    }

  /*-----------------------------------------------------------*/
  /* Check for RAM based device                              */
  /*-----------------------------------------------------------*/
  }else if( eCIFX_DEVICE_RAM_BASED == ptDevInstance->eDeviceType)
  {
    /* RAM based devices are always started by a hardware reset, followed by a BSL / FW download */
    if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
    {
      USER_Trace(ptDevInstance,
                 TRACE_LEVEL_DEBUG,
                 "Device Type is fixed defined to: eCIFX_DEVICE_RAM_BASED");
    }

    /* Evaluation OK */
    lRet = CIFX_NO_ERROR;

  /*-----------------------------------------------------------*/
  /* Try to autodetect the device type                         */
  /*-----------------------------------------------------------*/
  }else if( eCIFX_DEVICE_AUTODETECT == ptDevInstance->eDeviceType)
  {
    /* Check for PCI hardware first */
    if(ptDevInstance->fPCICard)
    {
      /* All current PCI cards are RAM based, so default to RAM
         NOTE: If the user builds a flash based PCI card, he must pass
               eCIFX_DEVICE_AUTODETECT */

      if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
      {
        USER_Trace(ptDevInstance,
                   TRACE_LEVEL_DEBUG,
                   "Device Type autodetection: RAM Based Device found!");
      }

      /* Evaluation OK */
      ptDevInstance->eDeviceType = eCIFX_DEVICE_RAM_BASED;
      lRet = CIFX_NO_ERROR;

    /* Check for DPM hardware */
    } else
    {
      /* None PCI device depending on the DPM content
         If we have a valid cookie, than we have a FLASH based device */
      char szCookie[5];

      OS_Memset(szCookie, 0, sizeof(szCookie));

      HWIF_READN(ptDevInstance, szCookie, ptDevInstance->pbDPM, 4);

      /* Check for a valid cookie */
      if( (0 == OS_Strcmp( szCookie, CIFX_DPMSIGNATURE_BSL_STR)) ||
          (0 == OS_Strcmp( szCookie, CIFX_DPMSIGNATURE_FW_STR)) )
      {
        /* We have a firmware or bootloader running, so we assume it is a flash based device */
        /* NOTE: If the driver is restarted and a RAM based FW was downloaded before this
                 will result in the device being handled as flash based.
                 Currently there is no way to detect this */
        if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
        {
          USER_Trace(ptDevInstance,
                     TRACE_LEVEL_DEBUG,
                     "Device Type autodetection: Flash Based Device found!");
        }

        ptDevInstance->eDeviceType = eCIFX_DEVICE_FLASH_BASED;
        lRet = CIFX_NO_ERROR;

      /* If it is not a PCI device and the cookie could not be evaluated
         we try to check if we can handle it also as a RAM based device. */
      } else
      {
        /* If the DPM size is equal to 64 Kbyte we are able to access to the "netX Global Register Block" and we are not able
           start a firmware via the DPM and we could try to handle the device as a RAM based device.
           If the DPM size is less than 64 KByte we can't handle the device at all. */

        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                     TRACE_LEVEL_ERROR,
                     "Device Type autodetection: DPM device with unknown cookie detected, try to handle it as a RAM based device (cookie='%02X','%02X','%02X','%02X').",
                     szCookie[0],
                     szCookie[1],
                     szCookie[2],
                     szCookie[3]);
        }

        /* Check DPM size */
        if(ptDevInstance->ulDPMSize < NETX_DPM_MEMORY_SIZE)
        {
          /* We don't have access to Global register block and no FW or Bootloader is running
             and we are not able to execute a reset and to work with this card */
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "Device Type autodetection: Driver is unable to start a RAM based device with a DPM < 64kB.");
          }

        } else
        {

          if(g_ulTraceLevel & TRACE_LEVEL_INFO)
          {
            USER_Trace(ptDevInstance,
                       TRACE_LEVEL_INFO,
                       "Device Type autodetection: RAM based device forced (No FW / Bootloader active)! Card will be reset and all files downloaded!");
          }

          ptDevInstance->eDeviceType = eCIFX_DEVICE_RAM_BASED;
          lRet = CIFX_NO_ERROR;
        }
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Basic netX device start-up
*   \param ptDevInstance Instance to start up
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifXStartDevice(PDEVICEINSTANCE ptDevInstance)
{
  int32_t               lRet            = CIFX_DRV_INIT_STATE_ERROR;
  DEVICE_CHANNEL_CONFIG tDevChannelCfg;

  OS_Memset(&tDevChannelCfg, 0, sizeof(tDevChannelCfg));

  ptDevInstance->lInitError = CIFX_NO_ERROR;

  /* Assume every card has the register block at the end of the DPM */
  ptDevInstance->ptGlobalRegisters = (PNETX_GLOBAL_REG_BLOCK)(ptDevInstance->pbDPM +
                                                              ptDevInstance->ulDPMSize -
                                                              sizeof(NETX_GLOBAL_REG_BLOCK));

  /* Try to determine RAM or Flash based device configuration */
  if( CIFX_NO_ERROR == (lRet = cifXEvaluateDeviceType(ptDevInstance)) )
  {

    switch(ptDevInstance->eDeviceType)
    {
      case eCIFX_DEVICE_RAM_BASED:
        /*-----------------------------------*/
        /* This is a RAM based device        */
        /*-----------------------------------*/
        if( (CIFX_NO_ERROR == (lRet = cifXStartRAMDevice(ptDevInstance))) &&
            (CIFX_NO_ERROR == (lRet = cifXCreateSystemDevice( ptDevInstance)))  )
        {
          /* Just store error happening here into lInitError. This will make sure
             that this device which already has a system channel, being handled by
             Toolkit even if firmware startup fails (e.g. Wrong firmware for this card) */
          int32_t lTempResult;

          /* Check if we have a BASE OS system to download and to start*/
          lTempResult = cifXHandleRAMBaseOSModule( ptDevInstance);
          if( CIFX_NO_ERROR == lTempResult)
          {
            /* Check if we have a BASE OS module running */
            (void)cifXIsBaseOSModule(ptDevInstance);

            /* Download firmware / module files */
            (void)cifXDownloadFWFiles(ptDevInstance, &tDevChannelCfg);

            /* Download configuration files */
            (void)cifXDownloadCNFFiles(ptDevInstance, &tDevChannelCfg);

            /* Start firmware / module files if necessary */
            lTempResult = cifXStartRAMFirmware(ptDevInstance, &tDevChannelCfg);
          }

          /* Only enter our error if no function already inserted one. Readout of channel
             Information may already have inserted an error */
          if(CIFX_NO_ERROR == ptDevInstance->lInitError)
            ptDevInstance->lInitError = lTempResult;
        }
        break;

      case eCIFX_DEVICE_FLASH_BASED:
        /*-----------------------------------*/
        /* This is a flash based device      */
        /*-----------------------------------*/
        if( IsNetX4x00FLASH(ptDevInstance) ||
            IsNetX90FLASH(ptDevInstance)    )
        {
          /* netX90 and netX4000 devices should only be actively updated by user,
             therefore no update handling is done in case user defines
             eCIFX_DEVICE_FLASH_BASED */

          if( (CIFX_NO_ERROR != (lRet = cifXStartFlashDevice( ptDevInstance)))  ||
              (CIFX_NO_ERROR != (lRet = cifXCreateSystemDevice( ptDevInstance))) )
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "Unable to access the hardware while device type is set to eCIFX_DEVICE_FLASH_BASED. Aborting device handling!");
          }

        }
        else if( (CIFX_NO_ERROR == (lRet = cifXStartFlashDevice( ptDevInstance)))   &&
                 (CIFX_NO_ERROR == (lRet = cifXCreateSystemDevice( ptDevInstance)))  )
        {
          /* Just store error happening here into lInitError. This will make sure
             that this device which already has a system channel, being handled by
             Toolkit even if firmware startup fails (e.g. Wrong firmware for this card) */
          int32_t lTempResult;

          /* Check if we have a BASE OS system to download and to start*/
          lTempResult = cifXHandleFlashBaseOSModule( ptDevInstance);
          if( CIFX_NO_ERROR == lTempResult)
          {
            /* Check if we have a BASE OS module running */
            (void)cifXIsBaseOSModule(ptDevInstance);

            /* Download firmware / module files */
            (void)cifXDownloadFWFiles(ptDevInstance, &tDevChannelCfg);

            /* Download configuration files */
            (void)cifXDownloadCNFFiles(ptDevInstance, &tDevChannelCfg);

            /* Start firmware / module files if necessary */
            lTempResult = cifXStartFlashFirmware(ptDevInstance, &tDevChannelCfg);
          }

          /* Only enter our error if no function already inserted one. Readout of channel
             Information may already have inserted an error */
          if(CIFX_NO_ERROR == ptDevInstance->lInitError)
            ptDevInstance->lInitError = lTempResult;
        }
        break;

      case eCIFX_DEVICE_DONT_TOUCH:
        /* Leave the device in the current state, don't execute a reset and expect it is running */

        if( CIFX_NO_ERROR != (lRet = cifXCreateSystemDevice( ptDevInstance)))
        {
          USER_Trace(ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Unable to access the hardware while device type is set to eCIFX_DEVICE_DONT_TOUCH. Aborting device handling!");
        }
        break;

      default:
        /* This should never happen */
        break;
    }
  }

  if(CIFX_NO_ERROR == lRet)
  {
    /* Create sync resources  */
    if (NULL == (ptDevInstance->tSyncData.pvLock = OS_CreateLock()))
    {
      lRet = CIFX_INVALID_POINTER;

      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_ERROR,
                  "Error creating sync resources!");
      }

    } else
    {
      HIL_DPM_SYSTEM_CHANNEL_T* ptSysChannel = (HIL_DPM_SYSTEM_CHANNEL_T*)ptDevInstance->tSystemDevice.pbDPMChannelStart;

      /* NOTE: If the Slot Number is different after FW start (e.g. Firmware does not support
               Slot Number), we overwrite it with the internal value to make sure the Slot Number
               is identical between Bootloader and Firmware */
      if(ptDevInstance->ulSlotNumber != HWIF_READ8(ptDevInstance, ptSysChannel->tSystemInfo.bDevIdNumber))
        HWIF_WRITE8(ptDevInstance, ptSysChannel->tSystemInfo.bDevIdNumber, (uint8_t)HOST_TO_LE32(ptDevInstance->ulSlotNumber));

      /* Check if we already have a netX Chip-Type information,       */
      /* if not try to read them via a HIL_HW_IDENTFY_REQ.            */
      /* If this does not work, use eCHIP_TYPE_UNKNOWN, like before.  */
      if( eCHIP_TYPE_UNKNOWN == ptDevInstance->eChipType)
      {
        if( CIFX_NO_ERROR != cifXReadHardwareIdent( ptDevInstance, NULL, NULL))
        {
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "Error reading chip type!");
          }
        }
      }

      /* Read the channel layouts, and build the CHANNELINSTANCES for this device */
      lRet = cifXCreateChannels(ptDevInstance, &tDevChannelCfg);
    }
  }

#ifdef CIFX_TOOLKIT_TIME
  if(CIFX_NO_ERROR == lRet)
  {
    /* Update the system time of the target if a RTC is available */
    cifXInitTime(ptDevInstance);
  }
#endif

  if(CIFX_NO_ERROR == lRet)
  {
    /* On Flash based devices we may need to perform a CHANNEL_INIT if we have
       updated the configuration */
    if(eCIFX_DEVICE_FLASH_BASED == ptDevInstance->eDeviceType)
    {
      lRet = cifXStartFlashConfiguration(ptDevInstance, &tDevChannelCfg);
    }

    /* Handle warmstart for all channels */
    if(CIFX_NO_ERROR == lRet)
      lRet = cifXHandleWarmstartParameter(ptDevInstance);
  }

#ifdef CIFX_TOOLKIT_DMA
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
  int32_t          lRet = CIFX_NO_ERROR;
  uint32_t         ulIdx          = 0;
  PCHANNELINSTANCE ptSystemDevice = &ptDevInstance->tSystemDevice;

  /* Process all created communication channels */
  for(ulIdx = 0; ulIdx < ptDevInstance->ulCommChannelCount; ++ulIdx)
  {
    PCHANNELINSTANCE ptChannelInst = ptDevInstance->pptCommChannels[ulIdx];

    cifXDeleteChannelInstance(ptChannelInst);
  }

  /*-------------------------------------------------*/
  /* Delete system channel objects                   */
  /*-------------------------------------------------*/
  /* Clean up all interrupt events */
  for( ulIdx = 0; ulIdx < sizeof(ptSystemDevice->ahHandshakeBitEvents) / sizeof(ptSystemDevice->ahHandshakeBitEvents[0]); ++ulIdx)
  {
    if(NULL != ptSystemDevice->ahHandshakeBitEvents[ulIdx])
    {
      OS_DeleteEvent(ptSystemDevice->ahHandshakeBitEvents[ulIdx]);
      ptSystemDevice->ahHandshakeBitEvents[ulIdx] = NULL;
    }
  }

  OS_DeleteLock(ptSystemDevice->pvLock);
  ptSystemDevice->pvLock = NULL;
  OS_DeleteMutex(ptSystemDevice->pvInitMutex);
  ptSystemDevice->pvInitMutex = NULL;
  OS_DeleteMutex(ptSystemDevice->tRecvMbx.pvRecvMBXMutex);
  ptSystemDevice->tRecvMbx.pvRecvMBXMutex = NULL;
  OS_DeleteMutex(ptSystemDevice->tSendMbx.pvSendMBXMutex);
  ptSystemDevice->tSendMbx.pvSendMBXMutex = NULL;

  /*-------------------------------------------------*/
  /* Delete Communication channel array              */
  /*-------------------------------------------------*/
  OS_Memfree(ptDevInstance->pptCommChannels);
  ptDevInstance->pptCommChannels    = NULL;
  ptDevInstance->ulCommChannelCount = 0;

  /* Remove sync resources */
  for(ulIdx = 0; ulIdx < sizeof(ptDevInstance->tSyncData.ahSyncBitEvents) / sizeof(ptDevInstance->tSyncData.ahSyncBitEvents[0]); ++ulIdx)
  {
    if(NULL != ptDevInstance->tSyncData.ahSyncBitEvents[ulIdx])
    {
      OS_DeleteEvent(ptDevInstance->tSyncData.ahSyncBitEvents[ulIdx]);
      ptDevInstance->tSyncData.ahSyncBitEvents[ulIdx] = NULL;
    }
  }
  OS_DeleteLock(ptDevInstance->tSyncData.pvLock);
  ptDevInstance->tSyncData.pvLock = NULL;

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

      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_ERROR,
                  "Error creating device buffer!");
      }
    }
  }

  return lRet;
}

#ifdef CIFX_TOOLKIT_DMA
/*****************************************************************************/
/*! Check DMA buffer configuration.
*   \param ptDevInstance Holding the DMA buffer configuration
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t cifXTKitCheckDMABufferConfig(PDEVICEINSTANCE ptDevInstance)
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
/*! Physically Enable Interrupts on hardware
*   \param ptDevInstance Device instance                                     */
/*****************************************************************************/
void cifXTKitEnableHWInterrupt(PDEVICEINSTANCE ptDevInstance)
{
  /* Set interrupt enable bits in PCI mode only if the complete 64KByte DPM is available */
  if( (ptDevInstance->fPCICard) ||
      (ptDevInstance->ulDPMSize >= NETX_DPM_MEMORY_SIZE) )
  {
    /* Enable global and handshake interrupts */
    HWIF_WRITE32(ptDevInstance, ptDevInstance->ptGlobalRegisters->ulIRQEnable_0,
                 HOST_TO_LE32((MSK_IRQ_EN0_INT_REQ | MSK_IRQ_EN0_HANDSHAKE) ));

    HWIF_WRITE32(ptDevInstance, ptDevInstance->ptGlobalRegisters->ulIRQEnable_1, 0);
  }
}

/*****************************************************************************/
/*! Physically Disable Interrupts on hardware
*   \param ptDevInstance Device instance                                     */
/*****************************************************************************/
void cifXTKitDisableHWInterrupt(PDEVICEINSTANCE ptDevInstance)
{
  /* Clear interrupt enable bits in PCI mode or if the complete 64Kb DPM is available */
  if( (ptDevInstance->fPCICard) ||
      (ptDevInstance->ulDPMSize == NETX_DPM_MEMORY_SIZE) )
  {
    /* Disable all interrupts */
    HWIF_WRITE32(ptDevInstance, ptDevInstance->ptGlobalRegisters->ulIRQEnable_0, 0);
    HWIF_WRITE32(ptDevInstance, ptDevInstance->ptGlobalRegisters->ulIRQEnable_1, 0);
  }
}

/*****************************************************************************/
/*! Adds a newly found device to the list of handled device
*   \param ptDevInstance Device to add (must at least include the pointer to
*                        the DPM)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t cifXTKitAddDevice(PDEVICEINSTANCE ptDevInstance)
{
  int32_t lRet;

  /* Check if we have a pointer */
  if(NULL == ptDevInstance)
    return CIFX_INVALID_POINTER;

  /* Disable interrupts during startup phase. Just in case the user has set this flag! */
  ptDevInstance->fIrqEnabled = 0;

#ifdef CIFX_TOOLKIT_HWIF
  /* Validate hardware access function pointers != NULL */
  if ( (ptDevInstance->pfnHwIfRead    == NULL) ||
       (ptDevInstance->pfnHwIfWrite   == NULL)   )
    return CIFX_INVALID_PARAMETER;
#endif /* CIFX_TOOLKIT_HWIF */

#ifdef CIFX_TOOLKIT_DMA
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

      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_ERROR,
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
        if(CIFX_TKIT_IRQ_DSR_REQUESTED == cifXTKitISRHandler(ptDevInstance, 1))
          cifXTKitDSRHandler(ptDevInstance);

#ifndef CIFX_TOOLKIT_MANUAL_IRQ_ENABLE
        OS_EnableInterrupts(ptDevInstance->pvOSDependent);
        cifXTKitEnableHWInterrupt(ptDevInstance);
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
int32_t cifXTKitRemoveDevice(char* szBoard, int fForceRemove)
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
    PDEVICEINSTANCE ptDevInst   = g_pptDevices[ulIdx];
    int             fStop       = 0;
    int             fIrqEnabled = ptDevInst->fIrqEnabled;

    if(ptDevInst->fIrqEnabled)
    {
#ifndef CIFX_TOOLKIT_MANUAL_IRQ_ENABLE
      cifXTKitDisableHWInterrupt(ptDevInst);
      OS_DisableInterrupts(ptDevInst->pvOSDependent);
#endif /* CIFX_TOOLKIT_MANUAL_IRQ_ENABLE */

      /* mark IRQ as disabled, as the device is now in polling mode */
      ptDevInst->fIrqEnabled = 0;
    }

    if(fForceRemove)
    {
      /* user requested to force the remove, so don't check for open connections */
      fStop = 1;
    } else
    {
      uint32_t ulChannel = 0;

      if(ptDevInst->tSystemDevice.ulOpenCount != 0)
      {
        /* system channel is in use, so deny device removal */
        lRet = CIFX_DEV_HW_PORT_IS_USED;
      } else
      {
        fStop = 1;
        /* we need to check if any channel has an open reference */
        for(ulChannel = 0; ulChannel < ptDevInst->ulCommChannelCount; ++ulChannel)
        {
          if(ptDevInst->pptCommChannels[ulChannel]->ulOpenCount > 0)
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
      lRet = cifXStopDevice(ptDevInst);

    /* Restore IRQ mode in case the user wants to reuse this device instance */
    ptDevInst->fIrqEnabled = fIrqEnabled;
  }

  OS_LeaveLock(g_pvTkitLock);

  return lRet;
}

/*****************************************************************************/
/*! Initializes the cifX Toolkit
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t cifXTKitInit( void)
{
  int32_t lRet = CIFX_NO_ERROR;

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
      /* Toolkit successfully initialized */
      g_tDriverInfo.fInitialized = 1;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Un-Initializes the cifX Toolkit                                          */
/*****************************************************************************/
void cifXTKitDeinit( void)
{
  uint32_t ulIdx = 0;

  if(g_pvTkitLock)
  {
    OS_EnterLock(g_pvTkitLock);
  }

  for(ulIdx = 0; ulIdx < g_ulDeviceCount; ++ulIdx)
  {
    (void)cifXStopDevice(g_pptDevices[ulIdx]);
  }

  if(g_pptDevices)
  {
    OS_Memfree(g_pptDevices);
    g_pptDevices    = NULL;
  }
  g_ulDeviceCount = 0;

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
}

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/
