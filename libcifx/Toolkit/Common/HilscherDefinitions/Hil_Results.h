/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_Results.h $: *//*!

  \file Hil_Results.h

  Common error code definitions.

**************************************************************************************/
#ifndef HIL_RESULTS_H_
#define HIL_RESULTS_H_

#include<stdint.h>

/***********************************************************************************
  Common status codes
************************************************************************************/

/* MessageId: SUCCESS_HIL_OK */
/* MessageText: Operation succeeded. */
#define SUCCESS_HIL_OK                   ((uint32_t)0x00000000L)

/* MessageId: ERR_HIL_FAIL */
/* MessageText: Common error, detailed error information optionally present in the data area of packet. */
#define ERR_HIL_FAIL                     ((uint32_t)0xC0000001L)

/* MessageId: ERR_HIL_UNEXPECTED */
/* MessageText: Unexpected failure. */
#define ERR_HIL_UNEXPECTED               ((uint32_t)0xC0000002L)

/* MessageId: ERR_HIL_OUTOFMEMORY */
/* MessageText: Ran out of memory. */
#define ERR_HIL_OUTOFMEMORY              ((uint32_t)0xC0000003L)

/***********************************************************************************
  Packet Related Results
************************************************************************************/

/* MessageId: ERR_HIL_UNKNOWN_COMMAND */
/* MessageText: Unknown Command in Packet received. */
#define ERR_HIL_UNKNOWN_COMMAND          ((uint32_t)0xC0000004L)

/* MessageId: ERR_HIL_UNKNOWN_DESTINATION */
/* MessageText: Unknown Destination in Packet received. */
#define ERR_HIL_UNKNOWN_DESTINATION      ((uint32_t)0xC0000005L)

/* MessageId: ERR_HIL_UNKNOWN_DESTINATION_ID */
/* MessageText: Unknown Destination Id in Packet received. */
#define ERR_HIL_UNKNOWN_DESTINATION_ID   ((uint32_t)0xC0000006L)

/* MessageId: ERR_HIL_INVALID_PACKET_LEN */
/* MessageText: Packet length is invalid. */
#define ERR_HIL_INVALID_PACKET_LEN       ((uint32_t)0xC0000007L)

/* MessageId: ERR_HIL_INVALID_EXTENSION */
/* MessageText: Invalid Extension in Packet received. */
#define ERR_HIL_INVALID_EXTENSION        ((uint32_t)0xC0000008L)

/* MessageId: ERR_HIL_INVALID_PARAMETER */
/* MessageText: Invalid Parameter in Packet found. */
#define ERR_HIL_INVALID_PARAMETER        ((uint32_t)0xC0000009L)

/* MessageId: ERR_HIL_INVALID_ALIGNMENT */
/* MessageText: Invalid alignment. */
#define ERR_HIL_INVALID_ALIGNMENT        ((uint32_t)0xC000000AL)

/* MessageId: 0xC000000BL (No symbolic name defined) */
/* MessageText: Missing description. */


/* MessageId: ERR_HIL_WATCHDOG_TIMEOUT */
/* MessageText: Watchdog error occurred. */
#define ERR_HIL_WATCHDOG_TIMEOUT         ((uint32_t)0xC000000CL)

/* MessageId: ERR_HIL_INVALID_LIST_TYPE */
/* MessageText: List type is invalid. */
#define ERR_HIL_INVALID_LIST_TYPE        ((uint32_t)0xC000000DL)

/* MessageId: ERR_HIL_UNKNOWN_HANDLE */
/* MessageText: Handle is unknown. */
#define ERR_HIL_UNKNOWN_HANDLE           ((uint32_t)0xC000000EL)

/* MessageId: ERR_HIL_PACKET_OUT_OF_SEQ */
/* MessageText: A packet index has been not in the expected sequence. */
#define ERR_HIL_PACKET_OUT_OF_SEQ        ((uint32_t)0xC000000FL)

/* MessageId: ERR_HIL_PACKET_OUT_OF_MEMORY */
/* MessageText: The amount of fragmented data contained the packet sequence has been too large. */
#define ERR_HIL_PACKET_OUT_OF_MEMORY     ((uint32_t)0xC0000010L)

/* MessageId: ERR_HIL_QUE_PACKETDONE */
/* MessageText: The packet done function has failed. */
#define ERR_HIL_QUE_PACKETDONE           ((uint32_t)0xC0000011L)

/* MessageId: ERR_HIL_QUE_SENDPACKET */
/* MessageText: The sending of a packet has failed. */
#define ERR_HIL_QUE_SENDPACKET           ((uint32_t)0xC0000012L)

/* MessageId: ERR_HIL_POOL_PACKET_GET */
/* MessageText: The request of a packet from packet pool has failed. */
#define ERR_HIL_POOL_PACKET_GET          ((uint32_t)0xC0000013L)

/* MessageId: ERR_HIL_POOL_PACKET_RELEASE */
/* MessageText: The release of a packet to packet pool has failed. */
#define ERR_HIL_POOL_PACKET_RELEASE      ((uint32_t)0xC0000014L)

/* MessageId: ERR_HIL_POOL_GET_LOAD */
/* MessageText: The get packet pool load function has failed. */
#define ERR_HIL_POOL_GET_LOAD            ((uint32_t)0xC0000015L)

/* MessageId: ERR_HIL_QUE_GET_LOAD */
/* MessageText: The get queue load function has failed. */
#define ERR_HIL_QUE_GET_LOAD             ((uint32_t)0xC0000016L)

/* MessageId: ERR_HIL_QUE_WAITFORPACKET */
/* MessageText: The waiting for a packet from queue has failed. */
#define ERR_HIL_QUE_WAITFORPACKET        ((uint32_t)0xC0000017L)

/* MessageId: ERR_HIL_QUE_POSTPACKET */
/* MessageText: The posting of a packet has failed. */
#define ERR_HIL_QUE_POSTPACKET           ((uint32_t)0xC0000018L)

/* MessageId: ERR_HIL_QUE_PEEKPACKET */
/* MessageText: The peek of a packet from queue has failed. */
#define ERR_HIL_QUE_PEEKPACKET           ((uint32_t)0xC0000019L)

/* MessageId: ERR_HIL_REQUEST_RUNNING */
/* MessageText: Request is already running. */
#define ERR_HIL_REQUEST_RUNNING          ((uint32_t)0xC000001AL)

/* MessageId: ERR_HIL_CREATE_TIMER */
/* MessageText: Creating a timer failed. */
#define ERR_HIL_CREATE_TIMER             ((uint32_t)0xC000001BL)

/* MessageId: ERR_HIL_BUFFER_TOO_SHORT */
/* MessageText: Supplied buffer too short for the data. */
#define ERR_HIL_BUFFER_TOO_SHORT         ((uint32_t)0xC000001CL)

/* MessageId: ERR_HIL_NAME_ALREADY_EXIST */
/* MessageText: Supplied name alreday exists. */
#define ERR_HIL_NAME_ALREADY_EXIST       ((uint32_t)0xC000001DL)

/* MessageId: ERR_HIL_PACKET_FRAGMENTATION_TIMEOUT */
/* MessageText: The packet fragmentation has timed out. */
#define ERR_HIL_PACKET_FRAGMENTATION_TIMEOUT ((uint32_t)0xC000001EL)

/***********************************************************************************
  General Protocol Stack Results
************************************************************************************/

/* MessageId: ERR_HIL_INIT_FAULT */
/* MessageText: General initialization fault. */
#define ERR_HIL_INIT_FAULT               ((uint32_t)0xC0000100L)

/* MessageId: ERR_HIL_DATABASE_ACCESS_FAILED */
/* MessageText: Database access failure. */
#define ERR_HIL_DATABASE_ACCESS_FAILED   ((uint32_t)0xC0000101L)

/* MessageId: ERR_HIL_CIR_MASTER_PARAMETER_FAILED */
/* MessageText: Master parameter can not activated at state operate. */
#define ERR_HIL_CIR_MASTER_PARAMETER_FAILED ((uint32_t)0xC0000102L)

/* MessageId: ERR_HIL_CIR_SLAVE_PARAMTER_FAILED */
/* MessageText: Slave parameter can not activated at state operate. */
#define ERR_HIL_CIR_SLAVE_PARAMTER_FAILED ((uint32_t)0xC0000103L)

/* MessageId: ERR_HIL_NOT_CONFIGURED */
/* MessageText: Configuration not available */
#define ERR_HIL_NOT_CONFIGURED           ((uint32_t)0xC0000119L)

/* MessageId: ERR_HIL_CONFIGURATION_FAULT */
/* MessageText: General configuration fault. */
#define ERR_HIL_CONFIGURATION_FAULT      ((uint32_t)0xC0000120L)

/* MessageId: ERR_HIL_INCONSISTENT_DATA_SET */
/* MessageText: Inconsistent configuration data. */
#define ERR_HIL_INCONSISTENT_DATA_SET    ((uint32_t)0xC0000121L)

/* MessageId: ERR_HIL_DATA_SET_MISMATCH */
/* MessageText: Configuration data set mismatch. */
#define ERR_HIL_DATA_SET_MISMATCH        ((uint32_t)0xC0000122L)

/* MessageId: ERR_HIL_INSUFFICIENT_LICENSE */
/* MessageText: Insufficient license. */
#define ERR_HIL_INSUFFICIENT_LICENSE     ((uint32_t)0xC0000123L)

/* MessageId: ERR_HIL_PARAMETER_ERROR */
/* MessageText: Parameter error. */
#define ERR_HIL_PARAMETER_ERROR          ((uint32_t)0xC0000124L)

/* MessageId: ERR_HIL_INVALID_NETWORK_ADDRESS */
/* MessageText: Network address invalid. */
#define ERR_HIL_INVALID_NETWORK_ADDRESS  ((uint32_t)0xC0000125L)

/* MessageId: ERR_HIL_NO_SECURITY_MEMORY */
/* MessageText: Security memory chip missing or broken. */
#define ERR_HIL_NO_SECURITY_MEMORY       ((uint32_t)0xC0000126L)

/* MessageId: ERR_HIL_NO_MAC_ADDRESS_AVAILABLE */
/* MessageText: no MAC address available. */
#define ERR_HIL_NO_MAC_ADDRESS_AVAILABLE ((uint32_t)0xC0000127L)

/* MessageId: ERR_HIL_INVALID_DDP_CONTENT */
/* MessageText: DeviceDataProvider contains invalid data. */
#define ERR_HIL_INVALID_DDP_CONTENT ((uint32_t)0xC0000128L)

/* MessageId: ERR_HIL_FIRMWARE_STARTUP_ERROR */
/* MessageText: Firmware startup failed. Check System logbook for details. */
#define ERR_HIL_FIRMWARE_STARTUP_ERROR ((uint32_t)0xC0000129L)

/* MessageId: ERR_HIL_COMM_CHANNEL_STARTUP_ERROR */
/* MessageText: Communication Channel startup failed. Check Communication Channel logbook for details. */
#define ERR_HIL_COMM_CHANNEL_STARTUP_ERROR ((uint32_t)0xC000012AL)

/* MessageId: ERR_HIL_FIRMWARE_SPECIFIC_STARTUP_FAILED */
/* MessageText: An error occurred while starting firmware or protocol specific functionality. */
#define ERR_HIL_FIRMWARE_SPECIFIC_STARTUP_FAILED ((uint32_t)0xC000012BL)

/* MessageId: ERR_HIL_INVALID_TAGLIST_CONTENT */
/* MessageText: While evaluating the firmware taglist an invalid taglist parameter was detected. */
#define ERR_HIL_INVALID_TAGLIST_CONTENT ((uint32_t)0xC000012CL)

/* MessageId: ERR_HIL_OPERATION_NOT_POSSIBLE_IN_CURRENT_STATE */
/* MessageText: The requested operation can not be executed in current state. */
#define ERR_HIL_OPERATION_NOT_POSSIBLE_IN_CURRENT_STATE ((uint32_t)0xC000012DL)

/* MessageId: ERR_HIL_REMANENT_DATA_MISSING */
/* MessageText: The requested operation can not be executed because remanent data was not set correctly. */
#define ERR_HIL_REMANENT_DATA_MISSING    ((uint32_t)0xC000012EL)

/* MessageId: ERR_HIL_INVALID_DDP_OEM_SERIALNUMBER_CODING */
/* MessageText: The content of DDPs OEM field SerialNumber can not be converted for usage with current protocol stack. */
#define ERR_HIL_INVALID_DDP_OEM_SERIALNUMBER_CODING    ((uint32_t)0xC000012FL)

/* MessageId: ERR_HIL_INVALID_DDP_OEM_ORDERNUMBER_CODING */
/* MessageText: The content of DDPs OEM field OrderNumber can not be converted for usage with current protocol stack. */
#define ERR_HIL_INVALID_DDP_OEM_ORDERNUMBER_CODING    ((uint32_t)0xC0000130L)

/* MessageId: ERR_HIL_INVALID_DDP_OEM_HARDWAREREVISION_CODING */
/* MessageText: The content of DDPs OEM field HardwareRevision can not be converted for usage with current protocol stack. */
#define ERR_HIL_INVALID_DDP_OEM_HARDWAREREVISION_CODING    ((uint32_t)0xC0000131L)

/* MessageId: ERR_HIL_INVALID_DDP_OEM_PRODUCTIONDATE_CODING */
/* MessageText: The content of DDPs OEM field ProductionDate can not be converted for usage with current protocol stack. */
#define ERR_HIL_INVALID_DDP_OEM_PRODUCTIONDATE_CODING    ((uint32_t)0xC0000132L)

/* MessageId: ERR_HIL_NETWORK_FAULT */
/* MessageText: General communication fault. */
#define ERR_HIL_NETWORK_FAULT            ((uint32_t)0xC0000140L)

/* MessageId: ERR_HIL_CONNECTION_CLOSED */
/* MessageText: Connection closed. */
#define ERR_HIL_CONNECTION_CLOSED        ((uint32_t)0xC0000141L)

/* MessageId: ERR_HIL_CONNECTION_TIMEOUT */
/* MessageText: Connection timeout. */
#define ERR_HIL_CONNECTION_TIMEOUT       ((uint32_t)0xC0000142L)

/* MessageId: ERR_HIL_LONELY_NETWORK */
/* MessageText: Lonely network. */
#define ERR_HIL_LONELY_NETWORK           ((uint32_t)0xC0000143L)

/* MessageId: ERR_HIL_DUPLICATE_NODE */
/* MessageText: Duplicate network address. */
#define ERR_HIL_DUPLICATE_NODE           ((uint32_t)0xC0000144L)

/* MessageId: ERR_HIL_CABLE_DISCONNECT */
/* MessageText: Cable disconnected. */
#define ERR_HIL_CABLE_DISCONNECT         ((uint32_t)0xC0000145L)

/* MessageId: ERR_HIL_BUS_OFF */
/* MessageText: Bus Off flag is set. */
#define ERR_HIL_BUS_OFF                  ((uint32_t)0xC0000180L)

/* MessageId: ERR_HIL_CONFIG_LOCK */
/* MessageText: Changing configuration is not allowed. */
#define ERR_HIL_CONFIG_LOCK              ((uint32_t)0xC0000181L)

/* MessageId: ERR_HIL_APPLICATION_NOT_READY */
/* MessageText: Application is not at ready state. */
#define ERR_HIL_APPLICATION_NOT_READY    ((uint32_t)0xC0000182L)

/* MessageId: ERR_HIL_RESET_IN_PROCESS */
/* MessageText: Application is performing a reset. */
#define ERR_HIL_RESET_IN_PROCESS         ((uint32_t)0xC0000183L)

/* MessageId: ERR_HIL_UNSUPPORTED_PHY_REVISION */
/* MessageText: The Revision of integrated netX PHYs is not supported by firmware. */
#define ERR_HIL_UNSUPPORTED_PHY_REVISION ((uint32_t)0xC0000184L)

/* MessageId: ERR_HIL_WATCHDOG_TIME_INVALID */
/* MessageText: Watchdog time is out of range. */
#define ERR_HIL_WATCHDOG_TIME_INVALID    ((uint32_t)0xC0000200L)

/* MessageId: ERR_HIL_APPLICATION_ALREADY_REGISTERED */
/* MessageText: Application is already registered. */
#define ERR_HIL_APPLICATION_ALREADY_REGISTERED ((uint32_t)0xC0000201L)

/* MessageId: ERR_HIL_NO_APPLICATION_REGISTERED */
/* MessageText: No application registered. */
#define ERR_HIL_NO_APPLICATION_REGISTERED ((uint32_t)0xC0000202L)

/* MessageId: ERR_HIL_INVALID_COMPONENT_ID */
/* MessageText: Invalid component identifier. */
#define ERR_HIL_INVALID_COMPONENT_ID     ((uint32_t)0xC0000203L)

/* MessageId: ERR_HIL_INVALID_DATA_LENGTH */
/* MessageText: Invalid data length. */
#define ERR_HIL_INVALID_DATA_LENGTH      ((uint32_t)0xC0000204L)

/* MessageId: ERR_HIL_DATA_ALREADY_SET */
/* MessageText: The data was already set. */
#define ERR_HIL_DATA_ALREADY_SET         ((uint32_t)0xC0000205L)

/* MessageId: ERR_HIL_NO_LOGBOOK_AVAILABLE */
/* MessageText: Logbook not available. */
#define ERR_HIL_NO_LOGBOOK_AVAILABLE     ((uint32_t)0xC0000206L)

/***********************************************************************************
  General Driver Related Results
************************************************************************************/

/* MessageId: ERR_HIL_INVALID_HANDLE */
/* MessageText: No description available - ERR_HIL_INVALID_HANDLE. */
#define ERR_HIL_INVALID_HANDLE           ((uint32_t)0xC0001000L)

/* MessageId: ERR_HIL_UNKNOWN_DEVICE */
/* MessageText: No description available - ERR_HIL_UNKNOWN_DEVICE. */
#define ERR_HIL_UNKNOWN_DEVICE           ((uint32_t)0xC0001001L)

/* MessageId: ERR_HIL_RESOURCE_IN_USE */
/* MessageText: No description available - ERR_HIL_RESOURCE_IN_USE. */
#define ERR_HIL_RESOURCE_IN_USE          ((uint32_t)0xC0001002L)

/* MessageId: ERR_HIL_NO_MORE_RESOURCES */
/* MessageText: No description available - ERR_HIL_NO_MORE_RESOURCES. */
#define ERR_HIL_NO_MORE_RESOURCES        ((uint32_t)0xC0001003L)

/* MessageId: ERR_HIL_DRV_OPEN_FAILED */
/* MessageText: No description available - ERR_HIL_DRV_OPEN_FAILED. */
#define ERR_HIL_DRV_OPEN_FAILED          ((uint32_t)0xC0001004L)

/* MessageId: ERR_HIL_DRV_INITIALIZATION_FAILED */
/* MessageText: No description available - ERR_HIL_DRV_INITIALIZATION_FAILED. */
#define ERR_HIL_DRV_INITIALIZATION_FAILED ((uint32_t)0xC0001005L)

/* MessageId: ERR_HIL_DRV_NOT_INITIALIZED */
/* MessageText: No description available - ERR_HIL_DRV_NOT_INITIALIZED. */
#define ERR_HIL_DRV_NOT_INITIALIZED      ((uint32_t)0xC0001006L)

/* MessageId: ERR_HIL_DRV_ALREADY_INITIALIZED */
/* MessageText: No description available - ERR_HIL_DRV_ALREADY_INITIALIZED. */
#define ERR_HIL_DRV_ALREADY_INITIALIZED  ((uint32_t)0xC0001007L)

/* MessageId: ERR_HIL_CRC */
/* MessageText: No description available - ERR_HIL_CRC. */
#define ERR_HIL_CRC                      ((uint32_t)0xC0001008L)

/* Driver resources */

/* MessageId: ERR_HIL_DRV_INVALID_RESOURCE */
/* MessageText: No description available - ERR_HIL_DRV_INVALID_RESOURCE. */
#define ERR_HIL_DRV_INVALID_RESOURCE     ((uint32_t)0xC0001010L)

/* MessageId: ERR_HIL_DRV_INVALID_MEM_RESOURCE */
/* MessageText: No description available - ERR_HIL_DRV_INVALID_MEM_RESOURCE. */
#define ERR_HIL_DRV_INVALID_MEM_RESOURCE ((uint32_t)0xC0001011L)

/* MessageId: ERR_HIL_DRV_INVALID_MEM_SIZE */
/* MessageText: No description available - ERR_HIL_DRV_INVALID_MEM_SIZE. */
#define ERR_HIL_DRV_INVALID_MEM_SIZE     ((uint32_t)0xC0001012L)

/* MessageId: ERR_HIL_DRV_INVALID_PHYS_MEM_BASE */
/* MessageText: No description available - ERR_HIL_DRV_INVALID_PHYS_MEM_BASE. */
#define ERR_HIL_DRV_INVALID_PHYS_MEM_BASE ((uint32_t)0xC0001013L)

/* MessageId: ERR_HIL_DRV_INVALID_PHYS_MEM_SIZE */
/* MessageText: No description available - ERR_HIL_DRV_INVALID_PHYS_MEM_SIZE. */
#define ERR_HIL_DRV_INVALID_PHYS_MEM_SIZE ((uint32_t)0xC0001014L)

/* MessageId: ERR_HIL_DRV_UNDEFINED_HANDLER */
/* MessageText: No description available - ERR_HIL_DRV_UNDEFINED_HANDLER. */
#define ERR_HIL_DRV_UNDEFINED_HANDLER    ((uint32_t)0xC0001015L)

/* Driver interrupt handling */

/* MessageId: ERR_HIL_DRV_ILLEGAL_VECTOR_ID */
/* MessageText: No description available - ERR_HIL_DRV_ILLEGAL_VECTOR_ID. */
#define ERR_HIL_DRV_ILLEGAL_VECTOR_ID    ((uint32_t)0xC0001020L)

/* MessageId: ERR_HIL_DRV_ILLEGAL_IRQ_MASK */
/* MessageText: No description available - ERR_HIL_DRV_ILLEGAL_IRQ_MASK. */
#define ERR_HIL_DRV_ILLEGAL_IRQ_MASK     ((uint32_t)0xC0001021L)

/* MessageId: ERR_HIL_DRV_ILLEGAL_SUBIRQ_MASK */
/* MessageText: No description available - ERR_HIL_DRV_ILLEGAL_SUBIRQ_MASK. */
#define ERR_HIL_DRV_ILLEGAL_SUBIRQ_MASK  ((uint32_t)0xC0001022L)

/* MessageId: ERR_HIL_DRV_STATE_INVALID */
/* MessageText: Driver is in invalid state. */
#define ERR_HIL_DRV_STATE_INVALID        ((uint32_t)0xC0001023L)

/* DPM-Driver specific errors */

/* MessageId: ERR_HIL_DPM_CHANNEL_UNKNOWN */
/* MessageText: No description available - ERR_HIL_DPM_CHANNEL_UNKNOWN. */
#define ERR_HIL_DPM_CHANNEL_UNKNOWN      ((uint32_t)0xC0001100L)

/* MessageId: ERR_HIL_DPM_CHANNEL_INVALID */
/* MessageText: No description available - ERR_HIL_DPM_CHANNEL_INVALID. */
#define ERR_HIL_DPM_CHANNEL_INVALID      ((uint32_t)0xC0001101L)

/* MessageId: ERR_HIL_DPM_CHANNEL_NOT_INITIALIZED */
/* MessageText: No description available - ERR_HIL_DPM_CHANNEL_NOT_INITIALIZED. */
#define ERR_HIL_DPM_CHANNEL_NOT_INITIALIZED ((uint32_t)0xC0001102L)

/* MessageId: ERR_HIL_DPM_CHANNEL_ALREADY_INITIALIZED */
/* MessageText: No description available - ERR_HIL_DPM_CHANNEL_ALREADY_INITIALIZED. */
#define ERR_HIL_DPM_CHANNEL_ALREADY_INITIALIZED ((uint32_t)0xC0001103L)

/* MessageId: 0xC0001104L (No symbolic name defined) */
/* MessageText: No description available - ERR_HIL_DPM_CHANNEL_IN_USE. */


/* MessageId: ERR_HIL_DPM_CHANNEL_LAYOUT_UNKNOWN */
/* MessageText: No description available - ERR_HIL_DPM_CHANNEL_LAYOUT_UNKNOWN. */
#define ERR_HIL_DPM_CHANNEL_LAYOUT_UNKNOWN ((uint32_t)0xC0001120L)

/* MessageId: ERR_HIL_DPM_CHANNEL_SIZE_INVALID */
/* MessageText: No description available - ERR_HIL_DPM_CHANNEL_SIZE_INVALID. */
#define ERR_HIL_DPM_CHANNEL_SIZE_INVALID ((uint32_t)0xC0001121L)

/* MessageId: ERR_HIL_DPM_CHANNEL_SIZE_EXCEEDED */
/* MessageText: No description available - ERR_HIL_DPM_CHANNEL_SIZE_EXCEEDED. */
#define ERR_HIL_DPM_CHANNEL_SIZE_EXCEEDED ((uint32_t)0xC0001122L)

/* MessageId: ERR_HIL_DPM_CHANNEL_TOO_MANY_BLOCKS */
/* MessageText: No description available - ERR_HIL_DPM_CHANNEL_TOO_MANY_BLOCKS. */
#define ERR_HIL_DPM_CHANNEL_TOO_MANY_BLOCKS ((uint32_t)0xC0001123L)

/* MessageId: ERR_HIL_DPM_BLOCK_UNKNOWN */
/* MessageText: No description available - ERR_HIL_DPM_BLOCK_UNKNOWN. */
#define ERR_HIL_DPM_BLOCK_UNKNOWN        ((uint32_t)0xC0001130L)

/* MessageId: ERR_HIL_DPM_BLOCK_SIZE_EXCEEDED */
/* MessageText: No description available - ERR_HIL_DPM_BLOCK_SIZE_EXCEEDED. */
#define ERR_HIL_DPM_BLOCK_SIZE_EXCEEDED  ((uint32_t)0xC0001131L)

/* MessageId: ERR_HIL_DPM_BLOCK_CREATION_FAILED */
/* MessageText: No description available - ERR_HIL_DPM_BLOCK_CREATION_FAILED. */
#define ERR_HIL_DPM_BLOCK_CREATION_FAILED ((uint32_t)0xC0001132L)

/* MessageId: ERR_HIL_DPM_BLOCK_OFFSET_INVALID */
/* MessageText: No description available - ERR_HIL_DPM_BLOCK_OFFSET_INVALID. */
#define ERR_HIL_DPM_BLOCK_OFFSET_INVALID ((uint32_t)0xC0001133L)

/* MessageId: ERR_HIL_DPM_CHANNEL_HOST_MBX_FULL */
/* MessageText: No description available - ERR_HIL_DPM_CHANNEL_HOST_MBX_FULL. */
#define ERR_HIL_DPM_CHANNEL_HOST_MBX_FULL ((uint32_t)0xC0001140L)

/* MessageId: ERR_HIL_DPM_CHANNEL_SEGMENT_LIMIT */
/* MessageText: No description available - ERR_HIL_DPM_CHANNEL_SEGMENT_LIMIT. */
#define ERR_HIL_DPM_CHANNEL_SEGMENT_LIMIT ((uint32_t)0xC0001141L)

/* MessageId: ERR_HIL_DPM_CHANNEL_SEGMENT_UNUSED */
/* MessageText: No description available - ERR_HIL_DPM_CHANNEL_SEGMENT_UNUSED. */
#define ERR_HIL_DPM_CHANNEL_SEGMENT_UNUSED ((uint32_t)0xC0001142L)

/* MessageId: ERR_HIL_NAME_INVALID */
/* MessageText: No description available - ERR_HIL_NAME_INVALID. */
#define ERR_HIL_NAME_INVALID             ((uint32_t)0xC0001143L)

/* MessageId: ERR_HIL_UNEXPECTED_BLOCK_SIZE */
/* MessageText: No description available - ERR_HIL_UNEXPECTED_BLOCK_SIZE. */
#define ERR_HIL_UNEXPECTED_BLOCK_SIZE    ((uint32_t)0xC0001144L)

/* MessageId: ERR_HIL_COMPONENT_BUSY */
/* MessageText: The component is busy and can not handle the requested service. */
#define ERR_HIL_COMPONENT_BUSY           ((uint32_t)0xC0001145L)

/* MessageId: ERR_HIL_INVALID_HEADER */
/* MessageText: Invalid (file) header. E.g. wrong CRC/MD5/Cookie. */
#define ERR_HIL_INVALID_HEADER           ((uint32_t)0xC0001150L)

/* MessageId: ERR_HIL_INCOMPATIBLE */
/* MessageText: Firmware does not match device. */
#define ERR_HIL_INCOMPATIBLE             ((uint32_t)0xC0001151L)

/* MessageId: ERR_HIL_NOT_AVAILABLE */
/* MessageText: Update file or destination (XIP-Area) not found. */
#define ERR_HIL_NOT_AVAILABLE            ((uint32_t)0xC0001152L)

/* MessageId: ERR_HIL_READ */
/* MessageText: Failed to read from file/area. */
#define ERR_HIL_READ                     ((uint32_t)0xC0001153L)

/* MessageId: ERR_HIL_WRITE */
/* MessageText: Failed to write from file/area. */
#define ERR_HIL_WRITE                    ((uint32_t)0xC0001154L)

/* MessageId: ERR_HIL_IDENTICAL */
/* MessageText: Update firmware and installed firmware are identical. */
#define ERR_HIL_IDENTICAL                ((uint32_t)0xC0001155L)

/* MessageId: ERR_HIL_INSTALLATION */
/* MessageText: Error during installation of firmware. */
#define ERR_HIL_INSTALLATION             ((uint32_t)0xC0001156L)

/* MessageId: ERR_HIL_VERIFICATION */
/* MessageText: Error during verification of firmware. */
#define ERR_HIL_VERIFICATION             ((uint32_t)0xC0001157L)

/* MessageId: ERR_HIL_INVALIDATION */
/* MessageText: Error during invalidation of firmware files. */
#define ERR_HIL_INVALIDATION             ((uint32_t)0xC0001158L)

/* MessageId: ERR_HIL_FORMAT */
/* MessageText: Volume is not formated. */
#define ERR_HIL_FORMAT                   ((uint32_t)0xC0001160L)

/* MessageId: ERR_HIL_VOLUME */
/* MessageText: (De-)Initialization of volume failed. */
#define ERR_HIL_VOLUME                   ((uint32_t)0xC0001161L)

/* MessageId: ERR_HIL_VOLUME_DRV */
/* MessageText: (De-)Initialization of volume driver failed. */
#define ERR_HIL_VOLUME_DRV               ((uint32_t)0xC0001162L)

/* MessageId: ERR_HIL_VOLUME_INVALID */
/* MessageText: The volume is invalid. */
#define ERR_HIL_VOLUME_INVALID           ((uint32_t)0xC0001163L)

/* MessageId: ERR_HIL_VOLUME_EXCEEDED */
/* MessageText: Number of supported volumes exceeded. */
#define ERR_HIL_VOLUME_EXCEEDED          ((uint32_t)0xC0001164L)

/* MessageId: ERR_HIL_VOLUME_MOUNT */
/* MessageText: The volume is mounted (in use). */
#define ERR_HIL_VOLUME_MOUNT             ((uint32_t)0xC0001165L)

/* MessageId: ERR_HIL_ERASE */
/* MessageText: Failed to erase file/directory/flash. */
#define ERR_HIL_ERASE                    ((uint32_t)0xC0001166L)

/* MessageId: ERR_HIL_OPEN */
/* MessageText: Failed to open file/directory. */
#define ERR_HIL_OPEN                     ((uint32_t)0xC0001167L)

/* MessageId: ERR_HIL_CLOSE */
/* MessageText: Failed to close file/directory. */
#define ERR_HIL_CLOSE                    ((uint32_t)0xC0001168L)

/* MessageId: ERR_HIL_CREATE */
/* MessageText: Failed to create file/directory. */
#define ERR_HIL_CREATE                   ((uint32_t)0xC0001169L)

/* MessageId: ERR_HIL_MODIFY */
/* MessageText: Failed to modify file/directory. */
#define ERR_HIL_MODIFY                   ((uint32_t)0xC0001170L)

/* MessageId: ERR_HIL_FS_NOT_AVAILABLE */
/* MessageText: File system not available. */
#define ERR_HIL_FS_NOT_AVAILABLE         ((uint32_t)0xC0001171L)

/* MessageId: ERR_HIL_FILE_NOT_FOUND */
/* MessageText: File not available. */
#define ERR_HIL_FILE_NOT_FOUND           ((uint32_t)0xC0001172L)

/* MessageId: ERR_HIL_DIAG_NO_INFO */
/* MessageText: No diagnostic information available. */
#define ERR_HIL_DIAG_NO_INFO             ((uint32_t)0xC0001173L)

/* MessageId: ERR_HIL_QUEUE_UNKNOWN */
/* MessageText: Queue is not available. */
#define ERR_HIL_QUEUE_UNKNOWN            ((uint32_t)0xC0001174L)

/* MessageId: ERR_HIL_NAME_UNKNOWN */
/* MessageText: Name is unknown / not available. */
#define ERR_HIL_NAME_UNKNOWN             ((uint32_t)0xC0001175L)

/* MessageId: ERR_HIL_UPDATE_ERROR */
/* MessageText: Failed to update firmware. */
#define ERR_HIL_UPDATE_ERROR             ((uint32_t)0xC0001176L)

/* MessageId: ERR_HIL_DDP_STATE_INVALID */
/* MessageText: DDP is in wrong state. */
#define ERR_HIL_DDP_STATE_INVALID        ((uint32_t)0xC0001177L)

/* MessageId: ERR_HIL_MANUFACTURER_INVALID */
/* MessageText: Manufacturer in file header does not match target. */
#define ERR_HIL_MANUFACTURER_INVALID     ((uint32_t)0xC0001178L)

/* MessageId: ERR_HIL_DEVICE_CLASS_INVALID */
/* MessageText: Device class in file header does not match target. */
#define ERR_HIL_DEVICE_CLASS_INVALID     ((uint32_t)0xC0001179L)

/* MessageId: ERR_HIL_HW_COMPATIBILITY_INVALID */
/* MessageText: Hardware compatibility index in file header does not match target. */
#define ERR_HIL_HW_COMPATIBILITY_INVALID ((uint32_t)0xC000117AL)

/* MessageId: ERR_HIL_HW_OPTIONS_INVALID */
/* MessageText: Hardware options in file header does not match target. */
#define ERR_HIL_HW_OPTIONS_INVALID       ((uint32_t)0xC000117BL)

/* MessageId: ERR_HIL_INIT_FAULT_FTL */
/* MessageText: FTL volume initialization fault. */
#define ERR_HIL_INIT_FAULT_FTL           ((uint32_t)0xC000117CL)

/* MessageId: ERR_HIL_MD5 */
/* MessageText: MD5 checksum is invalid. */
#define ERR_HIL_MD5                      ((uint32_t)0xC000117DL)

/* MessageId: ERR_HIL_SIGNATURE */
/* MessageText: The signature is invalid. */
#define ERR_HIL_SIGNATURE                ((uint32_t)0xC000117EL)

/***********************************************************************************
  Miscellaneous Results
************************************************************************************/

/* MessageId: SUCCESS_HIL_FRAGMENTED */
/* MessageText: Fragment accepted. */
#define SUCCESS_HIL_FRAGMENTED           ((uint32_t)0x0000F005L)

/* MessageId: ERR_HIL_RESET_REQUIRED */
/* MessageText: Reset required. */
#define ERR_HIL_RESET_REQUIRED           ((uint32_t)0xC000F006L)

/* MessageId: ERR_HIL_EVALUATION_TIME_EXPIRED */
/* MessageText: Evaluation time expired. Reset required. */
#define ERR_HIL_EVALUATION_TIME_EXPIRED  ((uint32_t)0xC000F007L)

/* MessageId: ERR_HIL_FIRMWARE_CRASHED */
/* MessageText: The firmware has crashed and the exception handler is running. */
#define ERR_HIL_FIRMWARE_CRASHED         ((uint32_t)0xC000DEADL)

#endif  /* HIL_RESULTS_H_ */
