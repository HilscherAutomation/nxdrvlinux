/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/V1.0.2.0/includes/Hil_Taglist.h $: *//**

  \file Hil_Taglist.h

  The Hilscher tag list is a data structure which can be added to the binary firmware
  file. The data which is stored within this list can be manipulated via a special
  tag list editor tool without recompiling the firmware itself.

  The firmware also can access the data of the tag list and use the data to accept
  behavior or configurations which is compiled into the firmware binary.

  This file defines all related tag codes and structures.

**************************************************************************************/
#ifndef HIL_TAGLIST_H_
#define HIL_TAGLIST_H_

#include <stdint.h>
#include "Hil_Compiler.h"

/**************************************************************************************
  Tag type code ranges and segmentation.
**************************************************************************************/

/** Tag type code modifier, if set the tag shall be ignored by the firmware */
#define HIL_TAG_IGNORE_FLAG                               0x80000000

/** Tag type mask */
#define HIL_TAG_MASK                                      0x7FFFFFFF

/* Tag type ranges */
#define HIL_TAG_SPECIAL_START                             0x00000000
#define HIL_TAG_SPECIAL_END                               0x000007FF

#define HIL_TAG_GENERAL_START                             0x00000800
#define HIL_TAG_GENERAL_END                               0x00000FFF

#define HIL_TAG_FIRMWARE_START                            0x00001000
#define HIL_TAG_FIRMWARE_END                              0x0FFFFFFF

#define HIL_TAG_FACILITY_START                            0x10000000
#define HIL_TAG_FACILITY_END                              0x1FFFFFFF

#define HIL_TAG_USER_START                                0x20000000
#define HIL_TAG_USER_END                                  0x2FFFFFFF

#define HIL_TAG_PROTOCOL_START                            0x30000000
#define HIL_TAG_PROTOCOL_END                              0x3FFFFFFF

#define HIL_TAG_BSL_START                                 0x40000000
#define HIL_TAG_BSL_END                                   0x4FFFFFFF



/**************************************************************************************
  Tag type codes.
  Note: New tag codes are coordinated by the netX tools department (NXT).
**************************************************************************************/

/* Tag types from the special tag code range */
#define HIL_TAG_END_OF_LIST                               0x00000000



/* Tag types from the general tag code range */
#define HIL_TAG_MEMSIZE                                   0x00000800
#define HIL_TAG_MIN_PERSISTENT_STORAGE_SIZE               0x00000801
#define HIL_TAG_MIN_OS_VERSION                            0x00000802
#define HIL_TAG_MAX_OS_VERSION                            0x00000803
#define HIL_TAG_MIN_CHIP_REV                              0x00000804
#define HIL_TAG_MAX_CHIP_REV                              0x00000805
#define HIL_TAG_NUM_COMM_CHANNEL                          0x00000806  /* deprecated */



/* Tag types from the firmware tag code range */
#define HIL_TAG_TASK_GROUP                                0x00001000
#define HIL_TAG_IT_STATIC_TASK_PARAMETER_BLOCK            0x00001001  /* deprecated */
#define HIL_TAG_IT_STATIC_TASK_ENTRY                      0x00001002  /* deprecated */
#define HIL_TAG_TASK                                      0x00001003

#define HIL_TAG_TIMER                                     0x00001010

#define HIL_TAG_INTERRUPT_GROUP                           0x00001020
#define HIL_TAG_INTERRUPT                                 0x00001023

#define HIL_TAG_UART                                      0x00001030

#define HIL_TAG_LED                                       0x00001040
#define HIL_TAG_IOPIN                                     0x00001041
#define HIL_TAG_SWAP_LNK_ACT_LED                          0x00001042

#define HIL_TAG_XC                                        0x00001050

#define HIL_TAG_DPM_COMM_CHANNEL                          0x00001060
#define HIL_TAG_DPM_SETTINGS                              0x00001061
#define HIL_TAG_DPM_BEHAVIOUR                             0x00001062

#define HIL_TAG_REMANENT_DATA_RESPONSIBLE                 0x00001070

#define HIL_TAG_DDP_MODE_AFTER_STARTUP                    0x00001081

#define HIL_TAG_PHY_ENABLE_TIMEOUT                        0x00001090


/* Tag types from the Ethernet Interface facility tag code range */
#define HIL_TAG_PNS_ETHERNET_PARAMS                       0x100F0000

#define HIL_TAG_EIF_EDD_CONFIG                            0x105D0000
#define HIL_TAG_EIF_EDD_INSTANCE                          0x105D0001
#define HIL_TAG_EIF_NDIS_ENABLE                           0x105D0002



/* Tag types from the netX Diagnostics and Remote Access facility tag code range */
#define HIL_TAG_DIAG_IF_CTRL_UART                         0x10820000
#define HIL_TAG_DIAG_IF_CTRL_USB                          0x10820001
#define HIL_TAG_DIAG_IF_CTRL_TCP                          0x10820002
#define HIL_TAG_DIAG_TRANSPORT_CTRL_CIFX                  0x10820010
#define HIL_TAG_DIAG_TRANSPORT_CTRL_PACKET                0x10820011

/* Tag types for specific usecase */
#define HIL_TAG_HTTP_PORT_CONFIG                          0x10920000
#define HIL_TAG_HTTPS_PORT_CONFIG                         0x10920001
#define HIL_TAG_WEBSERVER_ENABLE                          0x10920002

#define HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX100_PARAMS     0x10960000
#define HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX50_PARAMS      0x10960001

#define HIL_TAG_NETPLC_IO_HANDLER_ENABLE                  0x10a30000 /* tag structure description is not available */
#define HIL_TAG_NETPLC_IO_HANDLER_DIGITAL                 0x10a30001 /* tag structure description is not available */
#define HIL_TAG_NETPLC_IO_HANDLER_ANALOG                  0x10a30002 /* tag structure description is not available */

                                                       /* 0x10e00000 Tag id is worn out */
#define HIL_TAG_NF_GEN_DIAG_RESOURCES                     0x10e00001
#define HIL_TAG_NF_PROFI_ENERGY_MODES                     0x10e00002
#define HIL_TAG_NF_PN_IOL_PROFILE_PADDING                 0x10e00003
#define HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM             0x10e00004
#define HIL_TAG_NF_SWAP_COM_LEDS                          0x10e00005
#define HIL_TAG_NF_PN_IOL_PROFILE_CFG_FLAGS               0x10e00006

#define HIL_TAG_LWIP_PORTS_FOR_IP_ZERO                    0x10e90000
#define HIL_TAG_LWIP_NETIDENT_BEHAVIOUR                   0x10e90001
#define HIL_TAG_LWIP_QUANTITY_STRUCTURE                   0x10e90002
#define HIL_TAG_LWIP_AMOUNT_SOCKET_API_MULTICAST_GROUPS   0x10e90003

/* Tag types from the user defined tag code range */



/* Tag types from the protocol tag code range */
/* TagID is 0x3ppppnn where pppp is the protocol class and nnn is the identifier of the specific tag */
#define HIL_TAG_CO_DEVICEID                               0x30004000

#define HIL_TAG_CCL_DEVICEID                              0x30005000

#define HIL_TAG_COMPONET_DEVICEID                         0x30006000 /* tag structure description is not available */

#define HIL_TAG_DEVICENET_DEVICEID                        0x30008000
#define HIL_TAG_DEVICENET_CAN_SAMPLING                    0x30008001

#define HIL_TAG_ECS_DEVICEID                              0x30009000 /* tag structure description is not available */
#define HIL_TAG_ECS_ENABLE_BOOTSTRAP                      0x30009001 /* tag structure description is not available */
#define HIL_TAG_ECS_SELECT_SOE_COE                        0x30009002 /* tag structure description is not available */
#define HIL_TAG_ECS_CONFIG_EOE                            0x30009003 /* tag structure description is not available */
#define HIL_TAG_ECS_MBX_SIZE                              0x30009004 /* tag structure description is not available */
#define HIL_TAG_ECM_ENI_BUS_STATE                         0x30009005
#define HIL_TAG_ECS_EOE_MODE                              0x30009006

#define HIL_TAG_EIP_DEVICEID                              0x3000A000
#define HIL_TAG_EIP_EDD_CONFIGURATION                     0x3000A001 /* Tag is obsolete */
#define HIL_TAG_EIP_DLR_PROTOCOL                          0x3000A002
//#define HIL_TAG_EIP_EIS_CONFIG                          0x3000A003 /* Tag is obsolete */
#define HIL_TAG_EIP_RESOURCES                             0x3000A004
#define HIL_TAG_EIP_TIMESYNC_ENABLE_DISABLE               0x3000A005
#define HIL_TAG_EIP_FILE_OBJECT_ENABLE_DISABLE            0x3000A006

#define HIL_TAG_DP_DEVICEID                               0x30013000

#define HIL_TAG_PN_DEVICEID                               0x30015000
#define HIL_TAG_PROFINET_FEATURES                         0x30015001
#define HIL_TAG_PROFINET_FEATURES_V2                      0x30015002
#define HIL_TAG_PROFINET_SYSTEM_REDUNDANCY_FEATURES       0x30015003
#define HIL_TAG_PROFINET_CONTROLLER_QUANTITIES            0x30015004

#define HIL_TAG_S3S_DEVICEID                              0x30018000 /* tag structure description is not available */
#define HIL_TAG_S3S_SCP_FEATURES                          0x30018001

#define HIL_TAG_TCP_PORT_NUMBERS                          0x30019000

#define HIL_TAG_PLS_DEVICEID                              0x3001A000 /* tag structure description is not available */



/* Tag types from the 2nd stage loader tag code range */
#define HIL_TAG_BSL_SDRAM_PARAMS                          0x40000000 /* tag structure description is not available */
#define HIL_TAG_BSL_HIF_PARAMS                            0x40000001 /* tag structure description is not available */
#define HIL_TAG_BSL_SDMMC_PARAMS                          0x40000002 /* tag structure description is not available */
#define HIL_TAG_BSL_UART_PARAMS                           0x40000003 /* tag structure description is not available */
#define HIL_TAG_BSL_USB_PARAMS                            0x40000004 /* tag structure description is not available */
#define HIL_TAG_BSL_MEDIUM_PARAMS                         0x40000005 /* tag structure description is not available */
#define HIL_TAG_BSL_EXTSRAM_PARAMS                        0x40000006 /* tag structure description is not available */
#define HIL_TAG_BSL_HWDATA_PARAMS                         0x40000007 /* tag structure description is not available */
#define HIL_TAG_BSL_FSU_PARAMS                            0x40000008 /* tag structure description is not available */
#define HIL_TAG_BSL_MMIO_NETX50_PARAMS                    0x40000009 /* tag structure description is not available */
#define HIL_TAG_BSL_MMIO_NETX10_PARAMS                    0x4000000A /* tag structure description is not available */
#define HIL_TAG_BSL_HIF_NETX10_PARAMS                     0x4000000B /* tag structure description is not available */
#define HIL_TAG_BSL_USB_DESCR_PARAMS                      0x4000000C /* tag structure description is not available */
#define HIL_TAG_BSL_DISK_POS_PARAMS                       0x4000000D /* tag structure description is not available */
#define HIL_TAG_BSL_BACKUP_POS_PARAMS                     0x4000000E /* tag structure description is not available */
#define HIL_TAG_BSL_MMIO_NETX51_52_PARAMS                 0x4000000F /* tag structure description is not available */
#define HIL_TAG_BSL_HIF_NETX51_52_PARAMS                  0x40000010 /* tag structure description is not available */
#define HIL_TAG_BSL_SERFLASH_PARAMS                       0x40000011 /* tag structure description is not available */
#define HIL_TAG_BSL_SQIFLASH_PARAMS                       0x40000012 /* tag structure description is not available */
#define HIL_TAG_BSL_FLASH_LAYOUT_PARAMS                   0x40000013 /* tag structure description is not available */


/**************************************************************************************
  General tag list definitions
**************************************************************************************/

/** Macro for forcing an instance of a tag list or single tag into a separate
 * ".taglist" section (needed for NXFs) */
#define __SEC_TAGLIST__       __attribute__ ((section (".taglist")))

#define HIL_TAGLIST_START_TOKEN  "TagList>"
#define HIL_TAGLIST_END_TOKEN    "<TagList"

/** Taglist header.
 * Taglist for netX90/netX4000 based firmware has a proper header and footer.
 * This enclosure don't exist in nxf firmware files.
 *
 * File header of nxf files HIL_FILE_COMMON_HEADER_V3_0_T.ulTagListOffset points
 * to the first tag structure.
 * |  File header of nxi files HIL_FILE_COMMON_HEADER_V3_0_T.ulTagListOffset points
 * |  to this header structure.
 * |  |
 * |  \-> [Header] - HIL_TAGLIST_HEADER_T   <- not present in nxf files (this structure)
 * \----> [Tag]    - HIL_TAG_*
 *        [Tag]    - HIL_TAG_*
 *        ...      - ...
 *        [Tag]    - HIL_TAG_END_OF_LIST_T  <- Always required
 *        [Footer] - HIL_TAGLIST_FOOTER_T   <- Not present in nxf files */
typedef struct
{
  /** Start token of the taglist data area.
   * This field must contain the token sting defined by HIL_TAGLIST_START_TOKEN. */
  uint8_t abStartToken[8];

  /** Size of the taglist data area.
   * Note: This includes the Header and Footer and possible padding/ spare space */
  uint16_t  usTagListSize;

  /** Size of the filled taglist data.
   * Note: This is the size of all tags in the taglist without header,footer and spare space*/
  uint16_t  usContentSize;
} HIL_TAGLIST_HEADER_T;

/** Taglist footer.
 * Taglist footer for use with netX90/netX4000 based firmware. */
typedef struct
{
  /** Reserved, set to zero */
  uint32_t  ulReserved;

  /** End token of the taglist data area.
   * This field must contain the token string defined by HIL_TAGLIST_END_TOKEN. */
  uint8_t abEndToken[8];
} HIL_TAGLIST_FOOTER_T;

/** Tag header with type code and length of following tag data */
typedef struct __HIL_ALIGNED_DWORD__
{
  uint32_t ulTagType;
  uint32_t ulTagDataLength;
} HIL_TAG_HEADER_T;

/** Identifier string for named resources */
typedef struct
{
  char abName[16];
} HIL_TAG_IDENTIFIER_T;



/**************************************************************************************
  End of tag list tag definitions.
  Tag codes: HIL_TAG_END_OF_LIST
**************************************************************************************/
typedef struct
{
  HIL_TAG_HEADER_T tHeader;
} HIL_TAG_END_OF_LIST_T;



/**************************************************************************************
  UINT32 tag definitions.
  Tag codes: HIL_TAG_MEMSIZE, HIL_TAG_MIN_PERSISTENT_STORAGE_SIZE,
             HIL_TAG_MIN_CHIP_REV, HIL_TAG_MAX_CHIP_REV
**************************************************************************************/
typedef struct
{
  uint32_t ulValue;
} HIL_TAG_UINT32_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T      tHeader;
  HIL_TAG_UINT32_DATA_T tData;
} HIL_TAG_UINT32_T;



/**************************************************************************************
  Version tag definitions.
  Tag codes: HIL_TAG_MIN_OS_VERSION, HIL_TAG_MAX_OS_VERSION
**************************************************************************************/
typedef struct
{
  uint16_t usMajor;
  uint16_t usMinor;
  uint16_t usBuild;
  uint16_t usRevision;
} HIL_TAG_VERSION_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T       tHeader;
  HIL_TAG_VERSION_DATA_T tData;
} HIL_TAG_VERSION_T;



/**************************************************************************************
  Tag:  HIL_TAG_LED
  Name: LED Settings
  Desc: Configure physical LED connection settings.

  Help: This tag is used to modify physical LED connection settings in the firmware.
        The pin number for the LED determines which LED is configured by this setting.
**************************************************************************************/
/* resource codes for LED tag */
#define HIL_TAG_LED_RESOURCE_TYPE_GPIO              1
#define HIL_TAG_LED_RESOURCE_TYPE_PIO               2
#define HIL_TAG_LED_RESOURCE_TYPE_HIFPIO            3

/* polarity codes for LED tag */
#define HIL_TAG_LED_POLARITY_NORMAL                 0
#define HIL_TAG_LED_POLARITY_INVERTED               1

typedef struct
{
  /** rcX LED object identifier
   * Object identifier (read-only) */
  HIL_TAG_IDENTIFIER_T tIdentifier;

  /** Resource type
   * The type of I/O (PIO, GPIO) used to control this LED
   * - 1: GPIO
   * - 2: PIO
   * - 3: HIF PIO */
  uint32_t             ulUsesResourceType;

  /** Pin Number
   * - 0: COM0_GREEN
   * - 1: COM0_RED
   * - 2: COM1_GREEN
   * - 3: COM1_RED
   * The number of the pin the LED is connected to */
  uint32_t             ulPinNumber;

  /** Polarity
   * Polarity of the LED.
   * - 0: Normal
   * - 1: Inverted */
  uint32_t             ulPolarity;

} HIL_TAG_LED_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T   tHeader;
  HIL_TAG_LED_DATA_T tData;
} HIL_TAG_LED_T;



/**************************************************************************************
  Tag: HIL_TAG_IOPIN
  Name: IO Pin Definition
  Desc: Configures an IO pin.

  Help: This tag is used to define and configure an IO pin.

        The function of the pin and its direction (input/output) is defined by the firmware
        and is indicated by the identifier string:

        |Identifier     |Direction  |Function                                       |
        |---------------|-----------|------------------------------------------------|
        |Ready          | Output    | Indicates if the device is ready               |
        |Start/Stop Com | Input     | Tells the device to start or stop communication|
**************************************************************************************/
typedef struct
{
  /** rcX LED object identifier
   * Object identifier (read-only) */
  HIL_TAG_IDENTIFIER_T tIdentifier;

  /** Resource type
   * The type of I/O (PIO, GPIO) used to control this pin
   * - 1: GPIO
   * - 2: PIO
   * - 3: HIF PIO */
  uint32_t             ulUsesResourceType;

  /** Pin number
   * Number of the pin */
  uint32_t             ulPinNumber;

  /** Polarity
   * Polarity of the pin.
   * - 0: Normal
   * - 1: Inverted */
  uint32_t             ulPolarity;

} HIL_TAG_IOPIN_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T     tHeader;
  HIL_TAG_IOPIN_DATA_T tData;
} HIL_TAG_IOPIN_T;



/**************************************************************************************
  Tag: HIL_TAG_SWAP_LNK_ACT_LED
  Name: Swap Link/Activity LEDs
  Desc: Configure swapping of the Link/Activity LEDs

  Help: This tag allows swapping the Link and Activity LED output.
**************************************************************************************/
typedef struct
{
  /** Swap Link/Activity
   * - 0: Not swapped
   * - 1: Swap LEDs */
  uint32_t bSwapLnkActLeds;
} HIL_TAG_SWAP_LNK_ACT_LED_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                tHeader;
  HIL_TAG_SWAP_LNK_ACT_LED_DATA_T tData;
} HIL_TAG_SWAP_LNK_ACT_LED_T;



/**************************************************************************************
  Tag:  HIL_TAG_TASK_GROUP
  Name: Task Group Settings
  Desc: Configures the priority of an group of task with the same group reference number.

  Help: This tag allows to reconfigure the key settings of a group of tasks.
        Using the tag parameters, you can add a common base offset to the priority and token entries
        of all tasks within this group while preserving their relative priorities.

        This feature is useful for manipulating tasks which have a close relationship.

        E.g.: A firmware could use different task groups for tasks with critical functionality
        running in a high priority band and for other tasks with less critical functionality
        running in a lower priority band.

        Please note that each rcX task needs to have a unique token and a unique priority.

        *Recommendation*: In order to avoid conflicts, always use identical values for task priorities
        and for the associated task tokens.

        Task priorities `TSK_PRIO_1` and `TSK_PRIO_2` should be reserved for tasks which are critical
        to the system. Please consult the rcX Configuration and the Kernel API Function Reference
        for more details about task priorities and tokens.
**************************************************************************************/
typedef struct
{
  /** Group Name (read-only) */
  char      szTaskListName[64];

  /** Base Priority
   * Base value (=offset) added to the preconfigured task priorities of all tasks in the group.
   * - TSK_PRIO_1..TSK_PRIO_n: Where n is (56 - Task Range), if the preconfigured task priorities are in the range of 0..(Task Range - 1) */
  uint32_t  ulBasePriority;

  /** Base Token
   * Base value (=offset) added to the preconfigured task tokens of all tasks in the group.
   * - TSK_TOK_1..TSK_TOK_n: Where n is (56 - Task Range), if the preconfigured task tokens are in the range of 0..(Task Range - 1) */
  uint32_t  ulBaseToken;

  /** Task Range
   * Number of tasks in the group.
   * This read-only parameter is useful when checking the allowed range for `Base Priority` and `Base Token`. */
  uint32_t  ulRange;

  /** Task Group Reference
   * Group reference number common to all tasks in the group.
   * This read-only parameter can be used to identify the tasks belonging in the group. */
  uint32_t  ulTaskGroupRef;

} HIL_TAG_TASK_GROUP_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T          tHeader;
  HIL_TAG_TASK_GROUP_DATA_T tData;
} HIL_TAG_TASK_GROUP_T;



/**************************************************************************************
  Tag:  HIL_TAG_TASK
  Name: Task Settings
  Desc: Configures an individual task priority.

  Help: This tag allows to configure the key settings of an individual task.
        Using the tag parameters, you can reassign the priority and token parameters of a preconfigured task.
        This feature is especially useful when adapting a standard firmware for use with specific hardware.

        Please note that each rcX task needs to have a unique token and a unique priority.
        In order to avoid conflicts, it is recommended to always use identical values for
        task priority the associated task token.

        Task priorities `TSK_PRIO_1` and `TSK_PRIO_2` should be reserved for tasks which are
        critical to the system. Please consult the rcX Configuration and the Kernel API Function Reference
        for more details about task priorities and tokens.
**************************************************************************************/
typedef struct
{
  /** rcX Task Object Identifier
   * Object identifier (read-only) */
  HIL_TAG_IDENTIFIER_T  tIdentifier;

  /** Priority
   * - TSK_PRIO_1..TSK_PRIO_55: (Offset) Value that replaces the preconfigured task priority. */
  uint32_t              ulPriority;

  /** Token
   * - TSK_TOK_1..TSK_TOK_55: (Offset) Value that replaces the preconfigured task token. */
  uint32_t              ulToken;
} HIL_TAG_TASK_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T      tHeader;
  HIL_TAG_TASK_DATA_T   tData;
} HIL_TAG_TASK_T;



/**************************************************************************************
  Tag:  HIL_TAG_INTERRUPT_GROUP
  Name: Interrupt Group
  Desc: Configures the priority of interrupts with the same group reference number.

  Help: This tag allows to reconfigure the key settings of a group of interrupts.
        Using the tag parameters, you can add a common base offset to the priority and token entries
        of all interrupts within this group while preserving their relative priorities.
        This feature is useful for manipulating interrupts which have a close relationship.

        E.g.: A firmware could use an interrupt group for adapting the interrupts used for servicing
        a hardware interface with multiple ports.

        The group may contain interrupts which execute as rcX tasks.
        In this case you must configure the Task Priority Base and the Task Token Base.
        (These values have no relevance for interrupts which do not execute as rcX tasks).

        Task priorities `TSK_PRIO_1` and `TSK_PRIO_2` should be reserved for tasks which are critical
        to the system. Please consult the rcX Configuration and the Kernel API Function Reference
        for more details about task priorities and tokens.

        *Recommendation*: In order to avoid conflicts, always use identical values
        for task priorities and for the associated task tokens.
**************************************************************************************/
typedef struct
{
  /** Interrupt Group name
   * Group name (read-only) */
  char      szInterruptListName[64];

  /** Interrupt Priority Base
   * - 0..(32 - Interrupt Priority Range): Base value (=offset) added to the preconfigured interrupt priorities of all interrupts in the group. */
  uint32_t  ulBaseIntPriority;

  /** Number of interrupts in the group (read-only).
   * - 0..(Interrupt Priority Range - 1): This read-only parameter is useful when checking the allowed range for Interrupt Priority Base, provided that the preconfigured interrupt priority values are in the range. */
  uint32_t  ulRangeInt;

  /** Task Priority Base
   * - TSK_PRIO_1..TSK_PRIO_n: Where n is (56 - Task Range Size), if the preconfigured task priorities are in the range of 0..(Task Range - 1).
   * Base value (=offset) added to the preconfigured task priorities of all interrupts in the group that execute as rcX tasks. */
  uint32_t  ulBaseTaskPriority;

  /** Task Token Base
   * - TSK_TOK_1..TSK_TOK_n: Where n is (56 - Task Range Size), if the preconfigured task priorities are in the range of 0..(Task Range - 1).
   * Base value (=offset) added to the preconfigured task tokens of all interrupts in the group that execute as rcX tasks. */
  uint32_t  ulBaseTaskToken;

  /** Task Range
   * Number of interrupts in the group that execute in task mode.
   * This read-only parameter helps to calculate the allowed ranges for Task Priority Base and Task Token Base,
   * provided that the preconfigured task priority and task token values are in the range of 0..(Task Range - 1). */
  uint32_t  ulRangeTask;

  /** Interrupt Group Reference
   * Group reference number common to all interrupts in the group.
   * This read-only parameter can be used to identify the interrupts belonging to the group. */
  uint32_t  ulInterruptGroupRef;

} HIL_TAG_INTERRUPT_GROUP_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T               tHeader;
  HIL_TAG_INTERRUPT_GROUP_DATA_T tData;
} HIL_TAG_INTERRUPT_GROUP_T;



/**************************************************************************************
  Tag:  HIL_TAG_INTERRUPT
  Name: Interrupt Settings
  Desc: Configures an individual interrupt's priority.

  Help: This tag allows to configure the key settings of an individual interrupt.
        Using the tag parameters, you can reassign the priority and token parameters of a
        preconfigured interrupt. This feature is especially useful when adapting a standard
        firmware for use with specific hardware.

        If the interrupt executes as an rcX task, you must configure the Task Priority and the Task Token.
        (These values have no relevance for interrupts which do not execute as rcX tasks.)

        Task priorities `TSK_PRIO_1` and `TSK_PRIO_2` should be reserved for tasks which are
        critical to the system. Please consult the rcX Configuration and the Kernel API Function Reference
        for more details about task priorities and tokens.

        *Recommendation*: In order to avoid conflicts, always use identical values for task priorities
        and for the associated task tokens.
**************************************************************************************/
typedef struct
{
  /** rcX interrupt object identifier
   * Object identifier (read-only) */
  HIL_TAG_IDENTIFIER_T  tIdentifier;

  /** Interrupt Priority
   * - 0..31: (Offset) Value that replaces the preconfigured interrupt priority. */
  uint32_t              ulInterruptPriority;

  /** Task Priority
   * - TSK_PRIO_1..TSK_PRIO_55: (Offset) value that replaces the preconfigured task priority if the interrupt executes as an rcX task. */
  uint32_t              ulTaskPriority;

  /** Task Token
   * - TSK_TOK_1..TSK_TOK_55: (Offset) value that replaces the preconfigured task token if the interrupt executes as an rcX task. */
  uint32_t              ulTaskToken;
} HIL_TAG_INTERRUPT_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T         tHeader;
  HIL_TAG_INTERRUPT_DATA_T tData;
} HIL_TAG_INTERRUPT_T;



/**************************************************************************************
  Tag:  HIL_TAG_TIMER
  Name: Hardware Timer
  Desc: Configures a timer number.

  Help: With this tag you can set the value for a hardware timer.
**************************************************************************************/
typedef struct
{
  /** Identifier
   * Name of the timer instance (read-only) */
  HIL_TAG_IDENTIFIER_T  tIdentifier;

  /** Timer Number
   * Number of the physical GPIO timer. */
  uint32_t              ulTimNum;
} HIL_TAG_TIMER_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T     tHeader;
  HIL_TAG_TIMER_DATA_T tData;
} HIL_TAG_TIMER_T;



/**************************************************************************************
  Tag:  HIL_TAG_UART
  Name: UART
  Desc: Configures UART settings.

  Help: With this tag you can configure various settings for the UART protocol.
**************************************************************************************/
typedef struct
{
  /** rcX UART Object Identifier
   * Object identifier (read-only) */
  HIL_TAG_IDENTIFIER_T  tIdentifier;

  /** UART Number
   * Specifies the UART interface
   * - 0..2: Use the specified UART interface */
  uint32_t              ulUrtNumber;

  /** UART Baud Rate
   * Set the UART Baud Rate
   * - 3: 300 Hz
   * - 6: 600 Hz
   * - 12: 1200 Hz
   * - 24: 2400 Hz
   * - 48: 4800 Hz
   * - 96: 9600 Hz
   * - 192: 19200 Hz
   * - 384: 38400 Hz
   * - 576: 57600 Hz
   * - 1152: 115200 Hz */
  uint32_t              ulBaudRate;

  /** Parity Checking
   * Set the parity check preference
   * - 0: None
   * - 1: Odd
   * - 2: Even */
  uint32_t              ulParity;

  /** Stop bits
   * Number of stop bits
   * - 0: 1 Stop bit
   * - 1: 2 Stop bits */
  uint32_t              ulStopBits;

  /** Data bits
   * Number of data bits
   * - 1: 5 data bits
   * - 2: 6 data bits
   * - 3: 7 data bits
   * - 4: 8 data bits
   * - 5: 9 data bits */
  uint32_t              ulDataBits;

  /** Receive (Rx) FIFO trigger level
   * Configures the receive FIFO and the "Rx FIFO ready" IRQ trigger level.
   * - 0: Disables FIFO and forces immediate notification for each character received.
   * - 1..16: Enables FIFO. An IRQ is triggered when the receive FIFO fill level is greater than or equal to the value provided (recommended: 12). */
  uint32_t              ulRxFifoLevel;

  /** Transfer (Tx) FIFO trigger level
   * Configures the transmit FIFO and the "Tx FIFO empty" IRQ trigger level.
   * - 0: Disables FIFO and forces immediate sending or each character.
   * - 1..16: Enables FIFO. An IRQ is triggered when the number of characters left in the FIFO is less than the value (recommended: 4). */
  uint32_t              ulTxFifoLevel;

  /** RTS Mode
   * Configures the "Request-to-Send" Mode
   * - 0: None, RTS is not used.
   * - 1: Auto/Bits: RTS is controlled by the driver. RTS forerun/trail are given in bit times.
   * - 2: Auto/Cycles: RTS is controlled by the driver. RTS forerun/trail are given in clock cycles.
   * - 4: Self: RTS is controlled by the application. */
  uint32_t              ulRtsMode;

  /** RTS Polarity
   * Configures the polarity of the "Request-to-Send" signal
   * - 0: Default
   * - 1: Active High
   * - 2: Active Low */
  uint32_t              ulRtsPolarity;

  /** RTS Forerun
   * If RTS Mode is set to Auto/Bits or Auto/Cycles, this value defines the delay between
   * the assertion of RTS and the beginning of the start bit. The unit of this value is
   * either bit times or clock cycles, depending on the value of RTS mode.
   * See netX Technical Reference Guide, Electrical Specifications, UART.
   * - 0..255: Range
   * - Bit times: 1..5 (recommended)
   * - Clock cycles: 1..10 (recommended) */
  uint32_t              ulRtsForerun;

  /** RTS Trail
   * If RTS Mode is set to Auto/Bits or Auto/Cycles, this value defines the delay between
   * the end of the stop bit(s) and de-assertion of RTS. The unit of this value is
   * either bit times or clock cycles, depending on the value of RTS mode.
   * See netX Technical Reference Guide, Electrical Specifications, UART.
   * - 0..255: Range
   * - Bit times: 1..5 (recommended)
   * - Clock cycles: 1..10 (recommended) */
  uint32_t              ulRtsTrail;

  /** CTS Mode
   * Configures the "Clear-to-Send" mode
   * - 0: None: CTS is not used.
   * - 1: Auto: CTS is controlled by the driver.
   * - 2: Self: CTS is controlled by the application. */
  uint32_t              ulCtsMode;

  /** CTS Polarity
   * Configures the "Clear-to-Send" polarity
   * - 0: Default
   * - 1: Active High
   * - 2: Active Low */
  uint32_t              ulCtsPolarity;
} HIL_TAG_UART_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T    tHeader;
  HIL_TAG_UART_DATA_T tData;
} HIL_TAG_UART_T;



/**************************************************************************************
  Tag:  HIL_TAG_XC
  Name: xC Unit
  Desc: Modify the XC unit which should be used

  Help: This tag allows configuring the usage of the flexible communication processors (xC Unit).
**************************************************************************************/
typedef struct
{
  /** rcX xC object identifier
   * Object identifier (read-only) */
  HIL_TAG_IDENTIFIER_T  tIdentifier;

  /** netX xC unit number
   * Number of the xC unit to use
   * - 0..3: xC Unit 1 through 4 */
  uint32_t              ulXcId;
} HIL_TAG_XC_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T  tHeader;
  HIL_TAG_XC_DATA_T tData;
} HIL_TAG_XC_T;



/**************************************************************************************
  Tag:  HIL_TAG_DPM_COMM_CHANNEL
  Name: DPM Communication Channels
  Desc: Redefining DPM communication channels

  Help: This tag allows the user to modify the `RX_HIF_CHANNEL_Ts` and the `RX_HIF_CHANNEL_BLOCK_Ts`
        describing communication channels: the number of communication channels used and the sizes
        of the input/output images of each channel.

        *Note*: The I/O data size values *replace* the statically defined values at system startup time,
        i.e. they serve as *absolute values*. The values are also used for calculating the start offsets
        of the subsequent channels and blocks.

        *Caution:* If both the "DPM Settings" and "DPM Communication Channels" tags are present in the tag list,
        then the DPM size specified in the "DPM Settings" tag must be large enough to allow for the channel sizes
        defined in "DPM Communication Channels".

        === Restrictions and Calculation Rules

        * Each I/O area byte size must be an integral multiple of 4 (i.e. DWORD size).
        * The minimal byte size of an I/O area is 4.
          (This ensures that an application can always rely on the real presence of the I/O area.)
        * The maximum size is 27904.
        * I/O areas of 11520 bytes or above can only be used with a 32K DPM.
        * The total byte size of a communication channel must be an integral multiple of 256.
        * The total communication channel byte size is: `(4096 + ulInDataSize + ulOutDataSize)`

        === Fallback Behavior of the Base Firmware

        * If a given I/O area byte size is not an integral multiple of 4, then it is rounded to the next integral multiple of 4.
        * If a given I/O area byte size is smaller than 4, then it is set to 4.
        * If the resulting communication channel byte size is not an integral multiple of 256,
          then the size of the input area is increased so that the resulting communication channel byte size becomes an integral multiple of 256.
**************************************************************************************/
/** Maximum number of communication channels for DPM communication channels tag */
#define DPM_MAX_COMM_CHANNELS                   4

typedef struct
{
  /** Number of communication channels
   * - 0..4: Number of communication channels to be instantiated */
  uint32_t ulNumCommChannels;
  struct
  {
    /** Channel n Input Area size
     * Total size of the normal priority input data area (area 0)
     * Note: Please refer to the restrictions above.
     * - 4..27904: Size in bytes */
    uint32_t ulInDataSize;

    /** Channel n Output Area size
     * Total size of the normal priority output data area (area 0)
     * Note: Please refer to the restrictions above.
     * - 4..27904: Size in bytes */
    uint32_t ulOutDataSize;
  } atCommChannelSizes[DPM_MAX_COMM_CHANNELS];
} HIL_TAG_DPM_COMM_CHANNEL_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                  tHeader;
  HIL_TAG_DPM_COMM_CHANNEL_DATA_T   tData;
} HIL_TAG_DPM_COMM_CHANNEL_T;



/**************************************************************************************
  Tag:  HIL_TAG_DPM_SETTINGS
  Name: DPM Settings
  Desc: Modify the  DPM location and access.

  Help: This tag allows the user to modify the `RX_HIF_SET_T` describing DPM location and access.

        Note: The values _replace_ the statically defined values at system startup time,
        i.e. they serve as _absolute values_.

        *Caution*: If both the "DPM Settings" and "DPM Communication Channels" tags are present
        in the tag list, then the DPM size specified in the "DPM Settings" tag must be large enough
        to allow for the channel sizes defined in "DPM Communication Channels".

        === Restrictions

        * DPM Base Address >= 0x80 (The first 128 bytes are used by the exception vector table.)
        * DPM Size >= 0x1200 bytes (512 + 4096 bytes are needed for the system channel and a minimal communication channel.)
        * DPM Size must be a multiple of 256 bytes
        * DPM Base Address + DPM Size <= end of the INTRAM
**************************************************************************************/
typedef struct
{
  /** DPM mode
   * - 2: 8-bit
   * - 3: 16-bit
   * - 5: PCI (default) */
  uint32_t ulDpmMode;

  /** DPM size in bytes
   * - 16384: Default for comX
   * - 32768: For other targets */
  uint32_t ulDpmSize;

  /** DPM base address (in INTRAM)
   * - 0x00018000: Default */
  uint32_t ulDpmBaseAddress;
} HIL_TAG_DPM_SETTINGS_DATA_T;


typedef struct
{
  HIL_TAG_HEADER_T            tHeader;
  HIL_TAG_DPM_SETTINGS_DATA_T tData;
} HIL_TAG_DPM_SETTINGS_T;



/**************************************************************************************
  Tag:  HIL_TAG_DIAG_IF_CTRL_UART
  Name: UART Diagnostics Interface
  Desc: UART interface of netX Diagnostics and Remote Access component.

  Help: This tag controls the interface used by the
        netX Diagnostics and Remote Access component ("marshaller").

        Configure which transport protocol is used via the tags
        `HIL_TAG_DIAG_TRANSPORT_CTRL_CIFX` and `HIL_TAG_DIAG_TRANSPORT_CTRL_PACKET`.
**************************************************************************************/
typedef struct
{
  /** Enable UART Diagnostics Interface
   * - 0: Interface deactivated
   * - 1: Interface activated */
  uint8_t bEnableFlag;

  /** netX UART interface to use
   * Depending on the device, there might be more than one interface (Default: 0 (first interface)). */
  uint8_t bIfNumber;

  /** Reserved for future use */
  uint8_t abReserved[2];
} HIL_TAG_DIAG_IF_CTRL_UART_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                 tHeader;
  HIL_TAG_DIAG_IF_CTRL_UART_DATA_T tData;
} HIL_TAG_DIAG_IF_CTRL_UART_T;



/**************************************************************************************
  Tag:  HIL_TAG_DIAG_IF_CTRL_USB
  Name: USB Diagnostics Interface
  Desc: USB interface of netX Diagnostics and Remote Access component.

  Help: This tag controls the interface used by the
        netX Diagnostics and Remote Access component ("marshaller").

        Configure which transport protocol is used via the tags
        `HIL_TAG_DIAG_TRANSPORT_CTRL_CIFX` and `HIL_TAG_DIAG_TRANSPORT_CTRL_PACKET`.
**************************************************************************************/
typedef struct
{
  /** Enable USB Diagnostics Interface
   * - 0: FALSE, Do not use this interface
   * - 1: TRUE, Activate this interface */
  uint8_t bEnableFlag;

  /** netX USB interface to use
   * Currently, 0 is the only valid value */
  uint8_t bIfNumber;

  /** Reserved for future use */
  uint8_t abReserved[2];
} HIL_TAG_DIAG_IF_CTRL_USB_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                tHeader;
  HIL_TAG_DIAG_IF_CTRL_USB_DATA_T tData;
} HIL_TAG_DIAG_IF_CTRL_USB_T;



/**************************************************************************************
  Tag:  HIL_TAG_DIAG_IF_CTRL_TCP
  Name: TCP Diagnostics Interface
  Desc: TCP interface of netX Diagnostics and Remote Access component.

  Help: This tag controls the interface used by the
        netX Diagnostics and Remote Access component ("marshaller").

        Configure which transport protocol is used via the tags
        `HIL_TAG_DIAG_TRANSPORT_CTRL_CIFX` and `HIL_TAG_DIAG_TRANSPORT_CTRL_PACKET`.
**************************************************************************************/
typedef struct
{
  /** Enable TCP Diagnostics Interface
  * - 0: FALSE, Do not use this interface
  * - 1: TRUE, Activate this interface */
  uint8_t bEnableFlag;

  /** TCP port number
   * Typically set with tag `HIL_TRANSPORT_IP_PORT` (Default: 50111, see HilTransport.h) */
  uint16_t usPortNumber;

  /** Reserved for future use */
  uint8_t bReserved;
} HIL_TAG_DIAG_IF_CTRL_TCP_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                tHeader;
  HIL_TAG_DIAG_IF_CTRL_TCP_DATA_T tData;
} HIL_TAG_DIAG_IF_CTRL_TCP_T;



/**************************************************************************************
  Tag:  HIL_TAG_DIAG_TRANSPORT_CTRL_CIFX
  Name: Remote Access via cifX API
  Desc: cifX transport interface of netX Diagnostics and Remote Access component.

  Help: This tag controls which transport protocol is used by the
        netX Diagnostics and Remote Access component ("marshaller").

        Configure the used interface(s) with the tags `HIL_TAG_DIAG_IF_CTRL_UART`,
        `HIL_TAG_DIAG_IF_CTRL_USB` and `HIL_TAG_DIAG_IF_CTRL_TCP`.
**************************************************************************************/
typedef struct
{
  /** Enable cifX API protocol
   * - 0: FALSE, do not use this transport type
   * - 1: TRUE, activate support for this transport type */
  uint8_t bEnableFlag;

  /** Reserved for future use */
  uint8_t abReserved[3];
} HIL_TAG_DIAG_TRANSPORT_CTRL_CIFX_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                        tHeader;
  HIL_TAG_DIAG_TRANSPORT_CTRL_CIFX_DATA_T tData;
} HIL_TAG_DIAG_TRANSPORT_CTRL_CIFX_T;



/**************************************************************************************
  Tag:  HIL_TAG_DIAG_TRANSPORT_CTRL_PACKET
  Name: Remote Access via rcX Packets
  Desc: Packet transport interface of netX Diagnostics and Remote Access component.

  Help: This tag controls which transport protocol is used by the
        netX Diagnostics and Remote Access component ("marshaller").

        Configure the used interface(s) with the tags `HIL_TAG_DIAG_IF_CTRL_UART`,
        `HIL_TAG_DIAG_IF_CTRL_USB` and `HIL_TAG_DIAG_IF_CTRL_TCP`.
**************************************************************************************/
typedef struct
{
  /** Enable rcX Packet protocol
   * - 0: FALSE, do not use this transport type
   * - 1: TRUE, activate support for this transport type */
  uint8_t bEnableFlag;

  /** Reserved for future use */
  uint8_t abReserved[3];
} HIL_TAG_DIAG_TRANSPORT_CTRL_PACKET_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                          tHeader;
  HIL_TAG_DIAG_TRANSPORT_CTRL_PACKET_DATA_T tData;
} HIL_TAG_DIAG_TRANSPORT_CTRL_PACKET_T;

/**************************************************************************************
  Tag:  HIL_TAG_HTTP_PORT_CONFIG
  Name: HTTP port configuration
  Desc: Configures the HTTP port of the build in web server component.

  Help: This tag sets the number of the TCP port that the web server listens on for HTTP.
        If the port number is set to 0, the web server will deactivate the HTTP port.

        The default value is 80. The valid port range is: 0 - 65535

        Note: Changing the port to a number that is already used by another component
        inside the firmware, will lead to undefined firmware behavior and must be avoided
        (check the firmware's data sheet for already used TCP ports).
**************************************************************************************/
typedef struct
{
  /** Webserver TCP Port Number for HTTP
   * - 0: Do not start http web server
   * - 80: Default
   * - 0..65535: Valid port range */
  uint16_t usPort;

  /** Reserved field for future use
   * Set to 0 to avoid unwanted behavior with upcoming version. */
  uint8_t  abReserved[2];

} HIL_TAG_HTTP_PORT_CONFIG_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                 tHeader;
  HIL_TAG_HTTP_PORT_CONFIG_DATA_T  tData;
} HIL_TAG_HTTP_PORT_CONFIG_T;



/**************************************************************************************
  Tag:  HIL_TAG_HTTPS_PORT_CONFIG
  Name: HTTPS port configuration
  Desc: Configures the HTTPS port of the build in web server component.

  Help: This tag sets the number of the TCP port that the web server listens on for HTTPS.
        If the port number is set to 0, the web server will deactivate the HTTPS port.

        The default value is 443. The valid port range is: 0 - 65535

        Note: Changing the port to a number that is already used by another component
        inside the firmware, will lead to undefined firmware behavior and must be avoided
        (check the firmware's data sheet for already used TCP ports).

        The Certificate Handling parameter defines the web server's behavior regarding
        the security configuration (private key, default certificate).
**************************************************************************************/
typedef struct
{
  /** Webserver TCP Port Number for HTTPS
   * - 0: Do not start HTTP web server
   * - 443: Default
   * - 0..65535: Valid port range */
  uint16_t usPort;

  /** Certification Handling
   * - 0: Certificate and private key must be present in CertDB, otherwise HTTPS won't start.
   * - 1: Self signed certificate and private key will be generated if any configuration issue in CertDB is detected.
   * - 2: Self signed certificate and private key will be generated if no configuration within CertDB is present.
   * - 3..255: Reserved for future use */
  uint8_t  bCertificateHandling;

  /** Reserved field for future use
   * Set to 0 to avoid unwanted behavior with upcoming version. */
  uint8_t  abReserved;

} HIL_TAG_HTTPS_PORT_CONFIG_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                  tHeader;
  HIL_TAG_HTTPS_PORT_CONFIG_DATA_T  tData;
} HIL_TAG_HTTPS_PORT_CONFIG_T;

/**************************************************************************************
  Tag:  HIL_TAG_WEBSERVER_ENABLE
  Name: Web server enable/disable
  Desc: Enables or disables the integrated web server.

  Help:
        ulEnable:

        This parameter enables or disables the web server.
        0:  disabled
             The web server is completely turned off and cannot be dynamically started (e.g. by the host application).
             Use case: the web server is not needed by the product. Disabling the web server saves memory that can be used for other purposes.

        !=0: enabled
             The firmware will initialize the web server with the provided configuration (below).

**************************************************************************************/
typedef struct
{
  /** Enable/Disable the integrated webserver. */
  uint32_t ulEnable;

} HIL_TAG_WEBSERVER_ENABLE_DATA_T;


typedef struct
{
  HIL_TAG_HEADER_T                 tHeader;
  HIL_TAG_WEBSERVER_ENABLE_DATA_T  tData;
} HIL_TAG_WEBSERVER_ENABLE_T;

/**************************************************************************************
  Tag:  HIL_TAG_DPM_BEHAVIOUR
  Name: DPM Behaviour
  Desc: AP Task DPM Behaviour settings

  Help: This tag allows the user to modify the DPM behaviour.
        If the setting is enabled (value of 1) the field `ulCommunicationState` will be
        handled in "legacy mode" for more information please refer to the Protocol API manual.
**************************************************************************************/
typedef struct
{
  /** COM State Legacy Mode
    * - 0: Disable legacy mode
    * - 1: Enable legacy mode */
  uint8_t      bComStateLegacyMode;

  /** Reserved field for future use
   * Set to 0 to avoid unwanted behavior with upcoming version. */
  uint8_t      abReserved[3];
} HIL_TAG_DPM_BEHAVIOUR_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T             tHeader;
  HIL_TAG_DPM_BEHAVIOUR_DATA_T tData;
} HIL_TAG_DPM_BEHAVIOUR_T;



/**************************************************************************************
  Tag:  HIL_TAG_REMANENT_DATA_RESPONSIBLE
  Name: Remanent Data Responsibility
  Desc: With this tag, you can adjust whether loading and storing of remanent data is entirely
        performed by the communication firmware or by the host application.

  Help: When remanent data handling is configured to be done in the host domain, the host
        application has to provide remanent data for each relevant component of the firmware
        via packet `HIL_SET_REMANENT_DATA_REQ` during initialization.
        Subsequently, it must handle the packets `HIL_STORE_REMANENT_DATA_IND` with which modified
        remanent data for each component is submitted in order for the host to store it onto a
        non-volatile storage device during runtime.

        Depending on the specific stack implementation, there still may be the need for the
        host to store certain non-volatile data even if the stack is responsible for remanent
        data storage. Please refer to the manual of your particular Firmware for further
        information.
**************************************************************************************/
typedef struct
{
  /** Remanent data stored by Host.
   * - 0: Communication firmware stores remanent data (default)
   * - 1: Host application stores remanent data */
  uint8_t      bHandledByHost;

  /** Reserved field for future use
   * Set to 0 to avoid unwanted behavior with upcoming version. */
  uint8_t      abReserved[3];

} HIL_TAG_REMANENT_DATA_RESPONSIBLE_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                          tHeader;
  HIL_TAG_REMANENT_DATA_RESPONSIBLE_DATA_T  tData;
} HIL_TAG_REMANENT_DATA_RESPONSIBLE_T;



/**************************************************************************************
  Tag:  HIL_TAG_LWIP_NETIDENT_BEHAVIOUR
  Name: LWIP netident behavior
  Desc: Configures the behavior of the netident protocol.

  Help: With this tag, you can adjust whether the firmware shall activate the
        Hilscher netident protocol, which is build-in in the IP stack.
**************************************************************************************/
typedef struct
{
  /** Enable netident
   * - 0: netident is disabled
   * - 1: netident is enabled */
  uint8_t      bNetidentEnable;

  /** Reserved field for future use
   * Set to 0 to avoid unwanted behavior with upcoming version. */
    uint8_t      abReserved[3];

} HIL_TAG_LWIP_NETIDENT_BEHAVIOUR_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                        tHeader;
  HIL_TAG_LWIP_NETIDENT_BEHAVIOUR_DATA_T  tData;
} HIL_TAG_LWIP_NETIDENT_BEHAVIOUR_T;



/**************************************************************************************
  Tag:  HIL_TAG_LWIP_QUANTITY_STRUCTURE
  Name: Socket API Quantity Structure
  Desc: Adjust the resources allocated and provided by the built-in IP stack

  Help: With this tag, you can adjust the resources allocated and provided by the
        built-in IP stack integrated in the firmware.

        The number of Socket API services at DPM via Mailbox can be configured.
        The number of sockets for Socket API usage inside the IP stack can be configured.
**************************************************************************************/

#define HIL_TAG_LWIP_MIN_NUM_SERVICE      1
#define HIL_TAG_LWIP_MAX_NUM_SERVICE      8  /** Highest allowed value. Individual firmware may define its own, lower maximum value */
#define HIL_TAG_LWIP_DEFAULT_NUM_SERVICE  4
#define HIL_TAG_LWIP_MIN_NUM_SOCKET       1
#define HIL_TAG_LWIP_MAX_NUM_SOCKET       64 /** Highest allowed value. Individual firmware may define its own, lower maximum value */
#define HIL_TAG_LWIP_DEFAULT_NUM_SOCKET   8

typedef struct
{
  /** Number of Socket API services at DPM level
   * - 1..8: Number of services
   * An individual firmware might define the maximum lower. Default: 4. */
  uint8_t      bNumberDpmSocketServices;

  /** Number of Sockets for Socket API usage
  * - 1..64: Number of sockets
  * An individual firmware might define the maximum lower. Default: 8. */
  uint8_t      bNumberSockets;

  /** Reserved field for future use
   * Set to 0 to avoid unwanted behavior with upcoming version. */
  uint8_t      abReserved[2];

} HIL_TAG_LWIP_QUANTITY_STRUCTURE_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                        tHeader;
  HIL_TAG_LWIP_QUANTITY_STRUCTURE_DATA_T  tData;
} HIL_TAG_LWIP_QUANTITY_STRUCTURE_T;



/**************************************************************************************
  Tag:  HIL_TAG_LWIP_AMOUNT_SOCKET_API_MULTICAST_GROUPS
  Name: Socket API IP Multicast Group configuration
  Desc: Sets the amount of IP Multicast groups to be used by Socket API sockets

  Help: With this tag, you can adjust the IP Multicast communication resources provided
        by the built-in IP stack integrated in the firmware.

        The number of IP Multicast groups used by Socket API communication can be configured.
**************************************************************************************/

#define HIL_TAG_LWIP_MIN_SOCKET_API_MULTICAST_GROUPS     0     /** Lowest allowed value. */
#define HIL_TAG_LWIP_MAX_SOCKET_API_MULTICAST_GROUPS     65535 /** Highest allowed value. Individual firmware may define its own, lower maximum value */
#define HIL_TAG_LWIP_DEFAULT_SOCKET_API_MULTICAST_GROUPS 0     /** default value (IP Multicast not used by Socket API). */

typedef struct
{
  /** Number of IP Multicast groups intended for Socket API usage
   * - 0..65535: Number of IP Multicast groups
   * An individual firmware might define the maximum lower. Default: 0. */
  uint32_t      ulNumberIpMulticastsForSocketServices;
} HIL_TAG_LWIP_AMOUNT_SOCKET_API_MULTICAST_GROUPS_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                                        tHeader;
  HIL_TAG_LWIP_AMOUNT_SOCKET_API_MULTICAST_GROUPS_DATA_T  tData;
} HIL_TAG_LWIP_AMOUNT_SOCKET_API_MULTICAST_GROUPS_T;


/**************************************************************************************
  Tag:  HIL_TAG_DEVICENET_DEVICEID
  Name: DeviceNet Product Information
  Desc: Configures CIP Product Information to be used by firmware.

  Help: With this tag you set various parameters to define a CIP device
        (EtherNet/IP, DeviceNet, CompoNet).

        Except for 'Product Name', all of these parameters are used by a
        connection originator to identify the device with certainty.
        The parameters shall match the values defined in the corresponding EDS file.
**************************************************************************************/
typedef struct
{
  /** Vendor ID
   *  Defines the vendor of the device. Vendor IDs are managed by ODVA.
   *  Note: A value of zero is not valid. */
  uint16_t usVendorID;

  /** Device Type
   * The device type is used to identify the device profile that the product is using.
   * Device profiles define minimum requirements a device must implement as well as common options.
   * The list of device types is managed by ODVA. */
  uint16_t usDeviceType;

  /** Product Code
   * The product code identifies a product within a device type. It is assigned by the vendor.
   * Typically, the product code maps to one or more catalog/model numbers.
   * Products shall have different codes if their configuration and/or runtime options are different.
   * Such devices present a different logical view to the network.
   * On the other hand for example, two products that are the same except for their color
   * or mounting feet are the same logically and may share the same product code.
   *
   * Note: A value of zero is not valid. */
  uint16_t usProductCode;

  /** Major Revision
   * Identifies the hardware revision of the item the Identity Object is representing.
   * Note: A value of zero is not valid. */
  uint8_t  bMajRev;

  /** Minor Revision
   * Identifies the hardware revision of the item the Identity Object is representing.
   * Note: A value of zero is not valid. */
  uint8_t  bMinRev;

  /** Product Name
   * This text string should contain a short description of the product/product family represented
   * by the product code. The same product code may have a variety of product name strings.
   * Note: The maximum number of characters in this string is 32. */
  uint8_t  abProductName[32];
} HIL_TAG_DEVICENET_DEVICEID_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T            tHeader;
  HIL_TAG_DEVICENET_DEVICEID_DATA_T tData;
} HIL_TAG_DEVICENET_DEVICEID_T;

/**************************************************************************************
  Tag:  HIL_TAG_DEVICENET_CAN_SAMPLING
  Name: CAN Sample point
  Desc: Configures the CAN sample point

  Help: The sample point location is defined as <= 80% of the bit timing.
        With this option it is possible to shift the sample point between the recommended
        default setting to an alternative setting.
        This may solve timing issue when devices are used, which run out of specification
        regarding to the clock timings.

        Note: Using this setting may make the product non-compliant to the specification!

        |Baud rate   |default timing  |alternative timing |
        |------------|----------------|-------------------|
        |250   kBaud |      90,0%     |      80,0%        |
        |125   kBaud |      90,0%     |      80,0%        |
        |100   kBaud |      90,0%     |      80,0%        |
        | 50   kBaud |      90,0%     |      80,0%        |
        | 20   kBaud |      90,0%     |      80,0%        |
        | 12,5 kBaud |      90,0%     |      80,0%        |
        | 10   kBaud |      90,0%     |      80,0%        |
**************************************************************************************/
typedef struct
{
  /** Sample point timing settings.
   * - 0: Default timing
   * - 1: Alternative timing */
  uint8_t      bEnableAlternativeSamplePointTimings;

  /** Reserved field for future use.
   * Set to 0 to avoid unwanted behavior with upcoming version. */
  uint8_t      abReserved[3];

} HIL_TAG_DEVICENET_CAN_SAMPLING_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                       tHeader;
  HIL_TAG_DEVICENET_CAN_SAMPLING_DATA_T  tData;
} HIL_TAG_DEVICENET_CAN_SAMPLING_T;



/**************************************************************************************
  Tag:  HIL_TAG_CO_DEVICEID
  Name: CANopen Product Information
  Desc: Defines a CANopen Slave device

  Help: This tag defines product information for a CANopen slave device.
**************************************************************************************/
typedef struct
{
  /** Vendor ID
   * The Vendor ID is a unique number identifying a registered company or a department of a registered company.
   * The allocation of the Vendor ID is handled by CiA Headquarters. */
  uint32_t ulVendorId;

  /** Product Code
   * The Product code identifies a specific device version. It is assigned by the manufacturer. */
  uint32_t ulProductCode;

  /** Major Revision
   * The manufacturer-specific major revision number identifies a specific CANopen behavior.
   * If the CANopen functionality is expanded, the major revision number has to be increased. */
  uint16_t usMajRev;

  /** Minor Revision
   * The manufacturer-specific minor revision number identifies different versions of the same CANopen behavior. */
  uint16_t usMinRev;

  /** Device Profile Number
   * The Device Profile Number describes the CANopen profile that is used by the device.
   * Together with the "Additional Information", it forms the object Device Type (Object 1000h)
   * that describes the supported CANopen standard.
   *
   * Note: These two parameters are supported from CANopen stack version 3 and higher.
   * For earlier versions set Device Type to zero. */
  uint16_t usDeviceProfileNumber;

  /** Additional Information
   * The Additional Information field provides information about optional functionality of the device.
   * The Additional Information parameter is specified in the appropriate device profile. */
  uint16_t usAdditionalInfo;
} HIL_CO_DEVICEID_ID_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T          tHeader;
  HIL_CO_DEVICEID_ID_DATA_T tData;

} HIL_CO_DEVICEID_ID_T;



/**************************************************************************************
  Tag:  HIL_TAG_CCL_DEVICEID
  Name: CC-Link Product Information
  Desc: Describes a CC-Link Product

  Help: The following parameters define a CC-Link device.
**************************************************************************************/
typedef struct
{
  /** Vendor Code
   * Vendor identification number of the manufacturer. The Vendor Codes are managed by CLPA.*/
  uint32_t ulVendorId;

  /** Model Type
   * Defines the model type. The model type is assigned by the manufacturer.*/
  uint32_t ulModelType;

  /** Software Version
   * Defines the software version
   * - 0..63: Software version */
  uint32_t ulSwVersion;

} HIL_CCL_DEVICEID_ID_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T          tHeader;
  HIL_CCL_DEVICEID_ID_DATA_T tData;

} HIL_CCL_DEVICEID_ID_T;



/**************************************************************************************
  Tag:  HIL_TAG_DDP_MODE_AFTER_STARTUP
  Name: Device Data Provider Mode
  Desc: Controls the DDP mode on firmware startup

  Help: The Device Data Provider is integrated in the firmware operating system.
        The DDP offers two modes: "Active" or "Passive".

        This tag influences the ability of your application to use the DDP Set API to modify
        DDP parameters.

        When set to "Active", the firmware starts and sets the DDP mode to "Active" (default).
        The firmware will directly use and activate the device-specific data from the DDP
        (which are typically contained in FDL). The application cannot change device-specific
        data via DDP Set API.

        In "Passive" mode, the firmware starts and sets the DDP mode to "Passive".
        The usage of the DDP Set API is possible (for writable parameters).
        The firmware will treat the existing writable DDP parameters as invalid
        (with all corresponding consequences) until the application informs the firmware
        that DDP parameters are now valid.

        The application is required to use the DDP Set API to set DDP mode to "Active" later
        (once the parameters are valid).

        Consequences of "Passive" DDP mode (incomplete list):

        - Protocol specific configuration is rejected
        - NDIS is not working
**************************************************************************************/
typedef struct
{
  /** DDP mode after firmware startup.
   * - 0: Startup in mode "Passive". Using DDP Set API is possible
   * - 1: Startup in mode "Active".  Using DDP Set API is impossible */
  uint8_t      bDdpModeAfterStartup;

  /** Reserved field for future use
   * Set to 0 to avoid unwanted behavior with upcoming version. */
    uint8_t      abReserved[3];

} HIL_TAG_DDP_INITIAL_STATE_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                  tHeader;
  HIL_TAG_DDP_INITIAL_STATE_DATA_T  tData;
} HIL_TAG_DDP_INITIAL_STATE_T;



/**************************************************************************************
  Tag:  HIL_TAG_PHY_ENABLE_TIMEOUT
  Name: PHY enable timeout after firmware startup
  Desc: Specify a maximum delay until PHYs will be activated by firmware.

  Help: With this tag, you can set the timeout to enable the PHYs after firmware startup.
        By default, the timeout is set to 0 seconds and PHY will not be enabled automatically.
        The main protocol stack in the firmware will wait for a valid PHY configuration.
        The PHYs will be enabled as soon as a configuration is provided.

        When a timeout between 1 and 300 seconds is set, the firmware waits for PHY configuration.

        If no PHY configuration is provided before the set timeout, all PHYs will be enabled with default settings.
        The default settings are "Autonegotiation" and "Autocrossover".

        If the main protocol stack gets its valid PHY configuration prior to hitting the timeout,
        the PHYs will be enabled and configured with the parameters from the configuration.

        If the main protocol stack gets its valid PHY configuration after the timeout hits,
        the protocol stack will reparameterize the PHYs according to its configuration.

        Note: This may lead to short link loss in case the configuration deviates from the defaults.

        *Caution:* There is a dependency on other tag "DDP Mode after firmware startup":
                   If the tag entry "DDP Mode after firmware startup" is configured to PASSIVE,
                   the host application must set the DDP mode to ACTIVE before the PHY Enable Timeout (this tag) elapses.
                   The value of PHY Enable Timeout must be sufficiently large to ensure this requirement can be met.
**************************************************************************************/

/* PHY timeout value definitions */
#define HIL_TAG_PHY_ENABLE_TIMEOUT_DISABLED     0   /** Never enable PHY without configuration */
#define HIL_TAG_PHY_ENABLE_TIMEOUT_MIN          1   /** Wait for 1 second, minimum allowed value */
#define HIL_TAG_PHY_ENABLE_TIMEOUT_MAX          300 /** Wait for 300 seconds, maximum allowed value */
#define HIL_TAG_PHY_ENABLE_TIMEOUT_DEFAULT      0   /** Do not enable PHY without configuration, default value */
typedef struct
{
  /** PHY enable timeout
   * - 0: Wait for protocol stack configuration.
   * - 1..300: PHY shall be enabled 1-300 seconds after firmware startup. */
  uint32_t      ulPhyEnableTimeoutAfterStartup;

} HIL_TAG_PHY_ENABLE_TIMEOUT_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                   tHeader;
  HIL_TAG_PHY_ENABLE_TIMEOUT_DATA_T  tData;
} HIL_TAG_PHY_ENABLE_TIMEOUT_T;



/**************************************************************************************
  Tag:  HIL_TAG_EIF_EDD_INSTANCE
  Name: Ethernet Channel Number
  Desc: Configure DPM channel for RTE Ethernet Interface

  Help: This tag configures the DPM channel to which the Ethernet Interface should connect
        when used with run-time linking to an RTE communication stack.
**************************************************************************************/
typedef struct
{
  /** RTE channel number
   * - 0..3: Communication channel number of the linked RTE stack */
  uint32_t ulEddInstanceNo;
} HIL_TAG_EIF_EDD_INSTANCE_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                  tHeader;
  HIL_TAG_EIF_EDD_INSTANCE_DATA_T   tData;
} HIL_TAG_EIF_EDD_INSTANCE_T;



/**************************************************************************************
  Tag:  HIL_TAG_EIF_EDD_CONFIG
  Name: Ethernet Interface configuration
  Desc: Configures the ethernet interface component

  Help: This tag configures the Ethernet Interface component when used without run-time
        linking to an RTE communication stack.
**************************************************************************************/

/* EDD type definitions for Ethernet Interface configuration tag */
#define RX_EIF_EDD_TYPE_VIRTUAL                 0   /** Virtual EDD attached to TCP stack */
#define RX_EIF_EDD_TYPE_STD_MAC                 1   /** Single-port standard Ethernet interface */
#define RX_EIF_EDD_TYPE_2PORT_SWITCH            2   /** 2-port Switch */
#define RX_EIF_EDD_TYPE_2PORT_HUB               3   /** 2-port Hub */

typedef struct
{
  /** EDD Type
   * - 0: Virtual EDD attached to TCP stack
   * - 1: Single-port standard Ethernet interface
   * - 2: 2-port Switch
   * - 3: 2-port Hub */
  uint32_t ulEddType;

  /** xC
   * Number of the first (or the only) xC used (0-3) */
  uint32_t ulFirstXcNumber;
} HIL_TAG_EIF_EDD_CONFIG_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                  tHeader;
  HIL_TAG_EIF_EDD_CONFIG_DATA_T     tData;
} HIL_TAG_EIF_EDD_CONFIG_T;



/**************************************************************************************
  Tag:  HIL_TAG_EIF_NDIS_ENABLE
  Name: Ethernet NDIS Support
  Desc: Configures NDIS support for the ethernet interface.

  Help: With this tag you can enable or disable NDIS support for the ethernet interface.
**************************************************************************************/
typedef struct
{
  /** Enable NDIS Support
   * - 0: NDIS support is disabled (default)
   * - 1: NDIS support is enabled */
  uint32_t      ulNDISEnable;
} HIL_TAG_EIF_NDIS_ENABLE_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                tHeader;
  HIL_TAG_EIF_NDIS_ENABLE_DATA_T  tData;
} HIL_TAG_EIF_NDIS_ENABLE_T;



/**************************************************************************************
  Tag:  HIL_TAG_TCP_PORT_NUMBERS
  Name: Ethernet Interface TCP Port Numbers
  Desc: Configures the IP port ranges handled by the Ethernet Interface.

  Help: This tag is used to specify IP port ranges to be handled by Ethernet Interface.

        Usually, unless the Ethernet Interface module is used, the TCP/IP stack handles
        all incoming TCP/UDP frames by itself. If the Ethernet Interface module is used,
        the destination port number of a frame determines whether it is handled by the TCP/IP stack
        or forwarded to the Ethernet Interface module. This tag defines the ports
        which are to be handled by the TCP/IP stack.
        It specifies a range of ports and a list of additional port numbers.

        Note: The range of available TCP ports must not lie below 1024.
**************************************************************************************/
#define HIL_TAG_ETHINTF_TCPUDP_PORT_NUMBERS_RANGE_START_DEFAULT     1024 /** Default value for start port (ulPortStart) */
#define HIL_TAG_ETHINTF_TCPUDP_PORT_NUMBERS_RANGE_END_DEFAULT       2048 /** Default value for end port   (ulPortEnd)   */

typedef struct
{
  /** TCP/UDP Port Range (First Port)
   * The first port of the TCP/UDP-Port range.
   * The minimum value is: 1024 */
  uint32_t    ulPortStart;

  /** TCP/UDP Port Range (Last Port)
   * The last port of the TCP/UDP-Port range.
   * The range must contain at least 1024 ports, i.e. `last port - first port >= 1023`.
   * The minimum value is: 2048 */
  uint32_t    ulPortEnd;

  /** Number of Protocol Stack Ports
   * This is the number of entries from the beginning of the port list which are used by the the protocol stack.
   * These entries are required by the protocol stack. They are shown for information only and must not be changed. */
  uint32_t    ulNumberOfProtocolStackPorts;

  /** Number of Additional Ports
   * This is the number of valid entries which have been added by the user. */
  uint32_t    ulNumberOfUserPorts;

  /** Port List (20 Ports)
   * A list of port numbers, interpreted according to "Number of Protocol Stack Ports" and "Number of Additional Ports".
   *
   *
   * *Example:* "Number of Protocol Stack Ports" is 5 and "Number of Additional Ports" is 2.
   *
   *
   * - Entries 1 - 5 are predefined by the protocol and must not be changed.
   *
   * - Entries 6 and 7 have been added by the user.
   *
   * - Entries 8 - 20 are ignored. */
  uint16_t    ausPortList[20];
} HIL_TAG_TCP_PORT_NUMBERS_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                tHeader;
  HIL_TAG_TCP_PORT_NUMBERS_DATA_T tData;
} HIL_TAG_ETHINTF_TCPUDP_PORT_NUMBERS_T;



/**************************************************************************************
  Tag:  HIL_TAG_NF_GEN_DIAG_RESOURCES
  Name: Generic Diagnosis Resources
  Desc: Configures generic diagnosis resources.

  Help: With this tag you can configure generic diagnosis resources.

        This tag controls the number of generic diagnosis instances provided to the
        OEM application. You can select a value between 32 and 256 ins steps of 8.
**************************************************************************************/
typedef struct
{
  /** Number of additional netPROXY Generic Diagnosis instances
   * - 32..256: Number of instances (in steps of 8)
  */
  uint16_t usNumOfDiagnosisInstances;

  /** Reserved, set to zero */
  uint8_t abReserved[2];

} HIL_TAG_NF_GEN_DIAG_RESOURCES_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                     tHeader;
  HIL_TAG_NF_GEN_DIAG_RESOURCES_DATA_T tData;
} HIL_TAG_NF_GEN_DIAG_RESOURCES_T;



/**************************************************************************************
  Tag:  HIL_TAG_NF_PROFI_ENERGY_MODES
  Name: PROFIenergy Support
  Desc: Enable and configure PROFIenergy feature.

  Help: With this tag you can enable the PROFIenergy feature and set the number of modes
        (1 through 8).
**************************************************************************************/
typedef struct
{
  /** Enable PROFIenergy support
   * - 0: PROFIenergy is disabled
   * - 1..8: PROFIenergy is supported with 1..8 modes */
  uint8_t bPROFIenergyMode;

  /** Reserved, set to zero */
  uint8_t abReserved[3];

} HIL_TAG_NF_PROFI_ENERGY_MODES_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                     tHeader;
  HIL_TAG_NF_PROFI_ENERGY_MODES_DATA_T tData;
} HIL_TAG_NF_PROFI_ENERGY_MODES_T;



/**************************************************************************************
  Tag:  HIL_TAG_NF_PN_IOL_PROFILE_PADDING
  Name: PROFINET IO-Link Profile Submodule Padding
  Desc: Configure the padding used for IO-Link Master profile submodules

  Help: With this tag you can configure the padding used for IO-Link Master profile submodules
**************************************************************************************/
#define HIL_TAG_NF_PN_IOL_PROFILE_PADDING_PADMODE_UNALIGNMENT     0 /** No padding (default) */
#define HIL_TAG_NF_PN_IOL_PROFILE_PADDING_PADMODE_2BYTE_ALIGNMENT 1 /** Padding for 2 bytes alignment */
#define HIL_TAG_NF_PN_IOL_PROFILE_PADDING_PADMODE_4BYTE_ALIGNMENT 2 /** Padding for 4 bytes alignment */

typedef struct
{
  /** Padding mode
   * - 0: No padding (default)
   * - 2: Padding for 2 bytes alignment
   * - 4: Padding for 4 bytes alignment */
  uint8_t bProfilePaddingMode;

  /** Reserved, set to zero */
  uint8_t abReserved[3];

} HIL_TAG_NF_PN_IOL_PROFILE_PADDING_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                        tHeader;
  HIL_TAG_NF_PN_IOL_PROFILE_PADDING_DATA_T tData;
} HIL_TAG_NF_PN_IOL_PROFILE_PADDING_T;



/**************************************************************************************
  Tag:  HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM
  Name: PROFINET IO-Link Profile Pin4 DIO in IOLM Submodule
  Desc: Specify where Pin4 DIO data is located.

  Help: This tag specifies where Pin4 DIO data is located.
        It allows you to activate handling of Pin4 DIO data in IOLM Submodule.
        Profile DI/DO Submodules do not have any IO data in this case.
**************************************************************************************/
#define HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM_DISABLED 0 /** Pin4 DIO data in regular profile DI/DO submodules (default) */
#define HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM_ENABLED  1 /** Pin4 DIO data in IOLM Submodule */

typedef struct
{
  /** Location of Pin4 DIO Data
   * - 0: Pin4 DIO data in regular profile DI/DO submodules (default)
   * - 1: Pin4 DIO data in IOLM Submodule */
  uint8_t bDioInIolm;

  /** Reserved, set to zero */
  uint8_t abReserved[3];
} HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                             tHeader;
  HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM_DATA_T tData;
} HIL_TAG_NF_PN_IOL_PROFILE_DIO_IN_IOLM_T;



/**************************************************************************************
  Tag:  HIL_TAG_NF_SWAP_COM_LEDS
  Name: Swap COM LEDs
  Desc: Swap the COM0 and COM1 indicator LEDs.

  Help: This tag allows to swap the COM0 and COM1 indicator LEDs with each other.

        By default, the LEDs are not swapped.
**************************************************************************************/
typedef struct
{
  /** Swap COM LEDs
   * - 1: TRUE, Swap the COM0 and COM1 LEDs
   * - 0: FALSE, Don't swap (default) */
  uint8_t bSwapComLeds;

  /** Reserved, set to zero */
  uint8_t abReserved[3];

} HIL_TAG_NF_SWAP_COM_LEDS_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                tHeader;
  HIL_TAG_NF_SWAP_COM_LEDS_DATA_T tData;
} HIL_TAG_NF_SWAP_COM_LEDS_T;



/**************************************************************************************
  Tag:  HIL_TAG_NF_PN_IOL_PROFILE_CFG_FLAGS
  Name: netFIELD PROFINET IO-Link profile Configuration Flags.
  Desc: Configure specia features in the PNSIOLM package

  Help: This tag allows configuring the features of the PNSIOLM package.
**************************************************************************************/
#define HIL_TAG_NF_PN_IOL_PROFILE_CFG_FLAGS_DIO_IN_IOLM_DISABLED   0
#define HIL_TAG_NF_PN_IOL_PROFILE_CFG_FLAGS_DIO_IN_IOLM_ENABLED    1
#define HIL_TAG_NF_PN_IOL_PROFILE_CFG_FLAGS_DIAG_ENABLED           0
#define HIL_TAG_NF_PN_IOL_PROFILE_CFG_FLAGS_DIAG_DISABLED          1
#define HIL_TAG_NF_PN_IOL_PROFILE_CFG_FLAGS_PA_ENABLED             0
#define HIL_TAG_NF_PN_IOL_PROFILE_CFG_FLAGS_PA_DISABLED            1

typedef struct
{
  /** PROFINET IO-Link profile submodule DIO in IOLM.
   * Allowed values: All HIL_TAG_NF_PN_IOL_PROFILE_CFG_FLAGS_DIO_IN_IOLM_* defines */
  uint8_t bDioInIolm;

  /** PROFINET IO-Link profile enable/disable general diagnosis mapping.
   * Allowed values: All HIL_TAG_NF_PN_IOL_PROFILE_CFG_FLAGS_DIAG_* defines */
  uint8_t bDisableDiag;

  /** PROFINET IO-Link profile enable/disable general process alarms mapping.
   * Allowed values: All HIL_TAG_NF_PN_IOL_PROFILE_CFG_FLAGS_PA_* defines */
  uint8_t bDisablePA;

  /** Reserved, set to zero */
  uint8_t bReserved;

} HIL_TAG_NF_PN_IOL_PROFILE_CFG_FLAGS_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                            tHeader;
  HIL_TAG_NF_PN_IOL_PROFILE_CFG_FLAGS_DATA_T  tData;
} HIL_TAG_NF_PN_IOL_PROFILE_CFG_FLAGS_T;



/**************************************************************************************
  Tag:  HIL_TAG_PNS_ETHERNET_PARAMS
  Name: netX Ethernet and Fiber Optic Interface
  Desc: Configures which port of the hardware uses fiber optic physics.

  Help: This tag defines the the active Ethernet hardware port(s) of netX and whether the
        fiber optic interface is enabled.
**************************************************************************************/
#define HIL_TAG_PNS_ETHERNET_FIBEROPTICMODE_OFF            0
#define HIL_TAG_PNS_ETHERNET_FIBEROPTICMODE_ON             1
#define HIL_TAG_PNS_ETHERNET_FIBEROPTICMODE_PORT0_ONLY_ON  2
#define HIL_TAG_PNS_ETHERNET_FIBEROPTICMODE_PORT1_ONLY_ON  3

typedef struct
{
  /** Selection of Active Ports
   * - 1: Port 1 only
   * - 3: Both Port 0 and Port 1 */
  uint8_t       bActivePortsBf;

  /** Enable Fiber Optic Interface support
   * - 0: Off
   * - 1: Both 0 and Port 1
   * - 2: Port 0 only
   * - 3: Port 1 only */
  uint8_t       bFiberOpticMode; /** see defines above */

  /** Reserved, set to zero */
  uint8_t       abReserved[2];
} HIL_TAG_PNS_ETHERNET_PARAMS_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                    tHeader;
  HIL_TAG_PNS_ETHERNET_PARAMS_DATA_T  tData;
} HIL_TAG_PNS_ETHERNET_PARAMS_T;



/**************************************************************************************
  Tag:  HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX50_PARAMS
  Name: Fiber Optic DMI (netX50)
  Desc: Configures the Fiber Optic Transceiver DMI settings.

  Help: This tag defines the assignment of the I2C lines to MMIO pins.
**************************************************************************************/
typedef struct
{
  /** I2C SDA1 Pin Number
     * MMIO pin number for I2C SDA line to access DMI of the _first_ Fiber Optic Transceiver */
  uint8_t      bSDA1PinIdx;

  /** I2C SDA2 Pin Number
   * MMIO pin number for I2C SDA line to access DMI of the _second_ Fiber Optic Transceiver */
  uint8_t      bSDA2PinIdx;

  /** I2C SCL Pin Number
   * MMIO pin number for I2C SCL line to access DMI of the Fiber Optic Transceiver */
  uint8_t      bSCLPinIdx;

  /** Reserved, set to zero */
  uint8_t      bReserved;
} HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX50_PARAMS_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                                     tHeader;
  HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX50_PARAMS_DATA_T  tData;
} HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX50_PARAMS_T;



/**************************************************************************************
  Tag:  HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX100_PARAMS
  Name: Fiber Optic DMI (netX100/500)
  Desc: Configures the Fiber Optic Transceiver DMI settings.

  Help: This tag defines the I2C Transceiver Select Pin used to switch between DMI Interfaces of
        both transceivers.
**************************************************************************************/
#define HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_PINTYPE_NONE   0
#define HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_PINTYPE_GPIO   1
#define HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_PINTYPE_PIO    2

typedef struct
{
  /** I2C Transceiver Select Pin Type
   * - 0: None
   * - 1: GPIO
   * - 2: PIO */
  uint8_t      bSelectPinType;

  /** Invert Pin
   * - 0: Not inverted
   * - 1: Inverted */
  uint8_t      bSelectPinInvert;

  /** Pin number for tranceiver select pin
   * - 0..15: GPIO
   * - 0..84: PIO  */
  uint8_t      bSelectPinIdx;

  /** Reserved, set to zero */
  uint8_t      bReserved;
} HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX100_PARAMS_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                                      tHeader;
  HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX100_PARAMS_DATA_T  tData;
} HIL_TAG_PNS_FIBER_OPTIC_IF_DMI_NETX100_PARAMS_T;



/**************************************************************************************
  Tag:  HIL_TAG_ECM_ENI_BUS_STATE
  Name: EtherCAT Master bus state for ENI
  Desc: Configure the target Bus state of the  firmware when configured with ENI file.

  Help: This tag configures EtherCAT Master target bus state for ENI files on ChannelInit.
**************************************************************************************/
#define HIL_TAG_ECM_ENI_BUS_STATE_OFF  0 /** BusState off after ChannelInit (Application controlled startup) */
#define HIL_TAG_ECM_ENI_BUS_STATE_ON   1 /** BusState on after ChannelInit (Automatic startup) */

typedef struct
{
  /** Target bus state for ENI files on ChannelInit
   * - 0: Off (Application controlled startup)
   * - 1: On (Automatic startup)*/
  uint32_t ulTargetBusState;
} HIL_TAG_ECM_ENI_BUS_STATE_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                 tHeader;
  HIL_TAG_ECM_ENI_BUS_STATE_DATA_T tData;
} HIL_TAG_ECM_ENI_BUS_STATE_T;



/**************************************************************************************
  Tag:  HIL_TAG_ECS_EOE_MODE
  Name: EtherCAT Slave EoE mode
  Desc: Select destination of EOE frames

  Help: With this tag you can configure where the EoE interface (first address,
        assigned by master) is bound to.

**************************************************************************************/
#define HIL_TAG_ECS_EOE_MODE_INTERNAL  (0) /*!< EoE address is assigned to 1st chassis (LWIP) */
#define HIL_TAG_ECS_EOE_MODE_EXTERNAL  (1) /*!< EoE address is assigned to NDIS interface     */

typedef struct
{
  /** Enable DLR
   * - 0: EoE address is assigned to 1st chassis (LWIP)
   * - 1: EoE address is assigned to NDIS interface     */
  uint32_t ulEoEMode;
} HIL_TAG_ECS_EOE_MODE_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T            tHeader;
  HIL_TAG_ECS_EOE_MODE_DATA_T tData;
} HIL_TAG_ECS_EOE_MODE_T;



/**************************************************************************************
  Tag:  HIL_TAG_EIP_DEVICEID
  Name: EtherNet/IP Product Information
  Desc: Configures CIP Product Information to be used by firmware.

  Help: With this tag you set various parameters to define a CIP device
        (EtherNet/IP, DeviceNet, CompoNet).

        Except for 'Product Name', all of these parameters are used by a
        connection originator to identify the device with certainty.
        The parameters shall match the values defined in the corresponding EDS file.
**************************************************************************************/
typedef struct
{
  /** Vendor ID
   *  Defines the vendor of the device. Vendor IDs are managed by ODVA.
   *  Note: A value of zero is not valid. */
  uint16_t usVendorID;

  /** Device Type
   * The device type is used to identify the device profile that the product is using.
   * Device profiles define minimum requirements a device must implement as well as common options.
   * The list of device types is managed by ODVA. */
  uint16_t usDeviceType;

  /** Product Code
   * The product code identifies a product within a device type. It is assigned by the vendor.
   * Typically, the product code maps to one or more catalog/model numbers.
   * Products shall have different codes if their configuration and/or runtime options are different.
   * Such devices present a different logical view to the network.
   * On the other hand for example, two products that are the same except for their color
   * or mounting feet are the same logically and may share the same product code.
   *
   * Note: A value of zero is not valid. */
  uint16_t usProductCode;

  /** Major Revision
   * Identifies the hardware revision of the item the Identity Object is representing.
   * Note: A value of zero is not valid. */
  uint8_t  bMajRev;

  /** Minor Revision
   * Identifies the hardware revision of the item the Identity Object is representing.
   * Note: A value of zero is not valid. */
  uint8_t  bMinRev;

  /** Product Name
   * This text string should contain a short description of the product/product family represented
   * by the product code. The same product code may have a variety of product name strings.
   * Note: The maximum number of characters in this string is 32. */
  uint8_t  abProductName[32];
} HIL_TAG_EIP_DEVICEID_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T            tHeader;
  HIL_TAG_EIP_DEVICEID_DATA_T tData;
} HIL_TAG_EIP_DEVICEID_T;



/**************************************************************************************
  Tag:  HIL_TAG_EIP_DLR_PROTOCOL
  Name: DLR Protocol
  Desc: Configures use of the EtherNet/IP DLR protocol.

  Help: This tag enables or disables the DLR (Device Level Ring) protocol.
**************************************************************************************/
#define HIL_TAG_EIP_DLR_PROTOCOL_OFF      0   /** DLR functionality is turned OFF */
#define HIL_TAG_EIP_DLR_PROTOCOL_ON       1   /** DLR functionality is turned ON */

typedef struct
{
  /** Enable DLR
   * - 0: DLR functionality is turned Off
   * - 1: DLR functionality is turned On */
  uint32_t ulEnableDLR;

} HIL_TAG_EIP_DLR_PROTOCOL_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                tHeader;
  HIL_TAG_EIP_DLR_PROTOCOL_DATA_T tData;
} HIL_TAG_EIP_DLR_PROTOCOL_T;

/**************************************************************************************
  Tag:  HIL_TAG_EIP_RESOURCES
  Name: EtherNet/IP resources configuration
  Desc: This taglist entry allows for static tailoring of the EtherNet/IP stack’s resource limits.

  Help: See Tag list editor

**************************************************************************************/
typedef struct
{
  /** Maximum number of supported CIP services the host can register (not bound to any user object), default = EIP_MAX_USER_SERVICES_TAGLIST_DEFAULT. Taglist-controlled. */
  uint16_t usMaxUserServices;

  /** Maximum number of CIP objects the host can register through MR_REGISTER_REQ, default = EIP_MAX_USER_OBJECTS_TAGLIST_DEFAULT.  Taglist-controlled. */
  uint16_t usMaxUserObjects;

  /** Maximum number of Assembly Instances the host can register, platform-specific default = EIP_ADAPTER_MAX_ASSEMBLY_INSTANCE_TAGLIST_DEFAULT. Taglist-controlled. */
  uint16_t usMaxAdapterAssemblyInstance;

  /** Size of the Assembly object's data memory pool in bytes. Platform-specific default = EIP_AS_DATA_MEMPOOL_SIZE_TAGLIST_DEFAULT. Taglist-controlled.
      The Assembly object memory pool will be dynamically used for assembly data on assembly registration/creation.

      Memory pool sizes are already adjusted carefully according to the firmware specifications given in the firmware datasheet.
      If you adjust pool sizes, we recommend to proportionally increase or decrease with the other parameters.
      For instance, you could decrease the maximum number of assemblies to 50% of the original value while also halving the pool sizes.
  */
  uint16_t usAssemblyDataMemPoolSize;

  /** Size of the Assembly object's meta memory pool in bytes. Platform-specific default = EIP_AS_META_MEMPOOL_SIZE_TAGLIST_DEFAULT. Taglist-controlled.
      The Assembly object meta memory pool will be dynamically used for management data on assembly creation and connection establishment.
      Insufficient pool sizes may result in pool overuse which in case of the Assembly object may cause system runtime failure (e.g. some I/O connections cannot be established).
  */
  uint16_t usAssemblyMetaMemPoolSize;

  /** Depth of the server's UDP receive pointer queue for encapsulation commands (ListIdentity and friends). There is a single queue of bMaxUdpQueueElements entries. Default is EIP_MAX_UDP_QUEUE_ELEMENT_TAGLIST_DEFAULT (8). Taglist controlled. */
  uint8_t bMaxUdpQueueElements;

  /** Depth of the server's UDP receive pointer queue for I/O frames. There is a single queue of bMaxIoQueueElements entries.  This is scaled to two times the maximum number of I/O connections. Taglist-controlled. Default EIP_MAX_IO_QUEUE_ELEMENT_TAGLIST_DEFAULT. */
  uint8_t bMaxIoQueueElements;

  /** Maximum number of TCP connections/sockets. Taglist-controlled.
   *
   * The TCP server will open up to (usMaxTcpConnections + 1) sockets, where the (+ 1) is for listening socketand usMaxTcpConnections is the depth of the socket backlog,
   * i.e. the maximum number of parallel TCP sessions. TCP connections are used for UCMM or CL3 explicit messaging, where one TCP session can open multiple Class 3 connections
   * and perform multiple UCCM requests in parallel. These are controlled through parameters usMaxTargetCl3Connections and usMaxTargetUcmmConnections.
   *
   * This setting is entirely unrelated to the LWIP quantities setting.</td>
   */
  uint16_t usMaxTcpConnections;

  /** For each TCP socket a send and receive pointer queue is allocated. This is the depth of the queue. Default is EIP_MAX_TCP_QUEUE_ELEMENT_TAGLIST_DEFAULT. Taglist-controlled. */
  uint8_t bMaxTcpQueueElements;
  uint8_t bPad0;

  /** The overall number of target and originator I/O connections. This limits the overall maximum number of producers and consumers in the system. One of each for every I/O connection. Taglist-controlled. */
  uint16_t usMaxIoConnections;

  /** The connection manager object maintains target connections of types Class 0/1 and Class 3. This parameter dimensions the number of CL0/1 connections the CM object can maintain in parallel.
      If the given number of connections is established, further connection attempts will be rejected. Default is EIP_ADAPTER_MAX_IO_CONNECTIONS_TAGLIST_DEFAULT. Taglist-controlled. */
  uint16_t usMaxTargetIoConnections;

  /** The connection manager object maintains target connections of types Class 0/1 and Class 3. This parameter dimensions the number of CL3 connections the CM object can maintain in parallel.
      If the given number of connections is established, further connection attempts will be rejected. Default is EIP_MAX_CL3_CONNECTIONS_TAGLIST_DEFAULT (== EIP_MAX_TCP_CONNECTIONS_TAGLIST_DEFAULT). Taglist-controlled. */
  uint16_t usMaxTargetCl3Connections;

  /** The number of UCMM connections determines how many unconnected requests can be handled in parallel. Typically, clients issue a request and wait for the response, which
      occupies one UCCM entry while the request is being processed. That is why we scale the number of UCCM connections to the number of TCP connections. Anyway, each client is perfectly capable to issue multiple requests at once, thus consuming multiple (all) of these objects. Default ist EIP_MAX_UCMM_CONNECTIONS_TAGLIST_DEFAULT. Taglist-controlled. */
  uint16_t usMaxTargetUcmmConnections;

  /** Maximum number of PDC Instances the host can register, platform-specific default = EIP_MAX_PDC_INSTANCE_TAGLIST_DEFAULT. Taglist-controlled.
      Each PDC instance serves as an endpoint of a CIP implicit connection for which we are the target. Therefore, each PDC instance
      refers to an input, an output, and optionally a configuration assembly instance. The PDC instance will further maintain connection types,
      and RPI ranges. PDC instances control how often a connection (e.g. InputOnly) can be opened. Opening of a connection allocates a
      producer and a consumer transport from the PDC object's global memory pool. PDC instances partition that memory pool.
  */
  uint8_t  bMaxPdcInstance;
  uint8_t  bPad1;

  /** Size of the PDC object's global memory pool in bytes. Platform-specific default = EIP_PDC_MEMPOOL_SIZE_TAGLIST_DEFAULT. Taglist-controlled. Producing and consuming transports
      will be allocated from this pool on connection opening. Especially producing transports maintained by the PDC object instances will require memory buffers
      according to the size of the producing assembly, plus overhead. This memory is taken from the PDC's memory pool as well.
      If the PDC's memory pool is exhausted, no more PDC instances can be created.

      Memory pool sizes are already adjusted carefully according to the firmware specifications given in the firmware datasheet.
      If you adjust pool sizes, we recommend to proportionally increase or decrease with the other parameters.
      For instance, you could increase the maximum number of PDC instances by 100% of the original value while also doubling the pool size.
  */
  uint16_t usPdcMemPoolSize;

  /** Some reserved values for further extension of this tag. Do not touch these values. */
  uint16_t usReserved1;
  uint16_t usReserved2;
  uint16_t usReserved3;
  uint16_t usReserved4;

} HIL_TAG_EIP_RESOURCES_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T              tHeader;
  HIL_TAG_EIP_RESOURCES_DATA_T  tData;
} HIL_TAG_EIP_RESOURCES_T;


/**************************************************************************************
  Tag:  HIL_TAG_EIP_TIMESYNC_ENABLE_DISABLE
  Name: EtherNet/IP CIPSync support enable/disable
  Desc: Controls support for Time Sync (CIPSync). Disabling will allow the memory to be used for other means.

  Help: See Tag list editor

**************************************************************************************/
typedef struct
{
  uint8_t  bRegisterTimesyncObject;   /**!< Defines whether the Time Sync object will be available after firmware start as a builtin object. */
  uint8_t  abPadding[3];
} HIL_TAG_EIP_TIMESYNC_ENABLE_DISABLE_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                           tHeader;
  HIL_TAG_EIP_TIMESYNC_ENABLE_DISABLE_DATA_T tData;
} HIL_TAG_EIP_TIMESYNC_ENABLE_DISABLE_T;

/**************************************************************************************
  Tag:  HIL_TAG_EIP_FILE_OBJECT_ENABLE_DISABLE
  Name: EtherNet/IP CIP File Object support enable/disable
  Desc: Controls support for the CIP File Object (class code 0x37). Disabling will allow the memory to be used for other means.

  Help:
        ulEnableFileObject:
          This parameter enables or disables the CIP File Object (class code 0x37).

        0:  disabled
            CIP File Object is disabled and will not be available in the firmware's CIP object dictionary.

        !=0: enabled
            CIP File Object is enabled and will be available in the firmware's CIP object dictionary, if configured by the host application.
**************************************************************************************/

typedef struct
{
  uint32_t ulEnableFileObject;

} HIL_TAG_EIP_FILE_OBJECT_ENABLE_DISABLE_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                              tHeader;
  HIL_TAG_EIP_FILE_OBJECT_ENABLE_DISABLE_DATA_T tData;
} HIL_TAG_EIP_FILE_OBJECT_ENABLE_DISABLE_T;

/**************************************************************************************
  Tag:  HIL_TAG_DP_DEVICEID
  Name: PROFIBUS Product Information
  Desc: Configures the PROFIBUS product information.

  Help: With this tag you can modify PROFIBUS product information when using database configuration.
**************************************************************************************/
typedef struct HIL_TAG_DP_DEVICEID_DATA_Ttag
{
  /** PROFIBUS Ident Number */
  uint16_t   usIdentNr;

  /** Reserved field for future use
   * Set to 0 to avoid unwanted behavior with upcoming version. */
  uint8_t      abReserved[2];
} HIL_TAG_DP_DEVICEID_DATA_T;

typedef struct HILX_TAG_DP_DEVICEID_Ttag
{
  HIL_TAG_HEADER_T            tHeader;
  HIL_TAG_DP_DEVICEID_DATA_T  tData;
} HIL_TAG_DP_DEVICEID_T;


/**************************************************************************************
  Tag:  HIL_TAG_PN_DEVICEID
  Name: PROFINET Product Information
  Desc: Configures the PROFINET product information.

  Help: With this tag you can modify PROFINET product information when using database configuration.
**************************************************************************************/
typedef struct
{
  /** Vendor ID
   * - 1..65279 (0xFEFF): Vendor identification number of the manufacturer, which has been assigned to the vendor by the PROFIBUS Nutzerorganisation e. V.. All Hilscher products use the value `0x011E`. */
  uint32_t ulVendorId;

  /** Device ID
   * - 1..65535 (0xFFFF): This is a fixed and unique identification number for every device. It is assigned by the manufacturer. */
  uint32_t ulDeviceId;
} HIL_TAG_PN_DEVICEID_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T             tHeader;
  HIL_TAG_PN_DEVICEID_DATA_T   tData;
} HIL_TAG_PN_DEVICEID_T;



/**************************************************************************************
  Tag:  HIL_TAG_PROFINET_FEATURES
  Name: PROFINET Features
  Desc: Configures PROFINET features of the firmware.

  Help: With this tag you can configure several PROFINET features of the firmware.
**************************************************************************************/
typedef struct
{
  /** Number of additional IO Connections (ARs)
   * - 0: A single cyclic PROFINET connection
   * - 1..3: Number of cyclic PROFINET connections
   * Note: Please refer to the technical data in your PROFINET IO Device API Manual for capabilities of the stack version used (it may support less than 3 additional ARs). */
  uint8_t      bNumAdditionalIoAR;

  /** IO Supervisor communication accepted
   * - 0: IO Supervisor communication is not accepted by firmware
   * - 1: IO Supervisor communication is accepted by firmware */
  uint8_t      bIoSupervisorSupported;

  /** IRT Communication accepted
   * - 0: IRT communication is not accepted by firmware
   * - 1: IRT communication is accepted by firmware */
  uint8_t      bIRTSupported;

  /** Padding
   * Reserved for padding to PROFINET spec */
  uint8_t      bReserved;

  /** MinDeviceInterval
   * - 8..4096: The MinDeviceInterval according to GSDML file of the product. Power of 2 in range. */
  uint16_t     usMinDeviceInterval;

  /** Padding
   * Reserved for future use */
  uint8_t      abReserved[2];
} HIL_TAG_PROFINET_FEATURES_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                      tHeader;
  HIL_TAG_PROFINET_FEATURES_DATA_T      tData;
} HIL_TAG_PROFINET_FEATURES_T;



/**************************************************************************************
  Tag:  HIL_TAG_PROFINET_FEATURES_V2
  Name: PROFINET Features V2
  Desc: Configures PROFINET features of the firmware.

  Help: With this tag you can configure several PROFINET features of the firmware.


        Note: Due to limited memory resources of some firmware implementations,
        restrictions apply regarding the possible combinations of the features.
        Please refer to PROFINET IO Device API Manual for more details about the
        consequences of changing one of these values.
**************************************************************************************/
typedef struct
{
  /** Number of configurable submodules
   * - 1..1000: The maximum amount of submodules an application can configure in the firmware.
   * This number has an impact on the runtime memory required by the firmware.
   *
   *
   * Note: Please refer to the technical data in your PROFINET IO Device API Manual
   * for the capabilities of the stack version used (it may support less than 1000 submodules). */
  uint16_t     usNumSubmodules;

  /** Minimum size of each RPC buffer in KB
   * - 4..64: The minimum size in KB of each existing RPC buffer preallocated by the firmware.
   * Each AR (IO, DeviceAccess, Supervisor, ReadImplicit) has its own buffer of this size.
   * The real buffer size depends on other factors and may be higher than this value.
   *
   *
   * Note: Please refer to the technical data in your PROFINET IO Device API Manual for the capabilities of the stack version used. */
  uint8_t      bRPCBufferSize;

  /** Number of additional IO Connections (ARs)
   * Number of additional IO ARs available for Shared Device and SystemRedundancy. Nonzero value enables additional IO connections.
   * - 0: Only 1 cyclic PROFINET connection is possible. Shared Device is not supported.
   * - 1..7: In total, 2-8 cyclic PROFINET connections are supported. Shared device is supported.
   *
   *
   * Note: Please refer to the technical data in your PROFINET IO Device API Manual for the capabilities of the stack version used (it may support less than 7 additional ARs).
   *
   * The value of this parameter has an influence on the GSDML file provided with the product.
   * The GSDML parameter "NumberOfAR" needs to be set to the value of this parameter plus 1.
   *
   *
   * Note: Special handling for firmware including System Redundancy
   *
   *
   * - Setting this parameter to 0 still allows using S2 redundancy with 2 redundant IO-Controllers.
   * Shared Device with 2 non-redundant IO Controllers is not supported in this case. This represents an IO Device supporting S2 redundancy but without Shared Device.
   * In this special case, the GSDML parameter "NumberOfAR" needs to be set to value 2.
   *
   * - Setting this parameter to 1 (or higher) activates Shared Device plus S2 redundancy in the firmware.
   * In this case, the default rule for GSDML parameter "NumberOfAR" applies and needs to be set to this value plus 1. */
  uint8_t      bNumAdditionalIOAR;

  /** Number of parallel Read Implicit Requests
   * - 1..8: Number of implicit ARs used for Read Implicit Services.
   * The amount of parallel acyclic Read Implicit Requests that can be handled by the firmware simultaneously.
   *
   *
   * Note: Please refer to the technical data in your PROFINET IO Device API Manual for the capabilities of the stack version used (it may support less than 8 parallel Services). */
  uint8_t      bNumImplicitAR;

  /** Number of parallel DeviceAccess ARs
   * Number of parallel Device Access ARs supported by device.
   * - 0: DeviceAccess AR is not supported
   * - 1: DeviceAccess AR is enabled */
  uint8_t      bNumDAAR;

  /** Number of available diagnosis buffers
   * The total amount of buffers the firmware shall reserve for Channel diagnosis datasets.
   * The application can not create more diagnosis datasets than this number.
   * Allowed values are: 0..1000 */
  uint16_t     usNumSubmDiagnosis;
} HIL_TAG_PROFINET_FEATURES_V2_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                         tHeader;
  HIL_TAG_PROFINET_FEATURES_V2_DATA_T      tData;
} HIL_TAG_PROFINET_FEATURES_V2_T;



/**************************************************************************************
  Tag:  HIL_TAG_PROFINET_SYSTEM_REDUNDANCY_FEATURES
  Name: PROFINET SystemRedundancy
  Desc: Configures SystemRedundancy functionality.

  Help: With this tag you can configure SystemRedundancy functionality.

        Note: Due to limited memory resources of some firmware implementations,
        restrictions apply regarding the possible combinations of the features.
        Please refer to PROFINET IO Device API Manual for more details about the
        consequences of changing one of these values.
**************************************************************************************/
typedef struct
{
  /** Number of parallel ARSets
   * Number of AR Sets supported by the device. Set to non-zero value to allow SR type connections.
   * - 0: No SR-ARSet is allowed. SystemRedundancy is not available.
   * - 1: One SR-ARSet is allowed. SystenRedundancy is available.
   * Note: Please refer to the technical data in your PROFINET IO Device API Manual
   * for the capabilities of the stack version used (it may not support System Redundancy). */
  uint8_t      bNumberOfARSets;

  /** 32Bits alignment
   * Padding to comply with PROFINET spec. */
  uint8_t      abPadding[3];
} HIL_TAG_PROFINET_SYSTEM_REDUNDANCY_FEATURES_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                                        tHeader;
  HIL_TAG_PROFINET_SYSTEM_REDUNDANCY_FEATURES_DATA_T      tData;
} HIL_TAG_PROFINET_SYSTEM_REDUNDANCY_FEATURES_T;



/**************************************************************************************
  Tag:  HIL_TAG_PROFINET_CONTROLLER_QUANTITIES
  Name: PROFINET Controller Features
  Desc: Configures the quantity structures of PROFINET IO-Controller features.

  Help: With this tag, you can modify the quantity structures of PROFINET IO-Controller part
        of the firmware. This may be required in case the Hilscher default values do not meet
        specific customer requirements.
        Changing the parameters will influence the memory requirements of the firmware.

        Note: Due to available memory resources, restrictions apply regarding the possible
        combinations of the features. It is not possible to max out all quantity structures in parallel.
**************************************************************************************/
#define HIL_TAG_PNM_MIN_NUM_IO_AR                   1
#define HIL_TAG_PNM_MAX_NUM_IO_AR                   128
#define HIL_TAG_PNM_MIN_NUM_DA_AR                   0
#define HIL_TAG_PNM_MAX_NUM_DA_AR                   1
#define HIL_TAG_PNM_MIN_NUM_SUBMODULE               32
#define HIL_TAG_PNM_MAX_NUM_SUBMODULE               2048
#define HIL_TAG_PNM_MIN_SIZE_PARAM_RECORD_STORE     4
#define HIL_TAG_PNM_DEFAULT_PARAM_RECORD_STORE      16
#define HIL_TAG_PNM_MAX_SIZE_PARAM_RECORD_STORE     256
#define HIL_TAG_PNM_MIN_NUM_AR_VENDOR_BLOCKS        1
#define HIL_TAG_PNM_DEFAULT_NUM_AR_VENDOR_BLOCKS    256
#define HIL_TAG_PNM_MAX_NUM_AR_VENDOR_BLOCKS        512
#define HIL_TAG_PNM_MIN_SIZE_AR_VENDOR_BLOCK        128
#define HIL_TAG_PNM_DEFAULT_SIZE_AR_VENDOR_BLOCK    512
#define HIL_TAG_PNM_MAX_SIZE_AR_VENDOR_BLOCK        4096
#define HIL_TAG_PNM_MIN_SIZE_APPL_RPC_BUFFER        4
#define HIL_TAG_PNM_DEFAULT_SIZE_AR_APPL_RPC_BUFFER 64
#define HIL_TAG_PNM_MAX_SIZE_APPL_RPC_BUFFER        256

typedef struct
{
  /** Number of parallel IO Connections (ARs)
   * - 1..128: The number of cyclic PROFINET IO connections supported by the firmware.
   * Configurations containing more configured ARs will be rejected.  */
  uint16_t     usNumIOAR;

  /** Number of parallel Device Access ARs
   * - 0: DeviceAccess AR is not supported.
   * - 1: DeviceAccess AR is supported.
   * Configurations containing more configured ARs will be rejected.  */
  uint16_t     usNumDAAR;

  /** Number of submodules
   * - 32..2048: The amount of submodules supported by the firmware (sum of all submodules of all IO ARs).
   * Configurations containing more submodules will be rejected. */
  uint16_t     usNumSubmodules;

  /** GSD parameter record storage size
   * - 4..256: The available memory for storing GSD record parameters in KB.
   * Each IO AR has its own storage of this size.
   * The PROFINET Specification requires a minimum value of 16 (default).
   * Configurations containing more GSD parameters per IO AR will be rejected. */
  uint16_t     usParamRecordStorage;

  /** Number of ARVendorBlocks
   * - 1..512: The amount of ARVendorBlocks supported by the firmware (sum of all ARVendorBlocks of all IO ARs).
   * Configurations containing more ARVendorBlocks will be rejected. */
  uint16_t     usNumARVendorBlocks;

  /** ARVendorBlock storage size
   * - 128..4096: The supported size per ARVendorBlock supported by the firmware.
   * Configurations containing larger ARVendorBlocks will be rejected. */
  uint16_t     usSizeARVendorBlock;

  /** Size for acyclic application read/write requests
   * - 4..256: Size of the application service RPC buffer in KB.
   * The firmware offers one large buffer for runtime read/write requests.
   * The size of this buffer in KB is set here.
   * Each IO AR automatically has a small additional buffer whose size can not be modified. */
  uint16_t     usSizeApplRpcBuffer;

  /** 32Bits alignment
   * The tag contains 2 bytes of padding to comply with the PROFINET spec. */
  uint8_t      abPadding[2];

} HIL_TAG_PROFINET_CONTROLLER_QUANTITIES_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                                        tHeader;
  HIL_TAG_PROFINET_CONTROLLER_QUANTITIES_DATA_T           tData;
} HIL_TAG_PROFINET_CONTROLLER_QUANTITIES_T;



/**************************************************************************************
  Tag:  HIL_TAG_LWIP_PORTS_FOR_IP_ZERO
  Name: LWIP Ports for IP 0.0.0.0
  Desc: Enable IP ports for usage with broadcast communication when IP is 0.0.0.0.

  Help: With this tag, you can set IP ports which the LWIP stack integrated in firmware
        shall handle even if it does not have a valid IP configuration.

        By default (if taglist is unchanged) the ports will be set to 0 and the LWIP
        stack will not receive broadcast frames until the stack is configured.

        When a port is set from 1 to 65535, the ports will be usable with broadcast
        communication only as long as the LWIP stack is not configured.

        Setting a port to 0 means that this entry in taglist shall be ignored.
        This allows only activating a single port (or even zero ports).
**************************************************************************************/
typedef struct
{
  /** Defines port 1
   * - 0: Entry is ignored, no handling until IP configuration (default).
   * - 1..65535: Port to be handled with broadcast until stack is configured. */
  uint16_t      usPort0;

  /** Defines port 2
   * - 0: Entry is ignored, no handling until IP configuration (default).
   * - 1..65535: Port to be handled with broadcast until stack is configured. */
  uint16_t      usPort1;

} HIL_TAG_LWIP_PORTS_FOR_IP_ZERO_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T                       tHeader;
  HIL_TAG_LWIP_PORTS_FOR_IP_ZERO_DATA_T  tData;
} HIL_TAG_LWIP_PORTS_FOR_IP_ZERO_T;



/**************************************************************************************
  Tag: HIL_TAG_S3S_SCP_FEATURES
  Name: Sercos SCP Features
  Desc: Enable or disable Sercos SCP features.

  Help: Due to CRA requirements it could be necessary to disable the SCP types
        SIP (Sercos Internet Protocol Services) or the TFTP (Trivial File Transfer Protocol).

**************************************************************************************/
typedef struct
{
  /** SIP (Sercos Internet Protocol Services)
   * - 0: Disabled
   * - 1: Enabled */
  uint8_t bSIPSupported;

  /** TFTP (Trivial File Transfer Protocol)
   * - 0: Disabled
   * - 1: Enabled */
  uint8_t bTFTPSupported;

  /** Padding
   * Reserved for future use */
  uint8_t      abReserved[2];
} HIL_TAG_S3S_SCP_FEATURES_DATA_T;

typedef struct
{
  HIL_TAG_HEADER_T tHeader;
  HIL_TAG_S3S_SCP_FEATURES_DATA_T tData;
} HIL_TAG_S3S_SCP_FEATURES_T;


/**************************************************************************************
  Backward Compatibility Definitions.
  Note: It is recommend to update components which using those definitions
**************************************************************************************/
#ifndef HIL_TAG_DISABLE_COMPATIBILITY_MODE
  #define HIL_MOD_TAG_IGNORE_FLAG                     HIL_TAG_IGNORE_FLAG
  #define HIL_MOD_TAG_END                             HIL_TAG_END_OF_LIST
  #define HIL_MOD_TAG_NUM_COMM_CHANNEL                HIL_TAG_NUM_COMM_CHANNEL
  #define HIL_MOD_TAG_IT_STATIC_TASKS                 HIL_TAG_TASK_GROUP
  #define HIL_MOD_TAG_IT_STATIC_TASK_PARAMETER_BLOCK  HIL_TAG_IT_STATIC_TASK_PARAMETER_BLOCK
  #define HIL_MOD_TAG_IT_STATIC_TASK_ENTRY            HIL_TAG_IT_STATIC_TASK_ENTRY
  #define HIL_MOD_TAG_IT_TIMER                        HIL_TAG_TIMER
  #define HIL_MOD_TAG_IT_INTERRUPT                    HIL_TAG_INTERRUPT_GROUP
  #define HIL_MOD_TAG_IT_LED                          HIL_TAG_LED
  #define HIL_MOD_TAG_IT_XC                           HIL_TAG_XC
  #define HIL_MOD_TAG_IT_LED_RESOURCE_TYPE_PIO        HIL_TAG_LED_RESOURCE_TYPE_PIO
  #define HIL_MOD_TAG_IT_LED_RESOURCE_TYPE_GPIO       HIL_TAG_LED_RESOURCE_TYPE_GPIO
  #define HIL_MOD_TAG_IT_LED_RESOURCE_TYPE_HIFPIO     HIL_TAG_LED_RESOURCE_TYPE_HIFPIO
  #define HIL_MOD_TAG_IT_LED_POLARITY_NORMAL          HIL_TAG_LED_POLARITY_NORMAL
  #define HIL_MOD_TAG_IT_LED_POLARITY_INVERTED        HIL_TAG_LED_POLARITY_INVERTED


  typedef HIL_TAG_HEADER_T                            HIL_MODULE_TAG_ENTRY_HEADER_T;
  typedef HIL_TAG_IDENTIFIER_T                        HIL_MOD_TAG_IDENTIFIER_T;
  typedef HIL_TAG_UINT32_T                            HIL_MODULE_TAG_ENTRY_UINT32_T;
  typedef HIL_TAG_TASK_GROUP_DATA_T                   HIL_MOD_TAG_IT_STATIC_TASKS_T;
  typedef HIL_TAG_TASK_GROUP_T                        HIL_MOD_TAG_IT_STATIC_TASKS_TAG_T;
  typedef HIL_TAG_INTERRUPT_GROUP_DATA_T              HIL_MOD_TAG_IT_INTERRUPT_T;
  typedef HIL_TAG_INTERRUPT_GROUP_T                   HIL_MOD_TAG_IT_INTERRUPT_TAG_T;
  typedef HIL_TAG_TIMER_DATA_T                        HIL_MOD_TAG_IT_TIMER_T;
  typedef HIL_TAG_TIMER_T                             HIL_MOD_TAG_IT_TIMER_TAG_T;
  typedef HIL_TAG_LED_DATA_T                          HIL_MOD_TAG_IT_LED_T;
  typedef HIL_TAG_LED_T                               HIL_MOD_TAG_IT_LED_TAG_T;
  typedef HIL_TAG_XC_DATA_T                           HIL_MOD_TAG_IT_XC_T;
  typedef HIL_TAG_XC_T                                HIL_MOD_TAG_IT_XC_TAG_T;

  typedef struct
  {
    char szTaskName[16];      /** task name, read-only */
    uint32_t ulTaskGroupRef;  /** group reference number (common to all tasks in the group), read-only */
    uint32_t ulPriority;      /** task priority (offset) relative to task group's base task priority */
    uint32_t ulToken;         /** task token (offset) relative to task group's base task token */
  } HIL_MOD_TAG_IT_STATIC_TASK_ENTRY_T;

  typedef struct
  {
    HIL_MODULE_TAG_ENTRY_HEADER_T      tHeader;
    HIL_MOD_TAG_IT_STATIC_TASK_ENTRY_T tData;
  } HIL_MOD_TAG_IT_STATIC_TASK_ENTRY_TAG_T;

  /** generic task parameter block / substructure referenced by index */
  typedef struct
  {
    uint32_t ulSubstructureIdx;  /** read-only */
    uint32_t ulTaskIdentifier;   /** read-only, Task identifier as specified in TLR_TaskIdentifier.h */
    uint32_t ulParameterVersion; /** read-only, parameter version as specified by particular task */
  } HIL_MOD_TAG_IT_STATIC_TASK_PARAMETER_BLOCK_T;

  /** servX port configuration tag has been renamed  */
  #define HIL_TAG_SERVX_PORT_NUMBER         HIL_TAG_HTTP_PORT_CONFIG
  #define HIL_TAG_SERVX_PORT_NUMBER_DATA_T  HIL_TAG_HTTP_PORT_CONFIG_DATA_T
  #define HIL_TAG_SERVX_PORT_NUMBER_T       HIL_TAG_HTTP_PORT_CONFIG_T

#endif /* HIL_TAG_DISABLE_COMPATIBILITY_MODE */
/**************************************************************************************
  End of backward compatibility Definitions.
**************************************************************************************/



#endif  /* HIL_TAGLIST_H_ */
