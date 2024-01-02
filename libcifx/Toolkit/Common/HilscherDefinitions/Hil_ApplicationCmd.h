/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_ApplicationCmd.h $: *//*!

  \file Hil_ApplicationCmd.h

  Hilscher Packet Command Codes Handled by the Application Task.

**************************************************************************************/
#ifndef HIL_APPLICATIONCMD_H_
#define HIL_APPLICATIONCMD_H_

#include "Hil_Packet.h"

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_PACK_1(HIL_APPLICATIONCMD)
#endif

/***************************************************************************************/
/* Common global stack commands */


#define HIL_GET_DIAG_INFO_COMMON_STATE_REQ            0x00002F00
#define HIL_GET_DIAG_INFO_COMMON_STATE_CNF            0x00002F01

#define HIL_GET_WATCHDOG_TIME_REQ                     0x00002F02
#define HIL_GET_WATCHDOG_TIME_CNF                     0x00002F03

#define HIL_SET_WATCHDOG_TIME_REQ                     0x00002F04
#define HIL_SET_WATCHDOG_TIME_CNF                     0x00002F05

#define HIL_GET_DIAG_INFO_EXTENDED_STATE_REQ          0x00002F06
#define HIL_GET_DIAG_INFO_EXTENDED_STATE_CNF          0x00002F07

#define HIL_GET_SLAVE_HANDLE_REQ                      0x00002F08
#define HIL_GET_SLAVE_HANDLE_CNF                      0x00002F09

#define HIL_GET_SLAVE_CONN_INFO_REQ                   0x00002F0A
#define HIL_GET_SLAVE_CONN_INFO_CNF                   0x00002F0B

/*! \defgroup HIL_GET_DPM_IO_INFO_doc  Get DPM I/O information
 * \{ */
#define HIL_GET_DPM_IO_INFO_REQ                       0x00002F0C  /*!< Get DPM I/O information request */
#define HIL_GET_DPM_IO_INFO_CNF                       0x00002F0D  /*!< Get DPM I/O information confirmation */
/*! \} */

/*! \defgroup HIL_REGISTER_APP_doc  Register application
 * \{ */
#define HIL_REGISTER_APP_REQ                          0x00002F10  /*!< Register application request */
#define HIL_REGISTER_APP_CNF                          0x00002F11  /*!< Register application confirmation */
/*! \} */

/*! \defgroup HIL_UNREGISTER_APP_doc  Unregister application
 * \{ */
#define HIL_UNREGISTER_APP_REQ                        0x00002F12  /*!< Unregister application request */
#define HIL_UNREGISTER_APP_CNF                        0x00002F13  /*!< Unregister application confirmation */
/*! \} */

#define HIL_DELETE_CONFIG_REQ                         0x00002F14
#define HIL_DELETE_CONFIG_CNF                         0x00002F15

#define HIL_READ_IO_DATA_IMAGE_REQ                    0x00002F20
#define HIL_READ_IO_DATA_IMAGE_CNF                    0x00002F21

#define HIL_BUSSCAN_REQ                               0x00002f22
#define HIL_BUSSCAN_CNF                               0x00002f23

#define HIL_GET_DEVICE_INFO_REQ                       0x00002f24
#define HIL_GET_DEVICE_INFO_CNF                       0x00002f25

/*! \defgroup HIL_START_STOP_COMM_doc Start or stop bus communication
 * \{ */
#define HIL_START_STOP_COMM_REQ                       0x00002F30  /*!< Start or stop bus communication request */
#define HIL_START_STOP_COMM_CNF                       0x00002F31  /*!< Start or stop bus communication confirmation */
/*! \} */

/*! \defgroup HIL_LOCK_UNLOCK_CONFIG_doc Lock or unlock configuration
 * \{ */
#define HIL_LOCK_UNLOCK_CONFIG_REQ                    0x00002F32  /*!< Lock or unlock configuration request */
#define HIL_LOCK_UNLOCK_CONFIG_CNF                    0x00002F33  /*!< Lock or unlock configuration confirmation */
/*! \} */

#define HIL_SET_HANDSHAKE_CONFIG_REQ                  0x00002F34
#define HIL_SET_HANDSHAKE_CONFIG_CNF                  0x00002F35

/*! \defgroup HIL_CHANNEL_INIT_doc Channel initialization
 * \{ */
#define HIL_CHANNEL_INIT_REQ                          0x00002F80  /*!< Channel initialization request */
#define HIL_CHANNEL_INIT_CNF                          0x00002F81  /*!< Channel initialization confirmation */
/*! \} */

#define HIL_VERIFY_DATABASE_REQ                       0x00002F82
#define HIL_VERIFY_DATABASE_CNF                       0x00002F83

#define HIL_ACTIVATE_DATABASE_REQ                     0x00002F84
#define HIL_ACTIVATE_DATABASE_CNF                     0x00002F85

/*! \defgroup HIL_SET_FW_PARAMETER_doc  Set firmware parameter
 * \{ */
#define HIL_SET_FW_PARAMETER_REQ                      0x00002F86  /*!< Set firmware parameter request */
#define HIL_SET_FW_PARAMETER_CNF                      0x00002F87  /*!< Set firmware parameter confirmation */
/*! \} */

/*! \defgroup HIL_GET_FW_PARAMETER_doc  Get firmware parameter
 * \{ */
#define HIL_GET_FW_PARAMETER_REQ                      0x00002F88  /*!< Get firmware parameter request */
#define HIL_GET_FW_PARAMETER_CNF                      0x00002F89  /*!< Get firmware parameter confirmation */
/*! \} */

/*! \defgroup HIL_LINK_STATUS_CHANGE_doc  Link status change
 * \{ */
#define HIL_LINK_STATUS_CHANGE_IND                    0x00002F8A  /*!< Link status change indication */
#define HIL_LINK_STATUS_CHANGE_RES                    0x00002F8B  /*!< Link status change response */
/*! \} */

/*! \defgroup HIL_SET_REMANENT_DATA_doc  Set remanent data service
 * \{ */
#define HIL_SET_REMANENT_DATA_REQ                     0x00002F8C  /*!< Set remanent data service request */
#define HIL_SET_REMANENT_DATA_CNF                     0x00002F8D  /*!< Set remanent data service confirmation */
/*! \} */

/*! \defgroup HIL_STORE_REMANENT_DATA_doc  Store remanent data
 * \{ */
#define HIL_STORE_REMANENT_DATA_IND                   0x00002F8E  /*!< Store remanent data indication */
#define HIL_STORE_REMANENT_DATA_RES                   0x00002F8F  /*!< Store remanent data response */
/*! \} */

/*! \defgroup HIL_SET_TRIGGER_TYPE_doc  Set data exchange trigger mode
 * \{ */
#define HIL_SET_TRIGGER_TYPE_REQ                      0x00002F90  /*!< Set data exchange trigger mode request */
#define HIL_SET_TRIGGER_TYPE_CNF                      0x00002F91  /*!< Set data exchange trigger mode confirmation */
/*! \} */

/*! \defgroup HIL_GET_TRIGGER_TYPE_doc  Get data exchange trigger mode
 * \{ */
#define HIL_GET_TRIGGER_TYPE_REQ                      0x00002F92  /*!< Get data exchange trigger mode request */
#define HIL_GET_TRIGGER_TYPE_CNF                      0x00002F93  /*!< Get data exchange trigger mode confirmation */
/*! \} */


/*! \defgroup HIL_READ_LOG_BOOK_ENTRIES_doc  Read out the logbook
 * \{ */
#define HIL_READ_LOG_BOOK_ENTRIES_REQ                 0x00002F94  /*!< Read out the logbook request */
#define HIL_READ_LOG_BOOK_ENTRIES_CNF                 0x00002F95  /*!< Read out the logbook confirmation */
/*! \} */

/*! \defgroup HIL_GET_LOG_BOOK_LAYOUT_doc  Get logbook layout
 * \{ */
#define HIL_GET_LOG_BOOK_LAYOUT_REQ                   0x00002F96  /*!< Get logbook layout request */
#define HIL_GET_LOG_BOOK_LAYOUT_CNF                   0x00002F97  /*!< Get logbook layout confirmation */
/*! \} */

/*! \defgroup HIL_CLEAR_LOG_BOOK_doc  Clear the logbook
 * \{ */
#define HIL_CLEAR_LOG_BOOK_REQ                        0x00002F98  /*!< Clear the logbook request */
#define HIL_CLEAR_LOG_BOOK_CNF                        0x00002F99  /*!< Clear the logbook confirmation */
/*! \} */

/*! \defgroup HIL_SET_LOG_BOOK_SEVERITY_LEVEL_doc  Set severity level of logbook
 * \{ */
#define HIL_SET_LOG_BOOK_SEVERITY_LEVEL_REQ           0x00002F9A  /*!< Set severity level of logbook request */
#define HIL_SET_LOG_BOOK_SEVERITY_LEVEL_CNF           0x00002F9B  /*!< Set severity level of logbook confirmation */
/*! \} */


/******************************************************************************
 * Packet Definition
 ******************************************************************************/



/******************************************************************************
 * Packet: HIL_GET_DIAG_INFO_COMMON_STATE_REQ/HIL_GET_DIAG_INFO_COMMON_STATE_CNF
 *
 *          This packet allows to read out the common state block of the
 *          channel over the mailbox.
 */

/***** request packet *****/
typedef HIL_EMPTY_PACKET_T HIL_GET_DIAG_INFO_COMMON_STATE_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_DIAG_INFO_COMMON_STATE_CNF_DATA_Ttag
{
  uint8_t abCommonState[64];
} HIL_GET_DIAG_INFO_COMMON_STATE_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_DIAG_INFO_COMMON_STATE_CNF_Ttag
{
  HIL_PACKET_HEADER_T                       tHead;
  HIL_GET_DIAG_INFO_COMMON_STATE_CNF_DATA_T tData;
} HIL_GET_DIAG_INFO_COMMON_STATE_CNF_T;

/******************************************************************************
 * Packet: HIL_GET_WATCHDOG_TIME_REQ/HIL_GET_WATCHDOG_TIME_CNF
 *
 *          This packet allows retrieving the actual watchdog time
 */

/***** request packet *****/
typedef HIL_EMPTY_PACKET_T HIL_GET_WATCHDOG_TIME_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_WATCHDOG_TIME_CNF_DATA_Ttag
{
  /** watchdog time in ms */
  uint32_t ulWdgTime;
} HIL_GET_WATCHDOG_TIME_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_WATCHDOG_TIME_CNF_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;
  HIL_GET_WATCHDOG_TIME_CNF_DATA_T  tData;
} HIL_GET_WATCHDOG_TIME_CNF_T;

/******************************************************************************
 * Packet: HIL_SET_WATCHDOG_TIME_REQ/HIL_SET_WATCHDOG_TIME_CNF
 *
 *          This packet allows setting the actual watchdog time
 */

/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_WATCHDOG_TIME_REQ_DATA_Ttag
{
  /** watchdog time in ms */
  uint32_t ulWdgTime;
} HIL_SET_WATCHDOG_TIME_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_WATCHDOG_TIME_REQ_Ttag
{
  HIL_PACKET_HEADER_T                     tHead;
  HIL_SET_WATCHDOG_TIME_REQ_DATA_T  tData;
} HIL_SET_WATCHDOG_TIME_REQ_T;

/***** confirmation packet *****/
typedef HIL_EMPTY_PACKET_T HIL_SET_WATCHDOG_TIME_CNF_T;

/******************************************************************************
 * Packet: HIL_GET_DIAG_INFO_EXTENDED_STATE_REQ/HIL_GET_DIAG_INFO_EXTENDED_STATE_CNF
 *
 *          This packet allows to read out the extended state block of the
 *          channel over the mailbox.
 */

/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_DIAG_INFO_EXTENDED_STATE_REQ_DATA_Ttag
{
  uint32_t ulOffset;
  uint32_t ulDataLen;
} HIL_GET_DIAG_INFO_EXTENDED_STATE_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_DIAG_INFO_EXTENDED_STATE_REQ_Ttag
{
  HIL_PACKET_HEADER_T                         tHead;
  HIL_GET_DIAG_INFO_EXTENDED_STATE_REQ_DATA_T tData;
} HIL_GET_DIAG_INFO_EXTENDED_STATE_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_DIAG_INFO_EXTENDED_STATE_CNF_DATA_Ttag
{
  uint32_t ulOffset;
  uint32_t ulDataLen;
  uint8_t  abExtendedState[432];
} HIL_GET_DIAG_INFO_EXTENDED_STATE_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_DIAG_INFO_EXTENDED_STATE_CNF_Ttag
{
  HIL_PACKET_HEADER_T                         tHead;
  HIL_GET_DIAG_INFO_EXTENDED_STATE_CNF_DATA_T tData;
} HIL_GET_DIAG_INFO_EXTENDED_STATE_CNF_T;

/******************************************************************************
 * Packet: HIL_PACKET_GET_SLAVE_HANDLE_REQ/HIL_PACKET_GET_SLAVE_HANDLE_CNF
 *
 *          This packet allows retrieving diagnostic information of the
 *          connected devices
 */

/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PACKET_GET_SLAVE_HANDLE_REQ_DATA_Tag
{
  uint32_t ulParam;
}  HIL_PACKET_GET_SLAVE_HANDLE_REQ_DATA_T;

#define HIL_LIST_CONF_SLAVES                0x00000001
#define HIL_LIST_ACTV_SLAVES                0x00000002
#define HIL_LIST_FAULTED_SLAVES             0x00000003

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PACKET_GET_SLAVE_HANDLE_REQ_Tag
{
  HIL_PACKET_HEADER_T                     tHead;
  HIL_PACKET_GET_SLAVE_HANDLE_REQ_DATA_T  tData;
} HIL_PACKET_GET_SLAVE_HANDLE_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PACKET_GET_SLAVE_HANDLE_CNF_DATA_Tag
{
  uint32_t ulParam;
  uint32_t aulHandle[1];
}  HIL_PACKET_GET_SLAVE_HANDLE_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PACKET_GET_SLAVE_HANDLE_CNF_Tag
{
  HIL_PACKET_HEADER_T                     tHead;
  HIL_PACKET_GET_SLAVE_HANDLE_CNF_DATA_T  tData;
} HIL_PACKET_GET_SLAVE_HANDLE_CNF_T;

/******************************************************************************
 * Packet: HIL_PACKET_GET_SLAVE_CONN_INFO_REQ/HIL_PACKET_GET_SLAVE_CONN_INFO_CNF
 *
 *          This packet allows retrieving detail information of a slave
 */

/***** request packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PACKET_GET_SLAVE_CONN_INFO_REQ_DATA_Tag
{
  uint32_t ulHandle;
}  HIL_PACKET_GET_SLAVE_CONN_INFO_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PACKET_GET_SLAVE_CONN_INFO_REQ_Tag
{
  HIL_PACKET_HEADER_T                        tHead;
  HIL_PACKET_GET_SLAVE_CONN_INFO_REQ_DATA_T  tData;
} HIL_PACKET_GET_SLAVE_CONN_INFO_REQ_T;

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PACKET_GET_SLAVE_CONN_INFO_CNF_DATA_Tag
{
  uint32_t ulHandle;
  uint32_t ulStructID;
  /*
    Feldbus specific structure
  */
} HIL_PACKET_GET_SLAVE_CONN_INFO_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PACKET_GET_SLAVE_CONN_INFO_CNF_Tag
{
  HIL_PACKET_HEADER_T                        tHead;
  HIL_PACKET_GET_SLAVE_CONN_INFO_CNF_DATA_T  tData;
}  HIL_PACKET_GET_SLAVE_CONN_INFO_CNF_T;




/******************************************************************************/
/*! \addtogroup HIL_GET_DPM_IO_INFO_doc
 * Read the configured size of the I/O process data image.
 *
 * \note For more information please refer netX Dual-Port Memory
 *       packet-based services manual (DOC161001APIxxxx).
 *
 * \{ */

/*! Get DPM IO information request packet. */
typedef HIL_EMPTY_PACKET_T    HIL_GET_DPM_IO_INFO_REQ_T;


/*! DPM IO block information. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_DPM_IO_BLOCK_INFO_Ttag
{
  /*! Index of sub block.
   * The value identifies the index of the sub block. This field is only informative
   * and shall not be used by an application.
   * The values shall be set (for default DPM layouts) to
   *  - 6 for standard input image
   *  - and to 5 for standard output */
  uint32_t    ulSubblockIndex;

  /*! Type of sub block.
   * The type definitions (HIL_BLOCK_*) can be found in Hil_DualPortMemory.h. */
  uint32_t    ulType;

  /*! Flags of the sub block.
   * The flag definitions (HIL_DIRECTION_* and HIL_TRANSMISSION_TYPE_*)
   * can be found in Hil_DualPortMemory.h. */
  uint16_t    usFlags;

  /*! Reserved */
  uint16_t    usReserved;

  /*! Start offset of the IO data  */
  uint32_t    ulOffset;

  /*! Length of used IO data   */
  uint32_t    ulLength;
} HIL_DPM_IO_BLOCK_INFO_T;

/*! Get DPM IO information confirmation data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_DPM_IO_INFO_CNF_DATA_Ttag
{
  uint32_t                ulNumIOBlockInfo; /*!< Number of IO Block Info  */
  HIL_DPM_IO_BLOCK_INFO_T atIOBlockInfo[2]; /*!< Array of I/O Block information */
}   HIL_GET_DPM_IO_INFO_CNF_DATA_T;

/*! Get DPM IO information confirmation packet. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_DPM_IO_INFO_CNF_Ttag
{
  HIL_PACKET_HEADER_T             tHead; /*!< Packet header. */
  HIL_GET_DPM_IO_INFO_CNF_DATA_T  tData; /*!< Packet data. */
}   HIL_GET_DPM_IO_INFO_CNF_T;

/*! \} ************************************************************************/




/******************************************************************************/
/*! \addtogroup HIL_REGISTER_APP_doc
 * Registering the application at the protocol (application task).
 *
 * \note For more information please refer netX Dual-Port Memory
 *       packet-based services manual (DOC161001APIxxxx).
 *
 * \{ */

/*! Register application request packet. */
typedef HIL_EMPTY_PACKET_T          HIL_REGISTER_APP_REQ_T;


/*! Register application confirmation packet. */
typedef HIL_EMPTY_PACKET_T          HIL_REGISTER_APP_CNF_T;

/*! \} ************************************************************************/




/******************************************************************************/
/*! \addtogroup HIL_UNREGISTER_APP_doc
 * Unregister the application at the protocol (application task).
 *
 * \note For more information please refer netX Dual-Port Memory
 *       packet-based services manual (DOC161001APIxxxx).
 *
 * \{ */

/*! Unregister application request packet. */
typedef HIL_EMPTY_PACKET_T          HIL_UNREGISTER_APP_REQ_T;


/*! Unregister application confirmation packet. */
typedef HIL_EMPTY_PACKET_T          HIL_UNREGISTER_APP_CNF_T;

/*! \} ************************************************************************/




/******************************************************************************
 * Packet: HIL_DELETE_CONFIG_REQ/HIL_DELETE_CONFIG_CNF
 *
 *          This packet allows to delete the actual configuration
 */

/***** request packet *****/
typedef HIL_EMPTY_PACKET_T          HIL_DELETE_CONFIG_REQ_T;

/***** confirmation packet *****/
typedef HIL_EMPTY_PACKET_T          HIL_DELETE_CONFIG_CNF_T;

/******************************************************************************
 * HIL_BUSSCAN_REQ
 ******************************************************************************/
#define HIL_BUSSCAN_CMD_START    0x01
#define HIL_BUSSCAN_CMD_STATUS   0x02
#define HIL_BUSSCAN_CMD_ABORT    0x03

/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BUSSCAN_REQ_DATA_Ttag
{
  uint32_t ulAction;
} HIL_BUSSCAN_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BUSSCAN_REQ_Ttag
{
  HIL_PACKET_HEADER_T tHead;
  HIL_BUSSCAN_REQ_DATA_T tData;
} HIL_BUSSCAN_REQ_T;

#define HIL_BUSSCAN_REQ_SIZE     (sizeof(HIL_BUSSCAN_REQ_DATA_T))

/***** confirmation packet *****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BUSSCAN_CNF_DATA_Ttag
{
  uint32_t ulMaxProgress;
  uint32_t ulActProgress;
  uint8_t  abDeviceList[4];
} HIL_BUSSCAN_CNF_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BUSSCAN_CNF_Ttag
{
  HIL_PACKET_HEADER_T tHead;
  HIL_BUSSCAN_CNF_DATA_T tData;
} HIL_BUSSCAN_CNF_T;

#define HIL_BUSSCAN_CNF_SIZE     (sizeof(HIL_BUSSCAN_CNF_DATA_T) - 4)


/******************************************************************************
 * HIL_GET_DEVICE_INFO_REQ
 ******************************************************************************/
/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_DEVICE_INFO_REQ_DATA_Ttag
{
  uint32_t ulDeviceIdx;
} HIL_GET_DEVICE_INFO_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_DEVICE_INFO_REQ_Ttag
{
  HIL_PACKET_HEADER_T            tHead;
  HIL_GET_DEVICE_INFO_REQ_DATA_T tData;
} HIL_GET_DEVICE_INFO_REQ_T;

#define HIL_GET_DEVICE_INFO_REQ_SIZE     (sizeof(HIL_GET_DEVICE_INFO_REQ_DATA_T))

/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_DEVICE_INFO_CNF_DATA_Ttag
{
  uint32_t ulDeviceIdx;
  uint32_t ulStructId;
  /* uint8_t  tStruct; Fieldbus specific structure */
} HIL_GET_DEVICE_INFO_CNF_DATA_T;

typedef struct HIL_GET_DEVICE_INFO_CNF_Ttag
{
  HIL_PACKET_HEADER_T            tHead;
  HIL_GET_DEVICE_INFO_CNF_DATA_T tData;
} HIL_GET_DEVICE_INFO_CNF_T;

#define HIL_GET_DEVICE_INFO_CNF_SIZE     (sizeof(HIL_GET_DEVICE_INFO_CNF_DATA_T))



/******************************************************************************/
/*! \addtogroup HIL_START_STOP_COMM_doc
 * Start or stop bus communication.
 *
 * \note For more information please refer netX Dual-Port Memory
 *       packet-based services manual (DOC161001APIxxxx).
 *
 * \{ */

#define HIL_START_STOP_COMM_PARAM_START     0x00000001  /*!< Start the bus communication. */
#define HIL_START_STOP_COMM_PARAM_STOP      0x00000002  /*!< Stop the bus communication. */


/*! Start or stop bus communication request data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_START_STOP_COMM_REQ_DATA_Ttag
{
  /*! Parameter (HIL_START_STOP_COMM_PARAM_*) to control the bus communication state. */
  uint32_t  ulParam;
}   HIL_START_STOP_COMM_REQ_DATA_T;


/*! Start or stop bus communication request packet. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_START_STOP_COMM_REQ_Ttag
{
  HIL_PACKET_HEADER_T             tHead; /*!< Packet header. */
  HIL_START_STOP_COMM_REQ_DATA_T  tData; /*!< Packet data. */
}   HIL_START_STOP_COMM_REQ_T;


/*! Start or stop bus communication confirmation packet. */
typedef HIL_EMPTY_PACKET_T    HIL_START_STOP_COMM_CNF_T;

/*! \} ************************************************************************/




/******************************************************************************/
/*! \addtogroup HIL_LOCK_UNLOCK_CONFIG_doc
 * Lock or unlock the configuration settings.
 *
 * \note For more information please refer netX Dual-Port Memory
 *       packet-based services manual (DOC161001APIxxxx).
 *
 * \{ */

#define HIL_LOCK_UNLOCK_CONFIG_PARAM_LOCK     0x00000001
#define HIL_LOCK_UNLOCK_CONFIG_PARAM_UNLOCK   0x00000002


/*! Lock or unlock the configuration settings request data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_LOCK_UNLOCK_CONFIG_REQ_DATA_Ttag
{
  /*! Parameter (HIL_LOCK_UNLOCK_CONFIG_PARAM_*) to control the configuration lock state. */
  uint32_t    ulParam;
}   HIL_LOCK_UNLOCK_CONFIG_REQ_DATA_T;

/*! Lock or unlock the configuration settings request packet. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_LOCK_UNLOCK_CONFIG_REQ_Ttag
{
  HIL_PACKET_HEADER_T               tHead;  /*!< Packet header. */
  HIL_LOCK_UNLOCK_CONFIG_REQ_DATA_T tData;  /*!< Packet data. */
}   HIL_LOCK_UNLOCK_CONFIG_REQ_T;


/*! Lock or unlock the configuration settings confirmation packet. */
typedef HIL_EMPTY_PACKET_T    HIL_LOCK_UNLOCK_CONFIG_CNF_T;

/*! \} ************************************************************************/




/******************************************************************************
 * HIL_SET_HANDSHAKE_CONFIG_REQ
 ******************************************************************************/
/***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_HANDSHAKE_CONFIG_REQ_DATA_Ttag
{
  /*! Input process data handshake mode */
  uint8_t                           bPDInHskMode;
  /*! Input process data trigger source. Currently unused, set to zero. */
  uint8_t                           bPDInSource;
  /*! Threshold for input process data handshake handling errors */
  uint16_t                          usPDInErrorTh;

  /*! Output process data handshake mode */
  uint8_t                           bPDOutHskMode;
  /*! Output process data trigger source. Currently unused, set to zero. */
  uint8_t                           bPDOutSource;
  /*! Threshold for output process data handshake handling errors */
  uint16_t                          usPDOutErrorTh;

  /*! Synchronization handshake mode */
  uint8_t                           bSyncHskMode;
  /*! Synchronization source */
  uint8_t                           bSyncSource;
  /*! Threshold for synchronization handshake handling errors */
  uint16_t                          usSyncErrorTh;

  /*! Reserved for future use. Set to zero. */
  uint32_t                          aulReserved[2];
} HIL_SET_HANDSHAKE_CONFIG_REQ_DATA_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_HANDSHAKE_CONFIG_REQ_Ttag
{
  HIL_PACKET_HEADER_T                 tHead;
  HIL_SET_HANDSHAKE_CONFIG_REQ_DATA_T tData;
} HIL_SET_HANDSHAKE_CONFIG_REQ_T;

#define HIL_SET_HANDSHAKE_CONFIG_REQ_SIZE     (sizeof(HIL_SET_HANDSHAKE_CONFIG_REQ_DATA_T))

/***** confirmation packet *****/
typedef HIL_EMPTY_PACKET_T  HIL_SET_HANDSHAKE_CONFIG_CNF_T;

#define HIL_SET_HANDSHAKE_CONFIG_CNF_SIZE     (0)




/******************************************************************************/
/*! \addtogroup HIL_CHANNEL_INIT_doc
 * Initialization or re-initialization of the channel.
 *
 * \note For more information please refer netX Dual-Port Memory
 *       packet-based services manual (DOC161001APIxxxx).
 *
 * \{ */

/*! Initialization of channel request packet. */
typedef HIL_EMPTY_PACKET_T    HIL_CHANNEL_INIT_REQ_T;


/*! Initialization of channel confirmation packet. */
typedef HIL_EMPTY_PACKET_T    HIL_CHANNEL_INIT_CNF_T;

/*! \} ************************************************************************/




/******************************************************************************
 * Packet: HIL_VERIFY_DATABASE_REQ /HIL_VERIFY_DATABASE_CNF
 *
 *          This packet adds new slaves to the database
 */
 /***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_VERIFY_DATABASE_REQ_Ttag
{
  HIL_PACKET_HEADER_T tHead;                    /*!< packet header */
} HIL_VERIFY_DATABASE_REQ_T;

#define HIL_VERIFY_DATABASE_REQ_SIZE 0


/***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_VERIFY_SLAVE_DATABASE_LIST_Ttag
{
    uint32_t  ulLen;
    uint8_t  abData[16];
} HIL_VERIFY_SLAVE_DATABASE_LIST_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_VERIFY_MASTER_DATABASE_Ttag
{
  uint32_t  ulMasterSettings;   /*!< field bus independent changes */
  uint32_t  ulMasterStatus;     /*!< field bus specific status */
  uint32_t  ulReserved[2];
} HIL_VERIFY_MASTER_DATABASE_T;


#define HIL_VERIFY_SLAVE_DATABASE_LIST_SIZE sizeof(HIL_VERIFY_SLAVE_DATABASE_LIST_T)
#define HIL_CIR_MST_SET_STARTUP       0x00000001
#define HIL_CIR_MST_SET_WATCHDOG      0x00000002
#define HIL_CIR_MST_SET_STATUSOFFSET  0x00000004
#define HIL_CIR_MST_SET_BUSPARAMETER  0x00000008

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_VERIFY_DATABASE_CNF_DATA_Ttag
{
    HIL_VERIFY_SLAVE_DATABASE_LIST_T                tNewSlaves;
    HIL_VERIFY_SLAVE_DATABASE_LIST_T                tDeactivatedSlaves;
    HIL_VERIFY_SLAVE_DATABASE_LIST_T                tChangedSlaves;
    HIL_VERIFY_SLAVE_DATABASE_LIST_T                tUnchangedSlaves;
    HIL_VERIFY_SLAVE_DATABASE_LIST_T                tImpossibleSlaveChanges;
    HIL_VERIFY_MASTER_DATABASE_T                    tMasterChanges;
} HIL_VERIFY_DATABASE_CNF_DATA_T;

#define HIL_VERIFY_DATABASE_CNF_DATA_SIZE sizeof(HIL_VERIFY_DATABASE_CNF_DATA_T)

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_VERIFY_DATABASE_CNF_Ttag
{
  HIL_PACKET_HEADER_T                       tHead;    /*!< packet header  */
  HIL_VERIFY_DATABASE_CNF_DATA_T            tData;    /*!< packet data    */
} HIL_VERIFY_DATABASE_CNF_T;

#define HIL_VERFIY_DATABASE_CNF_PACKET_SIZE sizeof(HIL_VERIFY_DATABASE_CNF_T)



/******************************************************************************
 * Packet: HIL_CHANNEL_ACTIVATE_REQ/HIL_CHANNEL_NEW_DATABASE_CNF
 *
 *          This packet activates the new configured slaves
  */
 /***** request packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_ACTIVATE_DATABASE_REQ_Ttag
{
  HIL_PACKET_HEADER_T tHead;                    /*!< packet header */
} HIL_ACTIVATE_DATABASE_REQ_T;



 /***** confirmation packet *****/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_ACTIVATE_DATABASE_CNF_DATA_Ttag
{
  uint8_t                 abSlvSt[16];                    /*!< State of the slaves after configuration */
} HIL_ACTIVATE_DATABASE_CNF_DATA_T;

#define HIL_ACTIVATE_DATABASE_CNF_DATA_SIZE sizeof(HIL_ACTIVATE_DATABASE_CNF_DATA_T)


typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_ACTIVATE_DATABASE_CNF_Ttag
{
  HIL_PACKET_HEADER_T                       tHead;                   /*!< packet header */
  HIL_ACTIVATE_DATABASE_CNF_DATA_T          tData;
} HIL_ACTIVATE_DATABASE_CNF_T;

#define HIL_ACTIVATE_DATABASE_CNF_PACKET_SIZE sizeof(HIL_ACTIVATE_DATABASE_CNF_T)

/******************************************************************************/
/*! \addtogroup HIL_SET_FW_PARAMETER_doc
 *
 * This service allows a host application to modify certain protocol stack
 * parameters from the current configuration. This requires that Controlled
 * Start of Communication is set in the configuration database file and the
 * protocol stack is waiting for the BUS ON / APPLICATION READY command.
 *
 * \note For more information please refer netX Dual-Port Memory
 *       packet-based services manual (DOC161001APIxxxx).
 *
 * \{ */

/***** Parameter IDs defines  *****/
#define HIL_FW_PID_STATION_ADDRESS            0x30000001 /*!< Station address of device */
#define HIL_FW_PID_BAUDRATE                   0x30000002 /*!< Baudrate of device */
#define HIL_FW_PID_PN_NAME_OF_STATION         0x30015001 /*!< PROFINET: Name of Station String */
#define HIL_FW_PID_ECS_DEVICE_IDENTIFICATION  0x30009001 /*!< EtherCAT: Value for Explicit Device Identification */
#define HIL_FW_PID_ECS_SCND_STATION_ADDRESS   0x30009002 /*!< EtherCAT: Second Station Address */


/*! Set firmware parameter request data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_FW_PARAMETER_REQ_DATA_Ttag
{
  /*! Parameter ID which shall be modified, see HIL_FW_PID_* */
  uint32_t ulParameterID;
  /*! Length of following abParameter member in bytes */
  uint32_t ulParameterLen;
  /*! Parameter data, structure depend on ulParameterID.
   * Please refer the related API Manual of Protocol stack for more information. */
  uint8_t  abParameter[__HIL_VARIABLE_LENGTH_ARRAY];
} HIL_SET_FW_PARAMETER_REQ_DATA_T;

/*! Set firmware parameter request. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_FW_PARAMETER_REQ_Ttag
{
  HIL_PACKET_HEADER_T             tHead; /*!< Packet header. */
  HIL_SET_FW_PARAMETER_REQ_DATA_T tData; /*!< Packet data. */
} HIL_SET_FW_PARAMETER_REQ_T;

#define HIL_SET_FW_PARAMETER_REQ_SIZE(parameterLen) (2 * sizeof(uint32_t) + (parameterLen) * sizeof(uint8_t))


/*! Set firmware parameter confirmation. */
typedef HIL_EMPTY_PACKET_T  HIL_SET_FW_PARAMETER_CNF_T;

#define HIL_SET_FW_PARAMETER_CNF_SIZE     (0)

/*! \} ************************************************************************/

/******************************************************************************/
/*! \addtogroup HIL_GET_FW_PARAMETER_doc
 *
 * \note This service is usually not supported by any application layer.
 *
 * \{ */

/*! Get firmware parameter request data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_FW_PARAMETER_REQ_DATA_Ttag
{
  /*! Parameter ID which shall be read, see HIL_FW_PID_* */
  uint32_t ulParameterID;
} HIL_GET_FW_PARAMETER_REQ_DATA_T;

/*! Get firmware parameter request. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_FW_PARAMETER_REQ_Ttag
{
  HIL_PACKET_HEADER_T             tHead;  /*!< Packet header. */
  HIL_GET_FW_PARAMETER_REQ_DATA_T tData;  /*!< Packet data. */
} HIL_GET_FW_PARAMETER_REQ_T;

#define HIL_GET_FW_PARAMETER_REQ_SIZE (sizeof(HIL_GET_FW_PARAMETER_REQ_DATA_T))



/*! Get firmware parameter confirmation. */
typedef HIL_SET_FW_PARAMETER_REQ_T   HIL_GET_FW_PARAMETER_CNF_T;

#define HIL_GET_FW_PARAMETER_CNF_SIZE HIL_SET_FW_PARAMETER_REQ_SIZE

/*! \} ************************************************************************/


/******************************************************************************/
/*! \addtogroup HIL_LINK_STATUS_CHANGE_doc
 *
 * The link status change indication is generated by the device to inform the
 * host application about a change of PHY link.
 * The indication is only send when the host has registered his application
 * by sending "HIL_REGISTER_APP_REQ" to the channel. After registration the
 * actual status is will be notified with this indication.
 *
 * There is only one indication at any time send to the host application.
 * This indication gives no guarantee that all changes of the PHYs can be
 * observed with this indication (e.g. fast link changes and slow host
 * application). However the host application gets always the last
 * (actual) status reported.
 *
 * \note For more information please refer netX Dual-Port Memory
 *       packet-based services manual (DOC161001APIxxxx).
 *
 * \{ */

/*! Link status change indication data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_LINK_STATUS_Ttag
{
  uint32_t ulPort;           /*!< Port ID, starting with 0.
                                  This is also the index of atLinkData field in the
                                  HIL_LINK_STATUS_CHANGE_IND_DATA_T structure. */
  uint32_t fIsFullDuplex;    /*!< If a full duplex link is available on this port. */
  uint32_t fIsLinkUp;        /*!< If a link is available on this port. */
  uint32_t ulSpeed;          /*!< Speed of the link in MBit. */

} HIL_LINK_STATUS_T;

/*! Link status change indication data array. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_LINK_STATUS_CHANGE_IND_DATA_Ttag
{
  /** Link data for PHYs on netX.
   * \note For netX51/52/100/500/90 the array always contain 2 entries, regardless of
   *       actual number of used PHYs (e.g. single port devices).
   *       Other chips may use different amount of entries. */
  HIL_LINK_STATUS_T  atLinkData[2];
} HIL_LINK_STATUS_CHANGE_IND_DATA_T;

/*! Link status change indication. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_LINK_STATUS_CHANGE_IND_Ttag
{
  HIL_PACKET_HEADER_T               tHead; /*!< Packet header. */
  HIL_LINK_STATUS_CHANGE_IND_DATA_T tData; /*!< Packet data. */
} HIL_LINK_STATUS_CHANGE_IND_T;

/*! Link status change response. */
typedef HIL_EMPTY_PACKET_T                HIL_LINK_STATUS_CHANGE_RES_T;

/*! \} ************************************************************************/




/******************************************************************************/
/*! \addtogroup HIL_SET_REMANENT_DATA_doc
 *
 * This service shall be used by the application once after the system is started
 * to hand over (set) the last stored remanent data to the component.
 * In case the application has no currently stored remanent data for the component,
 * the service shall be initiated with the correct Component ID but a remanent data
 * size of zero. Otherwise, it is not ensured that the component can start properly.
 * In case of a protocol stack the network communication will not start.
 *
 * \note The application shall send the last stored remanent data without matching the size
 *       that the component has reported.
 * \note The service is only usable in case the component is in bus off state.
 * \note In case the component detects an invalid Component ID, the service
 *       shall be confirmed with the error code ERR_HIL_INVALID_COMPONENT_ID.
 * \note In case the component detects an invalid remanent data length,
 *       the service shall be confirmed with the error code ERR_HIL_INVALID_DATA_LENGTH.
 * \note In case the component detects invalid remanent data (inconsistent, wrong data),
 *       the service shall be confirmed with the error code ERR_HIL_INCONSISTENT_DATA_SET.
 * \note In case the component detected an invalid HIL_SET_REMANENT_DATA_REQ service the
 *       component shall act as if it has received HIL_SET_REMANENT_DATA_REQ with
 *       remanent data size of zero. Additionally, a permanent logbook entry shall be generated.
 *
 * \{ */

/*! Set remanent data request data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_REMANENT_DATA_REQ_DATA_Ttag
{
  /*! Unique component identifier HIL_COMPONENT_ID_*. */
  uint32_t  ulComponentId;
  /*! Remanent data buffer. */
  uint8_t   abData[__HIL_VARIABLE_LENGTH_ARRAY];
} HIL_SET_REMANENT_DATA_REQ_DATA_T;

/*! Set remanent data request. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_REMANENT_DATA_REQ_Ttag
{
  HIL_PACKET_HEADER_T               tHead;  /*!< Packet header. */
  HIL_SET_REMANENT_DATA_REQ_DATA_T  tData;  /*!< Packet data. */
} HIL_SET_REMANENT_DATA_REQ_T;

#define HIL_SET_REMANENT_DATA_REQ_SIZE(remanentDataBytes)   (sizeof(uint32_t) +  (remanentDataBytes) * sizeof(uint8_t))



/*! Set remanent data confirmation data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_REMANENT_DATA_CNF_DATA_Ttag
{
  /*! Unique component identifier HIL_COMPONENT_ID_*. */
  uint32_t  ulComponentId;
} HIL_SET_REMANENT_DATA_CNF_DATA_T;

/*! Set remanent data confirmation. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_REMANENT_DATA_CNF_Ttag
{
  HIL_PACKET_HEADER_T              tHead;  /*!< Packet header. */
  HIL_SET_REMANENT_DATA_CNF_DATA_T tData;  /*!< Packet data. */
} HIL_SET_REMANENT_DATA_CNF_T;

#define HIL_SET_REMANENT_DATA_CNF_SIZE   (sizeof(HIL_SET_REMANENT_DATA_CNF_DATA_T))

/*! \} ************************************************************************/




/******************************************************************************/
/*! \addtogroup HIL_STORE_REMANENT_DATA_doc
 *
 * This component generates this indication each time when the remanent data
 * needs to be stored.
 *
 * The remanent indication always contains the complete data block which must be
 * stored remanently. The application shall compare the remanent data with the last
 * stored remanent data in order to avoid writing the same data again and again.
 * It is up to the application to consider the wear of the storage device.
 *
 * \note The application shall be able to store remanent data even if the reported
 *       remanent data size changes (e.g. due to update of component).
 * \note The response should be generated synchronously to remanent data storage,
 *       i.e. after all data has been written to the storage device.
 * \note The response message must always contain at least the Component ID,
 *       even if the service is replied to with an error code.
 * \note Depending on the component a response to the network may
 *       be generated even if the application has not answered yet.
 *
 * \{ */

/*! Store remanent indication data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_STORE_REMANENT_DATA_IND_DATA_Ttag
{
  /*! Unique component identifier HIL_COMPONENT_ID_*. */
  uint32_t  ulComponentId;
  /*! Remanent data buffer. */
  uint8_t   abData[__HIL_VARIABLE_LENGTH_ARRAY];
} HIL_STORE_REMANENT_DATA_IND_DATA_T;

/*! Store remanent indication. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_STORE_REMANENT_DATA_IND_Ttag
{
  HIL_PACKET_HEADER_T                 tHead;  /*!< Packet header. */
  HIL_STORE_REMANENT_DATA_IND_DATA_T  tData;  /*!< Packet data. */
} HIL_STORE_REMANENT_DATA_IND_T;

#define HIL_STORE_REMANENT_DATA_IND_SIZE(remanentDataBytes)   (sizeof(uint32_t) +  (remanentDataBytes) * sizeof(uint8_t))


/*! Store remanent response data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_STORE_REMANENT_DATA_RES_DATA_Ttag
{
  /*! Unique component identifier HIL_COMPONENT_ID_*. */
  uint32_t  ulComponentId;
} HIL_STORE_REMANENT_DATA_RES_DATA_T;

/*! Store remanent response. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_STORE_REMANENT_DATA_RES_Ttag
{
  HIL_PACKET_HEADER_T                 tHead; /*!< Packet header. */
  HIL_STORE_REMANENT_DATA_RES_DATA_T  tData; /*!< Packet data. */
} HIL_STORE_REMANENT_DATA_RES_T;

#define HIL_STORE_REMANENT_DATA_RES_SIZE   (sizeof(HIL_STORE_REMANENT_DATA_RES_DATA_T))

/*! \} ************************************************************************/




/******************************************************************************/
/*! \addtogroup HIL_SET_TRIGGER_TYPE_doc
 *
 * This service offers the application to configure the data exchange trigger mode.
 * The trigger mode defines on which network-specific event the synchronization or
 * provider/consumer data update will be finished by the protocol stack.
 *
 * The synchronization or consumer data update is finished by the protocol stack:
 *  - immediately in free-run mode HIL_TRIGGER_TYPE_*_NONE
 *  - in case a new network connection is opened and new data is received
 *    HIL_TRIGGER_TYPE_*_RX_DATA_RECEIVED (bus cycle synchronous)
 *  - in case a defined time point is reached HIL_TRIGGER_TYPE_*_TIMED_ACTIVATION
 *    (time isochronous). The time point is protocol stack specific.
 *
 * The synchronization or provider data update is finished by the protocol stack:
 *  - immediately in free-run mode HIL_TRIGGER_TYPE_*_NONE
 *  - in case new data on the bus is required. E.g. the protocol stack will delay
 *    the update process until a new network connection is established
 *    HIL_TRIGGER_TYPE_*_READY_FOR_TX_DATA (bus cycle synchronous)
 *  - in case a defined time point is reached HIL_TRIGGER_TYPE_*_TIMED_LATCH
 *    (time isochronous). The time point is protocol stack specific.
 *
 * The configuration of the consumer and provider data update trigger mode are
 * independent from each other and can be used individually or combined.
 * However, the synchronization trigger mode can only be configured unequal to
 * HIL_TRIGGER_TYPE_SYNC_NONE in case both the consumer and provider trigger modes
 * are configured in free-run mode (HIL_TRIGGER_TYPE_*_NONE). In case the trigger mode
 * is not supported by the protocol stack, an error code in the response will be set
 * to signal an invalid configuration.
 *
 * \note In case the protocol stack is configured with a trigger mode unequal to free-run,
 *       it is protocol stack specific at which point of time the synchronization or
 *       provider/consumer data update is finished. E.g. the protocol stack will wait for
 *       a network connection to be established.
 * \note The protocol stack accepts the service in bus off mode. It is protocol stack specific
 *       if the service is accepted in bus on mode.
 * \note In case the application does not use the service, the protocol stack will start in
 *       default trigger mode. The default trigger mode is free-run (HIL_TRIGGER_TYPE_*_NONE).
 * \note On channel initialization the protocol stack keeps the previously configured
 *       trigger mode until active change or device reset.
 * \note The exchange trigger mode is restored to default on delete config.
 * \note The protocol stack which is configured for defined data exchange mode monitors if
 *       the host application handles the handshake as expected. Every time an error symptom
 *       occurs the respective handshake error counter is incremented.
 *       The error counter counts up to the maximal possible value and saturates.
 * \note In case the trigger mode is configured in default mode, the handshake error counters
 *       are set to 0 and do not count.
 * \note The protocol stack resets the handshake error counter to initial value (zero) after
 *       each channel init.
 *
 * \{ */

#define HIL_TRIGGER_TYPE_PDIN_NONE                       0x0010 /*!< No input data synchronization (free-run). */
#define HIL_TRIGGER_TYPE_PDIN_RX_DATA_RECEIVED           0x0011 /*!< Input data will be updated when new data was received. (bus cycle synchronous). */
#define HIL_TRIGGER_TYPE_PDIN_TIMED_ACTIVATION           0x0012 /*!< Input data will be updated on time event (time isochronous). */

#define HIL_TRIGGER_TYPE_PDOUT_NONE                      0x0010 /*!< No output data synchronization (free-run). */
#define HIL_TRIGGER_TYPE_PDOUT_READY_FOR_TX_DATA         0x0011 /*!< Output data will be send in next bus cycle. (bus cycle synchronous). */
#define HIL_TRIGGER_TYPE_PDOUT_TIMED_LATCH               0x0012 /*!< Output data will be delayed until next time event (time isochronous). */

#define HIL_TRIGGER_TYPE_SYNC_NONE                       0x0010 /*!< No sync signal generation */
#define HIL_TRIGGER_TYPE_SYNC_RX_DATA_RECEIVED           0x0011 /*!< Generate Sync event when new data was received. */
#define HIL_TRIGGER_TYPE_SYNC_READY_FOR_TX_DATA          0x0012 /*!< Generate Sync event when new data will be send. */
#define HIL_TRIGGER_TYPE_SYNC_TIMED_LATCH                0x0013 /*!< Generate Sync event when data shall be latched. */
#define HIL_TRIGGER_TYPE_SYNC_TIMED_ACTIVATION           0x0014 /*!< Generate Sync event when data shall be applied. */


/*! Set data exchange trigger data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_TRIGGER_TYPE_REQ_DATA_Ttag
{
  /*! Consumer data trigger type HIL_TRIGGER_TYPE_PDIN_*. */
  uint16_t usPdInHskTriggerType;
  /*! Provider data trigger type HIL_TRIGGER_TYPE_PDOUT_*. */
  uint16_t usPdOutHskTriggerType;
  /*! Synchronization trigger type HIL_TRIGGER_TYPE_SYNC_*. */
  uint16_t usSyncHskTriggerType;
} HIL_SET_TRIGGER_TYPE_REQ_DATA_T;

/*! Set data exchange trigger request. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SET_TRIGGER_TYPE_REQ_Ttag
{
  HIL_PACKET_HEADER_T               tHead;  /*!< Packet header. */
  HIL_SET_TRIGGER_TYPE_REQ_DATA_T   tData;  /*!< Packet data. */
} HIL_SET_TRIGGER_TYPE_REQ_T;

#define HIL_SET_TRIGGER_TYPE_REQ_SIZE (sizeof(HIL_SET_TRIGGER_TYPE_REQ_DATA_T))


/*! Set data exchange trigger confirmation structure. */
typedef HIL_EMPTY_PACKET_T      HIL_SET_TRIGGER_TYPE_CNF_T;

#define HIL_SET_TRIGGER_TYPE_CNF_SIZE   (0)

/*! \} ************************************************************************/




/******************************************************************************/
/*! \addtogroup HIL_GET_TRIGGER_TYPE_doc
 *
 * This service is used by the application to read the current
 * handshake trigger type configured in the protocol stack.
 *
 * \{ */

/*! Get data exchange trigger request. */
typedef HIL_EMPTY_PACKET_T      HIL_GET_TRIGGER_TYPE_REQ_T;


#define HIL_GET_TRIGGER_TYPE_REQ_SIZE   (0)


/*! Get data exchange trigger data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_TRIGGER_TYPE_CNF_DATA_Ttag
{
  /*! Input process data trigger type.
   * Value is a type of HIL_TRIGGER_TYPE_PDIN_*. */
  uint16_t usPdInHskTriggerType;
  /*! Output process data trigger type.
   * Value is a type of HIL_TRIGGER_TYPE_PDOUT_*. */
  uint16_t usPdOutHskTriggerType;
  /*! Synchronization trigger type.
   * Value is a type of HIL_TRIGGER_TYPE_SYNC_*. */
  uint16_t usSyncHskTriggerType;
  /*! Minimal provide/consumer data update interval in free-run mode.
   * The application shall ensure in free-run mode to not request faster
   * provider/consumer data update than this interval.
   * Unit of microseconds, default value is 1000us, value 0-31 is not valid. */
  uint16_t usMinFreeRunUpdateInterval;
} HIL_GET_TRIGGER_TYPE_CNF_DATA_T;

/*! Get data exchange trigger confirmation structure. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GET_TRIGGER_TYPE_CNF_Ttag
{
  HIL_PACKET_HEADER_T               tHead;  /*!< Packet header. */
  HIL_GET_TRIGGER_TYPE_CNF_DATA_T   tData;  /*!< Packet data. */
} HIL_GET_TRIGGER_TYPE_CNF_T;

#define HIL_GET_TRIGGER_TYPE_CNF_SIZE (sizeof(HIL_GET_TRIGGER_TYPE_CNF_DATA_T))

/*! \} ************************************************************************/





/******************************************************************************/
/*! \addtogroup HIL_READ_LOG_BOOK_ENTRIES_doc
 *
 * This service is used by the application to read out the log book
 * of an communication channel.
 *
 * \{ */

/*! Get log book entries request data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  /*! How many of the recent made entries shall be skipped */
  uint32_t ulSkipRecentEntries;
  /*! Number of entries to read.
   * \note Big numbers may lead to fragmentation of the packet. For normal
   *       communication channel mailbox 64 entries can be read without fragmentation. */
  uint32_t ulNumberOfEntriesToRead;
} HIL_READ_LOG_BOOK_ENTRIES_REQ_DATA_T;

/*! Get log book entries request. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  HIL_PACKET_HEADER_T                   tHead; /*!< Packet header. */
  HIL_READ_LOG_BOOK_ENTRIES_REQ_DATA_T  tData; /*!< Packet data. */
} HIL_READ_LOG_BOOK_ENTRIES_REQ_T;

#define HIL_READ_LOG_BOOK_ENTRIES_REQ_SIZE   (sizeof(HIL_READ_LOG_BOOK_ENTRIES_REQ_DATA_T))


/*! Get log book entry data.
 * \note This structure is compatible with HIL_LOGBOOK_ENTRY_T structure, which
 *       is internally used in the firmware. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST {
  /*! Time stamp of the OS in Millisecond when the entry was made */
  uint32_t  ulSystemTicks;

  /*! Severity level of this entry. Levels are defined in the Hil_Logbook.h */
  uint8_t   bLevel;

  /*! This field is reserved for later use, will be set to 0. */
  uint8_t   bReserved;

  /*! The type is used to determine how the following abData filed must be
   * interpreted. Types and related structures are defined in the Hil_Logbook.h */
  uint16_t  usType;

  /*! Data length and format depends on usType */
  uint8_t   abData[16];

} HIL_READ_LOG_BOOK_ENTRY_DATA_T;

/*! Get log book entries confirmation data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  /*! Requested amount of skipped entries */
  uint32_t ulSkipRecentEntries;

  /*! Successfully read entries. Valid in atEntries[] array */
  uint32_t ulNumberOfReadEntries;

  /*! Array of read entries, size is defined by ulNumberOfReadEntries */
  HIL_READ_LOG_BOOK_ENTRY_DATA_T atEntries[__HIL_VARIABLE_LENGTH_ARRAY];

} HIL_READ_LOG_BOOK_ENTRIES_CNF_DATA_T;

/*! Get log book entries confirmation. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  HIL_PACKET_HEADER_T                   tHead; /*!< Packet header. */
  HIL_READ_LOG_BOOK_ENTRIES_CNF_DATA_T  tData; /*!< Packet data. */
} HIL_READ_LOG_BOOK_ENTRIES_CNF_T;

#define HIL_READ_LOG_BOOK_ENTRIES_CNF_SIZE(numberOfEntries)   (2 * sizeof(uint32_t) + (numberOfEntries) * sizeof(HIL_LOGBOOK_ENTRY_T))

/*! \} ************************************************************************/





/******************************************************************************/
/*! \addtogroup HIL_GET_LOG_BOOK_LAYOUT_doc
 *
 * Get the amount of entries that the logbook can hold. Permanent entries
 * will not be overwritten with new entries.
 *
 * \{ */

/*! Get log book layout request. */
typedef HIL_EMPTY_PACKET_T      HIL_GET_LOG_BOOK_LAYOUT_REQ_T;

#define HIL_GET_LOG_BOOK_LAYOUT_REQ_SIZE   (0)


/*! Get log book layout confirmation data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  /*! Total number of entries the logbook can hold */
  uint32_t ulNumberOfEntries;
  /*! Number of entries which will no be overwritten by new ones */
  uint32_t ulPermanentEntries;

} HIL_GET_LOG_BOOK_LAYOUT_CNF_DATA_T;

/*! Get log book layout confirmation. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  HIL_PACKET_HEADER_T                tHead; /*!< Packet header. */
  HIL_GET_LOG_BOOK_LAYOUT_CNF_DATA_T tData; /*!< Packet data. */
} HIL_GET_LOG_BOOK_LAYOUT_CNF_T;

#define HIL_GET_LOG_BOOK_LAYOUT_CNF_SIZE   (sizeof(HIL_GET_LOG_BOOK_LAYOUT_CNF_DATA_T))

/*! \} ************************************************************************/





/******************************************************************************/
/*! \addtogroup HIL_CLEAR_LOG_BOOK_doc
 *
 * Clears all logbook entries. This also effects the Permanent entries.
 *
 * \{ */


/*! Clear log book entries request. */
typedef HIL_EMPTY_PACKET_T      HIL_CLEAR_LOG_BOOK_REQ_T;

#define HIL_CLEAR_LOG_BOOK_REQ_SIZE   (0)


/*! Clear log book entries confirmation. */
typedef HIL_EMPTY_PACKET_T      HIL_CLEAR_LOG_BOOK_CNF_T;

#define HIL_CLEAR_LOG_BOOK_CNF_SIZE   (0)

/*! \} ************************************************************************/





/******************************************************************************/
/*! \addtogroup HIL_SET_LOG_BOOK_SEVERITY_LEVEL_doc
 *
 * Set the severity levels which shall be logged.
 *
 * \{ */

/*! Set the severity levels request data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  /*! Bit list of severity levels which shall be logged.
   * Set related bit position to 1 to enable logging of level, or set to 0
   * to disable related level.
   * e.g. ulLevelsToBeLogged = (1<<HIL_LOGBOOK_SEVERITY_LEVEL_CRITICAL) | (1<<HIL_LOGBOOK_SEVERITY_LEVEL_WARNING) */
  uint32_t ulLevelsToBeLogged;
} HIL_SET_LOG_BOOK_SEVERITY_LEVEL_REQ_DATA_T;

/*! Set the severity levels request. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST
{
  HIL_PACKET_HEADER_T                         tHead; /*!< Packet header. */
  HIL_SET_LOG_BOOK_SEVERITY_LEVEL_REQ_DATA_T  tData; /*!< Packet data. */
} HIL_SET_LOG_BOOK_SEVERITY_LEVEL_REQ_T;

#define HIL_SET_LOG_BOOK_SEVERITY_LEVEL_REQ_SIZE   (sizeof(HIL_SET_LOG_BOOK_SEVERITY_LEVEL_REQ_DATA_T))


/*! Set the severity levels confirmation. */
typedef HIL_EMPTY_PACKET_T      HIL_SET_LOG_BOOK_SEVERITY_LEVEL_CNF_T;

#define HIL_SET_LOG_BOOK_SEVERITY_LEVEL_CNF_SIZE   (0)

/*! \} ************************************************************************/


#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_UNPACK_1(HIL_APPLICATIONCMD)
#endif

#endif /* HIL_APPLICATIONCMD_H_ */
