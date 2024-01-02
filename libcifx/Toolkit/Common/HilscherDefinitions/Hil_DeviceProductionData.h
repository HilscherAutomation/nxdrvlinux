/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_DeviceProductionData.h $: *//*!

  \file Hil_DeviceProductionData.h

  Device Production Data for the netX90 and netX400.

  Structure definition for the Device Production Data for the netX90 and netX400.
  The structure HIL_PRODUCT_DATA_LABEL_T is placed within the internal flash of the
  netX90 or in the serial flash of the netX4x00.

**************************************************************************************/
#ifndef HIL_DEVICEPRODUCTIONDATA_H_
#define HIL_DEVICEPRODUCTIONDATA_H_

#include <stdint.h>
#include "Hil_Compiler.h"

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_PACK_1(HIL_DEVICEPRODUCTIONDATA)
#endif


typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_LLICENSE_FLAGS_Ttag
{
  uint32_t  ulLicenseId;            /*!< netX license ID/Flags */
  uint32_t  ulLicenseFlags1;        /*!< netX license flags 1 */
  uint32_t  ulLicenseFlags2;        /*!< netX license flags 2 */
} HIL_PRODUCT_DATA_LICENSE_FLAGS_T;

/*! Basic device data.
 * Basic production information about the device.
 */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_BASIC_DEVICE_DATA_Ttag
{
  /*! Manufacturer ID.
   * 0 = Undefined; 1 - 255 = Hilscher GmbH; 256 - x = OEM
   * \note All numbers are managed and assigned by Hilscher GmbH. */
  uint16_t  usManufacturer;

  /*! Device classification number.
   * Possible values are defined in the dual port memory definitions (see HIL_HW_DEV_CLASS_*). */
  uint16_t  usDeviceClass;

  /*! Device number.
   * For usManufacturer 1-255 the numbers are managed by Hilscher GmbH. */
  uint32_t  ulDeviceNumber;

  /*! Serial number.
   * Serial number of the device. */
  uint32_t  ulSerialNumber;

  /*! Hardware compatibility number.
   * This number indicates an incompatible hardware change */
  uint8_t   bHwCompatibility;

  /*! Hardware revision number.
   * Production related hardware revision number */
  uint8_t   bHwRevision;

  /*! Production date
   * Format is 0xYYWW:
   * Year = ((usProductionDate >> 8) & 0x00ff) + 2000
   * Week = ((usProductionDate >> 0) & 0x00ff)
   * e.g. 0C2Bh, where 0Ch is year 2012 and 2Bh is week 43. */
  uint16_t  usProductionDate;

  uint8_t   bReserved1;       /*!< Reserved. bHostInterfaceType is deprecated. */
  uint8_t   bReserved2;       /*!< Reserved. bHwAssemblyFeatures is deprecated. */

  /*! Reserved area.
   * Currently not used set to 0 to avoid conflicts with future definitions. */
  uint8_t   abReserved[14];

} HIL_PRODUCT_DATA_BASIC_DEVICE_DATA_T;


/*------------------------------------------------------------*/
/*  MAC Address information.                                  */
/*------------------------------------------------------------*/
#define HIL_PRODUCT_NUMBER_OF_MAC_ADDRESSES_COM   8   /*!< Number of MAC Addresses used by Communication firmware. */
#define HIL_PRODUCT_NUMBER_OF_MAC_ADDRESSES_APP   4   /*!< Number of MAC Addresses used by Application firmware. */

/*! Structure to hold MAC one address. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_MAC_ADDRESS_Ttag
{
  /*! MAC address. */
  uint8_t abMacAddress[6];

  /*! Reserved area.
   * Currently not used set to 0 to avoid conflicts with future definitions. */
  uint8_t abReserved[2];

} HIL_PRODUCT_DATA_MAC_ADDRESS_T;

/*! Structure holding MAC addresses for communication firmware. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_MAC_ADDRESSES_COM_Ttag
{
  /*! Array of MAC address for communication side. */
  HIL_PRODUCT_DATA_MAC_ADDRESS_T atMAC[HIL_PRODUCT_NUMBER_OF_MAC_ADDRESSES_COM];

} HIL_PRODUCT_DATA_MAC_ADDRESSES_COM_T;

/*! Structure holding MAC addresses for application firmware. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_MAC_ADDRESSES_APP_Ttag
{
  /*! Array of MAC address for application side. */
  HIL_PRODUCT_DATA_MAC_ADDRESS_T atMAC[HIL_PRODUCT_NUMBER_OF_MAC_ADDRESSES_APP];

} HIL_PRODUCT_DATA_MAC_ADDRESSES_APP_T;


/*------------------------------------------------------------*/
/*! Product Identification information.                       */
/*------------------------------------------------------------*/
#define HIL_PRODUCT_USB_VENDOR_NAME_SIZE    16  /*!< Size of the USB vendor name. */
#define HIL_PRODUCT_USB_PRODUCT_NAME_SIZE   16  /*!< Size of the USB product name. */

/*! Structure holding USB related vendor information. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_USB_Ttag
{
  /*! USB Device vendor ID (VID). */
  uint16_t    usUSBVendorID;

  /*! USB Device product ID (PID). */
  uint16_t    usUSBProductID;

  /*! USB Vendor name.
   * The Vendor name shown over USB, please refer the USB documentation
   * which data format shall be used. */
  uint8_t     abUSBVendorName[HIL_PRODUCT_USB_VENDOR_NAME_SIZE];

  /*! USB Product name.
   * The Product name shown over USB, please refer the USB documentation
   * which data format shall be used. */
  uint8_t     abUSBProductName[HIL_PRODUCT_USB_PRODUCT_NAME_SIZE];

} HIL_PRODUCT_DATA_USB_T;


/*! Structure holding Product identification data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_PRODUCT_IDENTIFICATION_Ttag
{
  /*! USB product data. */
  HIL_PRODUCT_DATA_USB_T  tUSBInfo;

  /*! Reserved area.
   * Currently not used set to 0 to avoid conflicts with future definitions. */
  uint8_t                 abReserved[76];

} HIL_PRODUCT_DATA_PRODUCT_IDENTIFICATION_T;


/*------------------------------------------------------------*/
/*! OEM product specific device data.                         */
/*------------------------------------------------------------*/
#define HIL_PRODUCT_DATA_OEM_IDENTIFICATION_FLAG_SERIALNUMBER_VALID       0x00000001 /*!< OEM serial number stored in szSerialNumber field is valid */
#define HIL_PRODUCT_DATA_OEM_IDENTIFICATION_FLAG_ORDERNUMBER_VALID        0x00000002 /*!< OEM order number stored in szOrderNumber is valid */
#define HIL_PRODUCT_DATA_OEM_IDENTIFICATION_FLAG_HARDWAREREVISION_VALID   0x00000004 /*!< OEM hardware revision stored in szHardwareRevision field is valid */
#define HIL_PRODUCT_DATA_OEM_IDENTIFICATION_FLAG_PRODUCTIONDATA_VALID     0x00000008 /*!< OEM production date stored in szProductionDate field is valid */

/*! Structure holding OEM identification data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_OEM_IDENTIFICATION_Ttag
{
  /*! OEM Data Option Flags.
   * This flag field indicate which of the following members contains valid information. If a member
   * if flagged as 'valid' the related value will be evaluated by the stack.
   * (see HIL_PRODUCT_DATA_OEM_IDENTIFICATION_FLAG_* definitions)
   *
   * \note Not all firmware will support this feature at all, please refer the
   *       firmware documentation if this setting will be supported and how.
   *
   * \note If the field is 0, all parameters in this structure will be ignored by communication
   *       stacks with respect to configurations parameters
   */
  uint32_t    ulOemDataOptionFlags;

  /*! Serial number.
   * Serial number of the device as string (e.g. "20053" or "SC-D5SF5005")
   *
   * \note Null terminated c string.
   * \note Some communication stack only support numbers, or a subset of chars.
   *       Please refer the related specification.
   */
  char        szSerialNumber[28];

  /*! Order number.
   * Order number of the device as string (e.g. "1544.100" or "6ES7 511-1AK00-0AB0").
   *
   * \note Null terminated c string.
   * \note Some communication stack only support numbers, or a subset of chars.
   *       Please refer the related specification.
   */
  char        szOrderNumber[32];

  /*! Hardware revision.
   * Hardware revision as string (e.g. "3" or "V1.5").
   *
   * \note Null terminated c string.
   * \note Some communication stack only support numbers, or a subset of chars.
   *       Please refer the related specification.
   */
  char        szHardwareRevision[16];

  /*! Production date/time.
   * UTC (Universal Time Coordinated) without TZD (time zone designator, always 'Z') and no
   * fraction of a second E.g "2012-10-24T07:36:17Z".
   *
   * \note Null terminated c string.
   */
  char        szProductionDate[32];

  /*! Reserved area.
   * Currently not used set to 0 to avoid conflicts with future definitions.
   */
  uint8_t     abReserved[12];

  /*! Vendor specific data.
   * This field can be used to store any kind of information to this device label.
   */
  uint8_t     abVendorData[112];

} HIL_PRODUCT_DATA_OEM_IDENTIFICATION_T;


/*------------------------------------------------------------*/
/*! Flash layout information                                  */
/*------------------------------------------------------------*/

/*! Definition of content types of a flash memory.
 * Some of the content types are currently not of interest for the communication firmware, e.g. HWCONFIG.
 * Regardless of usage, we recommend to describe all areas which are used anyway for consistency.
 */
typedef enum HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_Etag
{
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_EMPTY        =  0, /*!< Entry is not used. */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_HWCONFIG     =  1, /*!< Hardware configuration data. */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_FDL          =  2, /*!< Device product data label (HIL_PRODUCT_DATA_LABEL_T). */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_FW           =  3, /*!< Communication firmware. */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_FW_CONT      =  4, /*!< Communication firmware continued. */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_CONFIG       =  5, /*!< Firmware configuration data. */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_REMANENT     =  6, /*!< Remanent data of firmware (libstorage). */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_MANAGEMENT   =  7, /*!< Management of remanent area (libstorage). */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_APP_CONT     =  8, /*!< Application firmware continued. */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_MFW          =  9, /*!< Maintenance firmware. */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_FILESYSTEM   = 10, /*!< File system. */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_FWUPDATE     = 11, /*!< Storage space for firmware updates. */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_MFW_HWCONFIG = 12, /*!< Hardware configuration when maintenance firmware is executed. */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_APP          = 13, /*!< Application firmware. */

} HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_E;

/*! Structure holding information about one flash area. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_FLASH_LAYOUT_AREAS_Ttag
{
  /*! Area content type.
   * Content type stored in this area. Possible values are defined as
   * HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_*. */
  uint32_t ulContentType;

  /*! Area start address.
   * Absolut physical start address within the netX address space. */
  uint32_t ulAreaStart;

  /*! Area size.
   * Size of the content area in bytes. */
  uint32_t ulAreaSize;

  /*! Chip number.
   * Chip number, where the area is located.
   * See HIL_PRODUCT_DATA_FLASH_LAYOUT_CHIPS_T */
  uint32_t ulChipNumber;

  /*! Area name.
   * Name string of the area. May be used to identify an area.
   * \note Null terminated c string. */
  char szName[16];


  /*! Area access type.
   * Access control. Standard defines from libC fcntl.h will be used.
   * \note Currently only O_RDONLY = 0 and O_RDWR = 2 shall be used. */
  uint8_t bAccessTyp;

  /*! Reserved area.
   * Currently not used set to 0 to avoid conflicts with future definitions. */
  uint8_t abReserved[3];

} HIL_PRODUCT_DATA_FLASH_LAYOUT_AREAS_T;

/*! Structure holding information about used flash memories chips. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_FLASH_LAYOUT_CHIPS_Ttag
{

  /*! Chip number.
   * Chip number, where the area is located:
   *
   * | Chip | netX90                       | netX4000                      |
   * | ---- | ---------------------------- | ----------------------------- |
   * |   0  | INTFLASH0                    |  Serial Flash @ SQI interface |
   * |   1  | INTFLASH1                    |                               |
   * |   2  | Serial Flash @ SQI interface |                               |
   *
   * \note The INTFLASH2 within netX90 has currently no number assigned. */
  uint32_t ulChipNumber;

  /*! Flash name.
   * Name string of the flash. May be used to identify an flash.
   * \note Null terminated c string. */
  char szFlashName[16];

  /*! Block size.
   * Size of a Block in bytes in the Flash memory, usually this is the smallest
   * area which can be erased independently. */
  uint32_t ulBlockSize;

  /*! Flash size.
   * Size of the flash block in bytes. */
  uint32_t ulFlashSize;

  /*! Endurance cycles.
   * Maximum erase/write cycles which the manufacturer allows for the flash.
   * \note Flashes are complex devices which may have additional requirements
   *       how to write and delete them. Please refers the related data sheet.
   *       This information is only to get an overview what can be expected. */
  uint32_t ulMaxEnduranceCycles;

} HIL_PRODUCT_DATA_FLASH_LAYOUT_CHIPS_T;

/*! Structure holding information about the flash layout of the product. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_FLASH_LAYOUT_Ttag
{
  /*! List of flash areas.
   * Flash areas which have been defined for this product.
   * \note Set the complete entry to 0, to indicate an not used entry. */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_AREAS_T atArea[10];

  /*! List of flash chips.
   * List of all flashes which can be accessed from the netX controller.
   * \note Set the complete entry to 0, to indicate an not used entry.
   */
  HIL_PRODUCT_DATA_FLASH_LAYOUT_CHIPS_T atChip[4];

} HIL_PRODUCT_DATA_FLASH_LAYOUT_T;


/* Defines for deprecated structure names, do not use them for newer implementations. */
#define HIL_PRODUCT_DATA_LIBSTORAGE_AREAS_T   HIL_PRODUCT_DATA_FLASH_LAYOUT_AREAS_T
#define HIL_PRODUCT_DATA_LIBSTORAGE_CHIPS_T   HIL_PRODUCT_DATA_FLASH_LAYOUT_CHIPS_T
#define HIL_PRODUCT_DATA_LIBSTORAGE_T         HIL_PRODUCT_DATA_FLASH_LAYOUT_T


/*------------------------------------------------------------*/
/*! PRODUCTION Data Header and Footer                         */
/*------------------------------------------------------------*/
#define HIL_PRODUCT_DATA_START_TOKEN  "ProductData>"
#define HIL_PRODUCT_DATA_END_TOKEN    "<ProductData"

/*! Production data header structure.
 * This structure indicates the beginning of production data information and
 * the used space of the label. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_HEADER_Ttag
{
  /*! Start token of the HIL_PRODUCT_DATA_LABEL_T.
   * This field must contain the token sting defined by HIL_PRODUCT_DATA_START_TOKEN. */
  uint8_t     abStartToken[12];

  /*! Size of the production label HIL_PRODUCT_DATA_LABEL_T.
   * \note This includes the Header and Footer sizeof(HIL_PRODUCT_DATA_LABEL_T). */
  uint16_t    usLabelSize;

  /*! Size of the production data structure HIL_PRODUCT_DATA_T.
   * \note This does not include the Header and Footer sizeof(HIL_PRODUCT_DATA_T). */
  uint16_t    usContentSize;

} HIL_PRODUCT_DATA_HEADER_T;

/*! Production data footer structure.
 * This structure indicates the end of production data information and an checksum
 * of the Production data content (HIL_PRODUCT_DATA_T) above it. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_FOOTER_Ttag
{
  /*! Checksum of the label array.
   * \note CRC-32 (IEEE 802.3) of HIL_PRODUCT_DATA_T. */
  uint32_t    ulChecksum;

  /*! End token of the HIL_PRODUCT_DATA_LABEL_T.
   * This field must contain the token string defined by HIL_PRODUCT_DATA_END_TOKEN. */
  uint8_t     abEndToken[12];

} HIL_PRODUCT_DATA_FOOTER_T;

/*! Collection of the production label data content. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_Ttag
{
  HIL_PRODUCT_DATA_BASIC_DEVICE_DATA_T        tBasicDeviceData;
  HIL_PRODUCT_DATA_MAC_ADDRESSES_COM_T        tMACAddressesCom;
  HIL_PRODUCT_DATA_MAC_ADDRESSES_APP_T        tMACAddressesApp;
  HIL_PRODUCT_DATA_PRODUCT_IDENTIFICATION_T   tProductIdentification;
  HIL_PRODUCT_DATA_OEM_IDENTIFICATION_T       tOEMIdentification;
  HIL_PRODUCT_DATA_FLASH_LAYOUT_T             tFlashLayout;

} HIL_PRODUCT_DATA_T;


/*! Device production label DPL.
 * The device production label structure is the complete assembled structure
 * of all structures above. This structure can be used to lay over the data in
 * the flash memory.
 * \note Before accessing the data within the device production label you have to
 *       check if the content is valid and correct. The following checks shall be performed:
 *       1. The DPL begins with the token "ProductData>" (not null terminated!)
 *       2. The size values have an expected values.
 *       3. The label ends with the token "<ProductData" (not null terminated!)
 *       4. Calculate the check sum over the content HIL_PRODUCT_DATA_T and compare the
 *          result against the checksum in the footer of this label.
 *
 *       If all tests have been passes, it can be assumed that correct data are available.
 */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PRODUCT_DATA_LABEL_Ttag
{

  HIL_PRODUCT_DATA_HEADER_T                 tHeader;
  HIL_PRODUCT_DATA_T                        tProductData;
  HIL_PRODUCT_DATA_FOOTER_T                 tFooter;

} HIL_PRODUCT_DATA_LABEL_T;

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_UNPACK_1(HIL_DEVICEPRODUCTIONDATA)
#endif

#endif /* HIL_DEVICEPRODUCTIONDATA_H_ */
