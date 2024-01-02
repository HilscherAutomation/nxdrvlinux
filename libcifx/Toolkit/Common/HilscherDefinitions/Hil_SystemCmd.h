/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_SystemCmd.h $: *//*!

  \file Hil_SystemCmd.h

  Hilscher Packet Command Codes Handled by the System.

**************************************************************************************/
#ifndef HIL_SYSTEMCMD_H_
#define HIL_SYSTEMCMD_H_

#include "Hil_Types.h"
#include "Hil_Packet.h"
#include "Hil_DualPortMemory.h"
#include "Hil_FirmwareIdent.h"

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_PACK_1(HIL_SYSTEMCMD)
#endif

#define RCX_PROCESS_QUEUE_NAME "RCX_QUE"

/* firmware control */
#define HIL_FIRMWARE_RESET_REQ                        0x00001E00
#define HIL_FIRMWARE_RESET_CNF                        0x00001E01

/* firmware information */
#define HIL_LISTS_GET_NUM_ENTRIES_REQ                 0x00001E10
#define HIL_LISTS_GET_NUM_ENTRIES_CNF                 0x00001E11

/* system error log access */
#define HIL_SYSTEM_ERRORLOG_REQ                       0x00001E12
#define HIL_SYSTEM_ERRORLOG_CNF                       0x00001E13

#define HIL_EXCEPTION_INFO_REQ                        0x00001E14
#define HIL_EXCEPTION_INFO_CNF                        0x00001E15

/* queue identification */
#define HIL_QUE_IDENTIFY_REQ                          0x00001E20
#define HIL_QUE_IDENTIFY_CNF                          0x00001E21
#define HIL_QUE_IDENTIFY_IDX_REQ                      0x00001E22
#define HIL_QUE_IDENTIFY_IDX_CNF                      0x00001E23

/* queue load retrieval */
#define HIL_QUE_GET_LOAD_REQ                          0x00001E30
#define HIL_QUE_GET_LOAD_CNF                          0x00001E31

/* DPM data access functions */
#define HIL_SYSTEM_INFORMATION_BLOCK_REQ              0x00001E32
#define HIL_SYSTEM_INFORMATION_BLOCK_CNF              0x00001E33
#define HIL_CHANNEL_INFORMATION_BLOCK_REQ             0x00001E34
#define HIL_CHANNEL_INFORMATION_BLOCK_CNF             0x00001E35
#define HIL_SYSTEM_CONTROL_BLOCK_REQ                  0x00001E36
#define HIL_SYSTEM_CONTROL_BLOCK_CNF                  0x00001E37
#define HIL_SYSTEM_STATUS_BLOCK_REQ                   0x00001E38
#define HIL_SYSTEM_STATUS_BLOCK_CNF                   0x00001E39
#define HIL_CONTROL_BLOCK_REQ                         0x00001E3A
#define HIL_CONTROL_BLOCK_CNF                         0x00001E3B
#define HIL_HANDSHAKE_CHANNEL_REQ                     0x00001E3C
#define HIL_HANDSHAKE_CHANNEL_CNF                     0x00001E3D

/* task identification and readout of diagnosis data */
#define HIL_TSK_GET_NAME_REQ                          0x00001E3E
#define HIL_TSK_GET_NAME_CNF                          0x00001E3F
#define HIL_TSK_IDENTIFY_REQ                          0x00001E40
#define HIL_TSK_IDENTIFY_CNF                          0x00001E41
#define HIL_TSK_IDENTIFY_IDX_REQ                      0x00001E42
#define HIL_TSK_IDENTIFY_IDX_CNF                      0x00001E43
#define HIL_TSK_GET_STATUS_REQ                        0x00001E44
#define HIL_TSK_GET_STATUS_CNF                        0x00001E45
#define HIL_TSK_GET_INFO_FIELD_REQ                    0x00001E46
#define HIL_TSK_GET_INFO_FIELD_CNF                    0x00001E47

/* task control */
#define HIL_TSK_START_REQ                             0x00001E48
#define HIL_TSK_START_CNF                             0x00001E49
#define HIL_TSK_STOP_REQ                              0x00001E4A
#define HIL_TSK_STOP_CNF                              0x00001E4B

#define HIL_TSK_GET_STATUS_ARRAY_REQ                  0x00001E4E
#define HIL_TSK_GET_STATUS_ARRAY_CNF                  0x00001E4F

/* task array identification and structure information readout */
#define HIL_TSK_GET_INFO_ARRAY_REQ                    0x00001E50
#define HIL_TSK_GET_INFO_ARRAY_CNF                    0x00001E51
#define HIL_TSK_GET_INFO_STRUCT_REQ                   0x00001E52
#define HIL_TSK_GET_INFO_STRUCT_CNF                   0x00001E53
#define HIL_TSK_GET_INFO_STRUCT_IDX_REQ               0x00001E54
#define HIL_TSK_GET_INFO_STRUCT_IDX_CNF               0x00001E55
#define HIL_TSK_GET_INFO_FIELD_SIZE_REQ               0x00001E56
#define HIL_TSK_GET_INFO_FIELD_SIZE_CNF               0x00001E57
#define HIL_TSK_GET_INFO_FIELD_SIZE_IDX_REQ           0x00001E58
#define HIL_TSK_GET_INFO_FIELD_SIZE_IDX_CNF           0x00001E59

/* read information about memory usage */
#define HIL_MALLINFO_REQ                              0x00001E5A
#define HIL_MALLINFO_CNF                              0x00001E5B

/* file upload/download */
#define HIL_FILE_DOWNLOAD_REQ                         0x00001E62
#define HIL_FILE_DOWNLOAD_CNF                         0x00001E63
#define HIL_FILE_DOWNLOAD_DATA_REQ                    0x00001E64
#define HIL_FILE_DOWNLOAD_DATA_CNF                    0x00001E65

#define HIL_FILE_DOWNLOAD_ABORT_REQ                   0x00001E66
#define HIL_FILE_DOWNLOAD_ABORT_CNF                   0x00001E67

#define HIL_FILE_UPLOAD_REQ                           0x00001E60
#define HIL_FILE_UPLOAD_CNF                           0x00001E61
#define HIL_FILE_UPLOAD_DATA_REQ                      0x00001E6E
#define HIL_FILE_UPLOAD_DATA_CNF                      0x00001E6F

#define HIL_FILE_UPLOAD_ABORT_REQ                     0x00001E5E
#define HIL_FILE_UPLOAD_ABORT_CNF                     0x00001E5F

#define HIL_FORMAT_REQ                                0x00001ED6
#define HIL_FORMAT_CNF                                0x00001ED7

/* file information */
#define HIL_FILE_GET_MD5_REQ                          0x00001E68
#define HIL_FILE_GET_MD5_CNF                          0x00001E69
#define HIL_FILE_GET_HEADER_MD5_REQ                   0x00001E72
#define HIL_FILE_GET_HEADER_MD5_CNF                   0x00001E73

/* file delete */
#define HIL_FILE_DELETE_REQ                           0x00001E6A
#define HIL_FILE_DELETE_CNF                           0x00001E6B

/* file rename */
#define HIL_FILE_RENAME_REQ                           0x00001E7C
#define HIL_FILE_RENAME_CNF                           0x00001E7D

/* volume information */
#define HIL_VOLUME_GET_ENTRY_REQ                      0x00001E6C
#define HIL_VOLUME_GET_ENTRY_CNF                      0x00001E6D

/* directory list */
#define HIL_DIR_LIST_REQ                              0x00001E70
#define HIL_DIR_LIST_CNF                              0x00001E71

/* indexed task access */
#define HIL_TSK_GET_STATUS_IDX_REQ                    0x00001E74
#define HIL_TSK_GET_STATUS_IDX_CNF                    0x00001E75
#define HIL_TSK_GET_INFO_FIELD_IDX_REQ                0x00001E76
#define HIL_TSK_GET_INFO_FIELD_IDX_CNF                0x00001E77

/* indexed task control */
#define HIL_TSK_START_IDX_REQ                         0x00001E78
#define HIL_TSK_START_IDX_CNF                         0x00001E79
#define HIL_TSK_STOP_IDX_REQ                          0x00001E7A
#define HIL_TSK_STOP_IDX_CNF                          0x00001E7B

/* log queues */
#define HIL_QUE_LOG_SET_REQ                           0x00001EA0
#define HIL_QUE_LOG_SET_CNF                           0x00001EA1
#define HIL_QUE_LOG_CLR_REQ                           0x00001EA2
#define HIL_QUE_LOG_CLR_CNF                           0x00001EA3

/* read/write memory */
#define HIL_PHYSMEM_READ_REQ                          0x00001EA8
#define HIL_PHYSMEM_READ_CNF                          0x00001EA9

#define HIL_PHYSMEM_WRITE_REQ                         0x00001EAA
#define HIL_PHYSMEM_WRITE_CNF                         0x00001EAB

/* firmware info */
#define HIL_GET_LIB_VERSION_INFO_REQ                  0x00001EBA
#define HIL_GET_LIB_VERSION_INFO_CNF                  0x00001EBB

#define HIL_FIRMWARE_IDENTIFY_REQ                     0x00001EB6
#define HIL_FIRMWARE_IDENTIFY_CNF                     0x00001EB7


#define HIL_HW_IDENTIFY_REQ                           0x00001EB8
#define HIL_HW_IDENTIFY_CNF                           0x00001EB9

#define HIL_SECURITY_EEPROM_READ_REQ                  0x00001EBC
#define HIL_SECURITY_EEPROM_READ_CNF                  0x00001EBD
#define HIL_SECURITY_EEPROM_WRITE_REQ                 0x00001EBE
#define HIL_SECURITY_EEPROM_WRITE_CNF                 0x00001EBF

/* module management */
#define HIL_MODULE_INSTANTIATE_REQ                    0x00001EC0
#define HIL_MODULE_INSTANTIATE_CNF                    0x00001EC1
#define HIL_MODULE_GET_INFO_IDX_REQ                   0x00001EC2
#define HIL_MODULE_GET_INFO_IDX_CNF                   0x00001EC3
#define HIL_CHANNEL_INSTANTIATE_REQ                   0x00001EC4
#define HIL_CHANNEL_INSTANTIATE_CNF                   0x00001EC5

/* Device Data Provider (DDP) access */
#define HIL_DDP_SERVICE_GET_REQ                       0x00001EEA
#define HIL_DDP_SERVICE_GET_CNF                       0x00001EEB
#define HIL_DDP_SERVICE_SET_REQ                       0x00001EEC
#define HIL_DDP_SERVICE_SET_CNF                       0x00001EED

#define HIL_SET_MAC_ADDR_REQ                          0x00001EEE
#define HIL_SET_MAC_ADDR_CNF                          0x00001EEF

#define HIL_HW_LICENSE_INFO_REQ                       0x00001EF4
#define HIL_HW_LICENSE_INFO_CNF                       0x00001EF5

#define HIL_HW_HARDWARE_INFO_REQ                      0x00001EF6
#define HIL_HW_HARDWARE_INFO_CNF                      0x00001EF7

/* DPM info functions */
#define HIL_DPM_GET_BLOCK_INFO_REQ                    0x00001EF8
#define HIL_DPM_GET_BLOCK_INFO_CNF                    0x00001EF9

/* Communication flag info functions */
#define HIL_DPM_GET_COMFLAG_INFO_REQ                  0x00001EFA
#define HIL_DPM_GET_COMFLAG_INFO_CNF                  0x00001EFB


/* Common Status block functions */
#define HIL_DPM_GET_COMMON_STATE_REQ                  0x00001EFC
#define HIL_DPM_GET_COMMON_STATE_CNF                  0x00001EFD

/* Extended status block */
#define HIL_DPM_GET_EXTENDED_STATE_REQ                0x00001EFE
#define HIL_DPM_GET_EXTENDED_STATE_CNF                0x00001EFF

/* reserved for further functions (documented at DPM Spec) */

#define HIL_ENABLE_PERF_MEASUREMENT_REQ               0x00001ED2
#define HIL_ENABLE_PERF_MEASUREMENT_CNF               0x00001ED3

#define HIL_GET_PERF_COUNTERS_REQ                     0x00001ED4
#define HIL_GET_PERF_COUNTERS_CNF                     0x00001ED5

/* Time handling requests */
#define HIL_TIME_COMMAND_REQ                          0x00001ED8
#define HIL_TIME_COMMAND_CNF                          0x00001ED9


/* Backup / Restore commands */
#define HIL_BACKUP_REQ                                0x00001F50
#define HIL_BACKUP_CNF                                0x00001F51
#define HIL_RESTORE_REQ                               0x00001F52
#define HIL_RESTORE_CNF                               0x00001F53
/* do NOT add further commands in the range 0x00001FXX (because of collosion with PNS_IF task) */



/******************************************************************************
 * Packet: HIL_TIME_COMMAND_REQ/HIL_TIME_COMMAND_CNF
 *
 */

/* Time command codes */
#define TIME_CMD_GETSTATE                 0x00000001
#define TIME_CMD_GETTIME                  0x00000002
#define TIME_CMD_SETTIME                  0x00000003

/* Time RTC information */
#define TIME_INFO_RTC_MSK                 0x00000007
#define TIME_INFO_RTC_TYPE_MSK            0x00000003
#define TIME_INFO_RTC_RTC_STATE           0x00000004

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TIME_CMD_DATA_Ttag
{
  uint32_t  ulTimeCmd;
  uint32_t  ulData;
  uint32_t  ulReserved;
} HIL_TIME_CMD_DATA_T;

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TIME_CMD_REQ_Ttag
{
  HIL_PACKET_HEADER_T   tHead;  /*!< packet header  */
  HIL_TIME_CMD_DATA_T   tData;  /*!< packet data    */
} HIL_TIME_CMD_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TIME_CMD_CNF_Ttag
{
  HIL_PACKET_HEADER_T   tHead;  /*!< packet header  */
  HIL_TIME_CMD_DATA_T   tData;  /*!< packet data    */
} HIL_TIME_CMD_CNF_T;


/******************************************************************************
 * Packet: HIL_ENABLE_PERF_MEASUREMENT_REQ/HIL_ENABLE_PERF_MEASUREMENT_CNF
 *
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_ENABLE_PERF_MEASUREMENT_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;
} HIL_ENABLE_PERF_MEASUREMENT_REQ_T;

/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_ENABLE_PERF_MEASUREMENT_CNF_DATA_Ttag
{
  uint32_t                              ulOldState;
} HIL_ENABLE_PERF_MEASUREMENT_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_ENABLE_PERF_MEASUREMENT_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;
  HIL_ENABLE_PERF_MEASUREMENT_CNF_DATA_T  tData;
} HIL_ENABLE_PERF_MEASUREMENT_CNF_T;

/******************************************************************************
 * Packet: HIL_GET_PERF_COUNTERS_REQ/HIL_GET_PERF_COUNTERS_CNF
 *
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_PERF_COUNTERS_REQ_DATA_Ttag
{
  uint16_t                              usStartToken;
  uint16_t                              usTokenCount;
} HIL_GET_PERF_COUNTERS_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_PERF_COUNTERS_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;
  HIL_GET_PERF_COUNTERS_REQ_DATA_T        tData;
} HIL_GET_PERF_COUNTERS_REQ_T;

/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PERF_COUNTER_DATA_Ttag
{
  uint32_t                              ulNanosecondsLower;
  uint32_t                              ulNanosecondsUpper;
} HIL_PERF_COUNTER_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_PERF_COUNTERS_CNF_DATA_Ttag
{
  uint16_t                              usStartToken;
  uint16_t                              usTokenCount;
  HIL_PERF_COUNTER_DATA_T                 tPerfSystemUptime;
  /* following entry is a placeholder for a dynamic array whose length is given by ulLen in the packet header */
  HIL_PERF_COUNTER_DATA_T                 atPerfCounters[1];
} HIL_GET_PERF_COUNTERS_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_PERF_COUNTERS_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;
  HIL_GET_PERF_COUNTERS_CNF_DATA_T        tData;
} HIL_GET_PERF_COUNTERS_CNF_T;

/******************************************************************************
 * Packet: HIL_SET_MAC_ADDR_REQ/HIL_SET_MAC_ADDR_CNF
 *
 *          This packet reconfigures the MAC address
 *          If HIL_STORE_MAC_ADDRESS is set, it will also update the Sec EEPROM permanently.
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_MAC_ADDR_REQ_DATA_Ttag
{
  uint32_t                      ulParam;              /*!< Parameter Bit Field */
  uint8_t                       abMacAddr[6];         /*!< MAC address */
  uint8_t                       abPad[2];             /*!< Pad bytes, set to zero */
} HIL_SET_MAC_ADDR_REQ_DATA_T;

#define HIL_STORE_MAC_ADDRESS         0x00000001
#define HIL_FORCE_MAC_ADDRESS         0x00000002

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_MAC_ADDR_REQ_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /*!< packet header */
  HIL_SET_MAC_ADDR_REQ_DATA_T     tData;                /*!< packet data */
} HIL_SET_MAC_ADDR_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_MAC_ADDR_CNF_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /*!< packet header */
} HIL_SET_MAC_ADDR_CNF_T;


/******************************************************************************
 * Packet: HIL_FIRMWARE_RESET_REQ/HIL_FIRMWARE_RESET_CNF
 *
 *          This packet executes a RESET on the netX
 */

/* Hilscher reset mode definitions */
#define HIL_RESET_MODE_CMD_MSK                    0x0000000F
#define HIL_RESET_MODE_CMD_SRT                    0

#define HIL_RESET_MODE_COLDSTART                  0
#define HIL_RESET_MODE_WARMSTART                  1
#define HIL_RESET_MODE_BOOTSTART                  2
#define HIL_RESET_MODE_UPDATESTART                3
#define HIL_RESET_MODE_CONSOLESTART               4

/* Hilscher reset parameter definitions */
#define HIL_RESET_PARAM_MSK                       0x000000F0
#define HIL_RESET_PARAM_SRT                       4

#define HIL_RESET_PARAM_CONSOLE_ETH               0x00000000
#define HIL_RESET_PARAM_CONSOLE_UART              0x00000001
#define HIL_RESET_PARAM_CONSOLE_USB               0x00000002
#define HIL_RESET_PARAM_CONSOLE_DPM               0x00000003  /* Reserved / Unused */
#define HIL_RESET_PARAM_CONSOLE_DEFAULT           0x0000000F

/* Hilscher additional reset flag definitions */
#define HIL_RESET_FLAG_MSK                        0xFFFFFF00
#define HIL_RESET_FLAG_SRT                        8

/* Clear REMANENT flag */
#define HIL_RESET_FLAG_CLEAR_REMANENT_MSK         0x00000100
#define HIL_RESET_FLAG_CLEAR_REMANENT_SRT         8

/* Install VERIFICATION flag */
#define HIL_RESET_FLAG_VERIFY_INSTALLATION_MSK    0x00000200
#define HIL_RESET_FLAG_VERIFY_INSTALLATION_SRT    9


/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FIRMWARE_RESET_REQ_DATA_Ttag
{
  uint32_t                      ulTimeToReset;        /* time to reset in ms */
  uint32_t                      ulResetMode;          /* reset mode param */
} HIL_FIRMWARE_RESET_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FIRMWARE_RESET_REQ_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /* packet header */
  HIL_FIRMWARE_RESET_REQ_DATA_T   tData;                /* packet data */
} HIL_FIRMWARE_RESET_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FIRMWARE_RESET_CNF_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /* packet header */
} HIL_FIRMWARE_RESET_CNF_T;


/******************************************************************************
 * Packet: HIL_QUE_IDENTIFY_REQ/HIL_QUE_IDENTIFY_CNF
 *
 *        This packet allows identifying a queue by name
 */


/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_IDENTIFY_REQ_DATA_Ttag
{
  uint8_t                       szQueName[16];        /* queue name */
  uint32_t                      ulInst;               /* instance of queue */
} HIL_QUE_IDENTIFY_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_IDENTIFY_REQ_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /* packet header */
  HIL_QUE_IDENTIFY_REQ_DATA_T     tData;                /* packet data */
} HIL_QUE_IDENTIFY_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_IDENTIFY_CNF_DATA_Ttag
{
  uint32_t                      ulQue;                /* queue handle */
} HIL_QUE_IDENTIFY_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_IDENTIFY_CNF_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /* packet header */
  HIL_QUE_IDENTIFY_CNF_DATA_T     tData;                /* packet data */
} HIL_QUE_IDENTIFY_CNF_T;


/******************************************************************************
 * Packet: HIL_QUE_IDENTIFY_IDX_REQ/HIL_QUE_IDENTIFY_IDX_CNF
 *
 *        This packet allows identifying a queue by index
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_IDENTIFY_IDX_REQ_DATA_Ttag
{
  uint32_t                      ulIndex;              /* queue table index */
} HIL_QUE_IDENTIFY_IDX_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_IDENTIFY_IDX_REQ_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /* packet header */
  HIL_QUE_IDENTIFY_IDX_REQ_DATA_T tData;                /* packet data */
} HIL_QUE_IDENTIFY_IDX_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_IDENTIFY_IDX_CNF_DATA_Ttag
{
  uint32_t                      ulIndex;              /* queue table index */
  uint8_t                       szQueName[16];        /* queue name */
  uint32_t                      ulInst;               /* instance of queue */
  uint32_t                      ulQue;                /* queue handle */
} HIL_QUE_IDENTIFY_IDX_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_IDENTIFY_IDX_CNF_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /* packet header */
  HIL_QUE_IDENTIFY_IDX_CNF_DATA_T tData;                /* packet data */
} HIL_QUE_IDENTIFY_IDX_CNF_T;


/******************************************************************************
 * Packet: HIL_TSK_IDENTIFY_REQ/HIL_TSK_IDENTIFY_CNF
 *
 *          This packet allows identifying a task by name
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_IDENTIFY_REQ_DATA_Ttag
{
  uint8_t                       szTskName[16];        /* task name */
  uint32_t                      ulInst;               /* task instance */
} HIL_TSK_IDENTIFY_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_IDENTIFY_REQ_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /* packet header */
  HIL_TSK_IDENTIFY_REQ_DATA_T     tData;                /* packet data */
} HIL_TSK_IDENTIFY_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_IDENTIFY_CNF_DATA_Ttag
{
  uint32_t                      ulTsk;                /* task handle */
  uint32_t                      ulTskIdentifier;      /* task identifier */
  uint16_t                      usTskMajorVersion;    /* task major version */
  uint16_t                      usTskMinorVersion;    /* task minor version */
  uint32_t                      ulNumOfDiagStructs;   /* count of task diagnostic structures */
  uint32_t                      ulPriority;           /* task priority */
} HIL_TSK_IDENTIFY_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_IDENTIFY_CNF_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /* packet header */
  HIL_TSK_IDENTIFY_CNF_DATA_T     tData;                /* packet data */
} HIL_TSK_IDENTIFY_CNF_T;


/******************************************************************************
 * Packet: HIL_TSK_GET_NAME_REQ/HIL_TSK_GET_NAME_CNF
 *
 *          This packet allows retrieving a task name from a given handle
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_NAME_REQ_DATA_Ttag
{
  uint32_t                      ulTsk;                /* task handle */
} HIL_TSK_GET_NAME_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_NAME_REQ_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /* packet header */
  HIL_TSK_GET_NAME_REQ_DATA_T     tData;                /* packet data */
} HIL_TSK_GET_NAME_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_NAME_CNF_DATA_Ttag
{
  uint8_t                       abTskName[16];        /* task name */
  uint32_t                      ulInstance;           /* task instance */
} HIL_TSK_GET_NAME_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_NAME_CNF_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /* packet header */
  HIL_TSK_GET_NAME_CNF_DATA_T     tData;                /* packet data */
} HIL_TSK_GET_NAME_CNF_T;


/******************************************************************************
 * Packet: HIL_TSK_IDENTIFY_IDX_REQ/HIL_TSK_IDENTIFY_IDX_CNF
 *
 *          This packet allows identifying a task by index
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_IDENTIFY_IDX_REQ_DATA_Ttag
{
  uint32_t                      ulIndex;              /* task table index */
} HIL_TSK_IDENTIFY_IDX_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_IDENTIFY_IDX_REQ_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /* packet header */
  HIL_TSK_IDENTIFY_IDX_REQ_DATA_T tData;                /* packet data */
} HIL_TSK_IDENTIFY_IDX_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_IDENTIFY_IDX_CNF_DATA_Ttag
{
  uint32_t                      ulIndex;              /* task table index */
  uint8_t                       szTskName[16];        /* task name */
  uint32_t                      ulInstance;           /* task instance */
  uint32_t                      ulTsk;                /* task handle */
  uint32_t                      ulTskIdentifier;      /* task identifier */
  uint16_t                      usTskMajorVersion;    /* task major version */
  uint16_t                      usTskMinorVersion;    /* task minor version */
  uint32_t                      ulNumOfDiagStructs;   /* count of task diagnostic structures */
  uint32_t                      ulPriority;           /* task priority */
} HIL_TSK_IDENTIFY_IDX_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_IDENTIFY_IDX_CNF_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /* packet header */
  HIL_TSK_IDENTIFY_IDX_CNF_DATA_T tData;                /* packet data */
} HIL_TSK_IDENTIFY_IDX_CNF_T;


/******************************************************************************
 * Packet: HIL_TSK_GET_STATUS_IDX_REQ/HIL_TSK_GET_STATUS_IDX_CNF
 * Packet: HIL_TSK_GET_STATUS_REQ    /HIL_TSK_GET_STATUS_CNF
 *
 *          These packets allow retrieving the task status either by index or handle
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_STATUS_REQ_DATA_Ttag
{
  uint32_t                      ulTsk;                /* task handle */
} HIL_TSK_GET_STATUS_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_STATUS_REQ_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /* packet header */
  HIL_TSK_GET_STATUS_REQ_DATA_T   tData;                /* packet data */
} HIL_TSK_GET_STATUS_REQ_T;

typedef HIL_TSK_GET_STATUS_REQ_T HIL_TSK_GET_STATUS_IDX_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_STATUS_CNF_DATA_Ttag
{
  uint32_t                      ulStatusCode;         /* task status code */
} HIL_TSK_GET_STATUS_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_STATUS_CNF_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                /* packet header */
  HIL_TSK_GET_STATUS_CNF_DATA_T   tData;                /* packet data */
} HIL_TSK_GET_STATUS_CNF_T;

typedef HIL_TSK_GET_STATUS_CNF_T HIL_TSK_GET_STATUS_IDX_CNF_T;


/******************************************************************************
 * Packet: HIL_TSK_GET_INFO_FIELD_REQ/HIL_TSK_GET_INFO_FIELD_CNF
 *
 *          This packet retrieves the diagnostic structures of a given task
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_FIELD_REQ_DATA_Ttag
{
  uint32_t                      ulTsk;                /* task handle */
  uint32_t                      ulInfoNo;             /* index of information field */
} HIL_TSK_GET_INFO_FIELD_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_FIELD_REQ_Ttag
{
  HIL_PACKET_HEADER_T               tHead;              /* packet header */
  HIL_TSK_GET_INFO_FIELD_REQ_DATA_T tData;              /* packet data */
} HIL_TSK_GET_INFO_FIELD_REQ_T;

typedef HIL_TSK_GET_INFO_FIELD_REQ_T HIL_TSK_GET_INFO_FIELD_IDX_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_FIELD_CNF_DATA_Ttag
{
  uint8_t                       abData[1];       /* ATTENTION: This is a placeholder for info field data.
                                                                 The packet must be allocated in correct size*/
} HIL_TSK_GET_INFO_FIELD_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_FIELD_CNF_Ttag
{
  HIL_PACKET_HEADER_T               tHead;            /* packet header */
  HIL_TSK_GET_INFO_FIELD_CNF_DATA_T tData;            /* packet data */
} HIL_TSK_GET_INFO_FIELD_CNF_T;

typedef HIL_TSK_GET_INFO_FIELD_CNF_T HIL_TSK_GET_INFO_FIELD_IDX_CNF_T;


/******************************************************************************
 * Packet: HIL_TSK_GET_INFO_FIELD_SIZE_REQ/HIL_TSK_GET_INFO_FIELD_SIZE_CNF
 *
 *          This function retrieves the sizes of the diagnostic structures of a given task
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_FIELD_SIZE_REQ_DATA_Ttag
{
  uint32_t                      ulTsk;              /* task handle */
  uint32_t                      ulInfoNo;           /* index of information field */
} HIL_TSK_GET_INFO_FIELD_SIZE_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_FIELD_SIZE_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;      /* packet header */
  HIL_TSK_GET_INFO_FIELD_SIZE_REQ_DATA_T  tData;      /* packet data */
} HIL_TSK_GET_INFO_FIELD_SIZE_REQ_T;

typedef HIL_TSK_GET_INFO_FIELD_SIZE_REQ_T HIL_TSK_GET_INFO_FIELD_SIZE_IDX_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_FIELD_SIZE_CNF_DATA_Ttag
{
  uint32_t                              ulInfoSize; /* info field size in bytes */
} HIL_TSK_GET_INFO_FIELD_SIZE_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_FIELD_SIZE_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;      /* packet header */
  HIL_TSK_GET_INFO_FIELD_SIZE_CNF_DATA_T  tData;      /* packet data */
} HIL_TSK_GET_INFO_FIELD_SIZE_CNF_T;

typedef HIL_TSK_GET_INFO_FIELD_SIZE_CNF_T HIL_TSK_GET_INFO_FIELD_SIZE_IDX_CNF_T;


/******************************************************************************
 * Packet: HIL_TSK_GET_INFO_STRUCT_REQ/HIL_TSK_GET_INFO_STRUCT_CNF
 *
 *          This function retrieves the structural information of the
 *          diagnostic structures of a given task
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_STRUCT_REQ_DATA_Ttag
{
  uint32_t                              ulTsk;      /* task handle */
  uint32_t                              ulInfoNo;   /* index of information field */
} HIL_TSK_GET_INFO_STRUCT_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_STRUCT_FIELD_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;      /* packet header */
  HIL_TSK_GET_INFO_STRUCT_REQ_DATA_T      tData;      /* packet data */
} HIL_TSK_GET_INFO_STRUCT_REQ_T;

typedef HIL_TSK_GET_INFO_FIELD_REQ_T HIL_TSK_GET_INFO_STRUCT_IDX_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_STRUCT_CNF_DATA_Ttag
{
  uint8_t                               abData[1]; /* ATTENTION: This is a placeholder for info field data.
                                                                   The packet must be allocated in correct size*/
} HIL_TSK_GET_INFO_STRUCT_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_STRUCT_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;      /* packet header */
  HIL_TSK_GET_INFO_FIELD_CNF_DATA_T       tData;      /* packet data */
} HIL_TSK_GET_INFO_STRUCT_CNF_T;

typedef HIL_TSK_GET_INFO_FIELD_CNF_T HIL_TSK_GET_INFO_STRUCT_IDX_CNF_T;


/******************************************************************************
 * Packet: HIL_TSK_GET_INFO_ARRAY_REQ/HIL_TSK_GET_INFO_ARRAY_CNF
 *
 *          This function retrieves the diagnostic structure count and the task handle
 *          by reading the task table in an indexed way
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_ARRAY_REQ_DATA_Ttag
{
  uint32_t                              ulStartIndex; /* start index of requested list */
} HIL_TSK_GET_INFO_ARRAY_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_ARRAY_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;      /* packet header */
  HIL_TSK_GET_INFO_ARRAY_REQ_DATA_T       tData;      /* packet data */
} HIL_TSK_GET_INFO_ARRAY_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_ARRAY_CNF_DATA_ELEMENTtag
{
  uint32_t                              ulTsk;                /* task handle */
  uint32_t                              ulNumberOfInfoFields; /* number of info fields */
  /* the field ulInfoNo of the earlier two requests will have the value range of 0 - (ulNumberOfInfoFields-1) */
} HIL_TSK_GET_INFO_ARRAY_CNF_DATA_ELEMENT;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_ARRAY_CNF_DATA_Ttag
{
  uint32_t                              ulStartIndex;       /* start index */
  uint32_t                              ulNumberOfEntries;  /* number of entries (declaring range of start to start+count-1) */
  uint32_t                              ulCurrentTskCount;  /* current number of tasks */
  HIL_TSK_GET_INFO_ARRAY_CNF_DATA_ELEMENT atInfoData[1];      /* ATTENTION: This is a placeholder for task info data.
                                                                            The packet must be allocated in correct size*/
} HIL_TSK_GET_INFO_ARRAY_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_INFO_ARRAY_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;              /* packet header */
  HIL_TSK_GET_INFO_ARRAY_CNF_DATA_T       tData;              /* packet data */
} HIL_TSK_GET_INFO_ARRAY_CNF_T;


/******************************************************************************
 * Packet: HIL_TSK_START_REQ/HIL_TSK_START_CNF
 *
 *          This packet allows starting a user task
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_START_REQ_DATA_Ttag
{
  uint32_t                              ulTsk;              /* task handle */
} HIL_TSK_START_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_START_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;              /* packet header */
  HIL_TSK_START_REQ_DATA_T                tData;              /* packet data */
} HIL_TSK_START_REQ_T;

typedef HIL_TSK_START_REQ_T HIL_TSK_START_IDX_REQ_T;

/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_START_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
} HIL_TSK_START_CNF_T;


/******************************************************************************
 * Packet: HIL_TSK_STOP_REQ/HIL_TSK_STOP_CNF
 *
 *          This packet allows stopping a user task
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_STOP_REQ_DATA_Ttag
{
  uint32_t                              ulTsk;                /* task handle */
} HIL_TSK_STOP_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_STOP_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                /* packet header */
  HIL_TSK_STOP_REQ_DATA_T                 tData;                /* packet data */
} HIL_TSK_STOP_REQ_T;

typedef HIL_TSK_STOP_REQ_T HIL_TSK_STOP_IDX_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_STOP_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
} HIL_TSK_STOP_CNF_T;


/******************************************************************************
 * Packet: MID_SYS_QUE_LOG_SET_REQ/MID_SYS_QUE_LOG_SET_CNF
 *
 *          This packet sets a logical queue entry
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_LOG_SET_REQ_DATA_Ttag
{
  uint32_t                              ulLogQue;               /* logical queue id */
} HIL_QUE_LOG_SET_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_LOG_SET_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_QUE_LOG_SET_REQ_DATA_T              tData;                  /* packet data */
} HIL_QUE_LOG_SET_REQ_T;

/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_LOG_SET_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
} HIL_QUE_LOG_SET_CNF_T;


/******************************************************************************
 * Packet: MID_SYS_QUE_LOG_CLR_REQ/MID_SYS_QUE_LOG_CLR_CNF
 *
 *          This packet clears a logical queue entry
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_LOG_CLR_REQ_DATA_Ttag
{
  uint32_t                              ulLogQue;               /* logical queue id */
} HIL_QUE_LOG_CLR_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_LOG_CLR_REQ_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                          /* packet header */
  HIL_QUE_LOG_CLR_REQ_DATA_T      tData;                          /* packet data */
} HIL_QUE_LOG_CLR_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_LOG_CLR_CNF_Ttag
{
  HIL_PACKET_HEADER_T             tHead;                          /* packet data */
} HIL_QUE_LOG_CLR_CNF_T;


/******************************************************************************
 * Packet: HIL_QUE_GET_LOAD_REQ/HIL_QUE_GET_LOAD_CNF
 *
 *          This packet allows retrieving the queue load of a given queue
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_GET_LOAD_REQ_DATA_Ttag
{
  uint32_t                            ulQue[1];   /* ATTENTION: This is a placeholder for number of queues to query.
                                                                  The packet must be allocated in correct size*/
} HIL_QUE_GET_LOAD_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_GET_LOAD_REQ_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                  /* packet header */
  HIL_QUE_GET_LOAD_REQ_DATA_T           tData;                  /* packet data */
} HIL_QUE_GET_LOAD_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_GET_LOAD_CNF_DATA_Ttag
{
  uint32_t                            ulQueLoad[1]; /* ATTENTION: This is a placeholder for queue load entries
                                                                    The packet must be allocated in correct size*/
} HIL_QUE_GET_LOAD_CNF_DATA_T;

#define HIL_QUE_LOAD_INVALID (0xffffffffL)

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_QUE_GET_LOAD_CNF_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                  /* packet header */
  HIL_QUE_GET_LOAD_CNF_DATA_T           tData;                  /* packet data */
} HIL_QUE_GET_LOAD_CNF_T;


/******************************************************************************
 * Packet: HIL_PHYSMEM_READ_REQ/HIL_PHYSMEM_READ_CNF
 *
 *          This packet allows read accesss to physical memory area
 */

#define HIL_PHYSMEM_ACCESSTYPE_8BIT     0
#define HIL_PHYSMEM_ACCESSTYPE_16BIT    1
#define HIL_PHYSMEM_ACCESSTYPE_32BIT    2
#define HIL_PHYSMEM_ACCESSTYPE_TASK     3

/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PHYSMEM_READ_REQ_DATA_Ttag
{
  uint32_t ulPhysicalAddress;
  uint32_t ulAccessType;
  uint32_t ulReadLength;

} HIL_PHYSMEM_READ_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PHYSMEM_READ_REQ_Ttag
{
  HIL_PACKET_HEADER_T               tHead;
  HIL_PHYSMEM_READ_REQ_DATA_T       tData;
} HIL_PHYSMEM_READ_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PHYSMEM_READ_CNF_Ttag
{
  HIL_PACKET_HEADER_T               tHead;
} HIL_PHYSMEM_READ_CNF_T;

/******************************************************************************
 * Packet: HIL_PHYSMEM_WRITE_REQ/HIL_PHYSMEM_WRITE_CNF
 *
 *          This packet allows write accesss to physical memory area
 */

/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PHYSMEM_WRITE_REQ_DATA_Ttag
{
  uint32_t ulPhysicalAddress;
  uint32_t ulAccessType;
} HIL_PHYSMEM_WRITE_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PHYSMEM_WRITE_REQ_Ttag
{
  HIL_PACKET_HEADER_T                tHead;
  HIL_PHYSMEM_WRITE_REQ_DATA_T       tData;
} HIL_PHYSMEM_WRITE_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PHYSMEM_WRITE_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;
} HIL_PHYSMEM_WRITE_CNF_T;


/******************************************************************************
 * Packet: HIL_MODULE_INSTANTIATE_REQ/HIL_MODULE_INSTANTIATE_CNF
 *
 *          This packet allows starting of a firmware module for a given channel
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODULE_INSTANTIATE_REQ_DATA_Ttag
{
  HIL_TASK_UID_T                      tModuleUuid;            /* packet header */
  uint32_t                            ulInst;                 /* packet data */
} HIL_MODULE_INSTANTIATE_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODULE_INSTANTIATE_REQ_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                  /* packet header */
  HIL_MODULE_INSTANTIATE_REQ_DATA_T     tData;                  /* packet data */
} HIL_MODULE_INSTANTIATE_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODULE_INSTANTIATE_CNF_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                  /* packet header */
} HIL_MODULE_INSTANTIATE_CNF_T;


/******************************************************************************
 * Packet: HIL_MODULE_CHANNEL_REQ/HIL_MODULE_CHANNEL_CNF
 *
 *          This packet allows starting of a previously downloaded firmware module for a given channel
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_CHANNEL_INSTANTIATE_REQ_DATA_Ttag
{
  uint32_t                            ulChannelNo;            /* channel number */
} HIL_CHANNEL_INSTANTIATE_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_CHANNEL_INSTANTIATE_REQ_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                  /* packet header */
  HIL_CHANNEL_INSTANTIATE_REQ_DATA_T    tData;                  /* packet data */
} HIL_CHANNEL_INSTANTIATE_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_CHANNEL_INSTANTIATE_CNF_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                  /* packet header */
} HIL_CHANNEL_INSTANTIATE_CNF_T;


/******************************************************************************
 * Packet: HIL_MODULE_GET_INFO_IDX_REQ/HIL_MODULE_GET_INFO_IDX_CNF
 *
 *          This packet reads out the current available module information from the
 *          module table
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODULE_GET_INFO_IDX_REQ_DATA_Ttag
{
  uint32_t                            ulIdx;                  /* module table index */
} HIL_MODULE_GET_INFO_IDX_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODULE_GET_INFO_IDX_REQ_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                  /* packet header */
  HIL_MODULE_GET_INFO_IDX_REQ_DATA_T    tData;                  /* packet data */
} HIL_MODULE_GET_INFO_IDX_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODULE_GET_INFO_IDX_CNF_DATA_Ttag
{
  /* index in module table */
  uint32_t                            ulIdx;
  /* module uuid */
  HIL_UUID_T                          tModuleUuid;
  /* init table address */
  uint32_t                            ulInitTableAddress;
  /* code block start address */
  uint32_t                            ulCodeBlockStart;
  /* data block start address */
  uint32_t                            ulDataBlockStart;
  /* bss block start address */
  uint32_t                            ulBssBlockStart;
  /* GOT start address */
  uint32_t                            ulGOT;
  /* PLT start address */
  uint32_t                            ulPLT;
  /* task associated with module */
  uint32_t                            ulNumTasks;
  /* static task table start */
  uint32_t                            ulStaticTaskTableStart;
  /* debug table start */
  uint32_t                            ulDebugTableStart;
  uint32_t                            ulDebugTableSize;
  /* export table start */
  uint32_t                            ulExportTableStart;
  uint32_t                            ulExportTableEntries;
} HIL_MODULE_GET_INFO_IDX_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODULE_GET_INFO_IDX_CNF_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                  /* packet header */
  HIL_MODULE_GET_INFO_IDX_CNF_DATA_T    tData;                  /* packet data */
} HIL_MODULE_GET_INFO_IDX_CNF_T;


/******************************************************************************
 * Packet: HIL_TSK_GET_STATUS_ARRAY_REQ/HIL_TSK_GET_STATUS_ARRAY_CNF
 *
 *          This function reads out the task status' in an indexed way
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_STATUS_ARRAY_REQ_DATA_Ttag
{
  uint32_t                            ulStartIndex;           /* start index of requested list */
} HIL_TSK_GET_STATUS_ARRAY_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_STATUS_ARRAY_REQ_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                  /* packet header */
  HIL_TSK_GET_STATUS_ARRAY_REQ_DATA_T   tData;                  /* packet data */
} HIL_TSK_GET_STATUS_ARRAY_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_STATUS_ARRAY_CNF_DATA_Ttag
{
  uint32_t                            ulStartIndex;           /* start index */
  uint32_t                            ulNumberOfEntries;      /* number of entries (declaring range of start to start+count-1) */
  uint32_t                            ulCurrentTskCount;      /* current number of tasks */
  uint32_t                            aulStatusCodes[1];      /* ATTENTION: This is a placeholder for the status codes
                                                                  The packet must be allocated in correct size*/
} HIL_TSK_GET_STATUS_ARRAY_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TSK_GET_STATUS_ARRAY_CNF_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                  /* packet header */
  HIL_TSK_GET_STATUS_ARRAY_CNF_DATA_T   tData;                  /* packet data */
} HIL_TSK_GET_STATUS_ARRAY_CNF_T;


/******************************************************************************
 * Packet: HIL_DPM_GET_BLOCK_INFO_REQ/HIL_DPM_GET_BLOCK_INFO_CNF
 *
 *          This function retrieves the DPM Channel Block information
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_BLOCK_INFO_REQ_DATA_Ttag
{
  uint32_t                            ulAreaIndex;            /* area index */
  uint32_t                            ulSubblockIndex;        /* subblock index */
} HIL_DPM_GET_BLOCK_INFO_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_BLOCK_INFO_REQ_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                  /* packet header */
  HIL_DPM_GET_BLOCK_INFO_REQ_DATA_T     tData;                  /* packet data */
} HIL_DPM_GET_BLOCK_INFO_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_BLOCK_INFO_CNF_DATA_Ttag
{
  uint32_t                            ulAreaIndex;            /* area index */
  uint32_t                            ulSubblockIndex;        /* number of subblock to request data of */
  uint32_t                            ulType;                 /* Type of subblock */
  uint32_t                            ulOffset;               /* Relative Offset of this Subblock within the Area */
  uint32_t                            ulSize;                 /* Size of the Subblock */
  uint16_t                            usFlags;                /* flags of the subblock */
  uint16_t                            usHandshakeMode;        /* Handshake Mode */
  uint16_t                            usHandshakeBit;         /* Bit position in the Handshake register */
  uint16_t                            usReserved;             /* res */
} HIL_DPM_GET_BLOCK_INFO_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_BLOCK_INFO_CNF_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                  /* packet header */
  HIL_DPM_GET_BLOCK_INFO_CNF_DATA_T     tData;                  /* packet data */
} HIL_DPM_GET_BLOCK_INFO_CNF_T;


/******************************************************************************
 * Packet: HIL_DPM_GET_COMFLAG_INFO_REQ/HIL_DPM_GET_COMFLAG_INFO_CNF
 *
 *          This packet retrieves the currently set COM Flags
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_COMFLAG_INFO_REQ_DATA_Ttag
{
  uint32_t                            ulAreaIndex;            /* area index */
} HIL_DPM_GET_COMFLAG_INFO_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_COMFLAG_INFO_REQ_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                  /* packet header */
  HIL_DPM_GET_COMFLAG_INFO_REQ_DATA_T   tData;                  /* packet data */
} HIL_DPM_GET_COMFLAG_INFO_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_COMFLAG_INFO_CNF_DATA_Ttag
{
  uint32_t                            ulAreaIndex;              /* area index */
  uint32_t                            ulNetxComFlag;
  uint32_t                            ulHostComFlag;
} HIL_DPM_GET_COMFLAG_INFO_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_COMFLAG_INFO_CNF_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                    /* packet header */
  HIL_DPM_GET_COMFLAG_INFO_CNF_DATA_T   tData;                    /* packet data */
} HIL_DPM_GET_COMFLAG_INFO_CNF_T;


/******************************************************************************
 * Packet: HIL_LISTS_GET_NUM_ENTRIES_REQ/HIL_LISTS_GET_NUM_ENTRIES_CNF
 *
 *          This function retrieves the number of tasks and queues available on
 *          the system.
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_LISTS_GET_NUM_ENTRIES_REQ_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                    /* packet header */
} HIL_LISTS_GET_NUM_ENTRIES_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_LISTS_GET_NUM_ENTRIES_CNF_DATA_Ttag
{
  uint32_t                            ulNumTasks;               /* number of tasks */
  uint32_t                            ulNumQueues;              /* number of queues */
} HIL_LISTS_GET_NUM_ENTRIES_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_LISTS_GET_NUM_ENTRIES_CNF_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                    /* packet header */
  HIL_LISTS_GET_NUM_ENTRIES_CNF_DATA_T  tData;                    /* packet data */
} HIL_LISTS_GET_NUM_ENTRIES_CNF_T;



/******************************************************************************
 * Packet: HIL_FIRMWARE_IDENTIFY_REQ/HIL_FIRMWARE_IDENTIFY_CNF
 *
 *          This function identifies the currently running firmware on a given channel
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FIRMWARE_IDENTIFY_REQ_DATA_Ttag
{
  uint32_t                            ulChannelId;              /* channel id */
} HIL_FIRMWARE_IDENTIFY_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FIRMWARE_IDENTIFY_REQ_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                    /* packet header */
  HIL_FIRMWARE_IDENTIFY_REQ_DATA_T      tData;                    /* packet data */
} HIL_FIRMWARE_IDENTIFY_REQ_T;

#define HIL_FIRMWARE_IDENTIFY_SYSTEM  0xFFFFFFFF

/***** confirmation packet *****/

/*****************************************************************************/
/*! Firmware Identification Structure                                        */
/*****************************************************************************/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FIRMWARE_IDENTIFY_CNF_DATA_Ttag
{
  HIL_FW_IDENTIFICATION_T               tFirmwareIdentification;  /* firmware identification */
} HIL_FIRMWARE_IDENTIFY_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FIRMWARE_IDENTIFY_CNF_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;                    /* packet header */
  HIL_FIRMWARE_IDENTIFY_CNF_DATA_T      tData;                    /* packet data */
} HIL_FIRMWARE_IDENTIFY_CNF_T;


/******************************************************************************
 * Packet: HIL_HW_IDENTIFY_REQ/HIL_HW_IDENTIFY_CNF
 *
 *          This function retrieves the hardware identification
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HW_IDENTIFY_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
} HIL_HW_IDENTIFY_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HW_IDENTIFY_CNF_DATA_Ttag
{
  uint32_t                              ulDeviceNumber;         /* device number */
  uint32_t                              ulSerialNumber;         /* serial number */
  uint16_t                              ausHwOptions[4];        /* hardware options */
  uint16_t                              usDeviceClass;          /* device class */
  uint8_t                               bHwRevision;            /* hardware revision */
  uint8_t                               bHwCompatibility;       /* hardware compatibility */
  uint32_t                              ulBootType;             /* how did the device boot up */
  uint32_t                              ulChipTyp;              /* chip typ */
  uint32_t                              ulChipStep;             /* chip step */
  uint32_t                              ulRomcodeRevision;      /* romcode revision */
} HIL_HW_IDENTIFY_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HW_IDENTIFY_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_HW_IDENTIFY_CNF_DATA_T              tData;                  /* packet data */
} HIL_HW_IDENTIFY_CNF_T;






/**************************************************************************************************
 * file transfer packets
 */

#define HIL_FILE_XFER_INVALID                   0
#define HIL_FILE_XFER_FILESYSTEM                1           /* transfer is related to the Filesystem installed in the Firmware (i.e. 2nd stage bootloader etc.) */
#define HIL_FILE_XFER_FILE HIL_FILE_XFER_FILESYSTEM
#define HIL_FILE_XFER_MODULE                    2           /* will be directly loaded into ram and relocated to be integrated in the running firmware */
#define HIL_FILE_XFER_PARALLEL_FLASH            3           /* flasher interface */
#define HIL_FILE_XFER_SERIAL_FLASH              4           /* flasher interface */
#define HIL_FILE_XFER_LICENSE_CODE              5           /* license code interface */

#define HIL_FILE_CHANNEL_0 (0)
#define HIL_FILE_CHANNEL_1 (1)
#define HIL_FILE_CHANNEL_2 (2)
#define HIL_FILE_CHANNEL_3 (3)
#define HIL_FILE_CHANNEL_4 (4)
#define HIL_FILE_CHANNEL_5 (5)

#define HIL_FILE_SYSTEM (0xFFFFFFFF)


/******************************************************************************
 * Packet: HIL_FILE_UPLOAD_REQ/HIL_FILE_UPLOAD_CNF
 *
 *          This packet starts a file upload
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_UPLOAD_REQ_DATA_Ttag
{
  uint32_t                              ulXferType;
  uint32_t                              ulMaxBlockSize;
  uint32_t                              ulChannelNo;            /* 0 = Channel 0, ..., 3 = Channel 3, 0xFFFFFFFF = System, see HIL_FILE_xxxx */
  uint16_t                              usFileNameLength;       /* length of NUL-terminated file name that will follow */
  /* a NUL-terminated file name will follow here */
} HIL_FILE_UPLOAD_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_UPLOAD_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_FILE_UPLOAD_REQ_DATA_T              tData;                  /* packet data */
} HIL_FILE_UPLOAD_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_UPLOAD_CNF_DATA_Ttag
{
  uint32_t                              ulMaxBlockSize;         /* maximum block size possible */
  uint32_t                              ulFileLength;           /* file size to transfer */
} HIL_FILE_UPLOAD_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_UPLOAD_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_FILE_UPLOAD_CNF_DATA_T              tData;                  /* packet data */
} HIL_FILE_UPLOAD_CNF_T;


/******************************************************************************
 * Packet: HIL_FILE_UPLOAD_DATA_REQ/HIL_FILE_UPLOAD_DATA_CNF
 *
 *          This packet requests the data from a previously successful HIL_FILE_UPLOAD_REQ
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_UPLOAD_DATA_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
} HIL_FILE_UPLOAD_DATA_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_UPLOAD_DATA_CNF_DATA_Ttag
{
  uint32_t                              ulBlockNo;              /* block number starting from 0 in a download sequence */
  uint32_t                              ulChksum;               /* cumulative CRC-32 checksum */
  /* data block follows here */
} HIL_FILE_UPLOAD_DATA_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_UPLOAD_DATA_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_FILE_UPLOAD_DATA_CNF_DATA_T         tData;                  /* packet data */
} HIL_FILE_UPLOAD_DATA_CNF_T;


/******************************************************************************
 * Packet: HIL_FILE_UPLOAD_ABORT_REQ/HIL_FILE_UPLOAD_ABORT_CNF
 *
 *          This packet aborts a currently running file upload
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_UPLOAD_ABORT_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
} HIL_FILE_UPLOAD_ABORT_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_UPLOAD_ABORT_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
} HIL_FILE_UPLOAD_ABORT_CNF_T;



/******************************************************************************
 * Packet: HIL_FORMAT_REQ/HIL_FORMAT_CNF_T
 *
 *          Formats the default volume
 */
#define HIL_FORMAT_REQ_DATA_FLAGS_QUICKFORMAT 0x00000000
#define HIL_FORMAT_REQ_DATA_FLAGS_FULLFORMAT  0x00000001

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FORMAT_REQ_DATA_Ttag
{
  uint32_t  ulFlags;
  uint32_t  ulReserved;
  /* Future version may have the volume name starting here as NUL terminated string,
     Currently unsupported */
} HIL_FORMAT_REQ_DATA_T;

/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FORMAT_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_FORMAT_REQ_DATA_T                   tData;
} HIL_FORMAT_REQ_T;


/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FORMAT_CNF_DATA_Ttag
{
  /* Valid if format has failed during a full format with an error during
     erase / verify (ulSta = TLR_E_HIL_FORMAT_ERASE_FAILED or TLR_E_HIL_FORMAT_VERIFY_FAILED */
  uint32_t  ulExtendedErrorInfo;
  uint32_t  ulErrorOffset;
} HIL_FORMAT_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FORMAT_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_FORMAT_CNF_DATA_T                   tData;
} HIL_FORMAT_CNF_T;

/******************************************************************************
 * Packet: HIL_FILE_DOWNLOAD_REQ/HIL_FILE_DOWNLOAD_CNF
 *
 *          This packet starts a file download
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DOWNLOAD_REQ_DATA_Ttag
{
  uint32_t                              ulXferType;             /* transfer type */
  uint32_t                              ulMaxBlockSize;         /* maximum possible download size by requestor */
  uint32_t                              ulFileLength;           /* file size to download */
  uint32_t                              ulChannelNo;            /* 0 = Channel 0, ..., 3 = Channel 3, 0xFFFFFFFF = System, see HIL_FILE_xxxx */
  uint16_t                              usFileNameLength;       /* length of NUL-terminated file name that will follow */
  /* a NUL-terminated file name will follow here */
} HIL_FILE_DOWNLOAD_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DOWNLOAD_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_FILE_DOWNLOAD_REQ_DATA_T            tData;                  /* packet data */
} HIL_FILE_DOWNLOAD_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DOWNLOAD_CNF_DATA_Ttag
{
  uint32_t                              ulMaxBlockSize;         /* download block size selected */
} HIL_FILE_DOWNLOAD_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DOWNLOAD_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_FILE_DOWNLOAD_CNF_DATA_T            tData;                  /* packet data */
} HIL_FILE_DOWNLOAD_CNF_T;


/******************************************************************************
 * Packet: HIL_FILE_DOWNLOAD_DATA_REQ/HIL_FILE_DOWNLOAD_DATA_CNF
 *
 *          This packet transfers the file data of a previously successful HIL_DOWNLOAD_REQ
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DOWNLOAD_DATA_REQ_DATA_Ttag
{
  uint32_t                              ulBlockNo;              /* block number */
  uint32_t                              ulChksum;               /* cumulative CRC-32 checksum */
  /* data block follows here */
} HIL_FILE_DOWNLOAD_DATA_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DOWNLOAD_DATA_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_FILE_DOWNLOAD_DATA_REQ_DATA_T       tData;                  /* packet data */
} HIL_FILE_DOWNLOAD_DATA_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DOWNLOAD_DATA_CNF_DATA_Ttag
{
  uint32_t                              ulExpectedCrc32;        /* expected CRC-32 checksum */
} HIL_FILE_DOWNLOAD_DATA_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DOWNLOAD_DATA_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_FILE_DOWNLOAD_DATA_CNF_DATA_T       tData;                  /* packet data */
} HIL_FILE_DOWNLOAD_DATA_CNF_T;


/******************************************************************************
 * Packet: HIL_FILE_DOWNLOAD_ABORT_REQ/HIL_FILE_DOWNLOAD_ABORT_CNF
 *
 *          This packet aborts a currently running file download
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DOWNLOAD_ABORT_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
} HIL_FILE_DOWNLOAD_ABORT_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DOWNLOAD_ABORT_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
} HIL_FILE_DOWNLOAD_ABORT_CNF_T;


/******************************************************************************
 * Packet: HIL_FILE_GET_MD5_REQ/HIL_FILE_GET_MD5_CNF
 *
 *          This packet retrieves the MD5 sum of a given file
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_GET_MD5_REQ_DATA_Ttag
{
  uint32_t                              ulChannelNo;            /* 0 = Channel 0, ..., 3 = Channel 3, 0xFFFFFFFF = System, see HIL_FILE_xxxx */
  uint16_t                              usFileNameLength;       /* length of NUL-terminated file name that will follow */
  /* a NUL-terminated file name will follow here */
} HIL_FILE_GET_MD5_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_GET_MD5_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_FILE_GET_MD5_REQ_DATA_T             tData;                  /* packet data */
} HIL_FILE_GET_MD5_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_GET_MD5_CNF_DATA_Ttag
{
  uint8_t                               abMD5[16];              /* MD5 checksum */
} HIL_FILE_GET_MD5_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_GET_MD5_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_FILE_GET_MD5_CNF_DATA_T             tData;                  /* packet data */
} HIL_FILE_GET_MD5_CNF_T;

/* This packet has the same structure, so we are using a typedef here instead of copy and paste */
typedef HIL_FILE_GET_MD5_REQ_T  HIL_FILE_GET_HEADER_MD5_REQ_T;
typedef HIL_FILE_GET_MD5_CNF_T  HIL_FILE_GET_HEADER_MD5_CNF_T;

/******************************************************************************
 * Packet: HIL_FILE_DELETE_REQ/HIL_FILE_DELETE_CNF
 *
 *          This packet allows deleting a file on the system
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DELETE_REQ_DATA_Ttag
{
  uint32_t                              ulChannelNo;            /* 0 = Channel 0, ..., 3 = Channel 3, 0xFFFFFFFF = System, see HIL_FILE_xxxx */
  uint16_t                              usFileNameLength;       /* length of NUL-terminated file name that will follow */
  /* a NUL-terminated file name will follow here */
} HIL_FILE_DELETE_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DELETE_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_FILE_DELETE_REQ_DATA_T              tData;                  /* packet data */
} HIL_FILE_DELETE_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DELETE_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
} HIL_FILE_DELETE_CNF_T;

/******************************************************************************
 * Packet: HIL_FILE_RENAME_REQ/HIL_FILE_RENAME_CNF
 *
 *          This packet allows renaming a file on the system
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_RENAME_REQ_DATA_Ttag
{
  uint32_t                              ulChannelNo;            /* 0 = Channel 0, ..., 3 = Channel 3, 0xFFFFFFFF = System, see HIL_FILE_xxxx */
  uint16_t                              usOldNameLength;        /* length of NUL-terminated old file name that will follow */
  uint16_t                              usNewNameLength;        /* length of NUL-terminated new file name that will follow */
  /* a NUL-terminated file name will follow here */
} HIL_FILE_RENAME_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_RENAME_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_FILE_RENAME_REQ_DATA_T              tData;                  /* packet data */
} HIL_FILE_RENAME_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_RENAME_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
} HIL_FILE_RENAME_CNF_T;

/******************************************************************************
 * Packet: HIL_DIR_LIST_REQ/HIL_DIR_LIST_CNF
 *
 *          This packet retrieves a Directory Listing of a given directory
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DIR_LIST_REQ_DATA_Ttag
{
  uint32_t                              ulChannelNo;            /* 0 = Channel 0, ..., 3 = Channel 3, 0xFFFFFFFF = System, see HIL_FILE_xxxx */
  uint16_t                              usDirNameLength;        /* length of NUL-terminated file name that will follow */
  /* a NUL-terminated dir name will follow here */
} HIL_DIR_LIST_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DIR_LIST_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;
  HIL_DIR_LIST_REQ_DATA_T                 tData;
} HIL_DIR_LIST_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DIR_LIST_CNF_DATA_Ttag
{
  uint8_t                               szName[16];             /* file name */
  uint32_t                              ulFileSize;             /* file size */
  uint8_t                               bFileType;              /* file type */
  uint8_t                               bReserved;              /* reserved */
  uint16_t                              bReserved2;             /* reserved */
} HIL_DIR_LIST_CNF_DATA_T;

#define HIL_DIR_LIST_CNF_FILE_TYPE_DIRECTORY      1
#define HIL_DIR_LIST_CNF_FILE_TYPE_FILE           2

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DIR_LIST_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_DIR_LIST_CNF_DATA_T                 tData;                  /* packet data */
} HIL_DIR_LIST_CNF_T;

/******************************************************************************
 * Packet: HIL_VOLUME_GET_ENTRY_REQ/HIL_VOLUME_GET_ENTRY_CNF
 *
 *          This packet retrieves the volume information from the system by index
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_VOLUME_GET_ENTRY_REQ_DATA_Ttag
{
  uint32_t                              ulVolumeIndex;          /* volume entry table index */
} HIL_VOLUME_GET_ENTRY_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_VOLUME_GET_ENTRY_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_VOLUME_GET_ENTRY_REQ_DATA_T         tData;                  /* packet data */
} HIL_VOLUME_GET_ENTRY_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_VOLUME_GET_ENTRY_CNF_DATA_Ttag
{
  uint32_t                              ulVolumeCount;          /* count of volumes */
  uint8_t                               szName[16];             /* name of currently requested volume entry */
} HIL_VOLUME_GET_ENTRY_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_VOLUME_GET_ENTRY_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_VOLUME_GET_ENTRY_CNF_DATA_T         tData;                  /* packet data */
} HIL_VOLUME_GET_ENTRY_CNF_T;


/******************************************************************************
 * Packet: HIL_GET_COMMON_STATE_REQ/HIL_GET_COMMON_STATE_CNF
 *
 *          This packet retrieves the Common State Block of a given Channel in the DPM
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_COMMON_STS_BLOCK_REQ_DATA_Ttag
{
  uint32_t                              ulChannelId;
} HIL_READ_COMMON_STS_BLOCK_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_COMMON_STS_BLOCK_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;
  HIL_READ_COMMON_STS_BLOCK_REQ_DATA_T    tData;
} HIL_READ_COMMON_STS_BLOCK_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_COMMON_STS_BLOCK_CNF_DATA_Ttag
{
  HIL_DPM_COMMON_STATUS_BLOCK_T           tCommonStatus;
} HIL_READ_COMMON_STS_BLOCK_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_COMMON_STS_BLOCK_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;
  HIL_READ_COMMON_STS_BLOCK_CNF_DATA_T    tData;
} HIL_READ_COMMON_STS_BLOCK_CNF_T;


/**************************************
 * legacy definitions of HIL_READ_COMMON_STS_BLOCK_REQ_T packet for compatibility reasons
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_COMMON_STATE_REQ_DATA_Ttag
{
  uint32_t                              ulChannelIndex;         /* channel number */
} HIL_DPM_GET_COMMON_STATE_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_COMMON_STATE_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_DPM_GET_COMMON_STATE_REQ_DATA_T     tData;                  /* packet data */
} HIL_DPM_GET_COMMON_STATE_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_COMMON_STATE_CNF_DATA_Ttag
{
  uint8_t                               abData[64];             /* common status block data */
} HIL_DPM_GET_COMMON_STATE_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_COMMON_STATE_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_DPM_GET_COMMON_STATE_CNF_DATA_T     tData;                  /* packet data */
} HIL_DPM_GET_COMMON_STATE_CNF_T;



/******************************************************************************
 * Packet: HIL_GET_EXTENDED_STATE_REQ/HIL_GET_EXTENDED_STATE_CNF
 *
 *          This packet retrieves the Extended State Block of a given Channel in the DPM
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_EXTENDED_STATE_REQ_DATA_Ttag
{
  uint32_t                              ulOffset;               /* offset to start the reading from the extended status block */
  uint32_t                              ulDataLen;              /* size of block to be read from extended status block */
  uint32_t                              ulChannelIndex;         /* channel number from which to read the extended status block */
} HIL_DPM_GET_EXTENDED_STATE_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_EXTENDED_STATE_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_DPM_GET_EXTENDED_STATE_REQ_DATA_T   tData;                  /* packet data */
} HIL_DPM_GET_EXTENDED_STATE_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_EXTENDED_STATE_CNF_DATA_Ttag
{
  uint32_t                              ulOffset;               /* offset to start the reading from the extended status block */
  uint32_t                              ulDataLen;              /* size of block to be read from extended status block */
  uint8_t                               abData[432];            /* data block */
} HIL_DPM_GET_EXTENDED_STATE_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_GET_EXTENDED_STATE_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_DPM_GET_EXTENDED_STATE_CNF_DATA_T   tData;                  /* packet data */
} HIL_DPM_GET_EXTENDED_STATE_CNF_T;


/******************************************************************************
 * Packet: HIL_SECURITY_EEPROM_READ_REQ/HIL_SECURITY_EEPROM_READ_CNF
 *
 *          This packet allows reading the security eeprom
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SECURITY_EEPROM_READ_REQ_DATA_Ttag
{
  uint32_t                              ulZoneId;               /* zone id */
} HIL_SECURITY_EEPROM_READ_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SECURITY_EEPROM_READ_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_SECURITY_EEPROM_READ_REQ_DATA_T     tData;                  /* packet data */
} HIL_SECURITY_EEPROM_READ_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SECURITY_EEPROM_READ_CNF_DATA_Ttag
{
  uint8_t                               abZoneData[32];         /* zone data */
} HIL_SECURITY_EEPROM_READ_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SECURITY_EEPROM_READ_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_SECURITY_EEPROM_READ_CNF_DATA_T     tData;                  /* packet data */
} HIL_SECURITY_EEPROM_READ_CNF_T;

#define HIL_SECURITY_EEPROM_ZONE_0      0
#define HIL_SECURITY_EEPROM_ZONE_1      1
#define HIL_SECURITY_EEPROM_ZONE_2      2
#define HIL_SECURITY_EEPROM_ZONE_3      3


/******************************************************************************
 * Packet: HIL_SECURITY_EEPROM_WRITE_REQ/HIL_SECURITY_EEPROM_WRITE_CNF
 *
 *          This packet allows writing of the user zones in the security eeprom
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SECURITY_EEPROM_WRITE_REQ_DATA_Ttag
{
  uint32_t                              ulZoneId;               /* zone id , see HIL_SECURITY_EEPROM_ZONE_* defines */
  uint8_t                               abZoneData[32];         /* zone data */
} HIL_SECURITY_EEPROM_WRITE_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SECURITY_EEPROM_WRITE_REQ_Ttag
{
  /* Packet header */
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_SECURITY_EEPROM_WRITE_REQ_DATA_T    tData;                  /* packet data */
} HIL_SECURITY_EEPROM_WRITE_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SECURITY_EEPROM_WRITE_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
} HIL_SECURITY_EEPROM_WRITE_CNF_T;


/******************************************************************************
 * Packet: HIL_GET_LIB_VERSION_INFO_REQ/HIL_GET_LIB_VERSION_INFO_CNF
 *
 *          This packet allows retrieving the version information of libraries integrated
 *          into the firmware
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_LIB_VERSION_INFO_REQ_DATA_Ttag
{
  uint32_t                              ulVersionIndex;         /* version table index */
} HIL_GET_LIB_VERSION_INFO_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_LIB_VERSION_INFO_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_GET_LIB_VERSION_INFO_REQ_DATA_T     tData;                  /* packet data */
} HIL_GET_LIB_VERSION_INFO_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_LIB_VERSION_Ttag
{
  unsigned short                        usMajor;
  unsigned short                        usMinor;
  unsigned short                        usBuild;
  unsigned short                        usRevision;
} HIL_LIB_VERSION_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_LIB_NAME_Ttag
{
  unsigned char                         bNameLength;
  unsigned char                         abName[63];
} HIL_LIB_NAME_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_LIB_DATE_Ttag
{
  unsigned short                        usYear;
  unsigned char                         bMonth;
  unsigned char                         bDay;
} HIL_LIB_DATE_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_LIB_VERSION_INFO_CNF_DATA_Ttag
{
  HIL_LIB_VERSION_T                       tLibVersion;            /* !< library version */
  HIL_LIB_NAME_T                          tLibName;               /* !< library name    */
  HIL_LIB_DATE_T                          tLibDate;               /* !< library date    */
  uint32_t                              ulType;                 /* type of library */
} HIL_GET_LIB_VERSION_INFO_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_LIB_VERSION_INFO_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;                  /* packet header */
  HIL_GET_LIB_VERSION_INFO_CNF_DATA_T     tData;                  /* packet data */
} HIL_GET_LIB_VERSION_INFO_CNF_T;


/******************************************************************************
 * Packet: HIL_HW_LICENSE_INFO_REQ/HIL_HW_LICENSE_INFO_CNF
 *
 *          This packet allows retrieving the license information from the security memory
 */

/* request packet */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HW_LICENSE_INFO_REQ_Ttag
{
  HIL_PACKET_HEADER_T               tHead;
} HIL_HW_LICENSE_INFO_REQ_T;

/* confirmation packet */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HW_LICENSE_INFO_CNF_DATA_Ttag
{
  uint32_t                        ulLicenseFlags1;
  uint32_t                        ulLicenseFlags2;
  uint16_t                        usNetxLicenseID;
  uint16_t                        usNetxLicenseFlags;
} HIL_HW_LICENSE_INFO_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HW_LICENSE_INFO_CNF_Ttag
{
  HIL_PACKET_HEADER_T               tHead;
  HIL_HW_LICENSE_INFO_CNF_DATA_T    tData;
} HIL_HW_LICENSE_INFO_CNF_T;

/******************************************************************************
 * Packet: HIL_HW_HARDWARE_INFO_REQ/HIL_HW_HARDWARE_INFO_CNF
 *
 *          This packet allows retrieving hardware information
 */

/* request packet */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HW_HARDWARE_INFO_REQ_Ttag
{
  HIL_PACKET_HEADER_T               tHead;
} HIL_HW_HARDWARE_INFO_REQ_T;

/* confirmation packet */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HW_HARDWARE_INFO_CNF_DATA_Ttag
{
  /* device number */
  uint32_t                        ulDeviceNumber;
  /* serial number */
  uint32_t                        ulSerialNumber;
  /* hardware options */
  uint16_t                        ausHwOptions[4];
  /* manufacturer code */
  uint16_t                        usManufacturer;
  /* production date */
  uint16_t                        usProductionDate;
  /* license info */
  uint32_t                        ulLicenseFlags1;
  uint32_t                        ulLicenseFlags2;
  uint16_t                        usNetxLicenseID;
  uint16_t                        usNetxLicenseFlags;
  /* device class */
  uint16_t                        usDeviceClass;
  /* hardware revision */
  uint8_t                         bHwRevision;
  /* hardware compatibility */
  uint8_t                         bHwCompatibility;
  /* hardware features 1 */
  uint32_t                        ulHardwareFeatures1;
  /* hardware features 2 */
  uint32_t                        ulHardwareFeatures2;
  /* boot option */
  uint8_t                         bBootOption;
  /* reserved */
  uint8_t                         bReserved[11];
} HIL_HW_HARDWARE_INFO_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HW_HARDWARE_INFO_CNF_Ttag
{
  HIL_PACKET_HEADER_T               tHead;
  HIL_HW_HARDWARE_INFO_CNF_DATA_T   tData;
} HIL_HW_HARDWARE_INFO_CNF_T;

/*****************************************************************************
 * HIL_BACKUP_REQ/HIL_BACKUP_CNF
*****************************************************************************/

/*****************************************************************************
 *  This packet initiates a backup of the SYSVOLUME onto the given device
*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BACKUP_REQ_DATA_Ttag
{
  uint8_t    szBackupPoint[1]; /*!< NULL terminated string containing the path
                                                                              to backup to. This is just a placeholder, and the strucure needs to
                                                                              be extended by the number of characters of the backup point,
                                                                              e.g.  "SDMMC:/backup" */
} HIL_BACKUP_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BACKUP_REQ_Ttag
{
  HIL_PACKET_HEADER_T   tHead; /* packet header */
  HIL_BACKUP_REQ_DATA_T tData; /* packet data */
} HIL_BACKUP_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BACKUP_CNF_Ttag
{
  HIL_PACKET_HEADER_T tHead; /* packet header */
} HIL_BACKUP_CNF_T;

/*****************************************************************************
 * HIL_RESTORE_REQ/HIL_RESTORE_CNF
*****************************************************************************/

/*****************************************************************************
 *  This packet initiates a restore of the SYSVOLUME from the given device / path
*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_RESTORE_REQ_DATA_Ttag
{
  uint8_t    szRestorePoint[1]; /*!< NULL terminated string containing the path
                                                                                 to restore from. This is just a placeholder, and the strucure needs to
                                                                                 be extended by the number of characters of the restore point
                                                                                 e.g.  "SDMMC:/backup" */
} HIL_RESTORE_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_RESTORE_REQ_Ttag
{
  HIL_PACKET_HEADER_T    tHead; /* packet header */
  HIL_RESTORE_REQ_DATA_T tData; /* packet data */
} HIL_RESTORE_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_RESTORE_CNF_Ttag
{
  HIL_PACKET_HEADER_T tHead; /* packet header */
} HIL_RESTORE_CNF_T;


/******************************************************************************
 * HIL_SYSTEM_INFORMATION_BLOCK_REQ/HIL_SYSTEM_INFORMATION_BLOCK_CNF
 ******************************************************************************/

/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_SYS_INFO_BLOCK_REQ_Ttag
{
  HIL_PACKET_HEADER_T                         tHead;
} HIL_READ_SYS_INFO_BLOCK_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_SYS_INFO_BLOCK_CNF_DATA_Ttag
{
  HIL_DPM_SYSTEM_INFO_BLOCK_T                 tSystemInfo;
} HIL_READ_SYS_INFO_BLOCK_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_SYS_INFO_BLOCK_CNF_Ttag
{
  HIL_PACKET_HEADER_T                         tHead;
  HIL_READ_SYS_INFO_BLOCK_CNF_DATA_T          tData;
} HIL_READ_SYS_INFO_BLOCK_CNF_T;


/******************************************************************************
 * HIL_CHANNEL_INFORMATION_BLOCK_REQ/HIL_CHANNEL_INFORMATION_BLOCK_CNF
 ******************************************************************************/

/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_CHNL_INFO_BLOCK_REQ_DATA_Ttag
{
  uint32_t                                  ulChannelId;
} HIL_READ_CHNL_INFO_BLOCK_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_CHNL_INFO_BLOCK_REQ_Ttag
{
  HIL_PACKET_HEADER_T                         tHead;
  HIL_READ_CHNL_INFO_BLOCK_REQ_DATA_T         tData;
} HIL_READ_CHNL_INFO_BLOCK_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_CHNL_INFO_BLOCK_CNF_DATA_Ttag
{
  HIL_DPM_CHANNEL_INFO_BLOCK_T                  tChannelInfo;
} HIL_READ_CHNL_INFO_BLOCK_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_CHNL_INFO_BLOCK_CNF_Ttag
{
  HIL_PACKET_HEADER_T                         tHead;
  HIL_READ_CHNL_INFO_BLOCK_CNF_DATA_T         tData;
} HIL_READ_CHNL_INFO_BLOCK_CNF_T;


/******************************************************************************
 * HIL_SYSTEM_CONTROL_BLOCK_REQ/HIL_SYSTEM_CONTROL_BLOCK_CNF
 ******************************************************************************/

/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_SYS_CNTRL_BLOCK_REQ_Ttag
{
  HIL_PACKET_HEADER_T                         tHead;
} HIL_READ_SYS_CNTRL_BLOCK_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_SYS_CNTRL_BLOCK_CNF_DATA_Ttag
{
  HIL_DPM_SYSTEM_CONTROL_BLOCK_T              tSystemControl;
} HIL_READ_SYS_CNTRL_BLOCK_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_SYS_CNTRL_BLOCK_CNF_Ttag
{
  HIL_PACKET_HEADER_T                         tHead;
  HIL_READ_SYS_CNTRL_BLOCK_CNF_DATA_T         tData;
} HIL_READ_SYS_CNTRL_BLOCK_CNF_T;

/******************************************************************************
 * HIL_SYSTEM_STATUS_BLOCK_REQ/HIL_SYSTEM_STATUS_BLOCK_CNF
 ******************************************************************************/

/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_SYS_STATUS_BLOCK_REQ_Ttag
{
  HIL_PACKET_HEADER_T                         tHead;
} HIL_READ_SYS_STATUS_BLOCK_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_SYS_STATUS_BLOCK_CNF_DATA_Ttag
{
  HIL_DPM_SYSTEM_STATUS_BLOCK_T                 tSystemState;
} HIL_READ_SYS_STATUS_BLOCK_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_SYS_STATUS_BLOCK_CNF_Ttag
{
  HIL_PACKET_HEADER_T                         tHead;
  HIL_READ_SYS_STATUS_BLOCK_CNF_DATA_T        tData;
} HIL_READ_SYS_STATUS_BLOCK_CNF_T;


/******************************************************************************
 * HIL_CONTROL_BLOCK_REQ/HIL_CONTROL_BLOCK_CNF
 ******************************************************************************/

/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_COMM_CNTRL_BLOCK_REQ_DATA_Ttag
{
  uint32_t                                  ulChannelId;
} HIL_READ_COMM_CNTRL_BLOCK_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_COMM_CNTRL_BLOCK_REQ_Ttag
{
  HIL_PACKET_HEADER_T                         tHead;
  HIL_READ_COMM_CNTRL_BLOCK_REQ_DATA_T        tData;
} HIL_READ_COMM_CNTRL_BLOCK_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_COMM_CNTRL_BLOCK_CNF_DATA_Ttag
{
  HIL_DPM_CONTROL_BLOCK_T                       tControl;
} HIL_READ_COMM_CNTRL_BLOCK_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_READ_COMM_CNTRL_BLOCK_CNF_Ttag
{
  HIL_PACKET_HEADER_T                         tHead;
  HIL_READ_COMM_CNTRL_BLOCK_CNF_DATA_T        tData;
} HIL_READ_COMM_CNTRL_BLOCK_CNF_T;





/******************************************************************************
 * HIL_MALLINFO_REQ
 ******************************************************************************/
/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MALLINFO_REQ_Ttag
{
  HIL_PACKET_HEADER_T tHead;
} HIL_MALLINFO_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MALLINFO_CNF_DATA_Ttag
{
  /* values reported by mallinfo() call, see malloc documentation for further description */
  int32_t  arena;       /* total space allocated from system */
  int32_t  ordblks;     /* number of non-inuse chunks */
  int32_t  hblks;       /* number of mmapped regions */
  int32_t  hblkhd;      /* total space in mmapped regions */
  int32_t  uordblks;    /* total allocated space */
  int32_t  fordblks;    /* total non-inuse space */
  int32_t  keepcost;    /* top-most, releasable (via malloc_trim) space */
  uint32_t ulTotalHeap; /* Total Heap area size in bytes */
} HIL_MALLINFO_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MALLINFO_CNF_Ttag
{
  HIL_PACKET_HEADER_T tHead;
  HIL_MALLINFO_CNF_DATA_T tData;
} HIL_MALLINFO_CNF_T;


/******************************************************************************
 * Packet: HIL_SYSTEM_ERRORLOG_REQ/HIL_SYSTEM_ERRORLOG_CNF
 *
 *         This request handles multiple operations on the system error buffers
 */

/***** request packet *****/

/* Definition of error request commands */
#define HIL_SYSTEM_ERRORLOG_CMD_READINDEX    (0x1)
#define HIL_SYSTEM_ERRORLOG_CMD_READCOUNT    (0x2)
#define HIL_SYSTEM_ERRORLOG_CMD_CLEARBUFFERS (0x4)

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SYSTEM_ERRORLOG_REQ_DATA_Ttag
{
  uint32_t                              ulCommand;   /*!< See command defines above */
  uint32_t                              ulParameter; /*!< Additional parameters of command */
} HIL_SYSTEM_ERRORLOG_REQ_DATA_T;


typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SYSTEM_ERRORLOG_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;     /*!< packet header */
  HIL_SYSTEM_ERRORLOG_REQ_DATA_T          tData;     /*!< packet data */
} HIL_SYSTEM_ERRORLOG_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SYSTEM_ERRORLOG_CNF_DATA_Ttag
{
  uint32_t                              ulCommand;   /*!< Requested command */
  uint32_t                              ulResult;    /*!< Index or returning information of ulCommand */
  /* Here follows one HIL_SYSTEM_ERRORLOG_CNF_DATA_ELEMENT depending on ulCommand of request.
   * If available, ulLen in Header is set accordingly
   */
} HIL_SYSTEM_ERRORLOG_CNF_DATA_T;

/* Description string size (remaining space of the packet)
 * 124 bytes = Packet header + ((ulCommand + ulResult) + (ulTimeStamp + ulError)) + szDescription
 */
#define HIL_SYSTEM_ERRORLOG_STRING_LENGTH (HIL_DPM_SYSTEM_MAILBOX_MIN_SIZE - HIL_PACKET_HEADER_SIZE - sizeof(HIL_SYSTEM_ERRORLOG_CNF_DATA_T) - 2*sizeof(uint32_t))

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SYSTEM_ERRORLOG_CNF_DATA_ELEMENT_Ttag
{
  uint32_t ulTimeStamp;                                      /*!< Seconds since startup */
  uint32_t ulError;                                          /*!< Module specific error value */
  int8_t   szDescription[HIL_SYSTEM_ERRORLOG_STRING_LENGTH]; /*!< Description string, rest of available space */
} HIL_SYSTEM_ERRORLOG_CNF_DATA_ELEMENT_T;


typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SYSTEM_ERRORLOG_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;     /*!< packet header */
  HIL_SYSTEM_ERRORLOG_CNF_DATA_T          tData;     /*!< packet data */
} HIL_SYSTEM_ERRORLOG_CNF_T;


/**************************************************************************************************
 * device data provider packet API
 */

/* DDP datatype definitions */
#define HIL_DDP_SERVICE_DATATYPE_MASK                    (0xFFFFFFF0)
#define HIL_DDP_SERVICE_DATATYPE_OPTION_MASK             (0x0000000F)

#define HIL_DDP_SERVICE_DATATYPE_BASE_DEVICE_DATA        (0x00)

#define HIL_DDP_SERVICE_DATATYPE_MAC_ADDRESSES_APP       (0x10)

#define HIL_DDP_SERVICE_DATATYPE_MAC_ADDRESSES_COM       (0x20)

#define HIL_DDP_SERVICE_DATATYPE_USB_INFORMATION         (0x30)

#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_AREA      (0x40)
#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_AREA_0    (0x41)
#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_AREA_1    (0x42)
#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_AREA_2    (0x43)
#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_AREA_3    (0x44)
#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_AREA_4    (0x45)
#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_AREA_5    (0x46)
#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_AREA_6    (0x47)
#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_AREA_7    (0x48)
#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_AREA_8    (0x49)
#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_AREA_9    (0x4A)

#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_CHIP      (0x50)
#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_CHIP_0    (0x51)
#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_CHIP_1    (0x52)
#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_CHIP_2    (0x53)
#define HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_CHIP_3    (0x54)

#define HIL_DDP_SERVICE_DATATYPE_OEM_OPTIONS             (0x60)
#define HIL_DDP_SERVICE_DATATYPE_OEM_SERIALNUMBER        (0x61)
#define HIL_DDP_SERVICE_DATATYPE_OEM_ORDERNUMBER         (0x62)
#define HIL_DDP_SERVICE_DATATYPE_OEM_HARDWAREREVISION    (0x63)
#define HIL_DDP_SERVICE_DATATYPE_OEM_PRODUCTIONDATE      (0x64)
#define HIL_DDP_SERVICE_DATATYPE_OEM_VENDORDATA_0        (0x66) /* 80 Bytes payload */
#define HIL_DDP_SERVICE_DATATYPE_OEM_VENDORDATA_1        (0x67) /* 32 Bytes payload */

#define HIL_DDP_SERVICE_DATATYPE_STATE                   (0x70)

/* DDP number definitions, compare with values in DeviceProductionData.h */

#define HIL_DDP_SERVICE_DEFAULT_NAME_SIZE                (16)

#define HIL_DDP_SERVICE_MAC_APP_NUM                      (4)
#define HIL_DDP_SERVICE_MAC_COM_NUM                      (8)

#define HIL_DDP_SERVICE_FLASH_AREA_NUM                   (10)
#define HIL_DDP_SERVICE_FLASH_CHIP_NUM                   (4)

/* DDP state definitions. */
#define HIL_DDP_SERVICE_STATE_PASSIVE                    (0)
#define HIL_DDP_SERVICE_STATE_ACTIVE                     (1)

/* DDP service structures */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DDP_SERVICE_BASE_DEVICE_DATA_Ttag
{
  /* Members as defined in HIL_PRODUCT_DATA_BASIC_DEVICE_DATA_T of DeviceProductionData.h */
  uint16_t  usManufacturer;
  uint16_t  usDeviceClass;
  uint32_t  ulDeviceNumber;
  uint32_t  ulSerialNumber;
  uint8_t   bHwCompatibility;
  uint8_t   bHwRevision;
  uint16_t  usProductionDate;
} HIL_DDP_SERVICE_BASE_DEVICE_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DDP_SERVICE_MAC_ADDRESS_Ttag
{
  uint8_t   abMacAddress[6];
} HIL_DDP_SERVICE_MAC_ADDRESS_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DDP_SERVICE_USB_INFO_Ttag
{
  uint16_t  usUSBVendorID;                                       /*!< USB Device vendor ID (VID) */
  uint16_t  usUSBProductID;                                      /*!< USB Device product ID (PID) */
  uint8_t   abUSBVendorName[HIL_DDP_SERVICE_DEFAULT_NAME_SIZE];  /*!< USB Product name (Byte array) */
  uint8_t   abUSBProductName[HIL_DDP_SERVICE_DEFAULT_NAME_SIZE]; /*!< USB Product name string (Byte array) */
} HIL_DDP_SERVICE_USB_INFO_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DDP_SERVICE_LIBSTORAGE_AREA_Ttag
{
  /* Members as defined in HIL_PRODUCT_DATA_LIBSTORAGE_AREAS_T of DeviceProductionData.h */
  uint32_t  ulContentType;
  uint32_t  ulAreaStart;
  uint32_t  ulAreaSize;
  uint32_t  ulChipNumber;
  int8_t    szName[HIL_DDP_SERVICE_DEFAULT_NAME_SIZE];
  uint8_t   bAccessTyp;
} HIL_DDP_SERVICE_LIBSTORAGE_AREA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DDP_SERVICE_LIBSTORAGE_CHIP_Ttag
{
  /* Members as defined in HIL_PRODUCT_DATA_LIBSTORAGE_CHIPS_T of DeviceProductionData.h */
  uint32_t  ulChipNumber;
  int8_t    szFlashName[HIL_DDP_SERVICE_DEFAULT_NAME_SIZE];
  uint32_t  ulBlockSize;
  uint32_t  ulFlashSize;
  uint32_t  ulMaxEnduranceCycles;
} HIL_DDP_SERVICE_LIBSTORAGE_CHIP_T;

/******************************************************************************
 * Packet: HIL_DDP_SERVICE_GET_REQ/HIL_DDP_SERVICE_GET_CNF
 *
 *          Retrieve information about a device using Device Data Provider
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DDP_SERVICE_GET_REQ_DATA_Ttag
{
  uint32_t                            ulDataType; /*!< DDP_SERVICE_DATATYPE_* definitions */
} HIL_DDP_SERVICE_GET_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DDP_SERVICE_GET_REQ_Ttag
{
  HIL_PACKET_HEADER_T                 tHead;      /*!< packet header */
  HIL_DDP_SERVICE_GET_REQ_DATA_T      tData;      /*!< packet data */
} HIL_DDP_SERVICE_GET_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DDP_SERVICE_GET_CNF_DATA_Ttag
{
  uint32_t                             ulDataType;       /*!< DDP_SERVICE_DATATYPE_* definitions */
  union HIL_DDP_SERVICE_GET_DATATYPE_U
  {
    /* Fixed structures for specific ulDataType */
    HIL_DDP_SERVICE_BASE_DEVICE_DATA_T tBaseDeviceData;  /*!< HIL_DDP_SERVICE_DATATYPE_BASE_DEVICE_DATA */
    HIL_DDP_SERVICE_USB_INFO_T         tUSBInfo;         /*!< HIL_DDP_SERVICE_DATATYPE_USB_INFORMATION */
    HIL_DDP_SERVICE_LIBSTORAGE_AREA_T  tFlashArea;       /*!< HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_AREA_* */
    HIL_DDP_SERVICE_LIBSTORAGE_CHIP_T  tFlashChip;       /*!< HIL_DDP_SERVICE_DATATYPE_STORAGE_FLASH_CHIP_* */

    /* Members for multiple keys (ulDataType) */
    uint32_t                           ulValue;          /*!< Keys with 32bit values, e.g. OEM Option Flags */
    /* The following arrays are defined with maximum values.
       Actual valid or used length/sizes dependent on DataType and my be smaller. */
    int8_t                             szString[80];     /*!< Strings, e.g. OEM Serial Number; maximum 80 bytes (including NULL termination) */
    uint8_t                            abData[80];       /*!< Binary data, e.g. OEM Vendor Data; maximum 80 bytes */
    HIL_DDP_SERVICE_MAC_ADDRESS_T      atMacAddress[13]; /*!< HIL_DDP_SERVICE_DATATYPE_MAC_ADDRESSES_*; maximum 13 addresses (6bytes*13=78) */
  } uDataType;
} HIL_DDP_SERVICE_GET_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DDP_SERVICE_GET_CNF_Ttag
{
  HIL_PACKET_HEADER_T                 tHead;       /*!< packet header */
  HIL_DDP_SERVICE_GET_CNF_DATA_T      tData;       /*!< packet data */
} HIL_DDP_SERVICE_GET_CNF_T;


/******************************************************************************
 * Packet: HIL_DDP_SERVICE_SET_REQ/HIL_DDP_SERVICE_SET_CNF
 *
 *          Set information to Device Data Provider
 */

/***** request packet *****/

/* Set data structure equal to data structure of Get confirmation */
typedef HIL_DDP_SERVICE_GET_CNF_DATA_T HIL_DDP_SERVICE_SET_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DDP_SERVICE_SET_REQ_Ttag
{
  HIL_PACKET_HEADER_T                 tHead;       /*!< packet header */
  HIL_DDP_SERVICE_SET_REQ_DATA_T      tData;       /*!< packet data */
} HIL_DDP_SERVICE_SET_REQ_T;

/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DDP_SERVICE_SET_CNF_Ttag
{
  HIL_PACKET_HEADER_T                 tHead;       /*!< packet header */
} HIL_DDP_SERVICE_SET_CNF_T;


/******************************************************************************
 * Packet: HIL_EXCEPTION_INFO_REQ/HIL_EXCEPTION_INFO_CNF
 *
 *        This packet allows retrieving of the exception context
 */

#define HIL_EXCEPTION_TYPE_EXCEPTION    1
#define HIL_EXCEPTION_TYPE_THREAD       2
#define HIL_EXCEPTION_TYPE_INTERRUPT    3

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_EXCEPTION_INFO_REQ_Ttag
{
  HIL_PACKET_HEADER_T             tHead;
} HIL_EXCEPTION_INFO_REQ_T;


/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_EXCEPTION_INFO_CNF_DATA_Ttag
{
  uint32_t                        ulType;      /* State type:  exception, thread, interrupt */
  uint32_t                        ulVector;    /* Vector number */

  uint32_t                        aulR[11];    /* General purpose registers (R0..R10) */
  uint32_t                        ulFP;        /* Frame pointer (R11) */
  uint32_t                        ulIP;        /* Intra-procedure call scratch register (R12) */
  uint32_t                        ulSP;        /* Stack pointer (R13) */
  uint32_t                        ulLR;        /* Link register (R14) */
  uint32_t                        ulPC;        /* Program counter (R15) */
  uint32_t                        ulPSR;       /* Program status register (PSR/CPSR) */

  union
  {
    /* ARM/Cortex-R */
    struct
    {
      uint32_t                    ulDFSR;      /* Data fault status register */
      uint32_t                    ulDFAR;      /* Data fault address register */
    } arm;

    /* Cortex-M */
    struct
    {
      uint32_t                    ulXLR;       /* Exception return LR (Cortex-M) */
      uint32_t                    ulBASEPRI;   /* Base priority level (Cortex-M) */
    } cm;
  } u;
} HIL_EXCEPTION_INFO_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_EXCEPTION_INFO_CNF_Ttag
{
  HIL_PACKET_HEADER_T             tHead;
  HIL_EXCEPTION_INFO_CNF_DATA_T   tData;
} HIL_EXCEPTION_INFO_CNF_T;



/* Deprecated defines with wrong spelling, shall not be used anymore */
#define HIL_DDP_SERVICE_DATATYPE_OEM_VEDORDATA_0         HIL_DDP_SERVICE_DATATYPE_OEM_VENDORDATA_0
#define HIL_DDP_SERVICE_DATATYPE_OEM_VEDORDATA_1         HIL_DDP_SERVICE_DATATYPE_OEM_VENDORDATA_1

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_UNPACK_1(HIL_SYSTEMCMD)
#endif


#endif /* HIL_SYSTEMCMD_H_ */
