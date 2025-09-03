/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXFunctionList.h 15171 2025-08-05 08:18:45Z AMinor $:

  Description:
    cifX toolkit function list.

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-07-11  Created

**************************************************************************************/
#ifndef CIFX_FUNCTION_LIST__H
#define CIFX_FUNCTION_LIST__H

#include "cifXUser.h"
#include "cifXDMA.h"
#include "cifXHwif.h"
#include "cifXHWResources.h"
#include "Hil_SystemCmd.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef int32_t (*PFN_TRANSFER_PACKET) (void*                  pvChannel,
                                        CIFX_PACKET*           ptSendPkt,
                                        CIFX_PACKET*           ptRecvPkt,
                                        uint32_t               ulRecvBufferSize,
                                        uint32_t               ulTimeout,
                                        PFN_RECV_PKT_CALLBACK  pfnPktCallback,
                                        void*                  pvUser);

#ifndef CIFX_TOOLKIT_FUNCTION_LIST

#define CIFX_STATIC
#define CIFX_MAKE_CIFX_FUN(_function) (_function)
#define CIFX_MAKE_DEV_FUN(_function)  (_function)
#define CIFX_MAKE_TKIT_FUN(_function) (_function)

#else

#define CIFX_STATIC                   static
#define CIFX_MAKE_CIFX_FUN(_function) (ptDevInstance->ptCifxFun->pfn ## _function)
#define CIFX_MAKE_DEV_FUN(_function)  (ptDevInstance->ptDevFun->pfn ## _function)
#define CIFX_MAKE_TKIT_FUN(_function) (ptDevInstance->ptTkitFun->pfn ## _function)


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
  uint32_t              ulBlockID;                        /*!< Block ID                                        */

  void*                 pvLock;                           /*!< Lock for synchronizing interrupt accesses to flags   */
  uint32_t              ulOpenCount;                      /*!< Number of open device function called for channel    */

  int                   fIsSysDevice;                     /*!< !=0 if the channel instance belong to a systemdevice */
  int                   fIsChannel;                       /*!< !=0 this is a real channel                           */

  HIL_FW_IDENTIFICATION_T tFirmwareIdent;                 /*!< Firmware Identification                         */

  NETX_TX_MAILBOX_T     tSendMbx;                         /*!< Send mailbox administration structure   */
  NETX_RX_MAILBOX_T     tRecvMbx;                         /*!< Receive mailbox administration structure*/

  NETX_COM_STATE_T      tComState;                        /*!< defining resources for com-state notification */

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

  CACHED_MEMORY_AREA_T  tCachedIOInputArea;               /*!< Information about cached IO input memory area */
  CACHED_MEMORY_AREA_T  tCachedIOOutputArea;              /*!< Information about cached IO input memory area */


  /* HIF related structures */
  NETX_MAILBOX_AREA_U   tFromHostMbx;                     /*!< Send mailbox administration structure   */
  NETX_MAILBOX_AREA_U   tToHostMbx;                       /*!< Receive mailbox administration structure*/

  HIL_HIF_COMMUNICATION_STATUS_BLOCK_T* ptCommunicationStatusBlock;  /*!< Pointer to channel's communication status block   */

  NETX_HS_CTL_T         tHsCtrl;

  void*                 apvHsBitEvent[HIL_HIF_NCF_RESTART_REQUIRED_BIT_NO + 1]; /*!< Event handles for each handshake bit pair (used in interrupt mode) */

  NETX_IO_AREA_T        tIoArea;                          /*!< Input/Output areas */

} CHANNELINSTANCE, *PCHANNELINSTANCE;

/*****************************************************************************/
/*! Structure for ISR and DSR handling                                       */
/*****************************************************************************/
typedef struct IRQ_TO_DSR_BUFFER_Ttag
{
  HIL_DPM_HANDSHAKE_ARRAY_T tHandshakeBuffer;
  uint32_t                  aulHsk[HIL_HIF_HSC_MAX];
  uint32_t                  ulTlbStatus;
  int                       fValid;
} IRQ_TO_DSR_BUFFER_T;

/*****************************************************************************/
/*! Structure defining a physical device passed to the toolkit. Passing it,
*   will create all logical device associated with this instance             */
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
  int                       fModuleLoad;            /*!< This devices works with modules */

  CIFX_TOOLKIT_DEVICETYPE_E eDeviceType;            /*!< Type of the device. If set to AUTODETECT it will be updated during
                                                         cifXAddDevice                                                       */
  PFN_CIFXTK_NOTIFY         pfnNotify;              /*!< Function to notify user of different states in the toolkit, to allow
                                                         memory controller reconfiguration, etc. */
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

  uint8_t*                  pbHandshakeBlock;       /*!< Pointer to start of Handshake block (NULL if no handshake block was found */
  uint32_t*                 pulHandshakeBlock;      /*!< Pointer to start of Handshake block (NULL if no handshake block was found */
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

  /* Synch handling */
  CIFX_SYNCH_DATA_T         tSyncData;              /*!< Synchronization structure */

  int                       fResetActive;           /*!< !=0 if a reset is pending on device (DEV_DoSystemStart) */

  /* Extended memory (additional target memory) */
  uint8_t*                  pbExtendedMemory;       /*!< Virtual/usable pointer to an extended memory area       */
  uint32_t                  ulExtendedMemorySize;   /*!< Size of the extended memory area                        */

#ifdef CIFX_TOOLKIT_HWIF
  PFN_HWIF_MEMCPY           pfnHwIfRead;            /*!< Definable hardware read function                        */
  PFN_HWIF_MEMCPY           pfnHwIfWrite;           /*!< Definable hardware read function                        */
#endif /* CIFX_TOOLKIT_HWIF */
} DEVICEINSTANCE, *PDEVICEINSTANCE;


/* Function pointer typedef's for cifX Toolkit API functions. */
typedef int32_t (*PFN_CIFXTKITINIT)               (void);
typedef void    (*PFN_CIFXTKITDEINIT)             (void);
typedef int32_t (*PFN_CIFXTKITADDDEVICE)          (PDEVICEINSTANCE ptDevInstance);
typedef int32_t (*PFN_CIFXTKITREMOVEDEVICE)       (char* szBoard, int fForceRemove);
typedef void    (*PFN_cifXTKITENABLEHWINTERRUPT)  (PDEVICEINSTANCE ptDevInstance);
typedef void    (*PFN_cifXTKITDISABLEHWINTERRUPT) (PDEVICEINSTANCE ptDevInstance);
typedef int     (*PFN_cifXTKITISRHANDLER)         (PDEVICEINSTANCE ptDevInstance, int fPCIIgnoreGlobalIntFlag);
typedef void    (*PFN_cifXTKITDSRHANDLER)         (PDEVICEINSTANCE ptDevInstance);
typedef void    (*PFN_cifXTKITCYCLICTIMER)        (void);

/* Function list structure for the cifX Toolkit API functions. */
typedef struct CIFX_TKIT_FUNCTION_LIST_Ttag
{
  PFN_CIFXTKITINIT                pfncifXTKitInit;
  PFN_CIFXTKITDEINIT              pfncifXTKitDeinit;
  PFN_CIFXTKITADDDEVICE           pfncifXTKitAddDevice;
  PFN_CIFXTKITREMOVEDEVICE        pfncifXTKitRemoveDevice;
  PFN_cifXTKITENABLEHWINTERRUPT   pfncifXTKitEnableHWInterrupt;
  PFN_cifXTKITDISABLEHWINTERRUPT  pfncifXTKitDisableHWInterrupt;
  PFN_cifXTKITISRHANDLER          pfncifXTKitISRHandler;
  PFN_cifXTKITDSRHANDLER          pfncifXTKitDSRHandler;
  PFN_cifXTKITCYCLICTIMER         pfncifXTKitCyclicTimer;
} CIFX_TKIT_FUNCTION_LIST_T, *PCIFX_TKIT_FUNCTION_LIST_T;


/* Function list structure for cifX API functions. The function pointer
 * typedef's are already defined in cifXUser.h. */
typedef struct CIFX_API_FUNCTION_LIST_Ttag
{
  PFN_XDRIVEROPEN                     pfnxDriverOpen;
  PFN_XDRIVERCLOSE                    pfnxDriverClose;
  PFN_XDRIVERGETINFORMATION           pfnxDriverGetInformation;
  PFN_XDRIVERGETERRORDESCRIPTION      pfnxDriverGetErrorDescription;
  PFN_XDRIVERENUMBOARDS               pfnxDriverEnumBoards;
  PFN_XDRIVERENUMCHANNELS             pfnxDriverEnumChannels;
  PFN_XDRIVERMEMORYPOINTER            pfnxDriverMemoryPointer;
  PFN_XDRIVERRESTARTDEVICE            pfnxDriverRestartDevice;
  PFN_XSYSDEVICEOPEN                  pfnxSysdeviceOpen;
  PFN_XSYSDEVICECLOSE                 pfnxSysdeviceClose;
  PFN_XSYSDEVICEGETMBXSTATE           pfnxSysdeviceGetMBXState;
  PFN_XSYSDEVICEPUTPACKET             pfnxSysdevicePutPacket;
  PFN_XSYSDEVICEGETPACKET             pfnxSysdeviceGetPacket;
  PFN_XSYSDEVICEINFO                  pfnxSysdeviceInfo;
  PFN_XSYSDEVICEFINDFIRSTFILE         pfnxSysdeviceFindFirstFile;
  PFN_XSYSDEVICEFINDNEXTFILE          pfnxSysdeviceFindNextFile;
  PFN_XSYSDEVICEDOWNLOAD              pfnxSysdeviceDownload;
  PFN_XSYSDEVICEUPLOAD                pfnxSysdeviceUpload;
  PFN_XSYSDEVICERESET                 pfnxSysdeviceReset;
  PFN_XSYSDEVICERESETEX               pfnxSysdeviceResetEx;
  PFN_XSYSDEVICEBOOTSTART             pfnxSysdeviceBootstart;
  PFN_XSYSDEVICEEXTENDEDMEMORY        pfnxSysdeviceExtendedMemory;
  PFN_XCHANNELOPEN                    pfnxChannelOpen;
  PFN_XCHANNELCLOSE                   pfnxChannelClose;
  PFN_XCHANNELFINDFIRSTFILE           pfnxChannelFindFirstFile;
  PFN_XCHANNELFINDNEXTFILE            pfnxChannelFindNextFile;
  PFN_XCHANNELDOWNLOAD                pfnxChannelDownload;
  PFN_XCHANNELUPLOAD                  pfnxChannelUpload;
  PFN_XCHANNELGETMBXSTATE             pfnxChannelGetMBXState;
  PFN_XCHANNELPUTPACKET               pfnxChannelPutPacket;
  PFN_XCHANNELGETPACKET               pfnxChannelGetPacket;
  PFN_XCHANNELGETSENDPACKET           pfnxChannelGetSendPacket;
  PFN_XCHANNELCONFIGLOCK              pfnxChannelConfigLock;
  PFN_XCHANNELRESET                   pfnxChannelReset;
  PFN_XCHANNELINFO                    pfnxChannelInfo;
  PFN_XCHANNELWATCHDOG                pfnxChannelWatchdog;
  PFN_XCHANNELHOSTSTATE               pfnxChannelHostState;
  PFN_XCHANNELBUSSTATE                pfnxChannelBusState;
  PFN_XCHANNELDMASTATE                pfnxChannelDMAState;
  PFN_XCHANNELIOINFO                  pfnxChannelIOInfo;
  PFN_XCHANNELIOREAD                  pfnxChannelIORead;
  PFN_XCHANNELIOWRITE                 pfnxChannelIOWrite;
  PFN_XCHANNELIOREADSENDDATA          pfnxChannelIOReadSendData;
  PFN_XCHANNELCONTROLBLOCK            pfnxChannelControlBlock;
  PFN_XCHANNELCOMMONSTATUSBLOCK       pfnxChannelCommonStatusBlock;
  PFN_XCHANNELEXTENDEDSTATUSBLOCK     pfnxChannelExtendedStatusBlock;
  PFN_XCHANNELUSERBLOCK               pfnxChannelUserBlock;
  PFN_XCHANNELPLCMEMORYPTR            pfnxChannelPLCMemoryPtr;
  PFN_XCHANNELPLCISREADREADY          pfnxChannelPLCIsReadReady;
  PFN_XCHANNELPLCISWRITEREADY         pfnxChannelPLCIsWriteReady;
  PFN_XCHANNELPLCACTIVATEWRITE        pfnxChannelPLCActivateWrite;
  PFN_XCHANNELPLCACTIVATEREAD         pfnxChannelPLCActivateRead;
  PFN_XCHANNELREGISTERNOTIFICATION    pfnxChannelRegisterNotification;
  PFN_XCHANNELUNREGISTERNOTIFICATION  pfnxChannelUnregisterNotification;
  PFN_XCHANNELSYNCSTATE               pfnxChannelSyncState;
} CIFX_API_FUNCTION_LIST_T, *PCIFX_API_FUNCTION_LIST_T;


/* Function pointer typedef's for DEV API functions. */
typedef void    (*PFN_DEV_WRITEHANDSHAKEFLAGS)            (PCHANNELINSTANCE ptChannel);
typedef void    (*PFN_DEV_READHOSTFLAGS)                  (PCHANNELINSTANCE ptChannel, int fReadHostCOS);
typedef void    (*PFN_DEV_READHANDSHAKEFLAGS)             (PCHANNELINSTANCE ptChannel, int fReadSyncFlags, int fLockNeeded);

typedef uint8_t (*PFN_DEV_GETIOBITSTATE)                  (PCHANNELINSTANCE ptChannel, PIOINSTANCE ptIOInstance, int fOutput);
typedef int     (*PFN_DEV_WAITFORBITSTATE)                (PCHANNELINSTANCE ptChannel, uint32_t ulBitNumber, uint8_t bState, uint32_t ulTimeout);
typedef int     (*PFN_DEV_WAITFORIOBITSTATE)              (PCHANNELINSTANCE ptChannel, NETX_IO_BLOCK_T* ptInst, uint32_t ulTimeout);
typedef int     (*PFN_DEV_WAITFORMBXSTATE)                (PCHANNELINSTANCE ptChannel, PNETX_MAILBOX_BLOCK_T ptInst, uint32_t ulState, uint32_t ulTimeout);
typedef void    (*PFN_DEV_TOGGLEBIT)                      (PCHANNELINSTANCE ptChannel, uint32_t ulBitMask);
typedef void    (*PFN_DEV_TOGGLEIOACTION)                 (PCHANNELINSTANCE ptChannel, NETX_IO_BLOCK_T* ptInst);
typedef void    (*PFN_DEV_WRITECELL)                      (PCHANNELINSTANCE ptChannel, PNETX_MAILBOX_BLOCK_T ptInst, uint32_t ulValue);
typedef int     (*PFN_DEV_WAITFORSYNCSTATE)               (PCHANNELINSTANCE ptChannel, uint8_t bState, uint32_t ulTimeout);

typedef void    (*PFN_DEV_TOGGLESYNCBIT)                  (PDEVICEINSTANCE  ptDevInstance, uint32_t ulBitMask);
typedef int32_t (*PFN_DEV_PUTPACKET)                      (PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptSendPkt, uint32_t ulTimeout);
typedef int32_t (*PFN_DEV_GETPACKET)                      (PCHANNELINSTANCE ptChannel, CIFX_PACKET* ptRecvPkt, uint32_t ulRecvBufferSize, uint32_t ulTimeout);
typedef int32_t (*PFN_DEV_GETMBXSTATE)                    (PCHANNELINSTANCE ptChannel, uint32_t* pulRecvPktCnt, uint32_t* pulSendPktCnt);
typedef int32_t (*PFN_DEV_GETMBXFILLLEVEL)                (PNETX_MAILBOX_BLOCK_T  ptInst);
typedef int32_t (*PFN_DEV_TRANSFERPACKET)                 (void*                  pvChannel,        CIFX_PACKET* ptSendPkt, CIFX_PACKET* ptRecvPkt,
                                                           uint32_t               ulRecvBufferSize, uint32_t     ulTimeout,
                                                           PFN_RECV_PKT_CALLBACK  pfnRecvPacket,    void*        pvUser);
typedef int     (*PFN_DEV_ISREADY)                        (PCHANNELINSTANCE ptChannel);
typedef int     (*PFN_DEV_ISRUNNING)                      (PCHANNELINSTANCE ptChannel);
typedef int     (*PFN_DEV_ISCOMMUNICATING)                (PCHANNELINSTANCE ptChannel, int32_t* plError);
typedef int     (*PFN_DEV_WAITFORREADY_POLL)              (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);
typedef int     (*PFN_DEV_WAITFORNOTREADY_POLL)           (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);
typedef int     (*PFN_DEV_WAITFORRUNNING_POLL)            (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);
typedef int     (*PFN_DEV_WAITFORNOTRUNNING_POLL)         (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);
typedef int     (*PFN_DEV_WAITFORLOCK_POLL)               (PCHANNELINSTANCE ptChannel, PNETX_IO_BLOCK_T ptInst, uint32_t ulTimeout);

typedef int32_t (*PFN_DEV_TRIGGERWATCHDOG)                (PCHANNELINSTANCE ptChannel, uint32_t ulTriggerCmd, uint32_t* pulTriggerValue);
typedef int32_t (*PFN_DEV_GETHOSTSTATE)                   (PCHANNELINSTANCE ptChannel, uint32_t* pulState);
typedef int32_t (*PFN_DEV_SETHOSTSTATE)                   (PCHANNELINSTANCE ptChannel, uint32_t ulNewState, uint32_t ulTimeout);
typedef int32_t (*PFN_DEV_READWRITEBLOCK)                 (PCHANNELINSTANCE ptChannel, void* pvBlock,      uint32_t ulOffset, uint32_t ulBlockLen,
                                                           void* pvDest,               uint32_t ulDestLen, uint32_t ulCmd,    int      fWriteAllowed);
typedef int32_t (*PFN_DEV_DOCHANNELINIT)                  (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout);
typedef int32_t (*PFN_DEV_DOSYSTEMSTART)                  (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam);
typedef int32_t (*PFN_DEV_DOSYSTEMBOOTSTART)              (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam);
typedef int32_t (*PFN_DEV_DOUPDATESTART)                  (PCHANNELINSTANCE ptChannel, uint32_t ulTimeout, uint32_t ulParam);
typedef int32_t (*PFN_DEV_BUSSTATE)                       (PCHANNELINSTANCE ptChannel, uint32_t ulCmd, uint32_t* pulState, uint32_t ulTimeout);
typedef int32_t (*PFN_DEV_DOHOSTCOSCHANGE)                (PCHANNELINSTANCE ptChannel,          uint32_t ulSetCOSMask,     uint32_t ulClearCOSMask,
                                                           uint32_t         ulPostClearCOSMask, int32_t  lSignallingError, uint32_t ulTimeout);
typedef void    (*PFN_DEV_CHECKCOSFLAGS)                  (PDEVICEINSTANCE ptDevInstance);
typedef uint8_t (*PFN_DEV_GETHANDSHAKEBITSTATE)           (PCHANNELINSTANCE ptChannel, uint32_t ulBitMsk);

typedef int32_t (*PFN_DEV_REMOVECHANNELFILES)             (PCHANNELINSTANCE         ptChannel,
                                                           uint32_t                 ulChannel,
                                                           PFN_TRANSFER_PACKET      pfnTransferPacket,
                                                           PFN_RECV_PKT_CALLBACK    pfnRecvPacket,
                                                           void*                    pvUser,
                                                           char*                    szExceptFile);
typedef int32_t (*PFN_DEV_DELETEFILE)                     (void*                    pvChannel,
                                                           uint32_t                 ulChannelNumber,
                                                           char*                    pszFileName,
                                                           PFN_TRANSFER_PACKET      pfnTransferPacket,
                                                           PFN_RECV_PKT_CALLBACK    pfnRecvPacket,
                                                           void*                    pvUser);
typedef int32_t (*PFN_DEV_CHECKFORDOWNLOAD)               (void*                    pvChannel,
                                                           uint32_t                 ulChannelNumber,
                                                           int*                     pfDownload,
                                                           char*                    pszFileName,
                                                           void*                    pvFileData,
                                                           uint32_t                 ulFileSize,
                                                           PFN_TRANSFER_PACKET      pfnTransferPacket,
                                                           PFN_RECV_PKT_CALLBACK    pfnRecvPacket,
                                                           void*                    pvUser);
typedef int32_t (*PFN_DEV_ISFWFILE)                       (char*                    pszFileName);
typedef int32_t (*PFN_DEV_ISNXFFILE)                      (char*                    pszFileName);
typedef int32_t (*PFN_DEV_ISNXOFILE)                      (char*                    pszFileName);
typedef int32_t (*PFN_DEV_GETFWTRANSFERTYPEFROMFILENAME)  (CIFX_TOOLKIT_CHIPTYPE_E  eChipType,
                                                           char*                    pszFileName,
                                                           uint32_t*                pulTransperType);
typedef int32_t (*PFN_DEV_PROCESSFWDOWNLOAD)              (PDEVICEINSTANCE          ptDevInstance,
                                                           uint32_t                 ulChannel,
                                                           char*                    pszFullFileName,
                                                           char*                    pszFileName,
                                                           uint32_t                 ulFileLength,
                                                           uint8_t*                 pbBuffer,
                                                           uint8_t*                 pbLoadState,
                                                           PFN_TRANSFER_PACKET      pfnTransferPacket,
                                                           PFN_PROGRESS_CALLBACK    pfnCallback,
                                                           PFN_RECV_PKT_CALLBACK    pfnRecvPktCallback,
                                                           void*                    pvUser);
typedef int32_t (*PFN_DEV_DOWNLOADFILE)                   (void*                  pvChannel,
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
typedef int32_t (*PFN_DEV_UPLOADFILE)                     (void*                  pvChannel,
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
typedef int32_t (*PFN_DEV_DMASTATE)                       (PCHANNELINSTANCE ptChannel, uint32_t ulCmd, uint32_t* pulState);
typedef int32_t (*PFN_DEV_SETUPDMABUFFERS)                (PCHANNELINSTANCE ptChannel);
#endif


/* Function list structure for DEV API functions. */
typedef struct CIFX_DEV_FUNCTION_LIST_Ttag
{
  PFN_DEV_WRITEHANDSHAKEFLAGS           pfnDEV_WriteHandshakeFlags;
  PFN_DEV_READHOSTFLAGS                 pfnDEV_ReadHostFlags;
  PFN_DEV_READHANDSHAKEFLAGS            pfnDEV_ReadHandshakeFlags;
  PFN_DEV_GETIOBITSTATE                 pfnDEV_GetIOBitstate;
  PFN_DEV_WAITFORBITSTATE               pfnDEV_WaitForBitState;
  PFN_DEV_WAITFORIOBITSTATE             pfnDEV_WaitForIoBitState;
  PFN_DEV_WAITFORMBXSTATE               pfnDEV_WaitForMbxState;
  PFN_DEV_TOGGLEBIT                     pfnDEV_ToggleBit;
  PFN_DEV_TOGGLEIOACTION                pfnDEV_ToggleIoAction;
  PFN_DEV_WRITECELL                     pfnDEV_WriteCell;
  PFN_DEV_WAITFORSYNCSTATE              pfnDEV_WaitForSyncState;
  PFN_DEV_TOGGLESYNCBIT                 pfnDEV_ToggleSyncBit;
  PFN_DEV_PUTPACKET                     pfnDEV_PutPacket;
  PFN_DEV_GETPACKET                     pfnDEV_GetPacket;
  PFN_DEV_GETMBXSTATE                   pfnDEV_GetMBXState;
  PFN_DEV_GETMBXFILLLEVEL               pfnDEV_GetMBXFillLevel;
  PFN_DEV_TRANSFERPACKET                pfnDEV_TransferPacket;
  PFN_DEV_ISREADY                       pfnDEV_IsReady;
  PFN_DEV_ISRUNNING                     pfnDEV_IsRunning;
  PFN_DEV_ISCOMMUNICATING               pfnDEV_IsCommunicating;
  PFN_DEV_WAITFORREADY_POLL             pfnDEV_WaitForReady_Poll;
  PFN_DEV_WAITFORNOTREADY_POLL          pfnDEV_WaitForNotReady_Poll;
  PFN_DEV_WAITFORRUNNING_POLL           pfnDEV_WaitForRunning_Poll;
  PFN_DEV_WAITFORNOTRUNNING_POLL        pfnDEV_WaitForNotRunning_Poll;
  PFN_DEV_WAITFORLOCK_POLL              pfnDEV_WaitForLock_Poll;
  PFN_DEV_TRIGGERWATCHDOG               pfnDEV_TriggerWatchdog;
  PFN_DEV_GETHOSTSTATE                  pfnDEV_GetHostState;
  PFN_DEV_SETHOSTSTATE                  pfnDEV_SetHostState;
  PFN_DEV_READWRITEBLOCK                pfnDEV_ReadWriteBlock;
  PFN_DEV_DOCHANNELINIT                 pfnDEV_DoChannelInit;
  PFN_DEV_DOSYSTEMSTART                 pfnDEV_DoSystemStart;
  PFN_DEV_DOSYSTEMBOOTSTART             pfnDEV_DoSystemBootstart;
  PFN_DEV_DOUPDATESTART                 pfnDEV_DoUpdateStart;
  PFN_DEV_BUSSTATE                      pfnDEV_BusState;
  PFN_DEV_DOHOSTCOSCHANGE               pfnDEV_DoHostCOSChange;
  PFN_DEV_CHECKCOSFLAGS                 pfnDEV_CheckCOSFlags;
  PFN_DEV_GETHANDSHAKEBITSTATE          pfnDEV_GetHandshakeBitState;
  PFN_DEV_REMOVECHANNELFILES            pfnDEV_RemoveChannelFiles;
  PFN_DEV_DELETEFILE                    pfnDEV_DeleteFile;
  PFN_DEV_CHECKFORDOWNLOAD              pfnDEV_CheckForDownload;
  PFN_DEV_ISFWFILE                      pfnDEV_IsFWFile;
  PFN_DEV_ISNXFFILE                     pfnDEV_IsNXFFile;
  PFN_DEV_ISNXOFILE                     pfnDEV_IsNXOFile;
  PFN_DEV_GETFWTRANSFERTYPEFROMFILENAME pfnDEV_GetFWTransferTypeFromFileName;
  PFN_DEV_PROCESSFWDOWNLOAD             pfnDEV_ProcessFWDownload;
  PFN_DEV_DOWNLOADFILE                  pfnDEV_DownloadFile;
  PFN_DEV_UPLOADFILE                    pfnDEV_UploadFile;
#ifdef CIFX_TOOLKIT_DMA
  PFN_DEV_DMASTATE                      pfnDEV_DMAState;
  PFN_DEV_SETUPDMABUFFERS               pfnDEV_SetupDMABuffers;
#endif
} CIFX_DEV_FUNCTION_LIST_T, *PCIFX_DEV_FUNCTION_LIST_T;


/* Functions for retrieving the corresponding API function structure. */
PCIFX_API_FUNCTION_LIST_T  cifXTkitGetDpmApiFunctionList (void);
PCIFX_DEV_FUNCTION_LIST_T  cifXTkitGetDpmDevFunctionList (void);
PCIFX_TKIT_FUNCTION_LIST_T cifXTkitGetDpmTkitFunctionList(void);

PCIFX_API_FUNCTION_LIST_T  cifXTkitGetHifApiFunctionList (void);
PCIFX_DEV_FUNCTION_LIST_T  cifXTkitGetHifDevFunctionList (void);
PCIFX_TKIT_FUNCTION_LIST_T cifXTkitGetHifTkitFunctionList(void);

#endif /* CIFX_TOOLKIT_FUNCTION_LIST */

#ifdef __cplusplus
}
#endif

#endif /* CIFX_FUNCTION_LIST__H */
