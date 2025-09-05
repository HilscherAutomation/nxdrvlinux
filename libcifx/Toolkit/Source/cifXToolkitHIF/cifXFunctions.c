/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXFunctions.c 15171 2025-08-05 08:18:45Z AMinor $:

  Description:
    cifX API function implementation

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-05-26  Update to new DPMv2 handling
    2023-04-26  - Added new compiler option CIFX_TOOLKIT_USE_CUSTOM_DRV_FUNCS
                - Moved check parameter macros to cifXtoolkit.h
    2022-06-14  - Added option and handling for cached PLC memory pointers
                - Reorganized xChannelPLCMemoryPtr() to handle caching option for IO area 0
    2022-01-18  Fixed xSysdeviceResetEx() to allow additional flags passing to underlying functions
    2021-09-13  Propagate changes of cifXErrors.h (spelling of name
                CIFX_DEV_DMA_HANDSHAKEMODE_NOT_SUPPORTED)
    2020-02-06  xDriverEnumBoards() should return CIFX_NO_MORE_ENTRIES
                if ulBoard exceeds actual board count, instead of invalid board
    2019-10-11  Use internal buffer for endian conversion in xChannelControlBlock()
                Propagate prototype changes of endian conversion function
    2018-11-06  Added new function xSysdeviceResetEx()
    2018-10-10  - Updated header and definitions to new Hilscher defines
                - Derived from cifX Toolkit V1.6.0.0

**************************************************************************************/

/*****************************************************************************/
/*! \file cifXFunctions.c
*   cifX API function implementation                                         */
/*****************************************************************************/

#include "cifXToolkit.h"
#include "cifXErrors.h"
#include "cifXEndianess.h"
#include "cifXFunctionList.h"
#include "USER_Dependent.h"

#include "Hil_Results.h"
#include "Hil_Packet.h"
#include "Hil_ApplicationCmd.h"
#include "Hil_SystemCmd.h"

/* Commonly used function, not exposed to the user interface */
#ifdef CIFX_TOOLKIT_TIME
extern void cifXInitTime(PDEVICEINSTANCE ptDevInstance);
#endif

/*****************************************************************************/
/*!  \addtogroup CIFX_DRIVER_API cifX Driver API implementation
*    \{                                                                      */
/*****************************************************************************/

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
  {CIFX_DEV_COM_MODE_UNKNOWN         ,"Unknown I/O exchange mode "                       },
  {CIFX_DEV_FUNCTION_FAILED          ,"Device function failed "                          },
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
/*! Structure description of NETX_SYSTEM_CHANNEL_INFO                        */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atSystemChannelInfo[] =
{
  /* Offset, Width, Elements */
  { 0x04, eCIFX_ENDIANESS_WIDTH_32BIT, 1}, /* ulSizeOfChannel                */
  { 0x08, eCIFX_ENDIANESS_WIDTH_16BIT, 2}, /* usSizeOfMailbox
                                              usMailboxStartOffset           */
};

/*****************************************************************************/
/*! Structure description of NETX_HANDSHAKE_CHANNEL_INFO                     */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atHandshakeChannelInfo[] =
{
  /* Offset, Width, Elements */
  { 0x04, eCIFX_ENDIANESS_WIDTH_32BIT, 1}, /* ulSizeOfChannel                */
};

/*****************************************************************************/
/*! Structure description of NETX_COMMUNICATION_CHANNEL_INFO                 */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atCommChannelInfo[] =
{
  /* Offset, Width, Elements */
  { 0x04, eCIFX_ENDIANESS_WIDTH_32BIT, 1}, /* ulSizeOfChannel                */
  { 0x08, eCIFX_ENDIANESS_WIDTH_16BIT, 3}, /* usCommunicationClass
                                              usProtocolClass
                                              usProtocolConformanceClass     */
};

/*****************************************************************************/
/*! Structure description of NETX_APPLICATION_CHANNEL_INFO                   */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atAppChannelInfo[] =
{
  /* Offset, Width, Elements */
  { 0x04, eCIFX_ENDIANESS_WIDTH_32BIT, 1}, /* ulSizeOfChannel                */
};

/*****************************************************************************/
/*! Structure description of NETX_SYSTEM_INFO_BLOCK                          */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atSystemInfoBlock[] =
{
  /* Offset, Width, Elements */
  { 0x04, eCIFX_ENDIANESS_WIDTH_32BIT, 3}, /* DpmTotalSize, DevNr, SerNr     */
  { 0x10, eCIFX_ENDIANESS_WIDTH_16BIT, 6}, /* ausHwOptions, usMfg, usProdDat */
  { 0x1C, eCIFX_ENDIANESS_WIDTH_32BIT, 2}, /* ulLicenseFlags1/2              */
  { 0x24, eCIFX_ENDIANESS_WIDTH_16BIT, 3}, /* LicenseId/flags, DeviceClass   */
  { 0x2C, eCIFX_ENDIANESS_WIDTH_16BIT, 2}, /* ausReserved                    */
};

/*****************************************************************************/
/*! Structure description of NETX_SYSTEM_CONTROL_BLOCK                       */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atSystemControlBlock[] =
{
  /* Offset, Width, Elements */
  { 0x00, eCIFX_ENDIANESS_WIDTH_32BIT, 2}, /* ulSystemCmdCOS, ulReserved     */
};

/*****************************************************************************/
/*! Structure description of NETX_SYSTEM_STATUS_BLOCK                        */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atSystemStatusBlock[] =
{
  /* Offset, Width, Elements */
  { 0x00, eCIFX_ENDIANESS_WIDTH_32BIT, 5}, /* Status/HWFeatures/Error/Error/Time */
  { 0x14, eCIFX_ENDIANESS_WIDTH_16BIT, 1}, /* usCpuLoad                          */
};

/*****************************************************************************/
/*! Structure description of NETX_CONTROL_BLOCK                              */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atControlBlock[] =
{
  /* Offset, Width, Elements */
  { 0x00, eCIFX_ENDIANESS_WIDTH_32BIT, 2}, /* ulApplCos, ulWatchdog          */
};

/*****************************************************************************/
/*! Structure description of NETX_COMMON_STATUS_BLOCK                        */
/*****************************************************************************/
static const CIFX_ENDIANESS_ENTRY_T s_atCommonStatusBlock[] =
{
  /* Offset, Width, Elements */
  { 0x00, eCIFX_ENDIANESS_WIDTH_32BIT, 2}, /* ulCommState,ulCommError        */
};

extern uint32_t                g_ulDeviceCount; /*!< Number of available device (Array size of g_pptDevices) */
extern PDEVICEINSTANCE*        g_pptDevices;    /*!< Array containing all handled device instances           */
extern TKIT_DRIVER_INFORMATION g_tDriverInfo;   /*!< Global driver information                               */

#ifdef CIFX_TOOLKIT_PARAMETER_CHECK
#define CHECK_POINTER(param) if ((void*)NULL == param) return CIFX_INVALID_POINTER;
#define CHECK_DRIVERHANDLE(handle) if (&g_tDriverInfo != handle) return CIFX_INVALID_HANDLE;
#define CHECK_SYSDEVICEHANDLE(handle) if (CIFX_NO_ERROR != CheckSysdeviceHandle(handle)) return CIFX_INVALID_HANDLE;
#define CHECK_CHANNELHANDLE(handle) if (CIFX_NO_ERROR != CheckChannelHandle(handle)) return CIFX_INVALID_HANDLE;

/*****************************************************************************/
/*! Checks if the given sysdevice handle is valid
*   \param hChannel      Sysdevice handle
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t CheckSysdeviceHandle(CIFXHANDLE hChannel)
{
  int32_t  lRet  = CIFX_INVALID_HANDLE;

  if ( NULL != hChannel)
  {
    PCHANNELINSTANCE ptSysDevice = (PCHANNELINSTANCE)hChannel;
    if ( 1 == ptSysDevice->fIsSysDevice)
    {
      if( 0 == ptSysDevice->ulOpenCount)
      {
        /* We are probably in the initialization phase without an opened device handle */
        lRet = CIFX_NO_ERROR;

      }else if ( (0    == g_ulDeviceCount) ||
                 (NULL == g_pptDevices)    )
      {
        /* We can't search for the handle in the device list, because the list is not available. */
        /* Maybe we are in the initialization phase and the list is not created yet. */
        lRet = CIFX_NO_ERROR;

      }else
      {
        /* Try to find the ptSysDevice pointer in one of the device instances */
        uint32_t ulDev = 0;

        for(ulDev = 0; ulDev < g_ulDeviceCount; ++ulDev)
        {
          if (ptSysDevice == &g_pptDevices[ulDev]->tSystemDevice)
          {
            lRet = CIFX_NO_ERROR;
            break;
          }
        }
      }
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Checks if the given channel handle is valid
*   \param hChannel      Channel handle
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t CheckChannelHandle(CIFXHANDLE hChannel)
{
  int32_t  lRet  = CIFX_INVALID_HANDLE;

  if ( NULL != hChannel)
  {
    PCHANNELINSTANCE ptDevice = (PCHANNELINSTANCE)hChannel;
    if ( 0 == ptDevice->fIsSysDevice)
    {
      /* Check if we can find our device in the device table */
      if ( 0 == g_ulDeviceCount)
      {
        /* We are in the initialization phase without an device entry in the g_pptDevices table */
        lRet = CIFX_NO_ERROR;

      }else if ( (0    == g_ulDeviceCount) ||
                 (NULL == g_pptDevices)    )
      {
        /* We can't search for the handle in the device list, because the list is not available. */
        /* Maybe we are in the initialization phase and the list is not created yet. */
        lRet = CIFX_NO_ERROR;

      }else
      {
        /* Try to find the ptSysDevice pointer in one of the device instances */
        uint32_t ulDev = 0;

        /* Check if we can find our channel inside a device */
        for(ulDev = 0; ulDev < g_ulDeviceCount; ++ulDev)
        {
          uint32_t ulChannel = 0;
          for( ulChannel = 0; ulChannel < g_pptDevices[ulDev]->ulCommChannelCount; ++ulChannel)
          {
            if ( ptDevice == g_pptDevices[ulDev]->pptCommChannels[ulChannel])
            {
              lRet = CIFX_NO_ERROR;
              break;
            }
          }
        }
      }
    }
  }

  return lRet;
}
#else
#define CHECK_POINTER(param)
#define CHECK_DRIVERHANDLE(handle) UNREFERENCED_PARAMETER(handle)
#define CHECK_SYSDEVICEHANDLE(handle)
#define CHECK_CHANNELHANDLE(handle)
#endif

/*****************************************************************************/
/*! Opens the System device on the given board
*   \param hDriver      Driver handle
*   \param szBoard      Name of the board to open
*   \param phSysdevice  Returned handle to the System device area
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxSysdeviceOpen(CIFXHANDLE hDriver, char* szBoard, CIFXHANDLE* phSysdevice)
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
CIFX_STATIC int32_t APIENTRY HIFxSysdeviceClose(CIFXHANDLE hSysdevice)
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
CIFX_STATIC int32_t APIENTRY HIFxSysdeviceGetMBXState(CIFXHANDLE hSysdevice, uint32_t* pulRecvPktCount, uint32_t* pulSendPktCount)
{
  PCHANNELINSTANCE ptSysDevice   = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptSysDevice->pvDeviceInstance;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(pulRecvPktCount);
  CHECK_POINTER(pulSendPktCount);

  return CIFX_MAKE_DEV_FUN(DEV_GetMBXState)(ptSysDevice, pulRecvPktCount, pulSendPktCount);
}

/*****************************************************************************/
/*! Inserts a packet into the System Mailbox
*   \param hSysdevice      Handle to the System device
*   \param ptSendPkt       Packet to send to device
*   \param ulTimeout       maximum time to wait for packet to be accepted
*                          by device (in ms)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxSysdevicePutPacket(CIFXHANDLE hSysdevice, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptSysDevice   = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptSysDevice->pvDeviceInstance;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(ptSendPkt);

  if( !OS_WaitMutex( ptSysDevice->tFromHostMbx.tSys.pvMutex, ulTimeout))
    return CIFX_DRV_CMD_ACTIVE;

  lRet = CIFX_MAKE_DEV_FUN(DEV_PutPacket)(ptSysDevice, ptSendPkt, ulTimeout);

  OS_ReleaseMutex( ptSysDevice->tFromHostMbx.tSys.pvMutex);

  return lRet;
}

/*****************************************************************************/
/*! Retrieves a packet from the System Mailbox
*   \param hSysdevice      Handle to the System device
*   \param ulSize          Size of the buffer to retrieve the packet
*   \param ptRecvPkt       Pointer to buffer for received packet
*   \param ulTimeout       maximum time to wait for packet to be delivered
*                          by device (in ms)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxSysdeviceGetPacket(CIFXHANDLE hSysdevice, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptSysDevice   = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptSysDevice->pvDeviceInstance;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(ptRecvPkt);

  if( !OS_WaitMutex( ptSysDevice->tToHostMbx.tSys.pvMutex, ulTimeout))
    return CIFX_DRV_CMD_ACTIVE;

  lRet = CIFX_MAKE_DEV_FUN(DEV_GetPacket)(ptSysDevice, ptRecvPkt, ulSize, ulTimeout);

  OS_ReleaseMutex( ptSysDevice->tToHostMbx.tSys.pvMutex);

  return lRet;
}

/*****************************************************************************/
/*! Download a file (Firmware, Configuration, etc) to the device
*   \param hSysdevice         Handle to the system device
*   \param ulChannel          Channel number to load the file to
*   \param ulMode             Download mode (DOWNLOAD_MODE_FIRMWARE, etc)
*   \param pszFileName        Name of the file
*   \param pabFileData        Pointer to the file data
*   \param ulFileSize         Length of the file data
*   \param pfnCallback        Callback for progress indication
*                             (NULL for no callback)
*   \param pfnRecvPktCallback Callback Callback pointer for unsolicited receive packets
*                             (NULL for no callback)
*   \param pvUser             User parameter passed to callback
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxSysdeviceDownload( CIFXHANDLE            hSysdevice,
                                                 uint32_t              ulChannel,
                                                 uint32_t              ulMode,
                                                 char*                 pszFileName,
                                                 uint8_t*              pabFileData,
                                                 uint32_t              ulFileSize,
                                                 PFN_PROGRESS_CALLBACK pfnCallback,
                                                 PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                                 void*                 pvUser)
{
  PCHANNELINSTANCE ptSysDevice   = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptSysDevice->pvDeviceInstance;
  int32_t lRet                   = CIFX_NO_ERROR;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(pszFileName);
  CHECK_POINTER(pabFileData);

  switch(ulMode)
  {
    case DOWNLOAD_MODE_FIRMWARE:
    case DOWNLOAD_MODE_CONFIG:
    case DOWNLOAD_MODE_FILE:
        lRet = CIFX_MAKE_DEV_FUN(DEV_DownloadFile)(
            hSysdevice,
            ulChannel,
            ptSysDevice->tFromHostMbx.tSys.ulElementSize,
            HIL_FILE_XFER_FILE,
            pszFileName,
            ulFileSize,
            pabFileData,
            CIFX_MAKE_DEV_FUN(DEV_TransferPacket),
            pfnCallback,
            pfnRecvPktCallback,
            pvUser);
      break;

    default:
      lRet = CIFX_INVALID_PARAMETER;
      break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Uploads a file via system channel
*   \param hSysdevice         Handle to the System device
*   \param ulChannel          Channel number to get directory from
*   \param ulMode             Transfer Mode
*   \param pszFileName        Filename to upload
*   \param pabFileData        Pointer to buffer receiving upload
*   \param pulFileSize        [in]Length of buffer, [out] Bytes copied to buffer
*   \param pfnCallback        Callback pointer for progress
*                             (NULL for no callback)
*   \param pfnRecvPktCallback Callback pointer for unsolicited receive packets
*                             (NULL for no callback)
*   \param pvUser             User parameter on callback.
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxSysdeviceUpload(CIFXHANDLE            hSysdevice,
                                              uint32_t              ulChannel,
                                              uint32_t              ulMode,
                                              char*                 pszFileName,
                                              uint8_t*              pabFileData,
                                              uint32_t*             pulFileSize,
                                              PFN_PROGRESS_CALLBACK pfnCallback,
                                              PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                              void*                 pvUser)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  int32_t          lRet          = CIFX_NO_ERROR;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(pszFileName);
  CHECK_POINTER(pabFileData);
  CHECK_POINTER(pulFileSize);

  switch(ulMode)
  {
    case DOWNLOAD_MODE_FIRMWARE:
    case DOWNLOAD_MODE_CONFIG:
    case DOWNLOAD_MODE_FILE:
      lRet = CIFX_MAKE_DEV_FUN(DEV_UploadFile)(
          hSysdevice,
          ulChannel,
          ptChannel->tToHostMbx.tSys.ulElementSize,
          HIL_FILE_XFER_FILE,
          pszFileName,
          pulFileSize,
          pabFileData,
          CIFX_MAKE_DEV_FUN(DEV_TransferPacket),
          pfnCallback,
          pfnRecvPktCallback,
          pvUser);
      break;

    default:
      lRet = CIFX_INVALID_PARAMETER;
      break;
  }

  return lRet;
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
CIFX_STATIC int32_t APIENTRY HIFxSysdeviceFindFirstFile(CIFXHANDLE            hSysdevice,
                                                     uint32_t              ulChannel,
                                                     CIFX_DIRECTORYENTRY*  ptDirectoryInfo,
                                                     PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                                     void*                 pvUser)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  union
  {
    CIFX_PACKET        tPacket;
    HIL_DIR_LIST_REQ_T tDirListReq;

  }                   uSendPacket;
  HIL_DIR_LIST_CNF_T  tDirListCnf;

  OS_Memset(&uSendPacket, 0, sizeof(uSendPacket));
  OS_Memset(&tDirListCnf, 0, sizeof(tDirListCnf));

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(ptDirectoryInfo);

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

  lRet = CIFX_MAKE_DEV_FUN(DEV_TransferPacket)(
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
CIFX_STATIC int32_t APIENTRY HIFxSysdeviceFindNextFile(CIFXHANDLE            hSysdevice,
                                                    uint32_t              ulChannel,
                                                    CIFX_DIRECTORYENTRY*  ptDirectoryInfo,
                                                    PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                                    void*                 pvUser)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  union
  {
    CIFX_PACKET         tPacket;
    HIL_DIR_LIST_REQ_T  tDirListReq;

  }                   uSendPacket;
  HIL_DIR_LIST_CNF_T  tDirListCnf;
  uint16_t            usDirNameLen = 0;

  OS_Memset(&uSendPacket, 0, sizeof(uSendPacket));
  OS_Memset(&tDirListCnf, 0, sizeof(tDirListCnf));

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(ptDirectoryInfo);

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

  lRet = CIFX_MAKE_DEV_FUN(DEV_TransferPacket)(
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
/*! Gets the information of a system device
*   \param hSysdevice   Handle to the system device
*   \param ulCmd        Information to fetch (see defines CIFX_INFO_CMD_SYSTEM_XXX)
*   \param ulSize       Size of the passed structure
*   \param pvInfo       Pointer to the structure for returned data
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxSysdeviceInfo(CIFXHANDLE hSysdevice, uint32_t ulCmd, uint32_t ulSize, void* pvInfo)
{
  int32_t                     lRet         = CIFX_NO_ERROR;
  PCHANNELINSTANCE            ptSysDevice  = (PCHANNELINSTANCE)hSysdevice;
  HIL_HIF_SYSTEM_CHANNEL_T* ptSysChannel = NULL;

  CHECK_SYSDEVICEHANDLE(hSysdevice);
  CHECK_POINTER(pvInfo);

  ptSysChannel = (HIL_HIF_SYSTEM_CHANNEL_T*)ptSysDevice->pbDPMChannelStart;

  switch(ulCmd)
  {
    case CIFX_INFO_CMD_SYSTEM_INFORMATION:
      if( ulSize < (uint32_t)sizeof(SYSTEM_CHANNEL_SYSTEM_INFORMATION))
      {
        lRet = CIFX_INVALID_BUFFERSIZE;
      } else
      {
        /* Insert global system channel information */
        SYSTEM_CHANNEL_SYSTEM_INFORMATION* ptInfo = (SYSTEM_CHANNEL_SYSTEM_INFORMATION*)pvInfo;

        /* These values are directly read from DPM, so they need to be converted to host endianess */
        ptInfo->ulSystemError   = LE32_TO_HOST(HWIF_READ32(ptSysDevice->pvDeviceInstance, ptSysChannel->tSystemState.ulSystemError));
        ptInfo->ulDpmTotalSize  = LE32_TO_HOST(HWIF_READ32(ptSysDevice->pvDeviceInstance, ptSysChannel->tSystemInfo.ulDpmTotalSize));
        ptInfo->ulDeviceNumber  = LE32_TO_HOST(HWIF_READ32(ptSysDevice->pvDeviceInstance, ptSysChannel->tSystemInfo.ulDeviceNumber));
        ptInfo->ulSerialNumber  = LE32_TO_HOST(HWIF_READ32(ptSysDevice->pvDeviceInstance, ptSysChannel->tSystemInfo.ulSerialNumber));

        ptInfo->ulMBXSize       = ptSysDevice->tToHostMbx.tSys.ulElementSize;
        ptInfo->ulOpenCnt       = ptSysDevice->ulOpenCount;
      }
    break;

    case CIFX_INFO_CMD_SYSTEM_INFO_BLOCK:
      if( ulSize < (uint32_t)sizeof(SYSTEM_CHANNEL_SYSTEM_INFO_BLOCK))
      {
        lRet = CIFX_INVALID_BUFFERSIZE;
      } else
      {
        uint32_t ulCopyLen = HIL_MIN( ulSize,
                                      (uint32_t)sizeof(SYSTEM_CHANNEL_SYSTEM_INFO_BLOCK));

        HWIF_READN(ptSysDevice->pvDeviceInstance, pvInfo, &ptSysChannel->tSystemInfo, ulCopyLen);

        (void)cifXConvertEndianess(0,
                                   pvInfo,
                                   ulCopyLen,
                                   s_atSystemInfoBlock,
                                   HIL_CNT_ELEMENT(s_atSystemInfoBlock));
      }
    break;
#if 0 // TODO
    case CIFX_INFO_CMD_SYSTEM_CHANNEL_BLOCK:
      if( ulSize < (uint32_t)sizeof(SYSTEM_CHANNEL_CHANNEL_INFO_BLOCK))
      {
        lRet = CIFX_INVALID_BUFFERSIZE;
      } else
      {
        SYSTEM_CHANNEL_CHANNEL_INFO_BLOCK* ptInfoBuffer = (SYSTEM_CHANNEL_CHANNEL_INFO_BLOCK*) pvInfo; /* use the read buffer for data conversion */

        uint32_t ulCopyLen = HIL_MIN( ulSize,
                                      (uint32_t)sizeof(SYSTEM_CHANNEL_CHANNEL_INFO_BLOCK));
        int iChannel;

        HWIF_READN(ptSysDevice->pvDeviceInstance, pvInfo, &ptSysChannel->atChannelInfo[0], ulCopyLen);

        /* Convert channel information structure. This depends on the first byte (bChannelType),
           so we need to parse the whole array */
        for(iChannel = 0;
            iChannel < (int) HIL_CNT_ELEMENT(ptSysChannel->atChannelInfo);
            ++iChannel)
        {
          uint32_t ulBlockLength = (uint32_t)sizeof(ptSysChannel->atChannelInfo[0]);
          uint32_t ulOffset      = (uint32_t)(iChannel * ulBlockLength);

          if( ulOffset > ulCopyLen)
            break;

          if( (ulOffset + sizeof(ptSysChannel->atChannelInfo[0])) > ulCopyLen)
          {
            /* part of block copied, so calculate rest length */
            ulBlockLength = ulCopyLen - ulOffset;
          }

          /* Convert endianess */
          switch(ptInfoBuffer->abInfoBlock[iChannel][0])
          {
          case HIL_CHANNEL_TYPE_SYSTEM:
            (void)cifXConvertEndianess(0,
                                       &ptInfoBuffer->abInfoBlock[iChannel], /*lint !e545 */
                                       ulBlockLength,
                                       s_atSystemChannelInfo,
                                       HIL_CNT_ELEMENT(s_atSystemChannelInfo));
            break;

          case HIL_CHANNEL_TYPE_HANDSHAKE:
            (void)cifXConvertEndianess(0,
                                       &ptInfoBuffer->abInfoBlock[iChannel], /*lint !e545 */
                                       ulBlockLength,
                                       s_atHandshakeChannelInfo,
                                       HIL_CNT_ELEMENT(s_atHandshakeChannelInfo));
            break;

          case HIL_CHANNEL_TYPE_COMMUNICATION:
            (void)cifXConvertEndianess(0,
                                       &ptInfoBuffer->abInfoBlock[iChannel], /*lint !e545 */
                                       ulBlockLength,
                                       s_atCommChannelInfo,
                                       HIL_CNT_ELEMENT(s_atCommChannelInfo));
            break;

          case HIL_CHANNEL_TYPE_APPLICATION:
            (void)cifXConvertEndianess(0,
                                       &ptInfoBuffer->abInfoBlock[iChannel], /*lint !e545 */
                                       ulBlockLength,
                                       s_atAppChannelInfo,
                                       HIL_CNT_ELEMENT(s_atAppChannelInfo));
            break;

          default:
            /* This should never happen */
            break;
          }
        }
      }
    break;
#endif
    case CIFX_INFO_CMD_SYSTEM_CONTROL_BLOCK:
      lRet = CIFX_INVALID_COMMAND;
    break;

    case CIFX_INFO_CMD_SYSTEM_STATUS_BLOCK:
      if( ulSize < (uint32_t)sizeof(SYSTEM_CHANNEL_SYSTEM_STATUS_BLOCK))
      {
        lRet = CIFX_INVALID_BUFFERSIZE;
      } else
      {
        SYSTEM_CHANNEL_SYSTEM_STATUS_BLOCK* ptInfo = (SYSTEM_CHANNEL_SYSTEM_STATUS_BLOCK*) pvInfo; /* 64byte */
        HIL_HIF_SYSTEM_STATUS_BLOCK_T tSystemStatus = {0}; /* 48byte */

        uint32_t ulCopyLen = sizeof(tSystemStatus);

        HWIF_READN(ptSysDevice->pvDeviceInstance, &tSystemStatus, &ptSysChannel->tSystemState, ulCopyLen);

        (void)cifXConvertEndianess(0,
                                   &tSystemStatus,
                                   ulCopyLen,
                                   s_atSystemStatusBlock,
                                   HIL_CNT_ELEMENT(s_atSystemStatusBlock));

        /* For compatibility between DPMv2 and the cifX API
         * some fields have to be moved to different positions. */
        OS_Memset(ptInfo, 0, sizeof(*ptInfo));
        ptInfo->ulSystemCOS       = 0;
        ptInfo->ulSystemStatus    = tSystemStatus.ulSystemStatus;
        ptInfo->ulSystemError     = tSystemStatus.ulSystemError;
        ptInfo->ulBootError       = tSystemStatus.ulBootError;
        ptInfo->ulTimeSinceStart  = tSystemStatus.ulTimeSinceStart;
        ptInfo->usCpuLoad         = tSystemStatus.usCpuLoad;
        ptInfo->ulHWFeatures      = tSystemStatus.ulHWFeatures;
      }
    break;

    default:
      lRet = CIFX_INVALID_COMMAND;
    break;

  } /* end switch */

  return lRet;
}

/*****************************************************************************/
/*! Hard resets a complete device via system channel with reset parameter
*   \param hSysdevice Handle to system device
*   \param ulTimeout  Timeout to wait for card to finish reset
*   \param ulMode     Reset mode with parameter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxSysdeviceResetEx(CIFXHANDLE hSysdevice, uint32_t ulTimeout, uint32_t ulMode)
{
  PCHANNELINSTANCE ptSysDevice   = (PCHANNELINSTANCE) hSysdevice;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptSysDevice->pvDeviceInstance;
  int32_t          lRet          = CIFX_NO_ERROR;

  CHECK_SYSDEVICEHANDLE(hSysdevice);

  /* Evaluate which mode is selected, only write reset parameter if reset mode is supported */
  if(CIFX_RESETEX_SYSTEMSTART == (HIL_RESET_MODE_CMD_MSK & ulMode))
  {
    lRet = CIFX_MAKE_DEV_FUN(DEV_DoSystemStart)(ptSysDevice, ulTimeout, (HIL_RESET_PARAM_MSK|HIL_RESET_FLAG_MSK) & ulMode);
  }
  else if(CIFX_RESETEX_BOOTSTART == (HIL_RESET_MODE_CMD_MSK & ulMode))
  {
    lRet = CIFX_MAKE_DEV_FUN(DEV_DoSystemBootstart)(ptSysDevice, ulTimeout, (HIL_RESET_PARAM_MSK|HIL_RESET_FLAG_MSK) & ulMode);
  }
  else if(CIFX_RESETEX_UPDATESTART == (HIL_RESET_MODE_CMD_MSK & ulMode))
  {
    lRet = CIFX_MAKE_DEV_FUN(DEV_DoUpdateStart)(ptSysDevice, ulTimeout, (HIL_RESET_PARAM_MSK|HIL_RESET_FLAG_MSK) & ulMode);
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
CIFX_STATIC int32_t APIENTRY HIFxSysdeviceReset(CIFXHANDLE hSysdevice, uint32_t ulTimeout)
{
  int32_t          lRet;
  PCHANNELINSTANCE ptSysDevice   = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptSysDevice->pvDeviceInstance;

  CHECK_SYSDEVICEHANDLE(hSysdevice);

  lRet = CIFX_MAKE_DEV_FUN(DEV_DoSystemStart)(ptSysDevice, ulTimeout, 0);

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
CIFX_STATIC int32_t APIENTRY HIFxSysdeviceBootstart(CIFXHANDLE hSysdevice, uint32_t ulTimeout)
{
  int32_t          lRet;
  PCHANNELINSTANCE ptSysDevice   = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptSysDevice->pvDeviceInstance;

  CHECK_SYSDEVICEHANDLE(hSysdevice);

  lRet = CIFX_MAKE_DEV_FUN(DEV_DoSystemBootstart)(ptSysDevice, ulTimeout, 0);

  return lRet;
}

/*****************************************************************************/
/*! Get/Return a memory pointer to an extended board memory if available
*   \param hSysdevice   Handle to system device
*   \param ulCmd        Command for get/free
*   \param ptExtMemInfo Pointer to a user buffer to return the information
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxSysdeviceExtendedMemory(CIFXHANDLE hSysdevice, uint32_t ulCmd, CIFX_EXTENDED_MEMORY_INFORMATION* ptExtMemInfo)
{
  int32_t                     lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE            ptSysDevice   = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE             ptDevInstance = (PDEVICEINSTANCE)ptSysDevice->pvDeviceInstance;
  HIL_HIF_SYSTEM_CHANNEL_T*   ptSysChannel  = NULL;

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
            ptExtMemInfo->pvMemoryPtr  = pvMemoryPtr;
            ptExtMemInfo->ulMemorySize = ptDevInstance->ulExtendedMemorySize;
            ptExtMemInfo->ulMemoryType = LE32_TO_HOST(HWIF_READ32( ptDevInstance, ptSysChannel->tSystemState.ulHWFeatures)) & (HIL_SYSTEM_EXTMEM_ACCESS_MSK | HIL_SYSTEM_EXTMEM_TYPE_MSK);

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
        ptExtMemInfo->pvMemoryID    = NULL;
        ptExtMemInfo->pvMemoryPtr   = NULL;
        ptExtMemInfo->ulMemorySize  = ptDevInstance->ulExtendedMemorySize;
        ptExtMemInfo->ulMemoryType  = LE32_TO_HOST(HWIF_READ32( ptDevInstance, ptSysChannel->tSystemState.ulHWFeatures)) & (HIL_SYSTEM_EXTMEM_ACCESS_MSK | HIL_SYSTEM_EXTMEM_TYPE_MSK);

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

/*****************************************************************************/
/*! Opens a channel by name (Name can be obtained when enumerating Channels)
*   \param hDriver    Driver handle
*   \param szBoard    DOS Device Name of the Board to open
*   \param ulChannel  Channel number to open (0..n)
*   \param phChannel  Returned handle to the channel (Needed for all channel
*                     specific operations)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelOpen(CIFXHANDLE hDriver, char* szBoard, uint32_t ulChannel, CIFXHANDLE* phChannel)
{
  int32_t  lRet = CIFX_INVALID_BOARD;
  uint32_t ulIdx;

  if(0 == g_tDriverInfo.ulOpenCount)
    return CIFX_DRV_NOT_OPENED;

  if(ulChannel != 0)
    return CIFX_INVALID_CHANNEL;

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
CIFX_STATIC int32_t APIENTRY HIFxChannelClose(CIFXHANDLE hChannel)
{
  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  --ptChannel->ulOpenCount;

  return CIFX_NO_ERROR;
}

/*****************************************************************************/
/*! Download a file (Firmware, Configuration, etc) to the device
*   \param hChannel           Handle to the channel
*   \param ulMode             Download mode (DOWNLOAD_MODE_FIRMWARE, etc)
*   \param pszFileName        Name of the file
*   \param pabFileData        Pointer to the file data
*   \param ulFileSize         Length of the file data
*   \param pfnCallback        Callback for progress indication
*                             (NULL for no callback)
*   \param pfnRecvPktCallback Callback Callback pointer for unsolicited receive packets
*                             (NULL for no callback)
*   \param pvUser             User parameter passed to callback
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelDownload(CIFXHANDLE            hChannel,
                                              uint32_t              ulMode,
                                              char*                 pszFileName,
                                              uint8_t*              pabFileData,
                                              uint32_t              ulFileSize,
                                              PFN_PROGRESS_CALLBACK pfnCallback,
                                              PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                              void*                 pvUser)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  int32_t          lRet          = CIFX_NO_ERROR;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pszFileName);
  CHECK_POINTER(pabFileData);

  switch(ulMode)
  {
    case DOWNLOAD_MODE_FIRMWARE:
    case DOWNLOAD_MODE_CONFIG:
    case DOWNLOAD_MODE_FILE:
      lRet = CIFX_MAKE_DEV_FUN(DEV_DownloadFile)(
          hChannel,
          ptChannel->ulChannelNumber,
          ptChannel->tFromHostMbx.tCom.tBlock.ulElementSize,
          HIL_FILE_XFER_FILE,
          pszFileName,
          ulFileSize,
          pabFileData,
          CIFX_MAKE_DEV_FUN(DEV_TransferPacket),
          pfnCallback,
          pfnRecvPktCallback,
          pvUser);
      break;

    default:
      lRet = CIFX_INVALID_PARAMETER;
      break;
  }


  return lRet;
}

/*****************************************************************************/
/*! Returns the Mailbox state from a specific channel
*   \param hChannel         Channel handle acquired by xChannelOpen
*   \param pulRecvPktCount  Number of Messages waiting in receive mailbox
*   \param pulSendPktCount  State of the Send Mailbox
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelGetMBXState(CIFXHANDLE hChannel, uint32_t* pulRecvPktCount, uint32_t* pulSendPktCount)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  CHECK_CHANNELHANDLE(hChannel);

  return CIFX_MAKE_DEV_FUN(DEV_GetMBXState)(ptChannel, pulRecvPktCount, pulSendPktCount);
}

/*****************************************************************************/
/*! Inserts a packet into the channels mailbox
*   \param hChannel   Channel handle acquired by xChannelOpen
*   \param ptSendPkt  Packet to send to channel
*   \param ulTimeout  Time in ms to wait for card to accept the packet
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelPutPacket(CIFXHANDLE hChannel, CIFX_PACKET*  ptSendPkt, uint32_t ulTimeout)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  CHECK_CHANNELHANDLE(hChannel);

  /* Check if another command is active */
  if ( 0 == OS_WaitMutex( ptChannel->tFromHostMbx.tCom.tBlock.pvMutex, ulTimeout))
    return CIFX_DRV_CMD_ACTIVE;

  lRet = CIFX_MAKE_DEV_FUN(DEV_PutPacket)(ptChannel, ptSendPkt, ulTimeout);

  /* Release command */
  OS_ReleaseMutex(ptChannel->tFromHostMbx.tCom.tBlock.pvMutex);

  return lRet;
}

/*****************************************************************************/
/*! Gets a packet from the channels mailbox
*   \param hChannel   Channel handle acquired by xChannelOpen
*   \param ulSize     Size of the return packet buffer
*   \param ptRecvPkt  Returned packet
*   \param ulTimeout  Time in ms to wait for available message
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelGetPacket(CIFXHANDLE hChannel, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  CHECK_CHANNELHANDLE(hChannel);

  /* Check if another command is active */
  if ( 0 == OS_WaitMutex( ptChannel->tToHostMbx.tCom.tBlock.pvMutex, ulTimeout))
    return CIFX_DRV_CMD_ACTIVE;

  lRet = CIFX_MAKE_DEV_FUN(DEV_GetPacket)(ptChannel, ptRecvPkt, ulSize, ulTimeout);

  /* Release command */
  OS_ReleaseMutex(ptChannel->tToHostMbx.tCom.tBlock.pvMutex);

  return lRet;
}

/*****************************************************************************/
/*! Gets send packet from the channels mailbox
*   \param hChannel   Channel handle acquired by xChannelOpen
*   \param ulSize     Size of the return packet buffer
*   \param ptRecvPkt  Returned packet
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelGetSendPacket(CIFXHANDLE hChannel, uint32_t ulSize, CIFX_PACKET* ptRecvPkt)
{
  PCHANNELINSTANCE   ptChannel   = (PCHANNELINSTANCE)hChannel;
  NETX_BLOCK_T*      ptMailbox   = &ptChannel->tFromHostMbx.tCom.tBlock;
  NETX_HS_CTL_T*     ptCtl       = &ptChannel->tFromHostMbx.tCom.tCtl;
  uint32_t           ulHostFlags = ptCtl->ulHostFlags & ~HIL_HIF_MBX_WRAPAROUND;
  CIFX_PACKET*       ptPacket    = (CIFX_PACKET*) (ptMailbox->pbBlockStart + ulHostFlags * ptMailbox->ulElementSize);
  uint32_t           ulCopySize  = 0;
  int32_t            lRet        = CIFX_NO_ERROR;

  CHECK_CHANNELHANDLE(hChannel);

  ulCopySize  = HWIF_READ32(ptChannel->pvDeviceInstance, ptPacket->tHeader.ulLen) + HIL_PACKET_HEADER_SIZE;

  if( ulCopySize > ulSize)
  {
    /* Use the user buffer length if packet does not fit in the user buffer */
    ulCopySize = ulSize;
    lRet = CIFX_BUFFER_TOO_SHORT;
  }

  /* Just copy the available data into the user buffer */
  HWIF_READN(ptChannel->pvDeviceInstance, ptRecvPkt, ptPacket, ulCopySize);

  return lRet;
}

/*****************************************************************************/
/*! Send a packet to the device to lock/unlock the configuration
*   \param ptChannel  Channel instance
*   \param ulState    State to change to
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t DoConfigLock(PCHANNELINSTANCE ptChannel, uint32_t ulState)
{
  PDEVICEINSTANCE              ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  HIL_LOCK_UNLOCK_CONFIG_REQ_T tSendPkt;
  HIL_LOCK_UNLOCK_CONFIG_CNF_T tRecvPkt;
  int32_t                      lRet          = CIFX_NO_ERROR;

  OS_Memset(&tSendPkt, 0, sizeof(tSendPkt));
  OS_Memset(&tRecvPkt, 0, sizeof(tRecvPkt));

  /* Read firmware information */
  tSendPkt.tHead.ulDest   = HOST_TO_LE32(HIL_PACKET_DEST_DEFAULT_CHANNEL);
  tSendPkt.tHead.ulSrc    = HOST_TO_LE32(ptDevInstance->ulPhysicalAddress);
  tSendPkt.tHead.ulCmd    = HOST_TO_LE32(HIL_LOCK_UNLOCK_CONFIG_REQ);
  tSendPkt.tHead.ulLen    = HOST_TO_LE32(sizeof(tSendPkt.tData));
  tSendPkt.tData.ulParam  = HOST_TO_LE32(ulState);

  /* Transfer packet */
  lRet = CIFX_MAKE_DEV_FUN(DEV_TransferPacket)(
      ptChannel,
      (CIFX_PACKET*)&tSendPkt,
      (CIFX_PACKET*)&tRecvPkt,
      sizeof(tRecvPkt),
      CIFX_TO_SEND_PACKET,
      NULL,
      NULL);

  if( (CIFX_NO_ERROR  != lRet) ||
      (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(tRecvPkt.tHead.ulSta))) )
  {
    if(g_ulTraceLevel & CIFX_TRACE_LEVEL_WARNING)
    {
      USER_Trace(ptDevInstance,
                CIFX_TRACE_LEVEL_WARNING,
                "Error changing lock/unlock state! (lRet=0x%08X)",
                lRet);
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Lock the configuration on a communication channel
*   \param hChannel         Channel handle
*   \param ulCmd            CIFX_CONFIGURATION_XXX defines
*   \param pulState         Return locking state
*   \param ulTimeout        Timeout in [ms]
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelConfigLock(CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout)
{
  int32_t           lRet      = CIFX_NO_ERROR;
  PCHANNELINSTANCE  ptChannel = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pulState);

  UNREFERENCED_PARAMETER(ulTimeout);

  /* Read actual BUS state */
  *pulState = (LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->ulCommunicationState)) & HIL_HIF_COMM_STATE_CONFIG_LOCK) ?
      CIFX_CONFIGURATION_LOCK : CIFX_CONFIGURATION_UNLOCK;

  switch (ulCmd)
  {
    case CIFX_CONFIGURATION_LOCK:
    {
      /* Only change if UNLOCKED */
      if (CIFX_CONFIGURATION_LOCK != *pulState)
      {
        lRet = DoConfigLock(ptChannel, HIL_LOCK_UNLOCK_CONFIG_PARAM_LOCK);
      }
    }
    break;

    case CIFX_CONFIGURATION_UNLOCK:
    {
      /* Only change if LOCKED */
      if (CIFX_CONFIGURATION_UNLOCK != *pulState)
      {
        lRet = DoConfigLock(ptChannel, HIL_LOCK_UNLOCK_CONFIG_PARAM_UNLOCK);
      }
    }
    break;

    case CIFX_CONFIGURATION_GETLOCKSTATE:
      /* State already read above */
    break;

    default:
      /* Unknown command */
      lRet = CIFX_INVALID_COMMAND;
    break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Set BUS state off a communication channel
*   \param hChannel         Channel handle
*   \param ulCmd            CIFX_CONFIGURATION_XXX defines
*   \param pulState         Return actual state on CIFX_GET_BUS_STATE
*   \param ulTimeout        Timeout in [ms]
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelBusState(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  int32_t lRet                   = CIFX_INVALID_PARAMETER;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pulState);

  lRet = CIFX_MAKE_DEV_FUN(DEV_BusState)(
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
CIFX_STATIC int32_t APIENTRY HIFxChannelReset(CIFXHANDLE  hChannel, uint32_t ulResetMode, uint32_t ulTimeout)
{
  int32_t          lRet;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  CHECK_CHANNELHANDLE(hChannel);

  /* TODO: Get packet and I/O mutexe before processing a channel init */
  /* TODO: System start via channel ? */
  switch(ulResetMode)
  {
  case CIFX_SYSTEMSTART:
    lRet = CIFX_MAKE_DEV_FUN(DEV_DoSystemStart)(ptChannel, ulTimeout, 0);
    #ifdef CIFX_TOOLKIT_TIME
      if (CIFX_NO_ERROR == lRet)
        cifXInitTime((PDEVICEINSTANCE)ptChannel->pvDeviceInstance);
    #endif
    break;

  case CIFX_CHANNELINIT:
    lRet = CIFX_MAKE_DEV_FUN(DEV_DoChannelInit)(ptChannel, ulTimeout);
    break;

  default:
    lRet = CIFX_INVALID_PARAMETER;
    break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Reads I/O Area information for the given Channel
*   \param hChannel         Channel handle
*   \param ulCmd            CIFX_IO_INPUT_AREA/CIFX_IO_OUTPUT_AREA
*   \param ulAreaNumber     Number of area to get information for
*   \param ulSize           Size of returned data structure
*   \param pvData           Pointer to returned data
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelIOInfo(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulAreaNumber, uint32_t ulSize, void* pvData)
{
  int32_t                 lRet            = CIFX_NO_ERROR;
  PCHANNELINSTANCE        ptChannel       = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE         ptDevInstance   = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  CHANNEL_IO_INFORMATION* ptIoInformation = (CHANNEL_IO_INFORMATION*)pvData;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvData);

  if(ulSize != sizeof(*ptIoInformation))
    return CIFX_INVALID_BUFFERSIZE;

  if(ulAreaNumber >= 1)
    lRet = CIFX_INVALID_PARAMETER;

  if(!CIFX_MAKE_DEV_FUN(DEV_IsRunning)(ptChannel))
    lRet = CIFX_DEV_NOT_RUNNING;

  switch(ulCmd)
  {
    case CIFX_IO_INPUT_AREA:
      if(0 == ptChannel->tIoArea.ulIOInputAreas)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;

      } else if(ulAreaNumber >= ptChannel->tIoArea.ulIOInputAreas)
      {
        lRet = CIFX_INVALID_PARAMETER;
      } else
      {
        ptIoInformation->ulIOMode     = HIL_IO_MODE_BUFF_HST_CTRL;
        ptIoInformation->ulTotalSize  = ptChannel->tIoArea.aptIOInputAreas[ulAreaNumber]->tBlock.ulElementSize;
      }
      break;

    case CIFX_IO_OUTPUT_AREA:
      if(0 == ptChannel->tIoArea.ulIOOutputAreas)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;

      } else if(ulAreaNumber >= ptChannel->tIoArea.ulIOOutputAreas)
      {
        lRet = CIFX_INVALID_PARAMETER;
      } else
      {
        ptIoInformation->ulIOMode     = HIL_IO_MODE_BUFF_HST_CTRL;
        ptIoInformation->ulTotalSize  = ptChannel->tIoArea.aptIOOutputAreas[ulAreaNumber]->tBlock.ulElementSize;
      }
      break;

    default:
        lRet = CIFX_INVALID_COMMAND;
      break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Reads the Input data from the channel
*   \param hChannel     Channel handle acquired by xChannelOpen
*   \param ulAreaNumber Number of the I/O Area (0..n)
*   \param ulOffset     Data offset in Input area
*   \param ulDataLen    Length of data to read
*   \param pvData       Buffer to place returned data
*   \param ulTimeout    Timeout in ms to wait for finished I/O Handshake
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelIORead(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData, uint32_t ulTimeout)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  NETX_IO_BLOCK_T* ptIOArea      = NULL;
  int32_t          lRet          = CIFX_NO_ERROR;
  uint8_t          bIOBitState   = HIL_FLAGS_NONE;

  CHECK_CHANNELHANDLE(hChannel);

  if(!CIFX_MAKE_DEV_FUN(DEV_IsRunning)(ptChannel))
    return CIFX_DEV_NOT_RUNNING;

  if(ulAreaNumber >= ptChannel->tIoArea.ulIOInputAreas)
    return CIFX_INVALID_PARAMETER;

  ptIOArea    = ptChannel->tIoArea.aptIOInputAreas[ulAreaNumber];
#if 0 // TODO
  bIOBitState = DEV_GetIOBitstate(ptChannel, ptIOArea, 0);  // TODO this returns HIL_FLAGS_NOT_EQUAL
#endif

#ifdef CIFX_TOOLKIT_DMA
  /* Check for DMA transfer */
  if( ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_DMA)
  {
    /*------------------------------*/
    /* This is DMA IO data transfer */
    /*------------------------------*/
    /* This is DMACh n */
    PDEVICEINSTANCE   ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
    uint32_t          ulDMChIdx     = ptChannel->ulChannelNumber * 2 + eDMA_INPUT_BUFFER_IDX;
    PCIFX_DMABUFFER_T ptDmaInfo     = &ptDevInstance->atDmaBuffers[ulDMChIdx];

    if(0 != ulAreaNumber )                                  /* Only support for area 0 in DMA mode */
      return CIFX_DEV_DMA_IO_AREA_NOT_SUPPORTED;

    if( (ulOffset + ulDataLen) > ptDmaInfo->ulSize)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Check if another command is active */
    if ( !OS_WaitMutex( ptIOArea->pvMutex, ulTimeout))
      return CIFX_DRV_CMD_ACTIVE;

    /* Read data */
    if(!CIFX_MAKE_DEV_FUN(DEV_WaitForBitState)(ptChannel, ptIOArea->bHandshakeBit, bIOBitState, ulTimeout))
    {
      lRet = CIFX_DEV_EXCHANGE_FAILED;
    } else
    {
      /* Lock flag access */
      OS_EnterLock(ptChannel->pvLock);

      /* Read data done */
      CIFX_MAKE_DEV_FUN(DEV_ToggleBit)(ptChannel, (uint32_t)(1UL << ptIOArea->bHandshakeBit));

      /* Unlock flag access */
      OS_LeaveLock(ptChannel->pvLock);

      /* Read data */
      OS_Memcpy(  pvData,
                  ((uint8_t*)(ptDmaInfo->pvBuffer)) + ulOffset,
                  ulDataLen);

      /* Check COMM Flag for return value */
      (void)CIFX_MAKE_DEV_FUN(DEV_IsCommunicating)(ptChannel, &lRet);
    }

    /* Release command */
    OS_ReleaseMutex( ptIOArea->pvMutex);

  } else
#endif
  {
    /*---------------------------*/
    /* This is DPM data transfer */
    /*---------------------------*/
    ulOffset = ulOffset + HIL_HIF_IO_DATA_OFFSET; /* skip reserved area */
    if( (ulOffset + ulDataLen) > ptIOArea->tBlock.ulElementSize)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Check if another command is active */
    if ( !OS_WaitMutex( ptIOArea->tBlock.pvMutex, ulTimeout))
      return CIFX_DRV_CMD_ACTIVE;

    /* Read data */
    if(!CIFX_MAKE_DEV_FUN(DEV_WaitForIoBitState)(ptChannel, ptIOArea, ulTimeout))
    {
      lRet = CIFX_DEV_EXCHANGE_FAILED;
    } else
    {
      /* Lock flag access */
      OS_EnterLock(ptChannel->pvLock);

      /* Read data done */
      CIFX_MAKE_DEV_FUN(DEV_ToggleIoAction)(ptChannel, ptIOArea);

      /* Unlock flag access */
      OS_LeaveLock(ptChannel->pvLock);

      /* Read data */
      HWIF_READN( ptChannel->pvDeviceInstance,
                  pvData,
                  &ptIOArea->tBlock.pbBlockStart[ulOffset],
                  ulDataLen);

      /* Check COMM Flag for return value */
      (void)CIFX_MAKE_DEV_FUN(DEV_IsCommunicating)(ptChannel, &lRet);
    }

    /* Release command */
    OS_ReleaseMutex( ptIOArea->tBlock.pvMutex);
  }

  return lRet;
}

/*****************************************************************************/
/*! Writes the Output data to the channel
*   \param hChannel     Channel handle acquired by xChannelOpen
*   \param ulAreaNumber Number of the I/O Area (0..n)
*   \param ulOffset     Data offset in Output area
*   \param ulDataLen    Length of data to send
*   \param pvData       Buffer containing send data
*   \param ulTimeout    Timeout in ms to wait for handshake completion
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelIOWrite(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData, uint32_t ulTimeout)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  PNETX_IO_BLOCK_T ptIOArea      = NULL;
  int32_t          lRet          = CIFX_NO_ERROR;
  uint8_t          bIOBitState   = HIL_FLAGS_NONE;

  CHECK_CHANNELHANDLE(hChannel);

  if(!CIFX_MAKE_DEV_FUN(DEV_IsRunning)(ptChannel))
    return CIFX_DEV_NOT_RUNNING;

  if(ulAreaNumber >= ptChannel->tIoArea.ulIOOutputAreas)
    return CIFX_INVALID_PARAMETER;

  ptIOArea    = ptChannel->tIoArea.aptIOOutputAreas[ulAreaNumber];
#if 0
  bIOBitState = DEV_GetIOBitstate(ptChannel, ptIOArea, 1);  // TODO this returns HIL_FLAGS_NOT_EQUAL
#endif

#ifdef CIFX_TOOLKIT_DMA
  /* Check for DMA transfer */
  if( ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_DMA)
  {
    /*------------------------------*/
    /* This is DMA IO data transfer */
    /*------------------------------*/
    /* This is DMACh n+1 */
    PDEVICEINSTANCE   ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
    uint32_t          ulDMChIdx     = ptChannel->ulChannelNumber * 2 + eDMA_OUTPUT_BUFFER_IDX;
    PCIFX_DMABUFFER_T ptDmaInfo     = &ptDevInstance->atDmaBuffers[ulDMChIdx];

    if(0 != ulAreaNumber )                                  /* Only support for area 0 in DMA mode */
      return CIFX_DEV_DMA_IO_AREA_NOT_SUPPORTED;

    if( (ulOffset + ulDataLen) > ptDmaInfo->ulSize)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Check if another command is active */
    if ( !OS_WaitMutex( ptIOArea->pvMutex, ulTimeout))
      return CIFX_DRV_CMD_ACTIVE;

    if(!CIFX_MAKE_DEV_FUN(DEV_WaitForBitState)(ptChannel, ptIOArea->bHandshakeBit, bIOBitState, ulTimeout))
    {
      lRet = CIFX_DEV_EXCHANGE_FAILED;
    } else
    {
      /* Read data */
      OS_Memcpy( (((uint8_t*)(ptDmaInfo->pvBuffer)) + ulOffset),
                 pvData,
                 ulDataLen);

      /* Lock flag access */
      OS_EnterLock(ptChannel->pvLock);

      /* Read data done */
      CIFX_MAKE_DEV_FUN(DEV_ToggleBit)(ptChannel, (uint32_t)(1UL << ptIOArea->bHandshakeBit));

      /* Unlock flag access */
      OS_LeaveLock(ptChannel->pvLock);

      /* Check COMM Flag for return value */
      (void)CIFX_MAKE_DEV_FUN(DEV_IsCommunicating)(ptChannel, &lRet);
    }

    /* Release command */
    OS_ReleaseMutex( ptIOArea->pvMutex);

  } else
#endif
  {
    /*---------------------------*/
    /* This is DPM data transfer */
    /*---------------------------*/
    ulOffset = ulOffset + HIL_HIF_IO_DATA_OFFSET; /* skip reserved area */
    if( (ulOffset + ulDataLen) > ptIOArea->tBlock.ulElementSize)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Check if another command is active */
    if ( !OS_WaitMutex( ptIOArea->tBlock.pvMutex, ulTimeout))
      return CIFX_DRV_CMD_ACTIVE;

    /* Check if buffer is still locked by running DMA (incremental update mode only) */
    if(!CIFX_MAKE_DEV_FUN(DEV_WaitForLock_Poll)(ptChannel, ptIOArea, ulTimeout))
    {
      lRet = CIFX_DEV_EXCHANGE_FAILED;
    } else
    {
      /* Write data */
      HWIF_WRITEN(  ptChannel->pvDeviceInstance,
                   &ptIOArea->tBlock.pbBlockStart[ulOffset],
                    pvData,
                    ulDataLen);

      /* Lock flag access */
      OS_EnterLock(ptChannel->pvLock);

      /* Write data done */
      CIFX_MAKE_DEV_FUN(DEV_ToggleIoAction)(ptChannel, ptIOArea);

      /* Unlock flag access */
      OS_LeaveLock(ptChannel->pvLock);

      /* Check COMM Flag for return value */
      (void)CIFX_MAKE_DEV_FUN(DEV_IsCommunicating)(ptChannel, &lRet);

      /* Release command */
      OS_ReleaseMutex( ptIOArea->tBlock.pvMutex);
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Read back Send Data Area from channel
*   \param hChannel     Channel handle acquired by xChannelOpen
*   \param ulAreaNumber Number of the I/O Area (0..n)
*   \param ulOffset     Data start offset
*   \param ulDataLen    Data length to read
*   \param pvData       Data buffer
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelIOReadSendData(CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hChannel;
  NETX_IO_BLOCK_T* ptIOArea  = NULL;
  int32_t          lRet      = CIFX_NO_ERROR;

  CHECK_CHANNELHANDLE(hChannel);

#ifdef CIFX_TOOLKIT_DMA
  /* Check for DMA transfer */
  if( ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_DMA)
  {
    /* This is DMACh n */
    PDEVICEINSTANCE   ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
    uint32_t          ulDMChIdx     = ptChannel->ulChannelNumber * 2 + eDMA_OUTPUT_BUFFER_IDX;  /* We reading the Output buffer */
    PCIFX_DMABUFFER_T ptDmaInfo     = &ptDevInstance->atDmaBuffers[ulDMChIdx];

    if(0 != ulAreaNumber )                                  /* Only support for area 0 in DMA mode */
      return CIFX_DEV_DMA_IO_AREA_NOT_SUPPORTED;

    if( (ulOffset + ulDataLen) > ptDmaInfo->ulSize)
      return CIFX_INVALID_ACCESS_SIZE;                      /* read size too long */

    /* Read data */
    OS_Memcpy(  pvData,
                ((uint8_t*)(ptDmaInfo->pvBuffer)) + ulOffset,
                ulDataLen);
  } else
#endif
  {
    if(ulAreaNumber >= ptChannel->tIoArea.ulIOOutputAreas)
      return CIFX_INVALID_PARAMETER;

    ptIOArea = ptChannel->tIoArea.aptIOOutputAreas[ulAreaNumber];

    ulOffset = ulOffset + HIL_HIF_IO_DATA_OFFSET; /* skip reserved area */
    if( (ulOffset + ulDataLen) > ptIOArea->tBlock.ulElementSize)
      return CIFX_INVALID_ACCESS_SIZE; /* read size too long */

    /* Read data */
    HWIF_READN(ptChannel->pvDeviceInstance,
               pvData,
               &ptIOArea->tBlock.pbBlockStart[ulOffset],
               ulDataLen);
  }

  return lRet;
}

/*****************************************************************************/
/*! Read/Write Control block
*   \param hChannel       Handle to the channel
*   \param ulCmd          CIFX_CMD_READ_DATA/CIFX_CMD_WRITE_DATA
*   \param ulOffset       Start offset of read/write
*   \param ulDataLen      Length of data to read/write
*   \param pvData         Buffer to copy from/to
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelControlBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  UNREFERENCED_PARAMETER(hChannel);
  UNREFERENCED_PARAMETER(ulCmd);
  UNREFERENCED_PARAMETER(ulOffset);
  UNREFERENCED_PARAMETER(ulDataLen);
  UNREFERENCED_PARAMETER(pvData);
  return CIFX_FUNCTION_NOT_AVAILABLE;
}

/*****************************************************************************/
/*! Read/Write Common status block
*   \param hChannel       Handle to the channel
*   \param ulCmd          CIFX_CMD_READ_DATA
*   \param ulOffset       Start offset of read
*   \param ulDataLen      Length of data to read
*   \param pvData         Buffer to copy to
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelCommonStatusBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvData);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

    /* Check if STATUS block is available */
  } else if(NULL == ptChannel->ptCommunicationStatusBlock)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else
  {
    lRet = CIFX_MAKE_DEV_FUN(DEV_ReadWriteBlock)(
        ptChannel,
        (void*)ptChannel->ptCommunicationStatusBlock,
        ulOffset,
        sizeof(*ptChannel->ptCommunicationStatusBlock),
        pvData,
        ulDataLen,
        ulCmd,
        0);

    /* Note: We accept errors from the DEV_ReadWriteBlock() function because we want to inform the user in any case */
    /*       (e.g. CIFX_DEV_NOT_RUNNING) even if we running the conversion on an not filled / empty buffer! */
    if(CIFX_CMD_READ_DATA == ulCmd)
    {
      int32_t lRetTmp = cifXConvertEndianess(ulOffset,
                                             pvData,
                                             ulDataLen,
                                             s_atCommonStatusBlock,
                                             HIL_CNT_ELEMENT(s_atCommonStatusBlock));

      /* Error of DEV_ReadWriteBlock() takes priority over (possible) Endianess conversion error */
      if((CIFX_NO_ERROR == lRet) && (CIFX_NO_ERROR != lRetTmp))
        lRet = lRetTmp;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Read Extended status block
*   \param hChannel       Handle to the channel
*   \param ulCmd          CIFX_CMD_READ_DATA
*   \param ulOffset       Start offset of read
*   \param ulDataLen      Length of data to read
*   \param pvData         Buffer to copy to
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelExtendedStatusBlock(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  PCHANNELINSTANCE  ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE   ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  int32_t           lRet          = CIFX_NO_ERROR;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvData);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;
  } else
  {
    HIL_DPM_GET_EXTENDED_STATE_REQ_T tSendPkt;
    HIL_DPM_GET_EXTENDED_STATE_CNF_T tRecvPkt;

    OS_Memset(&tSendPkt, 0, sizeof(tSendPkt));
    OS_Memset(&tRecvPkt, 0, sizeof(tRecvPkt));

    /* Read firmware information */
    tSendPkt.tHead.ulDest         = HOST_TO_LE32(HIL_PACKET_DEST_DEFAULT_CHANNEL);
    tSendPkt.tHead.ulSrc          = HOST_TO_LE32(ptDevInstance->ulPhysicalAddress);
    tSendPkt.tHead.ulCmd          = HOST_TO_LE32(HIL_DPM_GET_EXTENDED_STATE_REQ);
    tSendPkt.tHead.ulLen          = HOST_TO_LE32(sizeof(tSendPkt.tData));
    tSendPkt.tData.ulOffset       = HOST_TO_LE32(ulOffset);
    tSendPkt.tData.ulDataLen      = HOST_TO_LE32(ulDataLen);
    tSendPkt.tData.ulChannelIndex = HOST_TO_LE32(ptChannel->ulChannelNumber);

    /* Transfer packet */
    lRet = CIFX_MAKE_DEV_FUN(DEV_TransferPacket)(
        ptChannel,
        (CIFX_PACKET*)&tSendPkt,
        (CIFX_PACKET*)&tRecvPkt,
        sizeof(tRecvPkt),
        CIFX_TO_SEND_PACKET,
        NULL,
        NULL);

    if( (CIFX_NO_ERROR  != lRet) ||
        (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(tRecvPkt.tHead.ulSta))) )
    {
      if(g_ulTraceLevel & CIFX_TRACE_LEVEL_WARNING)
      {
        USER_Trace(ptDevInstance,
                  CIFX_TRACE_LEVEL_WARNING,
                  "Error querying extended status block! (lRet=0x%08X)",
                  lRet);
      }
    } else
    {
      OS_Memcpy(pvData, tRecvPkt.tData.abData, ulDataLen);
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Read a user block
*   \param hChannel       Handle to the channel
*   \param ulAreaNumber   Userblock number
*   \param ulCmd          CIFX_CMD_READ_DATA
*   \param ulOffset       Start offset of read
*   \param ulDataLen      Length of data to read
*   \param pvData         Buffer to copy to
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelUserBlock(CIFXHANDLE  hChannel, uint32_t ulAreaNumber, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData)
{
  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvData);

  return CIFX_FUNCTION_NOT_AVAILABLE;
}

/*****************************************************************************/
/*! Gets a pointer to an IO Area
*   \param hChannel       Handle to the channel
*   \param ulCmd          CIFX_MEM_PTR_OPEN/CIFX_MEM_PTR_CLOSE
*   \param pvMemoryInfo   Pointer to requested memory structure
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelPLCMemoryPtr(CIFXHANDLE hChannel, uint32_t ulCmd, void* pvMemoryInfo)
{
  PLC_MEMORY_INFORMATION* ptMemory          = (PLC_MEMORY_INFORMATION*)pvMemoryInfo;
  PCHANNELINSTANCE        ptChannel         = (PCHANNELINSTANCE)hChannel;
  int                     fUseCaching       = 0;
  unsigned long           ulAreaDefinition  = 0;
  PDEVICEINSTANCE         ptDevInstance     = NULL;
  int32_t                 lRet              = CIFX_NO_ERROR;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvMemoryInfo);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
    return CIFX_DRV_CHANNEL_NOT_INITIALIZED;

  /* No IO blocks defined */
  if( (0 == ptChannel->tIoArea.ulIOInputAreas) ||
      (0 == ptChannel->tIoArea.ulIOOutputAreas)  )
    return CIFX_FUNCTION_NOT_AVAILABLE;

  ptDevInstance     = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  fUseCaching       = ptDevInstance->fCachedMemAccess;
  ulAreaDefinition  = ptMemory->ulAreaDefinition;

#ifdef CIFX_CACHE_TEST
  /* Check for additional option in area definition */
  /* and overwrite if additional flags are set. */
  if (0 != (ulAreaDefinition & ~CIFX_IO_AREA_MASK))
  {
    /* Override caching option to get uncached pointers */
    ulAreaDefinition &= CIFX_IO_AREA_MASK;
    fUseCaching      = 0;
  }
#endif

#ifdef CIFX_TOOLKIT_DMA

  /* Check for DMA transfer */
  if( ptChannel->ulDeviceCOSFlags & HIL_COMM_COS_DMA)
  {
    /*------------------------------*/
    /* This is DMA IO data transfer */
    /*------------------------------*/
    /* This is DMACh n */
    PCIFX_DMABUFFER_T       ptDmaInfo       = NULL;
    CACHED_MEMORY_AREA_T*   ptCachedMemInfo = NULL;
    uint32_t                ulDMChIdx       = 0;

    if(0 != ptMemory->ulAreaNumber)
    {
      /* Invalid IO area */
      lRet = CIFX_DEV_DMA_IO_AREA_NOT_SUPPORTED;  /* Only support for area 0 in DMA mode */
    } else
    {
      if (ulAreaDefinition == CIFX_IO_INPUT_AREA)
      {
        ulDMChIdx       = ptChannel->ulChannelNumber * 2 + eDMA_INPUT_BUFFER_IDX;
        ptDmaInfo       = &ptDevInstance->atDmaBuffers[ulDMChIdx];
        ptCachedMemInfo = &ptChannel->tCachedIOInputArea;
      } else
      {
        ulDMChIdx       = ptChannel->ulChannelNumber * 2 + eDMA_OUTPUT_BUFFER_IDX;
        ptDmaInfo       = &ptDevInstance->atDmaBuffers[ulDMChIdx];
        ptCachedMemInfo = &ptChannel->tCachedIOOutputArea;
      }

      /* Check user command */
      switch(ulCmd)
      {
        case CIFX_MEM_PTR_OPEN:
        {
          void*     pvMappedDPM = NULL;
          void*     pvDPM       = ptDmaInfo->pvBuffer;
          uint32_t  ulDPMSize   = ptDmaInfo->ulSize;

          /* Check for caching option */
          if ((0 != fUseCaching) &&
              (NULL != ptCachedMemInfo->pvMemPtr))
          {
            /* Mapping already exists */
            lRet = CIFX_NO_MORE_ENTRIES;
          } else
          {
            /* Return global memory information */
            if (NULL == (ptMemory->pvMemoryID = OS_MapUserPointer(pvDPM, ulDPMSize, &pvMappedDPM, ptDevInstance->pvOSDependent, (unsigned char)fUseCaching)))
            {
              lRet = CIFX_MEMORY_MAPPING_FAILED;
            } else
            {
              *(ptMemory->ppvMemoryPtr)         = (void*)pvMappedDPM;
              *(ptMemory->pulIOAreaStartOffset) = 0;                    /* In DMA mode, we don't have a DPM start offeset */
              *(ptMemory->pulIOAreaSize)        = ulDPMSize;

              /* Store the memory pointer for later cache operation (e.g. flush) */
              if (0 != fUseCaching)
              {
                ptCachedMemInfo->pvMemPtr   = (void*)pvMappedDPM;
                ptCachedMemInfo->ulAreaSize = ulDPMSize;
              }
            }
          }
        }
        break;

        case CIFX_MEM_PTR_CLOSE:
        {
          if(!OS_UnmapUserPointer(ptMemory->pvMemoryID, ptDevInstance->pvOSDependent))
          {
            lRet = CIFX_INVALID_HANDLE;
          } else
          {
            ptMemory->pvMemoryID              = NULL;
            *(ptMemory->ppvMemoryPtr)         = NULL;
            *(ptMemory->pulIOAreaStartOffset) = 0;
            *(ptMemory->pulIOAreaSize)        = 0;

            if (0 != fUseCaching)
            {
              /* Remove any stored cache buffer information */
              ptCachedMemInfo->pvMemPtr         = NULL;
              ptCachedMemInfo->ulAreaSize       = 0;
            }
          }
        }
        break;

        default:
          lRet = CIFX_INVALID_COMMAND;
        break;

      } /* end switch */
    }
  } else
#endif
  {
    NETX_IO_BLOCK_T**     ptIoBlock       = NULL;
    uint32_t              ulAreaCount     = 0;
    CACHED_MEMORY_AREA_T* ptCachedMemInfo = NULL;

    if(ulAreaDefinition == CIFX_IO_INPUT_AREA)
    {
      ulAreaCount     = ptChannel->tIoArea.ulIOInputAreas;
      ptIoBlock       = ptChannel->tIoArea.aptIOInputAreas;
      ptCachedMemInfo = &ptChannel->tCachedIOInputArea;

    } else
    {
      ulAreaCount     = ptChannel->tIoArea.ulIOOutputAreas;
      ptIoBlock       = ptChannel->tIoArea.aptIOOutputAreas;
      ptCachedMemInfo = &ptChannel->tCachedIOOutputArea;
    }

    /* Check if IO area is available */
    if (ptMemory->ulAreaNumber >= ulAreaCount)
    {
      /* Invalid IO area */
      lRet = CIFX_INVALID_PARAMETER;
    } else
    {
      /* Check user command */
      switch(ulCmd)
      {
        case CIFX_MEM_PTR_OPEN:
        {
          void*    pvMappedDPM  = NULL;
          void*    pvDPM        = ptIoBlock[ptMemory->ulAreaNumber]->tBlock.pbBlockStart;
          uint32_t ulDPMSize    = ptIoBlock[ptMemory->ulAreaNumber]->tBlock.ulElementSize;

          /* Check for caching option */
          if ((0 != fUseCaching) && (NULL != ptCachedMemInfo->pvMemPtr))
          {
            /* Mapping already exists */
            lRet = CIFX_NO_MORE_ENTRIES;

          }else
          {
            /* Return global memory information */
            if (NULL == (ptMemory->pvMemoryID = OS_MapUserPointer(pvDPM, ulDPMSize, &pvMappedDPM, ptDevInstance->pvOSDependent, (unsigned char)fUseCaching)))
            {
              lRet = CIFX_MEMORY_MAPPING_FAILED;
            } else
            {
              uint32_t ulOffset = HIL_HIF_IO_DATA_OFFSET; /* skip reserved area */

              *(ptMemory->ppvMemoryPtr)         = (void*)(((char*)pvMappedDPM) + ulOffset);
              *(ptMemory->pulIOAreaStartOffset) = (uint32_t)(ptIoBlock[ptMemory->ulAreaNumber]->tBlock.pbBlockStart - ptChannel->pbDPMChannelStart);
              *(ptMemory->pulIOAreaSize)        = ulDPMSize;

              if (0 != fUseCaching)
              {
                /* Remove any stored cache buffer information */
                ptCachedMemInfo->pvMemPtr     = (void*)(((char*)pvMappedDPM) + ulOffset);
                ptCachedMemInfo->ulAreaSize   = ulDPMSize;
              }
            }
          }
        }
        break;

        case CIFX_MEM_PTR_CLOSE:
        {
          if(!OS_UnmapUserPointer(ptMemory->pvMemoryID, ptDevInstance->pvOSDependent))
          {
            lRet = CIFX_INVALID_HANDLE;
          } else
          {
            ptMemory->pvMemoryID              = NULL;
            *(ptMemory->ppvMemoryPtr)         = NULL;
            *(ptMemory->pulIOAreaStartOffset) = 0;
            *(ptMemory->pulIOAreaSize)        = 0;

            if (0 != fUseCaching)
            {
              /* Remove any stored cache buffer information */
              ptCachedMemInfo->pvMemPtr       = NULL;
              ptCachedMemInfo->ulAreaSize     = 0;
            }
          }
        }
        break;

        default:
          lRet = CIFX_INVALID_COMMAND;
        break;
      } /* end switch */
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Checks if the given IO Area is ready for the next handshake
*   \param hChannel       Handle to the channel
*   \param ulAreaNumber   Area to check
*   \param pulReadState   Returned state of the area (!=0 means area is ready)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelPLCIsReadReady(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t* pulReadState)
{
  int32_t           lRet         = CIFX_NO_ERROR;
  PCHANNELINSTANCE  ptChannel    = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  CHECK_CHANNELHANDLE(hChannel);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

  } else if(0 == ptChannel->tIoArea.ulIOInputAreas)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else if(ulAreaNumber >= ptChannel->tIoArea.ulIOInputAreas)
  {
    lRet = CIFX_INVALID_PARAMETER;
  } else
  {
    /* Check if the device is communicating. If only the COM flag is missing, we return the current bit state */
    (void)CIFX_MAKE_DEV_FUN(DEV_IsCommunicating)(ptChannel, &lRet);

    if( (lRet != CIFX_DEV_NOT_READY) &&
        (lRet != CIFX_DEV_NOT_RUNNING) )
    {
      /* Read back the send data area */
      *pulReadState = 0;

      if(CIFX_MAKE_DEV_FUN(DEV_WaitForIoBitState)(ptChannel, ptChannel->tIoArea.aptIOInputAreas[ulAreaNumber], 0))
      {
        *pulReadState = 1;

        /* Invalidate the IO input buffer, for cache refresh */
        if (NULL != ptChannel->tCachedIOInputArea.pvMemPtr)
        {
          OS_InvalidateCacheMemory_FromDevice(ptChannel->tCachedIOInputArea.pvMemPtr, ptChannel->tCachedIOInputArea.ulAreaSize);
        }
      }
    }
  }

  /* Assuming that the request was handled */
  return lRet;
}

/*****************************************************************************/
/*! Checks if the given IO Area is ready for the next handshake
*   \param hChannel       Handle to the channel
*   \param ulAreaNumber   Area to check
*   \param pulWriteState  Returned state of the area (!=0 means area is ready)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelPLCIsWriteReady(CIFXHANDLE hChannel, uint32_t ulAreaNumber, uint32_t* pulWriteState)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  CHECK_CHANNELHANDLE(hChannel);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

  } else if(ulAreaNumber >= 1)
  {
    lRet = CIFX_INVALID_PARAMETER;
  } else
  {
    /* Check if the device is communication. If only the COM flag is missing, we return the current bit state */
    (void)CIFX_MAKE_DEV_FUN(DEV_IsCommunicating)(ptChannel, &lRet);

    if( (lRet != CIFX_DEV_NOT_READY) &&
        (lRet != CIFX_DEV_NOT_RUNNING) )
    {
      /* Check if buffer is still locked by running DMA (incremental update mode only) */
      if(CIFX_MAKE_DEV_FUN(DEV_WaitForLock_Poll)(ptChannel, ptChannel->tIoArea.aptIOOutputAreas[ulAreaNumber], 0))
      {
        *pulWriteState = 1;
      }
    }
  }

  /* Assuming that the request was handled */
  return lRet;
}

/*****************************************************************************/
/*! Toggles the Handshake bit for the given IO Output Area
*   \param hChannel       Handle to the channel
*   \param ulAreaNumber   Areanumber
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelPLCActivateWrite(CIFXHANDLE hChannel, uint32_t ulAreaNumber)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  CHECK_CHANNELHANDLE(hChannel);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

  } else if(0 == ptChannel->tIoArea.ulIOOutputAreas)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else if(ulAreaNumber >= ptChannel->tIoArea.ulIOOutputAreas)
  {
    lRet = CIFX_INVALID_PARAMETER;
  } else
  {
    /* Check if the device is communicating. If only the COM flag is missing, we toggle the bits */
    (void)CIFX_MAKE_DEV_FUN(DEV_IsCommunicating)(ptChannel, &lRet);

    if( (lRet != CIFX_DEV_NOT_READY) &&
        (lRet != CIFX_DEV_NOT_RUNNING) )
    {
      /* Flush the IO output cache to output buffer */
      if (NULL != ptChannel->tCachedIOOutputArea.pvMemPtr)
      {
        OS_FlushCacheMemory_ToDevice(ptChannel->tCachedIOOutputArea.pvMemPtr, ptChannel->tCachedIOOutputArea.ulAreaSize);
      }

      /* Lock flag access */
      OS_EnterLock(ptChannel->pvLock);

      /* Write data done */
      CIFX_MAKE_DEV_FUN(DEV_ToggleIoAction)(ptChannel, ptChannel->tIoArea.aptIOOutputAreas[ulAreaNumber]);

      /* Unlock flag access */
      OS_LeaveLock(ptChannel->pvLock);
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Toggles the Handshake bit for the given IO Input Area
*   \param hChannel       Handle to the channel
*   \param ulAreaNumber   Areanumber
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelPLCActivateRead(CIFXHANDLE hChannel, uint32_t ulAreaNumber)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  CHECK_CHANNELHANDLE(hChannel);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;

  } else if(0 == ptChannel->tIoArea.ulIOInputAreas)
  {
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
  } else if(ulAreaNumber >= ptChannel->tIoArea.ulIOInputAreas)
  {
    lRet = CIFX_INVALID_PARAMETER;
  } else
  {
    /* Check if the device is communicating. If only the COM flag is missing, we toggle the bits */
    (void)CIFX_MAKE_DEV_FUN(DEV_IsCommunicating)(ptChannel, &lRet);

    if( (lRet != CIFX_DEV_NOT_READY) &&
        (lRet != CIFX_DEV_NOT_RUNNING) )
    {
      /* Lock flag access */
      OS_EnterLock(ptChannel->pvLock);

      /* Read data done */
      CIFX_MAKE_DEV_FUN(DEV_ToggleIoAction)(ptChannel, ptChannel->tIoArea.aptIOInputAreas[ulAreaNumber]);

      /* Unlock flag access */
      OS_LeaveLock(ptChannel->pvLock);
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Get Channel information on an open channel
*   \param hChannel       Handle to the channel
*   \param ulSize         Size of return buffer
*   \param pvChannelInfo  Return buffer (CHANNEL_INFORMATION structure)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelInfo(CIFXHANDLE hChannel, uint32_t ulSize, void* pvChannelInfo)
{
  CHANNEL_INFORMATION*  ptChannelInfo = (CHANNEL_INFORMATION*)pvChannelInfo;
  PCHANNELINSTANCE      ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE       ptDevInstance = NULL;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pvChannelInfo);

  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  if(ulSize < (uint32_t)sizeof(*ptChannelInfo))
    return CIFX_INVALID_BUFFERSIZE;

  (void)OS_Strncpy(ptChannelInfo->abBoardName,
                   ptDevInstance->szName,
                   (uint32_t)sizeof(ptChannelInfo->abBoardName));
  (void)OS_Strncpy(ptChannelInfo->abBoardAlias,
                   ptDevInstance->szAlias,
                   (uint32_t)sizeof(ptChannelInfo->abBoardAlias));

  ptChannelInfo->ulDeviceNumber   = ptDevInstance->ulDeviceNumber;
  ptChannelInfo->ulSerialNumber   = ptDevInstance->ulSerialNumber;

  ptChannelInfo->usFWMajor        = ptChannel->tFirmwareIdent.tFwVersion.usMajor;
  ptChannelInfo->usFWMinor        = ptChannel->tFirmwareIdent.tFwVersion.usMinor;
  ptChannelInfo->usFWRevision     = ptChannel->tFirmwareIdent.tFwVersion.usRevision;
  ptChannelInfo->usFWBuild        = ptChannel->tFirmwareIdent.tFwVersion.usBuild;
  ptChannelInfo->bFWNameLength    = ptChannel->tFirmwareIdent.tFwName.bNameLength;

  OS_Memcpy(ptChannelInfo->abFWName,
            ptChannel->tFirmwareIdent.tFwName.abName,
            sizeof(ptChannelInfo->abFWName));

  ptChannelInfo->usFWYear         = ptChannel->tFirmwareIdent.tFwDate.usYear;
  ptChannelInfo->bFWMonth         = ptChannel->tFirmwareIdent.tFwDate.bMonth;
  ptChannelInfo->bFWDay           = ptChannel->tFirmwareIdent.tFwDate.bDay;

  ptChannelInfo->ulChannelError   = 0;
  if(0 != ptChannel->ptCommunicationStatusBlock)
  {
    ptChannelInfo->ulChannelError = LE32_TO_HOST(HWIF_READ32(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->ulCommunicationError));
  }

  ptChannelInfo->ulOpenCnt        = ptChannel->ulOpenCount;
  ptChannelInfo->ulPutPacketCnt   = ptChannel->tFromHostMbx.tCom.tBlock.ulTransmissionCnt;
  ptChannelInfo->ulGetPacketCnt   = ptChannel->tToHostMbx.tCom.tBlock.ulTransmissionCnt;
  ptChannelInfo->ulMailboxSize    = ptChannel->tFromHostMbx.tCom.tBlock.ulElementSize;
  ptChannelInfo->ulIOInAreaCnt    = ptChannel->tIoArea.ulIOInputAreas;
  ptChannelInfo->ulIOOutAreaCnt   = ptChannel->tIoArea.ulIOOutputAreas;
  ptChannelInfo->ulHskSize        = HIL_HANDSHAKE_SIZE_32BIT;

  /* Check if we are in interrupt mode */
  if(!((PDEVICEINSTANCE)(ptChannel->pvDeviceInstance))->fIrqEnabled)
    CIFX_MAKE_DEV_FUN(DEV_ReadHandshakeFlags)(ptChannel, 0, 1);

  ptChannelInfo->ulNetxFlags      = ptChannel->tHsCtrl.ulNetxFlags;
  ptChannelInfo->ulHostFlags      = ptChannel->tHsCtrl.ulHostFlags;
  ptChannelInfo->ulHostCOSFlags   = 0;  // TODO get values from commonstate field?
  ptChannelInfo->ulDeviceCOSFlags = 0;  // TODO get values from commonstate field?

  return CIFX_NO_ERROR;
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
CIFX_STATIC int32_t APIENTRY HIFxChannelWatchdog(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t* pulTrigger)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  return CIFX_MAKE_DEV_FUN(DEV_TriggerWatchdog)((PCHANNELINSTANCE)hChannel, ulCmd, pulTrigger);
}

/*****************************************************************************/
/*! Set/Get Host state of the card
*   \param hChannel       Handle to the channel
*   \param ulCmd          Host state command (CIFX_HOST_STATE_NOT_READY,
*                         CIFX_HOST_STATE_READY, CIFX_HOST_STATE_READ)
*   \param pulState       Returned state if command is CIFX_HOST_STATE_READ
*   \param ulTimeout      Time in ms to wait for start/stop communication
*                         flag
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelHostState(CIFXHANDLE hChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout)
{
  UNREFERENCED_PARAMETER(hChannel);
  UNREFERENCED_PARAMETER(ulCmd);
  UNREFERENCED_PARAMETER(pulState);
  UNREFERENCED_PARAMETER(ulTimeout);
  return CIFX_FUNCTION_NOT_AVAILABLE;
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
CIFX_STATIC int32_t APIENTRY HIFxChannelFindFirstFile(CIFXHANDLE             hChannel,
                                                   CIFX_DIRECTORYENTRY*   ptDirectoryInfo,
                                                   PFN_RECV_PKT_CALLBACK  pfnRecvPktCallback,
                                                   void*                  pvUser)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  return HIFxSysdeviceFindFirstFile(hChannel, ptChannel->ulChannelNumber, ptDirectoryInfo, pfnRecvPktCallback, pvUser);
}

/*****************************************************************************/
/*! Enumerate next entry in directory on the given channel
*   \param hChannel           Handle to the channel
*   \param ptDirectoryInfo    Pointer to enumeration result.
*   \param pfnRecvPktCallback Callback for unhandled packets
*   \param pvUser             User data for callback function
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelFindNextFile(CIFXHANDLE            hChannel,
                                                  CIFX_DIRECTORYENTRY*  ptDirectoryInfo,
                                                  PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                                  void*                 pvUser)
{
  PCHANNELINSTANCE ptChannel    = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  return HIFxSysdeviceFindNextFile(hChannel, ptChannel->ulChannelNumber, ptDirectoryInfo, pfnRecvPktCallback, pvUser);
}

/*****************************************************************************/
/*! Uploads a file via Communication channel
*   \param hChannel           Handle to the Channel
*   \param ulMode             Transfer Mode
*   \param pszFileName        Filename to upload
*   \param pabFileData        Pointer to buffer receiving upload
*   \param pulFileSize        [in]Length of buffer, [out] Bytes copied to buffer
*   \param pfnCallback        Callback pointer for progress
*                             (NULL for no callback)
*   \param pfnRecvPktCallback Callback pointer for unsolicited receive packets
*                             (NULL for no callback)
*   \param pvUser             User parameter on callback.
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelUpload(CIFXHANDLE            hChannel,
                                            uint32_t              ulMode,
                                            char*                 pszFileName,
                                            uint8_t*              pabFileData,
                                            uint32_t*             pulFileSize,
                                            PFN_PROGRESS_CALLBACK pfnCallback,
                                            PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                                            void*                 pvUser)
{
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;
  int32_t          lRet          = CIFX_NO_ERROR;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pszFileName);
  CHECK_POINTER(pabFileData);
  CHECK_POINTER(pulFileSize);

  switch(ulMode)
  {
    case DOWNLOAD_MODE_FIRMWARE:
    case DOWNLOAD_MODE_CONFIG:
    case DOWNLOAD_MODE_FILE:
      lRet = CIFX_MAKE_DEV_FUN(DEV_UploadFile)(
          ptChannel,
          ptChannel->ulChannelNumber,
          ptChannel->tToHostMbx.tCom.tBlock.ulElementSize,
          HIL_FILE_XFER_FILE,
          pszFileName,
          pulFileSize,
          pabFileData,
          CIFX_MAKE_DEV_FUN(DEV_TransferPacket),
          pfnCallback,
          pfnRecvPktCallback,
          pvUser);
      break;

    default:
      lRet = CIFX_INVALID_PARAMETER;
      break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Set DMA state of a communication channel
*   \param hChannel         Channel handle
*   \param ulCmd            Command CIFX_DMA_STATE_xxx
*   \param pulState         Return actual state on CIFX_GET_DMA_STATE
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelDMAState(CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t* pulState)
{
#ifdef CIFX_TOOLKIT_DMA

  int32_t          lRet          = CIFX_INVALID_PARAMETER;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance = NULL;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pulState);

  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  /* Only possible on PCI devices */
  if( !ptDevInstance->fPCICard)
    return CIFX_FUNCTION_NOT_AVAILABLE;

  /* Only possible if user provided DMA buffers */
  if(0 == ptDevInstance->ulDMABufferCount)
    return CIFX_DEV_DMA_INSUFF_BUFFER_COUNT;

  /* Check if firmware supports DMA functions */

  /*TODO: Check this */
  /*if( !ptChannel->ptCommunicationStatusBlock->ulCommunicationCOS->fPCICard) */
  /*  return CIFX_FUNCTION_NOT_AVAILABLE; */

  lRet = CIFX_MAKE_DEV_FUN(DEV_DMAState)( (PCHANNELINSTANCE)hChannel,
                                          ulCmd,
                                          pulState);

  return lRet;

#else

  UNREFERENCED_PARAMETER(hChannel);
  UNREFERENCED_PARAMETER(ulCmd);
  UNREFERENCED_PARAMETER(pulState);
  return CIFX_FUNCTION_NOT_AVAILABLE; /*lint !e438 : unused variables */

#endif
}

/*****************************************************************************/
/*! Register a callback notification
*   \param hChannel           Handle to the Channel
*   \param ulNotification     Notification
*   \param pfnCallback        Callback function
*   \param pvUser             User data pointer
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelRegisterNotification(CIFXHANDLE           hChannel,
                                                          uint32_t             ulNotification,
                                                          PFN_NOTIFY_CALLBACK  pfnCallback,
                                                          void*                pvUser)
{
  int32_t          lRet             = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel        = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE  ptDevInstance    = NULL;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pfnCallback);

  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  if(!ptDevInstance->fIrqEnabled)
    return CIFX_INTERRUPT_DISABLED;

  switch (ulNotification)
  {
    case CIFX_NOTIFY_RX_MBX_FULL:
      /* Check if already registered */
      if( NULL != ptChannel->tToHostMbx.tCom.tCtl.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        ptChannel->tToHostMbx.tCom.tCtl.pvUser      = pvUser;
        ptChannel->tToHostMbx.tCom.tCtl.pfnCallback = pfnCallback;

        if(CIFX_MAKE_DEV_FUN(DEV_WaitForMbxState)(ptChannel, &ptChannel->tToHostMbx.tCom, NETX_MBX_COM_STATE_FULL, 0))
        {
          CIFX_NOTIFY_RX_MBX_FULL_DATA_T tData = {
            .ulRecvCount = CIFX_MAKE_DEV_FUN(DEV_GetMBXFillLevel)(&ptChannel->tToHostMbx.tCom)
          };
          pfnCallback(CIFX_NOTIFY_RX_MBX_FULL, sizeof(tData), &tData, pvUser);
        }
      }
    break;

    case CIFX_NOTIFY_TX_MBX_EMPTY:
      /* Check if already registered */
      if( NULL != ptChannel->tFromHostMbx.tCom.tCtl.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        ptChannel->tFromHostMbx.tCom.tCtl.pvUser      = pvUser;
        ptChannel->tFromHostMbx.tCom.tCtl.pfnCallback = pfnCallback;

        if(CIFX_MAKE_DEV_FUN(DEV_WaitForMbxState)(ptChannel, &ptChannel->tFromHostMbx.tCom, NETX_MBX_COM_STATE_EMPTY, 0))
        {
          CIFX_NOTIFY_TX_MBX_EMPTY_DATA_T tData = {
            .ulMaxSendCount = ptChannel->tFromHostMbx.tCom.tBlock.ulElementCnt - CIFX_MAKE_DEV_FUN(DEV_GetMBXFillLevel)(&ptChannel->tFromHostMbx.tCom)
          };
          pfnCallback(CIFX_NOTIFY_TX_MBX_EMPTY, sizeof(tData), &tData, pvUser);
        }
      }
    break;

    case CIFX_NOTIFY_PD0_IN:
    case CIFX_NOTIFY_PD0_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[0];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD0_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[0];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have one area */
      if( 0 == ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL != ptBlock->tIoCtl.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        /* Add the callback */
        ptBlock->tIoCtl.pvUser      = pvUser;
        ptBlock->tIoCtl.pfnCallback = pfnCallback;

        if(CIFX_MAKE_DEV_FUN(DEV_WaitForIoBitState)(ptChannel, ptBlock, 0))
        {
          pfnCallback(ulNotification, 0, NULL, pvUser);
        }
      }
    }
    break;

    case CIFX_NOTIFY_PD1_IN:
    case CIFX_NOTIFY_PD1_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[1];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD1_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[1];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have two areas */
      if( 2 > ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL != ptBlock->tIoCtl.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        /* Add the callback */
        ptBlock->tIoCtl.pvUser      = pvUser;
        ptBlock->tIoCtl.pfnCallback = pfnCallback;

        if(CIFX_MAKE_DEV_FUN(DEV_WaitForIoBitState)(ptChannel, ptBlock, 0))
        {
          pfnCallback(ulNotification, 0, NULL, pvUser);
        }
      }
    }
    break;

    case CIFX_NOTIFY_PD2_IN:
    case CIFX_NOTIFY_PD2_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[2];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD2_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[2];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have three areas */
      if( 3 > ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL != ptBlock->tIoCtl.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        /* Add the callback */
        ptBlock->tIoCtl.pvUser      = pvUser;
        ptBlock->tIoCtl.pfnCallback = pfnCallback;

        if(CIFX_MAKE_DEV_FUN(DEV_WaitForIoBitState)(ptChannel, ptBlock, 0))
        {
          pfnCallback(ulNotification, 0, NULL, pvUser);
        }
      }
    }
    break;

    case CIFX_NOTIFY_PD3_IN:
    case CIFX_NOTIFY_PD3_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[3];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD3_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[3];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have four areas */
      if( 4 > ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL != ptBlock->tIoCtl.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        /* Add the callback */
        ptBlock->tIoCtl.pvUser      = pvUser;
        ptBlock->tIoCtl.pfnCallback = pfnCallback;

        if(CIFX_MAKE_DEV_FUN(DEV_WaitForIoBitState)(ptChannel, ptBlock, 0))
        {
          pfnCallback(ulNotification, 0, NULL, pvUser);
        }
      }
    }
    break;

    case CIFX_NOTIFY_SYNC:
      if( NULL != ptChannel->tSynch.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        /* Add the callback */
        uint8_t bState = HIL_FLAGS_NOT_EQUAL;

        ptChannel->tSynch.pvUser      = pvUser;
        ptChannel->tSynch.pfnCallback = pfnCallback;

#if 0 // TODO
        /* Add callback for sync on startup */
        if( HIL_SYNC_MODE_HST_CTRL == HWIF_READ8(ptDevInstance, ptChannel->ptCommunicationStatusBlock->bSyncHskMode))
          bState = HIL_FLAGS_EQUAL;
#endif

        if(CIFX_MAKE_DEV_FUN(DEV_WaitForSyncState)(ptChannel, bState, 0))
        {
          pfnCallback(ulNotification, 0, NULL, pvUser);
        }
      }
    break;

    case CIFX_NOTIFY_COM_STATE:
      /* Check if already registered */
      if( NULL != ptChannel->tComState.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_ALREADY_USED;
      } else
      {
        CIFX_NOTIFY_COM_STATE_T tData;

        ptChannel->tComState.pvUser      = pvUser;
        ptChannel->tComState.pfnCallback = pfnCallback;

        /* Just update the actual flag state by reading it once */
        (void)CIFX_MAKE_DEV_FUN(DEV_WaitForBitState)(
            ptChannel,
            NCF_COMMUNICATING_BIT_NO,
            HIL_FLAGS_SET,
            0);

        tData.ulComState = ptChannel->tHsCtrl.ulNetxFlags & NCF_COMMUNICATING;
        pfnCallback(CIFX_NOTIFY_COM_STATE, sizeof(tData), &tData, pvUser);
      }
    break;

    default:
      lRet = CIFX_INVALID_COMMAND;
    break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Unregister a callback notification
*   \param hChannel           Handle to the Channel
*   \param ulNotification     Notification
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelUnregisterNotification(CIFXHANDLE hChannel,
                                                            uint32_t   ulNotification)
{
  int32_t lRet = CIFX_NO_ERROR;

  PCHANNELINSTANCE ptChannel = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);

  switch (ulNotification)
  {
    case CIFX_NOTIFY_RX_MBX_FULL:
      /* Check if already registered */
      if( NULL == ptChannel->tToHostMbx.tCom.tCtl.pfnCallback)
      {
        /* Not registered */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        ptChannel->tToHostMbx.tCom.tCtl.pfnCallback = NULL;
      }
    break;

    case CIFX_NOTIFY_TX_MBX_EMPTY:
      /* Check if already registered */
      if( NULL == ptChannel->tFromHostMbx.tCom.tCtl.pfnCallback)
      {
        /* Already registered */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        ptChannel->tFromHostMbx.tCom.tCtl.pfnCallback = NULL;
      }
    break;

    case CIFX_NOTIFY_PD0_IN:
    case CIFX_NOTIFY_PD0_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[0];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD0_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[0];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have one area */
      if( 0 == ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL == ptBlock->tIoCtl.pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        ptBlock->tIoCtl.pfnCallback = NULL;
        ptBlock->tIoCtl.pvUser      = NULL;
        lRet = CIFX_NO_ERROR;
      }
    }
    break;

    case CIFX_NOTIFY_PD1_IN:
    case CIFX_NOTIFY_PD1_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[1];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD1_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[1];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have two areas */
      if( 2 > ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL == ptBlock->tIoCtl.pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        ptBlock->tIoCtl.pfnCallback = NULL;
        ptBlock->tIoCtl.pvUser      = NULL;
        lRet = CIFX_NO_ERROR;
      }
    }
    break;

    case CIFX_NOTIFY_PD2_IN:
    case CIFX_NOTIFY_PD2_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[2];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD2_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[2];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have three areas */
      if( 3 > ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL == ptBlock->tIoCtl.pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        ptBlock->tIoCtl.pfnCallback = NULL;
        ptBlock->tIoCtl.pvUser      = NULL;
        lRet = CIFX_NO_ERROR;
      }
    }
    break;

    case CIFX_NOTIFY_PD3_IN:
    case CIFX_NOTIFY_PD3_OUT:
    {
      NETX_IO_BLOCK_T* ptBlock     = ptChannel->tIoArea.aptIOInputAreas[3];
      uint32_t         ulAreaCount = ptChannel->tIoArea.ulIOInputAreas;

      if( CIFX_NOTIFY_PD3_OUT == ulNotification)
      {
        ptBlock     = ptChannel->tIoArea.aptIOOutputAreas[3];
        ulAreaCount = ptChannel->tIoArea.ulIOOutputAreas;
      }

      /* Check if we have four areas */
      if( 4 > ulAreaCount)
      {
        /* No area available */
        lRet = CIFX_INVALID_PARAMETER;

      } else if( NULL == ptBlock->tIoCtl.pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        ptBlock->tIoCtl.pfnCallback = NULL;
        ptBlock->tIoCtl.pvUser      = NULL;
        lRet = CIFX_NO_ERROR;
      }
    }
    break;

    case CIFX_NOTIFY_SYNC:
      if( NULL == ptChannel->tSynch.pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        /* Add the callback */
        ptChannel->tSynch.pfnCallback = NULL;
        ptChannel->tSynch.pvUser      = NULL;
      }
    break;

    case CIFX_NOTIFY_COM_STATE:
      if( NULL == ptChannel->tComState.pfnCallback)
      {
        /* Not registered before */
        lRet = CIFX_CALLBACK_NOT_REGISTERED;
      } else
      {
        /* delete the callback */
        ptChannel->tComState.pfnCallback = NULL;
        ptChannel->tComState.pvUser      = NULL;
      }
    break;

    default:
      lRet = CIFX_INVALID_COMMAND;
    break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Signal a sync state, either a sync command or acknowledge
*   \param hChannel           Handle to the Channel
*   \param ulCmd              Sync command
*   \param ulTimeout          Timeout to wait for sync / sync signalling
*   \param pulErrorCount      Actual sync error counter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxChannelSyncState( CIFXHANDLE  hChannel,
                                                uint32_t    ulCmd,
                                                uint32_t    ulTimeout,
                                                uint32_t*   pulErrorCount)
{
  int32_t           lRet      = CIFX_NO_ERROR;
  PCHANNELINSTANCE  ptChannel = (PCHANNELINSTANCE)hChannel;

  CHECK_CHANNELHANDLE(hChannel);
  CHECK_POINTER(pulErrorCount);

  /* Check if device installed and active */
  if(ptChannel->ulOpenCount == 0)
  {
    lRet = CIFX_DRV_CHANNEL_NOT_INITIALIZED;
  } else
  {
    switch (ulCmd)
    {
      case CIFX_SYNC_SIGNAL_CMD:
#if 0 // TODO
        /* Check if SYNC mode is host controlled */
        if(HIL_SYNC_MODE_HST_CTRL != HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->bSyncHskMode))
        {
          /* Invalid Device mode */
          lRet = CIFX_DEV_SYNC_STATE_INVALID_MODE;

        } else if(!DEV_WaitForSyncState(ptChannel, HIL_FLAGS_EQUAL, ulTimeout))
        {
          /* Sync cannot be signalled as bits are in wrong state */
          lRet = CIFX_DEV_SYNC_STATE_TIMEOUT;
        } else
        {
          /* Signal new sync */
          DEV_ToggleSyncBit( (PDEVICEINSTANCE)ptChannel->pvDeviceInstance, (1 << ptChannel->ulChannelNumber));

          /* Return actual error counter */
          *pulErrorCount = HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->bErrorSyncCnt);

          /* Check if the device is communication */
          (void)DEV_IsCommunicating(ptChannel, &lRet);
        }
#endif
      break;

      case CIFX_SYNC_ACKNOWLEDGE_CMD:
        /* Check if SYNC mode is device controlled */
#if 0 // TODO
        if(HIL_SYNC_MODE_DEV_CTRL != HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->bSyncHskMode))
        {
          /* Invalid Device mode */
          lRet = CIFX_DEV_SYNC_STATE_INVALID_MODE;

        } else if(!DEV_WaitForSyncState(ptChannel, HIL_FLAGS_NOT_EQUAL, ulTimeout))
        {
          /* Sync cannot be signalled as bits are in wrong state */
          lRet = CIFX_DEV_SYNC_STATE_TIMEOUT;
        } else
        {
          /* Acknowledge an device sys */
          DEV_ToggleSyncBit( (PDEVICEINSTANCE)ptChannel->pvDeviceInstance, (1 << ptChannel->ulChannelNumber));

          /* Return actual error counter */
          *pulErrorCount = HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->bErrorSyncCnt);

          /* Check if the device is communication */
          (void)DEV_IsCommunicating(ptChannel, &lRet);
        }
#endif
      break;

      case CIFX_SYNC_WAIT_CMD:
        {
#if 0 // TODO
          if( (HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->bSyncHskMode) != HIL_SYNC_MODE_HST_CTRL) &&
              (HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->bSyncHskMode) != HIL_SYNC_MODE_DEV_CTRL) )
          {
            /* Invalid Device mode */
            lRet = CIFX_DEV_SYNC_STATE_INVALID_MODE;
          } else
          {
            uint8_t bState = HIL_FLAGS_NOT_EQUAL;

            if( HIL_SYNC_MODE_HST_CTRL == HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->bSyncHskMode))
              bState = HIL_FLAGS_EQUAL;

            /* Wait for sync */
            if(!DEV_WaitForSyncState(ptChannel, bState, ulTimeout))
            {
              /* Sync timeout */
              lRet = CIFX_DEV_SYNC_STATE_TIMEOUT;
            } else
            {
              /* Return actual error counter */
              *pulErrorCount = HWIF_READ8(ptChannel->pvDeviceInstance, ptChannel->ptCommunicationStatusBlock->bErrorSyncCnt);

              /* Check if the device is communication */
              (void)DEV_IsCommunicating(ptChannel, &lRet);
            }
          }
#endif
        }
      break;

      default:
        lRet = CIFX_INVALID_COMMAND;
      break;
    }
  }

  return lRet;
}

#ifndef CIFX_TOOLKIT_USE_CUSTOM_DRV_FUNCS
/*****************************************************************************/
/*! Open a connection to the driver
*   \param phDriver     Returned handle to the driver
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
CIFX_STATIC int32_t APIENTRY HIFxDriverOpen(CIFXHANDLE* phDriver)
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
CIFX_STATIC int32_t APIENTRY HIFxDriverClose(CIFXHANDLE hDriver)
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
CIFX_STATIC int32_t APIENTRY HIFxDriverGetInformation(CIFXHANDLE hDriver, uint32_t ulSize, void* pvDriverInfo)
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
CIFX_STATIC int32_t APIENTRY HIFxDriverGetErrorDescription(int32_t lError, char* szBuffer, uint32_t ulBufferLen)
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
CIFX_STATIC int32_t APIENTRY HIFxDriverEnumBoards(CIFXHANDLE hDriver, uint32_t ulBoard, uint32_t ulSize, void* pvBoardInfo)
{
  BOARD_INFORMATION*        ptBoardInfo   = (BOARD_INFORMATION*)pvBoardInfo;
  PDEVICEINSTANCE           ptDevInstance = NULL;
  HIL_HIF_SYSTEM_CHANNEL_T* ptSysChannel  = NULL;

  if(g_tDriverInfo.ulOpenCount == 0)
    return CIFX_DRV_NOT_OPENED;

  CHECK_DRIVERHANDLE(hDriver);
  CHECK_POINTER(pvBoardInfo);

  if(ulSize < (uint32_t)sizeof(*ptBoardInfo))
    return CIFX_INVALID_BUFFERSIZE;

  if(ulBoard >= g_ulDeviceCount)
    return CIFX_NO_MORE_ENTRIES;

  ptDevInstance = g_pptDevices[ulBoard];
  ptSysChannel  = (HIL_HIF_SYSTEM_CHANNEL_T*)ptDevInstance->pbDPM;

  (void)OS_Strncpy(ptBoardInfo->abBoardName,  ptDevInstance->szName,  sizeof(ptBoardInfo->abBoardName));
  (void)OS_Strncpy(ptBoardInfo->abBoardAlias, ptDevInstance->szAlias, sizeof(ptBoardInfo->abBoardAlias));
  ptBoardInfo->ulBoardID         = ulBoard;

  {
    ptBoardInfo->ulSystemError     = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptSysChannel->tSystemState.ulSystemError));

    ptBoardInfo->ulPhysicalAddress = ptDevInstance->ulPhysicalAddress;
    ptBoardInfo->ulIrqNumber       = ptDevInstance->ulIrqNumber;
    ptBoardInfo->bIrqEnabled       = ptDevInstance->fIrqEnabled? 1 : 0;
    ptBoardInfo->ulDpmTotalSize    = ptDevInstance->ulDPMSize;
    ptBoardInfo->ulChannelCnt      = ptDevInstance->ulCommChannelCount;

    HWIF_READN(ptDevInstance, &ptBoardInfo->tSystemInfo, &ptSysChannel->tSystemInfo, sizeof(ptBoardInfo->tSystemInfo));
  }

  (void)cifXConvertEndianess(0,
                             &ptBoardInfo->tSystemInfo,
                             sizeof(ptBoardInfo->tSystemInfo),
                             s_atSystemInfoBlock,
                             HIL_CNT_ELEMENT(s_atSystemInfoBlock));

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
CIFX_STATIC int32_t APIENTRY HIFxDriverEnumChannels(CIFXHANDLE  hDriver, uint32_t ulBoard, uint32_t ulChannel, uint32_t ulSize, void* pvChannelInfo)
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
  lRet = CIFX_MAKE_CIFX_FUN(xChannelInfo)(ptChannel, ulSize, ptChannelInfo);

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
CIFX_STATIC int32_t APIENTRY HIFxDriverMemoryPointer(CIFXHANDLE hDriver, uint32_t ulBoard, uint32_t ulCmd, void* pvMemoryInfo)
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
          void* pvMappedDPM          = NULL;

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
              uint32_t         ulOffset  = (uint32_t)(ptChannel->pbDPMChannelStart - ptDevInstance->pbDPM);

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

#ifdef CIFX_TOOLKIT_FUNCTION_LIST

/*****************************************************************************/
/*! Local structure for cifX API function pointers                           */
/*****************************************************************************/
static CIFX_API_FUNCTION_LIST_T s_tCifxHifApiFuns =
{
  HIFxDriverOpen,
  HIFxDriverClose,
  HIFxDriverGetInformation,
  HIFxDriverGetErrorDescription,
  HIFxDriverEnumBoards,
  HIFxDriverEnumChannels,
  HIFxDriverMemoryPointer,
  NULL, /* OS specific, implemented by user. */
  HIFxSysdeviceOpen,
  HIFxSysdeviceClose,
  HIFxSysdeviceGetMBXState,
  HIFxSysdevicePutPacket,
  HIFxSysdeviceGetPacket,
  HIFxSysdeviceInfo,
  HIFxSysdeviceFindFirstFile,
  HIFxSysdeviceFindNextFile,
  HIFxSysdeviceDownload,
  HIFxSysdeviceUpload,
  HIFxSysdeviceReset,
  HIFxSysdeviceResetEx,
  HIFxSysdeviceBootstart,
  HIFxSysdeviceExtendedMemory,
  HIFxChannelOpen,
  HIFxChannelClose,
  HIFxChannelFindFirstFile,
  HIFxChannelFindNextFile,
  HIFxChannelDownload,
  HIFxChannelUpload,
  HIFxChannelGetMBXState,
  HIFxChannelPutPacket,
  HIFxChannelGetPacket,
  HIFxChannelGetSendPacket,
  HIFxChannelConfigLock,
  HIFxChannelReset,
  HIFxChannelInfo,
  HIFxChannelWatchdog,
  HIFxChannelHostState,
  HIFxChannelBusState,
  HIFxChannelDMAState,
  HIFxChannelIOInfo,
  HIFxChannelIORead,
  HIFxChannelIOWrite,
  HIFxChannelIOReadSendData,
  HIFxChannelControlBlock,
  HIFxChannelCommonStatusBlock,
  HIFxChannelExtendedStatusBlock,
  HIFxChannelUserBlock,
  HIFxChannelPLCMemoryPtr,
  HIFxChannelPLCIsReadReady,
  HIFxChannelPLCIsWriteReady,
  HIFxChannelPLCActivateWrite,
  HIFxChannelPLCActivateRead,
  HIFxChannelRegisterNotification,
  HIFxChannelUnregisterNotification,
  HIFxChannelSyncState,
};

PCIFX_API_FUNCTION_LIST_T cifXTkitGetHifApiFunctionList(void)
{
  return &s_tCifxHifApiFuns;
}

#endif

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/
