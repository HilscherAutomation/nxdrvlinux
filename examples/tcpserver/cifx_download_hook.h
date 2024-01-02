/* SPDX-License-Identifier: MIT */
/**************************************************************************************
 *
 * Copyright (c) 2024, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 * Changes:
 *
 *   Version   Date        Author   Description
 *   ----------------------------------------------------------------------------------
 *   1        02.01.24    SD        changed licensing terms
 *
 **************************************************************************************/

#ifndef __CIFXDOWNLOAD__H
#define __CIFXDOWNLOAD__H

/*****************************************************************************/
/*! \addtogroup CIFXDOWNLOADHOOK
*   \{                                                                       */
/*****************************************************************************/

#ifdef __cplusplus
  extern "C" {
#endif  /* __cplusplus */

#include "cifXAPI_Wrapper.h"

/*****************************************************************************/
/*! Definition of the file storage callback
*   \param ptBoardInfo       Board information
*   \param pszFileName       Name of file to download
*   \param ulFileSize        Size of file buffer (pabFileData)
*   \param pabFileData       File buffer
*   \param ulChannel         Destination channel
*   \param ulDownloadMode    Download mode (DOWNLOAD_MODE_FIRMWARE, 
                             DOWNLOAD_MODE_MODULE, DOWNLOAD_MODE_CONFIG)
*   \param pvUser            User pointer
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
typedef int32_t(*PFN_FILESTORAGE_CBK) ( BOARD_INFORMATION* ptBoardInfo, 
                                        char* pszFileName, uint32_t ulFileSize, uint8_t* pabFileData, 
                                        uint32_t ulChannel, uint32_t ulDownloadMode, void* pvUser);

/***************************************************************************
* Functions to install and remove download hook
***************************************************************************/
int32_t xDownloadHook_Install ( PDRIVER_FUNCTIONS ptDRVFunctions, 
                                PFN_FILESTORAGE_CBK pfnFileStorageCbk, void* pvUser);

int32_t xDownloadHook_Remove  ( void);

#ifdef __cplusplus
  }
#endif  /* __cplusplus */

/*****************************************************************************/
/*  \}                                                                       */
/*****************************************************************************/

#endif /* __CIFXDOWNLOAD__H */
