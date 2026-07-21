/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXCommonHWFunctions.c 15329 2025-11-24 13:34:32Z AMinor $:

  Description:
    Common cifX HW functions and variables shared by DPM and HIF

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-07-14  created

**************************************************************************************/

/*****************************************************************************/
/*! \file cifXCommonHHFunctions.c
*   Common cifX HW API functions                                             */
/*****************************************************************************/

#include "cifXToolkit.h"
#include "cifXEndianess.h"
#include "cifXErrors.h"
#include "cifXHWFunctionsWrapper.h"
#include "Hilcrc32.h"

#include <Hil_Results.h>

/*****************************************************************************/
/*! Download a file to the hardware
*   \param pvChannel          Channel instance the download is performed on
*   \param ulChannel          Channel number the download is for
*   \param ulMailboxSize      Size of the mailbox
*   \param ulTransferType     Type of transfer (see HIL_FILE_XFER_XXX defines)
*   \param szFileName         Short file name (needed by firmware to create the file by name)
*   \param ulFileLength       Length of the file to download
*   \param pvData             File data being downloaded
*   \param pfnTransferPacket  Function used for transferring packets
*   \param pfnCallback        User callback for download progress indications
*   \param pfnRecvPktCallback User callback for unsolicited receive packets
*   \param pvUser             User parameter passed on callback
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_DownloadFile(void*                 pvChannel,
                         uint32_t              ulChannel,
                         uint32_t              ulMailboxSize,
                         uint32_t              ulTransferType,
                         char*                 szFileName,
                         uint32_t              ulFileLength,
                         void*                 pvData,
                         PFN_TRANSFER_PACKET   pfnTransferPacket,
                         PFN_PROGRESS_CALLBACK pfnCallback,
                         PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                         void*                 pvUser)
{
  union
  {
    CIFX_PACKET                   tPacket;
    HIL_FILE_DOWNLOAD_REQ_T       tDownloadReq;
    HIL_FILE_DOWNLOAD_DATA_REQ_T  tDownloadDataReq;
    HIL_FILE_DOWNLOAD_ABORT_REQ_T tAbortReq;
  }          uSendPkt;
  union
  {
    CIFX_PACKET                   tPacket;
    HIL_FILE_DOWNLOAD_CNF_T       tDownloadCnf;
    HIL_FILE_DOWNLOAD_DATA_CNF_T  tDownloadDataCnf;
    HIL_FILE_DOWNLOAD_ABORT_CNF_T tAbortCnf;
  }          uRecvPkt;

  /* Set download state informations */
  uint32_t   ulMaxDataLength     = ulMailboxSize -  /* Maximum possible user data length */
                                   (uint32_t)sizeof(HIL_FILE_DOWNLOAD_DATA_REQ_T);

  char*      pbCopyPtr           = NULL;
  uint32_t   ulCopySize          = 0;
  uint32_t   ulSendLen           = 0;
  uint32_t   ulTransferedLength  = 0;
  uint8_t*   pabActData          = NULL;
  uint32_t   ulCRC               = 0;
  uint32_t   ulBlockNumber       = 0;
  uint32_t   ulState             = HIL_FILE_DOWNLOAD_REQ;
  uint32_t   ulCmdDataState      = HIL_PACKET_SEQ_NONE;
  int        fStopDownload       = 0;
  int32_t    lRetAbort           = CIFX_NO_ERROR;
  int32_t    lRet                = CIFX_NO_ERROR;
  uint32_t   ulCurrentId         = 0;
  uint32_t   ulSrc               = OS_GetMilliSecCounter(); /* Early versions used pvChannel as ulSrc,
                                                               but this won't work on 64 Bit machines.
                                                               As we need something unique we use the current system time */
  uint32_t   ulTransferTimeout   = CIFX_TO_SEND_PACKET;

  OS_Memset(&uSendPkt, 0, sizeof(uSendPkt));
  OS_Memset(&uRecvPkt, 0, sizeof(uRecvPkt));

  /* Check parameters */
  if( NULL == pvData)
    return CIFX_INVALID_POINTER;

  if( 0 == ulFileLength)
    return CIFX_INVALID_PARAMETER;

  pabActData = (uint8_t*)pvData;

  /* Performce download */
  do
  {
    switch (ulState)
    {
      /* Send download request */
      case HIL_FILE_DOWNLOAD_REQ:
      {
        /* Validate filename length to fit mailbox/packet */
        uint32_t ulFileNameLength = HIL_MIN( ((uint32_t)OS_Strlen(szFileName) + 1),
                                             (ulMailboxSize - (uint32_t)sizeof(HIL_FILE_DOWNLOAD_REQ_T))); /*lint !e666 : function call OS_Strlen() */

        /* Insert packet data */
        ++ulCurrentId;
        uSendPkt.tDownloadReq.tHead.ulDest   = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
        uSendPkt.tDownloadReq.tHead.ulSrc    = HOST_TO_LE32(ulSrc);
        uSendPkt.tDownloadReq.tHead.ulDestId = HOST_TO_LE32(0);
        uSendPkt.tDownloadReq.tHead.ulSrcId  = HOST_TO_LE32(0);
        uSendPkt.tDownloadReq.tHead.ulLen    = HOST_TO_LE32((uint32_t)(sizeof(HIL_FILE_DOWNLOAD_REQ_DATA_T) +
                                                                       ulFileNameLength));
        uSendPkt.tDownloadReq.tHead.ulId     = HOST_TO_LE32(ulCurrentId);
        uSendPkt.tDownloadReq.tHead.ulSta    = HOST_TO_LE32(0);
        uSendPkt.tDownloadReq.tHead.ulCmd    = HOST_TO_LE32(HIL_FILE_DOWNLOAD_REQ);
        uSendPkt.tDownloadReq.tHead.ulExt    = HOST_TO_LE32(ulCmdDataState);
        uSendPkt.tDownloadReq.tHead.ulRout   = HOST_TO_LE32(0);

        /* Insert command data (extended data) */
        uSendPkt.tDownloadReq.tData.ulFileLength     = HOST_TO_LE32(ulFileLength);
        uSendPkt.tDownloadReq.tData.ulMaxBlockSize   = HOST_TO_LE32(ulMaxDataLength);
        uSendPkt.tDownloadReq.tData.ulXferType       = HOST_TO_LE32(ulTransferType);
        uSendPkt.tDownloadReq.tData.ulChannelNo      = HOST_TO_LE32(ulChannel);
        uSendPkt.tDownloadReq.tData.usFileNameLength = HOST_TO_LE16((uint16_t)ulFileNameLength);

        /* Setup copy buffer and copy size */
        pbCopyPtr   = ((char*)(&uSendPkt.tPacket.abData[0])) + sizeof(uSendPkt.tDownloadReq.tData);
        ulCopySize  = HIL_MIN( (sizeof(uSendPkt.tPacket.abData) - sizeof(uSendPkt.tDownloadReq.tData)), uSendPkt.tDownloadReq.tData.usFileNameLength);

        /* Insert file name */
        (void)OS_Strncpy( pbCopyPtr, szFileName, ulCopySize);

        /* Usually a file system is used for file storage, but there are configurations
         * that use RAW FLASH instead (e.g. netX90 usecase A/B). Deleting a file inside
         * the file system is usually quite quick, but erasing a FLASH area can take a
         * considerable amount of time (depending on the area size and the erase time
         * of the FLASH chip). That's why we try to calculate the necessary timeout
         * value for a successful first download packet. If the netX requires less time,
         * the function will return sooner. The following assumptions are made here:
         * Erase sector size: 4096, Erase sector time: 150ms.
         */

        /* Transfer packet */
        lRet = pfnTransferPacket(pvChannel,
                                 &uSendPkt.tPacket,
                                 &uRecvPkt.tPacket,
                                 (uint32_t)sizeof(uRecvPkt.tPacket),
                                 ulTransferTimeout + ((ulFileLength / 4096) * 150),
                                 pfnRecvPktCallback,
                                 pvUser);

        if( (CIFX_NO_ERROR  != lRet)                                 ||
            (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST((int32_t)uRecvPkt.tDownloadCnf.tHead.ulSta))) )
        {
          /* Error during first packet, end download */
          /* Send progress notification */
          if(pfnCallback)
            pfnCallback(ulTransferedLength, ulFileLength, pvUser, CIFX_CALLBACK_FINISHED, lRet);

          /* Send abort request on unusable data */
          ulState = HIL_FILE_DOWNLOAD_ABORT_REQ;
        } else if( LE32_TO_HOST(uRecvPkt.tDownloadCnf.tData.ulMaxBlockSize) == 0)
        {
          /* Error in device information, stop download (Device returned illegal block size */
          lRet = CIFX_INVALID_ACCESS_SIZE;

          /* Send progress notification */
          if(pfnCallback)
            pfnCallback(ulTransferedLength, ulFileLength, pvUser, CIFX_CALLBACK_FINISHED, lRet);

          /* Send abort request on unusable data */
          ulState = HIL_FILE_DOWNLOAD_ABORT_REQ;
        } else
        {
          /* Everything went ok, so start transmitting file data now */
          /* Get download packet size from the device confirmation.
             If the devices packet size is smaller than our size, use the length from the device.
             Otherwise use our length. */
          if( ulMaxDataLength > LE32_TO_HOST(uRecvPkt.tDownloadCnf.tData.ulMaxBlockSize))
            ulMaxDataLength = LE32_TO_HOST(uRecvPkt.tDownloadCnf.tData.ulMaxBlockSize);

          /* Check if the file fits into one packet or if we have to send multiple packets */
          ulSendLen = ulMaxDataLength;
          if(ulFileLength <= ulSendLen)
          {
            /* We have only one packet to send */
            ulSendLen       = ulFileLength;
            ulCmdDataState  = HIL_PACKET_SEQ_NONE;
          } else
          {
            /* We have to send multiple packets */
            ulCmdDataState  = HIL_PACKET_SEQ_FIRST;
          }

          /* Goto next state */
          ulState = HIL_FILE_DOWNLOAD_DATA_REQ;
        }
      }
      break;

      /* Data download packets */
      case HIL_FILE_DOWNLOAD_DATA_REQ:
      {
        ++ulCurrentId;
        uSendPkt.tDownloadDataReq.tHead.ulDest     = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
        uSendPkt.tDownloadDataReq.tHead.ulSrc      = HOST_TO_LE32(ulSrc);
        uSendPkt.tDownloadDataReq.tHead.ulCmd      = HOST_TO_LE32(HIL_FILE_DOWNLOAD_DATA_REQ);
        uSendPkt.tDownloadDataReq.tHead.ulId       = HOST_TO_LE32(ulCurrentId);
        uSendPkt.tDownloadDataReq.tHead.ulExt      = HOST_TO_LE32(ulCmdDataState);

        /* Copy file data to packet */
        OS_Memcpy( &uSendPkt.tDownloadDataReq.tData + 1, pabActData, ulSendLen);

        /* Adjust packet length */
        uSendPkt.tDownloadDataReq.tHead.ulLen      = HOST_TO_LE32((uint32_t)(sizeof(HIL_FILE_DOWNLOAD_DATA_REQ_DATA_T) +
                                                                             ulSendLen));

        /* Create continued CRC */
        ulCRC = CreateCRC32( ulCRC, pabActData, ulSendLen);
        uSendPkt.tDownloadDataReq.tData.ulChksum   = HOST_TO_LE32(ulCRC);
        uSendPkt.tDownloadDataReq.tData.ulBlockNo  = HOST_TO_LE32(ulBlockNumber);
        ++ulBlockNumber;

        /* Transfer packet */
        lRet = pfnTransferPacket(pvChannel,
                                 &uSendPkt.tPacket,
                                 &uRecvPkt.tPacket,
                                 (uint32_t)sizeof(uRecvPkt.tPacket),
                                 ulTransferTimeout,
                                 pfnRecvPktCallback,
                                 pvUser);

        if( (CIFX_NO_ERROR  != lRet)                                   ||
            (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST((int32_t)(uRecvPkt.tDownloadDataCnf.tHead.ulSta)))) )
        {
          /* Driver error during transfer packet, end download */
          /* Always try to send an abort request */
          if(pfnCallback)
            pfnCallback(ulTransferedLength, ulFileLength, pvUser, CIFX_CALLBACK_FINISHED, lRet);

          ulState = HIL_FILE_DOWNLOAD_ABORT_REQ;
        } else
        {
          /* Add send size to transferred size */
          ulTransferedLength += ulSendLen;

          /* Indicate progress, if user wants a notification */
          if(pfnCallback)
            pfnCallback(ulTransferedLength, ulFileLength, pvUser,
                        (ulTransferedLength == ulFileLength) ? CIFX_CALLBACK_FINISHED : CIFX_CALLBACK_ACTIVE,
                        lRet);

          /* Check if we are done with the download */
          if( (HIL_PACKET_SEQ_LAST == ulCmdDataState) ||
              (HIL_PACKET_SEQ_NONE == ulCmdDataState) )
          {
            /* No more packets to send, end download */
            fStopDownload = 1;
          } else
          {
            /* Move data pointer to next data */
            pabActData += ulSendLen;

            /* Calculate next message length */
            if ( ulFileLength <= (ulSendLen + ulTransferedLength))
            {
              /* Set the send length to rest of data,
                 This will be the last packet */
              ulSendLen = ulFileLength - ulTransferedLength;
              ulCmdDataState = HIL_PACKET_SEQ_LAST;

              /* ATTENTION: Check the transfer type */
              if ( HIL_FILE_XFER_MODULE == ulTransferType)
              {
                /* Module loading will relocate the module with the last packet.
                   So the confirmation packet takes longer, depending on the
                   file size (and contained firmware).
                   Measurements showed that for every 100kB the module needs
                   one additional second for relocation */
                ulTransferTimeout += (ulFileLength / (100 * 1024)) * 1000;
              }
            } else
            {
              ulCmdDataState = HIL_PACKET_SEQ_MIDDLE;
            }

            /* Goto next state */
            ulState = HIL_FILE_DOWNLOAD_DATA_REQ;
          }
        }
      }
      break;

      /* Abort active download */
      case HIL_FILE_DOWNLOAD_ABORT_REQ:
      {
        ++ulCurrentId;
        uSendPkt.tAbortReq.tHead.ulDest   = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
        uSendPkt.tAbortReq.tHead.ulSrc    = HOST_TO_LE32(ulSrc);
        uSendPkt.tAbortReq.tHead.ulDestId = HOST_TO_LE32(0);
        uSendPkt.tAbortReq.tHead.ulSrcId  = HOST_TO_LE32(0);
        uSendPkt.tAbortReq.tHead.ulLen    = HOST_TO_LE32(0);
        uSendPkt.tAbortReq.tHead.ulId     = HOST_TO_LE32(ulCurrentId);
        uSendPkt.tAbortReq.tHead.ulSta    = HOST_TO_LE32(0);
        uSendPkt.tAbortReq.tHead.ulCmd    = HOST_TO_LE32(HIL_FILE_DOWNLOAD_ABORT_REQ);
        uSendPkt.tAbortReq.tHead.ulExt    = HOST_TO_LE32(HIL_PACKET_SEQ_NONE);
        uSendPkt.tAbortReq.tHead.ulRout   = HOST_TO_LE32(0);

        /* Transfer packet */
        lRetAbort = pfnTransferPacket(pvChannel,
                                      &uSendPkt.tPacket,
                                      &uRecvPkt.tPacket,
                                      (uint32_t)sizeof(uRecvPkt.tPacket),
                                      ulTransferTimeout,
                                      pfnRecvPktCallback,
                                      pvUser);

        if( lRetAbort == CIFX_NO_ERROR)
        {
          /* Return packet state if function succeeded */
          lRetAbort = LE32_TO_HOST((int32_t)uRecvPkt.tAbortCnf.tHead.ulSta);
        }

        /* End download */
        fStopDownload = 1;
      }
      break;

      default:
        /* unknown, leave command */
        lRet = CIFX_FUNCTION_FAILED;

        /* End download */
        fStopDownload = 1;
        break;
    }

  } while(!fStopDownload);

  /* Always return lRet first, then abort error */
  if( CIFX_NO_ERROR != lRet)
    return lRet;
  else if( CIFX_NO_ERROR != lRetAbort)
    return lRetAbort;
  else
    return CIFX_NO_ERROR;
} /*lint !e429 : pvData not freed or returned */

/*****************************************************************************/
/*! Uploads a file from the hardware. It is required to list the files
* on the hardware, to know the file length for creating the buffer.
*   \param pvChannel          Channel instance the upload is performed on
*   \param ulChannel          Channel number the upload made is for
*   \param ulMailboxSize      Size of the mailbox
*   \param ulTransferType     Type of transfer (see HIL_FILE_XFER_XXX defines)
*   \param szFileName         Short file name
*   \param pulDataBufferLen   Length of the provided buffer, returned length of data
*   \param pvData             Buffer for storing upload. This buffer must be allocated by the caller.
*   \param pfnTransferPacket  Function used for transferring packets
*   \param pfnCallback        User callback for upload progress indications
*   \param pfnRecvPktCallback User callback for unsolicited receive packets
*   \param pvUser             User parameter passed on callback
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_UploadFile(void*                   pvChannel,
                       uint32_t                ulChannel,
                       uint32_t                ulMailboxSize,
                       uint32_t                ulTransferType,
                       char*                   szFileName,
                       uint32_t*               pulDataBufferLen,
                       void*                   pvData,
                       PFN_TRANSFER_PACKET     pfnTransferPacket,
                       PFN_PROGRESS_CALLBACK   pfnCallback,
                       PFN_RECV_PKT_CALLBACK   pfnRecvPktCallback,
                       void*                   pvUser)
{
  /* Usually one brace should be enough, but GNU wants to have a second brace
     to initialize the structure. On GCC 4.0.3 the whole structure is initialized
     as described in ISOC90 */
  union
  {
    CIFX_PACKET                   tPacket;
    HIL_FILE_UPLOAD_REQ_T         tUploadReq;
    HIL_FILE_UPLOAD_DATA_REQ_T    tUploadDataReq;
    HIL_FILE_DOWNLOAD_ABORT_REQ_T tAbortReq;
  }                             uSendPkt;

  union
  {
    CIFX_PACKET                   tPacket;
    HIL_FILE_UPLOAD_CNF_T         tUploadCnf;
    HIL_FILE_UPLOAD_DATA_CNF_T    tUploadDataCnf;
  }                             uRecvPkt;

  char*                         pbCopyPtr         = NULL;
  uint32_t                      ulCopySize        = 0;
  uint32_t                      ulFileLength      = 0;
  uint16_t                      usFilenameLen     = (uint16_t)(OS_Strlen(szFileName) + 1); /*Firmware expects length including terminating NULL */
  uint32_t                      ulBlockSize       = ulMailboxSize -
                                                    (uint32_t)sizeof(uRecvPkt.tUploadDataCnf); /* maximum size of each file block */
  int                           fSendAbort        = 0;
  int32_t                       lRetAbort         = CIFX_NO_ERROR;
  int32_t                       lRet              = CIFX_NO_ERROR;
  uint32_t                      ulCurrentId       = 0;
  uint32_t                      ulSrc             = OS_GetMilliSecCounter(); /* Early versions used pvChannel as ulSrc,
                                                                                but this won't work on 64 Bit machines.
                                                                                As we need something unique we use the current system time */

  OS_Memset(&uSendPkt, 0, sizeof(uSendPkt));
  OS_Memset(&uRecvPkt, 0, sizeof(uRecvPkt));

  /* Check parameters */
  if( (NULL == pvData) || (NULL == pulDataBufferLen) )
    return CIFX_INVALID_POINTER;

  ++ulCurrentId;
  uSendPkt.tUploadReq.tHead.ulDest             = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
  uSendPkt.tUploadReq.tHead.ulSrc              = HOST_TO_LE32(ulSrc);
  uSendPkt.tUploadReq.tHead.ulDestId           = HOST_TO_LE32(0);
  uSendPkt.tUploadReq.tHead.ulSrcId            = HOST_TO_LE32(0);
  uSendPkt.tUploadReq.tHead.ulLen              = HOST_TO_LE32((uint32_t)(sizeof(uSendPkt.tUploadReq.tData) + usFilenameLen));
  uSendPkt.tUploadReq.tHead.ulId               = HOST_TO_LE32(ulCurrentId);
  uSendPkt.tUploadReq.tHead.ulSta              = HOST_TO_LE32(0);
  uSendPkt.tUploadReq.tHead.ulCmd              = HOST_TO_LE32(HIL_FILE_UPLOAD_REQ);
  uSendPkt.tUploadReq.tHead.ulExt              = HOST_TO_LE32(HIL_PACKET_SEQ_NONE);
  uSendPkt.tUploadReq.tHead.ulRout             = HOST_TO_LE32(0);

  uSendPkt.tUploadReq.tData.usFileNameLength   = HOST_TO_LE16(usFilenameLen);
  uSendPkt.tUploadReq.tData.ulXferType         = HOST_TO_LE32(ulTransferType);
  uSendPkt.tUploadReq.tData.ulMaxBlockSize     = HOST_TO_LE32(ulBlockSize);
  uSendPkt.tUploadReq.tData.ulChannelNo        = HOST_TO_LE32(ulChannel);

  /* Setup copy buffer and copy size */
  pbCopyPtr   = ((char*)(&uSendPkt.tPacket.abData[0])) + sizeof(uSendPkt.tUploadReq.tData);
  ulCopySize  = HIL_MIN((sizeof(uSendPkt.tPacket.abData) - sizeof(uSendPkt.tUploadReq.tData)), uSendPkt.tUploadReq.tData.usFileNameLength);

  (void)OS_Strncpy( pbCopyPtr, szFileName, ulCopySize);

  lRet = pfnTransferPacket(pvChannel,
                           &uSendPkt.tPacket,
                           &uRecvPkt.tPacket,
                           (uint32_t)sizeof(uRecvPkt.tPacket),
                           CIFX_TO_SEND_PACKET,
                           pfnRecvPktCallback,
                           pvUser);

  /* Read file length */
  ulFileLength = LE32_TO_HOST(uRecvPkt.tUploadCnf.tData.ulFileLength);

  /* ATTENTION: We have to send an "Abort" to the system if:                              */
  /*  1. Command or File Error occured                                                    */
  /*  2. If the file exists but the length is 0                                           */
  /* In both cases, it is possible the system has activated a data transfer and waits     */
  /* on data requests commands.                                                           */
  /* It is necessary to send a "Abort" command, otherwise the next file access will fail  */
  /* with an error "COMMAND_ACTIVE".                                                      */

  if( (CIFX_NO_ERROR  != lRet)                                                      ||
      (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(uRecvPkt.tPacket.tHeader.ulState)))   ||
      (0              == ulFileLength)                                              )
  {
    /* Set return of read file length to 0 */
    *pulDataBufferLen = 0;

    /* Send progress notification */
    if(pfnCallback)
      pfnCallback( 0, 0, pvUser, CIFX_CALLBACK_FINISHED, lRet);

    /* Execute an abort command */
    fSendAbort = 1;
  } else
  {
    /* Check file length against user buffer length */
    if(ulFileLength > *pulDataBufferLen)
    {
      fSendAbort = 1;
      lRet = CIFX_INVALID_BUFFERSIZE;
    } else
    {
      uint32_t  ulCRC              = 0;
      uint8_t*  pbData             = (uint8_t*)pvData; /* pointer to return buffer */
      uint32_t  ulTransferredBytes = 0;
      uint32_t  ulTotalBytes       = ulFileLength;

      /* Set return of read file length to 0 */
      *pulDataBufferLen = 0;

      /* Create upload data packet */
      ++ulCurrentId;
      OS_Memset( &uSendPkt.tUploadDataReq, 0, sizeof(uSendPkt.tUploadDataReq));
      uSendPkt.tUploadDataReq.tHead.ulDest     = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
      uSendPkt.tUploadDataReq.tHead.ulSrc      = HOST_TO_LE32(ulSrc);
      uSendPkt.tUploadDataReq.tHead.ulDestId   = HOST_TO_LE32(0);
      uSendPkt.tUploadDataReq.tHead.ulSrcId    = HOST_TO_LE32(0);
      uSendPkt.tUploadDataReq.tHead.ulLen      = HOST_TO_LE32(0);
      uSendPkt.tUploadDataReq.tHead.ulId       = HOST_TO_LE32(ulCurrentId);
      uSendPkt.tUploadDataReq.tHead.ulSta      = HOST_TO_LE32(0);
      uSendPkt.tUploadDataReq.tHead.ulCmd      = HOST_TO_LE32(HIL_FILE_UPLOAD_DATA_REQ);
      uSendPkt.tUploadDataReq.tHead.ulExt      = HOST_TO_LE32(HIL_PACKET_SEQ_NONE);
      uSendPkt.tUploadDataReq.tHead.ulRout     = HOST_TO_LE32(0);

      /* Adjust block size to the size of the system */
      if( LE32_TO_HOST(uRecvPkt.tUploadCnf.tData.ulMaxBlockSize) < ulBlockSize)
         ulBlockSize = LE32_TO_HOST(uRecvPkt.tUploadCnf.tData.ulMaxBlockSize);

      /* Check size we have to send */
      /* If this is only one packet, set extension to NONE */
      uSendPkt.tUploadDataReq.tHead.ulExt = HOST_TO_LE32(HIL_PACKET_SEQ_FIRST);
      if( ulTotalBytes <= ulBlockSize)
        uSendPkt.tUploadDataReq.tHead.ulExt = HOST_TO_LE32(HIL_PACKET_SEQ_NONE);    /* We can send all in one packet */

      /* Perform upload */
      while( (ulFileLength > 0) && (CIFX_NO_ERROR == lRet) )
      {
        /* Send and receive data */
        lRet = pfnTransferPacket(pvChannel,
                                 &uSendPkt.tPacket,
                                 &uRecvPkt.tPacket,
                                 (uint32_t)sizeof(uRecvPkt.tPacket),
                                 CIFX_TO_SEND_PACKET,
                                 pfnRecvPktCallback,
                                 pvUser);
        /* Check for errors */
        if( (CIFX_NO_ERROR  != lRet)                                ||
            (SUCCESS_HIL_OK != (lRet = LE32_TO_HOST(uRecvPkt.tPacket.tHeader.ulState))) )
        {
          /* This is a packet error from the hardware */
          /* - Inform application */
          /* - Leave upload and send abort */
          if(pfnCallback)
            pfnCallback(ulTransferredBytes, ulTotalBytes, pvUser, CIFX_CALLBACK_FINISHED, lRet);

          fSendAbort = 1;
          break;
        } else
        {
          uint32_t  ulCurrentDataLen = LE32_TO_HOST(uRecvPkt.tUploadDataCnf.tHead.ulLen) -
                                                    (uint32_t)sizeof(uRecvPkt.tUploadDataCnf.tData);
          uint8_t*  pbRecvData       = (uint8_t*)(&uRecvPkt.tUploadDataCnf.tData + 1);
          uint32_t  ulPacketCrc      = LE32_TO_HOST(uRecvPkt.tUploadDataCnf.tData.ulChksum);

          /* Create own checksum and compare with it */
          ulCRC = CreateCRC32( ulCRC, pbRecvData, ulCurrentDataLen);

          if(ulCRC != ulPacketCrc)
          {
            /* Abort, as a CRC32 error occurred */
            lRet = CIFX_FILE_CHECKSUM_ERROR;

            /* Send progress notification */
            if(pfnCallback)
              pfnCallback(ulTransferredBytes, ulTotalBytes, pvUser, CIFX_CALLBACK_FINISHED, lRet);

            fSendAbort = 1;
            break;
          } else
          {
            /* Next packet */
            ++ulCurrentId;
            uSendPkt.tUploadDataReq.tHead.ulId = HOST_TO_LE32(ulCurrentId);

            /* Calculate outstanding size */
            ulFileLength        -= ulCurrentDataLen;
            OS_Memcpy(pbData, pbRecvData, ulCurrentDataLen);
            pbData              += ulCurrentDataLen;
            ulTransferredBytes  += ulCurrentDataLen;
            *pulDataBufferLen   = ulTransferredBytes;

            /* Send progress notification */
            if(pfnCallback)
              pfnCallback(ulTransferredBytes, ulTotalBytes, pvUser,
                          (ulTransferredBytes == ulTotalBytes)? CIFX_CALLBACK_FINISHED : CIFX_CALLBACK_ACTIVE,
                          lRet);

            /* Calculate next packet length and packet extension */
            if(ulFileLength != 0)
            {
              if(ulFileLength <= ulBlockSize)
                uSendPkt.tUploadDataReq.tHead.ulExt = HOST_TO_LE32(HIL_PACKET_SEQ_LAST);
              else
                uSendPkt.tUploadDataReq.tHead.ulExt = HOST_TO_LE32(HIL_PACKET_SEQ_MIDDLE);
            }
          }
        }
      }
    }
  }

  /* If anything failed during upload, send an abort request */
  if( fSendAbort)
  {
    ++ulCurrentId;
    uSendPkt.tAbortReq.tHead.ulDest   = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
    uSendPkt.tAbortReq.tHead.ulSrc    = HOST_TO_LE32(ulSrc);
    uSendPkt.tAbortReq.tHead.ulDestId = HOST_TO_LE32(0);
    uSendPkt.tAbortReq.tHead.ulSrcId  = HOST_TO_LE32(0);
    uSendPkt.tAbortReq.tHead.ulLen    = HOST_TO_LE32(0);
    uSendPkt.tAbortReq.tHead.ulId     = HOST_TO_LE32(ulCurrentId);
    uSendPkt.tAbortReq.tHead.ulSta    = HOST_TO_LE32(0);
    uSendPkt.tAbortReq.tHead.ulCmd    = HOST_TO_LE32(HIL_FILE_UPLOAD_ABORT_REQ);
    uSendPkt.tAbortReq.tHead.ulExt    = HOST_TO_LE32(HIL_PACKET_SEQ_NONE);
    uSendPkt.tAbortReq.tHead.ulRout   = HOST_TO_LE32(0);

    /* Transfer packet */
    lRetAbort = pfnTransferPacket(pvChannel,
                                  &uSendPkt.tPacket,
                                  &uRecvPkt.tPacket,
                                  (uint32_t)sizeof(uRecvPkt.tPacket),
                                  CIFX_TO_SEND_PACKET,
                                  pfnRecvPktCallback,
                                  pvUser);

    if( lRetAbort == CIFX_NO_ERROR)
    {
      /* Return packet state if function succeeded */
      lRetAbort = LE32_TO_HOST((int32_t)uRecvPkt.tPacket.tHeader.ulState);
    }
  }

  /* Always return lRet first, then abort error */
  if( CIFX_NO_ERROR != lRet)
    return lRet;
  else if( CIFX_NO_ERROR != lRetAbort)
    return lRetAbort;
  else
    return CIFX_NO_ERROR;
}

/*****************************************************************************/
/*! Read/Write Block
*   \param ptChannel      Channel Instance
*   \param pvBlock        Pointer to the block to copy
*   \param ulOffset       Start offset to copy from/to
*   \param ulBlockLen     Total Length of the Block
*   \param pvDest         Source/Destination buffer
*   \param ulDestLen      Length of the Source/Destination Buffer
*   \param ulCmd          CIFX_CMD_READ_DATA/CIFX_CMD_WRITE_DATA
*   \param fWriteAllowed  !=0 if Write is allowed to the Block
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_ReadWriteBlock(PCHANNELINSTANCE ptChannel,
                           void*            pvBlock,
                           uint32_t         ulOffset,
                           uint32_t         ulBlockLen,
                           void*            pvDest,
                           uint32_t         ulDestLen,
                           uint32_t         ulCmd,
                           int              fWriteAllowed)
{
  int32_t lRet = CIFX_NO_ERROR;

  if( (ulOffset + ulDestLen) > ulBlockLen)
    return CIFX_INVALID_ACCESS_SIZE; /* Size too long */

  /* Process the state block area command */
  switch (ulCmd)
  {
    case  CIFX_CMD_WRITE_DATA:
      if(fWriteAllowed)
      {
        /* Write control block */
        HWIF_WRITEN( ptChannel->pvDeviceInstance,
                     ((uint8_t*)pvBlock) + ulOffset,
                      (uint8_t *)pvDest,
                      ulDestLen);
      } else
      {
        lRet = CIFX_INVALID_COMMAND;
      }
      break;

    case CIFX_CMD_READ_DATA:
      /* It is allowed to read the control block back */
      HWIF_READN( ptChannel->pvDeviceInstance,
                  (uint8_t *)pvDest,
                  ((uint8_t*)pvBlock) + ulOffset,
                  ulDestLen);
      break;

    default:
      /* Unknown command */
      lRet = CIFX_INVALID_COMMAND;
      break;
  } /* end switch */

  /* Always deliver back system errors */
  if( (CIFX_NO_ERROR == lRet) &&
      !DEV_IsRunning(ptChannel) )
    lRet = CIFX_DEV_NOT_RUNNING;

  return lRet;
}

/*****************************************************************************/
/*! Exchanges a packet with the device
*   ATTENTION: This function will poll for receive packet, and will discard
*              any packets that do not match the send packet. So don't use
*              it during active data transfers
*   \param pvChannel        Channel instance to exchange a packet
*   \param ptSendPkt        Send packet pointer
*   \param ptRecvPkt        Pointer to place received Packet in
*   \param ulRecvBufferSize Length of the receive buffer
*   \param ulTimeout        Maximum time in ms to wait for an empty mailbox
*   \param pvPktCallback    Packet callback for unhandled receive packets
*   \param pvUser           User data for callback function
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_TransferPacket(void*                 pvChannel,
                           CIFX_PACKET*          ptSendPkt,
                           CIFX_PACKET*          ptRecvPkt,
                           uint32_t              ulRecvBufferSize,
                           uint32_t              ulTimeout,
                           PFN_RECV_PKT_CALLBACK pvPktCallback,
                           void*                 pvUser)
{
  PCHANNELINSTANCE  ptChannel = (PCHANNELINSTANCE)pvChannel;
  int32_t           lCount = 0;
  int32_t           lRet = CIFX_NO_ERROR;

  if ((lRet = DEV_PutPacket(ptChannel, ptSendPkt, ulTimeout)) == CIFX_NO_ERROR)
  {
    do
    {
      if ((lRet = DEV_GetPacket(ptChannel, ptRecvPkt, ulRecvBufferSize, ulTimeout)) == CIFX_NO_ERROR)
      {
        /* Check if we got the answer */
        if (((LE32_TO_HOST(ptRecvPkt->tHeader.ulCmd) & ~HIL_MSK_PACKET_ANSWER) == LE32_TO_HOST(ptSendPkt->tHeader.ulCmd)) &&
          (ptRecvPkt->tHeader.ulSrc == ptSendPkt->tHeader.ulSrc) &&
          (ptRecvPkt->tHeader.ulId == ptSendPkt->tHeader.ulId) &&
          (ptRecvPkt->tHeader.ulSrcId == ptSendPkt->tHeader.ulSrcId))
        {
          /* We got the answer message */
          /* lRet = ptRecvPkt->tHeader.ulState; */ /* Do not deliver back this information */
          break;
        }
        else
        {
          /* This is not our packet, check if the user wants it */
          if (NULL != pvPktCallback)
          {
            pvPktCallback(ptRecvPkt, pvUser);
          }
        }
        /* Reset error, in case we might drop out of the loop, with no proper answer,
           returning a "good" state */
        lRet = CIFX_DEV_GET_TIMEOUT;
        lCount++;
      }
      else
      {
        /* Error during packet receive */
        break;
      }
    } while (lCount < 10);
  }

  return lRet;
}

/*****************************************************************************/
/*! Wait for NOT RUNNING in poll mode
*   \param ptChannel Channel instance to check
*   \param ulTimeout Wait time
*   \return 1 if channel is NOT running                                      */
/*****************************************************************************/
int DEV_WaitForNotRunning_Poll(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout)
{
  /* Poll for Ready bit */
  int      iActualState = 1;
  uint32_t ulDiffTime   = 0L;
  int32_t  lStartTime   = (int32_t)OS_GetMilliSecCounter();

  /* We not processing a system channel */
  if(ptChannel->fIsSysDevice)
    return iActualState;

  /* Check user timeout */
  if( 0 == ulTimeout)
  {
    if( DEV_IsRunning(ptChannel))
      iActualState = 0;
  } else
  {
    /* User wants to wait */
    while(DEV_IsRunning(ptChannel))
    {
      /* Check for timeout */
      ulDiffTime = OS_GetMilliSecCounter() - lStartTime;

      if(ulDiffTime > ulTimeout)
      {
        iActualState = 0;
        break;
      }

      OS_Sleep(1);
    }
  }

  return iActualState;
}

/*****************************************************************************/
/*! Wait for RUNNING in poll mode
*   \param ptChannel Channel instance to check
*   \param ulTimeout Wait time
*   \return 1 if channel is RUNNING                                          */
/*****************************************************************************/
int DEV_WaitForRunning_Poll(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout)
{
  /* Poll for Ready bit */
  int      iActualState = 1;
  uint32_t ulDiffTime   = 0L;
  int32_t  lStartTime   = (int32_t)OS_GetMilliSecCounter();

  /* We not processing a system channel, so always return a valid state */
  if(ptChannel->fIsSysDevice)
    return iActualState;

  /* Check user timeout */
  if( 0 == ulTimeout)
  {
    /* Just return the actual state */
    iActualState = DEV_IsRunning(ptChannel);
  } else
  {
    /* User wants to wait */
    while(!DEV_IsRunning(ptChannel))
    {
      /* Check for timeout */
      ulDiffTime = OS_GetMilliSecCounter() - lStartTime;

      if(ulDiffTime > ulTimeout)
      {
        iActualState = 0;
        break;
      }

      OS_Sleep(1);
    }
  }

  return iActualState;
}
