/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_ComponentID.h $: *//*!

  \file Hil_ComponentID.h

  This file contains all Hilscher components (task) identifiers.

  \note New component IDs will be administered by the software protocol (SPC)
        department.

**************************************************************************************/
#ifndef HIL_COMPONENT_ID_H_
#define HIL_COMPONENT_ID_H_

#include<stdint.h>

/* MessageId: HIL_COMPONENT_ID_UNKNOWN_IDENTIFIER */
/* MessageText: The task identifier is unknown. */
#define HIL_COMPONENT_ID_UNKNOWN_IDENTIFIER ((uint32_t)0x00000000L)

/***********************************************************************************/
/* TLR Timer identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_TIMER */
/* MessageText: TLR Timer Task. */
#define HIL_COMPONENT_ID_TIMER           ((uint32_t)0x00020001L)

/***********************************************************************************/
/* System task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_LIBSTORAGE */
/* MessageText: LibStorage. */
#define HIL_COMPONENT_ID_LIBSTORAGE      ((uint32_t)0x00010000L)

/* MessageId: HIL_COMPONENT_ID_MID_SYS */
/* MessageText: Middleware System Task. */
#define HIL_COMPONENT_ID_MID_SYS         ((uint32_t)0x00010001L)

/* MessageId: HIL_COMPONENT_ID_MID_DBG */
/* MessageText: Middleware System Debug Backend Task. */
#define HIL_COMPONENT_ID_MID_DBG         ((uint32_t)0x00010002L)

/* MessageId: HIL_COMPONENT_ID_RX_IDLE */
/* MessageText: RX IDLE Task. */
#define HIL_COMPONENT_ID_RX_IDLE         ((uint32_t)0x00010003L)

/* MessageId: HIL_COMPONENT_ID_IRQ_HANDLER */
/* MessageText: IRQ Handler Task. */
#define HIL_COMPONENT_ID_IRQ_HANDLER     ((uint32_t)0x00010004L)

/* MessageId: HIL_COMPONENT_ID_IDLE */
/* MessageText: Idle Task. */
#define HIL_COMPONENT_ID_IDLE            ((uint32_t)0x00010005L)

/* MessageId: HIL_COMPONENT_ID_BOOTUP */
/* MessageText: Bootup Task. */
#define HIL_COMPONENT_ID_BOOTUP          ((uint32_t)0x00010006L)

/* MessageId: HIL_COMPONENT_ID_RX_TIMER */
/* MessageText: rcX Timer. */
#define HIL_COMPONENT_ID_RX_TIMER        ((uint32_t)0x00010007L)

/* MessageId: HIL_COMPONENT_ID_MAINTENANCE */
/* MessageText: Maintenance Task. */
#define HIL_COMPONENT_ID_MAINTENANCE     ((uint32_t)0x00010008L)

/***********************************************************************************/
/* EtherCAT Base stack task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ECAT */
/* MessageText: EtherCAT Stack. */
#define HIL_COMPONENT_ID_ECAT            ((uint32_t)0x00200000L)

/* MessageId: HIL_COMPONENT_ID_ECAT_ESM */
/* MessageText: EtherCAT ESM Task. */
#define HIL_COMPONENT_ID_ECAT_ESM        ((uint32_t)0x00200001L)

/* MessageId: HIL_COMPONENT_ID_ECAT_MBX */
/* MessageText: EtherCAT Mailbox Task. */
#define HIL_COMPONENT_ID_ECAT_MBX        ((uint32_t)0x00200002L)

/* MessageId: HIL_COMPONENT_ID_ECAT_CYCLIC_IN */
/* MessageText: EtherCAT Cyclic Input Task. */
#define HIL_COMPONENT_ID_ECAT_CYCLIC_IN  ((uint32_t)0x00200003L)

/* MessageId: HIL_COMPONENT_ID_ECAT_CYCLIC_OUT */
/* MessageText: EtherCAT Cyclic Output Task. */
#define HIL_COMPONENT_ID_ECAT_CYCLIC_OUT ((uint32_t)0x00200004L)

/***********************************************************************************/
/* EtherCAT CoE stack task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ECAT_COE */
/* MessageText: EtherCAT CoE Task. */
#define HIL_COMPONENT_ID_ECAT_COE        ((uint32_t)0x00210001L)

/* MessageId: HIL_COMPONENT_ID_ECAT_COE_PDO */
/* MessageText: EtherCAT CoE PDO Task. */
#define HIL_COMPONENT_ID_ECAT_COE_PDO    ((uint32_t)0x00210002L)

/* MessageId: HIL_COMPONENT_ID_ECAT_COE_SDO */
/* MessageText: EtherCAT CoE SDO Task. */
#define HIL_COMPONENT_ID_ECAT_COE_SDO    ((uint32_t)0x00210003L)

/***********************************************************************************/
/* EtherCAT VoE stack task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ECAT_VOE */
/* MessageText: EtherCAT VoE Task. */
#define HIL_COMPONENT_ID_ECAT_VOE        ((uint32_t)0x00260001L)

/***********************************************************************************/
/* EtherCAT FoE stack task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ECAT_FOE */
/* MessageText: EtherCAT FoE Task. */
#define HIL_COMPONENT_ID_ECAT_FOE        ((uint32_t)0x00240001L)

/* MessageId: HIL_COMPONENT_ID_ECAT_FOE_FH */
/* MessageText: EtherCAT FoE File Handler Task. */
#define HIL_COMPONENT_ID_ECAT_FOE_FH     ((uint32_t)0x00240002L)

/***********************************************************************************/
/* EtherCAT EoE stack task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ECAT_EOE */
/* MessageText: EtherCAT EoE Task. */
#define HIL_COMPONENT_ID_ECAT_EOE        ((uint32_t)0x00230001L)

/***********************************************************************************/
/* EtherCAT SoE stack task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ECAT_SOE_SSC */
/* MessageText: EtherCAT SoE SSC-Task. */
#define HIL_COMPONENT_ID_ECAT_SOE_SSC    ((uint32_t)0x00220001L)

/* MessageId: HIL_COMPONENT_ID_ECAT_SOE_IDN */
/* MessageText: EtherCAT SoE IDN-Task. */
#define HIL_COMPONENT_ID_ECAT_SOE_IDN    ((uint32_t)0x00220002L)

/***********************************************************************************/
/* Example Tasks */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_EXAMPLE_TASK1 */
/* MessageText: Example Task 1. */
#define HIL_COMPONENT_ID_EXAMPLE_TASK1   ((uint32_t)0x00030001L)

/* MessageId: HIL_COMPONENT_ID_EXAMPLE_TASK2 */
/* MessageText: Example Task 2. */
#define HIL_COMPONENT_ID_EXAMPLE_TASK2   ((uint32_t)0x00040001L)

/* MessageId: HIL_COMPONENT_ID_EXAMPLE_TASK3 */
/* MessageText: Example Task 3. */
#define HIL_COMPONENT_ID_EXAMPLE_TASK3   ((uint32_t)0x00050001L)

/***********************************************************************************/
/* EtherNet/IP Encapsulation */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_EIP_ENCAP */
/* MessageText: EthernetIP Encapsulation Task. */
#define HIL_COMPONENT_ID_EIP_ENCAP       ((uint32_t)0x001E0001L)

/* MessageId: HIL_COMPONENT_ID_EIP_CL1 */
/* MessageText: EthernetIP Encapsulation Task Class 1 services. */
#define HIL_COMPONENT_ID_EIP_CL1         ((uint32_t)0x001E0002L)

/***********************************************************************************/
/* EtherNet/IP Object */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_EIP_OBJECT */
/* MessageText: EthernetIP Object Task. */
#define HIL_COMPONENT_ID_EIP_OBJECT      ((uint32_t)0x001F0001L)

/***********************************************************************************/
/* EtherNet/Ip HAL EDD 2PS task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_EIP_EDD_LOW */
/* MessageText: EtherNet/Ip Low Priority EDD Task. */
#define HIL_COMPONENT_ID_EIP_EDD_LOW     ((uint32_t)0x00EF0001L)

/* MessageId: HIL_COMPONENT_ID_EIP_EDD_HIGH */
/* MessageText: EtherNet/Ip High Priority EDD Task. */
#define HIL_COMPONENT_ID_EIP_EDD_HIGH    ((uint32_t)0x00EE0001L)

/***********************************************************************************/
/* PROFINET IO task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_PNIO_CMCTL */
/* MessageText: PROFINET IO-Controller Context Management Task. */
#define HIL_COMPONENT_ID_PNIO_CMCTL      ((uint32_t)0x000A0001L)

/* MessageId: HIL_COMPONENT_ID_PNIO_CMDEV */
/* MessageText: PROFINET IO-Device Context Management Task. */
#define HIL_COMPONENT_ID_PNIO_CMDEV      ((uint32_t)0x000B0001L)

/* MessageId: HIL_COMPONENT_ID_PNIO_ACP */
/* MessageText: PROFINET IO ACP Task. */
#define HIL_COMPONENT_ID_PNIO_ACP        ((uint32_t)0x00110001L)

/* MessageId: HIL_COMPONENT_ID_PNIO_DCP */
/* MessageText: PROFINET IO DCP Task. */
#define HIL_COMPONENT_ID_PNIO_DCP        ((uint32_t)0x00120001L)

/* MessageId: HIL_COMPONENT_ID_PNIO_EDD */
/* MessageText: PROFINET IO EDD Task. */
#define HIL_COMPONENT_ID_PNIO_EDD        ((uint32_t)0x000E0001L)

/* MessageId: HIL_COMPONENT_ID_PNIO_MGT */
/* MessageText: PROFINET IO Management Task. */
#define HIL_COMPONENT_ID_PNIO_MGT        ((uint32_t)0x00130001L)

/* MessageId: HIL_COMPONENT_ID_PNIO_APCTL */
/* MessageText: PROFINET IO-Controller Application Task. */
#define HIL_COMPONENT_ID_PNIO_APCTL      ((uint32_t)0x000C0001L)

/* MessageId: HIL_COMPONENT_ID_PNIO_APDEV */
/* MessageText: PROFINET IO-Device Application Task. */
#define HIL_COMPONENT_ID_PNIO_APDEV      ((uint32_t)0x000D0001L)

/* MessageId: HIL_COMPONENT_ID_PNIO_APCFG */
/* MessageText: PROFINET IO-Controller Configuration Task. */
#define HIL_COMPONENT_ID_PNIO_APCFG      ((uint32_t)0x00140001L)

/* MessageId: HIL_COMPONENT_ID_PNS_IF */
/* MessageText: PROFINET IO-Device Interface Task. */
#define HIL_COMPONENT_ID_PNS_IF          ((uint32_t)0x00300001L)

/* MessageId: HIL_COMPONENT_ID_PNIOD_16BITIO */
/* MessageText: PROFINET IO-Device 16Bit IO Application Task. */
#define HIL_COMPONENT_ID_PNIOD_16BITIO   ((uint32_t)0x003A0001L)

/* MessageId: HIL_COMPONENT_ID_PNS_32BITIO */
/* MessageText: PROFINET IO-Device 32Bit IO Application Task. */
#define HIL_COMPONENT_ID_PNS_32BITIO     ((uint32_t)0x005E0001L)

/* MessageId: HIL_COMPONENT_ID_PNS_4BITIO */
/* MessageText: PROFINET IO-Device 4Bit IO Application Task. */
#define HIL_COMPONENT_ID_PNS_4BITIO      ((uint32_t)0x00450001L)

/* MessageId: HIL_COMPONENT_ID_PNS_SOCKET_SRV */
/* MessageText: PROFINET IO-Device Socket Server Task. */
#define HIL_COMPONENT_ID_PNS_SOCKET_SRV  ((uint32_t)0x00520001L)

/* MessageId: HIL_COMPONENT_ID_PNS_EDD_HIGH */
/* MessageText: PROFINET IO-Device High Priority EDD Task. */
#define HIL_COMPONENT_ID_PNS_EDD_HIGH    ((uint32_t)0x00530001L)

/* MessageId: HIL_COMPONENT_ID_PNS_EDD_LOW */
/* MessageText: PROFINET IO-Device Low Priority EDD Task. */
#define HIL_COMPONENT_ID_PNS_EDD_LOW     ((uint32_t)0x00540001L)

/* MessageId: HIL_COMPONENT_ID_PNS_SOCKET */
/* MessageText: PROFINET IO-Device Socket Task. */
#define HIL_COMPONENT_ID_PNS_SOCKET      ((uint32_t)0x00550001L)

/* MessageId: HIL_COMPONENT_ID_PNS_DCP */
/* MessageText: PROFINET IO-Device DCP Task. */
#define HIL_COMPONENT_ID_PNS_DCP         ((uint32_t)0x00560001L)

/* MessageId: HIL_COMPONENT_ID_PNS_CLRPC */
/* MessageText: PROFINET IO-Device Connectionless RPC Task. */
#define HIL_COMPONENT_ID_PNS_CLRPC       ((uint32_t)0x00570001L)

/* MessageId: HIL_COMPONENT_ID_PNS_IF_INTERN */
/* MessageText: PROFINET IO-Device Stack internal Interface Task. */
#define HIL_COMPONENT_ID_PNS_IF_INTERN   ((uint32_t)0x00580001L)

/* MessageId: HIL_COMPONENT_ID_PNIO_IRT_SCHEDULER */
/* MessageText: PROFINET IO IRT Scheduler Task. */
#define HIL_COMPONENT_ID_PNIO_IRT_SCHEDULER ((uint32_t)0x00810001L)

/* MessageId: HIL_COMPONENT_ID_PNIO_RTA */
/* MessageText: PROFINET IO RTA Task. */
#define HIL_COMPONENT_ID_PNIO_RTA        ((uint32_t)0x00A70001L)

/* MessageId: HIL_COMPONENT_ID_PNIO_RTC */
/* MessageText: PROFINET IO RTC Task. */
#define HIL_COMPONENT_ID_PNIO_RTC        ((uint32_t)0x00A80001L)

/* MessageId: HIL_COMPONENT_ID_FODMI_TASK */
/* MessageText: PROFINET IO FODMI (FiberOptic Diagnosis) Task. */
#define HIL_COMPONENT_ID_FODMI_TASK      ((uint32_t)0x00A90001L)

/* MessageId: HIL_COMPONENT_ID_PNIO_HAL_COMPUTE */
/* MessageText: PROFINET IO HAL protocol Task. */
#define HIL_COMPONENT_ID_PNIO_HAL_COMPUTE ((uint32_t)0x00AA0001L)

/* MessageId: HIL_COMPONENT_ID_PNIO_HAL_IRQ */
/* MessageText: PROFINET IO HAL interrupt Task. */
#define HIL_COMPONENT_ID_PNIO_HAL_IRQ    ((uint32_t)0x00AB0001L)

/***********************************************************************************/
/* RPC task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_RPC_TASK */
/* MessageText: RPC Task. */
#define HIL_COMPONENT_ID_RPC_TASK        ((uint32_t)0x002E0001L)

/***********************************************************************************/
/* Router tasks */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ROUTER_OS_CONSOLE32 */
/* MessageText: TLR-Router OS_Console32. */
#define HIL_COMPONENT_ID_ROUTER_OS_CONSOLE32 ((uint32_t)0x002F0000L)

/* MessageId: HIL_COMPONENT_ID_ROUTER_ECAT_VOE */
/* MessageText: TLR-Router EtherCAT VoE. */
#define HIL_COMPONENT_ID_ROUTER_ECAT_VOE ((uint32_t)0x002F0001L)

/* MessageId: HIL_COMPONENT_ID_ROUTER_HIF_PACKET */
/* MessageText: TLR-Router DPM. */
#define HIL_COMPONENT_ID_ROUTER_HIF_PACKET ((uint32_t)0x002F0002L)

/***********************************************************************************/
/* PowerLink tasks */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_EPL_NMT */
/* MessageText: Ethernet PowerLink NMT Task. */
#define HIL_COMPONENT_ID_EPL_NMT         ((uint32_t)0x00170000L)

/* MessageId: HIL_COMPONENT_ID_EPL_PCK */
/* MessageText: Ethernet PowerLink Packet Task. */
#define HIL_COMPONENT_ID_EPL_PCK         ((uint32_t)0x00170001L)

/* MessageId: HIL_COMPONENT_ID_EPL_DPM */
/* MessageText: Ethernet PowerLink DPM Task. */
#define HIL_COMPONENT_ID_EPL_DPM         ((uint32_t)0x00170002L)

/* MessageId: HIL_COMPONENT_ID_PLSV3_AP */
/* MessageText: Ethernet PowerLink AP Task. */
#define HIL_COMPONENT_ID_PLSV3_AP        ((uint32_t)0x00E10000L)

/* MessageId: HIL_COMPONENT_ID_PLSV3_IF */
/* MessageText: Ethernet PowerLink CN IF Task. */
#define HIL_COMPONENT_ID_PLSV3_IF        ((uint32_t)0x00E30000L)

/* MessageId: HIL_COMPONENT_ID_PLSV3_NMT */
/* MessageText: Ethernet PowerLink CN NMT Task. */
#define HIL_COMPONENT_ID_PLSV3_NMT       ((uint32_t)0x00E60000L)

/* MessageId: HIL_COMPONENT_ID_PLV3_COMMON */
/* MessageText: Ethernet PowerLink Common Task. */
#define HIL_COMPONENT_ID_PLV3_COMMON     ((uint32_t)0x00E70000L)

/* MessageId: HIL_COMPONENT_ID_PLSV3_DRVETH */
/* MessageText: Ethernet Powerlink DrvEth Adapter Task. */
#define HIL_COMPONENT_ID_PLSV3_DRVETH    ((uint32_t)0x00E80000L)

/***********************************************************************************/
/* PowerLink PDO task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_EPL_PDO */
/* MessageText: Ethernet PowerLink PDO Task. */
#define HIL_COMPONENT_ID_EPL_PDO         ((uint32_t)0x00150000L)

/***********************************************************************************/
/* PowerLink SDO task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_EPL_SDO */
/* MessageText: Ethernet PowerLink SDO Task. */
#define HIL_COMPONENT_ID_EPL_SDO         ((uint32_t)0x00160000L)

/***********************************************************************************/
/* PowerLink MN task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_EPL_MN */
/* MessageText: Ethernet PowerLink MN Task. */
#define HIL_COMPONENT_ID_EPL_MN          ((uint32_t)0x003D0000L)

/***********************************************************************************/
/* AS-Interface task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ASI_ECTRL */
/* MessageText: AS-Interface ECTRL task (ASi stack). */
#define HIL_COMPONENT_ID_ASI_ECTRL       ((uint32_t)0x00320000L)

/* MessageId: HIL_COMPONENT_ID_ASI_AP */
/* MessageText: AS-Interface Application task (example). */
#define HIL_COMPONENT_ID_ASI_AP          ((uint32_t)0x00000002L)

/***********************************************************************************/
/* TCPUDP task identifiers (TCP/IP stack) */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_TCPUDP */
/* MessageText: TCPUDP task (TCP/IP stack). */
#define HIL_COMPONENT_ID_TCPUDP          ((uint32_t)0x00080000L)

/* MessageId: HIL_COMPONENT_ID_TCPIP_AP */
/* MessageText: TCP/IP stack Application task. */
#define HIL_COMPONENT_ID_TCPIP_AP        ((uint32_t)0x00000001L)

/* MessageId: HIL_COMPONENT_ID_TCPIP_SOCKIF */
/* MessageText: TCP/IP Socket Interface. */
#define HIL_COMPONENT_ID_TCPIP_SOCKIF    ((uint32_t)0x00740002L)

/***********************************************************************************/
/* Sercos task identifiers (Slave) */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SERCOSIII_API */
/* MessageText: Sercos API task. */
#define HIL_COMPONENT_ID_SERCOSIII_API   ((uint32_t)0x00340000L)

/* MessageId: HIL_COMPONENT_ID_SERCOSIII_DL */
/* MessageText: SERCOSIII DL task. */
#define HIL_COMPONENT_ID_SERCOSIII_DL    ((uint32_t)0x00350000L)

/* MessageId: HIL_COMPONENT_ID_SERCOSIII_SVC */
/* MessageText: SERCOSIII SVC task. */
#define HIL_COMPONENT_ID_SERCOSIII_SVC   ((uint32_t)0x00330000L)

/* MessageId: HIL_COMPONENT_ID_SERCOSIII_ETH */
/* MessageText: SERCOSIII ETH task. */
#define HIL_COMPONENT_ID_SERCOSIII_ETH   ((uint32_t)0x00360000L)

/* MessageId: HIL_COMPONENT_ID_SERCOSIII_NRT */
/* MessageText: SERCOSIII NRT task. */
#define HIL_COMPONENT_ID_SERCOSIII_NRT   ((uint32_t)0x00360001L)

/* MessageId: HIL_COMPONENT_ID_SERCOSIII_CYCLIC */
/* MessageText: SERCOSIII cyclic task. */
#define HIL_COMPONENT_ID_SERCOSIII_CYCLIC ((uint32_t)0x00370000L)

/***********************************************************************************/
/* PROFIBUS task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_PROFIBUS_DL */
/* MessageText: PROFIBUS Data Link Layer Task. */
#define HIL_COMPONENT_ID_PROFIBUS_DL     ((uint32_t)0x00060000L)

/* MessageId: HIL_COMPONENT_ID_PROFIBUS_FSPMS */
/* MessageText: PROFIBUS Slave Fieldbus Service Protocol Machine Task. */
#define HIL_COMPONENT_ID_PROFIBUS_FSPMS  ((uint32_t)0x00090000L)

/* MessageId: HIL_COMPONENT_ID_PROFIBUS_APS */
/* MessageText: PROFIBUS Slave Application Task. */
#define HIL_COMPONENT_ID_PROFIBUS_APS    ((uint32_t)0x001D0000L)

/***********************************************************************************/
/* PROFIBUS Master task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_PROFIBUS_FSPMM */
/* MessageText: PROFIBUS Master Fieldbus Service Protocol Machine Task. */
#define HIL_COMPONENT_ID_PROFIBUS_FSPMM  ((uint32_t)0x00380000L)

/* MessageId: HIL_COMPONENT_ID_PROFIBUS_APM */
/* MessageText: PROFIBUS Master Application Task. */
#define HIL_COMPONENT_ID_PROFIBUS_APM    ((uint32_t)0x00390000L)

/***********************************************************************************/
/* PROFIBUS Master task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SNMP_SERVER */
/* MessageText: SNMP Server Task. */
#define HIL_COMPONENT_ID_SNMP_SERVER     ((uint32_t)0x003B0000L)

/* MessageId: HIL_COMPONENT_ID_MIB_DATABASE */
/* MessageText: MIB Database for SNMP and LLDP. */
#define HIL_COMPONENT_ID_MIB_DATABASE    ((uint32_t)0x003C0000L)

/* MessageId: HIL_COMPONENT_ID_ICONL_TIMER */
/* MessageText: Icon-L Timer Task for iCon-L@netX. */
#define HIL_COMPONENT_ID_ICONL_TIMER     ((uint32_t)0x002A0000L)

/* MessageId: HIL_COMPONENT_ID_ICONL_RUN */
/* MessageText: Icon-L Run Task for iCon-L@netX. */
#define HIL_COMPONENT_ID_ICONL_RUN       ((uint32_t)0x00290000L)

/* MessageId: HIL_COMPONENT_ID_LLDP */
/* MessageText: LLDP protocol task */
#define HIL_COMPONENT_ID_LLDP            ((uint32_t)0x003E0000L)

/***********************************************************************************/
/* CAN task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CAN_DL */
/* MessageText: CAN DL Task (Data Link Layer). */
#define HIL_COMPONENT_ID_CAN_DL          ((uint32_t)0x003F0000L)

/* MessageId: HIL_COMPONENT_ID_CAN_DL_GCI_ADAPTER */
/* MessageText: CAN DL GCI-Adapter. */
#define HIL_COMPONENT_ID_CAN_DL_GCI_ADAPTER ((uint32_t)0x003F0001L)

/***********************************************************************************/
/* DDL task identifiers (J060219 Bosch Rexroth DDL mit netX) */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_DDL_ENPDDL */
/* MessageText: ENPDDL gateway task. */
#define HIL_COMPONENT_ID_DDL_ENPDDL      ((uint32_t)0x00400000L)

/* MessageId: HIL_COMPONENT_ID_DDL_DDL */
/* MessageText: DDL protocol task. */
#define HIL_COMPONENT_ID_DDL_DDL         ((uint32_t)0x00410000L)

/***********************************************************************************/
/* CANopen Master task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CANOPEN_MASTER */
/* MessageText: CANopen Master task (CANopen Master stack). */
#define HIL_COMPONENT_ID_CANOPEN_MASTER  ((uint32_t)0x00420000L)

/***********************************************************************************/
/* CANopen Slave task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CANOPEN_SLAVE */
/* MessageText: CANopen Slave task (CANopen Slave stack). */
#define HIL_COMPONENT_ID_CANOPEN_SLAVE   ((uint32_t)0x00430000L)

/***********************************************************************************/
/* UsbTLRRouter identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_USB_TLRROUTER */
/* MessageText: Usb-TLR-Router Task (Usb-TLRRouter). */
#define HIL_COMPONENT_ID_USB_TLRROUTER   ((uint32_t)0x00440000L)

/***********************************************************************************/
/* CAN DL AP sample identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CANDL_APSAMPLE */
/* MessageText: CAN DL application sample Task. */
#define HIL_COMPONENT_ID_CANDL_APSAMPLE  ((uint32_t)0x00460000L)

/***********************************************************************************/
/* DeviceNet FAL (Fieldbus Application Layer) task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_DEVNET_FAL */
/* MessageText: DeviceNet FAL Task (Fieldbus Application Layer). */
#define HIL_COMPONENT_ID_DEVNET_FAL      ((uint32_t)0x00470000L)

/***********************************************************************************/
/* DeviceNet AP task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_DEVNET_AP */
/* MessageText: DeviceNet AP Task (Application). */
#define HIL_COMPONENT_ID_DEVNET_AP       ((uint32_t)0x005B0000L)

/***********************************************************************************/
/* ObjectDictionary V2 DPM Adapter */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_DPM_OD2 */
/* MessageText: Object Dictionary DPM-Adapter Task. */
#define HIL_COMPONENT_ID_DPM_OD2         ((uint32_t)0x00480000L)

/***********************************************************************************/
/* CANopen Master application task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CANOPEN_APM */
/* MessageText: CANopen Master Application Task. */
#define HIL_COMPONENT_ID_CANOPEN_APM     ((uint32_t)0x00490000L)

/***********************************************************************************/
/* CANopen Slave application task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CANOPEN_APS */
/* MessageText: CANopen Slave Application Task. */
#define HIL_COMPONENT_ID_CANOPEN_APS     ((uint32_t)0x004A0000L)

/***********************************************************************************/
/* Sercos3 Slave application task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SERCOS_SL */
/* MessageText: Sercos3 Slave Task. */
#define HIL_COMPONENT_ID_SERCOS_SL       ((uint32_t)0x004B0000L)

/***********************************************************************************/
/* ECAT Slave DPM application task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ECAT_DPM */
/* MessageText: EtherCAT Slave DPM Task. */
#define HIL_COMPONENT_ID_ECAT_DPM        ((uint32_t)0x004C0000L)

/***********************************************************************************/
/* ECAT Slave InxAp application task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ECAT_INXAP */
/* MessageText: EtherCAT Slave InxAp Task. */
#define HIL_COMPONENT_ID_ECAT_INXAP      ((uint32_t)0x004C0001L)

/***********************************************************************************/
/* Sercos Slave Communication task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SERCOSIII_SL_COM */
/* MessageText: Sercos Slave Communication Task. */
#define HIL_COMPONENT_ID_SERCOSIII_SL_COM ((uint32_t)0x004E0000L)

/***********************************************************************************/
/* Sercos Slave Sercice Channel task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SERCOSIII_SL_SVC */
/* MessageText: Sercos Slave Service Channel Task. */
#define HIL_COMPONENT_ID_SERCOSIII_SL_SVC ((uint32_t)0x004F0000L)

/***********************************************************************************/
/* Sercos Slave Real Time Data task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SERCOSIII_SL_RTD */
/* MessageText: Sercos Slave Real Time Data Task. */
#define HIL_COMPONENT_ID_SERCOSIII_SL_RTD ((uint32_t)0x00500000L)

/***********************************************************************************/
/* Sercos Slave Application task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SERCOSIII_SL_AP */
/* MessageText: Sercos Slave Application Task. */
#define HIL_COMPONENT_ID_SERCOSIII_SL_AP ((uint32_t)0x00510000L)

/***********************************************************************************/
/* Sercos Slave IDN task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SERCOSIII_SL_IDN */
/* MessageText: Sercos Slave IDN Task. */
#define HIL_COMPONENT_ID_SERCOSIII_SL_IDN ((uint32_t)0x00850000L)

/***********************************************************************************/
/* Ethernet/Ip Adapter Application task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_EIP_APS */
/* MessageText: EtherNet/Ip Adapter Application-Task. */
#define HIL_COMPONENT_ID_EIP_APS         ((uint32_t)0x00590000L)

/***********************************************************************************/
/* Ethernet/Ip Scanner Application task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_EIP_APM */
/* MessageText: EtherNet/Ip Scanner Application-Task. */
#define HIL_COMPONENT_ID_EIP_APM         ((uint32_t)0x005A0000L)

/***********************************************************************************/
/* Ethernet Interface Task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ETH_INTF */
/* MessageText: Ethernet Interface Task. */
#define HIL_COMPONENT_ID_ETH_INTF        ((uint32_t)0x005D0000L)

/***********************************************************************************/
/* MID Startup Task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_MID_STARTUP */
/* MessageText: MID Startup Task. */
#define HIL_COMPONENT_ID_MID_STARTUP     ((uint32_t)0x005F0000L)

/***********************************************************************************/
/* OMB task identifiers (Open Modbus TCP) */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_OMB_OMBTASK */
/* MessageText: OMB task (Open Modbus TCP). */
#define HIL_COMPONENT_ID_OMB_OMBTASK     ((uint32_t)0x00600000L)

/* MessageId: HIL_COMPONENT_ID_OMB_OMBAPTASK */
/* MessageText: OMB task (Open Modbus TCP) Application task. */
#define HIL_COMPONENT_ID_OMB_OMBAPTASK   ((uint32_t)0x00610001L)

/***********************************************************************************/
/* DNS task identifiers (DeviceNet Slave) */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_DNS_FAL */
/* MessageText: DeviceNet Slave FAL Task (Fieldbus Application Layer). */
#define HIL_COMPONENT_ID_DNS_FAL         ((uint32_t)0x00620000L)

/* MessageId: HIL_COMPONENT_ID_DNS_AP */
/* MessageText: DeviceNet Slave AP Task (Dualport Application). */
#define HIL_COMPONENT_ID_DNS_AP          ((uint32_t)0x00630000L)

/***********************************************************************************/
/* EtherCAT Master AP Task identifiers (EtherCAT Master AP Task) */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ETHERCAT_MASTER_AP */
/* MessageText: EtherCAT Master AP Task. */
#define HIL_COMPONENT_ID_ETHERCAT_MASTER_AP ((uint32_t)0x00640000L)

/* MessageId: HIL_COMPONENT_ID_ETHERCAT_MASTER */
/* MessageText: EtherCAT Master Stack Task. */
#define HIL_COMPONENT_ID_ETHERCAT_MASTER ((uint32_t)0x00650000L)

/***********************************************************************************/
/* Ethernet Analyzer Transfer Task identifiers (Ethernet Analyzer Transfer Task) */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_AN_TRANS */
/* MessageText: Ethernet Analyzer Transfer Task. */
#define HIL_COMPONENT_ID_AN_TRANS        ((uint32_t)0x00660000L)

/***********************************************************************************/
/* PROFIBUS MPI */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_PROFIBUS_MPI */
/* MessageText: PROFIBUS MPI Task. */
#define HIL_COMPONENT_ID_PROFIBUS_MPI    ((uint32_t)0x00670000L)

/* MessageId: HIL_COMPONENT_ID_PROFIBUS_MPI_AP */
/* MessageText: PROFIBUS MPI Application Task. */
#define HIL_COMPONENT_ID_PROFIBUS_MPI_AP ((uint32_t)0x00680000L)

/* MessageId: HIL_COMPONENT_ID_PROFIBUS_MPI_RFC */
/* MessageText: PROFIBUS MPI Task. */
#define HIL_COMPONENT_ID_PROFIBUS_MPI_RFC ((uint32_t)0x00730000L)

/***********************************************************************************/
/* PROFIBUS FSPMM2 */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_PROFIBUS_FSPMM2 */
/* MessageText: PROFIBUS FSPMM2 Task. */
#define HIL_COMPONENT_ID_PROFIBUS_FSPMM2 ((uint32_t)0x00690000L)

/***********************************************************************************/
/* CC-Link Slave */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CCLINK_SLAVE */
/* MessageText: CC-Link Slave Task. */
#define HIL_COMPONENT_ID_CCLINK_SLAVE    ((uint32_t)0x006A0000L)

/* MessageId: HIL_COMPONENT_ID_CCLINK_APS */
/* MessageText: CC-Link Slave Application Task. */
#define HIL_COMPONENT_ID_CCLINK_APS      ((uint32_t)0x006B0000L)

/***********************************************************************************/
/* Lenze EtherCAT Slave Anbindung */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ECS_LENZE_MAIN */
/* MessageText: Lenze EtherCAT slave interface for 9400, main task. */
#define HIL_COMPONENT_ID_ECS_LENZE_MAIN  ((uint32_t)0x006C0000L)

/* MessageId: HIL_COMPONENT_ID_ECS_LENZE_BRIDGE */
/* MessageText: Lenze EtherCAT slave interface for 9400, bus bridge. */
#define HIL_COMPONENT_ID_ECS_LENZE_BRIDGE ((uint32_t)0x006C0001L)

/***********************************************************************************/
/* IO-Link Master Task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_IOLINK_MASTER */
/* MessageText: IO-Link Master Task. */
#define HIL_COMPONENT_ID_IOLINK_MASTER   ((uint32_t)0x006D0000L)

/* MessageId: HIL_COMPONENT_ID_IOLINK_APPLICATION */
/* MessageText: IO-Link Application and Application Layer Task. */
#define HIL_COMPONENT_ID_IOLINK_APPLICATION ((uint32_t)0x00A50000L)

/* MessageId: HIL_COMPONENT_ID_IOLINK_TMG_IO */
/* MessageText: Task to communicate with the IO-Link Master with TMG IO protocol. */
#define HIL_COMPONENT_ID_IOLINK_TMG_IO   ((uint32_t)0x00A50001L)

/* MessageId: HIL_COMPONENT_ID_IOLINK_TEST_PROTOCOL */
/* MessageText: Task to access the IO-Link master with test protocol. */
#define HIL_COMPONENT_ID_IOLINK_TEST_PROTOCOL ((uint32_t)0x00A50002L)

/***********************************************************************************/
/* MODBUS RTU */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_MODBUS_RTU */
/* MessageText: MODBUS RTU Task. */
#define HIL_COMPONENT_ID_MODBUS_RTU      ((uint32_t)0x006E0000L)

/***********************************************************************************/
/* MODBUS RTU AP */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_MODBUS_RTU_AP */
/* MessageText: MODBUS RTU Application Task. */
#define HIL_COMPONENT_ID_MODBUS_RTU_AP   ((uint32_t)0x006F0000L)

/***********************************************************************************/
/* Sercos Master stack task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SIII_MA_CP */
/* MessageText: Sercos Communication Phase Task */
#define HIL_COMPONENT_ID_SIII_MA_CP      ((uint32_t)0x00700000L)

/***********************************************************************************/
/* Sercos Master stack task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SIII_MA_AP */
/* MessageText: Sercos DPM AP Task */
#define HIL_COMPONENT_ID_SIII_MA_AP      ((uint32_t)0x00720000L)

/***********************************************************************************/
/* Sercos Master stack task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SERCOSIIIMASTER_SVC */
/* MessageText: Sercos Service Channel Task */
#define HIL_COMPONENT_ID_SERCOSIIIMASTER_SVC ((uint32_t)0x00710000L)

/***********************************************************************************/
/* Sercos Master stack task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SERCOSIIIMASTER_ACFG */
/* MessageText: Sercos Auto-Configure Task */
#define HIL_COMPONENT_ID_SERCOSIIIMASTER_ACFG ((uint32_t)0x00B70000L)

/***********************************************************************************/
/* Sercos Master stack task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SERCOSIIIMASTER_SIP_CLIENT */
/* MessageText: Sercos S/IP Client Task */
#define HIL_COMPONENT_ID_SERCOSIIIMASTER_SIP_CLIENT ((uint32_t)0x00BA0000L)

/* MessageId: HIL_COMPONENT_ID_SERCOSIIIMASTER_SIP_SERVER */
/* MessageText: Sercos S/IP Server Task */
#define HIL_COMPONENT_ID_SERCOSIIIMASTER_SIP_SERVER ((uint32_t)0x00BA0001L)

/***********************************************************************************/
/* SSIO task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SSIO */
/* MessageText: Serial Shift IO Task */
#define HIL_COMPONENT_ID_SSIO            ((uint32_t)0x00750000L)

/***********************************************************************************/
/* SSIO AP task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SSIO_AP */
/* MessageText: Serial Shift IO Application Task */
#define HIL_COMPONENT_ID_SSIO_AP         ((uint32_t)0x00760000L)

/***********************************************************************************/
/* MAP NIC task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_MEMORY_MAP */
/* MessageText: netIC Mapping Task */
#define HIL_COMPONENT_ID_MEMORY_MAP      ((uint32_t)0x00770000L)

/***********************************************************************************/
/* MPI Gateway task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_MPI_GATEWAY */
/* MessageText: MPI Gateway Task */
#define HIL_COMPONENT_ID_MPI_GATEWAY     ((uint32_t)0x00780000L)

/***********************************************************************************/
/* Sercos Master stack NRT task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SERCOSIIIMASTER_NRT */
/* MessageText: Sercos NRT Channel Task */
#define HIL_COMPONENT_ID_SERCOSIIIMASTER_NRT ((uint32_t)0x00790000L)

/***********************************************************************************/
/* AS-Interface Master task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ASI_MASTER */
/* MessageText: AS-Interface Master Task (AS-Interface Master stack). */
#define HIL_COMPONENT_ID_ASI_MASTER      ((uint32_t)0x007A0000L)

/***********************************************************************************/
/* AS-Interface Master application task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ASI_APM */
/* MessageText: AS-Interface Master Application Task. */
#define HIL_COMPONENT_ID_ASI_APM         ((uint32_t)0x007B0000L)

/***********************************************************************************/
/* CompoNet Slave task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_COMPONET_SLAVE */
/* MessageText: CompoNet Slave Task. */
#define HIL_COMPONENT_ID_COMPONET_SLAVE  ((uint32_t)0x007C0000L)

/* MessageId: HIL_COMPONENT_ID_COMPONET_SLAVE_AP */
/* MessageText: CompoNet Slave Application Task. */
#define HIL_COMPONENT_ID_COMPONET_SLAVE_AP ((uint32_t)0x007D0000L)

/***********************************************************************************/
/* ASCII task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ASCII */
/* MessageText: ASCII Protocol Task. */
#define HIL_COMPONENT_ID_ASCII           ((uint32_t)0x007E0000L)

/* MessageId: HIL_COMPONENT_ID_ASCII_AP */
/* MessageText: ASCII Application Task. */
#define HIL_COMPONENT_ID_ASCII_AP        ((uint32_t)0x007F0000L)

/***********************************************************************************/
/* netscript task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NETSCRIPT */
/* MessageText: netScript Task. */
#define HIL_COMPONENT_ID_NETSCRIPT       ((uint32_t)0x00800000L)

/***********************************************************************************/
/* Marshaller task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_MARSHALLER */
/* MessageText: Marshaller: Main Task. */
#define HIL_COMPONENT_ID_MARSHALLER      ((uint32_t)0x00820000L)

/* MessageId: HIL_COMPONENT_ID_PACKET_ROUTER */
/* MessageText: Marshaller: Packet Router Task. */
#define HIL_COMPONENT_ID_PACKET_ROUTER   ((uint32_t)0x00830000L)

/* MessageId: HIL_COMPONENT_ID_TCP_CONNECTOR */
/* MessageText: Marshaller: TCP Connection Task. */
#define HIL_COMPONENT_ID_TCP_CONNECTOR   ((uint32_t)0x00860000L)

/***********************************************************************************/
/* netTAP 100 Gateway task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NT100_GATEWAY */
/* MessageText: netTAP Gateway Task. */
#define HIL_COMPONENT_ID_NT100_GATEWAY   ((uint32_t)0x00840000L)

/***********************************************************************************/
/* Item Server task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ITEM_SERVER */
/* MessageText: Item Server Task. */
#define HIL_COMPONENT_ID_ITEM_SERVER     ((uint32_t)0x00870000L)

/***********************************************************************************/
/* ISaGRAF task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ISAGRAF_LOG */
/* MessageText: telnet task for ISaGRAF log. */
#define HIL_COMPONENT_ID_ISAGRAF_LOG     ((uint32_t)0x008C0000L)

/* MessageId: HIL_COMPONENT_ID_ISAGRAF_VM */
/* MessageText: Virtual Machine ISaGRAF task. */
#define HIL_COMPONENT_ID_ISAGRAF_VM      ((uint32_t)0x008B0000L)

/* MessageId: HIL_COMPONENT_ID_ISAGRAF */
/* MessageText: ISaGRAF command task. */
#define HIL_COMPONENT_ID_ISAGRAF         ((uint32_t)0x008A0000L)

/* MessageId: HIL_COMPONENT_ID_ISAGRAF_ETCP */
/* MessageText: Ethernet ISaGRAF interface. */
#define HIL_COMPONENT_ID_ISAGRAF_ETCP    ((uint32_t)0x00890000L)

/* MessageId: HIL_COMPONENT_ID_ISAGRAF_ISARSI */
/* MessageText: RS232 ISaGRAF interface. */
#define HIL_COMPONENT_ID_ISAGRAF_ISARSI  ((uint32_t)0x00880000L)

/* MessageId: HIL_COMPONENT_ID_ISAGRAF_MAINTENANCE */
/* MessageText: ISaGRAF maintenance task. */
#define HIL_COMPONENT_ID_ISAGRAF_MAINTENANCE ((uint32_t)0x008A0001L)

/* MessageId: HIL_COMPONENT_ID_ISAGRAF_NTP */
/* MessageText: ISaGRAF NTP task. */
#define HIL_COMPONENT_ID_ISAGRAF_NTP     ((uint32_t)0x008A0002L)

/* MessageId: HIL_COMPONENT_ID_ISAGRAF_AP */
/* MessageText: ISaGRAF AP task. */
#define HIL_COMPONENT_ID_ISAGRAF_AP      ((uint32_t)0x008A0003L)

/* MessageId: HIL_COMPONENT_ID_ISAGRAF_FTP */
/* MessageText: ISaGRAF FTP task. */
#define HIL_COMPONENT_ID_ISAGRAF_FTP     ((uint32_t)0x008A0004L)

/* MessageId: HIL_COMPONENT_ID_ISAGRAF_DS */
/* MessageText: ISaGRAF DS task. */
#define HIL_COMPONENT_ID_ISAGRAF_DS      ((uint32_t)0x008A0005L)

/***********************************************************************************/
/* DF1 task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_DF1 */
/* MessageText: DF1 Protocol Task. */
#define HIL_COMPONENT_ID_DF1             ((uint32_t)0x008D0000L)

/* MessageId: HIL_COMPONENT_ID_DF1_AP */
/* MessageText: DF1 Application Task. */
#define HIL_COMPONENT_ID_DF1_AP          ((uint32_t)0x008E0000L)

/***********************************************************************************/
/* 3964R task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_3964R */
/* MessageText: 3964R Protocol Task. */
#define HIL_COMPONENT_ID_3964R           ((uint32_t)0x008F0000L)

/* MessageId: HIL_COMPONENT_ID_3964R_AP */
/* MessageText: 3964R Application Task. */
#define HIL_COMPONENT_ID_3964R_AP        ((uint32_t)0x00900000L)

/***********************************************************************************/
/* IO Signals task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_IO_SIGNAL */
/* MessageText: IO Signal Task. */
#define HIL_COMPONENT_ID_IO_SIGNAL       ((uint32_t)0x00910000L)

/***********************************************************************************/
/* ServX task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SERVX */
/* MessageText: servX HTTP Server. */
#define HIL_COMPONENT_ID_SERVX           ((uint32_t)0x00920000L)

/***********************************************************************************/
/* TCPIP Application task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_TCPIP_TCP_AP */
/* MessageText: TCPUDP application task (TCP/IP stack). */
#define HIL_COMPONENT_ID_TCPIP_TCP_AP    ((uint32_t)0x00940000L)

/***********************************************************************************/
/* DLR task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_EIP_DLR */
/* MessageText: EthernetIP DLR task. */
#define HIL_COMPONENT_ID_EIP_DLR         ((uint32_t)0x00950000L)

/***********************************************************************************/
/* FODMI task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_FODMI */
/* MessageText: Fibre optic diagnostic monitoring interface task. */
#define HIL_COMPONENT_ID_FODMI           ((uint32_t)0x00960000L)

/***********************************************************************************/
/* PROFIDRIVE task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_PROFIDRIVE */
/* MessageText: PROFIDRIVE task. */
#define HIL_COMPONENT_ID_PROFIDRIVE      ((uint32_t)0x00970000L)

/***********************************************************************************/
/*****/
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_PROFIDRIVE_PA */
/* MessageText: PROFIDRIVE parameter Access interface task. */
#define HIL_COMPONENT_ID_PROFIDRIVE_PA   ((uint32_t)0x00980000L)

/***********************************************************************************/
/*****/
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_PROFIDRIVE_AP */
/* MessageText: PROFIDRIVE user application task. */
#define HIL_COMPONENT_ID_PROFIDRIVE_AP   ((uint32_t)0x009A0000L)

/***********************************************************************************/
/*****/
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_PROFIDRIVE_OD */
/* MessageText: PROFIDRIVE Object Dictionary task. */
#define HIL_COMPONENT_ID_PROFIDRIVE_OD   ((uint32_t)0x00990000L)

/***********************************************************************************/
/* CANopen Object Dictionary */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ODV3 */
/* MessageText: CANopen Object Dictionary task. */
#define HIL_COMPONENT_ID_ODV3            ((uint32_t)0x009B0000L)

/***********************************************************************************/
/* VARAN Client Application */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_VARAN_CLIENT_AP */
/* MessageText: VARAN Client application task. */
#define HIL_COMPONENT_ID_VARAN_CLIENT_AP ((uint32_t)0x009D0000L)

/***********************************************************************************/
/* VARAN Client */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_VARAN_CLIENT */
/* MessageText: VARAN Client task. */
#define HIL_COMPONENT_ID_VARAN_CLIENT    ((uint32_t)0x009C0000L)

/***********************************************************************************/
/* Modbus RTU Peripheral Task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_MODBUS_RTU_PERIPH */
/* MessageText: Modbus RTU Peripheral Task. */
#define HIL_COMPONENT_ID_MODBUS_RTU_PERIPH ((uint32_t)0x009E0000L)

/***********************************************************************************/
/* PROFINET RTA Task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_PROFINET_RTA */
/* MessageText: PROFINET RTA (RealTime Acyclic) Task. */
#define HIL_COMPONENT_ID_PROFINET_RTA    ((uint32_t)0x009F0000L)

/***********************************************************************************/
/* Sercos Internet Protocol Service(SIP) task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SIII_SIP */
/* MessageText: Sercos Internet Protocol Service Task. */
#define HIL_COMPONENT_ID_SIII_SIP        ((uint32_t)0x00A00000L)

/***********************************************************************************/
/* 3S CodeSys PLC Handler task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_3S_PLC_HANDLER */
/* MessageText: 3S CodeSys PLC Handler Task. */
#define HIL_COMPONENT_ID_3S_PLC_HANDLER  ((uint32_t)0x00A10000L)

/***********************************************************************************/
/* 3S CodeSys PLC Handler AP task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_3S_PLC_HANDLER_AP */
/* MessageText: 3S CodeSys PLC Handler AP Task. */
#define HIL_COMPONENT_ID_3S_PLC_HANDLER_AP ((uint32_t)0x00A20000L)

/***********************************************************************************/
/* netPLC I/O Handler task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NETPLC_IO_HANDLER */
/* MessageText: netPLC I/O Handler. */
#define HIL_COMPONENT_ID_NETPLC_IO_HANDLER ((uint32_t)0x00A30000L)

/***********************************************************************************/
/* PowerLink MN task identifiers */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_POWERLINK_MN_TASK */
/* MessageText: PowerLink MN Task */
#define HIL_COMPONENT_ID_POWERLINK_MN_TASK ((uint32_t)0x00A40000L)

/* MessageId: HIL_COMPONENT_ID_POWERLINK_MN_AP_TASK */
/* MessageText: PowerLink MN AP Task */
#define HIL_COMPONENT_ID_POWERLINK_MN_AP_TASK ((uint32_t)0x00A40001L)

/* MessageId: HIL_COMPONENT_ID_POWERLINK_MN_NEW_DATA_TASK */
/* MessageText: PowerLink MN NewData Task */
#define HIL_COMPONENT_ID_POWERLINK_MN_NEW_DATA_TASK ((uint32_t)0x00A40002L)

/* MessageId: HIL_COMPONENT_ID_POWERLINK_MN_SDO_UDP_SOCK_TASK */
/* MessageText: PowerLink MN SDO UDP Socket Task */
#define HIL_COMPONENT_ID_POWERLINK_MN_SDO_UDP_SOCK_TASK ((uint32_t)0x00A40003L)

/* MessageId: HIL_COMPONENT_ID_POWERLINK_MN_VETH_TASK */
/* MessageText: PowerLink MN VirtualEthernet Task */
#define HIL_COMPONENT_ID_POWERLINK_MN_VETH_TASK ((uint32_t)0x00A40004L)

/* MessageId: HIL_COMPONENT_ID_POWERLINK_MN_JOB_TOKEN_TASK */
/* MessageText: PowerLink MN JobToken Task */
#define HIL_COMPONENT_ID_POWERLINK_MN_JOB_TOKEN_TASK ((uint32_t)0x00A40005L)

/***********************************************************************************/
/* Recording task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_RECORDING */
/* MessageText: Recording and logging Task. */
#define HIL_COMPONENT_ID_RECORDING       ((uint32_t)0x00A60000L)

/***********************************************************************************/
/* Insight Debug task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_INSIGHT */
/* MessageText: Insight task. */
#define HIL_COMPONENT_ID_INSIGHT         ((uint32_t)0x00AC0000L)

/***********************************************************************************/
/* SmartWire Master Task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_SMARTWIRE_MASTER */
/* MessageText: SmartWire master task. */
#define HIL_COMPONENT_ID_SMARTWIRE_MASTER ((uint32_t)0x00AD0000L)

/***********************************************************************************/
/* EtherCAT Slave V4 Task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ECSV4_ESM */
/* MessageText: EtherCAT Slave V4 ESM Task. */
#define HIL_COMPONENT_ID_ECSV4_ESM       ((uint32_t)0x00AF0000L)

/* MessageId: HIL_COMPONENT_ID_ECSV4_MBX */
/* MessageText: EtherCAT Slave V4 MBX Task. */
#define HIL_COMPONENT_ID_ECSV4_MBX       ((uint32_t)0x00B00000L)

/* MessageId: HIL_COMPONENT_ID_ECSV4_COE */
/* MessageText: EtherCAT Slave V4 COE Task. */
#define HIL_COMPONENT_ID_ECSV4_COE       ((uint32_t)0x00B10000L)

/* MessageId: HIL_COMPONENT_ID_ECSV4_SDO */
/* MessageText: EtherCAT Slave V4 SDO Task. */
#define HIL_COMPONENT_ID_ECSV4_SDO       ((uint32_t)0x00B10001L)

/* MessageId: HIL_COMPONENT_ID_ECSV4_EOE */
/* MessageText: EtherCAT Slave V4 EOE Task. */
#define HIL_COMPONENT_ID_ECSV4_EOE       ((uint32_t)0x00B20000L)

/* MessageId: HIL_COMPONENT_ID_ECSV4_FOE */
/* MessageText: EtherCAT Slave V4 FOE Task. */
#define HIL_COMPONENT_ID_ECSV4_FOE       ((uint32_t)0x00B30000L)

/* MessageId: HIL_COMPONENT_ID_ECSV4_FOE_FH */
/* MessageText: EtherCAT Slave V4 FOE FH Task. */
#define HIL_COMPONENT_ID_ECSV4_FOE_FH    ((uint32_t)0x00C40000L)

/* MessageId: HIL_COMPONENT_ID_ECSV4_SOE_SSC */
/* MessageText: EtherCAT Slave V4 SOE_SSC Task. */
#define HIL_COMPONENT_ID_ECSV4_SOE_SSC   ((uint32_t)0x00B40000L)

/* MessageId: HIL_COMPONENT_ID_ECSV4_SOE_IDN */
/* MessageText: EtherCAT Slave V4 SOE_IDN Task. */
#define HIL_COMPONENT_ID_ECSV4_SOE_IDN   ((uint32_t)0x00B40001L)

/* MessageId: HIL_COMPONENT_ID_ECSV4_DPM */
/* MessageText: EtherCAT Slave V4 DPM Task. */
#define HIL_COMPONENT_ID_ECSV4_DPM       ((uint32_t)0x00AE0000L)

/* MessageId: HIL_COMPONENT_ID_NET_PROXY */
/* MessageText: netPROXY. */
#define HIL_COMPONENT_ID_NET_PROXY       ((uint32_t)0x00B50000L)

/* MessageId: HIL_COMPONENT_ID_ECSV4_AOE */
/* MessageText: EtherCAT Slave V4 AoE Task. */
#define HIL_COMPONENT_ID_ECSV4_AOE       ((uint32_t)0x00BB0000L)

/***********************************************************************************/
/* netSMART Gateway Task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NET_SMART_GATEWAY */
/* MessageText: netSMART Gateway. */
#define HIL_COMPONENT_ID_NET_SMART_GATEWAY ((uint32_t)0x00B60000L)

/***********************************************************************************/
/* TFTP Stack Task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_TFTP */
/* MessageText: TFTP Stack Task. */
#define HIL_COMPONENT_ID_TFTP            ((uint32_t)0x00B80000L)

/***********************************************************************************/
/* TFTP App Task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_TFTP_AP */
/* MessageText: TFTP Application Task. */
#define HIL_COMPONENT_ID_TFTP_AP         ((uint32_t)0x00B90000L)

/***********************************************************************************/
/* PTP Task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_PTP */
/* MessageText: PTP Task. */
#define HIL_COMPONENT_ID_PTP             ((uint32_t)0x00BD0000L)

/***********************************************************************************/
/* User Area */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_USER_AREA */
/* MessageText: User Area. */
#define HIL_COMPONENT_ID_USER_AREA       ((uint32_t)0x0FF00000L)

/***********************************************************************************/
/* User Area */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NETPROXY_S3S */
/* MessageText: S3S netProxy adaptation task */
#define HIL_COMPONENT_ID_NETPROXY_S3S    ((uint32_t)0x00BC0000L)

/***********************************************************************************/
/* EtherNet/IP DLR IRQ Task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_EIP_DLR_IRQ */
/* MessageText: PTP Task. */
#define HIL_COMPONENT_ID_EIP_DLR_IRQ     ((uint32_t)0x00BE0000L)

/***********************************************************************************/
/* FTP Server Stack Task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_FTP_SERVER */
/* MessageText: FTP Stack Task */
#define HIL_COMPONENT_ID_FTP_SERVER      ((uint32_t)0x00BF0000L)

/***********************************************************************************/
/* FTP Server Application Task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_FTP_SERVER_AP */
/* MessageText: FTP Application Task. */
#define HIL_COMPONENT_ID_FTP_SERVER_AP   ((uint32_t)0x00C00000L)

/***********************************************************************************/
/* netPROXY HIF Adapter Task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXHIFADAPTER */
/* MessageText: netPROXY HIF Adapter Task. */
#define HIL_COMPONENT_ID_NPXHIFADAPTER   ((uint32_t)0x00C20000L)

/***********************************************************************************/
/* netPROXY package EtherNetIP Slave */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_EIS */
/* MessageText: netPROXY package EtherNetIP Slave. */
#define HIL_COMPONENT_ID_NPXPACKAGE_EIS  ((uint32_t)0x00C30000L)

/***********************************************************************************/
/* netHost task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NETHOST */
/* MessageText: netHost Task. */
#define HIL_COMPONENT_ID_NETHOST         ((uint32_t)0x00C50000L)

/***********************************************************************************/
/* CIP Handler task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CIP_HANDLER */
/* MessageText: CIP Handler Task. */
#define HIL_COMPONENT_ID_CIP_HANDLER     ((uint32_t)0x00C60000L)

/***********************************************************************************/
/* CPX Driver task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CPX_DRIVER */
/* MessageText: CPX Driver Task. */
#define HIL_COMPONENT_ID_CPX_DRIVER      ((uint32_t)0x00C70000L)

/***********************************************************************************/
/* CPX Exchange task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CPX_EXCHANGE */
/* MessageText: CPX Exchange Task. */
#define HIL_COMPONENT_ID_CPX_EXCHANGE    ((uint32_t)0x00C80000L)

/***********************************************************************************/
/* OSAL */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CPX_OSAL */
/* MessageText: CPX Exchange Task. */
#define HIL_COMPONENT_ID_CPX_OSAL        ((uint32_t)0x00CA0000L)

/***********************************************************************************/
/* PROFINET IO CONTROLLER V3 */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_PNM_AP */
/* MessageText: PROFINET IO Controller V3 AP Task. */
#define HIL_COMPONENT_ID_PNM_AP          ((uint32_t)0x00CB0000L)

/***********************************************************************************/
/* ECMv4 */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ECMV4_AP */
/* MessageText: EtherCAT Master AP Task. */
#define HIL_COMPONENT_ID_ECMV4_AP        ((uint32_t)0x00D70000L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_IF */
/* MessageText: EtherCAT Master Interface Task. */
#define HIL_COMPONENT_ID_ECMV4_IF        ((uint32_t)0x00D60000L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_LLD_SCHED */
/* MessageText: LLD Scheduler. */
#define HIL_COMPONENT_ID_ECMV4_LLD_SCHED ((uint32_t)0x00CC0000L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_LLD_IF */
/* MessageText: LLD IF. */
#define HIL_COMPONENT_ID_ECMV4_LLD_IF    ((uint32_t)0x00CC0001L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_EMC */
/* MessageText: EtherCAT Master Control Task. */
#define HIL_COMPONENT_ID_ECMV4_EMC       ((uint32_t)0x00CD0000L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_EMC_LLD_IF */
/* MessageText: EtherCAT Master Control Task LLD IF. */
#define HIL_COMPONENT_ID_ECMV4_EMC_LLD_IF ((uint32_t)0x00CD0001L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_COE_SDOCLIENT */
/* MessageText: SDO Client Task. */
#define HIL_COMPONENT_ID_ECMV4_COE_SDOCLIENT ((uint32_t)0x00CF0000L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_COE_SDOINFOCLIENT */
/* MessageText: SDOINFO Client Task. */
#define HIL_COMPONENT_ID_ECMV4_COE_SDOINFOCLIENT ((uint32_t)0x00CF0001L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_COE_SDOSERVER */
/* MessageText: SDO Server Task. */
#define HIL_COMPONENT_ID_ECMV4_COE_SDOSERVER ((uint32_t)0x00CF0002L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_COE_SDOINFOSERVER */
/* MessageText: SDOINFO Server Task. */
#define HIL_COMPONENT_ID_ECMV4_COE_SDOINFOSERVER ((uint32_t)0x00CF0003L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_EOE */
/* MessageText: EoE Task. */
#define HIL_COMPONENT_ID_ECMV4_EOE       ((uint32_t)0x00D00000L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_EOE_IF */
/* MessageText: EoE Interface Task. */
#define HIL_COMPONENT_ID_ECMV4_EOE_IF    ((uint32_t)0x00D00001L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_SOE_CLIENT */
/* MessageText: SoE Client Task. */
#define HIL_COMPONENT_ID_ECMV4_SOE_CLIENT ((uint32_t)0x00D20000L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_FOE_CLIENT */
/* MessageText: FoE Client Task. */
#define HIL_COMPONENT_ID_ECMV4_FOE_CLIENT ((uint32_t)0x00D10000L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_AOE_ROUTER */
/* MessageText: AoE Router Task. */
#define HIL_COMPONENT_ID_ECMV4_AOE_ROUTER ((uint32_t)0x00CE0000L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_AOE_CLIENT */
/* MessageText: AoE Client Task. */
#define HIL_COMPONENT_ID_ECMV4_AOE_CLIENT ((uint32_t)0x00CE0001L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_MBXROUTER */
/* MessageText: MbxRouter Task. */
#define HIL_COMPONENT_ID_ECMV4_MBXROUTER ((uint32_t)0x00DD0000L)

/* MessageId: HIL_COMPONENT_ID_ECMV4_ACFG */
/* MessageText: ACfg Task. */
#define HIL_COMPONENT_ID_ECMV4_ACFG      ((uint32_t)0x00DE0000L)

/***********************************************************************************/
/* netPROXY package ODV3 */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_ODV3 */
/* MessageText: netPROXY package ODV3. */
#define HIL_COMPONENT_ID_NPXPACKAGE_ODV3 ((uint32_t)0x00D80000L)

/***********************************************************************************/
/* netPROXY package EtherCAT Slave */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_ECS */
/* MessageText: netPROXY package EtherCAT Slave. */
#define HIL_COMPONENT_ID_NPXPACKAGE_ECS  ((uint32_t)0x00D90000L)

/***********************************************************************************/
/* netPROXY package IO-Link Master */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_IOLM */
/* MessageText: netPROXY package IO-Link Master. */
#define HIL_COMPONENT_ID_NPXPACKAGE_IOLM ((uint32_t)0x00DA0000L)

/***********************************************************************************/
/* netPROXY Management task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXMGMT */
/* MessageText: netPROXY management task. */
#define HIL_COMPONENT_ID_NPXMGMT         ((uint32_t)0x00E00000L)

/***********************************************************************************/
/* Command Table task identifier */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CMD_TABLE */
/* MessageText: Command Table Task. */
#define HIL_COMPONENT_ID_CMD_TABLE       ((uint32_t)0x00E20000L)

/***********************************************************************************/
/* netTap DPM Bridge Task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NT_DPM_BRIDGE */
/* MessageText: netTap DPM Bridge Task. */
#define HIL_COMPONENT_ID_NT_DPM_BRIDGE   ((uint32_t)0x00E40000L)

/***********************************************************************************/
/* netPROXY LED Handler Task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXLEDHNDLR */
/* MessageText: netPROXY LED Handler Task. */
#define HIL_COMPONENT_ID_NPXLEDHNDLR     ((uint32_t)0x00E50000L)

/***********************************************************************************/
/* LWIP Task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_LWIP */
/* MessageText: LWIP Task. */
#define HIL_COMPONENT_ID_LWIP            ((uint32_t)0x00E90000L)

/* MessageId: HIL_COMPONENT_ID_LWIP_GCI_SOCKIF */
/* MessageText: LWIP packet socket API for GCI. */
#define HIL_COMPONENT_ID_LWIP_GCI_SOCKIF ((uint32_t)0x00E90001L)

/***********************************************************************************/
/* OSAL Worker Thread */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_WORKER_THREAD */
/* MessageText: OSAL Worker Thread. */
#define HIL_COMPONENT_ID_WORKER_THREAD   ((uint32_t)0x00EA0000L)

/***********************************************************************************/
/* netPROXY package PROFINET IO-Device V4 */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_PNS */
/* MessageText: netPROXY package PROFINET IO-Device. */
#define HIL_COMPONENT_ID_NPXPACKAGE_PNS  ((uint32_t)0x00EB0000L)

/***********************************************************************************/
/* Base Firmware Application Task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_BASEFW_AP */
/* MessageText: Base Firmware Application Task. */
#define HIL_COMPONENT_ID_BASEFW_AP       ((uint32_t)0x00EC0000L)

/***********************************************************************************/
/* Config Manager Task */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CONFIG_MANAGER */
/* MessageText: Config Manager Task. */
#define HIL_COMPONENT_ID_CONFIG_MANAGER  ((uint32_t)0x00ED0000L)

/***********************************************************************************/
/* netPROXY package Remanent */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_REMANENT */
/* MessageText: netPROXY package Remanent. */
#define HIL_COMPONENT_ID_NPXPACKAGE_REMANENT ((uint32_t)0x00F20000L)

/***********************************************************************************/
/* CC-Link IE Field Slave */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CCLIES */
/* MessageText: CC-Link IE Slave Task. */
#define HIL_COMPONENT_ID_CCLIES          ((uint32_t)0x00F30000L)

/* MessageId: HIL_COMPONENT_ID_CCLIES_IF */
/* MessageText: CC-Link IE Slave IF Task. */
#define HIL_COMPONENT_ID_CCLIES_IF       ((uint32_t)0x00F40000L)

/* MessageId: HIL_COMPONENT_ID_CCLIES_AP */
/* MessageText: CC-Link IE Slave AP Task. */
#define HIL_COMPONENT_ID_CCLIES_AP       ((uint32_t)0x00F50000L)

/***********************************************************************************/
/* Authentication manager */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_AUTH_MANAGER */
/* MessageText: Authentication manager component for user authentication. */
#define HIL_COMPONENT_ID_AUTH_MANAGER    ((uint32_t)0x00FB0000L)

/* MessageId: HIL_COMPONENT_ID_AUTH_MANAGER_X_509 */
/* MessageText: Authentication manager component for digital (X.509) certificates handling. */
#define HIL_COMPONENT_ID_AUTH_MANAGER_X_509 ((uint32_t)0x00FB0001L)

/***********************************************************************************/
/* CC-Link IE Field Basic */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_CCLIEFB */
/* MessageText: CC-Link IE Field Basic Slave Task. */
#define HIL_COMPONENT_ID_CCLIEFB         ((uint32_t)0x00FE0000L)

/* MessageId: HIL_COMPONENT_ID_CCLIEFBM */
/* MessageText: CC-Link IE Field Basic Master/Slave Task. */
#define HIL_COMPONENT_ID_CCLIEFBM        ((uint32_t)0x00FE0001L)

/* MessageId: HIL_COMPONENT_ID_CCLIEFB_IF */
/* MessageText: CC-Link IE Field Basic IF Task. */
#define HIL_COMPONENT_ID_CCLIEFB_IF      ((uint32_t)0x00FF0000L)

/* MessageId: HIL_COMPONENT_ID_CCLIEFB_AP */
/* MessageText: CC-Link IE Field Basic AP Task. */
#define HIL_COMPONENT_ID_CCLIEFB_AP      ((uint32_t)0x01000000L)

/***********************************************************************************/
/* netPROXY package MQTT Client */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_MQTT_CLIENT */
/* MessageText: netPROXY package MQTT Client. */
#define HIL_COMPONENT_ID_NPXPACKAGE_MQTT_CLIENT ((uint32_t)0x00F90000L)

/***********************************************************************************/
/* Generic AP */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_GENERIC_AP */
/* MessageText: Generic AP Task. */
#define HIL_COMPONENT_ID_GENERIC_AP      ((uint32_t)0x01090000L)

/***********************************************************************************/
/* netPROXY package OPCUA */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_OPCUA */
/* MessageText: netPROXY package OPC UA. */
#define HIL_COMPONENT_ID_NPXPACKAGE_OPCUA ((uint32_t)0x010A0000L)

/***********************************************************************************/
/* netPROXY package Profibus DP Slave */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_DPS */
/* MessageText: netPROXY package Profibus DP Slave. */
#define HIL_COMPONENT_ID_NPXPACKAGE_DPS  ((uint32_t)0x010C0000L)

/***********************************************************************************/
/* ECS V5 GCI */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_ECSV5_GCI */
/* MessageText: EtherCAT Slave V5 GCI. */
#define HIL_COMPONENT_ID_ECSV5_GCI       ((uint32_t)0x010D0000L)

/***********************************************************************************/
/* netPROXY package OPCUA to IO-Link Master */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_OPCUA_IOL */
/* MessageText: netPROXY package OPC UA to IO-Link Master. */
#define HIL_COMPONENT_ID_NPXPACKAGE_OPCUA_IOL ((uint32_t)0x010F0000L)

/***********************************************************************************/
/* netPROXY package OPCUA to netIOL */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_OPCUA_NETIOL */
/* MessageText: netPROXY package OPC UA to netIOL. */
#define HIL_COMPONENT_ID_NPXPACKAGE_OPCUA_NETIOL ((uint32_t)0x01100000L)

/***********************************************************************************/
/* netPROXY package netIOL */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_NETIOL */
/* MessageText: netPROXY package netIOL. */
#define HIL_COMPONENT_ID_NPXPACKAGE_NETIOL ((uint32_t)0x01130000L)

/***********************************************************************************/
/* MQTT Client Stack */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_MQTT_CLIENT */
/* MessageText: MQTT Client Stack. */
#define HIL_COMPONENT_ID_MQTT_CLIENT     ((uint32_t)0x01150000L)

/***********************************************************************************/
/* netPROXY package OPCUA TLV Parser */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_OPCUATLVPARSER */
/* MessageText: netPROXY package OPC UA TLV Parser. */
#define HIL_COMPONENT_ID_NPXPACKAGE_OPCUATLVPARSER ((uint32_t)0x01160000L)

/***********************************************************************************/
/* netPROXY package OPCUA AddOn */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_OPCUA_ADDON */
/* MessageText: netPROXY package OPC AddOn. */
#define HIL_COMPONENT_ID_NPXPACKAGE_OPCUA_ADDON ((uint32_t)0x01170000L)

/***********************************************************************************/
/* Driver Phy */
/***********************************************************************************/
/* MessageId: _HIL_UNQ_COMPONENT_ID_DRV_PHY */
/* MessageText: Driver Phy. */
#define _HIL_UNQ_COMPONENT_ID_DRV_PHY    ((uint32_t)0x01180000L)

/***********************************************************************************/
/* DeviceNet Core */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_DEVNET_CORE */
/* MessageText: DeviceNet Core. */
#define HIL_COMPONENT_ID_DEVNET_CORE     ((uint32_t)0x01190000L)

/***********************************************************************************/
/* DeviceNet Object Library */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_DEVNET_OBJECT */
/* MessageText: DeviceNet Object Library. */
#define HIL_COMPONENT_ID_DEVNET_OBJECT   ((uint32_t)0x011A0000L)

/***********************************************************************************/
/* DeviceNet GCI Adapter Slave */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_DEVNET_GCI_SLAVE */
/* MessageText: DeviceNet GCI Adapter Slave. */
#define HIL_COMPONENT_ID_DEVNET_GCI_SLAVE ((uint32_t)0x011B0000L)

/***********************************************************************************/
/* HTTP Web Server */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_HTTP_SERVER */
/* MessageText: HTTP and HTTPS Web Server. */
#define HIL_COMPONENT_ID_HTTP_SERVER     ((uint32_t)0x011C0000L)

/***********************************************************************************/
/* netPROXY package GCI */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_NPXPACKAGE_GCI */
/* MessageText: netPROXY package GCI. */
#define HIL_COMPONENT_ID_NPXPACKAGE_GCI  ((uint32_t)0x011D0000L)

/***********************************************************************************/
/* Protocol Detect */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_PDETECT */
/* MessageText: Protocol Detect. */
#define HIL_COMPONENT_ID_PDETECT         ((uint32_t)0x011F0000L)

/***********************************************************************************/
/* OpenModbus GCI Adapter */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_OPEN_MODBUS_GCI */
/* MessageText: OpenModbus GCI Adapter. */
#define HIL_COMPONENT_ID_OPEN_MODBUS_GCI ((uint32_t)0x01200000L)

/***********************************************************************************/
/* IEEE 802.1AS */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_IEEE802_1AS */
/* MessageText: IEEE 802.1AS. */
#define HIL_COMPONENT_ID_IEEE802_1AS     ((uint32_t)0x01210000L)

/***********************************************************************************/
/* TSN Core */
/***********************************************************************************/
/* MessageId: HIL_COMPONENT_ID_TSN_CORE */
/* MessageText: TSN Core. */
#define HIL_COMPONENT_ID_TSN_CORE        ((uint32_t)0x01220000L)

/************************************************************************************
  Deprecated task identifiers defines. Do not use them for new development
************************************************************************************/

#define HIL_TASK_ID_UNKNOWN_IDENTIFIER                HIL_COMPONENT_ID_UNKNOWN_IDENTIFIER
#define HIL_TASK_ID_TIMER                             HIL_COMPONENT_ID_TIMER
#define HIL_TASK_ID_ADMIN                             HIL_COMPONENT_ID_LIBSTORAGE
#define HIL_TASK_ID_MID_SYS                           HIL_COMPONENT_ID_MID_SYS
#define HIL_TASK_ID_MID_DBG                           HIL_COMPONENT_ID_MID_DBG
#define HIL_TASK_ID_RX_IDLE                           HIL_COMPONENT_ID_RX_IDLE
#define HIL_TASK_ID_IRQ_HANDLER                       HIL_COMPONENT_ID_IRQ_HANDLER
#define HIL_TASK_ID_IDLE                              HIL_COMPONENT_ID_IDLE
#define HIL_TASK_ID_BOOTUP                            HIL_COMPONENT_ID_BOOTUP
#define HIL_TASK_ID_RX_TIMER                          HIL_COMPONENT_ID_RX_TIMER
#define HIL_TASK_ID_ECAT                              HIL_COMPONENT_ID_ECAT
#define HIL_TASK_ID_ECAT_ESM                          HIL_COMPONENT_ID_ECAT_ESM
#define HIL_TASK_ID_ECAT_MBX                          HIL_COMPONENT_ID_ECAT_MBX
#define HIL_TASK_ID_ECAT_CYCLIC_IN                    HIL_COMPONENT_ID_ECAT_CYCLIC_IN
#define HIL_TASK_ID_ECAT_CYCLIC_OUT                   HIL_COMPONENT_ID_ECAT_CYCLIC_OUT
#define HIL_TASK_ID_ECAT_COE                          HIL_COMPONENT_ID_ECAT_COE
#define HIL_TASK_ID_ECAT_COE_PDO                      HIL_COMPONENT_ID_ECAT_COE_PDO
#define HIL_TASK_ID_ECAT_COE_SDO                      HIL_COMPONENT_ID_ECAT_COE_SDO
#define HIL_TASK_ID_ECAT_VOE                          HIL_COMPONENT_ID_ECAT_VOE
#define HIL_TASK_ID_ECAT_FOE                          HIL_COMPONENT_ID_ECAT_FOE
#define HIL_TASK_ID_ECAT_FOE_FH                       HIL_COMPONENT_ID_ECAT_FOE_FH
#define HIL_TASK_ID_ECAT_EOE                          HIL_COMPONENT_ID_ECAT_EOE
#define HIL_TASK_ID_ECAT_SOE_SSC                      HIL_COMPONENT_ID_ECAT_SOE_SSC
#define HIL_TASK_ID_ECAT_SOE_IDN                      HIL_COMPONENT_ID_ECAT_SOE_IDN
#define HIL_TASK_ID_EXAMPLE_TASK1                     HIL_COMPONENT_ID_EXAMPLE_TASK1
#define HIL_TASK_ID_EXAMPLE_TASK2                     HIL_COMPONENT_ID_EXAMPLE_TASK2
#define HIL_TASK_ID_EXAMPLE_TASK3                     HIL_COMPONENT_ID_EXAMPLE_TASK3
#define HIL_TASK_ID_EIP_ENCAP                         HIL_COMPONENT_ID_EIP_ENCAP
#define HIL_TASK_ID_EIP_CL1                           HIL_COMPONENT_ID_EIP_CL1
#define HIL_TASK_ID_EIP_OBJECT                        HIL_COMPONENT_ID_EIP_OBJECT
#define HIL_TASK_ID_EIP_EDD_LOW                       HIL_COMPONENT_ID_EIP_EDD_LOW
#define HIL_TASK_ID_EIP_EDD_HIGH                      HIL_COMPONENT_ID_EIP_EDD_HIGH
#define HIL_TASK_ID_PNIO_CMCTL                        HIL_COMPONENT_ID_PNIO_CMCTL
#define HIL_TASK_ID_PNIO_CMDEV                        HIL_COMPONENT_ID_PNIO_CMDEV
#define HIL_TASK_ID_PNIO_ACP                          HIL_COMPONENT_ID_PNIO_ACP
#define HIL_TASK_ID_PNIO_DCP                          HIL_COMPONENT_ID_PNIO_DCP
#define HIL_TASK_ID_PNIO_EDD                          HIL_COMPONENT_ID_PNIO_EDD
#define HIL_TASK_ID_PNIO_MGT                          HIL_COMPONENT_ID_PNIO_MGT
#define HIL_TASK_ID_PNIO_APCTL                        HIL_COMPONENT_ID_PNIO_APCTL
#define HIL_TASK_ID_PNIO_APDEV                        HIL_COMPONENT_ID_PNIO_APDEV
#define HIL_TASK_ID_PNIO_APCFG                        HIL_COMPONENT_ID_PNIO_APCFG
#define HIL_TASK_ID_PNS_IF                            HIL_COMPONENT_ID_PNS_IF
#define HIL_TASK_ID_PNIOD_16BITIO                     HIL_COMPONENT_ID_PNIOD_16BITIO
#define HIL_TASK_ID_PNS_32BITIO                       HIL_COMPONENT_ID_PNS_32BITIO
#define HIL_TASK_ID_PNS_4BITIO                        HIL_COMPONENT_ID_PNS_4BITIO
#define HIL_TASK_ID_PNS_SOCKET_SRV                    HIL_COMPONENT_ID_PNS_SOCKET_SRV
#define HIL_TASK_ID_PNS_EDD_HIGH                      HIL_COMPONENT_ID_PNS_EDD_HIGH
#define HIL_TASK_ID_PNS_EDD_LOW                       HIL_COMPONENT_ID_PNS_EDD_LOW
#define HIL_TASK_ID_PNS_SOCKET                        HIL_COMPONENT_ID_PNS_SOCKET
#define HIL_TASK_ID_PNS_DCP                           HIL_COMPONENT_ID_PNS_DCP
#define HIL_TASK_ID_PNS_CLRPC                         HIL_COMPONENT_ID_PNS_CLRPC
#define HIL_TASK_ID_PNS_IF_INTERN                     HIL_COMPONENT_ID_PNS_IF_INTERN
#define HIL_TASK_ID_PNIO_IRT_SCHEDULER                HIL_COMPONENT_ID_PNIO_IRT_SCHEDULER
#define HIL_TASK_ID_PNIO_RTA                          HIL_COMPONENT_ID_PNIO_RTA
#define HIL_TASK_ID_PNIO_RTC                          HIL_COMPONENT_ID_PNIO_RTC
#define HIL_TASK_ID_FODMI_TASK                        HIL_COMPONENT_ID_FODMI_TASK
#define HIL_TASK_ID_PNIO_HAL_COMPUTE                  HIL_COMPONENT_ID_PNIO_HAL_COMPUTE
#define HIL_TASK_ID_PNIO_HAL_IRQ                      HIL_COMPONENT_ID_PNIO_HAL_IRQ
#define HIL_TASK_ID_RPC_TASK                          HIL_COMPONENT_ID_RPC_TASK
#define HIL_TASK_ID_ROUTER_OS_CONSOLE32               HIL_COMPONENT_ID_ROUTER_OS_CONSOLE32
#define HIL_TASK_ID_ROUTER_ECAT_VOE                   HIL_COMPONENT_ID_ROUTER_ECAT_VOE
#define HIL_TASK_ID_ROUTER_HIF_PACKET                 HIL_COMPONENT_ID_ROUTER_HIF_PACKET
#define HIL_TASK_ID_EPL_NMT                           HIL_COMPONENT_ID_EPL_NMT
#define HIL_TASK_ID_EPL_PCK                           HIL_COMPONENT_ID_EPL_PCK
#define HIL_TASK_ID_EPL_DPM                           HIL_COMPONENT_ID_EPL_DPM
#define HIL_TASK_ID_PLSV3_AP                          HIL_COMPONENT_ID_PLSV3_AP
#define HIL_TASK_ID_PLSV3_IF                          HIL_COMPONENT_ID_PLSV3_IF
#define HIL_TASK_ID_PLSV3_NMT                         HIL_COMPONENT_ID_PLSV3_NMT
#define HIL_TASK_ID_PLV3_COMMON                       HIL_COMPONENT_ID_PLV3_COMMON
#define HIL_TASK_ID_PLSV3_DRVETH                      HIL_COMPONENT_ID_PLSV3_DRVETH
#define HIL_TASK_ID_EPL_PDO                           HIL_COMPONENT_ID_EPL_PDO
#define HIL_TASK_ID_EPL_SDO                           HIL_COMPONENT_ID_EPL_SDO
#define HIL_TASK_ID_EPL_MN                            HIL_COMPONENT_ID_EPL_MN
#define HIL_TASK_ID_ASI_ECTRL                         HIL_COMPONENT_ID_ASI_ECTRL
#define HIL_TASK_ID_ASI_AP                            HIL_COMPONENT_ID_ASI_AP
#define HIL_TASK_ID_TCPUDP                            HIL_COMPONENT_ID_TCPUDP
#define HIL_TASK_ID_TCPIP_AP                          HIL_COMPONENT_ID_TCPIP_AP
#define HIL_TASK_ID_TCPIP_SOCKIF                      HIL_COMPONENT_ID_TCPIP_SOCKIF
#define HIL_TASK_ID_SERCOSIII_API                     HIL_COMPONENT_ID_SERCOSIII_API
#define HIL_TASK_ID_SERCOSIII_DL                      HIL_COMPONENT_ID_SERCOSIII_DL
#define HIL_TASK_ID_SERCOSIII_SVC                     HIL_COMPONENT_ID_SERCOSIII_SVC
#define HIL_TASK_ID_SERCOSIII_ETH                     HIL_COMPONENT_ID_SERCOSIII_ETH
#define HIL_TASK_ID_SERCOSIII_NRT                     HIL_COMPONENT_ID_SERCOSIII_NRT
#define HIL_TASK_ID_SERCOSIII_CYCLIC                  HIL_COMPONENT_ID_SERCOSIII_CYCLIC
#define HIL_TASK_ID_PROFIBUS_DL                       HIL_COMPONENT_ID_PROFIBUS_DL
#define HIL_TASK_ID_PROFIBUS_FSPMS                    HIL_COMPONENT_ID_PROFIBUS_FSPMS
#define HIL_TASK_ID_PROFIBUS_APS                      HIL_COMPONENT_ID_PROFIBUS_APS
#define HIL_TASK_ID_PROFIBUS_FSPMM                    HIL_COMPONENT_ID_PROFIBUS_FSPMM
#define HIL_TASK_ID_PROFIBUS_APM                      HIL_COMPONENT_ID_PROFIBUS_APM
#define HIL_TASK_ID_SNMP_SERVER                       HIL_COMPONENT_ID_SNMP_SERVER
#define HIL_TASK_ID_MIB_DATABASE                      HIL_COMPONENT_ID_MIB_DATABASE
#define HIL_TASK_ID_ICONL_TIMER                       HIL_COMPONENT_ID_ICONL_TIMER
#define HIL_TASK_ID_ICONL_RUN                         HIL_COMPONENT_ID_ICONL_RUN
#define HIL_TASK_ID_LLDP                              HIL_COMPONENT_ID_LLDP
#define HIL_TASK_ID_CAN_DL                            HIL_COMPONENT_ID_CAN_DL
#define HIL_TASK_ID_DDL_ENPDDL                        HIL_COMPONENT_ID_DDL_ENPDDL
#define HIL_TASK_ID_DDL_DDL                           HIL_COMPONENT_ID_DDL_DDL
#define HIL_TASK_ID_CANOPEN_MASTER                    HIL_COMPONENT_ID_CANOPEN_MASTER
#define HIL_TASK_ID_CANOPEN_SLAVE                     HIL_COMPONENT_ID_CANOPEN_SLAVE
#define HIL_TASK_ID_USB_TLRROUTER                     HIL_COMPONENT_ID_USB_TLRROUTER
#define HIL_TASK_ID_CANDL_APSAMPLE                    HIL_COMPONENT_ID_CANDL_APSAMPLE
#define HIL_TASK_ID_DEVNET_FAL                        HIL_COMPONENT_ID_DEVNET_FAL
#define HIL_TASK_ID_DEVNET_AP                         HIL_COMPONENT_ID_DEVNET_AP
#define HIL_TASK_ID_DPM_OD2                           HIL_COMPONENT_ID_DPM_OD2
#define HIL_TASK_ID_CANOPEN_APM                       HIL_COMPONENT_ID_CANOPEN_APM
#define HIL_TASK_ID_CANOPEN_APS                       HIL_COMPONENT_ID_CANOPEN_APS
#define HIL_TASK_ID_SERCOS_SL                         HIL_COMPONENT_ID_SERCOS_SL
#define HIL_TASK_ID_ECAT_DPM                          HIL_COMPONENT_ID_ECAT_DPM
#define HIL_TASK_ID_ECAT_INXAP                        HIL_COMPONENT_ID_ECAT_INXAP
#define HIL_TASK_ID_SERCOSIII_SL_COM                  HIL_COMPONENT_ID_SERCOSIII_SL_COM
#define HIL_TASK_ID_SERCOSIII_SL_SVC                  HIL_COMPONENT_ID_SERCOSIII_SL_SVC
#define HIL_TASK_ID_SERCOSIII_SL_RTD                  HIL_COMPONENT_ID_SERCOSIII_SL_RTD
#define HIL_TASK_ID_SERCOSIII_SL_AP                   HIL_COMPONENT_ID_SERCOSIII_SL_AP
#define HIL_TASK_ID_SERCOSIII_SL_IDN                  HIL_COMPONENT_ID_SERCOSIII_SL_IDN
#define HIL_TASK_ID_EIP_APS                           HIL_COMPONENT_ID_EIP_APS
#define HIL_TASK_ID_EIP_APM                           HIL_COMPONENT_ID_EIP_APM
#define HIL_TASK_ID_ETH_INTF                          HIL_COMPONENT_ID_ETH_INTF
#define HIL_TASK_ID_MID_STARTUP                       HIL_COMPONENT_ID_MID_STARTUP
#define HIL_TASK_ID_OMB_OMBTASK                       HIL_COMPONENT_ID_OMB_OMBTASK
#define HIL_TASK_ID_OMB_OMBAPTASK                     HIL_COMPONENT_ID_OMB_OMBAPTASK
#define HIL_TASK_ID_DNS_FAL                           HIL_COMPONENT_ID_DNS_FAL
#define HIL_TASK_ID_DNS_AP                            HIL_COMPONENT_ID_DNS_AP
#define HIL_TASK_ID_ETHERCAT_MASTER_AP                HIL_COMPONENT_ID_ETHERCAT_MASTER_AP
#define HIL_TASK_ID_ETHERCAT_MASTER                   HIL_COMPONENT_ID_ETHERCAT_MASTER
#define HIL_TASK_ID_AN_TRANS                          HIL_COMPONENT_ID_AN_TRANS
#define HIL_TASK_ID_PROFIBUS_MPI                      HIL_COMPONENT_ID_PROFIBUS_MPI
#define HIL_TASK_ID_PROFIBUS_MPI_AP                   HIL_COMPONENT_ID_PROFIBUS_MPI_AP
#define HIL_TASK_ID_PROFIBUS_MPI_RFC                  HIL_COMPONENT_ID_PROFIBUS_MPI_RFC
#define HIL_TASK_ID_PROFIBUS_FSPMM2                   HIL_COMPONENT_ID_PROFIBUS_FSPMM2
#define HIL_TASK_ID_CCLINK_SLAVE                      HIL_COMPONENT_ID_CCLINK_SLAVE
#define HIL_TASK_ID_CCLINK_APS                        HIL_COMPONENT_ID_CCLINK_APS
#define HIL_TASK_ID_ECS_LENZE_MAIN                    HIL_COMPONENT_ID_ECS_LENZE_MAIN
#define HIL_TASK_ID_ECS_LENZE_BRIDGE                  HIL_COMPONENT_ID_ECS_LENZE_BRIDGE
#define HIL_TASK_ID_IOLINK_MASTER                     HIL_COMPONENT_ID_IOLINK_MASTER
#define HIL_TASK_ID_IOLINK_APPLICATION                HIL_COMPONENT_ID_IOLINK_APPLICATION
#define HIL_TASK_ID_MODBUS_RTU                        HIL_COMPONENT_ID_MODBUS_RTU
#define HIL_TASK_ID_MODBUS_RTU_AP                     HIL_COMPONENT_ID_MODBUS_RTU_AP
#define HIL_TASK_ID_SIII_MA_CP                        HIL_COMPONENT_ID_SIII_MA_CP
#define HIL_TASK_ID_SIII_MA_AP                        HIL_COMPONENT_ID_SIII_MA_AP
#define HIL_TASK_ID_SERCOSIIIMASTER_SVC               HIL_COMPONENT_ID_SERCOSIIIMASTER_SVC
#define HIL_TASK_ID_SERCOSIIIMASTER_ACFG              HIL_COMPONENT_ID_SERCOSIIIMASTER_ACFG
#define HIL_TASK_ID_SERCOSIIIMASTER_SIP_CLIENT        HIL_COMPONENT_ID_SERCOSIIIMASTER_SIP_CLIENT
#define HIL_TASK_ID_SERCOSIIIMASTER_SIP_SERVER        HIL_COMPONENT_ID_SERCOSIIIMASTER_SIP_SERVER
#define HIL_TASK_ID_SSIO                              HIL_COMPONENT_ID_SSIO
#define HIL_TASK_ID_SSIO_AP                           HIL_COMPONENT_ID_SSIO_AP
#define HIL_TASK_ID_MEMORY_MAP                        HIL_COMPONENT_ID_MEMORY_MAP
#define HIL_TASK_ID_MPI_GATEWAY                       HIL_COMPONENT_ID_MPI_GATEWAY
#define HIL_TASK_ID_SERCOSIIIMASTER_NRT               HIL_COMPONENT_ID_SERCOSIIIMASTER_NRT
#define HIL_TASK_ID_ASI_MASTER                        HIL_COMPONENT_ID_ASI_MASTER
#define HIL_TASK_ID_ASI_APM                           HIL_COMPONENT_ID_ASI_APM
#define HIL_TASK_ID_COMPONET_SLAVE                    HIL_COMPONENT_ID_COMPONET_SLAVE
#define HIL_TASK_ID_COMPONET_SLAVE_AP                 HIL_COMPONENT_ID_COMPONET_SLAVE_AP
#define HIL_TASK_ID_ASCII                             HIL_COMPONENT_ID_ASCII
#define HIL_TASK_ID_ASCII_AP                          HIL_COMPONENT_ID_ASCII_AP
#define HIL_TASK_ID_NETSCRIPT                         HIL_COMPONENT_ID_NETSCRIPT
#define HIL_TASK_ID_MARSHALLER                        HIL_COMPONENT_ID_MARSHALLER
#define HIL_TASK_ID_PACKET_ROUTER                     HIL_COMPONENT_ID_PACKET_ROUTER
#define HIL_TASK_ID_TCP_CONNECTOR                     HIL_COMPONENT_ID_TCP_CONNECTOR
#define HIL_TASK_ID_NT100_GATEWAY                     HIL_COMPONENT_ID_NT100_GATEWAY
#define HIL_TASK_ID_ITEM_SERVER                       HIL_COMPONENT_ID_ITEM_SERVER
#define HIL_TASK_ID_ISAGRAF_LOG                       HIL_COMPONENT_ID_ISAGRAF_LOG
#define HIL_TASK_ID_ISAGRAF_VM                        HIL_COMPONENT_ID_ISAGRAF_VM
#define HIL_TASK_ID_ISAGRAF                           HIL_COMPONENT_ID_ISAGRAF
#define HIL_TASK_ID_ISAGRAF_ETCP                      HIL_COMPONENT_ID_ISAGRAF_ETCP
#define HIL_TASK_ID_ISAGRAF_ISARSI                    HIL_COMPONENT_ID_ISAGRAF_ISARSI
#define HIL_TASK_ID_ISAGRAF_MAINTENANCE               HIL_COMPONENT_ID_ISAGRAF_MAINTENANCE
#define HIL_TASK_ID_ISAGRAF_NTP                       HIL_COMPONENT_ID_ISAGRAF_NTP
#define HIL_TASK_ID_ISAGRAF_AP                        HIL_COMPONENT_ID_ISAGRAF_AP
#define HIL_TASK_ID_ISAGRAF_FTP                       HIL_COMPONENT_ID_ISAGRAF_FTP
#define HIL_TASK_ID_ISAGRAF_DS                        HIL_COMPONENT_ID_ISAGRAF_DS
#define HIL_TASK_ID_DF1                               HIL_COMPONENT_ID_DF1
#define HIL_TASK_ID_DF1_AP                            HIL_COMPONENT_ID_DF1_AP
#define HIL_TASK_ID_3964R                             HIL_COMPONENT_ID_3964R
#define HIL_TASK_ID_3964R_AP                          HIL_COMPONENT_ID_3964R_AP
#define HIL_TASK_ID_IO_SIGNAL                         HIL_COMPONENT_ID_IO_SIGNAL
#define HIL_TASK_ID_SERVX                             HIL_COMPONENT_ID_SERVX
#define HIL_TASK_ID_TCPIP_TCP_AP                      HIL_COMPONENT_ID_TCPIP_TCP_AP
#define HIL_TASK_ID_EIP_DLR                           HIL_COMPONENT_ID_EIP_DLR
#define HIL_TASK_ID_FODMI                             HIL_COMPONENT_ID_FODMI
#define HIL_TASK_ID_PROFIDRIVE                        HIL_COMPONENT_ID_PROFIDRIVE
#define HIL_TASK_ID_PROFIDRIVE_PA                     HIL_COMPONENT_ID_PROFIDRIVE_PA
#define HIL_TASK_ID_PROFIDRIVE_AP                     HIL_COMPONENT_ID_PROFIDRIVE_AP
#define HIL_TASK_ID_PROFIDRIVE_OD                     HIL_COMPONENT_ID_PROFIDRIVE_OD
#define HIL_TASK_ID_ODV3                              HIL_COMPONENT_ID_ODV3
#define HIL_TASK_ID_VARAN_CLIENT_AP                   HIL_COMPONENT_ID_VARAN_CLIENT_AP
#define HIL_TASK_ID_VARAN_CLIENT                      HIL_COMPONENT_ID_VARAN_CLIENT
#define HIL_TASK_ID_MODBUS_RTU_PERIPH                 HIL_COMPONENT_ID_MODBUS_RTU_PERIPH
#define HIL_TASK_ID_PROFINET_RTA                      HIL_COMPONENT_ID_PROFINET_RTA
#define HIL_TASK_ID_SIII_SIP                          HIL_COMPONENT_ID_SIII_SIP
#define HIL_TASK_ID_3S_PLC_HANDLER                    HIL_COMPONENT_ID_3S_PLC_HANDLER
#define HIL_TASK_ID_3S_PLC_HANDLER_AP                 HIL_COMPONENT_ID_3S_PLC_HANDLER_AP
#define HIL_TASK_ID_NETPLC_IO_HANDLER                 HIL_COMPONENT_ID_NETPLC_IO_HANDLER
#define HIL_TASK_ID_POWERLINK_MN_TASK                 HIL_COMPONENT_ID_POWERLINK_MN_TASK
#define HIL_TASK_ID_POWERLINK_MN_AP_TASK              HIL_COMPONENT_ID_POWERLINK_MN_AP_TASK
#define HIL_TASK_ID_POWERLINK_MN_NEW_DATA_TASK        HIL_COMPONENT_ID_POWERLINK_MN_NEW_DATA_TASK
#define HIL_TASK_ID_POWERLINK_MN_SDO_UDP_SOCK_TASK    HIL_COMPONENT_ID_POWERLINK_MN_SDO_UDP_SOCK_TASK
#define HIL_TASK_ID_POWERLINK_MN_VETH_TASK            HIL_COMPONENT_ID_POWERLINK_MN_VETH_TASK
#define HIL_TASK_ID_POWERLINK_MN_JOB_TOKEN_TASK       HIL_COMPONENT_ID_POWERLINK_MN_JOB_TOKEN_TASK
#define HIL_TASK_ID_RECORDING                         HIL_COMPONENT_ID_RECORDING
#define HIL_TASK_ID_INSIGHT                           HIL_COMPONENT_ID_INSIGHT
#define HIL_TASK_ID_SMARTWIRE_MASTER                  HIL_COMPONENT_ID_SMARTWIRE_MASTER
#define HIL_TASK_ID_ECSV4_ESM                         HIL_COMPONENT_ID_ECSV4_ESM
#define HIL_TASK_ID_ECSV4_MBX                         HIL_COMPONENT_ID_ECSV4_MBX
#define HIL_TASK_ID_ECSV4_COE                         HIL_COMPONENT_ID_ECSV4_COE
#define HIL_TASK_ID_ECSV4_SDO                         HIL_COMPONENT_ID_ECSV4_SDO
#define HIL_TASK_ID_ECSV4_EOE                         HIL_COMPONENT_ID_ECSV4_EOE
#define HIL_TASK_ID_ECSV4_FOE                         HIL_COMPONENT_ID_ECSV4_FOE
#define HIL_TASK_ID_ECSV4_FOE_FH                      HIL_COMPONENT_ID_ECSV4_FOE_FH
#define HIL_TASK_ID_ECSV4_SOE_SSC                     HIL_COMPONENT_ID_ECSV4_SOE_SSC
#define HIL_TASK_ID_ECSV4_SOE_IDN                     HIL_COMPONENT_ID_ECSV4_SOE_IDN
#define HIL_TASK_ID_ECSV4_DPM                         HIL_COMPONENT_ID_ECSV4_DPM
#define HIL_TASK_ID_NET_PROXY                         HIL_COMPONENT_ID_NET_PROXY
#define HIL_TASK_ID_ECSV4_AOE                         HIL_COMPONENT_ID_ECSV4_AOE
#define HIL_TASK_ID_NET_SMART_GATEWAY                 HIL_COMPONENT_ID_NET_SMART_GATEWAY
#define HIL_TASK_ID_TFTP                              HIL_COMPONENT_ID_TFTP
#define HIL_TASK_ID_TFTP_AP                           HIL_COMPONENT_ID_TFTP_AP
#define HIL_TASK_ID_PTP                               HIL_COMPONENT_ID_PTP
#define HIL_TASK_ID_USER_AREA                         HIL_COMPONENT_ID_USER_AREA
#define HIL_TASK_ID_NETPROXY_S3S                      HIL_COMPONENT_ID_NETPROXY_S3S
#define HIL_TASK_ID_EIP_DLR_IRQ                       HIL_COMPONENT_ID_EIP_DLR_IRQ
#define HIL_TASK_ID_FTP_SERVER                        HIL_COMPONENT_ID_FTP_SERVER
#define HIL_TASK_ID_FTP_SERVER_AP                     HIL_COMPONENT_ID_FTP_SERVER_AP
#define HIL_TASK_ID_NPXHIFADAPTER                     HIL_COMPONENT_ID_NPXHIFADAPTER
#define HIL_TASK_ID_NPXPACKAGE_EIS                    HIL_COMPONENT_ID_NPXPACKAGE_EIS
#define HIL_TASK_ID_NETHOST                           HIL_COMPONENT_ID_NETHOST
#define HIL_TASK_ID_CIP_HANDLER                       HIL_COMPONENT_ID_CIP_HANDLER
#define HIL_TASK_ID_CPX_DRIVER                        HIL_COMPONENT_ID_CPX_DRIVER
#define HIL_TASK_ID_CPX_EXCHANGE                      HIL_COMPONENT_ID_CPX_EXCHANGE
#define HIL_TASK_ID_CPX_OSAL                          HIL_COMPONENT_ID_CPX_OSAL
#define HIL_TASK_ID_PNM_AP                            HIL_COMPONENT_ID_PNM_AP
#define HIL_TASK_ID_ECMV4_AP                          HIL_COMPONENT_ID_ECMV4_AP
#define HIL_TASK_ID_ECMV4_IF                          HIL_COMPONENT_ID_ECMV4_IF
#define HIL_TASK_ID_ECMV4_LLD_SCHED                   HIL_COMPONENT_ID_ECMV4_LLD_SCHED
#define HIL_TASK_ID_ECMV4_EMC                         HIL_COMPONENT_ID_ECMV4_EMC
#define HIL_TASK_ID_ECMV4_EMC_LLD_IF                  HIL_COMPONENT_ID_ECMV4_EMC_LLD_IF
#define HIL_TASK_ID_ECMV4_COE_SDOCLIENT               HIL_COMPONENT_ID_ECMV4_COE_SDOCLIENT
#define HIL_TASK_ID_ECMV4_COE_SDOINFOCLIENT           HIL_COMPONENT_ID_ECMV4_COE_SDOINFOCLIENT
#define HIL_TASK_ID_ECMV4_EOE                         HIL_COMPONENT_ID_ECMV4_EOE
#define HIL_TASK_ID_ECMV4_EOE_IF                      HIL_COMPONENT_ID_ECMV4_EOE_IF
#define HIL_TASK_ID_ECMV4_SOE_CLIENT                  HIL_COMPONENT_ID_ECMV4_SOE_CLIENT
#define HIL_TASK_ID_ECMV4_FOE_CLIENT                  HIL_COMPONENT_ID_ECMV4_FOE_CLIENT
#define HIL_TASK_ID_ECMV4_AOE_ROUTER                  HIL_COMPONENT_ID_ECMV4_AOE_ROUTER
#define HIL_TASK_ID_NPXPACKAGE_ODV3                   HIL_COMPONENT_ID_NPXPACKAGE_ODV3
#define HIL_TASK_ID_NPXPACKAGE_ECS                    HIL_COMPONENT_ID_NPXPACKAGE_ECS
#define HIL_TASK_ID_NPXPACKAGE_IOLM                   HIL_COMPONENT_ID_NPXPACKAGE_IOLM
#define HIL_TASK_ID_NPXMGMT                           HIL_COMPONENT_ID_NPXMGMT
#define HIL_TASK_ID_CMD_TABLE                         HIL_COMPONENT_ID_CMD_TABLE
#define HIL_TASK_ID_NT_DPM_BRIDGE                     HIL_COMPONENT_ID_NT_DPM_BRIDGE
#define HIL_TASK_ID_NPXLEDHNDLR                       HIL_COMPONENT_ID_NPXLEDHNDLR
#define HIL_TASK_ID_LWIP                              HIL_COMPONENT_ID_LWIP
#define HIL_TASK_ID_WORKER_THREAD                     HIL_COMPONENT_ID_WORKER_THREAD
#define HIL_TASK_ID_NPXPACKAGE_PNS                    HIL_COMPONENT_ID_NPXPACKAGE_PNS
#define HIL_TASK_ID_BASEFW_AP                         HIL_COMPONENT_ID_BASEFW_AP
#define HIL_TASK_ID_CONFIG_MANAGER                    HIL_COMPONENT_ID_CONFIG_MANAGER
#define HIL_TASK_ID_NPXPACKAGE_REMANENT               HIL_COMPONENT_ID_NPXPACKAGE_REMANENT

#endif /* HIL_COMPONENT_ID_H_ */

