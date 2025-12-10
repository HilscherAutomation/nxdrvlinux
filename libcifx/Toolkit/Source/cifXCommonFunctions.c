/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXCommonFunctions.c 15325 2025-11-21 13:31:48Z AMinor $:

  Description:
    Common cifX functions and variables shared by DPM and HIF

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-07-14  created

**************************************************************************************/

/*****************************************************************************/
/*! \file cifXCommonFunctions.c
*   Common cifX API functions                                                */
/*****************************************************************************/

#include "cifXToolkit.h"
#include "cifXErrors.h"
#include "cifXEndianess.h"
#include "cifXFunctionList.h"
#include "cifXHWFunctions.h"
#include "cifXHWFunctionsWrapper.h"

#include <Hil_Results.h>

#ifdef CIFX_TOOLKIT_TIME
extern void cifXInitTime(PDEVICEINSTANCE ptDevInstance);
#endif

/*****************************************************************************/
/*! Errorcode to Errordescription lookup table (english only)                */
/*****************************************************************************/
static struct CIFX_ERROR_TO_DESCRtag
{
  int32_t  lError;
  char* szErrorDescr;

} s_atErrorToDescrTable[] =
{
#ifndef CIFX_TOOLKIT_NO_ERRORLOOKUP
  /*******************************************************************************
  * cifX Device Driver Errors (Global)
  *******************************************************************************/
  {CIFX_INVALID_POINTER              ,"Invalid pointer (e.g. NULL) passed to driver"  },
  {CIFX_INVALID_BOARD                ,"No board with the given name / index available"},
  {CIFX_INVALID_CHANNEL              ,"No channel with the given index available"     },
  {CIFX_INVALID_HANDLE               ,"Invalid handle passed to driver"               },
  {CIFX_INVALID_PARAMETER            ,"Invalid parameter"                             },
  {CIFX_INVALID_COMMAND              ,"Invalid command"                               },
  {CIFX_INVALID_BUFFERSIZE           ,"Invalid buffer size"                           },
  {CIFX_INVALID_ACCESS_SIZE          ,"Invalid access size"                           },
  {CIFX_FUNCTION_FAILED              ,"Function failed"                               },
  {CIFX_FILE_OPEN_FAILED             ,"File could not be opened"                      },
  {CIFX_FILE_SIZE_ZERO               ,"File size is zero"                             },
  {CIFX_FILE_LOAD_INSUFF_MEM         ,"Insufficient memory to load file"              },
  {CIFX_FILE_READ_ERROR              ,"Error reading from file"                       },
  {CIFX_FILE_TYPE_INVALID            ,"Invalid file type"                             },
  {CIFX_FILE_NAME_INVALID            ,"Invalid file name"                             },
  {CIFX_FUNCTION_NOT_AVAILABLE       ,"Driver function not available"                 },
  {CIFX_BUFFER_TOO_SHORT             ,"Given buffer is too short"                     },
  {CIFX_MEMORY_MAPPING_FAILED        ,"Failed to map the memory"                      },
  {CIFX_NO_MORE_ENTRIES              ,"No more entries available"                     },
  {CIFX_CALLBACK_MODE_UNKNOWN        ,"Unknown callback handling mode"                },
  {CIFX_CALLBACK_CREATE_EVENT_FAILED ,"Failed to create callback events"              },
  {CIFX_CALLBACK_CREATE_RECV_BUFFER  ,"Failed to create callback receive buffer"      },
  {CIFX_CALLBACK_ALREADY_USED        ,"Callback already used"                         },
  {CIFX_CALLBACK_NOT_REGISTERED      ,"Callback was not registered before"            },
  {CIFX_INTERRUPT_DISABLED           ,"Interrupt is disabled"                         },
  /*******************************************************************************
  * Generic Driver Errors
  *******************************************************************************/
  {CIFX_DRV_NOT_INITIALIZED          ,"Driver not initialized"                           },
  {CIFX_DRV_INIT_STATE_ERROR         ,"Driver init state error"                          },
  {CIFX_DRV_READ_STATE_ERROR         ,"Driver read state error"                          },
  {CIFX_DRV_CMD_ACTIVE               ,"Command is active on device"                      },
  {CIFX_DRV_DOWNLOAD_FAILED          ,"General error during download"                    },
  {CIFX_DRV_WRONG_DRIVER_VERSION     ,"Wrong driver version"                             },
  {CIFX_DRV_DRIVER_NOT_LOADED        ,"CIFx driver is not running"                       },
  {CIFX_DRV_INIT_ERROR               ,"Failed to initialize the device"                  },
  {CIFX_DRV_CHANNEL_NOT_INITIALIZED  ,"Channel not initialized (xChannelOpen not called)"},
  {CIFX_DRV_IO_CONTROL_FAILED        ,"IOControl call failed"                            },
  {CIFX_DRV_NOT_OPENED               ,"Driver was not opened"                            },
  {CIFX_DRV_DOWNLOAD_STORAGE_UNKNOWN ,"Unknown download storage type (RAM/FLASH based) found"},
  {CIFX_DRV_DOWNLOAD_FW_WRONG_CHANNEL,"Channel number for a firmware download not supported" },
  {CIFX_DRV_DOWNLOAD_MODULE_NO_BASEOS,"Modules are not allowed without a Base OS firmware"   },
  /*******************************************************************************
  * Generic Device Errors
  *******************************************************************************/
  {CIFX_DEV_DPM_ACCESS_ERROR         ,"Dual port memory not accessible (board not found)"},
  {CIFX_DEV_NOT_READY                ,"Device not ready (ready flag failed)"             },
  {CIFX_DEV_NOT_RUNNING              ,"Device not running (running flag failed)"         },
  {CIFX_DEV_WATCHDOG_FAILED          ,"Watchdog test failed"                             },
  {CIFX_DEV_SYSERR                   ,"Error in handshake flags"                         },
  {CIFX_DEV_MAILBOX_FULL             ,"Send mailbox is full"                             },
  {CIFX_DEV_PUT_TIMEOUT              ,"Send packet timeout"                              },
  {CIFX_DEV_GET_TIMEOUT              ,"Receive packet timeout"                           },
  {CIFX_DEV_GET_NO_PACKET            ,"No packet available"                              },
  {CIFX_DEV_RESET_TIMEOUT            ,"Reset command timeout"                            },
  {CIFX_DEV_NO_COM_FLAG              ,"COM-flag not set"                                 },
  {CIFX_DEV_EXCHANGE_FAILED          ,"I/O data exchange failed"                         },
  {CIFX_DEV_EXCHANGE_TIMEOUT         ,"I/O data exchange timeout"                        },
  {CIFX_DEV_COM_MODE_UNKNOWN         ,"Unknown I/O exchange mode"                        },
  {CIFX_DEV_FUNCTION_FAILED          ,"Device function failed"                           },
  {CIFX_DEV_DPMSIZE_MISMATCH         ,"DPM size differs from configuration"              },
  {CIFX_DEV_STATE_MODE_UNKNOWN       ,"Unknown state mode"                               },
  {CIFX_DEV_HW_PORT_IS_USED          ,"Output port already in use"                       },
  {CIFX_DEV_CONFIG_LOCK_TIMEOUT      ,"Configuration locking timeout"                    },
  {CIFX_DEV_CONFIG_UNLOCK_TIMEOUT    ,"Configuration unlocking timeout"                  },
  {CIFX_DEV_HOST_STATE_SET_TIMEOUT   ,"Set HOST state timeout"                           },
  {CIFX_DEV_HOST_STATE_CLEAR_TIMEOUT ,"Clear HOST state timeout"                         },
  {CIFX_DEV_INITIALIZATION_TIMEOUT   ,"Timeout during channel initialization"            },
  {CIFX_DEV_BUS_STATE_ON_TIMEOUT     ,"Set Bus ON timeout"                               },
  {CIFX_DEV_BUS_STATE_OFF_TIMEOUT    ," Set Bus OFF timeout"                             },
  {CIFX_DEV_MODULE_ALREADY_RUNNING   ,"Module already running"                           },
  {CIFX_DEV_MODULE_ALREADY_EXISTS    ,"Module already exists"                            },
  {CIFX_DEV_DMA_INSUFF_BUFFER_COUNT  ,"Number of configured DMA buffers insufficient"    },
  {CIFX_DEV_DMA_BUFFER_TOO_SMALL     ,"DMA buffers size too small (min size 256Byte)"    },
  {CIFX_DEV_DMA_BUFFER_TOO_BIG       ,"DMA buffers size too big (max size 63,75KByte)"   },
  {CIFX_DEV_DMA_BUFFER_NOT_ALIGNED   ,"DMA buffer alignment failed (must be 256Byte)"    },
  {CIFX_DEV_DMA_HANDSHAKEMODE_NOT_SUPPORTED ,"I/O data uncontrolled handshake mode not supported"   },
  {CIFX_DEV_DMA_IO_AREA_NOT_SUPPORTED,"I/O area in DMA mode not supported (only area 0 possible)"   },
  {CIFX_DEV_DMA_STATE_ON_TIMEOUT     ,"Set DMA ON timeout"                               },
  {CIFX_DEV_DMA_STATE_OFF_TIMEOUT    ,"Set DMA OFF timeout"                              },
  {CIFX_DEV_SYNC_STATE_INVALID_MODE  ,"Device is in invalid mode for this operation"     },
  {CIFX_DEV_SYNC_STATE_TIMEOUT       ,"Waiting for synchronization event bits timed out" },
  {CIFX_DEV_DPM_LAYOUT_UNKNOWN       ,"Unsupported DPM layout"                           },

  /*******************************************************************************/
#else
  {CIFX_NO_ERROR, ""},
#endif
};

/*****************************************************************************/
/*! Structure description of NETX_SYSTEM_INFO_BLOCK                          */
/*****************************************************************************/
const CIFX_ENDIANESS_ENTRY_T g_atSystemInfoBlock[] =
{
  /* Offset, Width, Elements */
  { 0x04, eCIFX_ENDIANESS_WIDTH_32BIT, 3}, /* DpmTotalSize, DevNr, SerNr     */
  { 0x10, eCIFX_ENDIANESS_WIDTH_16BIT, 6}, /* ausHwOptions, usMfg, usProdDat */
  { 0x1C, eCIFX_ENDIANESS_WIDTH_32BIT, 2}, /* ulLicenseFlags1/2              */
  { 0x24, eCIFX_ENDIANESS_WIDTH_16BIT, 3}, /* LicenseId/flags, DeviceClass   */
  { 0x2C, eCIFX_ENDIANESS_WIDTH_16BIT, 2}, /* ausReserved                    */
};
uint32_t g_ulSystemInfoBlockSize = HIL_CNT_ELEMENT(g_atSystemInfoBlock);

extern uint32_t                g_ulDeviceCount; /*!< Number of available device (Array size of g_pptDevices) */
extern PDEVICEINSTANCE*        g_pptDevices;    /*!< Array containing all handled device instances           */
extern TKIT_DRIVER_INFORMATION g_tDriverInfo;   /*!< Global driver information                               */

#ifndef CIFX_TOOLKIT_USE_CUSTOM_DRV_FUNCS
/*****************************************************************************/
/*! Open a connection to the driver
*   \param phDriver     Returned handle to the driver
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xDriverOpen(CIFXHANDLE* phDriver)
{
  if(!g_tDriverInfo.fInitialized)
    return CIFX_DRV_DRIVER_NOT_LOADED;

  CHECK_POINTER(phDriver);

  *phDriver = &g_tDriverInfo;

  ++g_tDriverInfo.ulOpenCount;

  return CIFX_NO_ERROR;
}

/*****************************************************************************/
/*! Close a connection to the driver
*   \param hDriver     Handle to connection, that is being closed
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xDriverClose(CIFXHANDLE hDriver)
{
  if(!g_tDriverInfo.fInitialized)
    return CIFX_DRV_DRIVER_NOT_LOADED;

  if(g_tDriverInfo.ulOpenCount == 0)
    return CIFX_DRV_NOT_OPENED;

  CHECK_DRIVERHANDLE(hDriver);

  --g_tDriverInfo.ulOpenCount;

  return CIFX_NO_ERROR; /*lint !e438 */
}

/*****************************************************************************/
/*! Query Driver information
*   \param hDriver      Handle to the driver
*   \param ulSize       Size of the passed DRIVER_INFORMATION Structure
*   \param pvDriverInfo Pointer to returned data (DRIVER_INFORMATION
*                       structure)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xDriverGetInformation(CIFXHANDLE hDriver, uint32_t ulSize, void* pvDriverInfo)
{
  DRIVER_INFORMATION* ptDriverInfo = (DRIVER_INFORMATION*)pvDriverInfo;

  if(g_tDriverInfo.ulOpenCount == 0)
    return CIFX_DRV_NOT_OPENED;

  CHECK_DRIVERHANDLE(hDriver);
  CHECK_POINTER(pvDriverInfo);

  if(ulSize < (uint32_t)sizeof(*ptDriverInfo))
    return CIFX_INVALID_BUFFERSIZE;

  ptDriverInfo->ulBoardCnt = g_ulDeviceCount;
  (void)OS_Strncpy(ptDriverInfo->abDriverVersion, TOOLKIT_VERSION, sizeof(ptDriverInfo->abDriverVersion));

  return CIFX_NO_ERROR; /*lint !e438 */
}
#endif

/*****************************************************************************/
/*! Query human readable error description
*   \param lError       Error to look up
*   \param szBuffer     Pointer to return data
*   \param ulBufferLen  Length of return buffer
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xDriverGetErrorDescription(int32_t lError, char* szBuffer, uint32_t ulBufferLen)
{
  int32_t lRet = CIFX_FUNCTION_FAILED;
  int     iIdx = 0;

  CHECK_POINTER(szBuffer);

  for(iIdx = 0; iIdx < (int)HIL_CNT_ELEMENT(s_atErrorToDescrTable); ++iIdx)
  {
    if(s_atErrorToDescrTable[iIdx].lError == lError)
    {
      (void)OS_Strncpy(szBuffer, s_atErrorToDescrTable[iIdx].szErrorDescr, ulBufferLen);
      lRet = CIFX_NO_ERROR;
      break;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Enumerate over all handled boards/devices
*   \param hDriver      Driver handle
*   \param ulBoard      Board number (incremented from 0 up)
*   \param ulSize       Size of return buffer
*   \param pvBoardInfo  Return buffer (BOARD_INFORMATION structure)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xDriverEnumBoards(CIFXHANDLE hDriver, uint32_t ulBoard, uint32_t ulSize, void* pvBoardInfo)
{
  BOARD_INFORMATION*        ptBoardInfo   = (BOARD_INFORMATION*)pvBoardInfo;
  PDEVICEINSTANCE           ptDevInstance = NULL;
  HIL_DPM_SYSTEM_CHANNEL_T* ptSysChannel  = NULL;

  if(g_tDriverInfo.ulOpenCount == 0)
    return CIFX_DRV_NOT_OPENED;

  CHECK_DRIVERHANDLE(hDriver);
  CHECK_POINTER(pvBoardInfo);

  if(ulSize < (uint32_t)sizeof(*ptBoardInfo))
    return CIFX_INVALID_BUFFERSIZE;

  if(ulBoard >= g_ulDeviceCount)
    return CIFX_NO_MORE_ENTRIES;

  ptDevInstance = g_pptDevices[ulBoard];
  ptSysChannel  = (HIL_DPM_SYSTEM_CHANNEL_T*)ptDevInstance->pbDPM;

  (void)OS_Strncpy(ptBoardInfo->abBoardName,  ptDevInstance->szName,  sizeof(ptBoardInfo->abBoardName));
  (void)OS_Strncpy(ptBoardInfo->abBoardAlias, ptDevInstance->szAlias, sizeof(ptBoardInfo->abBoardAlias));

  /* The location of the ulSystemError is different in DPM and HIF. Use the
   * correct structure according to the layout information retrieved. */
  if (HIL_HIF_LAYOUT_NA == ptDevInstance->bDPMLayout)
    ptBoardInfo->ulSystemError   = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemError));
  else
    ptBoardInfo->ulSystemError   = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ((HIL_HIF_SYSTEM_CHANNEL_T*)ptSysChannel)->tSystemState.ulSystemError));

  ptBoardInfo->ulBoardID         = ulBoard;
  ptBoardInfo->ulPhysicalAddress = ptDevInstance->ulPhysicalAddress;
  ptBoardInfo->ulIrqNumber       = ptDevInstance->ulIrqNumber;
  ptBoardInfo->bIrqEnabled       = ptDevInstance->fIrqEnabled? 1 : 0;
  ptBoardInfo->ulDpmTotalSize    = ptDevInstance->ulDPMSize;
  ptBoardInfo->ulChannelCnt      = ptDevInstance->ulCommChannelCount;

  /* SystemInfo structure and location are identical in DPM and HIF. */
  HWIF_READN(ptDevInstance, &ptBoardInfo->tSystemInfo, &ptSysChannel->tSystemInfo, sizeof(ptBoardInfo->tSystemInfo));

  (void)cifXConvertEndianess(0,
                             &ptBoardInfo->tSystemInfo,
                             sizeof(ptBoardInfo->tSystemInfo),
                             g_atSystemInfoBlock,
                             g_ulSystemInfoBlockSize);

  return CIFX_NO_ERROR; /*lint !e438 */
}

/*****************************************************************************/
/*! Enumerate over all channels on the given boards
*   \param hDriver        Driver handle
*   \param ulBoard        Board number
*   \param ulChannel      Channel number (incremented from 0 up)
*   \param ulSize         Size of return buffer
*   \param pvChannelInfo  Return buffer (CHANNEL_INFORMATION structure)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xDriverEnumChannels(CIFXHANDLE hDriver, uint32_t ulBoard, uint32_t ulChannel, uint32_t ulSize, void* pvChannelInfo)
{
  CHANNEL_INFORMATION*  ptChannelInfo = (CHANNEL_INFORMATION*)pvChannelInfo;
  PDEVICEINSTANCE       ptDevInstance = NULL;
  PCHANNELINSTANCE      ptChannel     = NULL;
  int32_t               lRet          = CIFX_NO_ERROR;

  if(g_tDriverInfo.ulOpenCount == 0)
    return CIFX_DRV_NOT_OPENED;

  CHECK_DRIVERHANDLE(hDriver);
  CHECK_POINTER(pvChannelInfo);

  if(ulSize < (uint32_t)sizeof(*ptChannelInfo))
    return CIFX_INVALID_BUFFERSIZE;

  if(ulBoard >= g_ulDeviceCount)
    return CIFX_INVALID_BOARD;

  ptDevInstance = g_pptDevices[ulBoard];

  if(ulChannel >= ptDevInstance->ulCommChannelCount)
    return CIFX_NO_MORE_ENTRIES;

  ptChannel = ptDevInstance->pptCommChannels[ulChannel];

  /* Read the channel information */
  lRet = xChannelInfo(ptChannel, ulSize, ptChannelInfo);

  return lRet; /*lint !e438 */
}

/*! **************************************************************************
* Get/Return a memory pointer to the boards dual-port memory
*   \param hDriver      Driver handle
*   \param ulBoard      The board number (0)
*   \param ulCmd        Function command (CIFX_MEM_PTR_OPEN/CIFX_MEM_PTR_CLOSE)
*   \param pvMemoryInfo Memory information structure
*   \return CIFX_NO_ERROR on success
******************************************************************************/
int32_t APIENTRY xDriverMemoryPointer(CIFXHANDLE hDriver, uint32_t ulBoard, uint32_t ulCmd, void* pvMemoryInfo)
{
  int32_t             lRet     = CIFX_NO_ERROR;
  MEMORY_INFORMATION* ptMemory = (MEMORY_INFORMATION*)pvMemoryInfo;

  if(0 == g_tDriverInfo.ulOpenCount)
    return CIFX_DRV_NOT_OPENED;

  CHECK_DRIVERHANDLE(hDriver);
  CHECK_POINTER(pvMemoryInfo);

  /* We only support 1 board */
  if(ulBoard >= g_ulDeviceCount)
  {
    lRet = CIFX_INVALID_BOARD;
  } else
  {
    /* Get the device instance */
    PDEVICEINSTANCE ptDevInstance = g_pptDevices[ulBoard];
    switch(ulCmd)
    {
      case CIFX_MEM_PTR_OPEN:
        {
          void* pvMappedDPM = NULL;

          *(ptMemory->pulMemorySize) = 0;
          *(ptMemory->ppvMemoryPtr)  = NULL;

          /* Return global memory information */
          if(NULL == (ptMemory->pvMemoryID = OS_MapUserPointer(ptDevInstance->pbDPM, ptDevInstance->ulDPMSize, &pvMappedDPM, ptDevInstance->pvOSDependent, 0)))
          {
            lRet = CIFX_MEMORY_MAPPING_FAILED;
          } else
          {
            *(ptMemory->ppvMemoryPtr)  = (void*)pvMappedDPM;
            *(ptMemory->pulMemorySize) = ptDevInstance->ulDPMSize;
          }

          /* Check requested channel */
          if(ptMemory->ulChannel != CIFX_NO_CHANNEL)
          {
            /* Process channel information */
            if(ptMemory->ulChannel >= ptDevInstance->ulCommChannelCount)
            {
              *(ptMemory->pulChannelStartOffset) = 0;
              *(ptMemory->pulChannelSize)        = 0;
              lRet = CIFX_INVALID_CHANNEL;
            } else
            {
              PCHANNELINSTANCE ptChannel = ptDevInstance->pptCommChannels[ptMemory->ulChannel];
              uint32_t ulOffset = (uint32_t)(ptChannel->pbDPMChannelStart - ptDevInstance->pbDPM);

              /* Get Channel information */
              *(ptMemory->pulChannelSize)        = ptChannel->ulDPMChannelLength;
              *(ptMemory->pulChannelStartOffset) = ulOffset;
            }
          }
        }
        break;

      case CIFX_MEM_PTR_CLOSE:
        /* Clear user area */
        if(!OS_UnmapUserPointer(ptMemory->pvMemoryID, ptDevInstance->pvOSDependent))
        {
          lRet = CIFX_INVALID_HANDLE;
        } else
        {
          ptMemory->pvMemoryID       = NULL;
          *(ptMemory->ppvMemoryPtr)  = NULL;
          *(ptMemory->pulMemorySize) = 0;
          *(ptMemory->ppvMemoryPtr)  = NULL;
        }
        break;

      default:
        lRet = CIFX_INVALID_COMMAND;
        break;
    } /* end switch */
  }

  return lRet; /*lint !e438 */
}

#if 0 /* OS specific, implemented by user. */
int32_t APIENTRY xDriverRestartDevice(CIFXHANDLE hDriver, char* szBoardName, void* pvData)
{
  CHECK_DRIVERHANDLE(hDriver);
  CHECK_POINTER(szBoardName);
  CHECK_POINTER(pvData);

  /* Implement handling for device restart. */

  return CIFX_NO_ERROR;
}
#endif

/*****************************************************************************/
/*! Opens a channel by name (Name can be obtained when enumerating Channels)
*   \param hDriver    Driver handle
*   \param szBoard    DOS Device Name of the Board to open
*   \param ulChannel  Channel number to open (0..n)
*   \param phChannel  Returned handle to the channel (Needed for all channel
*                     specific operations)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xChannelOpen(CIFXHANDLE hDriver, char* szBoard, uint32_t ulChannel, CIFXHANDLE* phChannel)
{
  int32_t  lRet = CIFX_INVALID_BOARD;
  uint32_t ulIdx;

  if(0 == g_tDriverInfo.ulOpenCount)
    return CIFX_DRV_NOT_OPENED;

  CHECK_DRIVERHANDLE(hDriver);
  CHECK_POINTER(szBoard);
  CHECK_POINTER(phChannel);

  for(ulIdx = 0; ulIdx < g_ulDeviceCount; ++ulIdx)
  {
    /* Try to find the requested board */
    if( (OS_Strcmp(g_pptDevices[ulIdx]->szName,  szBoard) == 0) ||
        (OS_Strcmp(g_pptDevices[ulIdx]->szAlias, szBoard) == 0) )
    {
      /* Try to open the given channel */
      lRet = CIFX_INVALID_CHANNEL;
      if(ulChannel < g_pptDevices[ulIdx]->ulCommChannelCount)
      {
        /* We found the channel */
        PCHANNELINSTANCE ptChannel = g_pptDevices[ulIdx]->pptCommChannels[ulChannel];
        ++ptChannel->ulOpenCount;
        *phChannel = (CIFXHANDLE)ptChannel;
        lRet = CIFX_NO_ERROR;
      }
      break;
    }
  }

  return lRet; /*lint !e438 */
}

/*****************************************************************************/
/*! Closes a previously opened channel
*   \param hChannel Channel handle acquired by xChannelOpen
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xChannelClose(CIFXHANDLE hChannel)
{
  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  --ptChannel->ulOpenCount;

  return CIFX_NO_ERROR;
}

/*****************************************************************************/
/*! Returns the Mailbox state from a specific channel
*   \param hChannel         Channel handle acquired by xChannelOpen
*   \param pulRecvPktCount  Number of Messages waiting in receive mailbox
*   \param pulSendPktCount  State of the Send Mailbox
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xChannelGetMBXState(CIFXHANDLE hChannel, uint32_t* pulRecvPktCount, uint32_t* pulSendPktCount)
{
  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pulRecvPktCount);
  CHECK_POINTER(pulSendPktCount);

  return DEV_GetMBXState(ptChannel, pulRecvPktCount, pulSendPktCount);
}

/*****************************************************************************/
/*! Set BUS state off a communication channel
*   \param hChannel         Channel handle
*   \param ulCmd            CIFX_CONFIGURATION_XXX defines
*   \param pulState         Return actual state on CIFX_GET_BUS_STATE
*   \param ulTimeout        Timeout in [ms]
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xChannelBusState(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout)
{
  int32_t lRet = CIFX_INVALID_PARAMETER;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pulState);

  lRet = DEV_BusState(
      (PCHANNELINSTANCE)hChannel,
      ulCmd,
      pulState,
      ulTimeout);

  return lRet;
}

/*****************************************************************************/
/*! Reset a communication channel
*   \param hChannel        Channel handle
*   \param ulResetMode     Reset Mode
*   \param ulTimeout       Timeout to wait for reset complete in [ms]
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xChannelReset(CIFXHANDLE  hChannel, uint32_t ulResetMode, uint32_t ulTimeout)
{
  int32_t          lRet;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  /* TODO: Get packet and I/O mutexe before processing a channel init */
  /* TODO: System start via channel ? */
  switch(ulResetMode)
  {
  case CIFX_SYSTEMSTART:
    lRet = DEV_DoSystemStart(ptChannel, ulTimeout, 0);
    #ifdef CIFX_TOOLKIT_TIME
      if (CIFX_NO_ERROR == lRet)
        cifXInitTime((PDEVICEINSTANCE)ptChannel->pvDeviceInstance);
    #endif
    break;

  case CIFX_CHANNELINIT:
    lRet = DEV_DoChannelInit(ptChannel, ulTimeout);
    break;

  default:
    lRet = CIFX_INVALID_PARAMETER;
    break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Trigger channels watchdog
*   \param hChannel       Handle to the channel
*   \param ulCmd          Trigger command (CIFX_WATCHDOG_START to
*                         trigger/start, CIFX_WATCHDOG_STOP to end watchdog)
*   \param pulTrigger     Old trigger value from device
*                         (informational use only)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xChannelWatchdog(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t* pulTrigger)
{
  return DEV_TriggerWatchdog((PCHANNELINSTANCE)hChannel, ulCmd, pulTrigger);
}
/*****************************************************************************/
/*! Starts directory enumeration on the given channel
*   \param hChannel           Handle to the channel
*   \param ptDirectoryInfo    Pointer to enumeration result.
*                             (Will be initialized inside function)
*   \param pfnRecvPktCallback Callback for unhandled packets
*   \param pvUser             User data for callback function
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xChannelFindFirstFile(CIFXHANDLE             hChannel,
                                       CIFX_DIRECTORYENTRY*   ptDirectoryInfo,
                                       PFN_RECV_PKT_CALLBACK  pfnRecvPktCallback,
                                       void*                  pvUser)
{
  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  return xSysdeviceFindFirstFile(hChannel, ptChannel->ulChannelNumber, ptDirectoryInfo, pfnRecvPktCallback, pvUser);
}

/*****************************************************************************/
/*! Enumerate next entry in directory on the given channel
*   \param hChannel           Handle to the channel
*   \param ptDirectoryInfo    Pointer to enumeration result.
*   \param pfnRecvPktCallback Callback for unhandled packets
*   \param pvUser             User data for callback function
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xChannelFindNextFile(CIFXHANDLE            hChannel,
                                      CIFX_DIRECTORYENTRY*  ptDirectoryInfo,
                                      PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                      void*                 pvUser)
{
  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  return xSysdeviceFindNextFile(hChannel, ptChannel->ulChannelNumber, ptDirectoryInfo, pfnRecvPktCallback, pvUser);
}

/*****************************************************************************/
/*! Opens the System device on the given board
*   \param hDriver      Driver handle
*   \param szBoard      Name of the board to open
*   \param phSysdevice  Returned handle to the System device area
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xSysdeviceOpen(CIFXHANDLE hDriver, char* szBoard, CIFXHANDLE* phSysdevice)
{
  int32_t  lRet = CIFX_INVALID_BOARD;
  uint32_t ulIdx;

  if(0 == g_tDriverInfo.ulOpenCount)
    return CIFX_DRV_NOT_OPENED;

  CHECK_DRIVERHANDLE(hDriver);
  CHECK_POINTER(szBoard);
  CHECK_POINTER(phSysdevice);

  for(ulIdx = 0; ulIdx < g_ulDeviceCount; ++ulIdx)
  {
    if( (OS_Strcmp(g_pptDevices[ulIdx]->szName,  szBoard) == 0) ||
        (OS_Strcmp(g_pptDevices[ulIdx]->szAlias, szBoard) == 0) )
    {
      ++g_pptDevices[ulIdx]->tSystemDevice.ulOpenCount;
      *phSysdevice = (CIFXHANDLE)(&g_pptDevices[ulIdx]->tSystemDevice);
      lRet = CIFX_NO_ERROR;
      break;
    }
  }

  return lRet; /*lint !e438 */
}

/*****************************************************************************/
/*! Closes an open System device
*   \param hSysdevice  Handle to the System device to close
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xSysdeviceClose(CIFXHANDLE hSysdevice)
{
  PCHANNELINSTANCE ptSysDevice = (PCHANNELINSTANCE)hSysdevice;

  CHECK_SYSDEVICEHANDLE(hSysdevice);

  --ptSysDevice->ulOpenCount;

  return CIFX_NO_ERROR;
}

/*****************************************************************************/
/*! Gets the Mailbox state of an open system device
*   \param hSysdevice      Handle to the System device
*   \param pulRecvPktCount Number of packets in receive mailbox
*   \param pulSendPktCount Number of packets the application is able to send
*                          at once
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xSysdeviceGetMBXState(CIFXHANDLE hSysdevice, uint32_t* pulRecvPktCount, uint32_t* pulSendPktCount)
{
  PCHANNELINSTANCE ptSysDevice = (PCHANNELINSTANCE)hSysdevice;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(pulRecvPktCount);
  CHECK_POINTER(pulSendPktCount);

  return DEV_GetMBXState(ptSysDevice, pulRecvPktCount, pulSendPktCount);
}

/*****************************************************************************/
/*! Starts directory enumeration on the given channel
*   \param hSysdevice         Handle to the system device
*   \param ulChannel          Channel number to get directory from
*   \param ptDirectoryInfo    Pointer to enumeration result.
*                             (Will be initialized inside function)
*   \param pfnRecvPktCallback Callback for unhandled packets
*   \param pvUser             User data for callback function
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xSysdeviceFindFirstFile(CIFXHANDLE            hSysdevice,
                                         uint32_t              ulChannel,
                                         CIFX_DIRECTORYENTRY*  ptDirectoryInfo,
                                         PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                         void*                 pvUser)
{
  int32_t          lRet      = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hSysdevice;
  union
  {
    CIFX_PACKET        tPacket;
    HIL_DIR_LIST_REQ_T tDirListReq;

  }                   uSendPacket;
  HIL_DIR_LIST_CNF_T  tDirListCnf;

#ifdef CIFX_TOOLKIT_PARAMETER_CHECK
  if ( (CIFX_NO_ERROR != CheckSysdeviceHandle(hSysdevice)) &&
       (CIFX_NO_ERROR != CheckChannelHandle(hSysdevice)) )
       return CIFX_INVALID_HANDLE;
#endif

  CHECK_POINTER(ptDirectoryInfo);

  OS_Memset(&uSendPacket, 0, sizeof(uSendPacket));
  OS_Memset(&tDirListCnf, 0, sizeof(tDirListCnf));

  if(OS_Strlen(ptDirectoryInfo->szFilename) > 0)
  {
    uint16_t usDirNameLength = (uint16_t)(OS_Strlen(ptDirectoryInfo->szFilename) + 1);

    uSendPacket.tDirListReq.tData.usDirNameLength = HOST_TO_LE16(usDirNameLength);
    (void)OS_Strncpy( (char*)((&uSendPacket.tDirListReq.tData) + 1),
                       ptDirectoryInfo->szFilename,
                       usDirNameLength);
  }

  uSendPacket.tDirListReq.tHead.ulDest      = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
  uSendPacket.tDirListReq.tHead.ulSrc       = HOST_TO_LE32(((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->ulPhysicalAddress);
  uSendPacket.tDirListReq.tHead.ulCmd       = HOST_TO_LE32(HIL_DIR_LIST_REQ);
  uSendPacket.tDirListReq.tHead.ulLen       = HOST_TO_LE32( ((uint32_t)sizeof(uSendPacket.tDirListReq.tData) +
                                                            uSendPacket.tDirListReq.tData.usDirNameLength) );
  uSendPacket.tDirListReq.tData.ulChannelNo = HOST_TO_LE32(ulChannel);

  lRet = DEV_TransferPacket(
      ptChannel,
      &uSendPacket.tPacket,
      (CIFX_PACKET*)&tDirListCnf,
      sizeof(tDirListCnf),
      CIFX_TO_SEND_PACKET,
      pfnRecvPktCallback,
      pvUser);

  if( CIFX_NO_ERROR == lRet)
  {
    if( SUCCESS_HIL_OK == (lRet = LE32_TO_HOST(tDirListCnf.tHead.ulSta)) )
    {
      uint8_t* pbListEntry = (uint8_t*)&ptDirectoryInfo->hList;
      if ((tDirListCnf.tHead.ulExt & HIL_PACKET_SEQ_MASK) == HIL_PACKET_SEQ_LAST)
      {
        /* this is the last packet */
        lRet = CIFX_NO_MORE_ENTRIES;
        /* invalidate handle */
        *pbListEntry = 0;
      } else
      {
        /* TODO: Store handle for directory list, which needs to be set by firmware */
        *pbListEntry = 1;

        (void)OS_Strncpy(ptDirectoryInfo->szFilename,
                         (const char*)tDirListCnf.tData.szName,
                         sizeof(ptDirectoryInfo->szFilename));

        ptDirectoryInfo->bFiletype  = tDirListCnf.tData.bFileType;
        ptDirectoryInfo->ulFilesize = LE32_TO_HOST(tDirListCnf.tData.ulFileSize);
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Enumerate next entry in directory on the given channel
*   \param hSysdevice         Handle to the system device
*   \param ulChannel          Channel number to get directory from
*   \param ptDirectoryInfo    Pointer to enumeration result
*   \param pfnRecvPktCallback Callback for unhandled packets
*   \param pvUser             User data for callback function
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xSysdeviceFindNextFile(CIFXHANDLE            hSysdevice,
                                        uint32_t              ulChannel,
                                        CIFX_DIRECTORYENTRY*  ptDirectoryInfo,
                                        PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                        void*                 pvUser)
{
  int32_t          lRet      = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hSysdevice;
  union
  {
    CIFX_PACKET         tPacket;
    HIL_DIR_LIST_REQ_T  tDirListReq;

  }                   uSendPacket;
  HIL_DIR_LIST_CNF_T  tDirListCnf;
  uint16_t            usDirNameLen = 0;

#ifdef CIFX_TOOLKIT_PARAMETER_CHECK
  if ( (CIFX_NO_ERROR != CheckSysdeviceHandle(hSysdevice)) &&
       (CIFX_NO_ERROR != CheckChannelHandle(hSysdevice)) )
       return CIFX_INVALID_HANDLE;
#endif

  CHECK_POINTER(ptDirectoryInfo);

  OS_Memset(&uSendPacket, 0, sizeof(uSendPacket));
  OS_Memset(&tDirListCnf, 0, sizeof(tDirListCnf));

  usDirNameLen = (uint16_t)(OS_Strlen(ptDirectoryInfo->szFilename) + 1);
  uSendPacket.tDirListReq.tData.usDirNameLength = HOST_TO_LE16(usDirNameLen);
  (void)OS_Strncpy( (char*)((&uSendPacket.tDirListReq.tData) + 1),
                    ptDirectoryInfo->szFilename,
                    usDirNameLen);

  uSendPacket.tDirListReq.tHead.ulDest      = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
  uSendPacket.tDirListReq.tHead.ulSrc       = HOST_TO_LE32(((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->ulPhysicalAddress);
  uSendPacket.tDirListReq.tHead.ulCmd       = HOST_TO_LE32(HIL_DIR_LIST_REQ);
  uSendPacket.tDirListReq.tHead.ulLen       = HOST_TO_LE32( ((uint32_t)sizeof(uSendPacket.tDirListReq.tData) + usDirNameLen) );
  uSendPacket.tDirListReq.tHead.ulExt       = HOST_TO_LE32(HIL_PACKET_SEQ_MIDDLE);

  uSendPacket.tDirListReq.tData.ulChannelNo = HOST_TO_LE32(ulChannel);

  lRet = DEV_TransferPacket(
      ptChannel,
      &uSendPacket.tPacket,
      (CIFX_PACKET*)&tDirListCnf,
      sizeof(tDirListCnf),
      CIFX_TO_SEND_PACKET,
      pfnRecvPktCallback,
      pvUser);

  if(CIFX_NO_ERROR == lRet)
  {
    if( SUCCESS_HIL_OK == (lRet = (LE32_TO_HOST(tDirListCnf.tHead.ulSta))) )
    {
      if(( LE32_TO_HOST(tDirListCnf.tHead.ulExt) & HIL_PACKET_SEQ_MASK) == HIL_PACKET_SEQ_LAST)
      {
        uint8_t* pbListEntry = (uint8_t*)&ptDirectoryInfo->hList;

        /* invalidate handle */
        *pbListEntry = 0;

        /* this is the last packet */
        lRet = CIFX_NO_MORE_ENTRIES;

      } else
      {
        (void)OS_Strncpy(ptDirectoryInfo->szFilename,
                         (const char*)tDirListCnf.tData.szName,
                         sizeof(tDirListCnf.tData.szName));

        ptDirectoryInfo->bFiletype  = tDirListCnf.tData.bFileType;
        ptDirectoryInfo->ulFilesize = LE32_TO_HOST(tDirListCnf.tData.ulFileSize);
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Hard resets a complete device via system channel with reset parameter
*   \param hSysdevice Handle to system device
*   \param ulTimeout  Timeout to wait for card to finish reset
*   \param ulMode     Reset mode with parameter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xSysdeviceResetEx(CIFXHANDLE hSysdevice, uint32_t ulTimeout, uint32_t ulMode)
{
  PCHANNELINSTANCE ptSysDevice = (PCHANNELINSTANCE) hSysdevice;
  int32_t          lRet        = CIFX_NO_ERROR;

  CHECK_SYSDEVICEHANDLE(hSysdevice);

  /* Evaluate which mode is selected, only write reset parameter if reset mode is supported */
  if(CIFX_RESETEX_SYSTEMSTART == (HIL_RESET_MODE_CMD_MSK & ulMode))
  {
    lRet = DEV_DoSystemStart(ptSysDevice, ulTimeout, (HIL_RESET_PARAM_MSK|HIL_RESET_FLAG_MSK) & ulMode);
  }
  else if(CIFX_RESETEX_BOOTSTART == (HIL_RESET_MODE_CMD_MSK & ulMode))
  {
    lRet = DEV_DoSystemBootstart(ptSysDevice, ulTimeout, (HIL_RESET_PARAM_MSK|HIL_RESET_FLAG_MSK) & ulMode);
  }
  else if(CIFX_RESETEX_UPDATESTART == (HIL_RESET_MODE_CMD_MSK & ulMode))
  {
    lRet = DEV_DoUpdateStart(ptSysDevice, ulTimeout, (HIL_RESET_PARAM_MSK|HIL_RESET_FLAG_MSK) & ulMode);
  }
  else
  {
    lRet = CIFX_INVALID_PARAMETER;
  }

  #ifdef CIFX_TOOLKIT_TIME
    if (CIFX_NO_ERROR == lRet)
      cifXInitTime((PDEVICEINSTANCE)ptSysDevice->pvDeviceInstance);
  #endif

  return lRet;
}

/*****************************************************************************/
/*! Hard resets a complete device via system channel
*   \param hSysdevice Handle to system device
*   \param ulTimeout  Timeout to wait for card to finish reset
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xSysdeviceReset(CIFXHANDLE hSysdevice, uint32_t ulTimeout)
{
  PCHANNELINSTANCE ptSysDevice = (PCHANNELINSTANCE)hSysdevice;
  int32_t          lRet;

  CHECK_SYSDEVICEHANDLE(hSysdevice);

  lRet = DEV_DoSystemStart(ptSysDevice, ulTimeout, 0);

  #ifdef CIFX_TOOLKIT_TIME
    if (CIFX_NO_ERROR == lRet)
      cifXInitTime((PDEVICEINSTANCE)ptSysDevice->pvDeviceInstance);
  #endif

  return lRet;
}

/*****************************************************************************/
/*! Boot start reset to via system channel
*   \param hSysdevice Handle to system device
*   \param ulTimeout  Timeout to wait for card to finish reset
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xSysdeviceBootstart(CIFXHANDLE hSysdevice, uint32_t ulTimeout)
{
  PCHANNELINSTANCE ptSysDevice = (PCHANNELINSTANCE)hSysdevice;
  int32_t          lRet;

  CHECK_SYSDEVICEHANDLE(hSysdevice);

  lRet = DEV_DoSystemBootstart(ptSysDevice, ulTimeout, 0);

  return lRet;
}

/*****************************************************************************/
/*! Get/Return a memory pointer to an extended board memory if available
*   \param hSysdevice   Handle to system device
*   \param ulCmd        Command for get/free
*   \param ptExtMemInfo Pointer to a user buffer to return the information
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xSysdeviceExtendedMemory(CIFXHANDLE hSysdevice, uint32_t ulCmd, CIFX_EXTENDED_MEMORY_INFORMATION* ptExtMemInfo)
{
  PCHANNELINSTANCE            ptSysDevice   = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE             ptDevInstance = (PDEVICEINSTANCE)ptSysDevice->pvDeviceInstance;
  HIL_HIF_SYSTEM_CHANNEL_T*   ptSysChannel  = NULL;
  int32_t                     lRet          = CIFX_NO_ERROR;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(ptExtMemInfo);

  ptSysChannel = (HIL_HIF_SYSTEM_CHANNEL_T*)ptSysDevice->pbDPMChannelStart;

  if(0 == g_tDriverInfo.ulOpenCount)
    return CIFX_DRV_NOT_OPENED;

  switch(ulCmd)
  {
    case CIFX_GET_EXTENDED_MEMORY_POINTER:
      {
        void* pvMemoryPtr = NULL;

        if( (NULL == ptDevInstance->pbExtendedMemory)    ||
            (0    == ptDevInstance->ulExtendedMemorySize))
        {
          lRet = CIFX_MEMORY_MAPPING_FAILED;
        }else
        {
          ptExtMemInfo->pvMemoryID    = NULL;
          ptExtMemInfo->pvMemoryPtr   = NULL;
          ptExtMemInfo->ulMemorySize  = 0;
          ptExtMemInfo->ulMemoryType  = 0;

          /* Return global memory information */
          if(NULL == (ptExtMemInfo->pvMemoryID = OS_MapUserPointer(ptDevInstance->pbExtendedMemory,
                                                                   ptDevInstance->ulExtendedMemorySize,
                                                                   &pvMemoryPtr,
                                                                   ptDevInstance->pvOSDependent,
                                                                   0)))
          {
            lRet = CIFX_MEMORY_MAPPING_FAILED;
          } else
          {
            uint32_t ulHWFeatures;

            /* The location of the ulSystemError is different in DPM and HIF. Use the
             * correct structure according to the layout information retrieved. */
            if (HIL_HIF_LAYOUT_NA == ptDevInstance->bDPMLayout)
              ulHWFeatures = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulHWFeatures));
            else
              ulHWFeatures = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ((HIL_HIF_SYSTEM_CHANNEL_T*)ptSysChannel)->tSystemState.ulHWFeatures));

            ptExtMemInfo->pvMemoryPtr  = pvMemoryPtr;
            ptExtMemInfo->ulMemorySize = ptDevInstance->ulExtendedMemorySize;
            ptExtMemInfo->ulMemoryType = ulHWFeatures & (HIL_SYSTEM_EXTMEM_ACCESS_MSK | HIL_SYSTEM_EXTMEM_TYPE_MSK);

            if( HIL_SYSTEM_EXTMEM_ACCESS_BOTH == (ptExtMemInfo->ulMemoryType & HIL_SYSTEM_EXTMEM_ACCESS_MSK))
              ptExtMemInfo->ulMemorySize = ptDevInstance->ulExtendedMemorySize  / 2;
            else if( HIL_SYSTEM_EXTMEM_ACCESS_INTERNAL == (ptExtMemInfo->ulMemoryType & HIL_SYSTEM_EXTMEM_ACCESS_MSK))
              ptExtMemInfo->ulMemorySize = 0;
          }
        }
      }
      break;

    case CIFX_FREE_EXTENDED_MEMORY_POINTER:
      {
        /* Clear user area */
        if(!OS_UnmapUserPointer(ptExtMemInfo->pvMemoryID, ptDevInstance->pvOSDependent))
        {
          lRet = CIFX_INVALID_HANDLE;
        } else
        {
          ptExtMemInfo->pvMemoryID    = NULL;
          ptExtMemInfo->pvMemoryPtr   = NULL;
          ptExtMemInfo->ulMemorySize  = 0;
          ptExtMemInfo->ulMemoryType  = 0;
        }
      }
      break;

    case CIFX_GET_EXTENDED_MEMORY_INFO:
      {
        uint32_t ulHWFeatures;

        /* The location of the ulSystemError is different in DPM and HIF. Use the
          * correct structure according to the layout information retrieved. */
        if (HIL_HIF_LAYOUT_NA == ptDevInstance->bDPMLayout)
          ulHWFeatures = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulHWFeatures));
        else
          ulHWFeatures = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ((HIL_HIF_SYSTEM_CHANNEL_T*)ptSysChannel)->tSystemState.ulHWFeatures));

        ptExtMemInfo->pvMemoryID    = NULL;
        ptExtMemInfo->pvMemoryPtr   = NULL;
        ptExtMemInfo->ulMemorySize  = ptDevInstance->ulExtendedMemorySize;
        ptExtMemInfo->ulMemoryType  = ulHWFeatures & (HIL_SYSTEM_EXTMEM_ACCESS_MSK | HIL_SYSTEM_EXTMEM_TYPE_MSK);

        if( HIL_SYSTEM_EXTMEM_ACCESS_BOTH == (ptExtMemInfo->ulMemoryType & HIL_SYSTEM_EXTMEM_ACCESS_MSK))
          ptExtMemInfo->ulMemorySize = ptDevInstance->ulExtendedMemorySize  / 2;
        else if( HIL_SYSTEM_EXTMEM_ACCESS_INTERNAL == (ptExtMemInfo->ulMemoryType & HIL_SYSTEM_EXTMEM_ACCESS_MSK))
          ptExtMemInfo->ulMemorySize = 0;
      }
      break;

    default:
      lRet = CIFX_INVALID_COMMAND;
      break;
  } /* end switch */

  return lRet;
}
