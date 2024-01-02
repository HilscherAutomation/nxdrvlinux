/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_Types.h $: *//*!

  \file Hil_Types.h

  Hilscher Type Definition.

**************************************************************************************/
#ifndef HIL_TYPES_H_
#define HIL_TYPES_H_


#include <stdint.h>
#include "Hil_Compiler.h"

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_PACK_1(HIL_TYPES)
#endif

/** UUID */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_UUID_Ttag
{
  uint32_t  ulData1;
  uint16_t  usData2;
  uint16_t  usData3;
  uint8_t   abData4[8];
} HIL_UUID_T;


/** task UUID with special meaning of the elements */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_TASK_UID_Ttag
{
  uint32_t        ulProtocolType;         /*!< use any arbitrary value */
  uint16_t        usMajorVersion;         /*!< major number of the task (or stack) version */
  uint16_t        usTaskType;             /*!< see HIL_TASK_UID_TASK_xxx below */
  uint32_t        ulLayerLevel;           /*!< layer number (per the OSI model) */
  uint32_t        ulLayerSubTask;         /*!< subtask number, e.g. in case of multiple channels */
} HIL_TASK_UID_T;

/*********************** Task Types for HIL_TASK_UID_T ***********************/
/** task type not set */
#define HIL_TASK_UID_TASK_TYPE_INVALID          (0x0000)
/** user application task */
#define HIL_TASK_UID_TASK_TYPE_USER             (0x0001)
/** task belonging to a communication protocol stack */
#define HIL_TASK_UID_TASK_TYPE_PROTOCOL_STACK   (0x0002)
/** task belonging to the rcX operating system */
#define HIL_TASK_UID_TASK_TYPE_RCX              (0x0003)
/** task belonging to the Windows CE operating system (legacy only) */
#define HIL_TASK_UID_TASK_TYPE_WINCE            (0x0004)
/** XPEC channel that has not yet been allocated by a protocol stack (legacy only) */
#define HIL_TASK_UID_TASK_TYPE_XPEC             (0x0005)


#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_UNPACK_1(HIL_TYPES)
#endif

#endif  /* HIL_TYPES_H_ */
