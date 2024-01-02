/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_BootParameter.h $: *//*!

  \file Hil_BootParameter.h

  Structure definition for the boot parameter data for the netX90 and netX4000.

**************************************************************************************/
#ifndef HIL_BOOTPARAMETER_H_
#define HIL_BOOTPARAMETER_H_

#include <stdint.h>
#include "Hil_Compiler.h"

/*------------------------------------------------------------*/
/*! Boot Parameter Data Header and Footer                     */
/*------------------------------------------------------------*/
#define HIL_BOOT_PARAM_HEADER_TOKEN   0xAA5511EE
#define HIL_BOOT_PARAM_FOOTER_TOKEN   0xEE1155AA
#define HIL_BOOT_PARAM_VERSION        2

/*------------------------------------------------------------*/
/*! Boot Parameter Data Header                                */
/*------------------------------------------------------------*/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BOOT_PARAM_HEADER_Ttag
{
  /*! Start token of the HIL_BOOT_PARAM_T. */
  uint32_t    ulStart;

  /*! Size of the boot parameter structure HIL_BOOT_PARAM_T.
   * \note This includes the Header and Footer */
  uint32_t    ulSize;

  /*! Version of the HIL_BOOT_PARAM_T structure. */
  uint16_t    usVersion;

} HIL_BOOT_PARAM_HEADER_T;

/*------------------------------------------------------------*/
/*! Boot Parameter Data Footer                                */
/*------------------------------------------------------------*/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BOOT_PARAM_FOOTER_Ttag
{
  /*! End token of the HIL_BOOT_PARAM_T. */
  uint32_t    ulEnd;

} HIL_BOOT_PARAM_FOOTER_T;

/*------------------------------------------------------------*/
/*! Hardware information                                      */
/*------------------------------------------------------------*/
#define HIL_BOOT_PARAM_HW_DEV_ID_MAX        0x09  /*!< Max. device ID/slot number */

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BOOT_PARAM_HARDWARE_INFORMATION_Ttag
{
  /*! Hardware assembly options for XC units.
   * Hardware which is equipped for the related XC units (0-3). Possible values are defined
   * in the dual port memory definitions (see HIL_HW_ASSEMBLY_*). */
  uint16_t    ausHwOptionXc[4];

  /*! Hardware assembly options for xPIC units.
   * Hardware which is equipped for the related communication side xPIC units (0-3). On
   * netX90 the "Com xPIC" is located in option 0 all other options are not used. Possible
   * values are defined in the dual port memory definitions (see HIL_HW_ASSEMBLY_*). */
  uint16_t    ausHwOptionsXpic[4];

  /*! Rotary switch number. */
  uint8_t     bDevIdNumber;

  /*! COM UART available. */
  uint8_t     bComUartAvailable;

  /*! Hardware config (HWC) version. */
  uint8_t     bHwcVersion;

  /*! Information about PIO pins (for netANALYZER). */
  uint8_t     bHwcPio;

  /*! Information about GPIO pins (for netANALYZER). */
  uint16_t    usHwcGpio;

  /*! Reserved field.
   * \note Currently not used set to 0 */
  uint8_t     abReserved1[10];

} HIL_BOOT_PARAM_HARDWARE_INFORMATION_T;

/*------------------------------------------------------------*/
/*! DPM information                                           */
/*------------------------------------------------------------*/
#define HIL_BOOT_PARAM_DPM_MODE_NONE        0x00  /*!< No DPM configuration */
#define HIL_BOOT_PARAM_DPM_MODE_SPM0        0x01  /*!< SPM0 is configured */
#define HIL_BOOT_PARAM_DPM_MODE_SPM1        0x02  /*!< SPM1 is configured */
#define HIL_BOOT_PARAM_DPM_MODE_SPM2        0x03  /*!< SPM0 and SPM1 are configured */
#define HIL_BOOT_PARAM_DPM_MODE_DPM         0x04  /*!< DPM is configured */
#define HIL_BOOT_PARAM_DPM_MODE_IDPM0       0x05  /*!< iDPM0 is configured */
#define HIL_BOOT_PARAM_DPM_MODE_IDPM1       0x06  /*!< iDPM1 is configured */
#define HIL_BOOT_PARAM_DPM_MODE_PCIE        0x07  /*!< iDPM via PCIe is configured */
#define HIL_BOOT_PARAM_DPM_MODE_IDPM0SPM0   0x08  /*!< iDPM0 and SPM0 */

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BOOT_PARAM_DPM_INFORMATION_Ttag
{
  /*! DPM boot mode.
   * DPM interface configuration. */
  uint8_t     bDpmMode;
  uint8_t     bReserved;

} HIL_BOOT_PARAM_DPM_INFORMATION_T;

/*------------------------------------------------------------*/
/*! SDRAM information                                         */
/*------------------------------------------------------------*/
#define HIL_BOOT_PARAM_SDRAM_TYPE_NONE   0
#define HIL_BOOT_PARAM_SDRAM_TYPE_HIFMEM 1
#define HIL_BOOT_PARAM_SDRAM_TYPE_EXTMEM 2

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BOOT_PARAM_SDRAM_INFORMATION_Ttag
{
  /*! SDRAM Type: HIFMEM or EXTMEM. */
  uint8_t     bType;

  /*! SDRAM size in MB. */
  uint8_t     bSize;

} HIL_BOOT_PARAM_SDRAM_INFORMATION_T;

/*------------------------------------------------------------*/
/*! SQI information                                           */
/*------------------------------------------------------------*/
#define HIL_BOOT_PARAM_SQI_BASE_NONE  0x00
#define HIL_BOOT_PARAM_SQI_BASE_0     0x01
#define HIL_BOOT_PARAM_SQI_BASE_1     0x02

#define HIL_BOOT_PARAM_SQI_XIP_NONE   0x00
#define HIL_BOOT_PARAM_SQI_XIP        0x01

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BOOT_PARAM_SQI_Ttag
{
  uint8_t     bSqiBase;   /*!< SQI base register */
  uint8_t     bSqiXip;    /*!< XIP supported */
} HIL_BOOT_PARAM_SQI_T;

/*------------------------------------------------------------*/
/*! FLASH information                                         */
/*------------------------------------------------------------*/

/* Quad enable requirements (JEDEC Table 15) */
#define HIL_BOOT_PARAM_FLASH_QER_TYPE_1   0x01  /* 001b */
#define HIL_BOOT_PARAM_FLASH_QER_TYPE_2   0x02  /* 010b */
#define HIL_BOOT_PARAM_FLASH_QER_TYPE_3   0x03  /* 011b */
#define HIL_BOOT_PARAM_FLASH_QER_TYPE_4   0x04  /* 100b */
#define HIL_BOOT_PARAM_FLASH_QER_TYPE_5   0x05  /* 101b */
#define HIL_BOOT_PARAM_FLASH_QER_TYPE_6   0x06  /* 110b */

/* 0-4-4 Mode Entry Method (JEDEC Table 15) */
#define HIL_BOOT_PARAM_FLASH_ENTRY_TYPE_1 0x01  /* xxx1b */
#define HIL_BOOT_PARAM_FLASH_ENTRY_TYPE_2 0x02  /* xx1xb */

/* 0-4-4 Mode Exit Method (JEDEC Table 15) */
#define HIL_BOOT_PARAM_FLASH_EXIT_TYPE_1  0x01  /* xx_xxx1b */
#define HIL_BOOT_PARAM_FLASH_EXIT_TYPE_2  0x02  /* xx_1xxxb */

/* Status Register Polling Device Busy (JEDEC Table 14) */
#define HIL_BOOT_PARAM_FLASH_POLL_TYPE_1  0x01  /* xx_xx1xb */
#define HIL_BOOT_PARAM_FLASH_POLL_TYPE_2  0x02  /* xx_xxx1b */

typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BOOT_PARAM_FLASH_INFORMATION_Ttag
{
  uint8_t     bWriteEnable;   /*!< Write enable instruction */
  uint8_t     bPageProgram;   /*!< Page program instruction */
  uint8_t     bSectorErase;   /*!< Sector erase instruction */
  uint8_t     bRead;          /*!< Read data instruction */
  uint8_t     bQuadRead;      /*!< Fast read quad I/O instruction */
  uint8_t     bReadStatus1;   /*!< Read status register 1 instruction */
  uint8_t     bWriteStatus1;  /*!< Write status register 1 instruction */
  uint8_t     bReadStatus2;   /*!< Read status register 2 instruction */
  uint8_t     bWriteStatus2;  /*!< Write status register 2 instruction */

  uint8_t     bFreqMHz;       /*!< SPI FIFO Frequency */
  uint8_t     bAddrBytes;     /*!< Number of address bytes */
  uint8_t     bQERType;       /*!< Quad enable requirements type */
  uint8_t     bEntryType;     /*!< Sequence to enter 0-4-4 mode */
  uint8_t     bExitType;      /*!< Sequence to exit 0-4-4 mode */
  uint8_t     bPollingMethod; /*!< Polling mode */
  uint8_t     bSpiFifoMode;   /*!< 0 = Mode 0, ..., 3 = Mode 3, others reserved */
  uint32_t    ulPageSize;     /*!< Page size in bytes */
  uint32_t    ulSectorSize;   /*!< Sector size in bytes */
  uint32_t    ulSectorCount;  /*!< Number of sectors */
  uint32_t    ulSqiRomCfg;    /*!< netX SqiRomCfg configuration */

} HIL_BOOT_PARAM_FLASH_INFORMATION_T;

/*------------------------------------------------------------*/
/*! Boot Parameter Data Structure                             */
/*------------------------------------------------------------*/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BOOT_PARAM_DATA_Ttag
{
  HIL_BOOT_PARAM_HARDWARE_INFORMATION_T tHardwareInfo;
  HIL_BOOT_PARAM_DPM_INFORMATION_T      tDpmInfo;
  HIL_BOOT_PARAM_SDRAM_INFORMATION_T    tSdramInfo;
  HIL_BOOT_PARAM_SQI_T                  tSqiInfo;
  HIL_BOOT_PARAM_FLASH_INFORMATION_T    tFlashInfo;

} HIL_BOOT_PARAM_DATA_T;

/*------------------------------------------------------------*/
/*! Boot Parameter Structure                                  */
/*------------------------------------------------------------*/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_BOOT_PARAM_Ttag
{
  HIL_BOOT_PARAM_HEADER_T tHeader;
  HIL_BOOT_PARAM_DATA_T   tData;
  HIL_BOOT_PARAM_FOOTER_T tFooter;

} HIL_BOOT_PARAM_T;

#endif /* HIL_BOOTPARAMETER_H_ */
