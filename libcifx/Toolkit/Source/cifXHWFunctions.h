/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXHWFunctions.h 15329 2025-11-24 13:34:32Z AMinor $:

  Description:
    cifX API Hardware handling functions declaration

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-05-26  Update to new DPMv2 handling
    2023-04-26  DEV function definitions from cifXToolkit.h moved here
    2023-04-18  Added new option parameter for HWIF_READN / WRITEN function, to be able to
                recognize single HWIF_READ16/WRITE32 and HWIF_READ32/WRITE32 accesses
    2022-06-14  Added new structure and option for cached IO handling
    2021-10-15  Added ulHostCOSFlagsSaved variable used in DSR handling
    2019-10-16  Parameters for reset functions changed, removed DEV_DoResetEx() function
    2019-10-14  Add separate function for update device
    2018-10-10  - Updated header and definitions to new Hilscher defines
                - Added chip type definitions for netX90/netX4000 (eCHIP_TYPE_NETX90 / eCHIP_TYPE_NETX4000)
                - Derived from cifX Toolkit V1.6.0.0

**************************************************************************************/

/*****************************************************************************/
/*!  \file                                                                   *
*     cifX API Hardware handling functions declaration                       */
/*****************************************************************************/

#ifndef CIFX_HWFUNCTIONS__H
#define CIFX_HWFUNCTIONS__H

#ifdef __cplusplus
extern "C"
{
#endif

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
                         void*                 pvUser);

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
                       void*                   pvUser);

int32_t DEV_ReadWriteBlock(PCHANNELINSTANCE ptChannel,
                           void*            pvBlock,
                           uint32_t         ulOffset,
                           uint32_t         ulBlockLen,
                           void*            pvDest,
                           uint32_t         ulDestLen,
                           uint32_t         ulCmd,
                           int              fWriteAllowed);

int32_t DEV_TransferPacket(void*                 pvChannel,
                           CIFX_PACKET*          ptSendPkt,
                           CIFX_PACKET*          ptRecvPkt,
                           uint32_t              ulRecvBufferSize,
                           uint32_t              ulTimeout,
                           PFN_RECV_PKT_CALLBACK pvPktCallback,
                           void*                 pvUser);
int32_t DEV_GetMBXFillLevel(NETX_MAILBOX_BLOCK_T* ptInst);
int32_t DEV_GetFWTransferTypeFromFileName(CIFX_TOOLKIT_CHIPTYPE_E eChipType,
                                          char*                   pszFileName,
                                          uint32_t*               pulTransferType);
int     DEV_IsNXOFile(char* pszFileName);
int     DEV_IsNXFFile(char* pszFileName);
int     DEV_WaitForNotRunning_Poll(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);
int     DEV_WaitForRunning_Poll(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* CIFX_HWFUNCTIONS__H */
