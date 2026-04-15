/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: $: *//*!

  \file Hil_HostInterfaceDefines.h

  Definitions and structures used by HIF and DPM.

**************************************************************************************/
#ifndef HIL_HOSTINTERFACEDEFINES_H_
#define HIL_HOSTINTERFACEDEFINES_H_

#include <stdint.h>
#include "Hil_Compiler.h"
#include "Hil_SharedDefines.h"

/*****************************************************************************/
/*! Hilscher System Information                                              */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_SYSTEM_INFO_Ttag
{
  uint8_t   abCookie[4];                                         /*!< "netX" cookie */
  uint32_t  ulDpmTotalSize;                                      /*!< Total Size of the whole dual-port memory in bytes */
  uint32_t  ulDeviceNumber;                                      /*!< Device number */
  uint32_t  ulSerialNumber;                                      /*!< Serial number */
  uint16_t  ausHwOptions[4];                                     /*!< Hardware options, xC port 0..3 */
  uint16_t  usManufacturer;                                      /*!< Manufacturer Location */
  uint16_t  usProductionDate;                                    /*!< Date of production */
  uint32_t  ulLicenseFlags1;                                     /*!< License code flags 1 */
  uint32_t  ulLicenseFlags2;                                     /*!< License code flags 2 */
  uint16_t  usNetxLicenseID;                                     /*!< netX license identification */
  uint16_t  usNetxLicenseFlags;                                  /*!< netX license flags */
  uint16_t  usDeviceClass;                                       /*!< netX device class */
  uint8_t   bHwRevision;                                         /*!< Hardware revision index */
  uint8_t   bHwCompatibility;                                    /*!< Hardware compatibility index */
  uint8_t   bDevIdNumber;                                        /*!< Device identification number (rotary switch) */
  uint8_t   bHifLayout;                                          /*!< Host Interface Layout identifier */
  uint16_t  usReserved;                                          /*!< Reserved */
} HIL_SYSTEM_INFO_T;

/* HIF LAYOUT definitions */
#define HIL_HIF_LAYOUT_NA                   0                /*!< unknown Layout / variable HIF structure */
#define HIL_HIF_LAYOUT_16K                  1                /*!< 16k byte HIF Layout */
#define HIL_HIF_LAYOUT_32K                  2                /*!< 32k byte HIF Layout */
#define HIL_HIF_LAYOUT_64K                  3                /*!< 64k byte HIF Layout */
#define HIL_HIF_LAYOUT_256K                 4                /*!< 256k byte HIF Layout */


/*****************************************************************************/
/*! Hilscher Watchdog                                                        */
/*****************************************************************************/
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_WATCHDOG_Ttag
{
  uint32_t  ulWatchdogTime;                                      /*!< Configured watchdog time */
  uint32_t  ulHostWatchdog;                                      /*!< Host watchdog counter    */
  uint32_t  ulDeviceWatchdog;                                    /*!< Device watchdog counter  */
} HIL_WATCHDOG_T;

/*****************************************************************************/
/*! System status Definitions                                                */
/*****************************************************************************/
/* System Status */
#define HIL_SYS_STATUS_UNDEFINED                            0x00000000
#define HIL_SYS_STATUS_OK                                   0x00000001
#define HIL_SYS_STATUS_IDPM                                 0x00400000
#define HIL_SYS_STATUS_APP                                  0x00800000
#define HIL_SYS_STATUS_BOOTMEDIUM_MASK                      0x0F000000
#define HIL_SYS_STATUS_BOOTMEDIUM_RAM                       0x00000000
#define HIL_SYS_STATUS_BOOTMEDIUM_SERFLASH                  0x01000000
#define HIL_SYS_STATUS_BOOTMEDIUM_PARFLASH                  0x02000000
#define HIL_SYS_STATUS_NO_SYSVOLUME                         0x20000000
#define HIL_SYS_STATUS_SYSVOLUME_FFS                        0x40000000  /*!< _FFS = Flash File System */
#define HIL_SYS_STATUS_NXO_SUPPORTED                        0x80000000

/* System Error definitions */
#define HIL_SYS_ERROR_SUCCESS                               0

/* System Hardware Features */
/* Extended Memory */
#define HIL_SYSTEM_EXTMEM_TYPE_MSK                          0x0000000F
#define HIL_SYSTEM_EXTMEM_TYPE_NONE                         0x00000000
#define HIL_SYSTEM_EXTMEM_TYPE_MRAM_128K                    0x00000001

#define HIL_SYSTEM_EXTMEM_ACCESS_MSK                        0x000000C0
#define HIL_SYSTEM_EXTMEM_ACCESS_NONE                       0x00000000
#define HIL_SYSTEM_EXTMEM_ACCESS_EXTERNAL                   0x00000040
#define HIL_SYSTEM_EXTMEM_ACCESS_INTERNAL                   0x00000080
#define HIL_SYSTEM_EXTMEM_ACCESS_BOTH                       0x000000C0

/* RTC */
#define HIL_SYSTEM_HW_RTC_MSK                               0x00000700
#define HIL_SYSTEM_HW_RTC_TYPE_MSK                          0x00000300
#define HIL_SYSTEM_HW_RTC_TYPE_NONE                         0x00000000
#define HIL_SYSTEM_HW_RTC_TYPE_INTERNAL                     0x00000100
#define HIL_SYSTEM_HW_RTC_TYPE_EXTERNAL                     0x00000200
#define HIL_SYSTEM_HW_RTC_TYPE_EMULATED                     0x00000300
#define HIL_SYSTEM_HW_RTC_STATE                             0x00000400




#endif  /* HIL_HOSTINTERFACEDEFINES_H_ */
