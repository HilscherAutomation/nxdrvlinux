/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20240319-00/includes/Hil_FileHeaderV4.h $: *//*!

  \file Hil_FileHeaderV4.h

  Hilscher File Header V4.0.

**************************************************************************************/
#ifndef HIL_FILEHEADERV4_H_
#define HIL_FILEHEADERV4_H_

#include <stdint.h>
#include "Hil_Compiler.h"
#include "Hil_SharedDefines.h"

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_PACK_1(HIL_FILEHEADERV4)
#endif


/*****************************************************************************/
/* Constant Definitions for MCU Boot Headers                                 */
/*****************************************************************************/
#define HIL_MCU_BOOT_MAGIC           0x96f3b83d /**< cookie to identify MCU boot header        */

#define HIL_TLV_INFO_MAGIC            0x6907     /**< cookie to identify TLV info area        */
#define HIL_TLV_PROT_INFO_MAGIC       0x6908     /**< cookie to identify protected info area  */

#define HIL_TLV_TYPE_KEYHASH          0x01       /**< hash of the public key */
#define HIL_TLV_TYPE_PUBKEY           0x02       /**< public key */
#define HIL_TLV_TYPE_SHA256           0x10       /**< SHA256 of image hdr and body */
#define HIL_TLV_TYPE_SHA384           0x11       /**< SHA384 of image hdr and body */

/*****************************************************************************/
/* Constant Definitions for Hilscher File Headers                            */
/*****************************************************************************/

/* Common Header version constants */
#define HIL_VERSION_COMMON_HEADER_4_0       0x00040000 /**< V4.0, rework for netX9xx*/

/* Device Info structure version constants */
#define HIL_VERSION_DEVICE_INFO_V2_0        0x00020000 /**< V2.0, version used with Common Header V4.0 */


/* File header cookies (low order byte is first byte in memory) */
#define HIL_IMAGE_TYPE_NXI_COOKIE   0x49584E2E /**< used in NXI communication firmware files ".NXI"            */
#define HIL_IMAGE_TYPE_NXE_COOKIE   0x45584E2E /**< used in NXE communication firmware files ".NXE"            */
#define HIL_IMAGE_TYPE_NAI_COOKIE   0x49414E2E /**< used in NAI application firmware files ".NAI"              */
#define HIL_IMAGE_TYPE_NAE_COOKIE   0x45414E2E /**< used in NAE application firmware files ".NAE"              */

#define HIL_IMAGE_TYPE_NMI_COOKIE   0x494D4E2E /**< used in NMI motion firmware files ".NMI"                   */
#define HIL_IMAGE_TYPE_NSI_COOKIE   0x49534E2E /**< used in NSI security enclave firmware files ".NSI"         */
#define HIL_IMAGE_TYPE_HWC_COOKIE   0x4357482E /**< used in HWC hardware configuration files ".HWC"            */
#define HIL_IMAGE_TYPE_HWO_COOKIE   0x4F57482E /**< used in HWO hardware configuration overlay files ".HWO"    */

#define HIL_IMAGE_TYPE_FWU_COOKIE   0x5557462E /**< used in FWU firmware update container files ".FWU"         */

#define HIL_IMAGE_TYPE_NXD_COOKIE   0x44584E2E /**< used in NXD configuration database files ".NXD"            */
#define HIL_IMAGE_TYPE_NXL_COOKIE   0x4C584E2E /**< used in NXL license update files ".NXL"                    */

/** MCU Boot header (32 bytes) */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_MCU_BOOT_HEADER_Ttag
{
  uint32_t   ulIhMagic;
  uint32_t   ulIhLoadAddr;
  uint16_t   usIhHdrSize;
  uint16_t   usIhProtectedTlvSize;
  uint32_t   ulIhImgSize;
  uint32_t   ulIhFlags;
  uint8_t    bIvMajor;
  uint8_t    bIvMinor;
  uint16_t   usIvRevision;
  uint32_t   ulIvBuildNum;
  uint32_t   ulReserved;
} HIL_FILE_MCU_BOOT_HEADER_T, *PHIL_FILE_MCU_BOOT_HEADER_T;

/** COMMON header (32 bytes) */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_COMMON_HEADER_V4_0_Ttag
{
  uint32_t   ulHeaderVersion;              /**< structure version (major, minor), 0x00040000                                */
  uint32_t   ulHeaderLength;               /**< length of common header + device info header                                */
  uint32_t   ulImageType;                  /**< type of the image file                                                      */
  uint32_t   ulTagListOffset;              /**< offset of tag list (from beginning of firmware file)                        */
  uint32_t   ulTagListSize;                /**< tag list length in bytes (0 = no tag list)                                  */
  uint32_t   ulFirmwareID;                 /**< Unique number for nxi/nxe or nai/nae compability check                      */
  uint32_t   aulReserved[2];               /**< reserved, set to zero                                                       */
} HIL_FILE_COMMON_HEADER_V4_0_T, *PHIL_FILE_COMMON_HEADER_V4_0_T;


/** DEVICE-specific information (32 bytes) */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DEVICE_INFO_V2_0_Ttag
{
  uint32_t  ulStructVersion;              /**< structure version (major, minor), 0x00020000      */
  uint16_t  usManufacturer;               /**< manufacturer ID (see DPM Manual)                  */
  uint16_t  usDeviceClass;                /**< netX device class                                 */
  uint8_t   bHwCompatibility;             /**< hardware compatibility ID                         */
  uint8_t   bChipType;                    /**< see HIL_DEV_CHIP_TYPE_xxx definitions             */
  uint16_t  usReserved;                   /**< reserved, set to zero                             */
  uint16_t  ausHwOptions[4];              /**< required hardware assembly options (0=not used)   */
  uint32_t  aulReserved[3];               /**< reserved, set to zero                             */
}  HIL_FILE_DEVICE_INFO_V2_0_T, *PHIL_FILE_DEVICE_INFO_V2_0_T;



/** file header V4*/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_HEADER_V4_0_Ttag
{
  HIL_FILE_MCU_BOOT_HEADER_T      tMcuBootHeader;      /**< mcu boot header                              */
  HIL_FILE_COMMON_HEADER_V4_0_T   tCommonHeader;       /**< common header                                */
  HIL_FILE_DEVICE_INFO_V2_0_T     tDeviceInfo;         /**< device-specific information                  */
} HIL_FILE_HEADER_V4_0_T, *PHIL_FILE_HEADER_V4_0_T;


/*****************************************************************************/
/* MCU Boot image TLV structures                                             */
/*****************************************************************************/
/** TLV info.  All fields in little endian. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_MCU_BOOT_TLV_INFO_Ttag {
    uint16_t usItMagic;       /**< Cookie to identify the TLV Area (HIL_TLV_INFO_MAGIC or HIL_TLV_PROT_INFO_MAGIC) */
    uint16_t usItTlvTot;      /**< Size of TLV area (including tlv_info) */
} HIL_FILE_MCU_BOOT_TLV_INFO_T, *PHIL_FILE_MCU_BOOT_TLV_INFO_Ttag;

/** TLV entry format. All fields in little endian. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_MCU_BOOT_TLV_Ttag {
    uint16_t usItType;      /**< HIL_TLV_TYPE_[...]. */
    uint16_t usItLen;       /**< Value length (not including TLV header). */
    /* TLV value */
} HIL_FILE_MCU_BOOT_TLV_T, *PHIL_FILE_MCU_BOOT_TLV_T;

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_UNPACK_1(HIL_FILEHEADERV4)
#endif

#endif /*  HIL_FILEHEADERV4_H_ */
