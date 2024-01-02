// SPDX-License-Identifier: MIT
/**************************************************************************************
 *
 * Copyright (c) 2024, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 * Description: Extension module for file download and storage support
 *
 * Changes:
 *
 *   Version   Date        Author   Description
 *   ----------------------------------------------------------------------------------
 *   1        02.01.24    SD        changed licensing terms
 *
 **************************************************************************************/

#include <OS_Includes.h>
#include "HilFileHeaderV3.h"
#include "cifXUser.h"
#include "cifXErrors.h"
#include "rcX_Public.h"
#include "cifx_download_hook.h"

/*****************************************************************************/
/*! Structure holding resources for download transaction                     */
/*****************************************************************************/
typedef struct TXN_RSRC_Ttag
{
  void*         hDevice;        /*!< Device linked to this transaction resource */
  BOARD_INFORMATION tBoardInfo; /*!< Structure holding board information */
  uint32_t      ulState;        /*!< Current transaction state */
  uint32_t      ulChannel;      /*!< Channel download destination */
  uint32_t      ulMaxBlockSize; /*!< Maximum block size for packet transfer */
  int32_t       lError;         /*!< Transaction error */
  uint8_t       bTxnMode;       /*!< Capture or monitor mode */

  char          szFilename[CIFx_MAX_INFO_NAME_LENTH]; /*!< File name. */
  uint32_t      ulDownloadMode; /*!< Download mode (firmware, config, module or file download) */
  uint32_t      ulFilesize;     /*!< File size. */
  uint8_t*      pbFileBuf;      /*!< Buffer to store file data */
  uint8_t*      pabActData;     /*!< Pointer storing current buffer location */

  CIFX_PACKET*  ptConfPkt;      /*!< Confirmation packet for capture mode */
  
  struct TXN_RSRC_Ttag* pNext;  /*!< Next transaction resource */

} TXN_RSRC_T, *PTXN_RSRC_T;

/*****************************************************************************/
/*! Structure holding channel specific packet handling functions             */
/*****************************************************************************/
typedef struct DRVFNC_Ttag
{
  PFN_xChannelGetMBXState pfnGetMBXState;
  PFN_xChannelPutPacket   pfnPutPacket;
  PFN_xChannelGetPacket   pfnGetPacket;

} DRVFNC_T, *PDRVFNC_T;

/*!< Driver function for board enumeration */
static PFN_xDriverEnumBoards s_pfnDriverEnumBoards = NULL;

/*!< Sysdevice and channel specific function tables */
static DRVFNC_T s_tDRVFncSysdevice = {0};
static DRVFNC_T s_tDRVFncChannel   = {0};

/*!< List pointer for download transaction */
static PTXN_RSRC_T s_ptTxnList = NULL;

/*!< Files storage callback function */
static PFN_FILESTORAGE_CBK s_pfnFileStorageCbk = NULL;
static void*               s_pvUser            = NULL;

/* Download transaction modes */
#define TXN_MODE_PACKET_MONITOR 0x00
#define TXN_MODE_PACKET_CAPTURE 0x01

/* Download transaction states */
#define TXN_STATE_DOWNLOAD_REQUEST 0x00
#define TXN_STATE_DATA_TRANSFER    0x01
#define TXN_STATE_FINISHED         0x02
#define TXN_STATE_ERROR            0x03


/*****************************************************************************/
/*! Transfer packet function from the DLL
*   \param hDevice            Handle to the device
*   \param ptDrvFnc           Driver function table
*   \param ptSendPkt          Send packet pointer 
*   \param ptRecvPkt          Receive packet pointer
*   \param ulRecvBufferSize   Size of the receive packet buffer
*   \param ulTimeout          Wait timeout
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t TransferPacket(CIFXHANDLE hDevice, PDRVFNC_T ptDrvFnc,
                              CIFX_PACKET* ptSendPkt, CIFX_PACKET* ptRecvPkt,
                              uint32_t ulRecvBufferSize, uint32_t ulTimeout)
{
  long     lCount  = 0;
  int32_t  lRet    = CIFX_NO_ERROR;

  if( (lRet = ptDrvFnc->pfnPutPacket(hDevice, ptSendPkt, ulTimeout)) == CIFX_NO_ERROR) 
  {   
    do
    {
      if( (lRet = ptDrvFnc->pfnGetPacket(hDevice, ulRecvBufferSize, ptRecvPkt, ulTimeout)) == CIFX_NO_ERROR) 
      { 
        /* Check if we got the answer */
        if((ptRecvPkt->tHeader.ulCmd & ~RCX_MSK_PACKET_ANSWER) == ptSendPkt->tHeader.ulCmd )
        {
          /* Check rest of packet data */
          if ( (ptRecvPkt->tHeader.ulSrc   == ptSendPkt->tHeader.ulSrc)    &&
               (ptRecvPkt->tHeader.ulId    == ptSendPkt->tHeader.ulId)     &&
               (ptRecvPkt->tHeader.ulSrcId == ptSendPkt->tHeader.ulSrcId)  )
          {
            /* We got the answer message */
            break;
          }
        }
        /* Reset error, in case we might drop out of the loop, with no proper answer,
           returning a "good" state */
        lRet = CIFX_DEV_GET_TIMEOUT;
        lCount++;
        
      }else
      {
        /* Error during packet receive */
        break;       
      }
    } while ( lCount < 10);
  }

  return lRet;
}

/*****************************************************************************/
/*! Get the download mode by the file name
*   \param usFileNameLength Length of file name
*   \param pszFileName      Input file name
*   \return Download mode (firmware, config, module or file download)        */
/*****************************************************************************/
static uint32_t GetDownloadModeFromFileName( uint32_t usFileNameLength, char* pszFileName)
{
  uint32_t ulRet    = DOWNLOAD_MODE_FILE;

  /* File name should be at least x.abc */
  if( usFileNameLength >= CIFX_MIN_FILE_NAME_LENGTH)
  {
    /* It's a firmware if the extension matches NXF */
    if (0 == OS_Strnicmp( HIL_FILE_EXTENSION_FIRMWARE, &pszFileName[usFileNameLength - 4], 4))
    {
      ulRet = DOWNLOAD_MODE_FIRMWARE;

    /* It's a module if the extension matches NXO */
    } else if (0 == OS_Strnicmp( HIL_FILE_EXTENSION_MODULE, &pszFileName[usFileNameLength - 4], 4))
    {
      ulRet = DOWNLOAD_MODE_MODULE;

    /* It's a configuration if the extension matches NXD */
    } else if (0 == OS_Strnicmp( HIL_FILE_EXTENSION_DATABASE, &pszFileName[usFileNameLength - 4], 4))
    {
      ulRet = DOWNLOAD_MODE_CONFIG;

    } else if (0 == OS_Strnicmp( ".XML", &pszFileName[usFileNameLength - 4], 4))
    {
      ulRet = DOWNLOAD_MODE_CONFIG;

    }
  }

  return ulRet;
}

/*****************************************************************************/
/*! Get the transaction mode (capture or monitor) by device type (ram or flash based)
*   \param hDevice   Device handle
*   \param pbTxnMode Buffer to return mode
*   \param ptDrvFnc  Driver function table
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t GetTxnMode(CIFXHANDLE hDevice, uint8_t* pbTxnMode, PDRVFNC_T ptDrvFnc)
{
  int32_t                         lRet             = CIFX_NO_ERROR;
  RCX_READ_SYS_STATUS_BLOCK_REQ_T tSystemStatusReq = {{0}};
  RCX_READ_SYS_STATUS_BLOCK_CNF_T tSystemStatusCnf = {{0}};

  tSystemStatusReq.tHead.ulSrc    = 0;
  tSystemStatusReq.tHead.ulDest   = RCX_PACKET_DEST_SYSTEM;
  tSystemStatusReq.tHead.ulCmd    = RCX_SYSTEM_STATUS_BLOCK_REQ;

  if (CIFX_NO_ERROR == (lRet = TransferPacket(hDevice, ptDrvFnc, (CIFX_PACKET*)&tSystemStatusReq, 
                                              (CIFX_PACKET*)&tSystemStatusCnf, sizeof(tSystemStatusCnf), CIFX_TO_SEND_PACKET)))
  {
    if(CIFX_NO_ERROR == (lRet = tSystemStatusCnf.tHead.ulSta))
    {
      if (0 != (tSystemStatusCnf.tData.tSystemState.ulSystemStatus&RCX_SYS_STATUS_SYSVOLUME_FFS))
        *pbTxnMode = TXN_MODE_PACKET_MONITOR;
      else
        *pbTxnMode = TXN_MODE_PACKET_CAPTURE;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Get device information (board information struct)
*   \param hDevice         Device handle
*   \param ptBoardInfo     Buffer to device information
*   \param ptDrvFnc        Driver function table
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t GetDeviceInfo(CIFXHANDLE hDevice, BOARD_INFORMATION* ptBoardInfo, PDRVFNC_T ptDrvFnc)
{
  RCX_READ_SYS_INFO_BLOCK_REQ_T tSysInfoReq = {{0}};
  RCX_READ_SYS_INFO_BLOCK_CNF_T tSysInfoCnf = {{0}};
  int32_t                       lRet        = CIFX_NO_ERROR;

  tSysInfoReq.tHead.ulSrc    = 0;
  tSysInfoReq.tHead.ulDest   = RCX_PACKET_DEST_SYSTEM;
  tSysInfoReq.tHead.ulCmd    = RCX_SYSTEM_INFORMATION_BLOCK_REQ;

  /* Request serial and device number of our device */
  if (CIFX_NO_ERROR == (lRet = TransferPacket(hDevice, ptDrvFnc, (CIFX_PACKET*)&tSysInfoReq, 
                                              (CIFX_PACKET*)&tSysInfoCnf, sizeof(tSysInfoCnf), CIFX_TO_SEND_PACKET)))
  {
    if(CIFX_NO_ERROR == (lRet = tSysInfoCnf.tHead.ulSta))
    {
      uint32_t ulBoardNr  = 0;

      /* Iterate of local devices and find the corresponding board information structure 
         via device and serial number */
      do
      {
        BOARD_INFORMATION tBoardInfo = {0};
        if (CIFX_NO_ERROR == (lRet = s_pfnDriverEnumBoards(NULL, ulBoardNr, sizeof(tBoardInfo), &tBoardInfo)))
        {
          if ( (tBoardInfo.tSystemInfo.ulDeviceNumber == tSysInfoCnf.tData.tSystemInfo.ulDeviceNumber) &&
               (tBoardInfo.tSystemInfo.ulSerialNumber == tSysInfoCnf.tData.tSystemInfo.ulSerialNumber)   )
          {
            /* We found our device info */
            *ptBoardInfo = tBoardInfo;
            break;
          }
        }
        /* Enum next device */
        ulBoardNr++;

      } while (CIFX_NO_ERROR == lRet);
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Create Download confirmation packet for current transaction
*   \param hTxnRsrc      Transaction resource
*   \param ptReqPkt      Request packet
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t CreateTxnCnf(void* hTxnRsrc, CIFX_PACKET* ptReqPkt)
{
  PTXN_RSRC_T ptTxnRsrc = (PTXN_RSRC_T)hTxnRsrc;

  if ( (NULL == hTxnRsrc) ||
       (NULL == ptReqPkt)   )
    return CIFX_INVALID_POINTER;

  switch (ptReqPkt->tHeader.ulCmd)
  {
    /* Create confirmation packet for a download request */
    case RCX_FILE_DOWNLOAD_REQ:
    {
      RCX_FILE_DOWNLOAD_REQ_T* ptDownloadReq  = (RCX_FILE_DOWNLOAD_REQ_T*)ptReqPkt;
      RCX_FILE_DOWNLOAD_CNF_T* ptDownloadConf = (RCX_FILE_DOWNLOAD_CNF_T*)OS_Malloc(sizeof(RCX_FILE_DOWNLOAD_CNF_T));

      ptDownloadConf->tHead            = ptDownloadReq->tHead;
      ptDownloadConf->tHead.ulCmd      = ptDownloadReq->tHead.ulCmd|RCX_MSK_PACKET_ANSWER;
      ptDownloadConf->tHead.ulExt      = 0;
      ptDownloadConf->tHead.ulLen      = (ptTxnRsrc->lError != 0)?0:sizeof(ptDownloadConf->tData);
      ptDownloadConf->tHead.ulSta      = ptTxnRsrc->lError;
      ptDownloadConf->tData.ulMaxBlockSize = ptDownloadReq->tData.ulMaxBlockSize;

      /* Store confirmation packet in transaction resource */
      ptTxnRsrc->ptConfPkt = (CIFX_PACKET*)ptDownloadConf;
      break;
    }

    /* Create confirmation packet for a download data request */
    case RCX_FILE_DOWNLOAD_DATA_REQ:
    {
      RCX_FILE_DOWNLOAD_DATA_REQ_T* ptDownloadDataReq  = (RCX_FILE_DOWNLOAD_DATA_REQ_T*)ptReqPkt;
      RCX_FILE_DOWNLOAD_DATA_CNF_T* ptDownloadDataConf = (RCX_FILE_DOWNLOAD_DATA_CNF_T*)OS_Malloc(sizeof(RCX_FILE_DOWNLOAD_DATA_CNF_T));
      
      ptDownloadDataConf->tHead            = ptDownloadDataReq->tHead;
      ptDownloadDataConf->tHead.ulCmd      = ptDownloadDataReq->tHead.ulCmd|RCX_MSK_PACKET_ANSWER;
      ptDownloadDataConf->tHead.ulExt      = 0;
      ptDownloadDataConf->tHead.ulLen      = (ptTxnRsrc->lError != 0)?0:sizeof(ptDownloadDataConf->tData);
      ptDownloadDataConf->tHead.ulSta      = ptTxnRsrc->lError;
      ptDownloadDataConf->tData.ulExpectedCrc32 = ptDownloadDataReq->tData.ulChksum;

      /* Store confirmation packet in transaction resource */
      ptTxnRsrc->ptConfPkt = (CIFX_PACKET*)ptDownloadDataConf;
      break;
    }

    /* Create confirmation packet for a download abort request */
    case RCX_FILE_DOWNLOAD_ABORT_REQ:
    {
      RCX_FILE_DOWNLOAD_ABORT_REQ_T* ptDownloadAbortReq  = (RCX_FILE_DOWNLOAD_ABORT_REQ_T*)ptReqPkt;
      RCX_FILE_DOWNLOAD_ABORT_CNF_T* ptDownloadAbortConf = (RCX_FILE_DOWNLOAD_ABORT_CNF_T*)OS_Malloc(sizeof(RCX_FILE_DOWNLOAD_ABORT_CNF_T));
      
      ptDownloadAbortConf->tHead            = ptDownloadAbortReq->tHead;
      ptDownloadAbortConf->tHead.ulCmd      = ptDownloadAbortReq->tHead.ulCmd|RCX_MSK_PACKET_ANSWER;
      ptDownloadAbortConf->tHead.ulExt      = 0;
      ptDownloadAbortConf->tHead.ulLen      = 0;
      ptDownloadAbortConf->tHead.ulSta      = ptTxnRsrc->lError;

      /* Store confirmation packet in transaction resource */
      ptTxnRsrc->ptConfPkt = (CIFX_PACKET*)ptDownloadAbortConf;
      break;
    }
  }
  
  return CIFX_NO_ERROR;
}

/*****************************************************************************/
/*! Remove Transaction resource
*   \param hDevice      Device handle                                        */
/*****************************************************************************/
static void RemoveTxnRsrc(CIFXHANDLE hDevice)
{
  PTXN_RSRC_T ptTxnRsrcCur = s_ptTxnList;
  PTXN_RSRC_T ptTxnRsrcOld = NULL;
  
  /* Iterate the transaction resource list */
  while (ptTxnRsrcCur)
  {
    
    if (ptTxnRsrcCur->hDevice == hDevice)
    {
      /* We found the transaction resource which 
         belongs to our device */
      break;
    }
    
    ptTxnRsrcOld = ptTxnRsrcCur;
    ptTxnRsrcCur = ptTxnRsrcCur->pNext;
  }

  if (NULL != ptTxnRsrcCur)
  {
    if (NULL == ptTxnRsrcOld)
      s_ptTxnList = ptTxnRsrcCur->pNext;
    else
      ptTxnRsrcOld->pNext = ptTxnRsrcCur->pNext;

    if (ptTxnRsrcCur->pbFileBuf)
      OS_Free(ptTxnRsrcCur->pbFileBuf);

    if (ptTxnRsrcCur->ptConfPkt)
      OS_Free(ptTxnRsrcCur->ptConfPkt);

    OS_Free(ptTxnRsrcCur);
  }
}

/*****************************************************************************/
/*! Create transaction resource
*   \param hDevice           Device handle
*   \param ptBoardInfo       Board information
*   \param bTxnMode          Download transaction mode (monitor or capture)
*   \param pszFileName       File name
*   \param ulFileLength      File size
*   \param ulDownloadMode    Download mode (firmware, config, module or file download)
*   \param ulChannel         Download destination channel
*   \param ulMaxBlockSize    Maximal block size per packet
*   \return Pointer to download transaction resource, NULL if failed         */
/*****************************************************************************/
static void* CreateTxnRsrc(CIFXHANDLE hDevice, BOARD_INFORMATION* ptBoardInfo,
                           uint8_t bTxnMode, char* pszFileName, uint32_t ulFileLength, 
                           uint32_t ulDownloadMode, uint32_t ulChannel, uint32_t ulMaxBlockSize)
{
  PTXN_RSRC_T ptTxnRsrc = NULL;

  if (NULL == (ptTxnRsrc = (PTXN_RSRC_T)OS_Malloc(sizeof(TXN_RSRC_T))))
  {
    ptTxnRsrc = NULL;

  } else if (NULL == (ptTxnRsrc->pbFileBuf = (uint8_t*)OS_Malloc(ulFileLength)))
  {
    OS_Free(ptTxnRsrc);
    ptTxnRsrc = NULL;

  } else
  {
    ptTxnRsrc->ulState        = TXN_STATE_DOWNLOAD_REQUEST; /* Initial transaction state */
    ptTxnRsrc->pabActData     = ptTxnRsrc->pbFileBuf;
    ptTxnRsrc->ulFilesize     = ulFileLength;
    ptTxnRsrc->ulDownloadMode = ulDownloadMode;
    ptTxnRsrc->ulMaxBlockSize = ulMaxBlockSize; /* Maximum block size the host can handle. This could be
                                                   replaced by smaller value from the devices capability. */
    ptTxnRsrc->lError         = 0;
    ptTxnRsrc->bTxnMode       = bTxnMode;
    ptTxnRsrc->ulChannel      = ulChannel;
    ptTxnRsrc->hDevice        = hDevice;
    ptTxnRsrc->ptConfPkt      = NULL;
    ptTxnRsrc->tBoardInfo     = *ptBoardInfo;

    OS_STRNCPY(ptTxnRsrc->szFilename, pszFileName, sizeof(ptTxnRsrc->szFilename)/sizeof(ptTxnRsrc->szFilename[0]));
    
    /* Join the transaction resource list */
    ptTxnRsrc->pNext = s_ptTxnList;
    s_ptTxnList      = ptTxnRsrc;
  }
  return (void*)ptTxnRsrc;
}

/*****************************************************************************/
/*! Get transaction resource by device handle
*   \param hDevice           Device handle
*   \return Pointer to transaction resource                                  */
/*****************************************************************************/
static void* GetTxnRsrc(CIFXHANDLE hDevice)
{
  PTXN_RSRC_T ptTxnRsrc = s_ptTxnList;
  
  /* Iterate the transaction resource list */
  while (ptTxnRsrc)
  {
    if (ptTxnRsrc->hDevice == hDevice)
    {
      /* We found the transaction resource which 
         belongs to our device */
      break;
    }
    
    ptTxnRsrc = ptTxnRsrc->pNext;
  }
  
  return ptTxnRsrc;
}

/*****************************************************************************/
/*! Get mailbox state function with file download extension
*   \param hDevice           Device handle
*   \param pulRecvPktCount   Number of Messages waiting in receive mailbox
*   \param pulSendPktCount   State of the Send Mailbox
*   \param ptDrvFnc          Driver function table
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t xDownloadHook_GetMBXState(CIFXHANDLE hDevice, uint32_t* pulRecvPktCount, 
                                         uint32_t* pulSendPktCount, PDRVFNC_T ptDrvFnc)
{
  int32_t lRet = CIFX_NO_ERROR;

  if (CIFX_NO_ERROR == (lRet = ptDrvFnc->pfnGetMBXState(hDevice, pulRecvPktCount, pulSendPktCount)))
  {
    PTXN_RSRC_T ptTxnRsrc      = NULL;
    
    /* Check if the device has a active download transaction */
    if (NULL != (ptTxnRsrc = GetTxnRsrc(hDevice)))
    {
      /* We have a active download transaction. If we are dealing with
          the capture mode, there might be a confirmation packet. We add
          this packet to our receive packet counter. */
      if ( (ptTxnRsrc->bTxnMode == TXN_MODE_PACKET_CAPTURE) &&
           (ptTxnRsrc->ptConfPkt)                             )
      {
        (void)*pulRecvPktCount++;
      }
    }
  }  
  
  return lRet;
}

/*****************************************************************************/
/*! Get packet function with file download extension
*   \param hDevice           Device handle
*   \param ulRecvBufferSize  Size of receive buffer
*   \param ptRecvPkt         Buffer to store received packet
*   \param ulTimeout         Timeout value
*   \param ptDrvFnc          Driver function table
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t xDownloadHook_GetPacket(CIFXHANDLE hDevice, uint32_t ulRecvBufferSize, 
                                       CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout, 
                                       PDRVFNC_T ptDrvFnc)
{
  int32_t     lRet      = CIFX_NO_ERROR;
  PTXN_RSRC_T ptTxnRsrc = NULL;
  
  /* Check if the device has a active download transaction */
  if (NULL == (ptTxnRsrc = GetTxnRsrc(hDevice)))
  {
    /* No active download transaction. Receive packet from device */
    lRet = ptDrvFnc->pfnGetPacket(hDevice, ulRecvBufferSize, ptRecvPkt, ulTimeout);

  } else
  {
    /* We have a active download transaction. If we are dealing with
       the capture mode, there might be a confirmation packet, which 
       has to be transmitted to the host */
    if ( (ptTxnRsrc->bTxnMode == TXN_MODE_PACKET_CAPTURE) &&
         (ptTxnRsrc->ptConfPkt)                             )
    {
      uint32_t ulCopySize = ptTxnRsrc->ptConfPkt->tHeader.ulLen + RCX_PACKET_HEADER_SIZE;
      if(ulCopySize > ulRecvBufferSize)
      {
        /* We have to free the mailbox, read as much as possible */
        ulCopySize = ulRecvBufferSize;
        lRet = CIFX_BUFFER_TOO_SHORT;
      }
      
      OS_MEMCPY(ptRecvPkt, ptTxnRsrc->ptConfPkt, ulCopySize);
      
      /* The packet is copied to the receive buffer. So free the packet from
         the transaction resource */
      OS_Free(ptTxnRsrc->ptConfPkt);
      ptTxnRsrc->ptConfPkt = NULL;
    } else
    {
      /* We have a active download transaction, but no confirmation packet waiting.
        Call get packet routine to receive packet from device */
      lRet = ptDrvFnc->pfnGetPacket(hDevice, ulRecvBufferSize, ptRecvPkt, ulTimeout);
    }

    switch (ptTxnRsrc->ulState)
    {
      case TXN_STATE_DOWNLOAD_REQUEST:
        if ( (RCX_FILE_DOWNLOAD_CNF == ptRecvPkt->tHeader.ulCmd) &&
             (CIFX_NO_ERROR         == ptRecvPkt->tHeader.ulState) )
        {
          RCX_FILE_DOWNLOAD_CNF_T* ptDownloadConf = (RCX_FILE_DOWNLOAD_CNF_T*)ptRecvPkt;
          /* Get download packet size from the device confirmation.
             If the devices packet size is smaller than our size, use the length from the device.
             Otherwise use our length. */          
          if (ptDownloadConf->tData.ulMaxBlockSize < ptTxnRsrc->ulMaxBlockSize)
            ptTxnRsrc->ulMaxBlockSize = ptDownloadConf->tData.ulMaxBlockSize;
          
          /* We are now expecting file data packets, so set new state */
          ptTxnRsrc->ulState = TXN_STATE_DATA_TRANSFER;
        }
        break;

      case TXN_STATE_DATA_TRANSFER:
        break;

      case TXN_STATE_FINISHED:
          /* We captured the whole file now. Execute callback to handle the file storage */
          lRet = s_pfnFileStorageCbk(&ptTxnRsrc->tBoardInfo, ptTxnRsrc->szFilename, ptTxnRsrc->ulFilesize, 
                                     ptTxnRsrc->pbFileBuf, ptTxnRsrc->ulChannel, ptTxnRsrc->ulDownloadMode, s_pvUser);
          /* No break here. Download transaction ressource can be removed now */

      case TXN_STATE_ERROR:
        /* Remove the transaction resource if the download has finished or failed */
        RemoveTxnRsrc(hDevice);
        break;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Put packet function with file download extension
*   \param hDevice           Device handle
*   \param ptSendPkt         Packet to send
*   \param ulTimeout         Timeout value
*   \param ptDrvFnc          Driver function table
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t xDownloadHook_PutPacket(CIFXHANDLE hDevice, CIFX_PACKET* ptSendPkt, 
                                       uint32_t ulTimeout, PDRVFNC_T ptDrvFnc)
{
  PTXN_RSRC_T ptTxnRsrc = NULL;
  uint32_t    ulState   = TXN_STATE_DOWNLOAD_REQUEST;
  int32_t     lRet      = CIFX_NO_ERROR;

  /* Check if we are dealing with a download packet. If not, just route
     the packet to the device */
  if ( (RCX_FILE_DOWNLOAD_REQ       != ptSendPkt->tHeader.ulCmd) &&
       (RCX_FILE_DOWNLOAD_DATA_REQ  != ptSendPkt->tHeader.ulCmd) &&
       (RCX_FILE_DOWNLOAD_ABORT_REQ != ptSendPkt->tHeader.ulCmd)   )
  {
    return ptDrvFnc->pfnPutPacket(hDevice, ptSendPkt, ulTimeout);
  }
  
  /* Check if there is a active download transaction and resume with
     the current transaction state */
  if (NULL != (ptTxnRsrc = GetTxnRsrc(hDevice)))
  {
    ulState = ptTxnRsrc->ulState;
  }

  switch (ulState)
  {
    case TXN_STATE_FINISHED:
    case TXN_STATE_ERROR:
      /* A previous download transaction was not completed.
         We are now removing old transaction ressource as we
         dont need it anymore */
      RemoveTxnRsrc(hDevice);
      /* No break here as we are now handle the new download request */

    case TXN_STATE_DOWNLOAD_REQUEST:
      if (RCX_FILE_DOWNLOAD_REQ == ptSendPkt->tHeader.ulCmd)
      {
        RCX_FILE_DOWNLOAD_REQ_T* ptDownloadReq  = (RCX_FILE_DOWNLOAD_REQ_T*)ptSendPkt;
        char*                    pszFileName    = (char*)(&ptDownloadReq->tData + 1);
        uint32_t                 ulDownloadMode = DOWNLOAD_MODE_FILE;

        /* Get Download mode from file name */
        ulDownloadMode = GetDownloadModeFromFileName( ptDownloadReq->tData.usFileNameLength-1, 
                                                      pszFileName);

        /* We are only capturing the file if we are dealing with a firmware, module or configuration */
        if ( (ulDownloadMode == DOWNLOAD_MODE_FIRMWARE) ||
             (ulDownloadMode == DOWNLOAD_MODE_CONFIG)   ||
             (ulDownloadMode == DOWNLOAD_MODE_MODULE)     )
        {
          uint8_t  bTxnMode            = TXN_MODE_PACKET_CAPTURE;
          BOARD_INFORMATION tBoardInfo = {0};

          /* Set mode on the basis of device type:
             ram based device   = capture mode 
             flash based device = monitore mode */
          if (CIFX_NO_ERROR != GetTxnMode(hDevice, &bTxnMode, ptDrvFnc))
          {
            /* Determine device type failed. */

          } else if (CIFX_NO_ERROR != GetDeviceInfo(hDevice, &tBoardInfo, ptDrvFnc))
          {
            /* Failed to get device information */

          } else if (NULL == (ptTxnRsrc = CreateTxnRsrc( hDevice, &tBoardInfo, bTxnMode, 
                                                         pszFileName, ptDownloadReq->tData.ulFileLength,
                                                         ulDownloadMode, ptDownloadReq->tData.ulChannelNo,
                                                         ptDownloadReq->tData.ulMaxBlockSize)))
          {
            /* Failed to create download transaction resource */

          } else
          {
            /* We are now expecting file data packets. The transaction state will be change
               to TXN_STATE_DATA_TRANSFER if download confirmation packet was received sucessfully */
          }
        }
      }
      break;
    
    case TXN_STATE_DATA_TRANSFER:
      if (RCX_FILE_DOWNLOAD_DATA_REQ == ptSendPkt->tHeader.ulCmd)
      {
        RCX_FILE_DOWNLOAD_DATA_REQ_T* ptDownloadDataReq = (RCX_FILE_DOWNLOAD_DATA_REQ_T*)ptSendPkt;

        if( (RCX_PACKET_SEQ_FIRST  == ptDownloadDataReq->tHead.ulExt) ||
            (RCX_PACKET_SEQ_MIDDLE == ptDownloadDataReq->tHead.ulExt)   )
        {
          /* This is a first or middle data packet of a transaction sequence. 
            So the maximum block size will be copied to the file buffer */
          OS_MEMCPY( ptTxnRsrc->pabActData, &ptDownloadDataReq->tData + 1, ptTxnRsrc->ulMaxBlockSize);
          ptTxnRsrc->pabActData += ptTxnRsrc->ulMaxBlockSize;

        } else if( (RCX_PACKET_SEQ_LAST == ptDownloadDataReq->tHead.ulExt) ||
                   (RCX_PACKET_SEQ_NONE == ptDownloadDataReq->tHead.ulExt) )
        {
          /* This is the last data packet of a transaction sequence or a 
             non sequenced packet. We have to calculate the data block size
             as it might be smaller than the maximum block size */
          uint32_t ulBlockSize = ptDownloadDataReq->tHead.ulLen - sizeof(RCX_FILE_DOWNLOAD_DATA_REQ_DATA_T);
          OS_MEMCPY( ptTxnRsrc->pabActData, &ptDownloadDataReq->tData + 1, ulBlockSize);

          /* Download finished, so set state respectively */
          ptTxnRsrc->ulState  = TXN_STATE_FINISHED;
        }

      } else if (RCX_FILE_DOWNLOAD_ABORT_REQ == ptSendPkt->tHeader.ulCmd)
      {
        ptTxnRsrc->ulState  = TXN_STATE_ERROR;
      } else
      {
        ptTxnRsrc->lError   = RCX_E_PACKET_OUT_OF_SEQ;
        ptTxnRsrc->ulState  = TXN_STATE_ERROR;
      }
      break;
  }
  
  if ( (NULL                    != ptTxnRsrc)                  &&
       (TXN_MODE_PACKET_CAPTURE == ptTxnRsrc->bTxnMode )       &&
       (DOWNLOAD_MODE_FIRMWARE  == ptTxnRsrc->ulDownloadMode )    )
  {
    /* On ram based devices (packet capture mode) firmware downloads
       are not possible. Hence we need to capture the download request
       and create the confirmation packets by ourselves. In this case 
       the request packets must not be routed to the device */
    lRet = CreateTxnCnf(ptTxnRsrc, ptSendPkt);

  } else
  {
    /* Every download request on flash based devices (packet monitor mode) 
       can be handled by the device. So the download packets are just monitored 
       and afterwards routed to the device */
    lRet = ptDrvFnc->pfnPutPacket(hDevice, ptSendPkt, ulTimeout);
  }
  
  return lRet;
}

/*****************************************************************************/
/*! Gets the Mailbox state of an open system device
*   \param hSysdevice      Handle to the System device
*   \param pulRecvPktCount Number of packets in receive mailbox
*   \param pulSendPktCount Number of packets the application is able to send at once
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY xSysdeviceGetMBXStateWrapper(CIFXHANDLE hSysdevice, uint32_t* pulRecvPktCount, uint32_t* pulSendPktCount)
{
  return xDownloadHook_GetMBXState(hSysdevice, pulRecvPktCount, pulSendPktCount, &s_tDRVFncSysdevice);
}

/*****************************************************************************/
/*! Wrapper for xSysdevicePutPacket to handle download requests
*   \param hSysdevice      Handle to the System device
*   \param ptSendPkt       Packet to send to device
*   \param ulTimeout       maximum time to wait for packet to be accepted by device (in ms)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY xSysdevicePutPacketWrapper(CIFXHANDLE hSysdevice, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout)
{
  return xDownloadHook_PutPacket(hSysdevice, ptSendPkt, ulTimeout, &s_tDRVFncSysdevice);
}

/*****************************************************************************/
/*! Wrapper for xSysdeviceGetPacket to handle download requests
*   \param hSysdevice      Handle to the System device
*   \param ulSize          Size of the buffer to retrieve the packet
*   \param ptRecvPkt       Pointer to buffer for received packet
*   \param ulTimeout       maximum time to wait for packet to be delivered by device (in ms)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY xSysdeviceGetPacketWrapper(CIFXHANDLE hSysdevice, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout)
{
  return xDownloadHook_GetPacket(hSysdevice, ulSize, ptRecvPkt, ulTimeout, &s_tDRVFncSysdevice);
}

/*****************************************************************************/
/*! Returns the Mailbox state from a specific channel
*   \param hChannel         Channel handle acquired by xChannelOpen
*   \param pulRecvPktCount  Number of Messages waiting in receive mailbox
*   \param pulSendMbxState  State of the Send Mailbox
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY xChannelGetMBXStateWrapper(CIFXHANDLE hChannel, uint32_t* pulRecvPktCount, uint32_t* pulSendMbxState)
{
  return xDownloadHook_GetMBXState(hChannel, pulRecvPktCount,pulSendMbxState, &s_tDRVFncChannel);
}

/*****************************************************************************/
/*! Wrapper for xChannelPutPacket to handle download requests
*   \param hChannel        Handle to the System device
*   \param ptSendPkt       Packet to send to device
*   \param ulTimeout       maximum time to wait for packet to be accepted by device (in ms)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY xChannelPutPacketWrapper( CIFXHANDLE hChannel, CIFX_PACKET*  ptSendPkt, uint32_t ulTimeout)
{
  return xDownloadHook_PutPacket(hChannel, ptSendPkt, ulTimeout, &s_tDRVFncChannel);
}

/*****************************************************************************/
/*! Wrapper for xChannelGetPacket to handle download requests
*   \param hChannel        Handle to the System device
*   \param ulSize          Size of the buffer to retrieve the packet
*   \param ptRecvPkt       Pointer to buffer for received packet
*   \param ulTimeout       maximum time to wait for packet to be delivered by device (in ms)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t APIENTRY xChannelGetPacketWrapper( CIFXHANDLE  hChannel, uint32_t ulSize, CIFX_PACKET* ptRecvPkt, uint32_t ulTimeout)
{
  return xDownloadHook_GetPacket(hChannel, ulSize, ptRecvPkt, ulTimeout, &s_tDRVFncChannel);
}

/*****************************************************************************/
/*! Initialize download hook
*   \param ptDRVFunctions    Driver function table
*   \param pfnFileStorageCbk File storage callback
*   \param pvUser            User pointer for callback function
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t xDownloadHook_Install( PDRIVER_FUNCTIONS ptDRVFunctions, PFN_FILESTORAGE_CBK pfnFileStorageCbk, void* pvUser) 
{
  /* There must be an file storage callback */
  if (NULL == pfnFileStorageCbk)
    return CIFX_INVALID_POINTER;

  /* To hook the file download we need at least the following driver functions */
  if ( (NULL == ptDRVFunctions->pfnxSysdevicePutPacket)   ||
       (NULL == ptDRVFunctions->pfnxSysdeviceGetPacket)   ||
       (NULL == ptDRVFunctions->pfnxSysdeviceGetMBXState) ||
       (NULL == ptDRVFunctions->pfnxChannelPutPacket)     ||
       (NULL == ptDRVFunctions->pfnxChannelGetPacket)     ||
       (NULL == ptDRVFunctions->pfnxChannelGetMBXState)   || 
       (NULL == ptDRVFunctions->pfnxDriverEnumBoards)       )
    return CIFX_INVALID_POINTER;

  /* Assign sysdevice functions */
  s_tDRVFncSysdevice.pfnPutPacket     = ptDRVFunctions->pfnxSysdevicePutPacket;
  s_tDRVFncSysdevice.pfnGetPacket     = ptDRVFunctions->pfnxSysdeviceGetPacket;
  s_tDRVFncSysdevice.pfnGetMBXState   = ptDRVFunctions->pfnxSysdeviceGetMBXState;

  /* Assign channel functions */
  s_tDRVFncChannel.pfnPutPacket     = ptDRVFunctions->pfnxChannelPutPacket;
  s_tDRVFncChannel.pfnGetPacket     = ptDRVFunctions->pfnxChannelGetPacket;
  s_tDRVFncChannel.pfnGetMBXState   = ptDRVFunctions->pfnxChannelGetMBXState;

  s_pfnDriverEnumBoards = ptDRVFunctions->pfnxDriverEnumBoards;

  /* Install wrapper functions in driver table */
  ptDRVFunctions->pfnxSysdevicePutPacket   = xSysdevicePutPacketWrapper;
  ptDRVFunctions->pfnxSysdeviceGetPacket   = xSysdeviceGetPacketWrapper;
  ptDRVFunctions->pfnxSysdeviceGetMBXState = xSysdeviceGetMBXStateWrapper;

  ptDRVFunctions->pfnxChannelPutPacket   = xChannelPutPacketWrapper;
  ptDRVFunctions->pfnxChannelGetPacket   = xChannelGetPacketWrapper;
  ptDRVFunctions->pfnxChannelGetMBXState = xChannelGetMBXStateWrapper;

  /* Set file storage callback */
  s_pfnFileStorageCbk = pfnFileStorageCbk;
  s_pvUser            = pvUser;

  return CIFX_NO_ERROR;
}

/*****************************************************************************/
/*! Deinit download hook
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t xDownloadHook_Remove( void) 
{
  /* TODO: implement remove hook function */
  return CIFX_FUNCTION_NOT_AVAILABLE;
}
