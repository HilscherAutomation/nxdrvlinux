/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_SharedDefines.h $: *//*!

  \file Hil_SharedDefines.h

  Defines which are used in various places. E.g. Fileheader, dual puort memory.

**************************************************************************************/
#ifndef HIL_SHAREDDEFINES_H_
#define HIL_SHAREDDEFINES_H_

/*! \addtogroup HIL_DEV_BOOT_TYPE_doc
 *
 * Boot type definitions for netX chip family used for:
 *  - 2nd Stage Boot Loader
 *  - In Operating systems
 *  - Packet services
 *
 * \{ */
#define HIL_DEV_BOOT_TYPE_PFLASH_SRAMBUS            0x00   /*!< ROM Loader: Parallel Flash (SRAM Bus) */
#define HIL_DEV_BOOT_TYPE_PFLASH_EXTBUS             0x01   /*!< ROM Loader: Parallel Flash (Extension Bus) */
#define HIL_DEV_BOOT_TYPE_DUALPORT                  0x02   /*!< ROM Loader: Dual-Port Memory */
#define HIL_DEV_BOOT_TYPE_PCI                       0x03   /*!< ROM Loader: PCI Interface */
#define HIL_DEV_BOOT_TYPE_MMC                       0x04   /*!< ROM Loader: Multimedia Card */
#define HIL_DEV_BOOT_TYPE_I2C                       0x05   /*!< ROM Loader: I2C Bus */
#define HIL_DEV_BOOT_TYPE_SFLASH                    0x06   /*!< ROM Loader: Serial Flash */
#define HIL_DEV_BOOT_TYPE_2ND_STAGE_FLASH_BASED     0x07   /*!< 2nd Stage Boot Loader: Serial Flash */
#define HIL_DEV_BOOT_TYPE_2ND_STAGE_RAM_BASED       0x08   /*!< 2nd Stage Boot Loader: RAM */
#define HIL_DEV_BOOT_TYPE_IFLASH                    0x09   /*!< ROM Loader: Internal Flash */
/*! \} */

/*! \addtogroup HIL_DEV_CHIP_TYPE_doc
 *
 * Chip type definitions for netX chip family used for:
 *  - Fileheader V3 (HIL_FILE_DEVICE_INFO_V1_0_T)
 *  - In Operating system
 *
 * \{ */
#define HIL_DEV_CHIP_TYPE_UNKNOWN      0x00   /*!< unknown Chip */
#define HIL_DEV_CHIP_TYPE_NETX500      0x01   /*!< netX500 Chip */
#define HIL_DEV_CHIP_TYPE_NETX100      0x02   /*!< netX100 Chip */
#define HIL_DEV_CHIP_TYPE_NETX50       0x03   /*!< netX50 Chip */
#define HIL_DEV_CHIP_TYPE_NETX10       0x04   /*!< netX10 Chip */
#define HIL_DEV_CHIP_TYPE_NETX51       0x05   /*!< netX51 Chip */
#define HIL_DEV_CHIP_TYPE_NETX52       0x06   /*!< netX52 Chip */
#define HIL_DEV_CHIP_TYPE_NETX4000     0x07   /*!< netX4000 Chip */
#define HIL_DEV_CHIP_TYPE_NETX4100     0x08   /*!< netX4100 Chip */
#define HIL_DEV_CHIP_TYPE_NETX90       0x09   /*!< netX90 Chip */
#define HIL_DEV_CHIP_TYPE_NETIOL       0x0A   /*!< netIOL Chip */
#define HIL_DEV_CHIP_TYPE_NETXXXL_MPW  0x0B   /*!< netXXXL MPW Chip */
/*! \} */


/*! \addtogroup HIL_HW_ASSEMBLY_doc
 *
 * The assembly option defines the physical hardware interface connected to a
 * netX xC (Communication Controller) or xPIC port.
 * The defines will be used in the following locations:
 *  - Fileheader V3 (HIL_FILE_DEVICE_INFO_V1_0_T)
 *  - Second state loader
 *  - Device data (secmem, flash device lable, ...)
 *  - Configuration utilities (sycon.net, taglist editor)
 *
 * \{ */
#define HIL_HW_ASSEMBLY_UNDEFINED                 0x0000
#define HIL_HW_ASSEMBLY_NOT_AVAILABLE             0x0001

#define HIL_HW_ASSEMBLY_VALIDATION_START          0x0010  /*!< Start of HW option validation area */

#define HIL_HW_ASSEMBLY_SERIAL                    0x0010  /*!< UART interface (e.g. RS232, RS485) */
#define HIL_HW_ASSEMBLY_ASI                       0x0020  /*!< AS-Interface                       */
#define HIL_HW_ASSEMBLY_CAN                       0x0030  /*!< CAN interface                      */
#define HIL_HW_ASSEMBLY_DEVICENET                 0x0040  /*!< DeviceNet interface                */
#define HIL_HW_ASSEMBLY_PROFIBUS                  0x0050  /*!< PROFIBUS interface                 */

#define HIL_HW_ASSEMBLY_CCLINK                    0x0070  /*!< CC-Link Fieldbus interface   */
#define HIL_HW_ASSEMBLY_CCLINK_IE_FIELD_1GB       0x0071  /*!< CC-Link IE Field interface   */

#define HIL_HW_ASSEMBLY_ETHERNET                  0x0080  /*!< Ethernet with internal PHY   */
#define HIL_HW_ASSEMBLY_ETHERNET_X_PHY            0x0081  /*!< Ethernet with external PHY   */
#define HIL_HW_ASSEMBLY_ETHERNET_FIBRE_OPTIC      0x0082  /*!< Fiber optic Ethernet         */
#define HIL_HW_ASSEMBLY_ETHERNET_TAP              0x0083  /*!< Passive Ethernet TAP         */

#define HIL_HW_ASSEMBLY_SPI                       0x0090  /*!< SPI                */
#define HIL_HW_ASSEMBLY_IO_LINK                   0x00A0  /*!< IO-LINK interface  */
#define HIL_HW_ASSEMBLY_COMPONET                  0x00B0  /*!< CompoNet interface */

#define HIL_HW_ASSEMBLY_VALIDATION_END            0xFFEF  /*!< End of HW option validation area */

#define HIL_HW_ASSEMBLY_I2C_UNKNOWN               0xFFF4
#define HIL_HW_ASSEMBLY_SSI                       0xFFF5
#define HIL_HW_ASSEMBLY_SYNC                      0xFFF6

#define HIL_HW_ASSEMBLY_FIELDBUS                  0xFFF8

#define HIL_HW_ASSEMBLY_TOUCH_SCREEN              0xFFFA
#define HIL_HW_ASSEMBLY_I2C_PIO                   0xFFFB
#define HIL_HW_ASSEMBLY_I2C_PIO_NT                0xFFFC
#define HIL_HW_ASSEMBLY_PROPRIETARY               0xFFFD
#define HIL_HW_ASSEMBLY_NOT_CONNECTED             0xFFFE
#define HIL_HW_ASSEMBLY_RESERVED                  0xFFFF
/*! \} */


/*! \addtogroup HIL_MANUFACTURER_doc
 *
 * The manufacturer code defines the manufacturer of the device (hardware).
 *
 * \note: All numbers above 0x00FF are assigned to OEM customers. Those numbers
 *        are not listed here.
 *
 * The defines will be used in the
 * following locations:
 *  - Fileheader V3 (HIL_FILE_DEVICE_INFO_V1_0_T)
 *  - Second state loader
 *  - Device data (secmem, flash device lable, ...)
 *  - Configuration utilities (sycon.net, taglist editor)
 *
 * \{ */
#define HIL_MANUFACTURER_UNDEFINED                          0x0000
#define HIL_MANUFACTURER_HILSCHER_GMBH                      0x0001    /*!< Hilscher GmbH */
#define HIL_MANUFACTURER_HILSCHER_GMBH_MAX                  0x00FF    /*!< Hilscher GmbH max. value */
/*! \} */


/*! \addtogroup HIL_HW_DEV_CLASS_doc
 *
 * The device class is used to create device groups specified by their functionality
 * and hardware options (e.g. used netX chip, special functionalities).
 * This is necessary because a netX firmware may not support all hardware
 * devices or just one specific device. The following device classes are valid
 * for when is set to HIL_MANUFACTURER_HILSCHER_GMBH
 *
 * The defines will be used in the
 * following locations:
 *  - Fileheader V3 (HIL_FILE_DEVICE_INFO_V1_0_T)
 *  - Second state loader
 *  - Device data (secmem, flash device lable, ...)
 *  - Configuration utilities (sycon.net, taglist editor)
 *
 * \{ */
#define HIL_HW_DEV_CLASS_UNDEFINED                          0x0000
#define HIL_HW_DEV_CLASS_UNCLASSIFIABLE                     0x0001
#define HIL_HW_DEV_CLASS_CHIP_NETX_500                      0x0002
#define HIL_HW_DEV_CLASS_CIFX                               0x0003
#define HIL_HW_DEV_CLASS_COMX_100                           0x0004
#define HIL_HW_DEV_CLASS_EVA_BOARD                          0x0005
#define HIL_HW_DEV_CLASS_NETDIMM                            0x0006
#define HIL_HW_DEV_CLASS_CHIP_NETX_100                      0x0007
#define HIL_HW_DEV_CLASS_NETX_HMI                           0x0008
/* #define HIL_HW_DEV_CLASS_RESERVED (abandoned)            0x0009 */
#define HIL_HW_DEV_CLASS_NETIO_50                           0x000A
#define HIL_HW_DEV_CLASS_NETIO_100                          0x000B
#define HIL_HW_DEV_CLASS_CHIP_NETX_50                       0x000C
#define HIL_HW_DEV_CLASS_GW_NETPAC                          0x000D
#define HIL_HW_DEV_CLASS_GW_NETTAP                          0x000E
#define HIL_HW_DEV_CLASS_NETSTICK                           0x000F
#define HIL_HW_DEV_CLASS_NETANALYZER                        0x0010
#define HIL_HW_DEV_CLASS_NETSWITCH                          0x0011
#define HIL_HW_DEV_CLASS_NETLINK                            0x0012
#define HIL_HW_DEV_CLASS_NETIC_50                           0x0013
#define HIL_HW_DEV_CLASS_NPLC_C100                          0x0014
#define HIL_HW_DEV_CLASS_NPLC_M100                          0x0015
#define HIL_HW_DEV_CLASS_GW_NETTAP_50                       0x0016
#define HIL_HW_DEV_CLASS_NETBRICK                           0x0017
#define HIL_HW_DEV_CLASS_NPLC_T100                          0x0018
#define HIL_HW_DEV_CLASS_NETLINK_PROXY                      0x0019
#define HIL_HW_DEV_CLASS_CHIP_NETX_10                       0x001A
#define HIL_HW_DEV_CLASS_NETJACK_10                         0x001B
#define HIL_HW_DEV_CLASS_NETJACK_50                         0x001C
#define HIL_HW_DEV_CLASS_NETJACK_100                        0x001D
#define HIL_HW_DEV_CLASS_NETJACK_500                        0x001E
#define HIL_HW_DEV_CLASS_NETLINK_10_USB                     0x001F
#define HIL_HW_DEV_CLASS_COMX_10                            0x0020
#define HIL_HW_DEV_CLASS_NETIC_10                           0x0021
#define HIL_HW_DEV_CLASS_COMX_50                            0x0022
#define HIL_HW_DEV_CLASS_NETRAPID_10                        0x0023
#define HIL_HW_DEV_CLASS_NETRAPID_50                        0x0024
#define HIL_HW_DEV_CLASS_NETSCADA_T51                       0x0025
#define HIL_HW_DEV_CLASS_CHIP_NETX_51                       0x0026
#define HIL_HW_DEV_CLASS_NETRAPID_51                        0x0027
#define HIL_HW_DEV_CLASS_GW_EU5C                            0x0028
#define HIL_HW_DEV_CLASS_NETSCADA_T50                       0x0029
#define HIL_HW_DEV_CLASS_NETSMART_50                        0x002A
#define HIL_HW_DEV_CLASS_IOLINK_GW_51                       0x002B
#define HIL_HW_DEV_CLASS_NETHMI_B500                        0x002C
#define HIL_HW_DEV_CLASS_CHIP_NETX_52                       0x002D
#define HIL_HW_DEV_CLASS_COMX_51                            0x002E
#define HIL_HW_DEV_CLASS_NETJACK_51                         0x002F
#define HIL_HW_DEV_CLASS_NETHOST_T100                       0x0030
#define HIL_HW_DEV_CLASS_NETSCOPE_C100                      0x0031
#define HIL_HW_DEV_CLASS_NETRAPID_52                        0x0032
#define HIL_HW_DEV_CLASS_NETSMART_T51                       0x0033
#define HIL_HW_DEV_CLASS_NETSCADA_T52                       0x0034
#define HIL_HW_DEV_CLASS_NETSAFETY_51                       0x0035
#define HIL_HW_DEV_CLASS_NETSAFETY_52                       0x0036
#define HIL_HW_DEV_CLASS_NETPLC_J500                        0x0037
#define HIL_HW_DEV_CLASS_NETIC_52                           0x0038
#define HIL_HW_DEV_CLASS_GW_NETTAP_151                      0x0039
#define HIL_HW_DEV_CLASS_CHIP_NETX_4000_COM                 0x003A
/* #define HIL_HW_DEV_CLASS_CIFX_4000 (abandoned)           0x003B */
#define HIL_HW_DEV_CLASS_CHIP_NETX_90_COM                   0x003C
#define HIL_HW_DEV_CLASS_NETRAPID_51_IO                     0x003D
#define HIL_HW_DEV_CLASS_GW_NETTAP_151_CCIES                0x003E
#define HIL_HW_DEV_CLASS_CIFX_CCIES                         0x003F
#define HIL_HW_DEV_CLASS_COMX_51_CCIES                      0x0040
#define HIL_HW_DEV_CLASS_NIOT_E_NPEX_BP52_IO                0x0041
#define HIL_HW_DEV_CLASS_NIOT_E_NPEX_BP52_IOL               0x0042
#define HIL_HW_DEV_CLASS_CHIP_NETX_4000_COM_HIFSDR          0x0043
#define HIL_HW_DEV_CLASS_CHIP_NETX_4000_COM_SDR             0x0044
#define HIL_HW_DEV_CLASS_CHIP_NETX_90_COM_HIFSDR            0x0045
#define HIL_HW_DEV_CLASS_CHIP_NETX_90_APP_FOR_COM_USECASE_A HIL_HW_DEV_CLASS_CHIP_NETX_90_COM
#define HIL_HW_DEV_CLASS_CHIP_NETX_90_APP_FOR_COM_USECASE_C HIL_HW_DEV_CLASS_CHIP_NETX_90_COM_HIFSDR
#define HIL_HW_DEV_CLASS_NETFIELD_COM                       0x0046
#define HIL_HW_DEV_CLASS_NETFIELD_APP_FOR_NETFIELD_COM      HIL_HW_DEV_CLASS_NETFIELD_COM
#define HIL_HW_DEV_CLASS_COMX_52                            0x0047
#define HIL_HW_DEV_CLASS_NETFIELD_DEV_IOLM_W                0x0048
/*                                                          0x0049 device class is worn out */
/*                                                          0x004A device class is worn out */
/*                                                          0x004B device class is worn out */
#define HIL_HW_DEV_CLASS_NETJACK_52                         0x004C

/* NOTE: The device class will be assigned by TD department. */

#define HIL_HW_DEV_CLASS_HILSCHER_GMBH_MAX                  0x7FFF /*!< Hilscher GmbH max. value */
#define HIL_HW_DEV_CLASS_OEM_DEVICE                         0xFFFE /*!< OEM device */
/*! \} */


/*! \addtogroup HIL_COMM_CLASS_doc
 *
 * Communication class definition.
 *
 * The defines will be used in the
 * following locations:
 *  - Fileheader V3 (HIL_FILE_DEVICE_INFO_V1_0_T)
 *  - Second state loader
 *  - Device data (secmem, flash device lable, ...)
 *  - Configuration utilities (sycon.net, taglist editor)
 *
 * \{ */
#define HIL_COMM_CLASS_UNDEFINED                            0x0000
#define HIL_COMM_CLASS_UNCLASSIFIABLE                       0x0001
#define HIL_COMM_CLASS_MASTER                               0x0002
#define HIL_COMM_CLASS_SLAVE                                0x0003
#define HIL_COMM_CLASS_SCANNER                              0x0004
#define HIL_COMM_CLASS_ADAPTER                              0x0005
#define HIL_COMM_CLASS_MESSAGING                            0x0006
#define HIL_COMM_CLASS_CLIENT                               0x0007
#define HIL_COMM_CLASS_SERVER                               0x0008
#define HIL_COMM_CLASS_IO_CONTROLLER                        0x0009
#define HIL_COMM_CLASS_IO_DEVICE                            0x000A
#define HIL_COMM_CLASS_IO_SUPERVISOR                        0x000B
#define HIL_COMM_CLASS_GATEWAY                              0x000C
#define HIL_COMM_CLASS_MONITOR                              0x000D
#define HIL_COMM_CLASS_PRODUCER                             0x000E
#define HIL_COMM_CLASS_CONSUMER                             0x000F
#define HIL_COMM_CLASS_SWITCH                               0x0010
#define HIL_COMM_CLASS_HUB                                  0x0011
#define HIL_COMM_CLASS_COMBI                                0x0012
#define HIL_COMM_CLASS_MANAGING_NODE                        0x0013
#define HIL_COMM_CLASS_CONTROLLED_NODE                      0x0014
#define HIL_COMM_CLASS_PLC                                  0x0015
#define HIL_COMM_CLASS_HMI                                  0x0016
#define HIL_COMM_CLASS_ITEM_SERVER                          0x0017
#define HIL_COMM_CLASS_SCADA                                0x0018
#define HIL_COMM_CLASS_IO_CONTROLLER_SYSTEMREDUNDANCY       0x0019
#define HIL_COMM_CLASS_IO_DEVICE_SYSTEMREDUNDANCY           0x001A

/* NOTE: The comm class will be assigned by TD department. */

/*! \} */

/*! \addtogroup HIL_PROT_CLASS_doc
 *
 * Fieldbus protocol or a specific process definition.
 *
 * The defines will be used in the
 * following locations:
 *  - Fileheader V3 (HIL_FILE_DEVICE_INFO_V1_0_T)
 *  - Second state loader
 *  - Device data (secmem, flash device lable, ...)
 *  - Configuration utilities (sycon.net, taglist editor)
 *
 * \{ */
#define HIL_PROT_CLASS_UNDEFINED                            0x0000
#define HIL_PROT_CLASS_3964R                                0x0001
#define HIL_PROT_CLASS_ASINTERFACE                          0x0002
#define HIL_PROT_CLASS_ASCII                                0x0003
#define HIL_PROT_CLASS_CANOPEN                              0x0004
#define HIL_PROT_CLASS_CCLINK                               0x0005
#define HIL_PROT_CLASS_COMPONET                             0x0006
#define HIL_PROT_CLASS_CONTROLNET                           0x0007
#define HIL_PROT_CLASS_DEVICENET                            0x0008
#define HIL_PROT_CLASS_ETHERCAT                             0x0009
#define HIL_PROT_CLASS_ETHERNET_IP                          0x000A
#define HIL_PROT_CLASS_FOUNDATION_FB                        0x000B
#define HIL_PROT_CLASS_FL_NET                               0x000C
#define HIL_PROT_CLASS_INTERBUS                             0x000D
#define HIL_PROT_CLASS_IO_LINK                              0x000E
#define HIL_PROT_CLASS_LON                                  0x000F
#define HIL_PROT_CLASS_MODBUS_PLUS                          0x0010
#define HIL_PROT_CLASS_MODBUS_RTU                           0x0011
#define HIL_PROT_CLASS_OPEN_MODBUS_TCP                      0x0012
#define HIL_PROT_CLASS_PROFIBUS_DP                          0x0013
#define HIL_PROT_CLASS_PROFIBUS_MPI                         0x0014
#define HIL_PROT_CLASS_PROFINET_IO                          0x0015
#define HIL_PROT_CLASS_RK512                                0x0016
#define HIL_PROT_CLASS_SERCOS_II                            0x0017
#define HIL_PROT_CLASS_SERCOS_III                           0x0018
#define HIL_PROT_CLASS_TCP_IP_UDP_IP                        0x0019
#define HIL_PROT_CLASS_POWERLINK                            0x001A
#define HIL_PROT_CLASS_HART                                 0x001B
#define HIL_PROT_CLASS_COMBI                                0x001C
#define HIL_PROT_CLASS_PROG_GATEWAY                         0x001D
#define HIL_PROT_CLASS_PROG_SERIAL                          0x001E
#define HIL_PROT_CLASS_PLC_CODESYS                          0x001F
#define HIL_PROT_CLASS_PLC_PROCONOS                         0x0020
#define HIL_PROT_CLASS_PLC_IBH_S7                           0x0021
#define HIL_PROT_CLASS_PLC_ISAGRAF                          0x0022
#define HIL_PROT_CLASS_VISU_QVIS                            0x0023
#define HIL_PROT_CLASS_ETHERNET                             0x0024
#define HIL_PROT_CLASS_RFC1006                              0x0025
#define HIL_PROT_CLASS_DF1                                  0x0026
#define HIL_PROT_CLASS_VARAN                                0x0027
#define HIL_PROT_CLASS_3S_PLC_HANDLER                       0x0028
#define HIL_PROT_CLASS_ATVISE                               0x0029
#define HIL_PROT_CLASS_MQTT                                 0x002A
#define HIL_PROT_CLASS_OPCUA                                0x002B
#define HIL_PROT_CLASS_CCLINK_IE_BASIC                      0x002C
#define HIL_PROT_CLASS_CCLINK_IE_FIELD                      0x002D
#define HIL_PROT_CLASS_NETWORK_SERVICES                     0x002E
#define HIL_PROT_CLASS_NETPROXY                             0x002F
#define HIL_PROT_CLASS_PROTOCOL_DETECT                      0x0030
#define HIL_PROT_CLASS_TSN_CORE                             0x0031
#define HIL_PROT_CLASS_IEEE_802_1_AS                        0x0032

/* NOTE: The protocol class will be assigned by TD department. */

#define HIL_PROT_CLASS_OEM                                  0xFFF0
/*! \} */


/*! \addtogroup HIL_CONFORMANCE_CLASS_doc
 *
 * Conformance class definition for usage with network service protocol class.
 *
 * The defines will only be used in the combination with
 * protocol class HIL_PROT_CLASS_NETWORK_SERVICES.
 * The defines are used as individual bits.
 *
 * \{ */
#define HIL_CONF_CLASS_FLAG_NDIS_AWARE                      0x0001

/*! \} */


#endif /*  HIL_SHAREDDEFINES_H_ */
