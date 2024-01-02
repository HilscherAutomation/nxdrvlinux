/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_Logbook.h $: *//*!

  \file Hil_Logbook.h

  Definitions for the Hilscher firmware Logbook.

  \note In this file the Type IDs will be managed (HIL_LOGBOOK_ENTRY_TYPE_*) definitions
        are only made here. There is no structure to the type values, the next value is
        acquired directly after the last one. It is not desired to reserve numbers, this
        shall avoid dead number ranges and gaps between the types.

  \note This list is maintained by the protocol stack department (SPC).

  In the description the python like format strings will be used to give information over
  a possible presentation. (https://docs.python.org/3.4/library/string.html#formatspec)
  the parameter will by "selected" by position e.g.
    "{0} {1}" -> first and second member of structure.

**************************************************************************************/
#ifndef HIL_LOGBOOK_H_
#define HIL_LOGBOOK_H_

#include <stdint.h>
#include "Hil_Compiler.h"

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_PACK_1(HIL_LOGBOOK)
#endif

typedef enum {
  /*! The entry is not valid and shall not be evaluated */
  HIL_LOGBOOK_ENTRY_TYPE_INVALID                       = 0x0000,

  /*! The first 15 codes are reserved for internal use. */
  HIL_LOGBOOK_ENTRY_TYPE_RESERVED_BLOCK_END            = 0x000F,


  HIL_LOGBOOK_ENTRY_TYPE_LABEL                         = 0x0010, /*!< ASCII label */
  HIL_LOGBOOK_ENTRY_TYPE_DPM_COMMON_STATUS             = 0x0011, /*!< The common status block have been changed */
  /* HIL_LOGBOOK_ENTRY_TYPE_DPM_MASTER_STATUS          = 0x0012, */
  HIL_LOGBOOK_ENTRY_TYPE_PACKET_ISSUE                  = 0x0013, /*!< Information to a received packet/service  */
  HIL_LOGBOOK_ENTRY_TYPE_SDO_FAILURE                   = 0x0014, /*!< SDO Configuration issue occurred */
  HIL_LOGBOOK_ENTRY_TYPE_CFGMGR_TLV                    = 0x0015, /*!< Config Manager TLV */


  /*! The entry is not valid and shall not be evaluated */
  HIL_LOGBOOK_ENTRY_TYPE_UNKNOWN                       = 0xFFFF
} HIL_LOGBOOK_ENTRY_TYPE_E;

/* Type definition for public APIs of the HIL_LOGBOOK_ENTRY_TYPE_E enumeration. */
typedef uint16_t HIL_LOGBOOK_ENTRY_TYPE_T;


/* Severity levels */
typedef enum {
  /*! Critical conditions.
   * This entry severity indicates a situation which very likely leads to undefined
   * system behavior. Entries of this type should not be present in a system. Such entry
   * type should definitely be evaluated and analyzed.
   * e.g. System Out of resources, component could not be started. */
  HIL_LOGBOOK_SEVERITY_LEVEL_CRITICAL      = 2,

  /*! Warning conditions.
   * Warning conditions are shown if something happens which is unexpected or suspicious
   * for a normal operations but is not a serious error, because the device can handle
   * the situation.
   * Under normal operation such entry's are not likely to appear, but they can indicate
   * faulty configuration or illegal request which are made by other components.
   * e.g. Illegal request was issued, wrong configuration,
   * No remanent data available (may be fist start of the device?). */
  HIL_LOGBOOK_SEVERITY_LEVEL_WARNING       = 4,

  /*! Informational conditions.
   * Informational entries can be helpful to verify the behavior of the device. They
   * do not indicate an issue at all.
   * e.g. some state was reached, Request was handled (e.g. request was received form
   * the network). */
  HIL_LOGBOOK_SEVERITY_LEVEL_INFOMATIONAL  = 6,

} HIL_LOGBOOK_SEVERITY_LEVEL_E;

/*! Type definition for public APIs of the HIL_LOGBOOK_SEVERITY_LEVEL_E enumeration. */
typedef uint8_t HIL_LOGBOOK_SEVERITY_LEVEL_T;


/*! General definition of a Logbook entry */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST {
  /*! Time stamp of the OS in Millisecond when the entry was made */
  uint32_t                    ulSystemTicks;

  /*! Severity level of this entry */
  HIL_LOGBOOK_SEVERITY_LEVEL_T bLevel;

  /*! This field is reserved for later use, will be set to 0. */
  uint8_t                     bReserved;

  /*! The type is used to determine how the following abData filed must be
   * interpreted. Types and related structures are defined in the Hil_Logbook.h */
  HIL_LOGBOOK_ENTRY_TYPE_T     usType;

  /*! Data length and format depends on usType */
  uint8_t                     abData[16];

} HIL_LOGBOOK_ENTRY_T;


/*! Structure definition for HIL_LOGBOOK_ENTRY_TYPE_LABEL entries.
 * The label can be used to mark important events, which not contain any additional information.
 * The created label should contain a information who was the creator of the Label (e.g.
 * "PN:Signaling", "DLR:Ring open", "S3:CableBrake", ...).
 * Used Labels and there meaning will be described in the Manual of the related Firmware.
 *
 * Only Printable characters Codes 20hex to 7Ehex shall be used. Not used characters, at the end
 * of the Label, can be set to zero (00hex) or filled with spaces (20hex).
 *
 * \note  The entries shall be at least kind of human readable. In addition only English
 *        words and acronym shall be used.
 *
 * Notification string:
 *  "Label {0:<14}"
 **/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST {
  char  abLabel[14]; /*!< An ASCII label with fix length, not zero terminated. */

} HIL_LOGBOOK_ENTRY_TYPE_LABEL_T;


/*! Structure definition for HIL_LOGBOOK_ENTRY_TYPE_DPM_COMMON_STATUS entries.
 * A more detailed description of the values can be found in the DPM manual.
 *
 * Notification string:
 *  "The common status block was updated. COS {0:#010x}, COM State {1:#010x}, COM Error {2:#010x}"
 **/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST {
  uint32_t  ulCommunicationCOS;   /*!< Communication channel "Change Of State" */
  uint32_t  ulCommunicationState; /*!< Actual communication state */
  uint32_t  ulCommunicationError; /*!< Actual communication error */

} HIL_LOGBOOK_ENTRY_TYPE_DPM_COMMON_STATUS_T;


/*! Structure definition for HIL_LOGBOOK_ENTRY_TYPE_PACKET_ISSUE entries.
 * A packet have raise a issue within a component.
 *
 * \note: Do not generate an entry if you receive a packet with a already set
 *        status code. The component how had trouble with this packet shall
 *        made the entry.
 *
 * Notification string:
 *  "The command {0:#010x} from {1:#010x} raised an issue with the code {3:#010x}"
 **/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST {
  uint32_t ulCmd; /*!< Operation command code of the packet. */
  uint32_t ulSrc; /*!< Source of the packet which cause an issue. */
  uint32_t ulSta; /*!< Status code */

} HIL_LOGBOOK_ENTRY_TYPE_DPM_PACKET_ISSUE_T;


/*! Structure definition for HIL_LOGBOOK_ENTRY_TYPE_SDO_FAILURE entries.
 * An SDO transfer to a node failed.
 *
 * Notification string:
 *  "The SDO transfer to node {0:#06x} object {1:#06x}/{2:#04x} (Info flags:{3:08b}) failed with error code {5:#010x}"
 **/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST {
  uint16_t usNodeId;        /*!< Node id which issued the failure */
  uint16_t usIndex;         /*!< Index of object */
  uint8_t  bSubIndex;       /*!< Sub-index of object */
  uint8_t  bInfoFlags;      /*!< Info flags: Bit 0 - Complete access */
  uint8_t  abReserved[2];   /*!< Reserved for future or internal use */
  uint32_t ulSdoAbortCode;  /*!< SDO abort code */

} HIL_LOGBOOK_ENTRY_TYPE_SDO_FAILURE_T;

/*! Structure definition for HIL_LOGBOOK_ENTRY_TYPE_CFGMGR_TLV entries.
 * A TLV entry have raise an event within the Config Manager.
 *
 * Notification string:
 *  "The TLV type {0:#06x} with length {1:d} at index {2:#010x} raised an issue with the code {3:#010x}"
 **/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST {
  uint16_t usType;        /*!< TLV type */
  uint16_t usLength;      /*!< Length of TLV entry */
  uint32_t ulIndex;       /*!< Index of TLV within a config data stream */
  uint32_t ulSta;         /*!< Status code */
  uint8_t  abReserved[4]; /*!< Reserved for future or internal use */

} HIL_LOGBOOK_ENTRY_TYPE_CFGMGR_TLV_T;

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_UNPACK_1(HIL_LOGBOOK)
#endif

#endif /* HIL_LOGBOOK_H_ */
