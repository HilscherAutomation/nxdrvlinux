/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXDownload.c 14700 2023-04-27 08:11:12Z RMayer $:

  Description:
    cifX download functionality

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2023-04-21  Added file size 0 check and sending abort cmd in DEV_UploadFile(),
                to prevent ERR_HIL_RESOURCE_IN_USE because of not executed HIL_FILE_UPLOAD_DATA_REQ
    2021-08-13  Removed "\r\n" from trace strings, now generally handled in USER-Trace()
    2019-02-22  Add possibility to download netX90 /netX4000 firmware updates
    2018-10-10  - Updated header and definitions to new Hilscher defines
                - Derived from cifX Toolkit V1.6.0.0

**************************************************************************************/

/*****************************************************************************/
/*! \file cifXDownload.c
*   cifX download functionality                                              */
/*****************************************************************************/

#include "cifXToolkit.h"
#include "cifXErrors.h"
#include "cifXEndianess.h"

#include "Hil_ModuleLoader.h"
#include "Hil_SystemCmd.h"
#include "Hil_Results.h"
#include "Hilmd5.h"
#include "Hilcrc32.h"

/*****************************************************************************/
/*!  \addtogroup CIFX_TK_HARDWARE Hardware Access
*    \{                                                                      */
/*****************************************************************************/

/*****************************************************************************/
/*! Delete all existing files in a channel, from the file system.
*   \param ptChannel          Channel instance
*   \param ulChannel          Channel number
*   \param pfnTransferPacket  Function used for transferring packets
*   \param pfnRecvPacket      User callback for unsolicited receive packets
*   \param pvUser             User parameter passed on callback
*   \param szExceptFile       File extension to ignore while deleting files
*   \return always returns 1                                                 */
/*****************************************************************************/
int DEV_RemoveChannelFiles(PCHANNELINSTANCE ptChannel, uint32_t ulChannel,
                           PFN_TRANSFER_PACKET    pfnTransferPacket,
                           PFN_RECV_PKT_CALLBACK  pfnRecvPacket,
                           void*                  pvUser,
                           char*                  szExceptFile)
{
  /* Try to find file with the extension *.nxm, *.nxf, *.mod and remove it */
  CIFX_DIRECTORYENTRY tDirectoryEntry;
  int32_t             lRet            = CIFX_NO_ERROR;
  int                 fFindFirst      = 1;

  /* Search for all firmware files. If one is found. delete it an start with find first again, */
  /* because we can't store a directory list in here */
  do
  {
    if ( fFindFirst)
    {
       OS_Memset(&tDirectoryEntry, 0, sizeof(tDirectoryEntry));

      /* Search first file */
      if ( !(CIFX_NO_ERROR == (lRet = xSysdeviceFindFirstFile( ptChannel, ulChannel, &tDirectoryEntry, pfnRecvPacket, pvUser))))
      {
        /* No more files, or error during find first */
        break;
      } else
      {
        /* Is this a valid file name */
        int iStrlen = OS_Strlen(tDirectoryEntry.szFilename);
        if( iStrlen >= CIFX_MIN_FILE_NAME_LENGTH)  /* At least x.abc */
        {
          if( !((NULL != szExceptFile)                                                          &&
                (4 == OS_Strlen(szExceptFile))                                                  &&
                (0 == OS_Strnicmp( szExceptFile, &tDirectoryEntry.szFilename[iStrlen - 4], 4)))   )
          {
            /* Delete file and continue with find first file again */
            (void)DEV_DeleteFile( ptChannel, ulChannel, tDirectoryEntry.szFilename, pfnTransferPacket, pfnRecvPacket, pvUser);
          }
        } else
        {
          /* Not a valid file, search next file */
          fFindFirst = 0;
        }
      }
    } else
    {
      /* Search for more files */
      if ( !(CIFX_NO_ERROR == (lRet = xSysdeviceFindNextFile( ptChannel, ulChannel, &tDirectoryEntry, pfnRecvPacket, pvUser))))
      {
        /* No more files, or error during find next */
        break;
      } else
      {
        /* Is this a valid file name */
        int iStrlen = OS_Strlen(tDirectoryEntry.szFilename);
        if( iStrlen >= CIFX_MIN_FILE_NAME_LENGTH)  /* At least x.abc */
        {
          /* If firmware file, delete it, else search until all files checked */
          if( !((NULL != szExceptFile)                                            &&
                (4 == OS_Strlen(szExceptFile))                                    &&
                (0 == OS_Strnicmp( szExceptFile, &tDirectoryEntry.szFilename[iStrlen - 4], 4)))   )
          {
            /* Delete the file and start with find first again */
            (void)DEV_DeleteFile( ptChannel, ulChannel, tDirectoryEntry.szFilename, pfnTransferPacket, pfnRecvPacket, pvUser);
            fFindFirst = 1;
          }
        }
      }
    }
  } while ( CIFX_NO_ERROR == lRet);

  return 1;
}

/*****************************************************************************/
/*! Check if we have a firmware file
*   \param pszFileName      Input file name
*   \return 1 on success                                                     */
/*****************************************************************************/
int DEV_IsFWFile( char* pszFileName)
{
  /* Check if we have a .NXO, .NXF,.NXM or .MOD extension */
  int fRet    = 0;
  int iStrlen = OS_Strlen(pszFileName);

  if( iStrlen >= CIFX_MIN_FILE_NAME_LENGTH)  /* At least x.abc */
  {
    if ( (0 == OS_Strnicmp( HIL_FILE_EXTENSION_FIRMWARE,     &pszFileName[iStrlen - 4], 4) ) ||
         (0 == OS_Strnicmp( HIL_FILE_EXTENSION_NXM_FIRMWARE, &pszFileName[iStrlen - 4], 4) ) ||
         (0 == OS_Strnicmp( HIL_FILE_EXTENSION_OPTION,       &pszFileName[iStrlen - 4], 4) ) ||
         (0 == OS_Strnicmp( ".MOD", &pszFileName[iStrlen - 4], 4) )  )
    {
      fRet = 1;
    }
  }

  return fRet;
}

/*****************************************************************************/
/*! Check if we have a firmware (update) file for netX90 and netX4000
*   \param pszFileName      Input file name
*   \return 1 on success                                                     */
/*****************************************************************************/
int DEV_IsFWFileNetX90or4000( char* pszFileName)
{
  /* Check if we have a .NXI or .NAI extension, or FWUPDATE.ZIP / FWUPDATE.NXS
     Note: NXE or NAE should be downloaded in update container.
  */
  int fRet    = 0;
  int iStrlen = OS_Strlen(pszFileName);

  if( iStrlen >= CIFX_MIN_FILE_NAME_LENGTH)  /* At least x.abc */
  {
    if ( (0 == OS_Strnicmp( HIL_FILE_EXTENSION_NXI_FIRMWARE, &pszFileName[iStrlen - 4], 4) ) ||
         (0 == OS_Strnicmp( HIL_FILE_EXTENSION_NAI_FIRMWARE, &pszFileName[iStrlen - 4], 4) )  )
    {
      fRet = 1;

    /* Check for ZIP container */
    } else if( (iStrlen == OS_Strlen("FWUPDATE.ZIP"))                         &&
               (0       == OS_Strnicmp( "FWUPDATE.ZIP", &pszFileName[0], iStrlen)) )
    {
      fRet = 1;

    /* Check for NXS container */
    } else if( (iStrlen == OS_Strlen("FWUPDATE.NXS"))                         &&
               (0       == OS_Strnicmp( "FWUPDATE.NXS", &pszFileName[0], iStrlen)) )
    {
      fRet = 1;
    }
  }

  return fRet;
}

/*****************************************************************************/
/*! Check if we have a NXO file
*   \param pszFileName      Input file name
*   \return 1 on success                                                     */
/*****************************************************************************/
int DEV_IsNXOFile( char* pszFileName)
{
  /* Check if we have a .NXO, .NXF,.NXM or .MOD extension */
  int fRet    = 0;
  int iStrlen = OS_Strlen(pszFileName);

  if( iStrlen >= CIFX_MIN_FILE_NAME_LENGTH)  /* At least x.abc */
  {
    if ( 0 == OS_Strnicmp( HIL_FILE_EXTENSION_OPTION, &pszFileName[iStrlen - 4], 4) )
    {
      fRet = 1;
    }
  }

  return fRet;
}

/*****************************************************************************/
/*! Check if we have a NXF file
*   \param pszFileName      Input file name
*   \return 1 on success                                                     */
/*****************************************************************************/
int DEV_IsNXFFile( char* pszFileName)
{
  /* Check if we have a .NXO, .NXF,.NXM or .MOD extension */
  int fRet    = 0;
  int iStrlen = OS_Strlen(pszFileName);

  if( iStrlen >= CIFX_MIN_FILE_NAME_LENGTH)  /* At least x.abc */
  {
    if ( 0 == OS_Strnicmp( HIL_FILE_EXTENSION_FIRMWARE, &pszFileName[iStrlen - 4], 4) )
    {
      fRet = 1;
    }
  }

  return fRet;
}

/*****************************************************************************/
/*! Delete existing firmware file from file system.
* This should prevent multiple firmware files in the file system, where
* only the first one is usable.
*   \param ptChannel          Channel instance
*   \param ulChannel          Channel number
*   \param pfnTransferPacket  Function used for transferring packets
*   \param pfnRecvPacket      User callback for unsolicited receive packets
*   \param pvUser             User parameter passed on callback
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int DEV_RemoveFWFiles(PCHANNELINSTANCE ptChannel, uint32_t ulChannel,
                      PFN_TRANSFER_PACKET    pfnTransferPacket,
                      PFN_RECV_PKT_CALLBACK  pfnRecvPacket,
                      void*                  pvUser)
{
  /* Try to find file with the extension *.nxm, *.nxf, *.mod and remove it */
  CIFX_DIRECTORYENTRY tDirectoryEntry;
  int32_t             lRet            = CIFX_NO_ERROR;
  int                 fFindFirst      = 1;

  OS_Memset(&tDirectoryEntry, 0, sizeof(tDirectoryEntry));

  /* Search for all firmware files. If one is found. delete it an start with find first again, */
  /* because we can't store a directory list in here */
  do
  {
    if ( fFindFirst)
    {
      /* Search first file */
      if ( !(CIFX_NO_ERROR == (lRet = xSysdeviceFindFirstFile( ptChannel, ulChannel, &tDirectoryEntry, pfnRecvPacket, pvUser))))
      {
        /* No more files, or error during find first */
        break;
      } else
      {
        /* Check for firmware file */
        if( DEV_IsFWFile( tDirectoryEntry.szFilename))
        {
          /* Delete file and continue with find first file again */
          (void)DEV_DeleteFile( ptChannel, ulChannel, tDirectoryEntry.szFilename, pfnTransferPacket, pfnRecvPacket, pvUser);
        } else
        {
          /* Not a firmware, search next file */
          fFindFirst = 0;
        }
      }
    } else
    {
      /* Search for more files */
      if ( !(CIFX_NO_ERROR == (lRet = xSysdeviceFindNextFile( ptChannel, ulChannel, &tDirectoryEntry, pfnRecvPacket, pvUser))))
      {
        /* No more files, or error during find next */
        break;
      } else
      {
        /* If firmware file, delete it, else search until all files checked */
        if (DEV_IsFWFile( tDirectoryEntry.szFilename))
        {
          /* Delete file and continue with find first file again */
          (void)DEV_DeleteFile( ptChannel, ulChannel, tDirectoryEntry.szFilename, pfnTransferPacket, pfnRecvPacket, pvUser);
          fFindFirst = 1;
        }
      }
    }
  } while ( CIFX_NO_ERROR == lRet);

  return 1;
}

/*****************************************************************************/
/*! Delete the given file
*   \param pvChannel          Channel instance
*   \param ulChannelNumber    Channel number
*   \param pszFileName        Input file name
*   \param pfnTransferPacket  Function used for transferring packets
*   \param pfnRecvPacket      User callback for unsolicited receive packets
*   \param pvUser             User parameter passed on callback
*   \return 1 on success                                                     */
/*****************************************************************************/
int32_t DEV_DeleteFile(void* pvChannel, uint32_t ulChannelNumber, char* pszFileName,
                       PFN_TRANSFER_PACKET    pfnTransferPacket,
                       PFN_RECV_PKT_CALLBACK  pfnRecvPacket,
                       void*                  pvUser)
{
  /* Create delete packet */
  union
  {
    CIFX_PACKET           tPacket;
    HIL_FILE_DELETE_REQ_T tFileDelete;

  }                       uSendPkt;
  CIFX_PACKET             tConf;
  char*                   pbCopyPtr     = NULL;
  uint32_t                ulCopySize    = 0;
  uint16_t                usFileNameLen = (uint16_t)OS_Strlen(pszFileName);
  int32_t                 lRet          = CIFX_NO_ERROR;
  uint32_t                ulSrc         = OS_GetMilliSecCounter(); /* Early versions used pvChannel as ulSrc,
                                                                      but this won't work on 64 Bit machines.
                                                                      As we need something unique we use the current system time */


  OS_Memset(&uSendPkt, 0, sizeof(uSendPkt));
  OS_Memset(&tConf,    0, sizeof(tConf));

  /* Initialize the message */
  uSendPkt.tFileDelete.tHead.ulSrc    = HOST_TO_LE32(ulSrc);
  uSendPkt.tFileDelete.tHead.ulDest   = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
  uSendPkt.tFileDelete.tHead.ulCmd    = HOST_TO_LE32(HIL_FILE_DELETE_REQ);
  uSendPkt.tFileDelete.tHead.ulExt    = HOST_TO_LE32(HIL_PACKET_SEQ_NONE);
  uSendPkt.tFileDelete.tHead.ulLen    = HOST_TO_LE32((uint32_t)(sizeof(uSendPkt.tFileDelete.tData) +
                                                                usFileNameLen + 1));

  /* Insert file data */
  uSendPkt.tFileDelete.tData.ulChannelNo      = HOST_TO_LE32(ulChannelNumber);
  uSendPkt.tFileDelete.tData.usFileNameLength = HOST_TO_LE16( (uint16_t)(usFileNameLen + 1) );

  /* Setup copy buffer and copy size */
  pbCopyPtr   = ((char*)(&uSendPkt.tPacket.abData[0])) + sizeof(uSendPkt.tFileDelete.tData);
  ulCopySize  = min( (sizeof(uSendPkt.tPacket.abData) - sizeof(uSendPkt.tFileDelete.tData)), uSendPkt.tFileDelete.tData.usFileNameLength);

  /* Insert file name */
  (void)OS_Strncpy( pbCopyPtr, pszFileName, ulCopySize);

  /* Send delete packet */
  lRet = pfnTransferPacket( pvChannel,
                            &uSendPkt.tPacket,
                            (CIFX_PACKET*)&tConf,
                            (uint32_t)sizeof(tConf),
                            CIFX_TO_FIRMWARE_START,       /* Could take a little while */
                            pfnRecvPacket,
                            pvUser);

  if(CIFX_NO_ERROR == lRet)
    lRet = LE32_TO_HOST(tConf.tHeader.ulState);

  return lRet;
}

/*****************************************************************************/
/*! Get the firmware transfer type from file name
*   \param eChipType        netC chip type working on
*   \param pszFileName      Input file name
*   \param pulTransferType  Buffer for transfer type
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_GetFWTransferTypeFromFileName( CIFX_TOOLKIT_CHIPTYPE_E eChipType,
                                           char*                   pszFileName,
                                           uint32_t*               pulTransferType)
{
  /* Check if we have a NXF or .NXM / .MOD extension */
  int32_t lRet = CIFX_NO_ERROR;

  int iStrlen = (int)OS_Strlen(pszFileName);
  if( iStrlen < CIFX_MIN_FILE_NAME_LENGTH)  /* At least x.abc */
  {
    lRet = CIFX_FILE_NAME_INVALID;
  } else
  {
    /* Check if we have a valid firmware file */
    lRet = CIFX_FILE_TYPE_INVALID;

    /* netX90/netX4000 files */
    if( (eCHIP_TYPE_NETX90   == eChipType) ||
        (eCHIP_TYPE_NETX4000 == eChipType) )
    {
      if (DEV_IsFWFileNetX90or4000( pszFileName))
      {
        /* Use file transfer type for netX90/4000 updates */
        *pulTransferType = HIL_FILE_XFER_FILE;
        lRet = CIFX_NO_ERROR;
      }
    } else if (DEV_IsFWFile( pszFileName))
    {
      /* other firmware files */
      /* We have a firmware file, choose the correct download type */
      if ( (0 == OS_Strnicmp( HIL_FILE_EXTENSION_NXM_FIRMWARE, &pszFileName[iStrlen - 4], 4) ) ||
           (0 == OS_Strnicmp( HIL_FILE_EXTENSION_OPTION,       &pszFileName[iStrlen - 4], 4) ) ||
           (0 == OS_Strnicmp( ".MOD", &pszFileName[iStrlen - 4], 4) )  )
      {
        /* We are using the module file transfer type */
        *pulTransferType = HIL_FILE_XFER_MODULE;
      } else
      {
        /* All other files are downloaded via the file transfer type */
        *pulTransferType = HIL_FILE_XFER_FILE;
      }
      lRet = CIFX_NO_ERROR;
    }
  }

  return lRet;
}

/*****************************************************************************/
/*! Check if we have to download a file
*   \param pvChannel          Channel instance
*   \param ulChannelNumber    Channel number
*   \param pfDownload         Download flag
*   \param pszFileName        File name
*   \param pvFileData         File data buffer
*   \param ulFileSize         File size
*   \param pfnTransferPacket  Transfer packet function
*   \param pfnRecvPacket      Receive packet callback for unhandled packets
*   \param pvUser             User data for callback functions
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_CheckForDownload( void* pvChannel, uint32_t ulChannelNumber, int* pfDownload,
                           char* pszFileName, void* pvFileData, uint32_t ulFileSize,
                           PFN_TRANSFER_PACKET   pfnTransferPacket,
                           PFN_RECV_PKT_CALLBACK pfnRecvPacket,
                           void*                 pvUser)
{
  int32_t          lRet          = CIFX_NO_ERROR;
  PCHANNELINSTANCE ptChannel     = (PCHANNELINSTANCE)pvChannel;
  PDEVICEINSTANCE  ptDevInstance = (PDEVICEINSTANCE)ptChannel->pvDeviceInstance;

  /* Read the MD5 from the system */
  union
  {
    CIFX_PACKET             tPacket;
    HIL_FILE_GET_MD5_REQ_T  tRequest;
  }                         uSendPkt;
  union
  {
    CIFX_PACKET             tPacket;
    HIL_FILE_GET_MD5_CNF_T  tConf;
  }                         uConf;
  char*                     pbCopyPtr     = NULL;
  uint32_t                  ulCopySize    = 0;
  uint16_t                  usFileNameLen = (uint16_t)OS_Strlen(pszFileName);
  uint32_t                  ulSrc         = OS_GetMilliSecCounter(); /* Early versions used pvChannel as ulSrc,
                                                                        but this won't work on 64 Bit machines.
                                                                        As we need something unique we use the current system time */

  OS_Memset(&uSendPkt, 0, sizeof(uSendPkt));
  OS_Memset(&uConf,    0, sizeof(uConf));

  /* Set flag to download always necessary */
  *pfDownload = 1;

  /* Initialize the message */
  uSendPkt.tRequest.tHead.ulSrc              = HOST_TO_LE32(ulSrc);
  uSendPkt.tRequest.tHead.ulDest             = HOST_TO_LE32(HIL_PACKET_DEST_SYSTEM);
  uSendPkt.tRequest.tHead.ulCmd              = HOST_TO_LE32(HIL_FILE_GET_MD5_REQ);
  uSendPkt.tRequest.tHead.ulExt              = HOST_TO_LE32(HIL_PACKET_SEQ_NONE);
  uSendPkt.tRequest.tHead.ulLen              = HOST_TO_LE32((uint32_t)(sizeof(uSendPkt.tRequest.tData) + usFileNameLen + 1));
  uSendPkt.tRequest.tData.usFileNameLength   = HOST_TO_LE16( (uint16_t)(usFileNameLen + 1) );
  uSendPkt.tRequest.tData.ulChannelNo        = HOST_TO_LE32(ulChannelNumber);

  /* Setup copy buffer and copy size */
  pbCopyPtr   = ((char*)(&uSendPkt.tPacket.abData[0])) + sizeof(uSendPkt.tRequest.tData);
  ulCopySize  = min( (sizeof(uSendPkt.tPacket.abData) - sizeof(uSendPkt.tRequest.tData)), uSendPkt.tRequest.tData.usFileNameLength);

  /* Insert file name */
  (void)OS_Strncpy( pbCopyPtr, pszFileName, ulCopySize);

  /* Read the MD5 from the system */
  lRet = pfnTransferPacket( pvChannel,
                            &uSendPkt.tPacket,
                            &uConf.tPacket,
                            (uint32_t)sizeof(uConf.tPacket),
                            CIFX_TO_FIRMWARE_START,       /* Could take a little while */
                            pfnRecvPacket,
                            pvUser);

  if(CIFX_NO_ERROR != lRet)
  {
    /* Error reading MD5 checksum */
    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      USER_Trace(ptDevInstance,
                 TRACE_LEVEL_ERROR,
                 "Failed to send MD5 request, lRet = 0x%08x", lRet);
    }
  } else if(SUCCESS_HIL_OK != LE32_TO_HOST(uConf.tConf.tHead.ulSta))
  {
    /* Error reading MD5 checksum */
    if(g_ulTraceLevel & TRACE_LEVEL_INFO)
    {
      USER_Trace(ptDevInstance,
                 TRACE_LEVEL_INFO,
                 "No MD5 Information available. Probably the file does not exist on device. (ulState = 0x%08x)",
                 uConf.tConf.tHead.ulSta);
    }
  } else
  {
    /* We got an MD5 from the rcX, test it */
    /* Calculate MD5 */
    md5_state_t tMd5State;
    md5_byte_t  abMd5[16];

    OS_Memset(abMd5, 0, sizeof(abMd5));

    md5_init(&tMd5State);
    md5_append(&tMd5State, (md5_byte_t*)pvFileData, ulFileSize);
    md5_finish(&tMd5State, abMd5);

    if(OS_Memcmp(abMd5, uConf.tConf.tData.abMD5, sizeof(abMd5)) == 0)
    {
      /* same file already on device, suppress download */
      *pfDownload = 0;

      /* MD5 checksum is equal, no download necessary */
      if(g_ulTraceLevel & TRACE_LEVEL_INFO)
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_INFO,
                  "MD5 checksum is identical, download not necessary");
      }

    } else
    {
      /* MD5 checksum is not identical, download necessary */
      if(g_ulTraceLevel & TRACE_LEVEL_INFO)
      {
        USER_Trace(ptDevInstance,
                  TRACE_LEVEL_INFO,
                  "MD5 not identical, process file download");
      }
    }
  }

  return lRet;
} /*lint !e429 : pvFileData not freed or returned */

/*****************************************************************************/
/*! Process firmware download
*   \param ptDevInstance      Instance to start up
*   \param ulChannel          Channel number
*   \param pszFullFileName    Full file name (used for opening file)
*   \param pszFileName        Short file name (used on device)
*   \param ulFileLength       Length of the file
*   \param pbBuffer           File buffer
*   \param pbLoadState        Returned action of download (see CIFXTKIT_DOWNLOAD_XXX)
*   \param pfnTransferPacket  Function to used for exchanging packets
*   \param pfnCallback        Progress callback
*   \param pfnRecvPktCallback Callback for unexpected packets
*   \param pvUser             Callback user parameter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t DEV_ProcessFWDownload( PDEVICEINSTANCE       ptDevInstance,
                               uint32_t              ulChannel,
                               char*                 pszFullFileName,
                               char*                 pszFileName,
                               uint32_t              ulFileLength,
                               uint8_t*              pbBuffer,
                               uint8_t*              pbLoadState,
                               PFN_TRANSFER_PACKET   pfnTransferPacket,
                               PFN_PROGRESS_CALLBACK pfnCallback,
                               PFN_RECV_PKT_CALLBACK pfnRecvPktCallback,
                               void*                 pvUser)
{
  PCHANNELINSTANCE ptSysDevice = &ptDevInstance->tSystemDevice;
  int32_t          lRet        = CIFX_NO_ERROR;

  *pbLoadState  =  CIFXTKIT_DOWNLOAD_NONE;

  /*------------------------------------------------------------*/
  /* Process the firmware download depending on the eDeviceType */
  /*------------------------------------------------------------*/
  switch (ptDevInstance->eDeviceType)
  {
    /*----------------------------*/
    /* This is a RAM based device */
    /*----------------------------*/
    /* - RAM based devices are started by a RESET and therefore it is not necessary to delete a file */
    /* - Firmware files (NXF) and/or Modules (NXO) are loaded into RAM and not into the file system */
    case eCIFX_DEVICE_RAM_BASED:
    {
      /* We have not to delete files but we have to change the "transfer type" of the file */
      uint32_t ulTransfertype = HIL_FILE_XFER_MODULE;

      /* Check if we have a NXF*/
      if( DEV_IsNXFFile(pszFileName) &&
          (0 != ulChannel) )
      {
        /* Downloading an NXF to a channel other than 0 is not supported */
        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "Error channel number %u for a firmware is not supported",
                      ulChannel);
        }

      /* Check if we have an NXO file */
      } else if( DEV_IsNXOFile(pszFileName) &&
                 (!ptDevInstance->fModuleLoad))
      {
        /* Downloading an NXO without a running Base OS is not allowed */
        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "Error NXO files are not allowed without a Base OS firmware");
        }
      } else
      {
        /* Download the file stored in the buffer */
        lRet = DEV_DownloadFile(ptSysDevice,
                                ulChannel,
                                ptDevInstance->tSystemDevice.tSendMbx.ulSendMailboxLength,
                                ulTransfertype,
                                pszFileName,
                                ulFileLength,
                                pbBuffer,
                                pfnTransferPacket,
                                pfnCallback,
                                pfnRecvPktCallback,
                                pvUser);

        if(CIFX_NO_ERROR != lRet)
        {
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "Error downloading firmware to device '%s'"\
                      " - (lRet=0x%08X)!",
                      pszFullFileName,
                      lRet);
          }
        } else
        {
          /*-----------------------*/
          /* We have loaded a file */
          /*-----------------------*/

          /* Check if we have a NXF */
          if ( DEV_IsNXFFile( pszFileName))
          {
            /* NXF loaded, store information for startup handling */
            *pbLoadState = CIFXTKIT_DOWNLOAD_FIRMWARE | CIFXTKIT_DOWNLOAD_EXECUTED; /* we have a firmware loaded */
          }

          /* Check if we have a NXO */
          if ( DEV_IsNXOFile( pszFileName))
          {
            /* NXO loaded, store information for startup handling */
            *pbLoadState = CIFXTKIT_DOWNLOAD_MODULE  | CIFXTKIT_DOWNLOAD_EXECUTED;  /* we have a module loaded */
          }

          if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
          {
            USER_Trace(ptDevInstance,
                      TRACE_LEVEL_DEBUG,
                      "Successfully downloaded the firmware to device '%s'!",
                      pszFullFileName);
          }
        }
      }
    }
    break;

    /*------------------------------*/
    /* This is a FLASH based device */
    /*------------------------------*/
    /* - FLASH based devices are not reseted on the beginning */
    /* - Files are checked if they are already existing to prevent a download into FLASH */
    /* - If a new firmware files (NXF) is loaded, all other files (NXD/NXO etc) are deleted in all "PORTs" */
    /* - Firmware files are only allowed for PORT0 */
    /* - If an NXO is downloaded, all files (NXO/NXD) are deleted first. An existing NXF must be protected, because it is the base module! */
    case eCIFX_DEVICE_FLASH_BASED:
    {
      /* We have not to delete files but we have to change the "transfer type" of the file */
      uint32_t ulTransfertype  = HIL_FILE_XFER_FILE;
      int      fDownload       = 0;

      /* Does the file exist on the hardware, if so, skip the download */
      if ( CIFX_NO_ERROR != (lRet = DEV_CheckForDownload( ptSysDevice,
                                                          ulChannel,
                                                          &fDownload,
                                                          pszFileName,
                                                          pbBuffer,
                                                          ulFileLength,
                                                          pfnTransferPacket,
                                                          NULL,
                                                          NULL)))
      {
        /* Display an error */
        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInstance,
                      TRACE_LEVEL_ERROR,
                      "Error checking for download '%s'!",
                      pszFullFileName);
        }

      /* Check if we have to download the file */
      } else if(!fDownload)
      {
        /*-----------------------------------*/
        /* Download not necessary            */
        /*-----------------------------------*/
        /* Store NXO Information for startup */
        if(DEV_IsNXOFile(pszFileName))
        {
          if( !ptDevInstance->fModuleLoad)
          {
            /* Downloading an NXO without a running Base OS is not allowed */
            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                          TRACE_LEVEL_ERROR,
                          "Error NXO files are not allowed without a Base OS firmware");
            }

            lRet = CIFX_FILE_TYPE_INVALID;

          } else
          {
            if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
            {
              USER_Trace(ptDevInstance,
                        TRACE_LEVEL_DEBUG,
                        "Skipping download for file '%s'" \
                        "[checksum identical]!",
                        pszFullFileName);
            }

            *pbLoadState = CIFXTKIT_DOWNLOAD_MODULE;
          }
        } else if(DEV_IsNXFFile(pszFileName))
        {
          *pbLoadState = CIFXTKIT_DOWNLOAD_FIRMWARE;
        } else
        {
          lRet = CIFX_FILE_TYPE_INVALID;
        }
      } else
      {

        /*-----------------------------------*/
        /* Download is necessary             */
        /*-----------------------------------*/
        /* Check if we have a NXF*/
        if( DEV_IsNXFFile(pszFileName))
        {
          if (0 != ulChannel)
          {
            /* Downloading an NXF to a channel other than 0 is not supported */
            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                          TRACE_LEVEL_ERROR,
                          "Error channel number %u for a firmware is not supported",
                          ulChannel);
            }
            lRet = CIFX_INVALID_PARAMETER;
            fDownload = 0;

          } else
          {
            /* ATTENTION: If we are downloading an "NXF" file, we have a complete firmware.     */
            /*            In this case all other files should be deleted!                       */
            /*            NXF are stored under channel 0                                        */

            /* Files for a flash based device are always transfered into the FLASH file system */
            /* We have to delete existing files, depending of a NXF/NXO */
            uint32_t ulChNum = 0;

            /* Remove ALL files */
            for ( ulChNum = 0; ulChNum < CIFX_MAX_NUMBER_OF_CHANNELS; ulChNum++)
            {
              (void)DEV_RemoveChannelFiles(ptSysDevice, ulChNum, pfnTransferPacket, NULL, NULL, NULL);
            }

            /* We have loaded a new firmware file */
            *pbLoadState = CIFXTKIT_DOWNLOAD_FIRMWARE;
          }

         /* Check if we have an NXO file */
        } else if( DEV_IsNXOFile(pszFileName))
        {
          if( !ptDevInstance->fModuleLoad)
          {
            /* Downloading an NXO without a running Base OS is not allowed */
            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                          TRACE_LEVEL_ERROR,
                          "Error NXO files are not allowed without a Base OS firmware");
            }

            lRet = CIFX_FILE_TYPE_INVALID;
            fDownload = 0;

          } else
          {
            /* ATTENTION: If the file is an "NXO", we have to delete all files EXCEPT the "NXF" */
            /*            because this is our BASE OS file!                                     */
            /*            NXF are stored under channel 0, NXOs are storeable in each channel    */

            /* Files for a flash based device are always transfered into the FLASH file system */
            /* We have to delete existing files, depending of a NXF/NXO */
            /* Leave NXF file */
            (void)DEV_RemoveChannelFiles( ptSysDevice, ulChannel, pfnTransferPacket, NULL, NULL, HIL_FILE_EXTENSION_FIRMWARE);

            /* We have loaded a new module */
            *pbLoadState = CIFXTKIT_DOWNLOAD_MODULE;
          }
        } else
        {
          /* TODO: Unsupported file , do we need to check this???*/
          fDownload = 0;
        }

        if(fDownload)
        {
          /* Download the file stored in the buffer */
          lRet = DEV_DownloadFile(ptSysDevice,
                                  ulChannel,
                                  ptDevInstance->tSystemDevice.tSendMbx.ulSendMailboxLength,
                                  ulTransfertype,
                                  pszFileName,
                                  ulFileLength,
                                  pbBuffer,
                                  pfnTransferPacket,
                                  NULL,
                                  NULL,
                                  NULL);

          if(CIFX_NO_ERROR != lRet)
          {
            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace(ptDevInstance,
                        TRACE_LEVEL_ERROR,
                        "Error downloading firmware to device '%s'"\
                        " - (lRet=0x%08X)!",
                        pszFullFileName,
                        lRet);
            }

          } else
          {
            if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
            {
              USER_Trace(ptDevInstance,
                        TRACE_LEVEL_DEBUG,
                        "Successfully downloaded the firmware to device '%s'!",
                        pszFullFileName);
            }

            *pbLoadState |= CIFXTKIT_DOWNLOAD_EXECUTED;
          }
        }
      }
    }
    break;

    default:
      /* Unknown device type */
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInstance,
                    TRACE_LEVEL_ERROR,
                    "Error unsupported device type %u found for download handling!",
                    ptDevInstance->eDeviceType);
      }
    break;

  } /* end switch */

  return lRet;
} /*lint !e429 : pbBuffer not freed or returned */

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
        uint32_t ulFileNameLength = min( ((uint32_t)OS_Strlen(szFileName) + 1),
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
        ulCopySize  = min( (sizeof(uSendPkt.tPacket.abData) - sizeof(uSendPkt.tDownloadReq.tData)), uSendPkt.tDownloadReq.tData.usFileNameLength);

        /* Insert file name */
        (void)OS_Strncpy( pbCopyPtr, szFileName, ulCopySize);

        /* Transfer packet */
        lRet = pfnTransferPacket(pvChannel,
                                 &uSendPkt.tPacket,
                                 &uRecvPkt.tPacket,
                                 (uint32_t)sizeof(uRecvPkt.tPacket),
                                 ulTransferTimeout,
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

  if( ulMailboxSize < HIL_DPM_SYSTEM_MAILBOX_MIN_SIZE)
    return CIFX_DEV_MAILBOX_TOO_SHORT;

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
  ulCopySize  = min( (sizeof(uSendPkt.tPacket.abData) - sizeof(uSendPkt.tUploadReq.tData)), uSendPkt.tUploadReq.tData.usFileNameLength);

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
/*! \}                                                                       */
/*****************************************************************************/
