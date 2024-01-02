/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_FileHeaderNXS.h $: *//*!

  \file Hil_FileHeaderNXS.h

  File Header for NXS files.

**************************************************************************************/
#ifndef HIL_FILEHEADERNXS_H_
#define HIL_FILEHEADERNXS_H_

#include <stdint.h>
#include "Hil_Compiler.h"

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST BINDING_Ttag
{
  uint32_t                    ulUniqueId[3];
  uint32_t                    ulAnchors[4];
  uint32_t                    ulUniqueIdMask[3];
  uint32_t                    ulAnchorsMask[4];
} BINDING_T;

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST SEC_INFO_Ttag
{
  uint16_t                    usOptions;
  uint16_t                    usUsedKeys;
  BINDING_T                   tBindingCom;
  BINDING_T                   tBindingApp;
  uint8_t                     abSignature[512];
} SEC_INFO_T;

/** NXS file header (644 bytes)
  This structure contains the complete NXS file header as it can be found as binary file. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST FILE_NXS_HEADER_V1_0_Ttag
{
  uint32_t                    ulMagicCookie;        /**< HIL_FILE_HEADER_NXS_COOKIE                     */
  uint32_t                    ulHeaderVersion;      /**< structure version (major, minor), 0x00010000   */
  uint32_t                    ulFileLength;         /**< file length (header + data)                    */
  SEC_INFO_T                  tSigInfo;             /**< signature parameter                            */
  uint32_t                    ulHeaderCrc32;        /**< CRC32 of NXS header                            */
} FILE_NXS_HEADER_V1_0_T;

#endif /*  HIL_FILEHEADERNXS_H_ */
