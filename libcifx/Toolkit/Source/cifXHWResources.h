/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXHWResources.h 15529 2026-04-10 13:00:47Z RHornung $:

  Description:
    cifX toolkit hw resources.

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-07-11  Created

**************************************************************************************/
#ifndef CIFX_HWRESOURCES__H
#define CIFX_HWRESOURCES__H

#include "OS_Dependent.h"
#include "cifXDMA.h"
#include "cifXUser.h"
#include "cifXErrors.h"

#include "Hil_DualPortMemory.h"
#include "Hil_HostInterface.h"
#include "Hil_SystemCmd.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/*!  \addtogroup CIFX_TK_STRUCTURE Toolkit Structure Definitions
*    \{                                                                      */
/*****************************************************************************/

#define CIFX_TKIT_IRQ_OTHERDEVICE     0   /*!< cifXTKitISRHandler return, if the IRQ
                                              is a shared PCI IRQ and was not from
                                              the cifX device */
#define CIFX_TKIT_IRQ_HANDLED         1   /*!< cifXTKitISRHandler return, if the IRQ
                                              was handled directly inside ISR */
#define CIFX_TKIT_IRQ_DSR_REQUESTED   2   /*!< cifXTKitISRHandler return, if the IRQ
                                              requires deferred processing. The toolkit
                                              then expects the DSRHandler to be called. */

/* Defines for file/firmware downloads */
#define CIFXTKIT_DOWNLOAD_NONE        0x00 /*!< Set when file download was skipped. Only valid if CIFX_NO_ERROR is returned */
#define CIFXTKIT_DOWNLOAD_FIRMWARE    0x01 /*!< Successfully downloaded a firmware */
#define CIFXTKIT_DOWNLOAD_MODULE      0x02 /*!< Successfully downloaded a firmware */
#define CIFXTKIT_DOWNLOAD_EXECUTED    0x80 /*!< Download was executed */

/* Defines for mailbox states */
#define NETX_MBX_COM_STATE_EMPTY      0
#define NETX_MBX_COM_STATE_FULL       1

/* Defines for IO areas */
#define NETX_IO_STATUS_LOCKSTATE_MSK  0x30
#define NETX_IO_STATUS_BALANCECNT_MSK 0x0E
#define NETX_IO_STATUS_TOGGLEBIT_MSK  0x01

/* Defines for sync handling */
#define NETX_HSK_SYNCH_FLAG_POS       1 /*!< Position of the sync flahs in the HSK channel */
#define NETX_NUM_OF_SYNCH_FLAGS       4 /*!< Number of supported sync flags */

#ifdef CIFX_TOOLKIT_PARAMETER_CHECK
#define CHECK_POINTER(param) if ((void*)NULL == param) return CIFX_INVALID_POINTER;
#define CHECK_DRIVERHANDLE(handle) if (&g_tDriverInfo != handle) return CIFX_INVALID_HANDLE;
#define CHECK_SYSDEVICEHANDLE(handle) if (CIFX_NO_ERROR != CheckSysdeviceHandle(handle)) return CIFX_INVALID_HANDLE;
#define CHECK_CHANNELHANDLE(handle) if (CIFX_NO_ERROR != CheckChannelHandle(handle)) return CIFX_INVALID_HANDLE;
int32_t CheckSysdeviceHandle(CIFXHANDLE hChannel);
int32_t CheckChannelHandle (CIFXHANDLE hChannel);
#else
#define CHECK_POINTER(param)
#define CHECK_DRIVERHANDLE(handle) UNREFERENCED_PARAMETER(handle)
#define CHECK_SYSDEVICEHANDLE(handle)
#define CHECK_CHANNELHANDLE(handle)
#endif

/*****************************************************************************/
/*! Enumeration for different netX chip types                                */
/*****************************************************************************/
typedef enum CIFX_TOOLKIT_CHIPTYPE_Etag
{
  eCHIP_TYPE_UNKNOWN,                       /*!< Chip cannot be identified */
  eCHIP_TYPE_NETX500,                       /*!< netX500 */
  eCHIP_TYPE_NETX100,                       /*!< netX100, can currently only be detected when FW is already running */
  eCHIP_TYPE_NETX50,                        /*!< netX 50 */
  eCHIP_TYPE_NETX10,                        /*!< netX 10 */
  eCHIP_TYPE_NETX51,                        /*!< netX 51 */
  eCHIP_TYPE_NETX52,                        /*!< netX 52 */
  eCHIP_TYPE_NETX4000,                      /*!< netX 4000 */
  eCHIP_TYPE_NETX4100,                      /*!< netX 4100 */
  eCHIP_TYPE_NETX90,                        /*!< netX 90 */
  eCHIP_TYPE_NETX900,                       /*!< netX 9XX */
} CIFX_TOOLKIT_CHIPTYPE_E;

typedef enum CIFX_TOOLKIT_DEVICETYPE_Etag
{
  eCIFX_DEVICE_AUTODETECT = 0,              /*!< Autodetection of device. Default: PCI = RAM and DPM = Flash
                                                 For DPM: If no 'netX' or 'BOOT' cookie is present RAM based
                                                 device will be used                                            */
  eCIFX_DEVICE_AUTODETECT_ERROR,            /*!< Unable to autodetect device type                               */
  eCIFX_DEVICE_RAM_BASED,                   /*!< Always perform a reset on startup and re-download all files    */
  eCIFX_DEVICE_FLASH_BASED,                 /*!< Assume a running bootloader/FW from flash                      */
  eCIFX_DEVICE_DONT_TOUCH                   /*!< Leave the device in the current state and try to connect to it */
} CIFX_TOOLKIT_DEVICETYPE_E;

/*****************************************************************************/
/*! Notification events that can be signaled during cifXTKitAddDevice        */
/*****************************************************************************/
typedef enum CIFX_TOOLKIT_NOTIFY_Etag
{
  eCIFX_TOOLKIT_EVENT_PRERESET = 0,         /*!< Event signalled, before device is reset (HW Reset) */
  eCIFX_TOOLKIT_EVENT_POSTRESET,            /*!< Called after HW reset has been executed            */
  eCIFX_TOOLKIT_EVENT_PRE_BOOTLOADER,       /*!< Called before bootloader is downloaded             */
  eCIFX_TOOLKIT_EVENT_POST_BOOTLOADER       /*!< Called after bootloader was downloaded and started */
} CIFX_TOOLKIT_NOTIFY_E;

typedef void(*PFN_CIFXTK_NOTIFY) (void* pvDeviceInstance, CIFX_TOOLKIT_NOTIFY_E eEvent);

/*****************************************************************************/
/*! Definition for cached IO buffer access                                   */
/*****************************************************************************/
typedef enum CIFX_TOOLKIT_CACHED_MODE_Etag
{
  eCACHED_MODE_OFF = 0,                     /*!< Map IO buffer pointers in default uncached mode */
  eCACHED_MODE_ON                           /*!< Map IO buffer pointers in cached mode */
} CIFX_TOOLKIT_CACHED_MODE_E;

/*****************************************************************************/
/*! Driver information structure used internally in the toolkit              */
/*****************************************************************************/
typedef struct TKIT_DRIVER_INFORMATIONtag
{
  uint32_t ulOpenCount;                     /*!< Number of xDriverOpen calls */
  int      fInitialized;                    /*!< !=1 if the toolkit was initialized successfully */
} TKIT_DRIVER_INFORMATION;



/*****************************************************************************/
/*! Structures shared between DPM & HIF                                      */
/*****************************************************************************/

/*****************************************************************************/
/*! Structure used for COM-state notification                                */
/*****************************************************************************/
typedef struct NETX_COM_STATE_Ttag
{
  PFN_NOTIFY_CALLBACK           pfnCallback;              /*!< Notification callback     */
  void*                         pvUser;                   /*!< User pointer for callback */
} NETX_COM_STATE_T;

/*****************************************************************************/
/*! Structure defining the sync data                                         */
/*****************************************************************************/
typedef struct NETX_SYNC_DATA_Ttag
{
  PFN_NOTIFY_CALLBACK           pfnCallback;              /*!< Notification callback     */
  void*                         pvUser;                   /*!< User pointer for callback */
#if HIF_SUPPORT
  uint32_t                      ulBitmask;                /*!< Bitmask of sync irq       */
  uint32_t                      ulNotifyEvent;            /*!< Event that is signalled via callback */
  void*                         pvEvent;
#endif
} NETX_SYNC_DATA_T;

/*****************************************************************************/
/*! Structure defining cached memory information                             */
/*****************************************************************************/
typedef struct CACHED_MEMORY_AREA_Ttag
{
  void*                         pvMemPtr;                 /*!< Memory pointer to cached area           */
  unsigned long                 ulAreaSize;               /*!< Size of the cached memory area in bytes */
} CACHED_MEMORY_AREA_T;

/*****************************************************************************/
/*! Synchronisation structure
 *   If synchronization is supported, this structure is used to hold the
 *   necessary data. Sync information in device global.
 *   Synch-Flags are located in the handshake channel and 16 bit by default  */
/*****************************************************************************/
typedef struct CIFX_SYNCH_DATA_Ttag
{
  void*     pvLock;                                     /*!< Lock for synchronizing interrupt accesses to flags   */
  uint16_t  usSyncMode;                                 /*!< Synchronisation mode */
  uint16_t  usHSyncFlags;                               /*!< Host synchronisation flags */
  uint16_t  usNSyncFlags;                               /*!< netX synchronisation flags */
  void*     ahSyncBitEvents[HIL_DPM_HANDSHAKE_PAIRS];   /*!< netX synchronisation flags */
} CIFX_SYNCH_DATA_T;



/*****************************************************************************/
/*! -- DPM -- related structures                                             */
/*****************************************************************************/

/*****************************************************************************/
/*! Structure defining an User Block                                         */
/*****************************************************************************/
typedef struct USERINSTANCEtag
{
  uint8_t*                      pbUserBlockStart;         /*!< Pointer to user block start in DPM */
  uint32_t                      ulUserBlockLength;        /*!< Length of user block               */
} USERINSTANCE, *PUSERINSTANCE;

/*****************************************************************************/
/*! Structure defining an I/O Block                                          */
/*****************************************************************************/
typedef struct IOINSTANCEtag
{
  uint8_t*                      pbDPMAreaStart;           /*!< DPM Pointer to start of IO Instance                    */
  uint32_t                      ulDPMAreaLength;          /*!< Length of IO Instance                                  */
  uint8_t                       bHandshakeBit;            /*!< Handshake bit associated with IO Instance              */
  uint16_t                      usHandshakeMode;          /*!< Handshake mode for this IO instance                    */
  uint8_t                       bHandshakeBitState;       /*!< Handshake bit to wait for (depending on Handshake mode */
  void*                         pvMutex;                  /*!< Synchronisation object                                 */
  uint32_t                      ulNotifyEvent;            /*!< Event that is signalled via callback                   */
  PFN_NOTIFY_CALLBACK           pfnCallback;              /*!< Notification callback                                  */
  void*                         pvUser;                   /*!< User pointer for callback                              */
} IOINSTANCE, *PIOINSTANCE;

/*****************************************************************************/
/*! Structure defining the send mailbox                                      */
/*****************************************************************************/
typedef struct NETX_TX_MAILBOX_Ttag
{
  HIL_DPM_SEND_MAILBOX_BLOCK_T* ptSendMailboxStart;       /*!< virtual start address of send mailbox                                 */
  uint32_t                      ulSendMailboxLength;      /*!< Length of send mailbox in bytes                                       */
  uint32_t                      ulSendCMDBitmask;         /*!< Bitmask for Handshakeflags to send packet                             */
  uint8_t                       bSendCMDBitoffset;        /*!< Bitnumber for send packet flag (used for notification array indexing) */
  void*                         pvSendMBXMutex;           /*!< Synchronisation object for the send mailbox                           */
  uint32_t                      ulSendPacketCnt;          /*!< Number of packets sent on this mailbox                                */
  PFN_NOTIFY_CALLBACK           pfnCallback;              /*!< Notification callback                                                 */
  void*                         pvUser;                   /*!< User pointer for callback                                             */
} NETX_TX_MAILBOX_T;

/*****************************************************************************/
/*! Structure defining the receive mailbox                                   */
/*****************************************************************************/
typedef struct NETX_RX_MAILBOX_Ttag
{
  HIL_DPM_RECV_MAILBOX_BLOCK_T* ptRecvMailboxStart;       /*!< virtual start address of receive mailbox                                  */
  uint32_t                      ulRecvMailboxLength;      /*!< Length of receive mailbox in bytes                                        */
  uint32_t                      ulRecvACKBitmask;         /*!< Bitmask for Handshakeflags to ack recv. packet                            */
  uint8_t                       bRecvACKBitoffset;        /*!< Bitnumber for recv packet ack flag (used for notification array indexing) */
  void*                         pvRecvMBXMutex;           /*!< Synchronisation object for the receive mailbox                            */
  uint32_t                      ulRecvPacketCnt;          /*!< Number of packets received on this mailbox                                */
  PFN_NOTIFY_CALLBACK           pfnCallback;              /*!< Notification callback                                                     */
  void*                         pvUser;                   /*!< User pointer for callback                                                 */
} NETX_RX_MAILBOX_T;



/*****************************************************************************/
/*! -- HIF -- related structures                                             */
/*****************************************************************************/

/*****************************************************************************/
/*! Structure for TLB control/handling                                       */
/*****************************************************************************/
typedef struct NETX_TLB_CTL_Ttag
{
  volatile uint32_t*  pulTlbHostStatus;
  volatile uint32_t*  pulTlbNetxStatus;
  uint32_t            ulTlbNetxStatus;
} NETX_TLB_CTL_T, PNETX_TLB_CTL_T;

/*****************************************************************************/
/*! Structure for HS control/handling                                        */
/*****************************************************************************/
typedef struct NETX_HS_CTL_Ttag
{
  volatile uint32_t*  pulHostFlags;
  volatile uint32_t*  pulNetxFlags;
  uint32_t            ulHostFlags;
  uint32_t            ulNetxFlags;
  PFN_NOTIFY_CALLBACK pfnCallback;
  void*               pvUser;
  void*               pvEvent;
} NETX_HS_CTL_T, PNETX_HS_CTL_T;

/*****************************************************************************/
/*! Structure for IO statistics                                              */
/*****************************************************************************/
typedef struct NETX_IO_STAT_Ttag
{
  uint32_t ulExchangeCnt;
  uint32_t ulErrCnt;
  uint32_t ulMissDataCnt;
  uint32_t ulNoUpateCnt;
}NETX_IO_STAT_T, PNETX_IO_STAT_T;

/*****************************************************************************/
/*! Structure for IO control/handling                                        */
/*****************************************************************************/
typedef struct NETX_IO_CTL_Ttag
{
  volatile uint8_t*   pbAction;
  volatile uint8_t*   pbStatus;
  uint8_t             bAction;
  uint8_t             bIrqState;
  uint32_t            ulNotifyEvent;
  PFN_NOTIFY_CALLBACK pfnCallback;
  void*               pvUser;
  void*               pvEvent;
} NETX_IO_CTL_T, PNETX_IO_CTL_T;

/*****************************************************************************/
/*! Structure for IO/MBX blocks                                              */
/*****************************************************************************/
typedef struct NETX_BLOCK_Ttag
{
  uint8_t*            pbBlockStart;             /*!< Start of the block                                                  */
  uint32_t            ulBlockLength;            /*!< Length of the block                                                 */
  uint32_t            ulElementCnt;             /*!< Number of buffers/elements inside the block                         */
  uint32_t            ulElementSize;            /*!< Size of one buffer/element                                          */
  uint32_t            ulTransmissionCnt;        /*!< Counter for sent/received transmissions                             */
  uint32_t            ulBitmask;                /*!< Bitmask of Handshake flags                                          */
  uint8_t             bBitoffset;               /*!< Bitnumber of Handshake flags (used for notification array indexing) */
  void*               pvMutex;                  /*!< Synchronization object for this block                               */
} NETX_BLOCK_T, *PNETX_BLOCK_T;

/*****************************************************************************/
/*! Structure for IO blocks                                                  */
/*****************************************************************************/
typedef struct NETX_IO_BLOCK_Ttag
{
  NETX_BLOCK_T    tBlock;
  NETX_IO_CTL_T   tIoCtl;
  NETX_IO_STAT_T  tStat;
} NETX_IO_BLOCK_T, *PNETX_IO_BLOCK_T;

/*****************************************************************************/
/*! Structure for IO areas                                                   */
/*****************************************************************************/
typedef struct NETX_IO_AREA_Ttag
{
  NETX_IO_BLOCK_T*  aptIOInputAreas[HIL_HIF_MAX_TBUF_CHANNELS];
  NETX_IO_BLOCK_T*  aptIOOutputAreas[HIL_HIF_MAX_TBUF_CHANNELS];
  NETX_TLB_CTL_T    tTlbCtl;
  uint32_t          ulIOInputAreas;
  uint32_t          ulIOOutputAreas;
} NETX_IO_AREA_T, PNETX_IO_AREA_T;

/*****************************************************************************/
/*! Structure for mailbox blocks                                             */
/*****************************************************************************/
typedef struct NETX_MAILBOX_BLOCK_Ttag
{
  NETX_BLOCK_T      tBlock;
  NETX_HS_CTL_T     tCtl;
} NETX_MAILBOX_BLOCK_T, *PNETX_MAILBOX_BLOCK_T;

/*****************************************************************************/
/*! Union for COM/SYS mailbox area                                           */
/*****************************************************************************/
typedef union NETX_MAILBOX_AREA_Utag
{
  NETX_BLOCK_T          tSys;
  NETX_MAILBOX_BLOCK_T  tCom;
} NETX_MAILBOX_AREA_U, *PNETX_MAILBOX_AREA_U;


/*****************************************************************************/
/*! Structure defining a channel instance. Includes elements from DPM and
 *  HIF channel instances.                                                   */
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

  NETX_COM_STATE_T      tComState;                        /*!< defining resources for com-state notification */

  CACHED_MEMORY_AREA_T  tCachedIOInputArea;               /*!< Information about cached IO input memory area */
  CACHED_MEMORY_AREA_T  tCachedIOOutputArea;              /*!< Information about cached IO input memory area */

#if DPM_SUPPORT
  /* DPM related structures */
  int                   fIsChannel;                       /*!< !=0 this is a real channel                           */
  uint32_t              ulBlockID;                        /*!< Block ID                                        */

  NETX_TX_MAILBOX_T     tSendMbx;                         /*!< Send mailbox administration structure   */
  NETX_RX_MAILBOX_T     tRecvMbx;                         /*!< Receive mailbox administration structure*/

  uint16_t              usHostFlags;                      /*!< Copy of the last actual command flags   */
  uint16_t              usNetxFlags;                      /*!< Copy of the last read status flags      */

  uint32_t              ulDeviceCOSFlags;                 /*!< Device COS flags (copy, updated when COS Handshake is recognized) */
  uint32_t              ulDeviceCOSFlagsChanged;          /*!< Bitmask of changed bits since last COS Handshake                  */
  uint32_t              ulHostCOSFlags;                   /*!< Host COS flags (copy)                      */
  uint32_t              ulHostCOSFlagsSaved;              /*!< Actual written Host COS flags              */

  HIL_DPM_CONTROL_BLOCK_T*          ptControlBlock;       /*!< Pointer to channel's control block         */
  uint8_t                           bControlBlockBit;     /*!< Handshake bit associated with control block*/
  uint32_t                          ulControlBlockSize;   /*!< Size of the control block in bytes         */

  HIL_DPM_COMMON_STATUS_BLOCK_T*    ptCommonStatusBlock;  /*!< Pointer to channel's common status block   */
  uint8_t                           bCommonStatusBit;     /*!< Handshake bit associated with Common status block*/
  uint32_t                          ulCommonStatusSize;   /*!< Size of the common status block in bytes   */

  HIL_DPM_EXTENDED_STATUS_BLOCK_T*  ptExtendedStatusBlock;/*!< Pointer to channel's extended status block */
  uint8_t                           bExtendedStatusBit;   /*!< Handshake bit associated with Extended status block*/
  uint32_t                          ulExtendedStatusSize; /*!< Size of the extended status block in bytes */

  HIL_DPM_HANDSHAKE_CELL_T*         ptHandshakeCell;      /*!< pointer to channels handshake cell   */
  uint8_t                           bHandshakeWidth;      /*!< Width of the handshake cell          */

  void*                 ahHandshakeBitEvents[HIL_DPM_HANDSHAKE_PAIRS]; /*!< Event handle for each handshake bit pair. (used in interrupt mode) */

  PIOINSTANCE*          pptIOInputAreas;                  /*!< Input Areas array for this channel   */
  uint32_t              ulIOInputAreas;                   /*!< Number of Input areas                */

  PIOINSTANCE*          pptIOOutputAreas;                 /*!< Output Areas array for this channel  */
  uint32_t              ulIOOutputAreas;                  /*!< Number of Output areas               */

  PUSERINSTANCE*        pptUserAreas;                     /*!< User areas for this channel          */
  uint32_t              ulUserAreas;                      /*!< Number of user areas                 */

  NETX_SYNC_DATA_T      tSynch;                           /*!< Sync handling                        */
#endif

#if HIF_SUPPORT
  /* HIF related structures */
  NETX_MAILBOX_AREA_U   tFromHostMbx;                     /*!< Send mailbox administration structure   */
  NETX_MAILBOX_AREA_U   tToHostMbx;                       /*!< Receive mailbox administration structure*/

  HIL_HIF_COMMUNICATION_STATUS_BLOCK_T* ptCommunicationStatusBlock;  /*!< Pointer to channel's communication status block   */

  NETX_HS_CTL_T         tHsCtrl;

  void*                 apvHsBitEvent[HIL_HIF_HSC_MAX];   /*!< Event handles for each handshake bit pair (used in interrupt mode) */

  NETX_IO_AREA_T        tIoArea;                          /*!< Input/Output areas */
  NETX_SYNC_DATA_T      atSynch[NETX_NUM_OF_SYNCH_FLAGS]; /*!< Sync handling                        */
#endif

} CHANNELINSTANCE, *PCHANNELINSTANCE;

/*****************************************************************************/
/*! Structure for ISR and DSR handling                                       */
/*****************************************************************************/
typedef struct IRQ_TO_DSR_BUFFER_Ttag
{
#if DPM_SUPPORT
  HIL_DPM_HANDSHAKE_ARRAY_T tHandshakeBuffer;
#endif
#if HIF_SUPPORT
  uint32_t                  aulHsk[HIL_HIF_HSC_MAX];
  uint32_t                  ulTlbStatus;
#endif
  int                       fValid;
} IRQ_TO_DSR_BUFFER_T;

/*****************************************************************************/
/*! Structure defining a physical device passed to the toolkit. Passing it,
 *  will create all logical device associated with this instance. Includes
 *  elements from DPM and HIF device instances.                              */
/*****************************************************************************/
typedef struct DEVICEINSTANCEtag
{
  struct CIFX_API_FUNCTION_LIST_Ttag*   ptCifxFun;  /*!< Function pointer to CIFX API functions                   */
  struct CIFX_DEV_FUNCTION_LIST_Ttag*   ptDevFun;   /*!< Function pointer to DEV API functions                    */
  struct CIFX_TKIT_FUNCTION_LIST_Ttag*  ptTkitFun;  /*!< Function pointer to TKIT API functions                   */

  uint32_t                  ulPhysicalAddress;      /*!< Physical address of the cifX card                        */
  uint32_t                  ulIrqNumber;            /*!< IRQ number assigned to card                              */
  int                       fIrqEnabled;            /*!< !=0 if the IRQ is used on this device                    */

  int                       fPCICard;               /*!< !=0 if the card is a PCI card (netX directly connected to PCI)
                                                         ,this will reset the netX if eDeviceType is AUTODETECT or RAMBASE */

  CIFX_TOOLKIT_DEVICETYPE_E eDeviceType;            /*!< Type of the device. If set to AUTODETECT it will be updated during
                                                         cifXAddDevice                                                       */
  void*                     pvOSDependent;          /*!< OS dependent pointer to device identification (used for PCI read/write request).
                                                         This parameter must allow the OS/User to identify the card and access it's PCI registers */
  uint8_t*                  pbDPM;                  /*!< Virtual/usable pointer to the cards DPM              */
  uint32_t                  ulDPMSize;              /*!< Size of the cards DPM                                */
  uint8_t                   bDPMLayout;             /*!< Layout of the DPM                                    */
  CIFX_TOOLKIT_CHIPTYPE_E   eChipType;              /*!< Type of chip */

  char                      szName[CIFx_MAX_INFO_NAME_LENTH]; /*!< Default name of the card, must be inserted by user */
  char                      szAlias[CIFx_MAX_INFO_NAME_LENTH];/*!< Alias name of the card, must be inserted by user   */

  int32_t                   lInitError;             /*!< Initialization error of the card                         */

  void*                     pvGlobalRegisters;      /*!< Pointer to the global host registers (only available on PCI)  */
  uint32_t                  ulSerialNumber;         /*!< Serial number of the card (read on startup)        */
  uint32_t                  ulDeviceNumber;         /*!< Device number of the card (read on startup)        */
  uint32_t                  ulSlotNumber;           /*!< Slot number on card (read on startup)              */

#if DPM_SUPPORT
  int                       fModuleLoad;            /*!< This devices works with modules */
  PFN_CIFXTK_NOTIFY         pfnNotify;              /*!< Function to notify user of different states in the toolkit, to allow
                                                         memory controller reconfiguration, etc. */
  uint8_t*                  pbHandshakeBlock;       /*!< Pointer to start of Handshake block (NULL if no handshake block was found */

  /* Synch handling */
  CIFX_SYNCH_DATA_T         tSyncData;              /*!< Synchronization structure */

#endif

#if HIF_SUPPORT
  uint32_t*                 pulHandshakeBlock;      /*!< Pointer to start of Handshake block (NULL if no handshake block was found */
#endif

  int                       iIrqToDsrBuffer;        /*!< IRQ to DSR Buffer number to use               */
  IRQ_TO_DSR_BUFFER_T       atIrqToDsrBuffer[2];    /*!< IRQ to DSR Buffers                            */
  uint32_t                  ulIrqCounter;           /*!< Number of interrupts processed on this device */

  CHANNELINSTANCE           tSystemDevice;          /*!< Every card has at least one SystemDevice             */
  uint32_t                  ulCommChannelCount;     /*!< Number of fount communication channels on the card   */
  CHANNELINSTANCE**         pptCommChannels;        /*!< Array of all found channels                          */

#ifdef CIFX_TOOLKIT_DMA
  uint32_t                  ulDMABufferCount;                     /*!< Number of available DMA buffers  */
  CIFX_DMABUFFER_T          atDmaBuffers[CIFX_DMA_BUFFER_COUNT];  /*!< DMA buffer definition for the device */
#endif

  int                       fCachedMemAccess;                     /*!< Cached memory access to DMA buffer         */


  int                       fResetActive;           /*!< !=0 if a reset is pending on device (DEV_DoSystemStart) */

  /* Extended memory (additional target memory) */
  uint8_t*                  pbExtendedMemory;       /*!< Virtual/usable pointer to an extended memory area       */
  uint32_t                  ulExtendedMemorySize;   /*!< Size of the extended memory area                        */

#ifdef CIFX_TOOLKIT_HWIF
  PFN_HWIF_MEMCPY           pfnHwIfRead;            /*!< Definable hardware read function                        */
  PFN_HWIF_MEMCPY           pfnHwIfWrite;           /*!< Definable hardware read function                        */
#endif /* CIFX_TOOLKIT_HWIF */
} DEVICEINSTANCE, *PDEVICEINSTANCE;

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* CIFX_HWRESOURCES__H */
