/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_FileHeaderV3.h $: *//*!

  \file Hil_FileHeaderV3.h

  Hilscher File Header V3.0.

**************************************************************************************/
#ifndef HIL_FILEHEADERV3_H_
#define HIL_FILEHEADERV3_H_

#include <stdint.h>
#include "Hil_Compiler.h"
#include "Hil_SharedDefines.h"

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_PACK_1(HIL_FILEHEADERV3)
#endif

/*****************************************************************************/
/* Constant Definitions for Hilscher File Headers                            */
/*****************************************************************************/

/* File header cookies (low order byte is first byte in memory) */
#define HIL_FILE_HEADER_FIRMWARE_COOKIE       0xF8BEAF00 /**< used in NXF firmware or custom loadables */
#define HIL_FILE_HEADER_FIRMWARE_8_COOKIE     0xF8BEAF08 /**< used in NXF firmware 8bit parallel flash */
#define HIL_FILE_HEADER_FIRMWARE_16_COOKIE    0xF8BEAF16 /**< used in NXF firmware 16bit parallel flash */
#define HIL_FILE_HEADER_FIRMWARE_32_COOKIE    0xF8BEAF32 /**< used in NXF firmware 32bit parallel flash */
#define HIL_FILE_HEADER_FIRMWARE_NXI_COOKIE   0x49584E2E /**< used in NXI communication firmware files ".NXI" */
#define HIL_FILE_HEADER_FIRMWARE_NXE_COOKIE   0x45584E2E /**< used in NXE communication firmware files ".NXE" */
#define HIL_FILE_HEADER_FIRMWARE_MXF_COOKIE   0x46584D2E /**< used in MXF maintenance firmware files ".MXF" */
#define HIL_FILE_HEADER_FIRMWARE_NAI_COOKIE   0x49414E2E /**< used in NAI application firmware files ".NAI" */
#define HIL_FILE_HEADER_FIRMWARE_NAE_COOKIE   0x45414E2E /**< used in NAE application firmware files ".NAE" */

#define HIL_FILE_HEADER_MODULE_COOKIE         0x4D584E2E /**< ".NXM" */
#define HIL_FILE_HEADER_OPTION_COOKIE         0x4F584E2E /**< ".NXO" */
#define HIL_FILE_HEADER_DATABASE_COOKIE       0x44584E2E /**< ".NXD" */
#define HIL_FILE_HEADER_LICENSE_COOKIE        0x4C584E2E /**< ".NXL" */
#define HIL_FILE_HEADER_BINARY_COOKIE         0x42584E2E /**< ".NXB" */
#define HIL_FILE_HEADER_NXS_COOKIE            0x53584E2E /**< ".NXS" */

/* Valid Hilscher file extensions */
#define HIL_FILE_EXTENSION_NXF_FIRMWARE     ".NXF"  /**< Firmware File (netX5x/100/500)                  */
#define HIL_FILE_EXTENSION_NXI_FIRMWARE     ".NXI"  /**< Firmware File (netX90/4x00) Com CPU             */
#define HIL_FILE_EXTENSION_NXE_FIRMWARE     ".NXE"  /**< Firmware File (netX90/4x00) Com CPU             */
#define HIL_FILE_EXTENSION_MXF_FIRMWARE     ".MXF"  /**< Maintenance Firmware File (netX90/4x00) Com CPU */
#define HIL_FILE_EXTENSION_NAI_FIRMWARE     ".NAI"  /**< Firmware File (netX90) App CPU                  */
#define HIL_FILE_EXTENSION_NAE_FIRMWARE     ".NAE"  /**< Firmware File (netX90) App CPU                  */
#define HIL_FILE_EXTENSION_OPTION           ".NXO"  /**< Firmware Module File                            */
#define HIL_FILE_EXTENSION_LICENSE          ".NXL"  /**< License File                                    */
#define HIL_FILE_EXTENSION_DATABASE         ".NXD"  /**< Database File                                   */
#define HIL_FILE_EXTENSION_BINARY           ".NXB"  /**< Binary File                                     */
#define HIL_FILE_EXTENSION_NXS              ".NXS"  /**< Signed Firmware Update File                     */


/* Obsolete Hilscher file extensions */
#define HIL_FILE_EXTENSION_FIRMWARE         ".NXF"
#define HIL_FILE_EXTENSION_NXM_FIRMWARE     ".NXM"  /**< Firmware File (obsolete)       */

/* Structure/Header version constants */
#define HIL_VERSION_MAJOR_MSK               0xFFFF0000 /**< Mask for version/structure fields (major) */
#define HIL_VERSION_MINOR_MSK               0x0000FFFF /**< Mask for version/structure fields (minor) */

/* Common Header version constants */
#define HIL_VERSION_COMMON_HEADER_0_0       0x00000000 /**< V0.0, default initialization value */
#define HIL_VERSION_COMMON_HEADER_1_0       0x00010000 /**< V1.0, initial version */
#define HIL_VERSION_COMMON_HEADER_2_0       0x00020000 /**< V2.0, usManufacturer included in Common Header */
#define HIL_VERSION_COMMON_HEADER_3_0       0x00030000 /**< V3.0, usManufacturer moved to Device Info, additional sizes and offsets */

/* Device Info structure version constants */
#define HIL_VERSION_DEVICE_INFO_V1_0        0x00010000 /**< V1.0, initial version used with Common Header V3.0 */

/* Module Info structure version constants */
#define HIL_VERSION_MODULE_INFO_V1_0        0x00010000 /**< V1.0, initial version used with Common Header V3.0 */

/* NXS structure version constants */
#define HIL_VERSION_NXS_HEADER_V1_0         0x00010000 /**< V1.0, initial version */

/* source device type constants used in Default Header */
#define HIL_SRC_DEVICE_TYPE_PAR_FLASH_SRAM  1          /**< parallel flash on SRAM bus      */
#define HIL_SRC_DEVICE_TYPE_SER_FLASH       2          /**< serial flash on SPI bus         */
#define HIL_SRC_DEVICE_TYPE_EEPROM          3          /**< serial EEPROM on I2C bus        */
#define HIL_SRC_DEVICE_TYPE_SD_MMC          4          /**< boot image on MMC/SD card       */
#define HIL_SRC_DEVICE_TYPE_DPM             5          /**< DPM boot mode                   */
#define HIL_SRC_DEVICE_TYPE_DPM_EXT         6          /**< extended DPM boot mode          */
#define HIL_SRC_DEVICE_TYPE_PAR_FLASH_EXT   7          /**< parallel flash on extension bus */

/* Constants for NAI/NAE file signatures. */
#define HIL_ASIG_CHUNK_AVAILABLE            0x100      /**< NAI or NAE file is signed */
#define HIL_ASIG_CHUNK_SIZE                 0x400      /**< size of signature (always 1024 byte) */

/*****************************************************************************/
/* File Header Substructures for Hilscher Downloadable Files                 */
/*****************************************************************************/
#define HIL_FILE_BOOT_HEADER_SIGNATURE           0x5854454E /**< Boot header signature "NETX" */

/** BOOT header (64 bytes, used for NXF) */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_BOOT_HEADER_V1_0_Ttag
{
  /* boot block identification and bus width (8/16/32 bits) in case of a parallel flash source device  */
  uint32_t     ulMagicCookie;              /**< see HIL_FILE_HEADER_FIRMWARE_xxx_COOKIE           */

  /** boot image source device configuration value (either parallel or serial flash) */
  union
  { uint32_t   ulSramBusTiming;            /**< parallel flash on SRAM bus: bus timing value      */
    uint32_t   ulSpiClockSpeed;            /**< serial flash on SPI:        clock speed value     */
  } unSrcMemCtrl;

  /** application data description values */
  uint32_t     ulAppEntryPoint;            /**< app. entry point, netX code execution starts here     */
  uint32_t     ulAppChecksum;              /**< app. checksum starting from byte offset 64            */
  uint32_t     ulAppFileSize;              /**< app. size in DWORDs starting from byte offset 64      */
  uint32_t     ulAppStartAddress;          /**< app. relocation start address for binary image        */
  uint32_t     ulSignature;                /**< app. signature, always HIL_FILE_BOOT_HEADER_SIGNATURE */

  /** destination device control values */
  union
  { /** SDRAM */
    struct
    { uint32_t ulSdramGeneralCtrl;         /**< value for SDRAM General Control register          */
      uint32_t ulSdramTimingCtrl;          /**< value for SDRAM Timing register                   */
      uint32_t aulReserved[3];
    } tSDRAMCtrl;
    /** Extension Bus */
    struct
    { uint32_t ulExtConfigCS0;             /**< value for EXT_CONFIG_CS0 register                 */
      uint32_t ulIoRegMode0;               /**< value for DPMAS_IO_MODE0 register                 */
      uint32_t ulIoRegMode1;               /**< value for DPMAS_IO_MODE1 register                 */
      uint32_t ulIfConf0;                  /**< value for DPMAS_IF_CONF0 register                 */
      uint32_t ulIfConf1;                  /**< value for DPMAS_IF_CONF1 register                 */
    } tExtBusCtrl;
    /** SRAM */
    struct
    { uint32_t ulExtConfigSRAMn;           /**< value for EXT_SRAMn_CTRL register                 */
      uint32_t aulReserved[4];
    } tSRAMCtrl;
  } unDstMemCtrl;

  uint32_t     ulMiscAsicCtrl;             /**< internal ASIC control register value (set to 1)   */
  uint32_t     ulSerialNumber;             /**< serial no. or user param. (ignored by ROM loader) */
  uint32_t     ulSrcDeviceType;            /**< HIL_SRC_DEVICE_TYPE_xxx                           */
  uint32_t     ulBootHeaderChecksum;       /**< sums up all 16 DWORDs and multiplies result by -1 */
} HIL_FILE_BOOT_HEADER_V1_0_T, *PHIL_FILE_BOOT_HEADER_V1_0_T;


/** DEFAULT header (64 bytes, used for NXM, NXO, NXD, NXL, NXB) */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DEFAULT_HEADER_V1_0_Ttag
{
  uint32_t   ulMagicCookie;                /**< see HIL_FILE_HEADER_xxx_COOKIE definitions        */
  uint32_t   aulReserved[15];              /**< reserved, set to zero                             */
} HIL_FILE_DEFAULT_HEADER_V1_0_T, *PHIL_FILE_DEFAULT_HEADER_V1_0_T;


/** COMMON header (64 bytes) */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_COMMON_HEADER_V3_0_Ttag
{
  uint32_t   ulHeaderVersion;              /**< structure version (major, minor), 0x00030000                                */
  uint32_t   ulHeaderLength;               /**< Default+Common Header+Device Info+Module Info(s)                            */
  uint32_t   ulDataSize;                   /**< from ulDataStartOffset to ulTagListStartOffset                              */
  uint32_t   ulDataStartOffset;            /**< offset of binary data (from beginning of file)                              */
  uint8_t    bNumModuleInfos;              /**< number of Module Info structures in file header                             */
  uint8_t    bReserved;                    /**< reserved, set to zero                                                       */
  uint16_t   usReserved;                   /**< reserved, set to zero                                                       */
  uint32_t   aulMD5[4];                    /**< MD5 checksum for the whole firmware file                                    */
  uint32_t   ulTagListSize;                /**< tag list length in bytes (0 = no tag list), not used for netX90/4x00        */
  uint32_t   ulTagListOffset;              /**< offset of tag list (from beginning of file), not used for netX90/4x00       */
  uint32_t   ulTagListSizeMax;             /**< maximum tag list length in bytes (reserved space), not used for netX90/4x00 */
  uint32_t   ulCommonCRC32;                /**< netX90 only: common CRC32 for nxi/nxe nai/nae, not used for other chips     */
  uint32_t   aulReserved[2];               /**< reserved, set to zero                                                       */
  uint32_t   ulHeaderCRC32;                /**< CRC32 of Boot Header and Common Header                                      */
} HIL_FILE_COMMON_HEADER_V3_0_T, *PHIL_FILE_COMMON_HEADER_V3_0_T;


/** DEVICE-specific information (64 bytes) */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_DEVICE_INFO_V1_0_Ttag
{
  uint32_t  ulStructVersion;              /**< structure version (major, minor), 0x00010000      */
  uint16_t  usManufacturer;               /**< manufacturer ID (see DPM Manual)                  */
  uint16_t  usDeviceClass;                /**< netX device class                                 */
  uint8_t   bHwCompatibility;             /**< hardware compatibility ID                         */
  uint8_t   bChipType;                    /**< see HIL_DEV_CHIP_TYPE_xxx definitions             */
  uint16_t  usReserved;                   /**< reserved, set to zero                             */
  uint16_t  ausHwOptions[4];              /**< required hardware assembly options (0=not used)   */
  uint32_t  ulLicenseFlags1;              /**< netX license flags 1                              */
  uint32_t  ulLicenseFlags2;              /**< netX license flags 2                              */
  uint16_t  usNetXLicenseID;              /**< netX license id                                   */
  uint16_t  usNetXLicenseFlags;           /**< netX license flags                                */
  uint16_t  ausFwVersion[4];              /**< FW version (major, minor, build, revision)        */
  uint32_t  ulFwNumber;                   /**< FW product code (order number) or project code    */
  uint32_t  ulDeviceNumber;               /**< target device product code (order number)         */
  uint32_t  ulSerialNumber;               /**< target device serial number                       */
  uint32_t  aulReserved[3];               /**< reserved, set to zero                             */
} HIL_FILE_DEVICE_INFO_V1_0_T, *PHIL_FILE_DEVICE_INFO_V1_0_T;


/** MODULE-specific information (32 bytes) */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_MODULE_INFO_V1_0_Ttag
{
  uint32_t  ulStructVersion;              /**< structure version (major, minor), 0x00010000      */
  uint16_t  usCommunicationClass;         /**< communication class                               */
  uint16_t  usProtocolClass;              /**< protocol class                                    */
  uint32_t  ulDBVersion;                  /**< database version (major, minor)                   */
  uint16_t  ausChannelSizes[4];           /**< required DPM channel sizes, 0=end of list         */
  uint16_t  ausHwOptions[4];              /**< required hardware assembly options (0=not used)   */
  uint8_t   abHwAssignments[4];           /**< xC numbers for HW options (0xFF=free choice)      */
} HIL_FILE_MODULE_INFO_V1_0_T, *PHIL_FILE_MODULE_INFO_V1_0_T;



/*****************************************************************************/
/* File Header Subtructures Instantiated as Global Variables in the Firmware */
/*****************************************************************************/

/* !!!! The structure version of the Common Header determines the version of the whole structure !!!!  */

/** basic file header (without Default Header as included in ELF files used to create NXFs, 320 bytes)  */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_BASIC_HEADER_V3_0_Ttag
{
  HIL_FILE_COMMON_HEADER_V3_0_T   tCommonHeader;       /**< common header                                */
  HIL_FILE_DEVICE_INFO_V1_0_T     tDeviceInfo;         /**< device-specific information                  */
  HIL_FILE_MODULE_INFO_V1_0_T     tModuleInfo[6];      /**< module-specific info for up to 6 modules     */
} HIL_FILE_BASIC_HEADER_V3_0_T, *PHIL_FILE_BASIC_HEADER_V3_0_T;


/** basic file header (without Default Header as included in ELF files used to create NXOs, 160 bytes)  */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_MODULE_HEADER_V3_0_Ttag
{
  HIL_FILE_COMMON_HEADER_V3_0_T   tCommonHeader;       /**< common header                                */
  HIL_FILE_DEVICE_INFO_V1_0_T     tDeviceInfo;         /**< device-specific information                  */
  HIL_FILE_MODULE_INFO_V1_0_T     tModuleInfo;         /**< module-specific info for 1 module            */
} HIL_FILE_MODULE_HEADER_V3_0_T, *PHIL_FILE_MODULE_HEADER_V3_0_T;



/*****************************************************************************/
/* Minimal File Header Subset for all NX* Files (Usable with Older Versions) */
/*****************************************************************************/

/** MIN file header (common subset for all complete NX* files, 128 bytes)                               */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_MIN_HEADER_V3_0_Ttag
{
  HIL_FILE_BOOT_HEADER_V1_0_T     tBootHeader;         /**< boot header with chip settings               */
  HIL_FILE_COMMON_HEADER_V3_0_T   tCommonHeader;       /**< common header                                */
} HIL_FILE_MIN_HEADER_V3_0_T, *PHIL_FILE_MIN_HEADER_V3_0_T;



/*****************************************************************************/
/* File Header Structures for Hilscher Downloadable Files (V3.0, netX)       */
/*****************************************************************************/

/** NXF file header (for relocated base firmware or complete firmware with linked stacks, 384 bytes)    */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_NXF_HEADER_V3_0_Ttag
{
  HIL_FILE_BOOT_HEADER_V1_0_T     tBootHeader;         /**< boot header with chip settings               */
  HIL_FILE_COMMON_HEADER_V3_0_T   tCommonHeader;       /**< common header                                */
  HIL_FILE_DEVICE_INFO_V1_0_T     tDeviceInfo;         /**< device-specific information                  */
  HIL_FILE_MODULE_INFO_V1_0_T     tModuleInfo[6];      /**< module-specific info for 6 comm. channels    */
} HIL_FILE_NXF_HEADER_V3_0_T, *PHIL_FILE_NXF_HEADER_V3_0_T;


/** NXO file header (for unrelocated optional firmware modules, 224 bytes)                              */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_NXO_HEADER_V3_0_Ttag
{
  HIL_FILE_DEFAULT_HEADER_V1_0_T  tDefaultHeader;      /**< default header                               */
  HIL_FILE_COMMON_HEADER_V3_0_T   tCommonHeader;       /**< common header                                */
  HIL_FILE_DEVICE_INFO_V1_0_T     tDeviceInfo;         /**< device-specific information                  */
  HIL_FILE_MODULE_INFO_V1_0_T     tModuleInfo;         /**< module-specific info for 1 comm. channel     */
} HIL_FILE_NXO_HEADER_V3_0_T, *PHIL_FILE_NXO_HEADER_V3_0_T;


/** NXD file header (for database file download, 224 bytes)                                             */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_NXD_HEADER_V3_0_Ttag
{
  HIL_FILE_DEFAULT_HEADER_V1_0_T  tDefaultHeader;      /**< default header                               */
  HIL_FILE_COMMON_HEADER_V3_0_T   tCommonHeader;       /**< common header                                */
  HIL_FILE_DEVICE_INFO_V1_0_T     tDeviceInfo;         /**< device-specific information                  */
  HIL_FILE_MODULE_INFO_V1_0_T     tModuleInfo;         /**< module-specific info for 1 comm. channel     */
} HIL_FILE_NXD_HEADER_V3_0_T, *PHIL_FILE_NXD_HEADER_V3_0_T;


/** NXL file header (for license file download, 192 bytes)                                              */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_NXL_HEADER_V3_0_Ttag
{
  HIL_FILE_DEFAULT_HEADER_V1_0_T  tDefaultHeader;      /**< default header                               */
  HIL_FILE_COMMON_HEADER_V3_0_T   tCommonHeader;       /**< common header                                */
  HIL_FILE_DEVICE_INFO_V1_0_T     tDeviceInfo;         /**< device-specific information                  */
} HIL_FILE_NXL_HEADER_V3_0_T, *PHIL_FILE_NXL_HEADER_V3_0_T;


/** NXB file header (for binary file download, 192 bytes)                                               */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_NXB_HEADER_V3_0_Ttag
{
  HIL_FILE_DEFAULT_HEADER_V1_0_T  tDefaultHeader;      /**< default header                               */
  HIL_FILE_COMMON_HEADER_V3_0_T   tCommonHeader;       /**< common header                                */
  HIL_FILE_DEVICE_INFO_V1_0_T     tDeviceInfo;         /**< device-specific information                  */
} HIL_FILE_NXB_HEADER_V3_0_T, *PHIL_FILE_NXB_HEADER_V3_0_T;

/*=================================================================================================*/
/* netX90 APP CPU firmware (NAI/NAE) header definition                                             */
/*=================================================================================================*/
/** ROMLoader Hboot header for netX90(64 bytes, used for NAI and NAE) */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HBOOT_HEADER_NX90_Ttag
{
  uint32_t aulReserverd[16];    /**< NetX90 APP CPU system specific */
} HIL_HBOOT_HEADER_NX90_T, *PHIL_HBOOT_HEADER_NX90_T;

/** BOOT header netX90 (64 bytes, used for NAI and NAE) */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_BOOT_HEADER_NAI_NAE_V1_0_Ttag
{
    /** boot block identification */
  uint32_t     ulMagicCookie;              /**< see HIL_FILE_HEADER_FIRMWARE_xxx_COOKIE           */

  uint32_t     aulReserved1[2];

  /** application data description values */
  uint32_t     ulAppChecksum;              /**< app. checksum starting from byte offset 64            */
  uint32_t     ulAppFileSize;              /**< app. size in DWORDs starting from byte offset 64      */
  uint32_t     ulReserved1;
  uint32_t     ulSignature;                /**< app. signature, always HIL_FILE_BOOT_HEADER_SIGNATURE */

  uint32_t     aulReserved2[8];

  uint32_t     ulBootHeaderChecksum;       /**< sums up all previous 15 DWORDs and multiplies result by -1 */
} HIL_FILE_BOOT_HEADER_NAI_NAE_V1_0_T, *PHIL_FILE_BOOT_HEADER_NAI_NAE_V1_0_T;

/** NAI header
  This structure is used in the application core to generated new NAI or NAE header.
  The Cortex M4 Vector table and the Hboot header (M4 reset vector) are generated
  in a later build step, to obtain the complete file header (HIL_FILE_NAI_HEADER_V3_0_T)
  can't be used. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_NAI_NAE_APP_HEADER_V3_0_Ttag
{
  HIL_FILE_BOOT_HEADER_NAI_NAE_V1_0_T tBootHeader;          /**< NAI Boot header                      */
  HIL_FILE_COMMON_HEADER_V3_0_T       tCommonHeader;        /**< common header                        */
  HIL_FILE_DEVICE_INFO_V1_0_T         tDeviceInfo;          /**< device-specific information          */
} HIL_FILE_NAI_NAE_APP_HEADER_V3_0_T, *PHIL_FILE_NAI_NAE_APP_HEADER_V3_0_T;

/** NAI file header
  This structure contains the complete NAI file header as it can be found as binary file.
  It can be directly flashed into the application processes flash (INTFLASH2). */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_NAI_HEADER_V3_0_Ttag
{
  uint32_t                            aulVectorTable[112];  /**< Reserved for Cortex M4 Vector table  */
  HIL_HBOOT_HEADER_NX90_T             tHbootHeader;         /**< netX90 APP CPU Hboot header          */
  HIL_FILE_BOOT_HEADER_NAI_NAE_V1_0_T tBootHeader;          /**< NAI Boot header                      */
  HIL_FILE_COMMON_HEADER_V3_0_T       tCommonHeader;        /**< common header                        */
  HIL_FILE_DEVICE_INFO_V1_0_T         tDeviceInfo;          /**< device-specific information          */
} HIL_FILE_NAI_HEADER_V3_0_T, *PHIL_FILE_NAI_HEADER_V3_0_T;

/** NAE file header
  This structure contains the complete NAE file header as it can be found as binary file.
  The content will be stored in the SQI flash and will be copied by the rom loader to the SDRAM,
  before the application processor is started */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_NAE_HEADER_V3_0_Ttag
{
  HIL_HBOOT_HEADER_NX90_T             tHbootHeader;         /**< netX90 APP CPU Hboot header          */
  HIL_FILE_BOOT_HEADER_NAI_NAE_V1_0_T tBootHeader;          /**< NAE Boot header                      */
  HIL_FILE_COMMON_HEADER_V3_0_T       tCommonHeader;        /**< common header                        */
  HIL_FILE_DEVICE_INFO_V1_0_T         tDeviceInfo;          /**< device-specific information          */
} HIL_FILE_NAE_HEADER_V3_0_T, *PHIL_FILE_NAE_HEADER_V3_0_T;

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_UNPACK_1(HIL_FILEHEADERV3)
#endif

#endif /*  HIL_FILEHEADERV3_H_ */
