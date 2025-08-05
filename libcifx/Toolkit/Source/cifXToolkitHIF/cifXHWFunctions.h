/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXHWFunctions.h 15171 2025-08-05 08:18:45Z AMinor $:

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

#include "OS_Includes.h"
#include "cifXHWResources.h"
#include "cifXHwif.h"
#include "cifXFunctionList.h"

#include "Hil_FirmwareIdent.h"
#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/*!  \addtogroup CIFX_TK_STRUCTURE Toolkit Structure Definitions
*    \{                                                                      */
/*****************************************************************************/

#ifndef CIFX_TOOLKIT_FUNCTION_LIST
/*****************************************************************************/
/*! Structure defining a channel instance                                    */
/*****************************************************************************/
typedef struct CHANNELINSTANCEtag
{
  void*                 pvDeviceInstance;                 /*!< Pointer to the device instance belonging to this channel   */

  void*                 pvInitMutex;                      /*!< Device is currently initializing, e.g. while doing a reset */

  uint8_t*              pbDPMChannelStart;                /*!< virtual start address of channel block          */
  uint32_t              ulDPMChannelLength;               /*!< length of channel block                         */
  uint32_t              ulChannelNumber;                  /*!< Number of the Channel                           */

  void*                 pvLock;                           /*!< Lock for synchronizing interrupt accesses to flags   */
  uint32_t              ulOpenCount;                      /*!< Number of open device function called for channel    */

  int                   fIsSysDevice;                     /*!< !=0 if the channel instance belong to a systemdevice */
  HIL_FW_IDENTIFICATION_T tFirmwareIdent;                 /*!< Firmware Identification                         */

  NETX_MAILBOX_AREA_U   tFromHostMbx;                     /*!< Send mailbox administration structure   */
  NETX_MAILBOX_AREA_U   tToHostMbx;                       /*!< Receive mailbox administration structure*/

  NETX_COM_STATE_T      tComState;                        /*!< defining resources for com-state notification */

  HIL_HIF_COMMUNICATION_STATUS_BLOCK_T* ptCommunicationStatusBlock;  /*!< Pointer to channel's communication status block   */

  NETX_HS_CTL_T         tHsCtrl;

  void*                 apvHsBitEvent[HIL_HIF_NCF_RESTART_REQUIRED_BIT_NO + 1]; /*!< Event handles for each handshake bit pair (used in interrupt mode) */

  NETX_IO_AREA_T        tIoArea;                          /*!< Input/Output areas */

  CACHED_MEMORY_AREA_T  tCachedIOInputArea;               /*!< Information about cached IO input memory area */
  CACHED_MEMORY_AREA_T  tCachedIOOutputArea;              /*!< Information about cached IO input memory area */

  NETX_SYNC_DATA_T      tSynch;                           /*!< Sync handling                        */
} CHANNELINSTANCE, *PCHANNELINSTANCE;

/*****************************************************************************/
/*! Structure for ISR and DSR handling                                       */
/*****************************************************************************/
typedef struct IRQ_TO_DSR_BUFFER_Ttag
{
  uint32_t aulHsk[HIL_HIF_HSC_MAX];
  uint32_t ulTlbStatus;
  int      fValid;
} IRQ_TO_DSR_BUFFER_T;

/*****************************************************************************/
/*! Structure defining a physical device passed to the toolkit. Passing it,
*   will create all logical device associated with this instance             */
/*****************************************************************************/
typedef struct DEVICEINSTANCEtag
{
  struct CIFX_API_FUNCTION_LIST_Ttag*   ptCifxFun;            /*!< Function pointer to CIFX API functions                   */
  struct CIFX_DEV_FUNCTION_LIST_Ttag*   ptDevFun;             /*!< Function pointer to DEV API functions                    */
  struct CIFX_TKIT_FUNCTION_LIST_Ttag*  ptTkitFun;            /*!< Function pointer to TKIT API functions                   */

  uint32_t                            ulPhysicalAddress;      /*!< Physical address of the cifX card                        */
  uint32_t                            ulIrqNumber;            /*!< IRQ number assigned to card                              */
  int                                 fIrqEnabled;            /*!< !=0 if the IRQ is used on this device                    */

  int                                 fPCICard;               /*!< !=0 if the card is a PCI card (netX directly connected to PCI)
                                                                   ,this will reset the netX if eDeviceType is AUTODETECT or RAMBASE */
  CIFX_TOOLKIT_DEVICETYPE_E           eDeviceType;            /*!< Type of the device                                        */
  void*                               pvOSDependent;          /*!< OS dependent pointer to device identification (used for PCI read/write request).
                                                                   This parameter must allow the OS/User to identify the card and access it's PCI registers */
  uint8_t*                            pbDPM;                  /*!< Virtual/usable pointer to the cards DPM              */
  uint32_t                            ulDPMSize;              /*!< Size of the cards DPM                                */
  uint8_t                             bDPMLayout;             /*!< Layout of the DPM                                    */
  CIFX_TOOLKIT_CHIPTYPE_E             eChipType;              /*!< Type of chip */

  char                                szName[CIFx_MAX_INFO_NAME_LENTH]; /*!< Default name of the card, must be inserted by user */
  char                                szAlias[CIFx_MAX_INFO_NAME_LENTH];/*!< Alias name of the card, must be inserted by user   */

  int32_t                             lInitError;             /*!< Initialization error of the card                         */

  void*                               pvGlobalRegisters;      /*!< Pointer to the global host registers (only available on PCI)  */
  uint32_t                            ulSerialNumber;         /*!< Serial number of the card (read on startup)        */
  uint32_t                            ulDeviceNumber;         /*!< Device number of the card (read on startup)        */
  uint32_t                            ulSlotNumber;           /*!< Slot number on card (read on startup)              */

  uint32_t*                           pulHandshakeBlock;      /*!< Pointer to start of Handshake block (NULL if no handshake block was found */
  int                                 iIrqToDsrBuffer;        /*!< IRQ to DSR Buffer number to use               */
  IRQ_TO_DSR_BUFFER_T                 atIrqToDsrBuffer[2];    /*!< IRQ to DSR Buffers                            */
  uint32_t                            ulIrqCounter;           /*!< Number of interrupts processed on this device */

  CHANNELINSTANCE                     tSystemDevice;          /*!< Every card has at least one SystemDevice       */
  uint32_t                            ulCommChannelCount;     /*!< Number of communication channels on the card   */
  CHANNELINSTANCE**                   pptCommChannels;        /*!< Array of all found channels                    */

#ifdef CIFX_TOOLKIT_DMA
  uint32_t                            ulDMABufferCount;                     /*!< Number of available DMA buffers  */
  CIFX_DMABUFFER_T                    atDmaBuffers[CIFX_DMA_BUFFER_COUNT];  /*!< DMA buffer definition for the device */
#endif /* CIFX_TOOLKIT_DMA */
  int                                 fCachedMemAccess;                     /*!< Cached memory access to DMA buffer         */

  /* Synch handling */
  CIFX_SYNCH_DATA_T                   tSyncData;              /*!< Synchronization structure */

  int                                 fResetActive;           /*!< !=0 if a reset is pending on device (DEV_DoSystemStart) */

  /* Extended memory (additional target memory) */
  uint8_t*                            pbExtendedMemory;       /*!< Virtual/usable pointer to an extended memory area       */
  uint32_t                            ulExtendedMemorySize;   /*!< Size of the extended memory area                        */

#ifdef CIFX_TOOLKIT_HWIF
  PFN_HWIF_MEMCPY                     pfnHwIfRead;            /*!< Definable hardware read function                        */
  PFN_HWIF_MEMCPY                     pfnHwIfWrite;           /*!< Definable hardware read function                        */
#endif /* CIFX_TOOLKIT_HWIF */
} DEVICEINSTANCE, *PDEVICEINSTANCE;

void    DEV_WriteHandshakeFlags   (PCHANNELINSTANCE ptChannel);
void    DEV_ReadHostFlags         (PCHANNELINSTANCE ptChannel, int fReadHostCOS);
void    DEV_ReadHandshakeFlags    (PCHANNELINSTANCE ptChannel, int fReadSyncFlags, int fLockNeeded);

int     DEV_WaitForBitState       (PCHANNELINSTANCE ptChannel, uint32_t ulBitNumber, uint8_t bState, uint32_t ulTimeout);
int     DEV_WaitForIoBitState     (PCHANNELINSTANCE ptChannel, NETX_IO_BLOCK_T* ptInst, uint32_t ulTimeout);
int     DEV_WaitForMbxState       (PCHANNELINSTANCE ptChannel, NETX_MAILBOX_BLOCK_T* ptInst, uint32_t ulState, uint32_t ulTimeout);
void    DEV_ToggleBit             (PCHANNELINSTANCE ptChannel, uint32_t ulBitMask);
void    DEV_ToggleIoAction        (PCHANNELINSTANCE ptChannel, NETX_IO_BLOCK_T* ptInst);
void    DEV_WriteCell             (PCHANNELINSTANCE ptChannel, NETX_MAILBOX_BLOCK_T* ptInst, uint32_t ulValue);

int     DEV_WaitForSyncState      (PCHANNELINSTANCE ptChannel, uint8_t bState, uint32_t ulTimeout);
void    DEV_ToggleSyncBit         (PDEVICEINSTANCE  ptDevInstance, uint32_t ulBitMask);

int32_t DEV_PutPacket             (PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout);
int32_t DEV_GetPacket             (PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptRecvPkt, uint32_t ulRecvBufferSize, uint32_t ulTimeout);
int32_t DEV_GetMBXState           (PCHANNELINSTANCE ptChannel, uint32_t* pulRecvPktCnt, uint32_t* pulSendPktCnt);
int32_t DEV_GetMBXFillLevel       (NETX_MAILBOX_BLOCK_T* ptInst);

int32_t DEV_TransferPacket        (void*                  pvChannel,        CIFX_PACKET* ptSendPkt, CIFX_PACKET* ptRecvPkt,
                                   uint32_t               ulRecvBufferSize, uint32_t     ulTimeout,
                                   PFN_RECV_PKT_CALLBACK  pfnRecvPacket,    void*        pvUser);

int     DEV_IsReady               (PCHANNELINSTANCE ptChannel);
int     DEV_IsRunning             (PCHANNELINSTANCE ptChannel);
int     DEV_IsCommunicating       (PCHANNELINSTANCE ptChannel, int32_t* plError);
int     DEV_WaitForReady_Poll     (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);
int     DEV_WaitForNotReady_Poll  (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);
int     DEV_WaitForRunning_Poll   (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);
int     DEV_WaitForNotRunning_Poll(PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);
int     DEV_WaitForLock_Poll      (PCHANNELINSTANCE ptChannel, PNETX_IO_BLOCK_T ptInst, uint32_t ulTimeout);

int32_t DEV_TriggerWatchdog       (PCHANNELINSTANCE ptChannel, uint32_t ulTriggerCmd, uint32_t* pulTriggerValue);
int32_t DEV_ReadWriteBlock        (PCHANNELINSTANCE ptChannel, void* pvBlock, uint32_t ulOffset, uint32_t ulBlockLen, void* pvDest, uint32_t ulDestLen, uint32_t ulCmd, int fWriteAllowed);
int32_t DEV_DoChannelInit         (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);
int32_t DEV_DoSystemStart         (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam);
int32_t DEV_DoSystemBootstart     (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam);
int32_t DEV_DoUpdateStart         (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam);
int32_t DEV_BusState              (PCHANNELINSTANCE ptChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout);
uint8_t DEV_GetHandshakeBitState  (PCHANNELINSTANCE ptChannel, uint32_t ulBitMsk);

int32_t DEV_DownloadFile               (void*                  pvChannel,
                                        uint32_t               ulChannel,
                                        uint32_t               ulMailboxSize,
                                        uint32_t               ulTransferType,
                                        char*                  szFileName,
                                        uint32_t               ulFileLength,
                                        void*                  pvData,
                                        PFN_TRANSFER_PACKET    pfnTransferPacket,
                                        PFN_PROGRESS_CALLBACK  pfnCallback,
                                        PFN_RECV_PKT_CALLBACK  pfnRecvPacket,
                                        void*                  pvUser);

int32_t DEV_UploadFile                 (void*                  pvChannel,
                                        uint32_t               ulChannel,
                                        uint32_t               ulMailboxSize,
                                        uint32_t               ulTransfertype,
                                        char*                  szFilename,
                                        uint32_t*              pulFileLength,
                                        void*                  pvData,
                                        PFN_TRANSFER_PACKET    pfnTransferPacket,
                                        PFN_PROGRESS_CALLBACK  pfnCallback,
                                        PFN_RECV_PKT_CALLBACK  pfnRecvPacket,
                                        void*                  pvUser);

#ifdef CIFX_TOOLKIT_DMA
  int32_t DEV_DMAState            (PCHANNELINSTANCE ptChannel, uint32_t ulCmd, uint32_t* pulState);
  int32_t DEV_SetupDMABuffers     (PCHANNELINSTANCE ptChannel);
#endif /* CIFX_TOOLKIT_DMA */
#endif /* CIFX_TOOLKIT_FUNCTION_LIST */

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* CIFX_HWFUNCTIONS__H */
