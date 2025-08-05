/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXHWResources.h 15171 2025-08-05 08:18:45Z AMinor $:

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
#include "cifXUser.h"

#include "Hil_DualPortMemory.h"
#include "Hil_HostInterface.h"

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
#define NETX_IO_STATUS_TOGGLEBIT_MSK  0x01

/* Defines for sync handling */
#define NETX_HSK_SYNCH_FLAG_POS       1 /*!< Position of the sync flahs in the HSK channel */
#define NETX_NUM_OF_SYNCH_FLAGS       4 /*!< Number of supported sync flags */

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
  uint32_t            ulTlbHostStatus;
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
/*! Structure for IO control/handling                                        */
/*****************************************************************************/
typedef struct NETX_IO_CTL_Ttag
{
  volatile uint8_t*   pbAction;
  volatile uint8_t*   pbStatus;
  uint8_t             bAction;
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
/*! \}                                                                       */
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* CIFX_HWRESOURCES__H */
