/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_FirmwareIdent.h $: *//*!

  \file Hil_FirmwareIdent.h

  Definitions of Firmware Identification

**************************************************************************************/
#ifndef HIL_FIRMWAREIDENT_H_
#define HIL_FIRMWAREIDENT_H_

#include <stdint.h>
#include "Hil_Compiler.h"

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_PACK_1(HIL_FIRMWAREIDENT)
#endif

/*****************************************************************************/
/* Set byte alignment for structure members. */
/*****************************************************************************/


typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FW_VERSION_Ttag
{
  uint16_t    usMajor;
  uint16_t    usMinor;
  uint16_t    usBuild;
  uint16_t    usRevision;
} HIL_FW_VERSION_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FW_NAME_Ttag
{
  uint8_t bNameLength;
  uint8_t abName[ 63 ];
} HIL_FW_NAME_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FW_DATE_Ttag
{
  uint16_t  usYear;
  uint8_t   bMonth;
  uint8_t   bDay;
} HIL_FW_DATE_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FW_IDENTIFICATION_Ttag
{
  HIL_FW_VERSION_T tFwVersion;                           /*!< firmware version */
  HIL_FW_NAME_T    tFwName;                              /*!< firmware name    */
  HIL_FW_DATE_T    tFwDate;                              /*!< firmware date    */
} HIL_FW_IDENTIFICATION_T;

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_UNPACK_1(HIL_FIRMWAREIDENT)
#endif

#endif /* HIL_FIRMWAREIDENT_H_ */
