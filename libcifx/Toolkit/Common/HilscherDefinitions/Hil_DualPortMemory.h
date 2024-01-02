/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_DualPortMemory.h $: *//*!

  \file Hil_DualPortMemory.h

  Hilscher definitions and structures for dual ported memory.

**************************************************************************************/
#ifndef HIL_DUALPORTMEMORY_H_
#define HIL_DUALPORTMEMORY_H_

#include <stdint.h>
#include "Hil_Compiler.h"
#include "Hil_SharedDefines.h"

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_PACK_1(HIL_DUALPORTMEMORY)
#endif


/*===========================================================================*/
/*                                                                           */
/* DEFAULT DPM structure                                                     */
/*                                                                           */
/*===========================================================================*/
/*                                                                           */
/*   -------------------------    DPM Offset 0                               */
/*  | System Channel          |                                              */
/*   -------------------------                                               */
/*  | Handshake channel       |                                              */
/*   -------------------------                                               */
/*  | Communication channel 0 |                                              */
/*   -------------------------                                               */
/*  | Communication channel 1 |                                              */
/*   -------------------------                                               */
/*  | Communication channel 2 |                                              */
/*   -------------------------                                               */
/*  | Communication channel 3 |                                              */
/*   -------------------------                                               */
/*  | Application channel   0 |                                              */
/*   -------------------------                                               */
/*  | Application channel   1 |                                              */
/*   -------------------------   DPM Offset xxxx                             */
/*===========================================================================*/
/*\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/* Global definitions */
#define HIL_DPM_MAX_SUPPORTED_CHANNELS     8               /*!< Maximum number of possible channels */

/* Global system channel definitions */
#define HIL_DPM_SYSTEM_HSK_CELL_OFFSET     0               /*!< Offset of the system handshake cells */
#define HIL_DPM_SYSTEM_BLOCK_VERSION       0               /*!< Version of the system block structure */
#define HIL_DPM_SYSTEM_CHANNEL_SIZE        512             /*!< Size of the system channel in bytes */
#define HIL_DPM_SYSTEM_MAILBOX_MIN_SIZE    124             /*!< Size of a system packet mailbox in bytes */

#define HIL_DPM_SYSTEM_CHANNEL_INDEX       0               /*!< Index of the system channel, always 0 */
#define HIL_DPM_HANDSHAKE_CHANNEL_INDEX    1               /*!< Index of the handshake channel, always 1 if available */
#define HIL_DPM_COM_CHANNEL_START_INDEX    2               /*!< Start index of communication channel 0  */

/* Global handshake channel size */
#define HIL_DPM_HANDSHAKE_CHANNEL_SIZE     256             /*!< Length of the handshake channel in bytes */
#define HIL_DPM_HANDSHAKE_PAIRS            16              /*!< Number of possible handshake pairs */

/* Global communication channel definitions */
#define HIL_DPM_STATUS_BLOCK_VERSION       2               /*!< version of the common status block structure */
#define HIL_DPM_EXT_STATE_SIZE             432             /*!< Default size of the extended state block */

#define HIL_DPM_CHANNEL_MAILBOX_SIZE       1596            /*!< Default size of a channel packet mailbox in bytes */
#define HIL_DPM_HP_IO_DATA_SIZE            64              /*!< Default size of the high prio I/O data */
#define HIL_DPM_IO_DATA_SIZE               5760            /*!< Default I/O data size in bytes */
#define HIL_DPM_IO_DATA_SIZE_8K_DPM        1536            /*!< I/O data size in bytes for hardware with 8KByte DPM */

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XX                                                           XXXXXXXXXXXXXX*/
/*XX SYSTEM CHANNEL LAYOUT                                     XXXXXXXXXXXXXX*/
/*XX                                                           XXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/

/*****************************************************************************/
/*! System channel information structure (Size always 16 Byte)               */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_SYSTEM_CHANNEL_INFO_Ttag
{
  uint8_t   bChannelType;                                        /*!< 0x00 Type of this channel */
  uint8_t   bReserved;                                           /*!< 0x01 reserved */
  uint8_t   bSizePositionOfHandshake;                            /*!< 0x02 Size and position of the handshake cells */
  uint8_t   bNumberOfBlocks;                                     /*!< 0x03 Number of blocks in this channel */
  uint32_t  ulSizeOfChannel;                                     /*!< 0x04 Size of channel in bytes */
  uint16_t  usSizeOfMailbox;                                     /*!< 0x08 Size of the send and receive mailbox */
  uint16_t  usMailboxStartOffset;                                /*!< 0x0A Start offset of the mailbox structure (see NETX_MAILBOX) */
  uint8_t   abReserved[4];                                       /*!< 0x0C:0x0F Reserved area */
} HIL_DPM_SYSTEM_CHANNEL_INFO_T;

/*****************************************************************************/
/*! Handshake channel information structure (Size always 16 Byte)            */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_HANDSHAKE_CHANNEL_INFO_Ttag
{
  uint8_t   bChannelType;                                        /*!< 0x00 Type of this channel */
  uint8_t   bReserved[3];                                        /*!< 0x01 reserved */
  uint32_t  ulSizeOfChannel;                                     /*!< 0x04 Size of channel in bytes */
  uint8_t   abReserved[8];                                       /*!< 0x08:0x0F Reserved area */
} HIL_DPM_HANDSHAKE_CHANNEL_INFO_T;

/*****************************************************************************/
/*! Communication channel information structure (Size always 16 Byte)        */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_COMMUNICATION_CHANNEL_INFO_Ttag
{
  uint8_t   bChannelType;                                        /*!< 0x00 Type of this channel */
  uint8_t   bChannelId;                                          /*!< 0x01 Channel / Port ID */
  uint8_t   bSizePositionOfHandshake;                            /*!< 0x02 Size and position of the handshake cells */
  uint8_t   bNumberOfBlocks;                                     /*!< 0x03 Number of blocks in this channel */
  uint32_t  ulSizeOfChannel;                                     /*!< 0x04 Size of channel in bytes */
  uint16_t  usCommunicationClass;                                /*!< 0x08 Communication Class (Master, Slave...) */
  uint16_t  usProtocolClass;                                     /*!< 0x0A Protocol Class (PROFIBUS, PROFINET....) */
  uint16_t  usProtocolConformanceClass;                          /*!< 0x0C Protocol Conformance Class (DPV1, DPV2...) */
  uint8_t   abReserved[2];                                       /*!< 0x0E:0x0F Reserved area */
} HIL_DPM_COMMUNICATION_CHANNEL_INFO_T;

/*****************************************************************************/
/*! Application channel information structure (Size always 16 Byte)          */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_APPLICATION_CHANNEL_INFO_Ttag
{
  uint8_t   bChannelType;                                        /*!< 0x00 Type of this channel */
  uint8_t   bChannelId;                                          /*!< 0x01 Channel / Port ID */
  uint8_t   bSizePositionOfHandshake;                            /*!< 0x02 Size and position of the handshake cells */
  uint8_t   bNumberOfBlocks;                                     /*!< 0x03 Number of blocks in this channel */
  uint32_t  ulSizeOfChannel;                                     /*!< 0x04 Size of channel in bytes */
  uint8_t   abReserved[8];                                       /*!< 0x0C:0x0F Reserved area */
} HIL_DPM_APPLICATION_CHANNEL_INFO_T;

/*****************************************************************************/
/*! System information block (Size = 48 Byte)                                */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_SYSTEM_INFO_BLOCK_Ttag
{
  uint8_t   abCookie[4];                                         /*!< 0x00 "netX" cookie */
  uint32_t  ulDpmTotalSize;                                      /*!< 0x04 Total Size of the whole dual-port memory in bytes */
  uint32_t  ulDeviceNumber;                                      /*!< 0x08 Device number */
  uint32_t  ulSerialNumber;                                      /*!< 0x0C Serial number */
  uint16_t  ausHwOptions[4];                                     /*!< 0x10 Hardware options, xC port 0..3 */
  uint16_t  usManufacturer;                                      /*!< 0x18 Manufacturer Location */
  uint16_t  usProductionDate;                                    /*!< 0x1A Date of production */
  uint32_t  ulLicenseFlags1;                                     /*!< 0x1C License code flags 1 */
  uint32_t  ulLicenseFlags2;                                     /*!< 0x20 License code flags 2 */
  uint16_t  usNetxLicenseID;                                     /*!< 0x24 netX license identification */
  uint16_t  usNetxLicenseFlags;                                  /*!< 0x26 netX license flags */
  uint16_t  usDeviceClass;                                       /*!< 0x28 netX device class */
  uint8_t   bHwRevision;                                         /*!< 0x2A Hardware revision index */
  uint8_t   bHwCompatibility;                                    /*!< 0x2B Hardware compatibility index */
  uint8_t   bDevIdNumber;                                        /*!< 0x2C Device identification number (rotary switch) */
  uint8_t   bReserved;                                           /*!< 0x2D Reserved byte */
  uint16_t  usReserved;                                          /*!< 0x2E:0x2F Reserved */
} HIL_DPM_SYSTEM_INFO_BLOCK_T;

/*****************************************************************************/
/*! Channel information block (Size always 16 Byte)                          */
/*****************************************************************************/
typedef __HIL_PACKED_PRE union __HIL_PACKED_POST HIL_DPM_CHANNEL_INFO_BLOCK_Ttag
{
  HIL_DPM_SYSTEM_CHANNEL_INFO_T        tSystem;
  HIL_DPM_HANDSHAKE_CHANNEL_INFO_T     tHandshake;
  HIL_DPM_COMMUNICATION_CHANNEL_INFO_T tCom;
  HIL_DPM_APPLICATION_CHANNEL_INFO_T   tApp;
} HIL_DPM_CHANNEL_INFO_BLOCK_T;

/*****************************************************************************/
/*! System information block (Size = 8 Byte)                                 */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_SYSTEM_CONTROL_BLOCK_Ttag
{
  uint32_t  ulSystemCommandCOS;                                  /*!< 0x00 System channel change of state command */
  uint32_t  ulSystemControl;                                     /*!< 0x04 System channel control (only for eCos on netX90/netX4000) */
} HIL_DPM_SYSTEM_CONTROL_BLOCK_T;

/*****************************************************************************/
/*! System status block (Size = 64 Byte)                                     */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_SYSTEM_STATUS_BLOCK_Ttag
{
  uint32_t  ulSystemCOS;                                         /*!< 0x00 System channel change of state acknowledge */
  uint32_t  ulSystemStatus;                                      /*!< 0x04 Actual system state */
  uint32_t  ulSystemError;                                       /*!< 0x08 Actual system error */
  uint32_t  ulBootError;                                         /*!< 0x0C Bootup error (only set by 2nd Stage Bootloader) */
  uint32_t  ulTimeSinceStart;                                    /*!< 0x10 time since start in seconds */
  uint16_t  usCpuLoad;                                           /*!< 0x14 cpu load in 0,01% units (10000 => 100%) */
  uint16_t  usReserved;                                          /*!< 0x16 Reserved */
  uint32_t  ulHWFeatures;                                        /*!< 0x18 Hardware features */
  uint8_t   abReserved[36];                                      /*!< 0x1C:3F Reserved */
} HIL_DPM_SYSTEM_STATUS_BLOCK_T;

/*****************************************************************************/
/*! System send packet mailbox (Size 128 Byte)                               */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_SYSTEM_SEND_MAILBOX_Ttag
{
  uint16_t  usPackagesAccepted;                                 /*!< Number of packages that can be accepted */
  uint16_t  usReserved;                                         /*!< Reserved */
  uint8_t   abSendMbx[HIL_DPM_SYSTEM_MAILBOX_MIN_SIZE];            /*!< Send mailbox packet buffer */
} HIL_DPM_SYSTEM_SEND_MAILBOX_T;

/*****************************************************************************/
/*! System receive packet mailbox (Size 128 Byte)                            */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_SYSTEM_RECV_MAILBOX_Ttag
{
  uint16_t  usWaitingPackages;                                  /*!< Number of packages waiting to be processed */
  uint16_t  usReserved;                                         /*!< Reserved */
  uint8_t   abRecvMbx[HIL_DPM_SYSTEM_MAILBOX_MIN_SIZE];            /*!< Receive mailbox packet buffer */
} HIL_DPM_SYSTEM_RECV_MAILBOX_T;

/*****************************************************************************/
/*! Handshake cell definition                                                */
/*****************************************************************************/
typedef __HIL_PACKED_PRE union __HIL_PACKED_POST HIL_DPM_HANDSHAKE_CELL_Ttag
{
  __HIL_PACKED_PRE struct __HIL_PACKED_POST
  {
    volatile uint8_t abData[2];        /*!< Data value, not belonging to handshake */
    volatile uint8_t bNetxFlags;       /*!< Device status flags (8Bit Mode) */
    volatile uint8_t bHostFlags;       /*!< Device command flags (8Bit Mode) */
  } t8Bit;

  __HIL_PACKED_PRE struct __HIL_PACKED_POST
  {
    volatile uint16_t usNetxFlags;     /*!< Device status flags (16Bit Mode) */
    volatile uint16_t usHostFlags;     /*!< Device command flags (16Bit Mode)*/
  } t16Bit;
  volatile uint32_t ulValue;            /*!< Handshake cell value */
} HIL_DPM_HANDSHAKE_CELL_T;

/*****************************************************************************/
/*! Structure of the whole system channel (DPM) (Size 512 Byte)              */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_SYSTEM_CHANNEL_Ttag
{
  HIL_DPM_SYSTEM_INFO_BLOCK_T      tSystemInfo;                                    /*!< 0x000:0x02F System information block */
  HIL_DPM_CHANNEL_INFO_BLOCK_T     atChannelInfo[HIL_DPM_MAX_SUPPORTED_CHANNELS];  /*!< 0x030:0x0AF Channel information block */
  HIL_DPM_HANDSHAKE_CELL_T         tSysHandshake;                                  /*!< 0x0B0:0x0B3 Handshake cells used, if not in Handshake block */
  uint8_t                          abReserved[4];                                  /*!< 0x0B4:0x0B7 unused/reserved */
  HIL_DPM_SYSTEM_CONTROL_BLOCK_T   tSystemControl;                                 /*!< 0x0B8:0x0BF System control block */
  HIL_DPM_SYSTEM_STATUS_BLOCK_T    tSystemState;                                   /*!< 0x0C0:0x0FF System state block */
  HIL_DPM_SYSTEM_SEND_MAILBOX_T    tSystemSendMailbox;                             /*!< 0x100:0x17F Send mailbox */
  HIL_DPM_SYSTEM_RECV_MAILBOX_T    tSystemRecvMailbox;                             /*!< 0x180:0x1FF Receive mailbox */
} HIL_DPM_SYSTEM_CHANNEL_T;

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XX                                                           XXXXXXXXXXXXXX*/
/*XX HANDSHAKE CHANNEL LAYOUT                                  XXXXXXXXXXXXXX*/
/*XX                                                           XXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/

/*****************************************************************************/
/*! Handshake array definition                                               */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_HANDSHAKE_ARRAY_Ttag
{
  HIL_DPM_HANDSHAKE_CELL_T atHsk[HIL_DPM_HANDSHAKE_PAIRS]; /*!< Handshake cell block definition */
} HIL_DPM_HANDSHAKE_ARRAY_T;

/*****************************************************************************/
/*! Handshake channel definition                                             */
/*****************************************************************************/
typedef struct HIL_DPM_HANDSHAKE_CHANNEL_Ttag
{
  HIL_DPM_HANDSHAKE_CELL_T tSysFlags;                                                  /*!< 0x00 System handshake flags */
  HIL_DPM_HANDSHAKE_CELL_T tHskFlags;                                                  /*!< 0x04 not used */
  HIL_DPM_HANDSHAKE_CELL_T tCommFlags0;                                                /*!< 0x08 channel 0 handshake flags */
  HIL_DPM_HANDSHAKE_CELL_T tCommFlags1;                                                /*!< 0x0C channel 1 handshake flags */
  HIL_DPM_HANDSHAKE_CELL_T tCommFlags2;                                                /*!< 0x10 channel 2 handshake flags */
  HIL_DPM_HANDSHAKE_CELL_T tCommFlags3;                                                /*!< 0x14 channel 3 handshake flags */
  HIL_DPM_HANDSHAKE_CELL_T tAppFlags0;                                                 /*!< 0x18 not supported yet         */
  HIL_DPM_HANDSHAKE_CELL_T tAppFlags1;                                                 /*!< 0x1C not supported yet         */
  uint32_t                 aulReserved[ 56 ];                                          /*!< 0x20 - 0xFF */
} HIL_DPM_HANDSHAKE_CHANNEL_T;

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*XX                                                           XXXXXXXXXXXXXX*/
/*XX COMMUNICATION CHANNEL LAYOUT                              XXXXXXXXXXXXXX*/
/*XX                                                           XXXXXXXXXXXXXX*/
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/

/*****************************************************************************/
/*! Default master status structure                                          */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_MASTER_STATUS_Ttag
{
  uint32_t ulSlaveState;         /*!< Slave status */
  uint32_t ulSlaveErrLogInd;     /*!< Slave error indication */
  uint32_t ulNumOfConfigSlaves;  /*!< Number of configured slaves */
  uint32_t ulNumOfActiveSlaves;  /*!< Number of active slaves */
  uint32_t ulNumOfDiagSlaves;    /*!< Number of slaves in diag mode */
  uint32_t ulReserved;           /*!< unused/reserved */
} HIL_DPM_MASTER_STATUS_T;

/* Master Status definitions */
#define HIL_SLAVE_STATE_UNDEFINED                           0x00000000
#define HIL_SLAVE_STATE_OK                                  0x00000001
#define HIL_SLAVE_STATE_FAILED                              0x00000002

/*****************************************************************************/
/*! Channel handshake block (Size always 8 Byte)                             */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_HANDSHAKE_BLOCK_Ttag
{
  uint8_t  abReserved[8];                                       /*!< unused/reserved */
} HIL_DPM_HANDSHAKE_BLOCK_T;

/*****************************************************************************/
/*! Channel control block (Size 8 Byte)                                      */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_CONTROL_BLOCK_Ttag
{
  uint32_t   ulApplicationCOS;                                   /*!< 0x00 Application "Change Of State" flags */
  uint32_t   ulDeviceWatchdog;                                   /*!< 0x04 Watchdog cell written by the Host */
} HIL_DPM_CONTROL_BLOCK_T;

/*****************************************************************************/
/*! Channel common status block (Size 64 Byte)                               */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_COMMON_STATUS_BLOCK_Ttag
{
  uint32_t  ulCommunicationCOS;                                 /*!< 0x00 Communication channel "Change Of State" */
  uint32_t  ulCommunicationState;                               /*!< 0x04 Actual communication state */
  uint32_t  ulCommunicationError;                               /*!< 0x08 Actual communication error */
  uint16_t  usVersion;                                          /*!< 0x0C Version of the diagnostic structure */
  uint16_t  usWatchdogTime;                                     /*!< 0x0E Configured watchdog time */
  uint8_t   bPDInHskMode;                                       /*!< 0x10 Input area handshake mode. */
  uint8_t   bPDInSource;                                        /*!< 0x11 Reserved. Set to zero.*/
  uint8_t   bPDOutHskMode;                                      /*!< 0x12 Output area handshake mode. */
  uint8_t   bPDOutSource;                                       /*!< 0x13 Reserved. Set to zero.*/
  uint32_t  ulHostWatchdog;                                     /*!< 0x14 Host watchdog */
  uint32_t  ulErrorCount;                                       /*!< 0x18 Number of erros since power-up */
  uint8_t   bErrorLogInd;                                       /*!< 0x1C Counter of available Log Entries (not supported yet) */
  uint8_t   bErrorPDInCnt;                                      /*!< 0x1D Counter of input process data handshake handling errors  */
  uint8_t   bErrorPDOutCnt;                                     /*!< 0x1E Counter of output process data handshake handling errors */
  uint8_t   bErrorSyncCnt;                                      /*!< 0x1F Counter of synchronization handshake handling errors */
  uint8_t   bSyncHskMode;                                       /*!< 0x20 Synchronization Handshake mode. */
  uint8_t   bSyncSource;                                        /*!< 0x21 Synchronization source. */
  uint16_t  ausReserved[3];                                     /*!< 0x22 Reserved */
  __HIL_PACKED_PRE union __HIL_PACKED_POST
  {
    HIL_DPM_MASTER_STATUS_T                 tMasterStatusBlock;   /*!< For master implementations */
    uint32_t                                aulReserved[6];       /*!< reserved */
  } uStackDepended;                             /*!< 0x28 Stack depend status block */
} HIL_DPM_COMMON_STATUS_BLOCK_T;

/*****************************************************************************/
/*! Channel extended status block (Size 432 Byte)                            */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_EXTENDED_STATUS_BLOCK_Ttag
{
  uint8_t abExtendedStatus[HIL_DPM_EXT_STATE_SIZE];                /*!< 0x00 Extended status buffer */
} HIL_DPM_EXTENDED_STATUS_BLOCK_T;

/* Description of the extended status block structure */
#define HIL_EXT_STS_MAX_STRUCTURES           24
/* Location definiton of the ext. state information (bStateArea) */
#define HIL_EXT_STS_STD_OUTPUT_BLK_ID        8                            /*!< State field located in standard output area */
#define HIL_EXT_STS_HI_OUTPUT_BLK_ID         9                            /*!< State field located in high prio. output area */
#define HIL_EXT_STS_STD_INPUT_BLK_ID         0                            /*!< State field located in standard input area */
#define HIL_EXT_STS_HI_INPUT_BLK_ID          1                            /*!< State field located in high prio. input area */
/* Definition of state information (bStateTypeID) */
#define HIL_EXT_STS_SLAVE_CONFIGURED         (1)                          /*!< Configured slave bit field */
#define HIL_EXT_STS_SLAVE_ACTIV              (2)                          /*!< Activ slave bit field      */
#define HIL_EXT_STS_SLAVE_DIAGNOSTIC         (3)                          /*!< Slave diagnostic bit field */
#define HIL_EXT_STS_COMMANDS                 (4)                          /*!< Command table bit field    */
#define HIL_EXT_STS_IO_CHANGED               (5)                          /*!< CoS bit field              */
#define HIL_EXT_STS_IOPS_BYTEWISE            (6)                          /*!< IO provider state          */
#define HIL_EXT_STS_IOPS_BITWISE             (7)                          /*!< IO provider state          */
#define HIL_EXT_STS_IOCS_BYTEWISE            (8)                          /*!< IO consumer state          */
#define HIL_EXT_STS_IOCS_BITWISE             (9)                          /*!< IO consumer state          */
#define HIL_EXT_STS_EIP_ASSEMBLY_STATE      (10)                          /*!< Assembly bitfield          */
#define HIL_EXT_STS_COM_PDO_COUNTER         (11)                          /*!< CANopen PDO counter        */

#define HIL_EXT_STS_NAME_OUTPUT        "STD_OUTPUT"
#define HIL_EXT_STS_NAME_INPUT         "STD_INPUT"

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_EXTENDED_STATE_STRUCT_Ttag
{
  uint8_t                 bStateArea;                               /*!< Location of the ext. state information */
  uint8_t                 bStateTypeID;                             /*!< Type of the state information */
  uint16_t                usNumOfStateEntries;                      /*!< Number of state entries of the type bStateTypeID */
  uint32_t                ulStateOffset;                            /*!< Byte start offset in the defined I/O area*/
} HIL_DPM_EXTENDED_STATE_STRUCT_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_EXTENDED_STATE_FIELD_Ttag
{
  uint8_t                          bReserved[3];                             /*!< 3 Bytes preserved. Not used.  */
  uint8_t                          bNumStateStructs;                         /*!< Number of following state structures */
  HIL_DPM_EXTENDED_STATE_STRUCT_T  atStateStruct[HIL_EXT_STS_MAX_STRUCTURES];
} HIL_DPM_EXTENDED_STATE_FIELD_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_EXTENDED_STATE_FIELD_DEFINITION_Ttag
{
  uint8_t                        abReserved[172];                            /*!< Default, protocol specific information area */
  HIL_DPM_EXTENDED_STATE_FIELD_T tExtStateField;                             /*!< Extended status structures */
} HIL_DPM_EXTENDED_STATE_FIELD_DEFINITION_T;
/*****************************************************************************/
/*! Channel send packet mailbox block (Size 1600 Byte)                       */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_SEND_MAILBOX_BLOCK_Ttag
{
  uint16_t  usPackagesAccepted;                                 /*!< 0x00 Number of packages that can be accepted */
  uint16_t  usReserved;                                         /*!< 0x02 Reserved */
  uint8_t   abSendMailbox[HIL_DPM_CHANNEL_MAILBOX_SIZE];        /*!< 0x04 Send mailbox packet buffer */
} HIL_DPM_SEND_MAILBOX_BLOCK_T;

/*****************************************************************************/
/*! Channel receive packet mailbox block (Size 1600 Byte)                    */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_RECV_MAILBOX_BLOCK_Ttag
{
  uint16_t  usWaitingPackages;                                  /*!< 0x00 Number of packages waiting to be processed */
  uint16_t  usReserved;                                         /*!< 0x02 Reserved */
  uint8_t   abRecvMailbox[HIL_DPM_CHANNEL_MAILBOX_SIZE];        /*!< 0x04 Receive mailbox packet buffer */
} HIL_DPM_RECV_MAILBOX_BLOCK_T;

/*****************************************************************************/
/*! Structure of the DEFAULT communication channel (Size 15616 Byte)         */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_DEFAULT_COMM_CHANNEL_Ttag
{
  HIL_DPM_HANDSHAKE_BLOCK_T        tReserved;                                 /*!< 0x000:0x007 Reserved for later use */
  HIL_DPM_CONTROL_BLOCK_T          tControl;                                  /*!< 0x008:0x00F Control block */
  HIL_DPM_COMMON_STATUS_BLOCK_T    tCommonStatus;                             /*!< 0x010:0x04F Common status block */
  HIL_DPM_EXTENDED_STATUS_BLOCK_T  tExtendedStatus;                           /*!< 0x050:0x1FF Extended status block */
  HIL_DPM_SEND_MAILBOX_BLOCK_T     tSendMbx;                                  /*!< 0x200:0x83F Send mailbox block */
  HIL_DPM_RECV_MAILBOX_BLOCK_T     tRecvMbx;                                  /*!< 0x840:0xE7F Recveice mailbox block */
  uint8_t                          abPd1Output[HIL_DPM_HP_IO_DATA_SIZE];      /*!< 0xE80:0xEBF Process data 1 output area */
  uint8_t                          abPd1Input[HIL_DPM_HP_IO_DATA_SIZE];       /*!< 0xEC0:0xEFF Process data 1 input area */
  uint8_t                          abReserved1[256];                          /*!< 0xF00:0xFFF Reserved */
  uint8_t                          abPd0Output[HIL_DPM_IO_DATA_SIZE];         /*!< 0x1000:0x267F Process data 0 output area */
  uint8_t                          abPd0Input[HIL_DPM_IO_DATA_SIZE];          /*!< 0x2680:0x3CFF Process data 0 input area */
} HIL_DPM_DEFAULT_COMM_CHANNEL_T;


/*****************************************************************************/
/*! Structure of the communication channel for 8K DPM hardware (e.g. COMX10) */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_8K_DPM_COMM_CHANNEL_Ttag
{
  HIL_DPM_HANDSHAKE_BLOCK_T         tReserved;                                    /*!< 0x000:0x007 Reserved for later use */
  HIL_DPM_CONTROL_BLOCK_T           tControl;                                     /*!< 0x008:0x00F Control block */
  HIL_DPM_COMMON_STATUS_BLOCK_T     tCommonStatus;                                /*!< 0x010:0x04F Common status block */
  HIL_DPM_EXTENDED_STATUS_BLOCK_T   tExtendedStatus;                              /*!< 0x050:0x1FF Extended status block */
  HIL_DPM_SEND_MAILBOX_BLOCK_T      tSendMbx;                                     /*!< 0x200:0x83F Send mailbox block */
  HIL_DPM_RECV_MAILBOX_BLOCK_T      tRecvMbx;                                     /*!< 0x840:0xE7F Recveice mailbox block */
  uint8_t                           abPd1Output[HIL_DPM_HP_IO_DATA_SIZE];         /*!< 0xE80:0xEBF Process data 1 output area */
  uint8_t                           abPd1Input[HIL_DPM_HP_IO_DATA_SIZE];          /*!< 0xEC0:0xEFF Process data 1 input area */
  uint8_t                           abReserved1[256];                             /*!< 0xF00:0xFFF Reserved */
  uint8_t                           abPd0Output[HIL_DPM_IO_DATA_SIZE_8K_DPM];     /*!< 0x1000:0x15FF Process data 0 output area */
  uint8_t                           abPd0Input[HIL_DPM_IO_DATA_SIZE_8K_DPM];      /*!< 0x1600:0x1BFF Process data 0 input area */
  uint8_t                           abReserved2[256];                             /*!< 0x1C00:0x1CFF Reserved */
} HIL_DPM_8K_DPM_COMM_CHANNEL_T;


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */
/* End of DPM Layout definition */

/*===========================================================================*/
/*                                                                           */
/* Standardized Handshake Flags                                              */
/*                                                                           */
/*===========================================================================*/

/* --------------------------------------------*/
/* System Channel Handshake Flags              */
/* --------------------------------------------*/
/* HOST Flags */
#define HSF_RESET                         0x0001                      /*!< Reset command bitmask */
#define HSF_BOOTSTART                     0x0002                      /*!< Set when device has a second stage loader, to enter bootloader mode after a system start */
#define HSF_HOST_COS_CMD                  0x0004                      /*!< Host "Change Of State" command bitmask */
#define HSF_NETX_COS_ACK                  0x0008                      /*!< NetX "Change Of State" acknowlegde bitmask */
#define HSF_SEND_MBX_CMD                  0x0010                      /*!< Send mailbox command bitmask */
#define HSF_RECV_MBX_ACK                  0x0020                      /*!< Receive mailbox acknowledge bitmask */
#define HSF_EXT_SEND_MBX_CMD              0x0040                      /*!< Second stage loader extended mailbox command bitmask */
#define HSF_EXT_RECV_MBX_ACK              0x0080                      /*!< Second stage loader extended mailbox ack bitmask */

/* HOST Flags as Bit number */
#define HSF_RESET_BIT_NO                  0                           /*!< Reset command bitnumber */
#define HSF_BOOTLOADER_BIT_NO             1                           /*!< Bitnumber to be set when device has a second stage loader, to enter bootloader mode after a system start */
#define HSF_HOST_COS_CMD_BIT_NO           2                           /*!< Host "Change Of State" command bitnumber */
#define HSF_NETX_COS_ACK_BIT_NO           3                           /*!< NetX "Change Of State" acknowlegde bitnumber */
#define HSF_SEND_MBX_CMD_BIT_NO           4                           /*!< Send mailbox command bitnumber */
#define HSF_RECV_MBX_ACK_BIT_NO           5                           /*!< Receive mailbox acknowledge bitnumber */
#define HSF_EXT_SEND_MBX_CMD_BIT_NO       6                           /*!< Second stage loader extended mailbox command bitnumber */
#define HSF_EXT_RECV_MBX_ACK_BIT_NO       7                           /*!< Second stage loader extended mailbox ack bitnumber */


/* netX Flags */
#define NSF_READY                         0x0001                      /*!< netX System READY bitmask */
#define NSF_ERROR                         0x0002                      /*!< General system error bitmask */
#define NSF_HOST_COS_ACK                  0x0004                      /*!< Host "Change Of State" acknowledge bitmask */
#define NSF_NETX_COS_CMD                  0x0008                      /*!< NetX "Change of State command bitmask */
#define NSF_SEND_MBX_ACK                  0x0010                      /*!< Send mailbox acknowledge bitmask */
#define NSF_RECV_MBX_CMD                  0x0020                      /*!< Receive mailbox command bitmask */
#define NSF_EXT_SEND_MBX_ACK              0x0040                      /*!< Second stage loader extended mailbox ack bitmask */
#define NSF_EXT_RECV_MBX_CMD              0x0080                      /*!< Second stage loader extended mailbox command bitmask */
/* netX Flags as Bit number */
#define NSF_READY_BIT_NO                  0                           /*!< netX System READY bitnumber */
#define NSF_ERROR_BIT_NO                  1                           /*!< General system error bitnumber */
#define NSF_HOST_COS_ACK_BIT_NO           2                           /*!< Host "Change Of State" acknowledge bitnumber */
#define NSF_NETX_COS_CMD_BIT_NO           3                           /*!< NetX "Change of State command bitnumber */
#define NSF_SEND_MBX_ACK_BIT_NO           4                           /*!< Send mailbox acknowledge bitnumber */
#define NSF_RECV_MBX_CMD_BIT_NO           5                           /*!< Receive mailbox command bitnumber */
#define NSF_EXT_SEND_MBX_ACK_BIT_NO       6                           /*!< Second stage loader extended mailbox ack bitnumber */
#define NSF_EXT_RECV_MBX_CMD_BIT_NO       7                           /*!< Second stage loader extended mailbox command bitnumber */

/*--------------------------------------------*/
/* Communication Channel Handshake Flags      */
/*--------------------------------------------*/
/* HOST Communication Channel Flags */
#define HCF_HOST_READY                    0x0001                      /*!< Host application is Ready bitmask */
#define HCF_unused                        0x0002                      /*!< unused */
#define HCF_HOST_COS_CMD                  0x0004                      /*!< Host "Change Of State" command bitmask */
#define HCF_NETX_COS_ACK                  0x0008                      /*!< NetX "Change Of State" acknowledge bitmask */
#define HCF_SEND_MBX_CMD                  0x0010                      /*!< Send mailbox command bitmask */
#define HCF_RECV_MBX_ACK                  0x0020                      /*!< Receive mailbox ackowledge bitmask */
#define HCF_PD0_OUT_CMD                   0x0040                      /*!< Process data, block 0, output command bitmask */
#define HCF_PD0_IN_ACK                    0x0080                      /*!< Process data, block 0, input acknowlegde bitmask */
#define HCF_PD1_OUT_CMD                   0x0100                      /*!< Process data, block 1, output command bitmask */
#define HCF_PD1_IN_ACK                    0x0200                      /*!< Process data, block 1, input acknowlegde bitmask */
/* HOST Communication Channel Flags as Bit number */
#define HCF_HOST_READY_BIT_NO             0                           /*!< Host application is Ready bitnumber */
#define HCF_unused_BIT_NO                 1                           /*!< unused */
#define HCF_HOST_COS_CMD_BIT_NO           2                           /*!< Host "Change Of State" command bitnumber */
#define HCF_NETX_COS_ACK_BIT_NO           3                           /*!< NetX "Change Of State" acknowledge bitnumber */
#define HCF_SEND_MBX_CMD_BIT_NO           4                           /*!< Send mailbox command bitnumber */
#define HCF_RECV_MBX_ACK_BIT_NO           5                           /*!< Receive mailbox ackowledge bitnumber */
#define HCF_PD0_OUT_CMD_BIT_NO            6                           /*!< Process data, block 0, output command bitnumber */
#define HCF_PD0_IN_ACK_BIT_NO             7                           /*!< Process data, block 0, input acknowlegde bitnumber */
#define HCF_PD1_OUT_CMD_BIT_NO            8                           /*!< Process data, block 1, output command bitnumber */
#define HCF_PD1_IN_ACK_BIT_NO             9                           /*!< Process data, block 1, input acknowlegde bitnumber */

/* netX Communication Channel Flags */
#define NCF_COMMUNICATING                 0x0001                      /*!< Channel has an active conection bitmask */
#define NCF_ERROR                         0x0002                      /*!< Communication channel error bitmask */
#define NCF_HOST_COS_ACK                  0x0004                      /*!< Host "Change Of State" acknowledge bitmask */
#define NCF_NETX_COS_CMD                  0x0008                      /*!< NetX "Change Of State" command bitmask */
#define NCF_SEND_MBX_ACK                  0x0010                      /*!< Send mailbox acknowldege bitmask */
#define NCF_RECV_MBX_CMD                  0x0020                      /*!< Receive mailbox command bitmask */
#define NCF_PD0_OUT_ACK                   0x0040                      /*!< Process data, block 0, output acknowledge bitmask */
#define NCF_PD0_IN_CMD                    0x0080                      /*!< Process data, block 0, input command bitmask */
#define NCF_PD1_OUT_ACK                   0x0100                      /*!< Process data, block 1, output acknowlegde bitmask */
#define NCF_PD1_IN_CMD                    0x0200                      /*!< Process data, block 1, input command bitmask */
/* netX Communication Channel Flags as Bit number */
#define NCF_COMMUNICATING_BIT_NO          0                           /*!< Channel has an active conection bitnumber */
#define NCF_ERROR_BIT_NO                  1                           /*!< Communication channel error bitnumber */
#define NCF_HOST_COS_ACK_BIT_NO           2                           /*!< Host "Change Of State" acknowledge bitnumber */
#define NCF_NETX_COS_CMD_BIT_NO           3                           /*!< NetX "Change Of State" command bitnumber */
#define NCF_SEND_MBX_ACK_BIT_NO           4                           /*!< Send mailbox acknowldege bitnumber */
#define NCF_RECV_MBX_CMD_BIT_NO           5                           /*!< Receive mailbox command bitnumber */
#define NCF_PD0_OUT_ACK_BIT_NO            6                           /*!< Process data, block 0, output acknowledge bitnumber */
#define NCF_PD0_IN_CMD_BIT_NO             7                           /*!< Process data, block 0, input command bitnumber */
#define NCF_PD1_OUT_ACK_BIT_NO            8                           /*!< Process data, block 1, output acknowlegde bitnumber */
#define NCF_PD1_IN_CMD_BIT_NO             9                           /*!< Process data, block 1, input command bitnumber */

/*--------------------------------------------*/
/* Handshake Flags State Definitions          */
/*--------------------------------------------*/
/* Flag state definition */
#define HIL_FLAGS_EQUAL                   0
#define HIL_FLAGS_NOT_EQUAL               1
#define HIL_FLAGS_CLEAR                   2
#define HIL_FLAGS_SET                     3
#define HIL_FLAGS_NONE                    0xFF

#define HIL_FLAG_CLEAR                    0
#define HIL_FLAG_SET                      1

/*===========================================================================*/
/*                                                                           */
/* SYSTEM CHANNEL Configuration definitions                                  */
/*                                                                           */
/*===========================================================================*/

/*--------------------------------------------*/
/* SYSTEM CONTROL BLOCK                       */
/*--------------------------------------------*/
/* ulSystemCommandCOS flags */
#define HIL_SYS_RESET_COOKIE                                0x55AA55AA  /*!< System Reset cookie */

/* ulSystemControl flags and its functionality only exist for netX90/netX4000 */
/* ATTENTION:                                                                 */
/* The layout must correspond to ulResetMode defined in the hilscher packet   */
/* command HIL_FIRMWARE_RESET_REQ, because of the evaluation function         */
/* called by the DPM and Packet based functions.                              */

/* Hilscher reset mode definitions */
#define HIL_SYS_CONTROL_RESET_MODE_MASK                     0x0000000F
#define HIL_SYS_CONTROL_RESET_MODE_SRT                      0

#define HIL_SYS_CONTROL_RESET_MODE_COLDSTART                0
#define HIL_SYS_CONTROL_RESET_MODE_WARMSTART                1
#define HIL_SYS_CONTROL_RESET_MODE_BOOTSTART                2
#define HIL_SYS_CONTROL_RESET_MODE_UPDATESTART              3
#define HIL_SYS_CONTROL_RESET_MODE_CONSOLESTART             4           /* not useable via DPM */

/* Hilscher reset parameter definitions */
#define HIL_SYS_CONTROL_RESET_PARAM_MASK                    0x000000F0
#define HIL_SYS_CONTROL_RESET_PARAM_SRT                     4

/* Hilscher additional reset flag definitions */
#define HIL_SYS_CONTROL_RESET_FLAG_MASK                     0xFFFFFF00
#define HIL_SYS_CONTROL_RESET_FLAG_SRT                      8

/* Clear REMANENT flag */
#define HIL_SYS_CONTROL_RESET_FLAG_CLEAR_REMANENT_MASK      0x00000100
#define HIL_SYS_CONTROL_RESET_FLAG_CLEAR_REMANENT_SRT       8

/* Install VERIFICATION flag */
#define HIL_SYS_CONTROL_RESET_FLAG_VERIFY_INSTALL_MASK      0x00000200
#define HIL_SYS_CONTROL_RESET_FLAG_VERIFY_INSTALL_SRT       9

/* Hilscher reset parameter and flag definiton mask */
#define HIL_SYS_CONTROL_RESET_PARAM_FLAG_MASK               (HIL_SYS_CONTROL_RESET_FLAG_MASK | HIL_SYS_CONTROL_RESET_PARAM_MASK)

/*--------------------------------------------*/
/* SYSTEM STATUS BLOCK                        */
/*--------------------------------------------*/
/* System Change of State flags */
#define HIL_SYS_COS_UNDEFINED                               0
#define HIL_SYS_COS_DEFAULT_MEMORY                          0x80000000
/* System Change of State flags as bit number */
#define HIL_SYS_COS_DEFAULT_MEMORY_BIT_NO                   31

/* System Status */
#define HIL_SYS_STATUS_UNDEFINED                            0x00000000
#define HIL_SYS_STATUS_OK                                   0x00000001
#define HIL_SYS_STATUS_IDPM                                 0x00400000
#define HIL_SYS_STATUS_APP                                  0x00800000
#define HIL_SYS_STATUS_BOOTMEDIUM_MASK                      0x0F000000
#define HIL_SYS_STATUS_BOOTMEDIUM_RAM                       0x00000000
#define HIL_SYS_STATUS_BOOTMEDIUM_SERFLASH                  0x01000000
#define HIL_SYS_STATUS_BOOTMEDIUM_PARFLASH                  0x02000000
#define HIL_SYS_STATUS_NO_SYSVOLUME                         0x20000000
#define HIL_SYS_STATUS_SYSVOLUME_FFS                        0x40000000  /*!< _FFS = Flash File System */
#define HIL_SYS_STATUS_NXO_SUPPORTED                        0x80000000

/* System Status definition */
#define HIL_SYS_STATE_UNDEFINED                             0
#define HIL_SYS_STATE_OK                                    1

/* System Error definitions */
#define HIL_SYS_ERROR_SUCCESS                               0

/* System Status */
#define HIL_SYS_STATE_RESET                                 0x000000F0
#define HIL_SYS_STATE_SELF_TEST                             0x000000EF
#define HIL_SYS_STATE_RAM_TEST                              0x000000EE
#define HIL_SYS_STATE_FAULT_INIT                            0x000000ED
#define HIL_SYS_STATE_DEVICE_INIT                           0x000000EC
#define HIL_SYS_STATE_MAILBOX_INIT                          0x000000EB
#define HIL_SYS_STATE_SERIAL_INIT                           0x000000EA
#define HIL_SYS_STATE_SEMAPHORE_INIT                        0x000000E9
#define HIL_SYS_STATE_QUEUE_INIT                            0x000000E8
#define HIL_SYS_STATE_MUTEX_INIT                            0x000000E7
#define HIL_SYS_STATE_EVENT_INIT                            0x000000E6
#define HIL_SYS_STATE_SIGNAL_INIT                           0x000000E5
#define HIL_SYS_STATE_TIMER_INIT                            0x000000E4
#define HIL_SYS_STATE_BARRIER_INIT                          0x000000E3
#define HIL_SYS_STATE_DIAGNOSTIC_INIT                       0x000000E2
#define HIL_SYS_STATE_FINITE_STATE_INIT                     0x000000E1
#define HIL_SYS_STATE_INTERRUPT_INIT                        0x000000E0
#define HIL_SYS_STATE_LED_INIT                              0x000000DF
/*#define HIL_SYS_STATE_TIMER_INIT                          0x000000DE*/
#define HIL_SYS_STATE_PAR_FLASH_INIT                        0x000000DD
#define HIL_SYS_STATE_XC_INIT                               0x000000DC
#define HIL_SYS_STATE_PHY_INIT                              0x000000DB
#define HIL_SYS_STATE_UART_INIT                             0x000000DA
#define HIL_SYS_STATE_VOL_INIT                              0x000000D9
#define HIL_SYS_STATE_EDD_INIT                              0x000000D8
#define HIL_SYS_STATE_ICM_INIT                              0x000000D7
#define HIL_SYS_STATE_USB_INIT                              0x000000D6
#define HIL_SYS_STATE_FIFO_INIT                             0x000000D5
#define HIL_SYS_STATE_EBUS_INIT                             0x000000D4
#define HIL_SYS_STATE_MMU_INIT                              0x000000D3
#define HIL_SYS_STATE_TCM_INIT                              0x000000D2
#define HIL_SYS_STATE_CCH_INIT                              0x000000D1
#define HIL_SYS_STATE_MID_SYS_INIT                          0x000000D0
#define HIL_SYS_STATE_MID_DBM_INIT                          0x000000CF
#define HIL_SYS_STATE_HIF_INIT                              0x000000CE
#define HIL_SYS_STATE_HIFPIO_INIT                           0x000000CD
#define HIL_SYS_STATE_SPI_INIT                              0x000000CC
#define HIL_SYS_STATE_FIQ_INIT                              0x000000CB
#define HIL_SYS_STATE_SEC_INIT                              0x000000CA
#define HIL_SYS_STATE_CRC_INIT                              0x000000C9
#define HIL_SYS_STATE_MEMORY_INIT                           0x000000C8
#define HIL_SYS_STATE_SER_FLASH_INIT                        0x000000C7
#define HIL_SYS_STATE_TASKS_INIT                            0x000000C6
#define HIL_SYS_STATE_MID_STA_INIT                          0x000000C5
#define HIL_SYS_STATE_MULTITASK_INIT                        0x000000C4
#define HIL_SYS_STATE_IDLE_TASK_INIT                        0x000000C3
#define HIL_SYS_STATE_GPIO_INIT                             0x000000C2
#define HIL_SYS_STATE_PIO_INIT                              0x000000C1
#define HIL_SYS_STATE_SUCCESS                               0x00000000

/* System Error */
#define HIL_SYS_SUCCESS                                     0x00000000
#define HIL_SYS_RAM_NOT_FOUND                               0x00000001
#define HIL_SYS_RAM_TYPE                                    0x00000002
#define HIL_SYS_RAM_SIZE                                    0x00000003
#define HIL_SYS_RAM_TEST                                    0x00000004
#define HIL_SYS_FLASH_NOT_FOUND                             0x00000005
#define HIL_SYS_FLASH_TYPE                                  0x00000006
#define HIL_SYS_FLASH_SIZE                                  0x00000007
#define HIL_SYS_FLASH_TEST                                  0x00000008
#define HIL_SYS_EEPROM_NOT_FOUND                            0x00000009
#define HIL_SYS_EEPROM_TYPE                                 0x0000000A
#define HIL_SYS_EEPROM_SIZE                                 0x0000000B
#define HIL_SYS_EEPROM_TEST                                 0x0000000C
#define HIL_SYS_SECURE_EEPROM                               0x0000000D
#define HIL_SYS_SECURE_EEPROM_NOT_INIT                      0x0000000E
#define HIL_SYS_FILE_SYSTEM_FAULT                           0x0000000F
#define HIL_SYS_VERSION_CONFLICT                            0x00000010
#define HIL_SYS_NOT_INITIALIZED                             0x00000011
#define HIL_SYS_MEM_ALLOC                                   0x00000012

/* System Hardware Features */
/* Extended Memory */
#define HIL_SYSTEM_EXTMEM_TYPE_MSK                          0x0000000F
#define HIL_SYSTEM_EXTMEM_TYPE_NONE                         0x00000000
#define HIL_SYSTEM_EXTMEM_TYPE_MRAM_128K                    0x00000001

#define HIL_SYSTEM_EXTMEM_ACCESS_MSK                        0x000000C0
#define HIL_SYSTEM_EXTMEM_ACCESS_NONE                       0x00000000
#define HIL_SYSTEM_EXTMEM_ACCESS_EXTERNAL                   0x00000040
#define HIL_SYSTEM_EXTMEM_ACCESS_INTERNAL                   0x00000080
#define HIL_SYSTEM_EXTMEM_ACCESS_BOTH                       0x000000C0

/* RTC */
#define HIL_SYSTEM_HW_RTC_MSK                               0x00000700
#define HIL_SYSTEM_HW_RTC_TYPE_MSK                          0x00000300
#define HIL_SYSTEM_HW_RTC_TYPE_NONE                         0x00000000
#define HIL_SYSTEM_HW_RTC_TYPE_INTERNAL                     0x00000100
#define HIL_SYSTEM_HW_RTC_TYPE_EXTERNAL                     0x00000200
#define HIL_SYSTEM_HW_RTC_TYPE_EMULATED                     0x00000300
#define HIL_SYSTEM_HW_RTC_STATE                             0x00000400

/*--------------------------------------------*/
/* SYSTEM INFORMATION BLOCK                   */
/*--------------------------------------------*/

/* Production date definition */
#define HIL_PRODUCTION_DATE_YEAR_MASK                       0xFF00    /*!< Year offset (0..255) starting at 2000 */
#define HIL_PRODUCTION_DATE_WEEK_MASK                       0x00FF    /*!< Week of year ( 1..52) */

/*--------------------------------------------*/
/* CHANNEL INFORMATION BLOCK                  */
/*--------------------------------------------*/
/* Channel type definitions */
#define HIL_CHANNEL_TYPE_UNDEFINED                          0         /*!< Type of the channel is undefined */
#define HIL_CHANNEL_TYPE_NOT_AVAILABLE                      1         /*!< Type of the channel not available */
#define HIL_CHANNEL_TYPE_RESERVED                           2         /*!< Reserved */
#define HIL_CHANNEL_TYPE_SYSTEM                             3         /*!< System channel */
#define HIL_CHANNEL_TYPE_HANDSHAKE                          4         /*!< Handshake channel */
#define HIL_CHANNEL_TYPE_COMMUNICATION                      5         /*!< Communication channel */
#define HIL_CHANNEL_TYPE_APPLICATION                        6         /*!< Application channnel */
#define HIL_CHANNEL_TYPE_MAX                                127       /*!< Maximum used channel number */
#define HIL_CHANNEL_TYPE_USER_DEFINED_START                 128       /*!< User defined channel */

/* Handshake cell, size and position */
#define HIL_HANDSHAKE_SIZE_MASK                             0x0F      /*!< Handshake size mask */
#define HIL_HANDSHAKE_SIZE_NOT_AVAILABLE                    0x00      /*!< No handshake cells */
#define HIL_HANDSHAKE_SIZE_8BIT                             0x01      /*!< Handshake cell size 8bit */
#define HIL_HANDSHAKE_SIZE_16BIT                            0x02      /*!< Handshake cell size 16bit */

#define HIL_HANDSHAKE_POSITION_MASK                         0xF0      /*!< Handshake position mask */
#define HIL_HANDSHAKE_POSITION_BEGINNING                    0x00      /*!< Handshake cells located at the start of each channel */
#define HIL_HANDSHAKE_POSITION_CHANNEL                      0x10      /*!< Handshake cells located in an own channel */

/*===========================================================================*/
/*                                                                           */
/* COMMUNICATION / APPLICATION CHANNEL Configuration definitions             */
/*                                                                           */
/*===========================================================================*/
/*-----------------------------------*/
/* CHANNEL CONTROL BLOCK             */
/*-----------------------------------*/
/* Application Change of State */
#define HIL_APP_COS_APPLICATION_READY                       0x00000001
#define HIL_APP_COS_BUS_ON                                  0x00000002
#define HIL_APP_COS_BUS_ON_ENABLE                           0x00000004
#define HIL_APP_COS_INITIALIZATION                          0x00000008
#define HIL_APP_COS_INITIALIZATION_ENABLE                   0x00000010
#define HIL_APP_COS_LOCK_CONFIGURATION                      0x00000020
#define HIL_APP_COS_LOCK_CONFIGURATION_ENABLE               0x00000040
#define HIL_APP_COS_DMA                                     0x00000080
#define HIL_APP_COS_DMA_ENABLE                              0x00000100

/* Application Change of State flags as bit number */
#define HIL_APP_COS_APPLICATION_READY_BIT_NO                0
#define HIL_APP_COS_BUS_ON_BIT_NO                           1
#define HIL_APP_COS_BUS_ON_ENABLE_BIT_NO                    2
#define HIL_APP_COS_INITIALIZATION_BIT_NO                   3
#define HIL_APP_COS_INITIALIZATION_ENABLE_BIT_NO            4
#define HIL_APP_COS_LOCK_CONFIGURATION_BIT_NO               5
#define HIL_APP_COS_LOCK_CONFIGURATION_ENABLE_BIT_NO        6
#define HIL_APP_COS_DMA_BIT_NO                              7
#define HIL_APP_COS_DMA_ENABLE_BIT_NO                       8

/*-----------------------------------*/
/* CHANNEL COMMON STATUS BLOCK       */
/*-----------------------------------*/
/* Channel Change Of State flags */
#define HIL_COMM_COS_UNDEFINED                              0x00000000
#define HIL_COMM_COS_READY                                  0x00000001
#define HIL_COMM_COS_RUN                                    0x00000002
#define HIL_COMM_COS_BUS_ON                                 0x00000004
#define HIL_COMM_COS_CONFIG_LOCKED                          0x00000008
#define HIL_COMM_COS_CONFIG_NEW                             0x00000010
#define HIL_COMM_COS_RESTART_REQUIRED                       0x00000020
#define HIL_COMM_COS_RESTART_REQUIRED_ENABLE                0x00000040
#define HIL_COMM_COS_DMA                                    0x00000080

/* Channel Change Of State flags as bit numbers */
#define HIL_COMM_COS_READY_BIT_NO                           0
#define HIL_COMM_COS_RUN_BIT_NO                             1
#define HIL_COMM_COS_BUS_ON_BIT_NO                          2
#define HIL_COMM_COS_CONFIG_LOCKED_BIT_NO                   3
#define HIL_COMM_COS_CONFIG_NEW_BIT_NO                      4
#define HIL_COMM_COS_RESTART_REQUIRED_BIT_NO                5
#define HIL_COMM_COS_RESTART_REQUIRED_ENABLE_BIT_NO         6
#define HIL_COMM_COS_DMA_BIT_NO                             7

/*===========================================================================*/
/*                                                                           */
/* Channel block information                                                 */
/*                                                                           */
/*===========================================================================*/

/*****************************************************************************/
/*! Block configuration information                                          */
/*****************************************************************************/
typedef struct HIL_DPM_BLOCK_DEFINITION_Ttag
{
  uint8_t   bChannelNumber;
  uint8_t   bBlockNumber;
  uint8_t   bBlockID;
  uint8_t   bPad;
  uint32_t  ulOffset;
  uint32_t  ulSize;
  uint16_t  usFlags;
  uint16_t  usHandshakeMode;
  uint16_t  usHandshakePosition;
  uint16_t  usReserved;
} HIL_DPM_BLOCK_DEFINITION_T;

/* Block ID */
#define HIL_BLOCK_MASK                                      0x00FFL
#define HIL_BLOCK_UNDEFINED                                 0x0000L
#define HIL_BLOCK_UNKNOWN                                   0x0001L
#define HIL_BLOCK_DATA_IMAGE                                0x0002L
#define HIL_BLOCK_DATA_IMAGE_HI_PRIO                        0x0003L
#define HIL_BLOCK_MAILBOX                                   0x0004L
#define HIL_BLOCK_CTRL_PARAM                                0x0005L
#define HIL_BLOCK_COMMON_STATE                              0x0006L
#define HIL_BLOCK_EXTENDED_STATE                            0x0007L
#define HIL_BLOCK_USER                                      0x0008L
#define HIL_BLOCK_INFO                                      0x0009L

/* Flags definition: Direction */
#define HIL_DIRECTION_MASK                                  0x000F
#define HIL_DIRECTION_UNDEFINED                             0x0000
#define HIL_DIRECTION_IN                                    0x0001
#define HIL_DIRECTION_OUT                                   0x0002
#define HIL_DIRECTION_INOUT                                 0x0003

/* Flags definition: Transmission type */
#define HIL_TRANSMISSION_TYPE_MASK                          0x00F0
#define HIL_TRANSMISSION_TYPE_UNDEFINED                     0x0000
#define HIL_TRANSMISSION_TYPE_DPM                           0x0010
#define HIL_TRANSMISSION_TYPE_DMA                           0x0020

/* Block definition: I/O Mode */
#define HIL_IO_MODE_DEFAULT                                 0x0000    /*!< I/O mode default, for compatibility reasons this value is identical to 0x4 (buffered host controlled) */
#define HIL_IO_MODE_BUFF_DEV_CTRL                           0x0002    /*!< I/O mode buffered device controlled */
#define HIL_IO_MODE_UNCONTROLLED                            0x0003    /*!< I/O uncontrolled */
#define HIL_IO_MODE_BUFF_HST_CTRL                           0x0004    /*!< I/O mode buffered host controlled */

/* Block definition: Synchronization Mode */
#define HIL_SYNC_MODE_OFF                                   0x00
#define HIL_SYNC_MODE_DEV_CTRL                              0x01
#define HIL_SYNC_MODE_HST_CTRL                              0x02

/* Block definition: Synchronization Sources */
#define HIL_SYNC_SOURCE_OFF                                 0x00
#define HIL_SYNC_SOURCE_1                                   0x01
#define HIL_SYNC_SOURCE_2                                   0x02


/*===========================================================================*/
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*===========================================================================*/

/* Status Information */
#define HIL_SI_STATE_SUCCESS                                0x0000

/* Fault State */
#define HIL_FS_STATE_UNDEFINED                              0x0000
#define HIL_FS_STATE_NO_FAULT                               0x0001
#define HIL_FS_STATE_CONF_ERROR                             0x0002
#define HIL_FS_STATE_RECOVERABLE                            0x0003
#define HIL_FS_STATE_SEVERE                                 0x0004
#define HIL_FS_STATE_FATAL                                  0x0005
#define HIL_FS_STATE_WATCHDOG                               0x0006

/* Network State */
#define HIL_COMM_STATE_UNKNOWN                              0x0000
#define HIL_COMM_STATE_NOT_CONFIGURED                       0x0001
#define HIL_COMM_STATE_STOP                                 0x0002
#define HIL_COMM_STATE_IDLE                                 0x0003
#define HIL_COMM_STATE_OPERATE                              0x0004

/* Input / Output data states */
#define HIL_IODS_FIELDBUS_MASK                              0x00F0
#define HIL_IODS_DATA_STATE_GOOD                            0x0080
#define HIL_IODS_PROVIDER_RUN                               0x0040
#define HIL_IODS_GENERATED_LOCALLY                          0x0020

#define HIL_SYS_BAD_MEMORY_COOKIE                           0x0BAD

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_UNPACK_1(HIL_DUALPORTMEMORY)
#endif

#endif  /* HIL_DUALPORTMEMORY_H_ */
