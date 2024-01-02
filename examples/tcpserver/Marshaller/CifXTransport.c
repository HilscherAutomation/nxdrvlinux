/**************************************************************************************

   Copyright (c) Hilscher GmbH. All Rights Reserved.

 **************************************************************************************

   Filename:
    $Workfile: CifXTransport.c $
   Last Modification:
    $Author: AMinor $
    $Modtime: $
    $Revision: 13819 $

   Targets:
     Win32/ANSI   : yes
     Win32/Unicode: yes (define _UNICODE)
     WinCE        : yes

   Description:
    Implementation of Marshaller function call interpretation

   Changes:

     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
     13        12.11.2020  RMA      Changing general functions to OS_* implementation
     12        12.12.2019  LCO      Restructure handling for devices without Communication Channels
                                    in cifXTransportInit()
     11        05.11.2019  ALM      Remove TLR references
     10        21.04.2019  ALM      Added support for xSysdeviceRebootEx()
     9         21.07.2015  LCO      Fixed compiler warnings created by implicit data conversion
                                    on 64Bit systems
     8         16.07.2015  RM       Change:
                                     - Adapted to new APIHeader directory and MarshallerFrame.h
     7         30.06.2014  SS       Access to closed channel instance may cause system crash
     6         26.09.2013  SS       Added support for xDriverRestartDevice call
     5         28.06.2010  SD       Change:
                                     - new types for 64-bit support
     4         19.08.2009  MS       Corrected destination addresses in confirmation data copy operations for
                                     - xChannelFindFirstFile / xChannelFindNextFile
                                     - xSysdeviceFindFirstFile / xSysdeviceFindNextFile
     3         18.08.2009  MS       Initialize data buffer tChannelInfo before calling xDriverEnumChannels().
     2         08.07.2009  MT       Added functions:
                                     - xChannelIOInfo
                                     - xChannelFindFirstFile / xChannelFindNextFile
                                     - xSysdeviceFindFirstFile / xSysdeviceFindNextFile
     1         26.05.2009  PL       intitial version

**************************************************************************************/

/*****************************************************************************/
/*! \file CifXTransport.c
*   cifX marshalling via Hilscher Transport Protocol                                                                  */
/*****************************************************************************/

#include "OS_Dependent.h"
#include "CifXTransport.h"
#include "MarshallerFrame.h"
#include "MarshallerInternal.h"
#include "cifXUser.h"
#include "cifXErrors.h"

#ifndef min
  #define min(a,b) ((a > b) ? b : a)
#endif


#define  HANDLE_IS_VALID(a)        (0 != (a & MSK_MARSHALLER_HANDLE_VALID))
#define  HANDLE_GET_OBJSUBIDX(a)   ((a & MSK_MARSHALLER_HANDLE_OBJECTSUBIDX) >> SRT_MARSHALLER_HANDLE_OBJECTSUBIDX)
#define  HANDLE_GET_OBJIDX(a)      ((a & MSK_MARSHALLER_HANDLE_OBJECTIDX) >> SRT_MARSHALLER_HANDLE_OBJECTIDX)
#define  HANDLE_GET_OBJTYPE(a)     ((a & MSK_MARSHALLER_HANDLE_OBJECTTYPE) >> SRT_MARSHALLER_HANDLE_OBJECTTYPE)
#define  SEQ_GET_REQUEST(a)        ((a & MSK_MARSHALLER_SEQUENCE_REQUEST) >> SRT_MARSHALLER_SEQUENCE_REQUEST)
#define  SEQ_GET_SUPPORTED(a)      ((a & MSK_MARSHALLER_SEQUENCE_SUPPORTED) >> SRT_MARSHALLER_SEQUENCE_SUPPORTED)
#define  SEQ_GET_NUMBER(a)         ((a & MSK_MARSHALLER_SEQUENCE_NUMBER) >> SRT_MARSHALLER_SEQUENCE_NUMBER)

#define  HANDLE_SET_VALID(a)       (a |= ((uint32_t)1 << SRT_MARSHALLER_HANDLE_VALID))
#define  HANDLE_SET_OBJSUBIDX(a,b) (a |= ((uint32_t)b << SRT_MARSHALLER_HANDLE_OBJECTSUBIDX))
#define  HANDLE_SET_OBJIDX(a,b)    (a |= ((uint32_t)b << SRT_MARSHALLER_HANDLE_OBJECTIDX))
#define  HANDLE_SET_OBJTYPE(a,b)   (a |= ((uint32_t)b << SRT_MARSHALLER_HANDLE_OBJECTTYPE))
#define  SEQ_SET_REQUEST(a)        (a |= ((uint32_t)1 << SRT_MARSHALLER_SEQUENCE_REQUEST))
#define  SEQ_SET_SUPPORTED(a)      (a |= ((uint32_t)1 << SRT_MARSHALLER_SEQUENCE_SUPPORTED))
#define  SEQ_SET_RESPONSE(a)       (a &= ~((uint32_t)1 << SRT_MARSHALLER_SEQUENCE_REQUEST))

/*****************************************************************************/
/*! \addtogroup NETX_MARSHALLER_CIFX
*   \{                                                                       */
/*****************************************************************************/

/*****************************************************************************/
/*! Data for a channel object                                                */
/*****************************************************************************/
typedef struct MARSH_CIFX_CHANNEL_DATA_Ttag
{
  CIFXHANDLE    hChannel;   /*!< Handle returned from xChannelOpen       */
  unsigned long ulOpenCnt;  /*!< Number of times xChannelOpen was called */
  int           fValid;

} MARSH_CIFX_CHANNEL_DATA_T, *PMARSH_CIFX_CHANNEL_DATA_T;

/*****************************************************************************/
/*! Data for a board/device object                                                                                               */
/*****************************************************************************/
typedef struct MARSH_CIFX_DEVICE_DATA_Ttag
{
  int                         fValid;         /*!< !=0 if the device is valid                                     */
  unsigned long               ulBoard;        /*!< Board number (Idx in our array)                        */
  BOARD_INFORMATION           tBoardInfo;     /*!< cifX Board information                                      */
  MARSH_CIFX_CHANNEL_DATA_T   tSysDevice;     /*!< System device object                                          */
  PMARSH_CIFX_CHANNEL_DATA_T  ptChannels;     /*!< Array of channel objects                                    */
  unsigned long               ulChannelCount; /*!< Number of entries in channel objects array     */
} MARSH_CIFX_DEVICE_DATA_T, *PMARSH_CIFX_DEVICE_DATA_T;


/*****************************************************************************/
/*! Structure to handle more than one instance of marshaller                                                    */
/*****************************************************************************/
typedef struct MARSH_HANDLE_INSTANCE_Ttag
{
  unsigned long             ulDriverOpenCnt;    /*!< Number of times the driver was opened via Marshaller Calls         */
  unsigned long             ulDeviceCnt;        /*!< Number of known devices/board, initialized in MarshallerInit     */
  CIFXHANDLE                hDriver;            /*!< Driver handle                                                                                       */
  DRIVER_FUNCTIONS          tDRVFunctions;      /*!  Function list of CifX API   */
  PMARSH_CIFX_DEVICE_DATA_T ptDevices;          /*!< List of available devices, initialized in MarshallerInit                   */

} MARSH_HANDLE_INSTANCE_T, *PMARSH_HANDLE_INSTANCE_T;

static int32_t HandleClassfactoryCommand(HIL_MARSHALLER_BUFFER_T* ptBuffer);
static int32_t HandleDriverCommand      (HIL_MARSHALLER_BUFFER_T* ptBuffer, void* pvUser);
static int32_t HandleSysdeviceCommand   (HIL_MARSHALLER_BUFFER_T* ptBuffer, void* pvUser);
static int32_t HandleChannelCommand     (HIL_MARSHALLER_BUFFER_T* ptBuffer, void* pvUser);

static PMARSH_CIFX_DEVICE_DATA_T FindDevice(char* szBoard, void* pvUser);

static void HilMarshallerDeInitModul(void* pvUser);
static void HilMarshallerHandlePacket(void* pvMarshaller, HIL_MARSHALLER_BUFFER_T* ptBuffer, void* pvUser);

/*****************************************************************************/
/*! Initialize cifX API transport layer
*   \param pvMarshaller     Marshaller handle
*   \param pvConfig         cifX API specific configuration data
*   \return HIL_MARSHALLER_E_SUCCESS on success                              */
/*****************************************************************************/
uint32_t cifXTransportInit(void* pvMarshaller, void* pvConfig)
{
  uint32_t                 eRet = HIL_MARSHALLER_E_SUCCESS;
  PMARSH_HANDLE_INSTANCE_T ptInstance;

  /* Invalid parameter*/
  if( (!pvMarshaller) || (!pvConfig))
    return HIL_MARSHALLER_E_INVALIDPARAMETER;

  if(NULL == (ptInstance = OS_Malloc(sizeof(MARSH_HANDLE_INSTANCE_T))))
  {
    eRet = HIL_MARSHALLER_E_OUTOFMEMORY;
  } else
  {
    TRANSPORT_LAYER_DATA_T  tLayerData  = {0};
    CIFX_TRANSPORT_CONFIG*  ptConfig    = (CIFX_TRANSPORT_CONFIG*)pvConfig;
    DRIVER_INFORMATION      tDriverInfo;

    OS_Memset(ptInstance, 0, sizeof(*ptInstance));

    tLayerData.usDataType = HIL_TRANSPORT_TYPE_MARSHALLER;
    tLayerData.pfnHandler = HilMarshallerHandlePacket;
    tLayerData.pfnDeinit  = HilMarshallerDeInitModul;
    tLayerData.pvUser     = ptInstance;
    tLayerData.pfnPoll    = NULL;

    ptInstance->tDRVFunctions = ptConfig->tDRVFunctions;

    if( (CIFX_NO_ERROR == ptInstance->tDRVFunctions.pfnxDriverOpen(&ptInstance->hDriver)) &&
        (CIFX_NO_ERROR == ptInstance->tDRVFunctions.pfnxDriverGetInformation(ptInstance->hDriver, sizeof(tDriverInfo), &tDriverInfo)) )
    {
      if(0 != tDriverInfo.ulBoardCnt)
      {
        if(0 != (ptInstance->ptDevices = OS_Malloc((uint32_t)sizeof(*ptInstance->ptDevices) * tDriverInfo.ulBoardCnt)))
        {
          uint32_t                  ulBoard;
          PMARSH_CIFX_DEVICE_DATA_T ptDevice = ptInstance->ptDevices;

          OS_Memset(ptInstance->ptDevices, 0, sizeof(*ptInstance->ptDevices) * tDriverInfo.ulBoardCnt);
          ptInstance->ulDeviceCnt = tDriverInfo.ulBoardCnt;

          for(ulBoard = 0; ulBoard < tDriverInfo.ulBoardCnt; ulBoard++)
          {
            if(CIFX_NO_ERROR == ptInstance->tDRVFunctions.pfnxDriverEnumBoards(ptInstance->hDriver, ulBoard, sizeof(ptDevice->tBoardInfo), &ptDevice->tBoardInfo))
            {
              ptDevice->ulChannelCount = ptDevice->tBoardInfo.ulChannelCnt;

              if (0 == ptDevice->ulChannelCount)
              {
                /* Device has only system channel */
                ptDevice->ulBoard = ulBoard;
                ptDevice->fValid = 1;
              }
              else
              {
                /* Setup internals for device with Communication Channel(s) */
                uint32_t                   ulChannel = 0;
                PMARSH_CIFX_CHANNEL_DATA_T ptChannelData;

                ptDevice->ptChannels = OS_Malloc((uint32_t)(sizeof(*ptDevice->ptChannels) * ptDevice->ulChannelCount));

                /* Check returned pointer */
                if(NULL == ptDevice->ptChannels)
                {
                  eRet = HIL_MARSHALLER_E_OUTOFMEMORY;
                  break;
                }

                /* Memory available, continue with internal device setup */
                OS_Memset(ptDevice->ptChannels, 0, sizeof(*ptDevice->ptChannels) * ptDevice->ulChannelCount);

                ptDevice->ulBoard = ulBoard;
                ptDevice->fValid  = 1;

                ptChannelData = ptDevice->ptChannels;

                for(ulChannel = 0; ulChannel < ptDevice->ulChannelCount; ulChannel++)
                {
                  CHANNEL_INFORMATION tChannelInfo;
                  if(CIFX_NO_ERROR == (ptInstance->tDRVFunctions.pfnxDriverEnumChannels(ptInstance->hDriver, ulBoard, ulChannel, sizeof(tChannelInfo), &tChannelInfo)))
                  {
                    ptChannelData->fValid = 1;
                  }
                  ptChannelData++;
                }
              }
            }
            ptDevice++;
          }
        }
      }

      eRet = HilMarshallerRegisterTransport(pvMarshaller, &tLayerData);

    } else
    {
      eRet = (uint32_t)CIFX_DRV_DRIVER_NOT_LOADED;
    }

    if(HIL_MARSHALLER_E_SUCCESS != eRet)
    {
      HilMarshallerDeInitModul(ptInstance);
    }
  }

  return eRet;
}


/*****************************************************************************/
/*! De-Initialize Marshaller
*   \param pvUser Transport data                                             */
/*****************************************************************************/
void HilMarshallerDeInitModul( void* pvUser )
{
  PMARSH_HANDLE_INSTANCE_T ptInstance = (PMARSH_HANDLE_INSTANCE_T) pvUser;
  uint32_t                 ulDevice;

  if(NULL != ptInstance)
  {
    /* Check, that all structures are cleaned */
    for(ulDevice = 0; ulDevice < ptInstance->ulDeviceCnt; ++ulDevice)
    {
      PMARSH_CIFX_DEVICE_DATA_T ptDevice = &ptInstance->ptDevices[ulDevice];
      uint32_t ulChannel;

      for(ulChannel = 0; ulChannel < ptDevice->ulChannelCount; ++ulChannel)
      {
        PMARSH_CIFX_CHANNEL_DATA_T ptChannel = (PMARSH_CIFX_CHANNEL_DATA_T)&ptDevice->ptChannels[ulChannel];

        if(ptChannel->hChannel)
          ptInstance->tDRVFunctions.pfnxChannelClose(ptChannel->hChannel);
      }

      if(NULL != ptDevice->ptChannels)
        OS_Free(ptDevice->ptChannels);
    }

    if(NULL != ptInstance->ptDevices)
      OS_Free(ptInstance->ptDevices);

    if(ptInstance->hDriver)
    {
      ptInstance->tDRVFunctions.pfnxDriverClose(ptInstance->hDriver);
      ptInstance->hDriver = 0;
    }
    /* Clean structure for the instance */
    OS_Free(ptInstance);
  }
}

/*****************************************************************************/
/*! Handle incoming packets
*   \param pvMarshaller Marshaller instance this packet was received from
*   \param ptBuffer     Pointer receive buffer (also used for response packet)
*   \param pvUser       Pointer to cifX Internal instance data
*   \return MARSHALLER_NO_ERROR on success                                                                    */
/*****************************************************************************/
void HilMarshallerHandlePacket( void* pvMarshaller, HIL_MARSHALLER_BUFFER_T* ptBuffer, void* pvUser )
{
  PMARSHALLER_DATA_FRAME_HEADER_T ptMarshallerHeader  = (PMARSHALLER_DATA_FRAME_HEADER_T) &ptBuffer->abData[0];
  int32_t                         lRet;

  /* Check ClassFactory */
  if( (ptMarshallerHeader->ulHandle == 0) ||
      ((HANDLE_GET_OBJTYPE(ptMarshallerHeader->ulHandle) == MARSHALLER_OBJECT_TYPE_CLASSFACTORY)
       && (HANDLE_IS_VALID(ptMarshallerHeader->ulHandle))))
  {
    /* This is a request for the Classfactory */
    lRet = HandleClassfactoryCommand(ptBuffer);

  }else if(!HANDLE_IS_VALID(ptMarshallerHeader->ulHandle))
  {
    /* This is not a valid handle, so reject */
    lRet = CIFX_INVALID_HANDLE;
    ptMarshallerHeader->ulDataSize = 0;

  }else
  {
    /* Check object type */
    switch(HANDLE_GET_OBJTYPE(ptMarshallerHeader->ulHandle))
    {
      case MARSHALLER_OBJECT_TYPE_DRIVER:
        lRet = HandleDriverCommand(ptBuffer, pvUser);
        break;

      case MARSHALLER_OBJECT_TYPE_SYSDEVICE:
        lRet = HandleSysdeviceCommand(ptBuffer, pvUser);
        break;

      case MARSHALLER_OBJECT_TYPE_CHANNEL:
        lRet = HandleChannelCommand(ptBuffer, pvUser);
        break;

      default:
        /* Unsupported Object Type */
        lRet = CIFX_INVALID_HANDLE;
        ptMarshallerHeader->ulDataSize = 0;
        break;
    }
  }

  /* Enter marshaller return code into answer */
  ptBuffer->tMgmt.ulUsedDataBufferLen                = ptMarshallerHeader->ulDataSize + (uint32_t)sizeof(*ptMarshallerHeader);
  ptMarshallerHeader->ulError                        = lRet;
  SEQ_SET_RESPONSE(ptMarshallerHeader->ulSequence);
  ptMarshallerHeader->ulSequence =  0;

  /* Call send function, to send a answer  */
  HilMarshallerConnTxData(pvMarshaller, ptBuffer->tMgmt.ulConnectorIdx , ptBuffer);
}

/*****************************************************************************/
/*! Handle  packets which includes a class factory command
*   \param ptBuffer  Reference to the give marshaller packet
                     Note: The given reference is also used to save the confirmation
*   \return MARSHALLER_NO_ERROR  on success                                  */
/*****************************************************************************/
static int32_t HandleClassfactoryCommand ( HIL_MARSHALLER_BUFFER_T* ptBuffer)
{
  int32_t                         lRet;
  PMARSHALLER_DATA_FRAME_HEADER_T ptMarshallerHeader  = (PMARSHALLER_DATA_FRAME_HEADER_T) &ptBuffer->abData[0];

  switch(ptMarshallerHeader->ulMethodID)
  {
    case MARSHALLER_CF_METHODID_SERVERVERSION:
    {
      PCF_SERVERVERSION_CNF_T ptServerVersionCnf = (PCF_SERVERVERSION_CNF_T)ptMarshallerHeader;

      /* Check Size of buffer */
      if( (ptBuffer->tMgmt.ulDataBufferLen - sizeof(MARSHALLER_DATA_FRAME_HEADER_T)) >= sizeof(ptServerVersionCnf->tData))
      {
        /* ATTENTION: Version must be set to 0.900 to allow the actual TCP/IP Driver to work.
                      This version number may change in future*/
        ptServerVersionCnf->tData.ulVersion     = 0x00090000;
        ptServerVersionCnf->tHeader.ulDataSize  = sizeof(CF_SERVERVERSION_CNF_DATA_T);
        lRet                                    = CIFX_NO_ERROR;
      }else
      {
        lRet = CIFX_INVALID_PARAMETER;
        ptMarshallerHeader->ulDataSize = 0;
      }
    }
    break;

  case MARSHALLER_CF_METHODID_CREATEINSTANCE:
    {
      PCF_CREATEINSTANCE_REQ_T ptCreateInstReq = (PCF_CREATEINSTANCE_REQ_T)ptMarshallerHeader;
      PCF_CREATEINSTANCE_CNF_T ptCreateInstCnf = (PCF_CREATEINSTANCE_CNF_T)ptMarshallerHeader;

      if(ptMarshallerHeader->ulDataSize != sizeof(ptCreateInstReq->tData))
      {
        lRet = CIFX_INVALID_PARAMETER;
        ptMarshallerHeader->ulDataSize = 0;
      } else
      {
        switch(ptCreateInstReq->tData.ulObjectType)
        {
        case MARSHALLER_OBJECT_TYPE_CLASSFACTORY:
          if( (ptBuffer->tMgmt.ulDataBufferLen - sizeof(MARSHALLER_DATA_FRAME_HEADER_T)) >= sizeof(ptCreateInstCnf->tData))
          {
            ptCreateInstCnf->tData.ulHandle                       = 0;
            HANDLE_SET_OBJTYPE(ptCreateInstCnf->tData.ulHandle, MARSHALLER_OBJECT_TYPE_CLASSFACTORY);
            HANDLE_SET_VALID(ptCreateInstCnf->tData.ulHandle);
            ptCreateInstCnf->tHeader.ulDataSize                   = sizeof(ptCreateInstCnf->tData);
            lRet                                                  = CIFX_NO_ERROR;
          }else
          {
            lRet = CIFX_INVALID_PARAMETER;
            ptMarshallerHeader->ulDataSize = 0;
          }
          break;

        case MARSHALLER_OBJECT_TYPE_DRIVER:
          if( (ptBuffer->tMgmt.ulDataBufferLen - sizeof(MARSHALLER_DATA_FRAME_HEADER_T)) >= sizeof(ptCreateInstCnf->tData))
          {
            ptCreateInstCnf->tData.ulHandle = 0;
            HANDLE_SET_OBJTYPE(ptCreateInstCnf->tData.ulHandle, MARSHALLER_OBJECT_TYPE_DRIVER);
            HANDLE_SET_VALID(ptCreateInstCnf->tData.ulHandle);
            ptCreateInstCnf->tHeader.ulDataSize                  = sizeof(ptCreateInstCnf->tData);
            lRet = CIFX_NO_ERROR;
          } else
          {
            lRet = CIFX_INVALID_PARAMETER;
            ptMarshallerHeader->ulDataSize = 0;
          }
          break;

        default:
          lRet = CIFX_INVALID_PARAMETER;
          ptMarshallerHeader->ulDataSize = 0;
          break;
        }
      }
    }
    break;

  default:
    lRet = CIFX_INVALID_PARAMETER;
    ptMarshallerHeader->ulDataSize = 0;
    break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Handle packets which includes a driver command
*   \param ptBuffer  Reference to the received packet
*                    Note: The given memory of the packet is also
*                          used to save the confimation
*   \param pvUser       Pointer to cifX Internal instance data
*   \return MARSHALLER_NO_ERROR  on success                                  */
/*****************************************************************************/
static int32_t HandleDriverCommand ( HIL_MARSHALLER_BUFFER_T* ptBuffer, void* pvUser)
{
  int32_t                         lRet;
  PMARSHALLER_DATA_FRAME_HEADER_T ptMarshallerHeader  = (PMARSHALLER_DATA_FRAME_HEADER_T) &ptBuffer->abData[0];
  PMARSH_HANDLE_INSTANCE_T        ptInstance          = (PMARSH_HANDLE_INSTANCE_T) pvUser;

  switch(ptMarshallerHeader->ulMethodID)
  {
  /***************************************************************************
  * xDriverOpen
  ***************************************************************************/
  case MARSHALLER_DRV_METHODID_OPEN:
    {
      ptMarshallerHeader->ulDataSize = 0;
      if(NULL != ptInstance->hDriver)
      {
        ++ptInstance->ulDriverOpenCnt;
        lRet = CIFX_NO_ERROR;
      } else
      {
        lRet = CIFX_DRV_DRIVER_NOT_LOADED;
      }
    }
    break;

  /***************************************************************************
  * xDriverClose
  ***************************************************************************/
  case MARSHALLER_DRV_METHODID_CLOSE:
      ptMarshallerHeader->ulDataSize = 0;

      if(0 == ptInstance->ulDriverOpenCnt)
      {
        lRet = CIFX_DRV_NOT_OPENED;
      } else
      {
        --ptInstance->ulDriverOpenCnt;
        lRet = CIFX_NO_ERROR;
      }
    break;

  /***************************************************************************
  * xDriverGetInformation
  ***************************************************************************/
  case MARSHALLER_DRV_METHODID_GETINFO:
    {
      PDRV_GETINFORMATION_REQ_T ptReq = (PDRV_GETINFORMATION_REQ_T)(ptMarshallerHeader);
      DRIVER_INFORMATION        tDriverInfo;

      if(!ptInstance->tDRVFunctions.pfnxDriverGetInformation)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;
      } else
      {
        lRet = ptInstance->tDRVFunctions.pfnxDriverGetInformation(ptInstance->hDriver, sizeof(tDriverInfo), &tDriverInfo);
        if(CIFX_NO_ERROR == lRet)
        {
          uint32_t ulCopyLen = min(sizeof(tDriverInfo), ptReq->tData.ulSize);

          if( (ptBuffer->tMgmt.ulDataBufferLen - sizeof(MARSHALLER_DATA_FRAME_HEADER_T)) >= ulCopyLen )
          {
            OS_Memset(&ptReq->tData, 0, sizeof(tDriverInfo));
            OS_Memcpy(&ptReq->tData, &tDriverInfo, ulCopyLen);
            ptReq->tHeader.ulDataSize = ulCopyLen;
          }else
          {
            lRet = CIFX_INVALID_PARAMETER;
            ptMarshallerHeader->ulDataSize = 0;
          }
        }
      }
    }
    break;

  /***************************************************************************
  * xDriverGetErrorDescription
  ***************************************************************************/
  case MARSHALLER_DRV_METHODID_ERRORDESCR:
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
    ptMarshallerHeader->ulDataSize = 0;
    break;

  /***************************************************************************
  * xDriverEnumBoards
  ***************************************************************************/
  case MARSHALLER_DRV_METHODID_ENUMBOARDS:
    {
      PDRV_ENUMBOARD_REQ_T ptEnumReq  = (PDRV_ENUMBOARD_REQ_T)(ptMarshallerHeader);
      uint32_t             ulDataSize = ptMarshallerHeader->ulDataSize;

      ptMarshallerHeader->ulDataSize = 0;

      if(ulDataSize != sizeof(ptEnumReq->tData))
      {
        lRet = CIFX_INVALID_PARAMETER;

      } else if(!ptInstance->tDRVFunctions.pfnxDriverEnumBoards)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;

      } else
      {
        BOARD_INFORMATION tBoardInfo = {0};

        lRet = ptInstance->tDRVFunctions.pfnxDriverEnumBoards(ptInstance->hDriver, ptEnumReq->tData.ulBoard, sizeof(tBoardInfo), &tBoardInfo);

        if(lRet == CIFX_NO_ERROR)
        {
          uint32_t ulCopyLen = min(ptEnumReq->tData.ulSize, sizeof(tBoardInfo));
          if( (ptBuffer->tMgmt.ulDataBufferLen - sizeof(MARSHALLER_DATA_FRAME_HEADER_T)) >= ulCopyLen )
          {
            OS_Memset(&ptEnumReq->tData, 0, sizeof(tBoardInfo));
            OS_Memcpy(&ptEnumReq->tData, &tBoardInfo, ulCopyLen);
            ptEnumReq->tHeader.ulDataSize = ulCopyLen;

          }else
          {
            lRet = CIFX_BUFFER_TOO_SHORT;
          }
        }
      }
    }
    break;

  /***************************************************************************
  * xDriverEnumChannels
  ***************************************************************************/
  case MARSHALLER_DRV_METHODID_ENUMCHANNELS:
    {
      PDRV_ENUMCHANNELS_REQ_T ptEnumReq  = (PDRV_ENUMCHANNELS_REQ_T)(ptMarshallerHeader);
      uint32_t                ulDataSize = ptMarshallerHeader->ulDataSize;

      ptMarshallerHeader->ulDataSize = 0;

      if(ulDataSize != sizeof(ptEnumReq->tData))
      {
        lRet = CIFX_INVALID_PARAMETER;
      } else if(!ptInstance->tDRVFunctions.pfnxDriverEnumChannels)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
      } else
      {
        CHANNEL_INFORMATION tChannelInfo;

        OS_Memset(&tChannelInfo, 0, sizeof(tChannelInfo));
        lRet = ptInstance->tDRVFunctions.pfnxDriverEnumChannels(ptInstance->hDriver,
                                                                                    ptEnumReq->tData.ulBoard,
                                                                                    ptEnumReq->tData.ulChannel,
                                                                                    sizeof(tChannelInfo),
                                                                                    &tChannelInfo);

        if(lRet == CIFX_NO_ERROR)
        {
          uint32_t ulCopyLen = min(ptEnumReq->tData.ulSize, sizeof(tChannelInfo));
          if( (ptBuffer->tMgmt.ulDataBufferLen - sizeof(MARSHALLER_DATA_FRAME_HEADER_T)) >= ulCopyLen )
          {
            OS_Memcpy(&ptEnumReq->tData, &tChannelInfo, ulCopyLen);
            ptEnumReq->tHeader.ulDataSize = ulCopyLen;
          }else
          {
            lRet = CIFX_BUFFER_TOO_SHORT;
          }
        }
      }
    }
    break;

  /***************************************************************************
  * xChannelOpen
  ***************************************************************************/
  case MARSHALLER_DRV_METHODID_OPENCHANNEL:
    {
      uint32_t                  ulBoardNameLen;
      char                      szBoardName[CIFx_MAX_INFO_NAME_LENTH] ={0};
      uint32_t                  ulChannelNr;
      uint8_t*                  pbRequestData = (uint8_t*)(ptMarshallerHeader + 1);
      PMARSH_CIFX_DEVICE_DATA_T ptDevice;

      /* Open Channel gets
          1. length of the boardname
          2. Boardname
          3. Channel Number */
      OS_Memcpy(&ulBoardNameLen,
                pbRequestData,
                sizeof(ulBoardNameLen));

      OS_Memcpy(szBoardName,
                &pbRequestData[4],
                ulBoardNameLen);

      OS_Memcpy(&ulChannelNr,
                &pbRequestData[4 + ulBoardNameLen],
                sizeof(ulChannelNr));

      /* Search a board with this name */
      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxChannelOpen)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;

      } else if(0 == (ptDevice = FindDevice(szBoardName, pvUser)))
      {
        lRet = CIFX_INVALID_BOARD;
      } else if(ulChannelNr >= ptDevice->ulChannelCount)
      {
        lRet = CIFX_INVALID_CHANNEL;
      } else
      {
        PMARSH_CIFX_CHANNEL_DATA_T ptChannel = (PMARSH_CIFX_CHANNEL_DATA_T)&ptDevice->ptChannels[ulChannelNr];

        if(ptChannel->ulOpenCnt > 0)
        {
          /* Increment Reference Count */
          ++ptChannel->ulOpenCnt;
          lRet = CIFX_NO_ERROR;
        } else
        {
          /* Device not yet open, so open it now */
          if(CIFX_NO_ERROR == (lRet =  ptInstance->tDRVFunctions.pfnxChannelOpen(ptInstance->hDriver, szBoardName, ulChannelNr, &ptChannel->hChannel)))
          {
            ++ptChannel->ulOpenCnt;
          }
        }
        if( (ptBuffer->tMgmt.ulDataBufferLen - sizeof(MARSHALLER_DATA_FRAME_HEADER_T)) >= sizeof(uint32_t))
        {
          /* Device was opened successfully, so return valid handle */
          if(CIFX_NO_ERROR == lRet)
          {
            uint32_t* ptHandle = (uint32_t*)pbRequestData;

            *ptHandle                = 0;
            HANDLE_SET_OBJTYPE(*ptHandle, MARSHALLER_OBJECT_TYPE_CHANNEL);
            HANDLE_SET_OBJIDX(*ptHandle, ptDevice->ulBoard);
            HANDLE_SET_OBJSUBIDX(*ptHandle, ulChannelNr);
            HANDLE_SET_VALID(*ptHandle);

            ptMarshallerHeader->ulDataSize = sizeof(*ptHandle);
          }
        }else
        {
          lRet = HIL_MARSHALLER_E_INVALIDPARAMETER;
        }
      }
    }
    break;

  /***************************************************************************
  * xSysdeviceOpen
  ***************************************************************************/
  case MARSHALLER_DRV_METHODID_OPENSYSDEV:
      /* Open System Device gets
          1. length of the boardname
          2. Boardname      */
    {
      uint32_t                  ulBoardNameLen;
      char                      szBoardName[CIFx_MAX_INFO_NAME_LENTH] ={0};
      uint8_t*                  pbRequestData = (uint8_t*)(ptMarshallerHeader + 1);
      PMARSH_CIFX_DEVICE_DATA_T ptDevice;

      /* Open Channel gets
          1. length of the boardname
          2. Boardname
          3. Channel Number */
      OS_Memcpy(&ulBoardNameLen,
                pbRequestData,
                sizeof(ulBoardNameLen));

      OS_Memcpy(szBoardName,
                &pbRequestData[4],
                ulBoardNameLen);

      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxSysdeviceOpen)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;

        /* Search a board with this name */
      } else if(0 == (ptDevice = FindDevice(szBoardName, pvUser)))
      {
        lRet = CIFX_INVALID_BOARD;
      } else
      {
        PMARSH_CIFX_CHANNEL_DATA_T ptChannel = &ptDevice->tSysDevice;

        if(ptChannel->ulOpenCnt > 0)
        {
          /* Increment Reference Count */
          ++ptChannel->ulOpenCnt;
          lRet = CIFX_NO_ERROR;
        } else
        {
          /* Device not yet open, so open it now */
          if(CIFX_NO_ERROR == (lRet = ptInstance->tDRVFunctions.pfnxSysdeviceOpen(ptInstance->hDriver, szBoardName, &ptChannel->hChannel)))
          {
            ++ptChannel->ulOpenCnt;
          }
        }
        if( (ptBuffer->tMgmt.ulDataBufferLen - sizeof(MARSHALLER_DATA_FRAME_HEADER_T)) >= sizeof(uint32_t))
        {
          /* Device was opened successfully, so return valid handle */
          if(CIFX_NO_ERROR == lRet)
          {
            uint32_t* ptHandle = (uint32_t*)(ptMarshallerHeader + 1);

            *ptHandle                = 0;
            HANDLE_SET_OBJTYPE(*ptHandle, MARSHALLER_OBJECT_TYPE_SYSDEVICE);
            HANDLE_SET_OBJIDX(*ptHandle, ptDevice->ulBoard);
            HANDLE_SET_OBJSUBIDX(*ptHandle, MARSHALLER_SUBIDX_SYSTEMCHANNEL);
            HANDLE_SET_VALID(*ptHandle);

            ptMarshallerHeader->ulDataSize  = sizeof(*ptHandle);
          }
        }else
        {
          lRet = HIL_MARSHALLER_E_INVALIDPARAMETER;
        }
      }
    }
    break;

  /***************************************************************************
  * xDriverRestartDevice
  ***************************************************************************/
  case MARSHALLER_DRV_METHODID_RESTARTDEVICE:
    {
      PDRV_RESTARTDEVICE_REQ_T ptEnumReq  = (PDRV_RESTARTDEVICE_REQ_T)(ptMarshallerHeader);
      uint32_t                 ulDataSize = ptMarshallerHeader->ulDataSize;

      ptMarshallerHeader->ulDataSize = 0;

      if(ulDataSize != sizeof(ptEnumReq->tData))
      {
        lRet = CIFX_INVALID_PARAMETER;

      } else if(!ptInstance->tDRVFunctions.pfnxDriverRestartDevice)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;

      } else
      {
        lRet = ptInstance->tDRVFunctions.pfnxDriverRestartDevice(ptInstance->hDriver, ptEnumReq->tData.abBoardName, NULL);
      }
    }
    break;

  default:
    /* Unknown/unsupported method ID */
    lRet = CIFX_INVALID_PARAMETER;
    ptMarshallerHeader->ulDataSize = 0;
    break;
  }

return lRet;
}

/*****************************************************************************/
/*! Handle packets which includes a Sysdevice command
*   \param ptBuffer  Reference to the received packet
*     Note: The given memory of the packet is also used to save the confirmation
*   \param pvUser       Pointer to cifX Internal instance data
*   \return MARSHALLER_NO_ERROR  on success                                  */
/*****************************************************************************/
static int32_t HandleSysdeviceCommand (HIL_MARSHALLER_BUFFER_T* ptBuffer, void* pvUser)
{
  int32_t                         lRet;
  PMARSHALLER_DATA_FRAME_HEADER_T ptMarshallerHeader  = (PMARSHALLER_DATA_FRAME_HEADER_T) &ptBuffer->abData[0];
  PMARSH_HANDLE_INSTANCE_T        ptInstance          = (PMARSH_HANDLE_INSTANCE_T) pvUser;
  PMARSH_CIFX_CHANNEL_DATA_T      ptChannel;

  if((!ptBuffer) || (!pvUser))
    return HIL_MARSHALLER_E_INVALIDPARAMETER;

  if( (HANDLE_GET_OBJIDX(ptMarshallerHeader->ulHandle) > ptInstance->ulDeviceCnt) ||
      (HANDLE_GET_OBJSUBIDX(ptMarshallerHeader->ulHandle) != MARSHALLER_SUBIDX_SYSTEMCHANNEL) )
  {
    ptMarshallerHeader->ulDataSize = 0;
    return CIFX_INVALID_HANDLE;
  }

  ptChannel = &ptInstance->ptDevices[HANDLE_GET_OBJIDX(ptMarshallerHeader->ulHandle)].tSysDevice;

  if (ptChannel->ulOpenCnt == 0)
  {
    ptMarshallerHeader->ulDataSize = 0;
    return CIFX_DRV_CHANNEL_NOT_INITIALIZED;
  }

  switch(ptMarshallerHeader->ulMethodID)
  {
  /***************************************************************************
  * xSysdeviceClose
  ***************************************************************************/
  case MARSHALLER_SYSDEV_METHODID_CLOSE:
    {
      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxSysdeviceClose)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;

      } else if(ptChannel->ulOpenCnt > 1)
      {
        --ptChannel->ulOpenCnt;
        lRet = CIFX_NO_ERROR;

      } else
      {
        if(CIFX_NO_ERROR == (lRet = ptInstance->tDRVFunctions.pfnxSysdeviceClose(ptChannel->hChannel)))
        {
          --ptChannel->ulOpenCnt;
          ptChannel->hChannel = 0;
        }
      }
    }
    break;

  /***************************************************************************
  * xSysdeviceInfo
  ***************************************************************************/
  case MARSHALLER_SYSDEV_METHODID_INFO:
    {
      if(!ptInstance->tDRVFunctions.pfnxSysdeviceInfo)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;

      } else
      {
        PSYSDEV_INFO_REQ_T ptReq     = (PSYSDEV_INFO_REQ_T)ptMarshallerHeader;
        uint8_t*           pbRequest = (uint8_t*)(ptMarshallerHeader + 1);
        uint32_t           ulSize    = ptReq->tData.ulSize;

        lRet = ptInstance->tDRVFunctions.pfnxSysdeviceInfo( ptChannel->hChannel,
                                                            ptReq->tData.ulCmd,
                                                            ulSize,
                                                            pbRequest);
        if( (CIFX_NO_ERROR != lRet) &&
            (CIFX_BUFFER_TOO_SHORT != lRet) )
        {
          ptMarshallerHeader->ulDataSize = 0;
        } else
        {
          ptMarshallerHeader->ulDataSize = ulSize;
        }
      }
    }
    break;

  /***************************************************************************
  * xSysdeviceReset
  ***************************************************************************/
  case MARSHALLER_SYSDEV_METHODID_RESET:
    {
      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxSysdeviceReset)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
      } else
      {
        PSYSDEV_RESET_REQ_T ptReq      = (PSYSDEV_RESET_REQ_T)ptMarshallerHeader;

        lRet = ptInstance->tDRVFunctions.pfnxSysdeviceReset( ptChannel->hChannel,
                                                             ptReq->tData.ulTimeout);
      }
    }
    break;

  /***************************************************************************
  * xSysdeviceResetEx
  ***************************************************************************/
  case MARSHALLER_SYSDEV_METHODID_RESETEX:
    {
      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxSysdeviceResetEx)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
      } else
      {
        PSYSDEV_RESETEX_REQ_T ptReq    = (PSYSDEV_RESETEX_REQ_T)ptMarshallerHeader;

        lRet = ptInstance->tDRVFunctions.pfnxSysdeviceResetEx( ptChannel->hChannel,
                                                               ptReq->tData.ulTimeout,
                                                               ptReq->tData.ulMode);
      }
    }
    break;

  /***************************************************************************
  * xSysdeviceGetMBXState
  ***************************************************************************/
  case MARSHALLER_SYSDEV_METHODID_GETMBXSTATE:
    {
      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxSysdeviceGetMBXState)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
      } else
      {
        PCHANNEL_GETMBXSTATE_CNF_T ptGetMbxCnf = (PCHANNEL_GETMBXSTATE_CNF_T)(ptMarshallerHeader);

        if(CIFX_NO_ERROR == (lRet = ptInstance->tDRVFunctions.pfnxSysdeviceGetMBXState(ptChannel->hChannel,
                                                                                                          &ptGetMbxCnf->tData.ulRecvPktCnt,
                                                                                                          &ptGetMbxCnf->tData.ulSendPktCnt)))
        {
          ptMarshallerHeader->ulDataSize = sizeof(ptGetMbxCnf->tData);
        }
      }
    }
    break;

  /***************************************************************************
  * xSysdevicePutPacket
  ***************************************************************************/
  case MARSHALLER_SYSDEV_METHODID_PUTPACKET:
    {
      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxSysdevicePutPacket)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
      } else
      {
        uint8_t*     pbRequest = (uint8_t*)(ptMarshallerHeader + 1);
        uint32_t     ulSendSize;
        uint32_t     ulTimeout;
        CIFX_PACKET* ptPacket;

        OS_Memcpy(&ulSendSize, pbRequest, sizeof(ulSendSize));
        OS_Memcpy(&ulTimeout,  &pbRequest[sizeof(ulSendSize) + ulSendSize], sizeof(ulTimeout));
        ptPacket = (CIFX_PACKET*)&pbRequest[sizeof(ulSendSize)];

        lRet = ptInstance->tDRVFunctions.pfnxSysdevicePutPacket(ptChannel->hChannel,
                                                                ptPacket,
                                                                ulTimeout);
      }
    }
    break;

  /***************************************************************************
  * xSysdeviceGetPacket
  ***************************************************************************/
  case MARSHALLER_SYSDEV_METHODID_GETPACKET:
    {
      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxSysdeviceGetPacket)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
      } else
      {
        PCHANNEL_GETPACKET_REQ_T ptGetReq  = (PCHANNEL_GETPACKET_REQ_T)(ptMarshallerHeader);
        uint32_t                 ulReqSize = ptGetReq->tData.ulSize;
        CIFX_PACKET*             ptPacket  = (CIFX_PACKET*)(&ptGetReq->tData);

        lRet = ptInstance->tDRVFunctions.pfnxSysdeviceGetPacket(ptChannel->hChannel,
                                                                ulReqSize,
                                                                ptPacket,
                                                                ptGetReq->tData.ulTimeout);

        if(CIFX_NO_ERROR == lRet)
        {
          ptGetReq->tHeader.ulDataSize = ptPacket->tHeader.ulLen + (uint32_t)sizeof(ptPacket->tHeader);
        } else if(CIFX_BUFFER_TOO_SHORT == lRet)
        {
          ptGetReq->tHeader.ulDataSize = ulReqSize;
        }
      }
    }
    break;

  /***************************************************************************
  * xSysdeviceFindFirstFile
  ***************************************************************************/
  case MARSHALLER_SYSDEV_METHODID_FINDFIRSTFILE:
    {
      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxSysdeviceFindFirstFile)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
      } else
      {
        PSYSDEV_FIND_FIRSTFILE_REQ_T ptFindReq = (PSYSDEV_FIND_FIRSTFILE_REQ_T)(ptMarshallerHeader);

        CIFX_DIRECTORYENTRY tDirEntry = {0};

        *((uint32_t*)&tDirEntry.hList) = ptFindReq->tData.hList;
        tDirEntry.bFiletype  = ptFindReq->tData.bFiletype;
        tDirEntry.ulFilesize = ptFindReq->tData.ulFilesize;
        OS_Memcpy(tDirEntry.szFilename, ptFindReq->tData.szFilename, sizeof(ptFindReq->tData.szFilename));

        /* Note: Currently we will not route callback. All packet that do not belong to
                 this transaction will be discarded */
        lRet = ptInstance->tDRVFunctions.pfnxSysdeviceFindFirstFile(ptChannel->hChannel,
                                                                    ptFindReq->tData.ulChannel,
                                                                    &tDirEntry,
                                                                    0,
                                                                    0);

        if(CIFX_NO_ERROR == lRet)
        {
          PSYSDEV_FIND_FIRSTFILE_CNF_T ptFindCnf = (PSYSDEV_FIND_FIRSTFILE_CNF_T)(ptMarshallerHeader);

          ptFindCnf->tData.hList      = *((uint32_t*)&tDirEntry.hList);
          ptFindCnf->tData.bFiletype  = tDirEntry.bFiletype;
          ptFindCnf->tData.ulFilesize = tDirEntry.ulFilesize;
          OS_Memcpy(ptFindCnf->tData.szFilename, tDirEntry.szFilename, sizeof(ptFindCnf->tData.szFilename));

          ptMarshallerHeader->ulDataSize = sizeof(ptFindCnf->tData);
        }
      }
    }
    break;

  /***************************************************************************
  * xSysdeviceFindNextFile
  ***************************************************************************/
  case MARSHALLER_SYSDEV_METHODID_FINDNEXTFILE:
    {
      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxSysdeviceFindNextFile)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
      } else
      {
        PSYSDEV_FIND_NEXTFILE_REQ_T ptFindReq = (PSYSDEV_FIND_NEXTFILE_REQ_T)(ptMarshallerHeader);

        CIFX_DIRECTORYENTRY tDirEntry = {0};

        *((uint32_t*)&tDirEntry.hList) = ptFindReq->tData.hList;
        tDirEntry.bFiletype  = ptFindReq->tData.bFiletype;
        tDirEntry.ulFilesize = ptFindReq->tData.ulFilesize;
        OS_Memcpy(tDirEntry.szFilename, ptFindReq->tData.szFilename, sizeof(ptFindReq->tData.szFilename));

        /* Note: Currently we will not route callback. All packet that do not belong to
                 this transaction will be discarded */
        lRet = ptInstance->tDRVFunctions.pfnxSysdeviceFindNextFile(ptChannel->hChannel,
                                                                   ptFindReq->tData.ulChannel,
                                                                   &tDirEntry,
                                                                   0,
                                                                   0);

        if(CIFX_NO_ERROR == lRet)
        {
          PSYSDEV_FIND_NEXTFILE_CNF_T ptFindCnf = (PSYSDEV_FIND_NEXTFILE_CNF_T)(ptMarshallerHeader);

          ptFindCnf->tData.hList      = *((uint32_t*)&tDirEntry.hList);
          ptFindCnf->tData.bFiletype  = tDirEntry.bFiletype;
          ptFindCnf->tData.ulFilesize = tDirEntry.ulFilesize;
          OS_Memcpy(ptFindCnf->tData.szFilename, tDirEntry.szFilename, sizeof(ptFindCnf->tData.szFilename));

          ptMarshallerHeader->ulDataSize = sizeof(ptFindCnf->tData);
        }
      }
    }
    break;

  /***************************************************************************
  * xSysdeviceDownload
  ***************************************************************************/
  case MARSHALLER_SYSDEV_METHODID_DOWNLOAD:
    ptMarshallerHeader->ulDataSize = 0;
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
    break;

  /***************************************************************************
  * xSysdeviceUpload
  ***************************************************************************/
  case MARSHALLER_SYSDEV_METHODID_UPLOAD:
    ptMarshallerHeader->ulDataSize = 0;
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
    break;

  default:
    /* Unknown/unsupported method ID */
    ptMarshallerHeader->ulDataSize = 0;
    lRet = CIFX_INVALID_PARAMETER;
    break;
  }

  return lRet;
}

typedef int32_t(APIENTRY *PFN_CIFX_BLOCK)( CIFXHANDLE  hChannel, uint32_t ulCmd, uint32_t ulOffset, uint32_t ulDataLen, void* pvData);

static int32_t ChannelReadWriteBlock(PMARSHALLER_DATA_FRAME_HEADER_T ptMarshallerHeader, CIFXHANDLE hChannel, PFN_CIFX_BLOCK pfnBlock)
{
  PCHANNEL_BLOCK_READ_REQ_T ptReadReq = (PCHANNEL_BLOCK_READ_REQ_T)ptMarshallerHeader;
  int32_t                   lRet;

  if(!pfnBlock)
  {
    ptMarshallerHeader->ulDataSize = 0;
    return CIFX_FUNCTION_NOT_AVAILABLE;
  }

  switch(ptReadReq->tData.ulCmd)
  {
  case CIFX_CMD_READ_DATA:
    {
      PCHANNEL_BLOCK_READ_CNF_T ptReadCnf = (PCHANNEL_BLOCK_READ_CNF_T)ptMarshallerHeader;
      uint32_t                  ulDatalen = ptReadReq->tData.ulDatalen;

      lRet = pfnBlock( hChannel,
                       ptReadReq->tData.ulCmd,
                       ptReadReq->tData.ulOffset,
                       ulDatalen,
                       ptReadCnf->tData.abData);

      if( (CIFX_NO_ERROR == lRet) || (CIFX_BUFFER_TOO_SHORT == lRet) )
      {
        ptMarshallerHeader->ulDataSize = ulDatalen;
      } else
      {
        ptMarshallerHeader->ulDataSize = 0;
      }
    }
    break;

  case CIFX_CMD_WRITE_DATA:
    {
      PCHANNEL_BLOCK_WRITE_REQ_T ptWriteReq = (PCHANNEL_BLOCK_WRITE_REQ_T)ptMarshallerHeader;

      lRet = pfnBlock( hChannel,
                       ptWriteReq->tData.ulCmd,
                       ptWriteReq->tData.ulOffset,
                       ptWriteReq->tData.ulDatalen,
                       ptWriteReq->tData.abData);

      ptMarshallerHeader->ulDataSize = 0;
    }
    break;

  default:
    ptMarshallerHeader->ulDataSize = 0;
    lRet = CIFX_INVALID_PARAMETER;
    break;
  }

  return lRet;
}

/*****************************************************************************/
/*! Handle packets which includes a special channel command
*   \param ptBuffer     Reference to the give packet
*                       Note: The given memory of the packet is also
*                       used to save the confimation
*   \param pvUser       Pointer to cifX Internal instance data
*   \return MARSHALLER_NO_ERROR  on success                                  */
/*****************************************************************************/
static int32_t HandleChannelCommand (HIL_MARSHALLER_BUFFER_T* ptBuffer, void* pvUser)
{
  int32_t                         lRet                = CIFX_INVALID_PARAMETER;
  PMARSHALLER_DATA_FRAME_HEADER_T ptMarshallerHeader  = (PMARSHALLER_DATA_FRAME_HEADER_T) &ptBuffer->abData[0];
  PMARSH_HANDLE_INSTANCE_T        ptInstance          = (PMARSH_HANDLE_INSTANCE_T) pvUser;
  PMARSH_CIFX_CHANNEL_DATA_T      ptChannel;
  uint8_t                         bDeviceNumber;
  uint8_t                         bChannelNumber;

  if((!ptBuffer) || (!pvUser))
    return CIFX_INVALID_PARAMETER;

  if(HANDLE_GET_OBJIDX(ptMarshallerHeader->ulHandle) > ptInstance->ulDeviceCnt)
  {
    ptMarshallerHeader->ulDataSize = 0;
    return CIFX_INVALID_HANDLE;
  }

  if( (HANDLE_GET_OBJSUBIDX(ptMarshallerHeader->ulHandle) > ptInstance->ptDevices[HANDLE_GET_OBJIDX(ptMarshallerHeader->ulHandle)].ulChannelCount) ||
      !ptInstance->ptDevices[HANDLE_GET_OBJIDX(ptMarshallerHeader->ulHandle)].fValid )
  {
    ptMarshallerHeader->ulDataSize = 0;
    return CIFX_INVALID_HANDLE;
  }

  bDeviceNumber  = (uint8_t)HANDLE_GET_OBJIDX(ptMarshallerHeader->ulHandle);
  bChannelNumber = (uint8_t)HANDLE_GET_OBJSUBIDX(ptMarshallerHeader->ulHandle);
  ptChannel      = &ptInstance->ptDevices[bDeviceNumber].ptChannels[bChannelNumber];

  if (ptChannel->ulOpenCnt == 0)
  {
    ptMarshallerHeader->ulDataSize = 0;
    return CIFX_DRV_CHANNEL_NOT_INITIALIZED;
  }

  switch(ptMarshallerHeader->ulMethodID)
  {
  /***************************************************************************
  * xChannelClose
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_CLOSE:
    {
      if(!ptInstance->tDRVFunctions.pfnxChannelClose)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;
      } else
      {
        ptMarshallerHeader->ulDataSize = 0;
        if(ptChannel->ulOpenCnt > 1)
        {
          --ptChannel->ulOpenCnt;
          lRet = CIFX_NO_ERROR;
        } else
        {
          if(CIFX_NO_ERROR == (lRet = ptInstance->tDRVFunctions.pfnxChannelClose(ptChannel->hChannel)))
          {
            --ptChannel->ulOpenCnt;
            ptChannel->hChannel = 0;
          }
        }
      }
    }
    break;

  /***************************************************************************
  * xChannelGetMBXState
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_GETMBXSTATE:
    {
      if(!ptInstance->tDRVFunctions.pfnxChannelGetMBXState)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;

      } else
      {
        PCHANNEL_GETMBXSTATE_CNF_T ptGetMbxCnf = (PCHANNEL_GETMBXSTATE_CNF_T)(ptMarshallerHeader);

        ptMarshallerHeader->ulDataSize = 0;

        if(CIFX_NO_ERROR == (lRet = ptInstance->tDRVFunctions.pfnxChannelGetMBXState(ptChannel->hChannel,
                                                                                     &ptGetMbxCnf->tData.ulRecvPktCnt,
                                                                                     &ptGetMbxCnf->tData.ulSendPktCnt)))
        {
          ptGetMbxCnf->tHeader.ulDataSize = sizeof(ptGetMbxCnf->tData);
        }
      }
    }
    break;

  /***************************************************************************
  * xChannelPutPacket
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_PUTPACKET:
    {
      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxChannelPutPacket)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;

      } else
      {
        uint8_t*     pbRequest = (uint8_t*)(ptMarshallerHeader + 1);
        uint32_t     ulSendSize;
        uint32_t     ulTimeout;
        CIFX_PACKET* ptPacket;

        OS_Memcpy(&ulSendSize, pbRequest, sizeof(ulSendSize));
        OS_Memcpy(&ulTimeout,  &pbRequest[sizeof(ulSendSize) + ulSendSize], sizeof(ulTimeout));
        ptPacket = (CIFX_PACKET*)&pbRequest[sizeof(ulSendSize)];

        lRet = ptInstance->tDRVFunctions.pfnxChannelPutPacket(ptChannel->hChannel,
                                                              ptPacket,
                                                              ulTimeout);
      }
    }
    break;

  /***************************************************************************
  * xChannelGetPacket
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_GETPACKET:
    {
      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxChannelGetPacket)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;

      } else
      {
        PCHANNEL_GETPACKET_REQ_T ptGetReq  = (PCHANNEL_GETPACKET_REQ_T)(ptMarshallerHeader);
        uint32_t                 ulReqSize = ptGetReq->tData.ulSize;
        CIFX_PACKET*             ptPacket  = (CIFX_PACKET*)(&ptGetReq->tData);


        lRet = ptInstance->tDRVFunctions.pfnxChannelGetPacket(ptChannel->hChannel,
                                                              ulReqSize,
                                                              ptPacket,
                                                              ptGetReq->tData.ulTimeout);

        if(CIFX_NO_ERROR == lRet)
        {
          ptGetReq->tHeader.ulDataSize = ptPacket->tHeader.ulLen + (uint32_t)sizeof(ptPacket->tHeader);

        } else if(CIFX_BUFFER_TOO_SHORT == lRet)
        {
          ptGetReq->tHeader.ulDataSize = ulReqSize;
        }
      }
    }
    break;

  /***************************************************************************
  * xChannelGetSendPacket
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_GETSENDPACKET:
    {
      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxChannelGetSendPacket)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;

      } else
      {
        uint8_t* pbRequest = (uint8_t*)(ptMarshallerHeader + 1);
        uint32_t ulSize;
        CIFX_PACKET*   ptPacket;

        OS_Memcpy(&ulSize, pbRequest, sizeof(ulSize));
        ptPacket = (CIFX_PACKET*)pbRequest;

        lRet = ptInstance->tDRVFunctions.pfnxChannelGetSendPacket( ptChannel->hChannel,
                                                                   ulSize,
                                                                   ptPacket);
        if(CIFX_NO_ERROR == lRet)
        {
          ptMarshallerHeader->ulDataSize = ptPacket->tHeader.ulLen + (uint32_t)sizeof(ptPacket->tHeader);
        } else if(CIFX_BUFFER_TOO_SHORT == lRet)
        {
          ptMarshallerHeader->ulDataSize = ulSize;
        }
      }
    }
    break;

  /***************************************************************************
  * xChannelConfigLock
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_CONFIGLOCK:
    {
      if(!ptInstance->tDRVFunctions.pfnxChannelConfigLock)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;

      } else
      {
        PCHANNEL_CONFIGLOCK_REQ_T ptReq   = (PCHANNEL_CONFIGLOCK_REQ_T)ptMarshallerHeader;
        PCHANNEL_CONFIGLOCK_CNF_T ptCnf   = (PCHANNEL_CONFIGLOCK_CNF_T)ptMarshallerHeader;
        uint32_t                  ulState = ptReq->tData.ulState;

        lRet = ptInstance->tDRVFunctions.pfnxChannelConfigLock( ptChannel->hChannel,
                                                                ptReq->tData.ulCmd,
                                                                &ulState,
                                                                ptReq->tData.ulTimeout);
        ptCnf->tData.ulState           = ulState;
        ptMarshallerHeader->ulDataSize = sizeof(ptCnf->tData);
      }
    }
    break;

  /***************************************************************************
  * xChannelReset
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_RESET:
    {
      if(!ptInstance->tDRVFunctions.pfnxChannelReset)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;

      } else
      {
        PCHANNEL_RESET_REQ_T ptReq      = (PCHANNEL_RESET_REQ_T)ptMarshallerHeader;

        lRet = ptInstance->tDRVFunctions.pfnxChannelReset( ptChannel->hChannel,
                                                           ptReq->tData.ulMode,
                                                           ptReq->tData.ulTimeout);
        ptMarshallerHeader->ulDataSize = 0;
      }
    }
  break;

  /***************************************************************************
  * xChannelInfo
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_INFO:
    {
      if(!ptInstance->tDRVFunctions.pfnxChannelInfo)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;
      } else
      {
        PCHANNEL_INFO_REQ_T ptReq      = (PCHANNEL_INFO_REQ_T)ptMarshallerHeader;
        uint8_t*            pbRequest  = (uint8_t*)(ptMarshallerHeader + 1);
        uint32_t            ulSize     = ptReq->tData.ulSize;

        lRet = ptInstance->tDRVFunctions.pfnxChannelInfo( ptChannel->hChannel,
                                                          ulSize,
                                                          pbRequest);
        ptMarshallerHeader->ulDataSize = ulSize;
      }
    }
    break;

  /***************************************************************************
  * xChannelWatchdog
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_WATCHDOG:
    {
      if(!ptInstance->tDRVFunctions.pfnxChannelWatchdog)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;
      } else
      {
        PCHANNEL_WATCHDOG_REQ_T ptReq     = (PCHANNEL_WATCHDOG_REQ_T)ptMarshallerHeader;
        PCHANNEL_WATCHDOG_CNF_T ptCnf     = (PCHANNEL_WATCHDOG_CNF_T)ptMarshallerHeader;
        uint32_t                ulTrigger = ptReq->tData.ulTrigger;

        lRet = ptInstance->tDRVFunctions.pfnxChannelWatchdog( ptChannel->hChannel,
                                                              ptReq->tData.ulCmd,
                                                              &ulTrigger);

        ptMarshallerHeader->ulDataSize = sizeof(ptCnf->tData);
        ptCnf->tData.ulTrigger         = ulTrigger;
      }
    }
    break;

  /***************************************************************************
  * xChannelHostState
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_HOSTSTATE:
    {
      if(!ptInstance->tDRVFunctions.pfnxChannelHostState)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;
      } else
      {
        PCHANNEL_HOSTSTATE_REQ_T ptReq    = (PCHANNEL_HOSTSTATE_REQ_T)ptMarshallerHeader;
        PCHANNEL_HOSTSTATE_CNF_T ptCnf    = (PCHANNEL_HOSTSTATE_CNF_T)ptMarshallerHeader;
        uint32_t                 ulState  = ptReq->tData.ulState;

        lRet = ptInstance->tDRVFunctions.pfnxChannelHostState( ptChannel->hChannel,
                                                               ptReq->tData.ulCmd,
                                                               &ulState,
                                                               ptReq->tData.ulTimeout);
        ptCnf->tData.ulState = ulState;
        ptMarshallerHeader->ulDataSize = sizeof(ptCnf->tData);
      }
    }
    break;

  /***************************************************************************
  * xChannelIOInfo
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_IOINFO:
    {
      if(!ptInstance->tDRVFunctions.pfnxChannelIOInfo)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;
      } else
      {
        PCHANNEL_IOINFO_REQ_T ptReq     = (PCHANNEL_IOINFO_REQ_T)ptMarshallerHeader;
        uint8_t*              pbReturn  = (uint8_t*)(ptMarshallerHeader + 1);
        uint32_t              ulDataLen = min(ptReq->tData.ulDataLen, ptBuffer->tMgmt.ulDataBufferLen);

        lRet = ptInstance->tDRVFunctions.pfnxChannelIOInfo( ptChannel->hChannel,
                                                            ptReq->tData.ulCmd,
                                                            ptReq->tData.ulArea,
                                                            ulDataLen,
                                                            pbReturn);

        if(CIFX_NO_ERROR != lRet)
        {
          ptMarshallerHeader->ulDataSize = 0;
        } else
        {
          ptMarshallerHeader->ulDataSize = ulDataLen;
        }
      }
    }
    break;


  /***************************************************************************
  * xChannelIORead
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_IOREAD:
    {
      if(!ptInstance->tDRVFunctions.pfnxChannelIORead)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;
      } else
      {
        PCHANNEL_IOREAD_REQ_T ptReq     = (PCHANNEL_IOREAD_REQ_T)ptMarshallerHeader;
        uint8_t*              pbReturn  = (uint8_t*)(ptMarshallerHeader + 1);
        uint32_t              ulDataLen = ptReq->tData.ulDataLen;

        lRet = ptInstance->tDRVFunctions.pfnxChannelIORead( ptChannel->hChannel,
                                                            ptReq->tData.ulArea,
                                                            ptReq->tData.ulOffset,
                                                            ulDataLen,
                                                            pbReturn,
                                                            ptReq->tData.ulTimeout);

        if((CIFX_NO_ERROR != lRet) && (CIFX_DEV_NO_COM_FLAG != lRet))
        {
          ptMarshallerHeader->ulDataSize = 0;
        } else
        {
          ptMarshallerHeader->ulDataSize = ulDataLen;
        }
      }
    }
    break;

  /***************************************************************************
  * xChannelIOWrite
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_IOWRITE:
    {
      if(!ptInstance->tDRVFunctions.pfnxChannelIOWrite)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;
      } else
      {
        PCHANNEL_IOWRITE_REQ_T ptReq = (PCHANNEL_IOWRITE_REQ_T)ptMarshallerHeader;

        lRet = ptInstance->tDRVFunctions.pfnxChannelIOWrite( ptChannel->hChannel,
                                                             ptReq->tData.ulArea,
                                                             ptReq->tData.ulOffset,
                                                             ptReq->tData.ulDataLen,
                                                             ptReq->tData.abData,
                                                             ptReq->tData.ulTimeout);
        ptMarshallerHeader->ulDataSize = 0;
      }
    }
    break;

  /***************************************************************************
  * xChannelIOReadSendData
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_IOREADSENDDATA:
    {
      if(!ptInstance->tDRVFunctions.pfnxChannelIOReadSendData)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;
      } else
      {
        PCHANNEL_IOREADSENDDATA_REQ_T ptReq     = (PCHANNEL_IOREADSENDDATA_REQ_T)ptMarshallerHeader;
        uint8_t*                      pbReturn  = (uint8_t*)(ptMarshallerHeader + 1);
        uint32_t                      ulDataLen = ptReq->tData.ulDataLen;

        lRet = ptInstance->tDRVFunctions.pfnxChannelIOReadSendData( ptChannel->hChannel,
                                                                    ptReq->tData.ulArea,
                                                                    ptReq->tData.ulOffset,
                                                                    ulDataLen,
                                                                    pbReturn);

        if((CIFX_NO_ERROR != lRet) && (CIFX_DEV_NO_COM_FLAG != lRet))
        {
          ptMarshallerHeader->ulDataSize = 0;
        } else
        {
          ptMarshallerHeader->ulDataSize = ulDataLen;
        }
      }
    }
    break;

  /***************************************************************************
  * xChannelBusState
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_BUSSTATE:
    {
      if(!ptInstance->tDRVFunctions.pfnxChannelBusState)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
        ptMarshallerHeader->ulDataSize = 0;
      } else
      {
        PCHANNEL_BUSSTATE_REQ_T ptReq   = (PCHANNEL_BUSSTATE_REQ_T)ptMarshallerHeader;
        PCHANNEL_BUSSTATE_CNF_T ptCnf   = (PCHANNEL_BUSSTATE_CNF_T)ptMarshallerHeader;
        uint32_t                ulState = ptReq->tData.ulState;

        lRet = ptInstance->tDRVFunctions.pfnxChannelBusState( ptChannel->hChannel,
                                                              ptReq->tData.ulCmd,
                                                              &ulState,
                                                              ptReq->tData.ulTimeout);
        ptCnf->tData.ulState = ulState;
        ptMarshallerHeader->ulDataSize = sizeof(ptCnf->tData);
      }
    }
    break;

  /***************************************************************************
  * xChannelControlBlock
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_CONTROLBLOCK:
    lRet = ChannelReadWriteBlock(ptMarshallerHeader,
                                 ptChannel->hChannel,
                                 ptInstance->tDRVFunctions.pfnxChannelControlBlock);
    break;

  /***************************************************************************
  * xChannelStatusBlock
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_STATUSBLOCK:
    lRet = ChannelReadWriteBlock(ptMarshallerHeader,
                                 ptChannel->hChannel,
                                 ptInstance->tDRVFunctions.pfnxChannelCommonStatusBlock);
    break;

  /***************************************************************************
  * xChannelExtendedStatusBlock
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_EXTSTATUSBLOCK:
    lRet = ChannelReadWriteBlock(ptMarshallerHeader,
                                 ptChannel->hChannel,
                                 ptInstance->tDRVFunctions.pfnxChannelExtendedStatusBlock);
    break;

  /***************************************************************************
  * xChannelUserBlock
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_USERBLOCK:
    ptMarshallerHeader->ulDataSize = 0;
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
    break;

  /***************************************************************************
  * xChannelFindFirstFile
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_FINDFIRSTFILE:
    {
      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxChannelFindFirstFile)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
      } else
      {
        PCHANNEL_FIND_FIRSTFILE_REQ_T ptFindReq = (PCHANNEL_FIND_FIRSTFILE_REQ_T)(ptMarshallerHeader);

        CIFX_DIRECTORYENTRY tDirEntry = {0};

        *((uint32_t*)&tDirEntry.hList) = ptFindReq->tData.hList;
        tDirEntry.bFiletype  = ptFindReq->tData.bFiletype;
        tDirEntry.ulFilesize = ptFindReq->tData.ulFilesize;
        OS_Memcpy(tDirEntry.szFilename, ptFindReq->tData.szFilename, sizeof(ptFindReq->tData.szFilename));

        /* Note: Currently we will not route callback. All packet that do not belong to
                 this transaction will be discarded */
        lRet = ptInstance->tDRVFunctions.pfnxChannelFindFirstFile(ptChannel->hChannel,
                                                                  &tDirEntry,
                                                                  0,
                                                                  0);

        if(CIFX_NO_ERROR == lRet)
        {
          PCHANNEL_FIND_FIRSTFILE_CNF_T ptFindCnf = (PCHANNEL_FIND_FIRSTFILE_CNF_T)(ptMarshallerHeader);

          ptFindCnf->tData.hList      = *((uint32_t*)&tDirEntry.hList);
          ptFindCnf->tData.bFiletype  = tDirEntry.bFiletype;
          ptFindCnf->tData.ulFilesize = tDirEntry.ulFilesize;
          OS_Memcpy(ptFindCnf->tData.szFilename, tDirEntry.szFilename, sizeof(ptFindCnf->tData.szFilename));

          ptMarshallerHeader->ulDataSize = sizeof(ptFindReq->tData);
        }
      }
    }
    break;

  /***************************************************************************
  * xChannelFindNextFile
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_FINDNEXTFILE:
    {
      ptMarshallerHeader->ulDataSize = 0;

      if(!ptInstance->tDRVFunctions.pfnxChannelFindNextFile)
      {
        lRet = CIFX_FUNCTION_NOT_AVAILABLE;
      } else
      {
        PCHANNEL_FIND_NEXTFILE_REQ_T ptFindReq = (PCHANNEL_FIND_NEXTFILE_REQ_T)(ptMarshallerHeader);

        CIFX_DIRECTORYENTRY tDirEntry = {0};

        *((uint32_t*)&tDirEntry.hList) = ptFindReq->tData.hList;
        tDirEntry.bFiletype  = ptFindReq->tData.bFiletype;
        tDirEntry.ulFilesize = ptFindReq->tData.ulFilesize;
        OS_Memcpy(tDirEntry.szFilename, ptFindReq->tData.szFilename, sizeof(ptFindReq->tData.szFilename));

        /* Note: Currently we will not route callback. All packet that do not belong to
                 this transaction will be discarded */
        lRet = ptInstance->tDRVFunctions.pfnxChannelFindNextFile(ptChannel->hChannel,
                                                                 &tDirEntry,
                                                                 0,
                                                                 0);

        if(CIFX_NO_ERROR == lRet)
        {
          PCHANNEL_FIND_NEXTFILE_CNF_T ptFindCnf = (PCHANNEL_FIND_NEXTFILE_CNF_T)(ptMarshallerHeader);

          ptFindCnf->tData.hList = *((uint32_t*)&tDirEntry.hList);
          ptFindCnf->tData.bFiletype  = tDirEntry.bFiletype;
          ptFindCnf->tData.ulFilesize = tDirEntry.ulFilesize;
          OS_Memcpy(ptFindCnf->tData.szFilename, tDirEntry.szFilename, sizeof(ptFindCnf->tData.szFilename));

          ptMarshallerHeader->ulDataSize = sizeof(ptFindCnf->tData);
        }
      }
    }
    break;

  /***************************************************************************
  * xChannelDownload
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_DOWNLOAD:
    ptMarshallerHeader->ulDataSize = 0;
    lRet = CIFX_FUNCTION_NOT_AVAILABLE;
    break;

  /***************************************************************************
  * xChannelUpload
  ***************************************************************************/
  case MARSHALLER_CHANNEL_METHODID_UPLOAD:
      ptMarshallerHeader->ulDataSize = 0;
      lRet = CIFX_FUNCTION_NOT_AVAILABLE;
    break;

  default:
    /* Unknown/unsupported method ID */
    ptMarshallerHeader->ulDataSize = 0;
    lRet = CIFX_INVALID_COMMAND;
    break;
  }

return lRet;
}



/*****************************************************************************/
/*!Find a device by name
*   \param szBoard        Name of the search device
*   \param pvUser       Pointer to cifX Internal instance data
*   \return MARSHALLER_NO_ERROR  on success                                  */
/*****************************************************************************/
static PMARSH_CIFX_DEVICE_DATA_T FindDevice(char* szBoard, void* pvUser)
{
  unsigned long             ulIdx;
  PMARSH_HANDLE_INSTANCE_T  ptInstance = (PMARSH_HANDLE_INSTANCE_T) pvUser;
  PMARSH_CIFX_DEVICE_DATA_T ptRet     = NULL;

  if(!pvUser)
    return NULL;

  for(ulIdx = 0; ulIdx < ptInstance->ulDeviceCnt; ulIdx++)
  {
    if( (OS_Strnicmp(szBoard, ptInstance->ptDevices[ulIdx].tBoardInfo.abBoardName, sizeof(ptInstance->ptDevices[ulIdx].tBoardInfo.abBoardName)) == 0) ||
        (OS_Strnicmp(szBoard, ptInstance->ptDevices[ulIdx].tBoardInfo.abBoardAlias, sizeof(ptInstance->ptDevices[ulIdx].tBoardInfo.abBoardAlias)) == 0) )
    {
      ptRet = &ptInstance->ptDevices[ulIdx];
      break;
    }
  }

  return ptRet;
}

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/
