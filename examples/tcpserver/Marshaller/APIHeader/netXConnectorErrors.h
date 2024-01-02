#ifndef __NETXCONNECTORERRORS_H__
#define __NETXCONNECTORERRORS_H__

/*******************************************************************************
* netX Connector Errors
*******************************************************************************/
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: NXCON_NO_ERROR
//
// MessageText:
//
//  No Error
//
#define NXCON_NO_ERROR                   ((int32_t)0x00000000L)

/*******************************************************************************
* Generic Connector Errors
*******************************************************************************/
//
// MessageId: NXCON_DRV_INVALID_POINTER
//
// MessageText:
//
//  Invalid pointer (NULL)
//
#define NXCON_DRV_INVALID_POINTER        ((int32_t)0x800F0001L)

//
// MessageId: NXCON_DRV_INVALID_PARAMETER
//
// MessageText:
//
//  NetX Connector invalid parameters
//
#define NXCON_DRV_INVALID_PARAMETER      ((int32_t)0x800F0002L)

//
// MessageId: NXCON_DRV_NOT_INITIALIZED
//
// MessageText:
//
//  NetX Connector not initialized
//
#define NXCON_DRV_NOT_INITIALIZED        ((int32_t)0x800F0003L)

//
// MessageId: NXCON_DRV_FUNC_NOT_IMPL
//
// MessageText:
//
//  NetX Connector function not implemented
//
#define NXCON_DRV_FUNC_NOT_IMPL          ((int32_t)0x800F0004L)

//
// MessageId: NXCON_DRV_WAS_OPENED_BEFORE
//
// MessageText:
//
//  NetX Connector was opened before
//
#define NXCON_DRV_WAS_OPENED_BEFORE      ((int32_t)0x800F0005L)

//
// MessageId: NXCON_DRV_NOT_OPENED
//
// MessageText:
//
//  NetX Connector was not opened
//
#define NXCON_DRV_NOT_OPENED             ((int32_t)0x800F0006L)

//
// MessageId: NXCON_DRV_INIT_ERROR
//
// MessageText:
//
//  Failed to initialize NetX Connector
//
#define NXCON_DRV_INIT_ERROR             ((int32_t)0x800F0007L)

//
// MessageId: NXCON_DRV_NOT_START
//
// MessageText:
//
//  Failed to start NetX Connector
//
#define NXCON_DRV_NOT_START              ((int32_t)0x800F0008L)

//
// MessageId: NXCON_DRV_SEND_ERROR
//
// MessageText:
//
//  Failed to send data
//
#define NXCON_DRV_SEND_ERROR             ((int32_t)0x800F0009L)

//
// MessageId: NXCON_DRV_BUFFER_TOO_SHORT
//
// MessageText:
//
//  The supplied buffer was too short
//
#define NXCON_DRV_BUFFER_TOO_SHORT       ((int32_t)0x800F000AL)

//
// MessageId: NXCON_DRV_DISABLED
//
// MessageText:
//
//  The connector is disabled
//
#define NXCON_DRV_DISABLED               ((int32_t)0x800F000BL)

//
// MessageId: NXCON_CONNECTOR_IDENTIFIER_EMPTY
//
// MessageText:
//
//  The connector identifier is empty
//
#define NXCON_CONNECTOR_IDENTIFIER_EMPTY ((int32_t)0x800F0010L)

//
// MessageId: NXCON_CONNECTOR_DUPLICATE_IDENTIFIER
//
// MessageText:
//
//  Duplicate connector identifier found
//
#define NXCON_CONNECTOR_DUPLICATE_IDENTIFIER ((int32_t)0x800F0011L)

//
// MessageId: NXCON_CONNECTOR_FUNCTIONS_READ_ERROR
//
// MessageText:
//
//  Failed to read all connector functions
//
#define NXCON_CONNECTOR_FUNCTIONS_READ_ERROR ((int32_t)0x800F0012L)

/*******************************************************************************
* Hardware depending errors
*******************************************************************************/
//
// MessageId: NXCON_DRV_GETCOMMSTATE
//
// MessageText:
//
//  Failed to read connector hardware status
//
#define NXCON_DRV_GETCOMMSTATE           ((int32_t)0x800F0020L)

//
// MessageId: NXCON_DRV_SETCOMMSTATE
//
// MessageText:
//
//  Failed to setup connector hardware status
//
#define NXCON_DRV_SETCOMMSTATE           ((int32_t)0x800F0021L)

//
// MessageId: NXCON_DRV_GETTIMEOUT
//
// MessageText:
//
//  Failed to read connector hardware timeouts
//
#define NXCON_DRV_GETTIMEOUT             ((int32_t)0x800F0022L)

//
// MessageId: NXCON_DRV_SETTIMEOUT
//
// MessageText:
//
//  Failed to set connector hardware timeouts
//
#define NXCON_DRV_SETTIMEOUT             ((int32_t)0x800F0023L)

//
// MessageId: NXCON_DRV_SETUPCOMBUFFER
//
// MessageText:
//
//  Failed to set connector hardware buffers
//
#define NXCON_DRV_SETUPCOMBUFFER         ((int32_t)0x800F0024L)

//
// MessageId: NXCON_DRV_SETUPCOMMASK
//
// MessageText:
//
//  Failed to set connector hardware event masks
//
#define NXCON_DRV_SETUPCOMMASK           ((int32_t)0x800F0025L)

//
// MessageId: NXCON_DRV_SETUPCOMHWSIGNAL
//
// MessageText:
//
//  Failed to set connector hardware signals
//
#define NXCON_DRV_SETUPCOMHWSIGNAL       ((int32_t)0x800F0026L)

//
// MessageId: NXCON_DRV_SOCKETERROR
//
// MessageText:
//
//  Generic socket error
//
#define NXCON_DRV_SOCKETERROR            ((int32_t)0x800F0027L)

//
// MessageId: NXCON_DRV_CONNECTION_FAILED
//
// MessageText:
//
//  Failed to establish connection
//
#define NXCON_DRV_CONNECTION_FAILED      ((int32_t)0x800F0028L)

//
// MessageId: NXCON_DRV_CONNECT_TIMEOUT
//
// MessageText:
//
//  Connection failed due to a timeout
//
//
#define NXCON_DRV_CONNECT_TIMEOUT        ((int32_t)0x800F0029L)

/*******************************************************************************
* Configuration errors
*******************************************************************************/
//
// MessageId: NXCON_CONF_INVALID_KEY
//
// MessageText:
//
//  Supplied configuration key is invalid
//
#define NXCON_CONF_INVALID_KEY           ((int32_t)0xC00F0100L)

//
// MessageId: NXCON_CONF_INVALID_VALUE
//
// MessageText:
//
//  Supplied configuration value is invalid
//
#define NXCON_CONF_INVALID_VALUE         ((int32_t)0xC00F0101L)

//
// MessageId: NXCON_CONF_INVALID_INTERFACE
//
// MessageText:
//
//  Supplied interface name is invalid
//
#define NXCON_CONF_INVALID_INTERFACE     ((int32_t)0xC00F0102L)

//
// MessageId: NXCON_CONF_READ_FAILED
//
// MessageText:
//
//  Failed to read configuration
//
#define NXCON_CONF_READ_FAILED           ((int32_t)0xC00F0103L)

//
// MessageId: NXCON_CONF_WRITE_FAILED
//
// MessageText:
//
//  Failed to write configuration
//
#define NXCON_CONF_WRITE_FAILED          ((int32_t)0xC00F0104L)

/*******************************************************************************/

#endif  /*__NETXCONNECTORERRORS_H__ */
