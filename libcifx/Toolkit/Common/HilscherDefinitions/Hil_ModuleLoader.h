/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_ModuleLoader.h $: *//*!

  \file Hil_ModuleLoader.h

  Module Loader API (will be part of HIL_SYSTEM task).

**************************************************************************************/
#ifndef HIL_MODULELOADER_H_
#define HIL_MODULELOADER_H_

#include "Hil_Types.h"
#include "Hil_Packet.h"


/** queue name (RX_SYSTEM queue) */

/** Module Loader handles this many modules at most (TODO: tbd) */
#define HIL_MODLOAD_MAX_MODULES       10

/** Maximum file name length (TODO: tbd) */
#define HIL_MODLOAD_MAX_FILENAME      32

/* pragma pack */
#ifdef PRAGMA_PACK_ENABLE
  #pragma PRAGMA_PACK_1(HIL_MODULELOADER_PUBLIC)
#endif

/****************************************************************************************
* Module Loader command codes  */

/** Start of the reserved area from 0x4B00 - 0x4BFF for MODLOAD commands */
#define HIL_MODLOAD_PACKET_COMMAND_START                  0x00004B00

/** Get a module list */
#define HIL_MODLOAD_CMD_QUERY_MODULES_REQ             (HIL_MODLOAD_PACKET_COMMAND_START + 0)
#define HIL_MODLOAD_CMD_QUERY_MODULES_CNF             (HIL_MODLOAD_PACKET_COMMAND_START + 1)

/** Run a module */
#define HIL_MODLOAD_CMD_RUN_MODULE_REQ                (HIL_MODLOAD_PACKET_COMMAND_START + 2)
#define HIL_MODLOAD_CMD_RUN_MODULE_CNF                (HIL_MODLOAD_PACKET_COMMAND_START + 3)

/** Load module from filesystem */
#define HIL_MODLOAD_CMD_LOAD_MODULE_REQ               (HIL_MODLOAD_PACKET_COMMAND_START + 4)
#define HIL_MODLOAD_CMD_LOAD_MODULE_CNF               (HIL_MODLOAD_PACKET_COMMAND_START + 5)

/** Load and run module from filesystem */
#define HIL_MODLOAD_CMD_LOAD_AND_RUN_MODULE_REQ       (HIL_MODLOAD_PACKET_COMMAND_START + 6)
#define HIL_MODLOAD_CMD_LOAD_AND_RUN_MODULE_CNF       (HIL_MODLOAD_PACKET_COMMAND_START + 7)

/* download module (will be done via HIL_SYSTEM Download mechanism (Download type FILE_TYPE_MODULE)) */
/* download and run module (will be done via HIL_SYSTEM Download mechanism (Download type FILE_TYPE_RUN_MODULE)) */


/****************************************************************************************
* Module Loader structs */

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODULE_VERSION_Ttag
{
  uint32_t                        usMajor;
  uint32_t                        usMinor;
  uint32_t                        usRevision;
  uint32_t                        usBuild;
} HIL_MODULE_VERSION_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODULE_NAME_Ttag
{
  uint8_t                         bNameLength;
  uint8_t                         abName[15];
} HIL_MODULE_NAME_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODULE_DATE_Ttag
{
  uint32_t                        usYear;
  uint8_t                         bMonth;
  uint8_t                         bDay;
} HIL_MODULE_DATE_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODULE_ENTRY_Ttag
{
  HIL_MODULE_NAME_T               tFileName;                    /*!< file name of module */
} HIL_MODULE_ENTRY_T;


#define HIL_MODLOAD_MAX_ENTRIES_PER_PACKET        (1024 / sizeof(HIL_MODULE_ENTRY_T))

/****************************************************************************************
 * Packet: HIL_MODLOAD_CMD_QUERY_MODULES_REQ/HIL_MODLOAD_CMD_QUERY_MODULES_CNF
 *
 * Query modules list
 *
 */

/**** Request Packet ****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODLOAD_QUERY_MODULES_REQ_DATA_Ttag
{
  uint32_t                        ulListType;
} HIL_MODLOAD_QUERY_MODULES_REQ_DATA_T;

/* List Types */
#define HIL_MODLOAD_QUERY_MODULES_REQ_LIST_TYPE_STORAGE       0x00000000   /*!< static storage, like CF file system */
#define HIL_MODLOAD_QUERY_MODULES_REQ_LIST_TYPE_LOADED        0x00000001   /*!< currently loaded modules */
#define HIL_MODLOAD_QUERY_MODULES_REQ_LIST_TYPE_RUNNING       0x00000002   /*!< currently running modules */


typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODLOAD_QUERY_MODULES_REQ_Ttag
{
  /** packet header */
  HIL_PACKET_HEADER_T                           tHead;

  /** packet data */
  HIL_MODLOAD_QUERY_MODULES_REQ_DATA_T          tData;
} HIL_MODLOAD_QUERY_MODULES_REQ_T;



/**** Confirmation Packet ****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODLOAD_QUERY_MODULES_CNF_DATA_Ttag
{
  HIL_MODULE_ENTRY_T                atModules[HIL_MODLOAD_MAX_ENTRIES_PER_PACKET];
} HIL_MODLOAD_QUERY_MODULES_CNF_DATA_T;


typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODLOAD_QUERY_MODULES_CNF_Ttag
{
  /** packet header */
  HIL_PACKET_HEADER_T                           tHead;
  /** packet data */
  HIL_MODLOAD_QUERY_MODULES_CNF_DATA_T          tData;
} HIL_MODLOAD_QUERY_MODULES_CNF_T;


/****************************************************************************************
 * Packet: HIL_MODLOAD_CMD_RUN_MODULE_REQ/HIL_MODLOAD_CMD_RUN_MODULE_CNF
 *
 * Run a module which has been loaded
 */


/**** Request Packet ****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODLOAD_RUN_MODULE_REQ_DATA_Ttag
{
  /** channel parameter for modules supporting multi-instantiation */
  uint32_t                                      ulChannel;
  /* module name follows with NUL termination */
} HIL_MODLOAD_RUN_MODULE_REQ_DATA_T;


typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODLOAD_RUN_MODULE_REQ_Ttag
{
  /** packet header */
  HIL_PACKET_HEADER_T                           tHead;
  /** packet data */
  HIL_MODLOAD_RUN_MODULE_REQ_DATA_T             tData;
} HIL_MODLOAD_RUN_MODULE_REQ_T;



/**** Confirmation Packet ****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODLOAD_RUN_MODULE_CNF_Ttag
{
  /** packet header */
  HIL_PACKET_HEADER_T                           tHead;
} HIL_MODLOAD_RUN_MODULE_CNF_T;

/****************************************************************************************
 * Packet: HIL_MODLOAD_CMD_LOAD_MODULE_REQ/HIL_MODLOAD_CMD_LOAD_MODULE_CNF
 *
 * Load a module from file system
 */


/**** Request Packet ****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODLOAD_LOAD_MODULE_REQ_DATA_Ttag
{
  /** channel parameter for modules supporting multi-instantiation */
  uint32_t                                      ulChannel;
  /* file name (including path) follows with NUL termination */
  /* XXX mgr: dynamic lenght structs are evil - why not use HIL_MODULE_NAME_T? */
} HIL_MODLOAD_LOAD_MODULE_REQ_DATA_T;


typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODLOAD_LOAD_MODULE_REQ_Ttag
{
  /** packet header */
  HIL_PACKET_HEADER_T                           tHead;
  /** packet data */
  HIL_MODLOAD_LOAD_MODULE_REQ_DATA_T            tData;
} HIL_MODLOAD_LOAD_MODULE_REQ_T;



/**** Confirmation Packet ****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODLOAD_LOAD_MODULE_CNF_Ttag
{
  /** packet header */
  HIL_PACKET_HEADER_T                           tHead;
} HIL_MODLOAD_LOAD_MODULE_CNF_T;


/****************************************************************************************
 * Packet: HIL_MODLOAD_CMD_LOAD_AND_RUN_MODULE_REQ/HIL_MODLOAD_CMD_LOAD_AND_RUN_MODULE_CNF
 *
 * Run a loaded module
 */


/**** Request Packet ****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODLOAD_LOAD_AND_RUN_MODULE_REQ_DATA_Ttag
{
  /** channel parameter for modules supporting multi-instantiation.
   * if it doen't, this value has to match the actual channel supported
   * by the module
   */
  uint32_t                                      ulChannel;
  /* file name (including path) follows with NUL termination */
  /* XXX mgr: dynamic lenght structs are evil - why not use HIL_MODULE_NAME_T? */
} HIL_MODLOAD_LOAD_AND_RUN_MODULE_REQ_DATA_T;


typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODLOAD_LOAD_AND_RUN_MODULE_REQ_Ttag
{
  /** packet header */
  HIL_PACKET_HEADER_T                           tHead;
  /** packet data */
  HIL_MODLOAD_LOAD_AND_RUN_MODULE_REQ_DATA_T    tData;
} HIL_MODLOAD_LOAD_AND_RUN_MODULE_REQ_T;



/**** Confirmation Packet ****/

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_MODLOAD_LOAD_AND_RUN_MODULE_CNF_Ttag
{
  /** packet header */
  HIL_PACKET_HEADER_T                           tHead;
} HIL_MODLOAD_LOAD_AND_RUN_MODULE_CNF_T;


/*************************************************************************************/

/* pragma unpack */
#ifdef PRAGMA_PACK_ENABLE
#pragma PRAGMA_UNPACK_1(HIL_MODULELOADER_PUBLIC)
#endif

#endif /* HIL_MODULELOADER_H_ */
