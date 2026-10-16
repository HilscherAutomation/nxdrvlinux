/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $Id:  $:

  Description:
    This file contains all Hilscher components (task) identifiers.
    NOTE: New component IDs will be administered by the software protocol (SPC)
          department.

**************************************************************************************/
#ifndef HIL_COMPONENT_ID_H_
#define HIL_COMPONENT_ID_H_

#include <stdint.h>

/** The task identifier is unknown. */
#define HIL_COMPONENT_ID_UNKNOWN_IDENTIFIER 0x00000000L

/***********************************************************************************/
/* TLR Timer identifiers */
/***********************************************************************************/
/** TLR Timer Task. */
#define HIL_COMPONENT_ID_TIMER           0x00020001L

/***********************************************************************************/
/* System task identifiers */
/***********************************************************************************/
/** LibStorage. */
#define HIL_COMPONENT_ID_LIBSTORAGE      0x00010000L

/** Middleware System Task. */
#define HIL_COMPONENT_ID_MID_SYS         0x00010001L

/** Middleware System Debug Backend Task. */
#define HIL_COMPONENT_ID_MID_DBG         0x00010002L

/** RX IDLE Task. */
#define HIL_COMPONENT_ID_RX_IDLE         0x00010003L

/** IRQ Handler Task. */
#define HIL_COMPONENT_ID_IRQ_HANDLER     0x00010004L

/** Idle Task. */
#define HIL_COMPONENT_ID_IDLE            0x00010005L

/** Bootup Task. */
#define HIL_COMPONENT_ID_BOOTUP          0x00010006L

/** rcX Timer. */
#define HIL_COMPONENT_ID_RX_TIMER        0x00010007L

/** Maintenance Task. */
#define HIL_COMPONENT_ID_MAINTENANCE     0x00010008L

/***********************************************************************************/
/* EtherCAT Base stack task identifiers */
/***********************************************************************************/
/** EtherCAT Stack. */
#define HIL_COMPONENT_ID_ECAT            0x00200000L

/** EtherCAT ESM Task. */
#define HIL_COMPONENT_ID_ECAT_ESM        0x00200001L

/** EtherCAT Mailbox Task. */
#define HIL_COMPONENT_ID_ECAT_MBX        0x00200002L

/** EtherCAT Cyclic Input Task. */
#define HIL_COMPONENT_ID_ECAT_CYCLIC_IN  0x00200003L

/** EtherCAT Cyclic Output Task. */
#define HIL_COMPONENT_ID_ECAT_CYCLIC_OUT 0x00200004L

/***********************************************************************************/
/* EtherCAT CoE stack task identifiers */
/***********************************************************************************/
/** EtherCAT CoE Task. */
#define HIL_COMPONENT_ID_ECAT_COE        0x00210001L

/** EtherCAT CoE PDO Task. */
#define HIL_COMPONENT_ID_ECAT_COE_PDO    0x00210002L

/** EtherCAT CoE SDO Task. */
#define HIL_COMPONENT_ID_ECAT_COE_SDO    0x00210003L

/***********************************************************************************/
/* EtherCAT VoE stack task identifiers */
/***********************************************************************************/
/** EtherCAT VoE Task. */
#define HIL_COMPONENT_ID_ECAT_VOE        0x00260001L

/***********************************************************************************/
/* EtherCAT FoE stack task identifiers */
/***********************************************************************************/
/** EtherCAT FoE Task. */
#define HIL_COMPONENT_ID_ECAT_FOE        0x00240001L

/** EtherCAT FoE File Handler Task. */
#define HIL_COMPONENT_ID_ECAT_FOE_FH     0x00240002L

/***********************************************************************************/
/* EtherCAT EoE stack task identifiers */
/***********************************************************************************/
/** EtherCAT EoE Task. */
#define HIL_COMPONENT_ID_ECAT_EOE        0x00230001L

/***********************************************************************************/
/* EtherCAT SoE stack task identifiers */
/***********************************************************************************/
/** EtherCAT SoE SSC-Task. */
#define HIL_COMPONENT_ID_ECAT_SOE_SSC    0x00220001L

/** EtherCAT SoE IDN-Task. */
#define HIL_COMPONENT_ID_ECAT_SOE_IDN    0x00220002L

/***********************************************************************************/
/* Example Tasks */
/***********************************************************************************/
/** Example Task 1. */
#define HIL_COMPONENT_ID_EXAMPLE_TASK1   0x00030001L

/** Example Task 2. */
#define HIL_COMPONENT_ID_EXAMPLE_TASK2   0x00040001L

/** Example Task 3. */
#define HIL_COMPONENT_ID_EXAMPLE_TASK3   0x00050001L

/***********************************************************************************/
/* EtherNet/IP Encapsulation */
/***********************************************************************************/
/** EthernetIP Encapsulation Task. */
#define HIL_COMPONENT_ID_EIP_ENCAP       0x001E0001L

/** EthernetIP Encapsulation Task Class 1 services. */
#define HIL_COMPONENT_ID_EIP_CL1         0x001E0002L

/***********************************************************************************/
/* EtherNet/IP Object */
/***********************************************************************************/
/** EthernetIP Object Task. */
#define HIL_COMPONENT_ID_EIP_OBJECT      0x001F0001L

/***********************************************************************************/
/* EtherNet/Ip HAL EDD 2PS task identifiers */
/***********************************************************************************/
/** EtherNet/Ip Low Priority EDD Task. */
#define HIL_COMPONENT_ID_EIP_EDD_LOW     0x00EF0001L

/** EtherNet/Ip High Priority EDD Task. */
#define HIL_COMPONENT_ID_EIP_EDD_HIGH    0x00EE0001L

/***********************************************************************************/
/* PROFINET IO task identifiers */
/***********************************************************************************/
/** PROFINET IO-Controller Context Management Task. */
#define HIL_COMPONENT_ID_PNIO_CMCTL      0x000A0001L

/** PROFINET IO-Device Context Management Task. */
#define HIL_COMPONENT_ID_PNIO_CMDEV      0x000B0001L

/** PROFINET IO ACP Task. */
#define HIL_COMPONENT_ID_PNIO_ACP        0x00110001L

/** PROFINET IO DCP Task. */
#define HIL_COMPONENT_ID_PNIO_DCP        0x00120001L

/** PROFINET IO EDD Task. */
#define HIL_COMPONENT_ID_PNIO_EDD        0x000E0001L

/** PROFINET IO Management Task. */
#define HIL_COMPONENT_ID_PNIO_MGT        0x00130001L

/** PROFINET IO-Controller Application Task. */
#define HIL_COMPONENT_ID_PNIO_APCTL      0x000C0001L

/** PROFINET IO-Device Application Task. */
#define HIL_COMPONENT_ID_PNIO_APDEV      0x000D0001L

/** PROFINET IO-Controller Configuration Task. */
#define HIL_COMPONENT_ID_PNIO_APCFG      0x00140001L

/** PROFINET IO-Device Interface Task. */
#define HIL_COMPONENT_ID_PNS_IF          0x00300001L

/** PROFINET IO-Device 16Bit IO Application Task. */
#define HIL_COMPONENT_ID_PNIOD_16BITIO   0x003A0001L

/** PROFINET IO-Device 32Bit IO Application Task. */
#define HIL_COMPONENT_ID_PNS_32BITIO     0x005E0001L

/** PROFINET IO-Device 4Bit IO Application Task. */
#define HIL_COMPONENT_ID_PNS_4BITIO      0x00450001L

/** PROFINET IO-Device Socket Server Task. */
#define HIL_COMPONENT_ID_PNS_SOCKET_SRV  0x00520001L

/** PROFINET IO-Device High Priority EDD Task. */
#define HIL_COMPONENT_ID_PNS_EDD_HIGH    0x00530001L

/** PROFINET IO-Device Low Priority EDD Task. */
#define HIL_COMPONENT_ID_PNS_EDD_LOW     0x00540001L

/** PROFINET IO-Device Socket Task. */
#define HIL_COMPONENT_ID_PNS_SOCKET      0x00550001L

/** PROFINET IO-Device DCP Task. */
#define HIL_COMPONENT_ID_PNS_DCP         0x00560001L

/** PROFINET IO-Device Connectionless RPC Task. */
#define HIL_COMPONENT_ID_PNS_CLRPC       0x00570001L

/** PROFINET IO-Device Stack internal Interface Task. */
#define HIL_COMPONENT_ID_PNS_IF_INTERN   0x00580001L

/** PROFINET IO IRT Scheduler Task. */
#define HIL_COMPONENT_ID_PNIO_IRT_SCHEDULER 0x00810001L

/** PROFINET IO RTA Task. */
#define HIL_COMPONENT_ID_PNIO_RTA        0x00A70001L

/** PROFINET IO RTC Task. */
#define HIL_COMPONENT_ID_PNIO_RTC        0x00A80001L

/** PROFINET IO FODMI (FiberOptic Diagnosis) Task. */
#define HIL_COMPONENT_ID_FODMI_TASK      0x00A90001L

/** PROFINET IO HAL protocol Task. */
#define HIL_COMPONENT_ID_PNIO_HAL_COMPUTE 0x00AA0001L

/** PROFINET IO HAL interrupt Task. */
#define HIL_COMPONENT_ID_PNIO_HAL_IRQ    0x00AB0001L

/***********************************************************************************/
/* RPC task */
/***********************************************************************************/
/** RPC Task. */
#define HIL_COMPONENT_ID_RPC_TASK        0x002E0001L

/***********************************************************************************/
/* Router tasks */
/***********************************************************************************/
/** TLR-Router OS_Console32. */
#define HIL_COMPONENT_ID_ROUTER_OS_CONSOLE32 0x002F0000L

/** TLR-Router EtherCAT VoE. */
#define HIL_COMPONENT_ID_ROUTER_ECAT_VOE 0x002F0001L

/** TLR-Router DPM. */
#define HIL_COMPONENT_ID_ROUTER_HIF_PACKET 0x002F0002L

/***********************************************************************************/
/* PowerLink tasks */
/***********************************************************************************/
/** Ethernet PowerLink NMT Task. */
#define HIL_COMPONENT_ID_EPL_NMT         0x00170000L

/** Ethernet PowerLink Packet Task. */
#define HIL_COMPONENT_ID_EPL_PCK         0x00170001L

/** Ethernet PowerLink DPM Task. */
#define HIL_COMPONENT_ID_EPL_DPM         0x00170002L

/** Ethernet PowerLink AP Task. */
#define HIL_COMPONENT_ID_PLSV3_AP        0x00E10000L

/** Ethernet PowerLink CN IF Task. */
#define HIL_COMPONENT_ID_PLSV3_IF        0x00E30000L

/** Ethernet PowerLink CN NMT Task. */
#define HIL_COMPONENT_ID_PLSV3_NMT       0x00E60000L

/** Ethernet PowerLink Common Task. */
#define HIL_COMPONENT_ID_PLV3_COMMON     0x00E70000L

/** Ethernet Powerlink DrvEth Adapter Task. */
#define HIL_COMPONENT_ID_PLSV3_DRVETH    0x00E80000L

/***********************************************************************************/
/* PowerLink PDO task */
/***********************************************************************************/
/** Ethernet PowerLink PDO Task. */
#define HIL_COMPONENT_ID_EPL_PDO         0x00150000L

/***********************************************************************************/
/* PowerLink SDO task */
/***********************************************************************************/
/** Ethernet PowerLink SDO Task. */
#define HIL_COMPONENT_ID_EPL_SDO         0x00160000L

/***********************************************************************************/
/* PowerLink MN task */
/***********************************************************************************/
/** Ethernet PowerLink MN Task. */
#define HIL_COMPONENT_ID_EPL_MN          0x003D0000L

/***********************************************************************************/
/* AS-Interface task identifiers */
/***********************************************************************************/
/** AS-Interface ECTRL task (ASi stack). */
#define HIL_COMPONENT_ID_ASI_ECTRL       0x00320000L

/** AS-Interface Application task (example). */
#define HIL_COMPONENT_ID_ASI_AP          0x00000002L

/***********************************************************************************/
/* TCPUDP task identifiers (TCP/IP stack) */
/***********************************************************************************/
/** TCPUDP task (TCP/IP stack). */
#define HIL_COMPONENT_ID_TCPUDP          0x00080000L

/** TCP/IP stack Application task. */
#define HIL_COMPONENT_ID_TCPIP_AP        0x00000001L

/** TCP/IP Socket Interface. */
#define HIL_COMPONENT_ID_TCPIP_SOCKIF    0x00740002L

/***********************************************************************************/
/* Sercos task identifiers (Slave) */
/***********************************************************************************/
/** Sercos API task. */
#define HIL_COMPONENT_ID_SERCOSIII_API   0x00340000L

/** SERCOSIII DL task. */
#define HIL_COMPONENT_ID_SERCOSIII_DL    0x00350000L

/** SERCOSIII SVC task. */
#define HIL_COMPONENT_ID_SERCOSIII_SVC   0x00330000L

/** SERCOSIII ETH task. */
#define HIL_COMPONENT_ID_SERCOSIII_ETH   0x00360000L

/** SERCOSIII NRT task. */
#define HIL_COMPONENT_ID_SERCOSIII_NRT   0x00360001L

/** SERCOSIII cyclic task. */
#define HIL_COMPONENT_ID_SERCOSIII_CYCLIC 0x00370000L

/***********************************************************************************/
/* PROFIBUS task identifiers */
/***********************************************************************************/
/** PROFIBUS Data Link Layer Task. */
#define HIL_COMPONENT_ID_PROFIBUS_DL     0x00060000L

/** PROFIBUS Slave Fieldbus Service Protocol Machine Task. */
#define HIL_COMPONENT_ID_PROFIBUS_FSPMS  0x00090000L

/** PROFIBUS Slave Application Task. */
#define HIL_COMPONENT_ID_PROFIBUS_APS    0x001D0000L

/***********************************************************************************/
/* PROFIBUS Master task identifiers */
/***********************************************************************************/
/** PROFIBUS Master Fieldbus Service Protocol Machine Task. */
#define HIL_COMPONENT_ID_PROFIBUS_FSPMM  0x00380000L

/** PROFIBUS Master Application Task. */
#define HIL_COMPONENT_ID_PROFIBUS_APM    0x00390000L

/***********************************************************************************/
/* PROFIBUS Master task identifiers */
/***********************************************************************************/
/** SNMP Server Task. */
#define HIL_COMPONENT_ID_SNMP_SERVER     0x003B0000L

/** MIB Database for SNMP and LLDP. */
#define HIL_COMPONENT_ID_MIB_DATABASE    0x003C0000L

/** Icon-L Timer Task for iCon-L@netX. */
#define HIL_COMPONENT_ID_ICONL_TIMER     0x002A0000L

/** Icon-L Run Task for iCon-L@netX. */
#define HIL_COMPONENT_ID_ICONL_RUN       0x00290000L

/** LLDP protocol task */
#define HIL_COMPONENT_ID_LLDP            0x003E0000L

/***********************************************************************************/
/* CAN task identifiers */
/***********************************************************************************/
/** CAN DL Task (Data Link Layer). */
#define HIL_COMPONENT_ID_CAN_DL          0x003F0000L

/** CAN DL GCI-Adapter. */
#define HIL_COMPONENT_ID_CAN_DL_GCI_ADAPTER 0x003F0001L

/***********************************************************************************/
/* DDL task identifiers (J060219 Bosch Rexroth DDL mit netX) */
/***********************************************************************************/
/** ENPDDL gateway task. */
#define HIL_COMPONENT_ID_DDL_ENPDDL      0x00400000L

/** DDL protocol task. */
#define HIL_COMPONENT_ID_DDL_DDL         0x00410000L

/***********************************************************************************/
/* CANopen Master task identifiers */
/***********************************************************************************/
/** CANopen Master task (CANopen Master stack). */
#define HIL_COMPONENT_ID_CANOPEN_MASTER  0x00420000L

/***********************************************************************************/
/* CANopen Slave task identifiers */
/***********************************************************************************/
/** CANopen Slave task (CANopen Slave stack). */
#define HIL_COMPONENT_ID_CANOPEN_SLAVE   0x00430000L

/***********************************************************************************/
/* UsbTLRRouter identifiers */
/***********************************************************************************/
/** Usb-TLR-Router Task (Usb-TLRRouter). */
#define HIL_COMPONENT_ID_USB_TLRROUTER   0x00440000L

/***********************************************************************************/
/* CAN DL AP sample identifiers */
/***********************************************************************************/
/** CAN DL application sample Task. */
#define HIL_COMPONENT_ID_CANDL_APSAMPLE  0x00460000L

/***********************************************************************************/
/* DeviceNet FAL (Fieldbus Application Layer) task identifier */
/***********************************************************************************/
/** DeviceNet FAL Task (Fieldbus Application Layer). */
#define HIL_COMPONENT_ID_DEVNET_FAL      0x00470000L

/***********************************************************************************/
/* DeviceNet AP task identifier */
/***********************************************************************************/
/** DeviceNet AP Task (Application). */
#define HIL_COMPONENT_ID_DEVNET_AP       0x005B0000L

/***********************************************************************************/
/* ObjectDictionary V2 DPM Adapter */
/***********************************************************************************/
/** Object Dictionary DPM-Adapter Task. */
#define HIL_COMPONENT_ID_DPM_OD2         0x00480000L

/***********************************************************************************/
/* CANopen Master application task identifier */
/***********************************************************************************/
/** CANopen Master Application Task. */
#define HIL_COMPONENT_ID_CANOPEN_APM     0x00490000L

/***********************************************************************************/
/* CANopen Slave application task identifier */
/***********************************************************************************/
/** CANopen Slave Application Task. */
#define HIL_COMPONENT_ID_CANOPEN_APS     0x004A0000L

/***********************************************************************************/
/* Sercos3 Slave application task identifier */
/***********************************************************************************/
/** Sercos3 Slave Task. */
#define HIL_COMPONENT_ID_SERCOS_SL       0x004B0000L

/***********************************************************************************/
/* ECAT Slave DPM application task identifier */
/***********************************************************************************/
/** EtherCAT Slave DPM Task. */
#define HIL_COMPONENT_ID_ECAT_DPM        0x004C0000L

/***********************************************************************************/
/* ECAT Slave InxAp application task identifier */
/***********************************************************************************/
/** EtherCAT Slave InxAp Task. */
#define HIL_COMPONENT_ID_ECAT_INXAP      0x004C0001L

/***********************************************************************************/
/* Sercos Slave Communication task identifier */
/***********************************************************************************/
/** Sercos Slave Communication Task. */
#define HIL_COMPONENT_ID_SERCOSIII_SL_COM 0x004E0000L

/***********************************************************************************/
/* Sercos Slave Sercice Channel task identifier */
/***********************************************************************************/
/** Sercos Slave Service Channel Task. */
#define HIL_COMPONENT_ID_SERCOSIII_SL_SVC 0x004F0000L

/***********************************************************************************/
/* Sercos Slave Real Time Data task identifier */
/***********************************************************************************/
/** Sercos Slave Real Time Data Task. */
#define HIL_COMPONENT_ID_SERCOSIII_SL_RTD 0x00500000L

/***********************************************************************************/
/* Sercos Slave Application task identifier */
/***********************************************************************************/
/** Sercos Slave Application Task. */
#define HIL_COMPONENT_ID_SERCOSIII_SL_AP 0x00510000L

/***********************************************************************************/
/* Sercos Slave IDN task identifier */
/***********************************************************************************/
/** Sercos Slave IDN Task. */
#define HIL_COMPONENT_ID_SERCOSIII_SL_IDN 0x00850000L

/***********************************************************************************/
/* Ethernet/Ip Adapter Application task identifier */
/***********************************************************************************/
/** EtherNet/Ip Adapter Application-Task. */
#define HIL_COMPONENT_ID_EIP_APS         0x00590000L

/***********************************************************************************/
/* Ethernet/Ip Scanner Application task identifier */
/***********************************************************************************/
/** EtherNet/Ip Scanner Application-Task. */
#define HIL_COMPONENT_ID_EIP_APM         0x005A0000L

/***********************************************************************************/
/* Ethernet Interface Task */
/***********************************************************************************/
/** Ethernet Interface Task. */
#define HIL_COMPONENT_ID_ETH_INTF        0x005D0000L

/***********************************************************************************/
/* MID Startup Task */
/***********************************************************************************/
/** MID Startup Task. */
#define HIL_COMPONENT_ID_MID_STARTUP     0x005F0000L

/***********************************************************************************/
/* OMB task identifiers (Open Modbus TCP) */
/***********************************************************************************/
/** OMB task (Open Modbus TCP). */
#define HIL_COMPONENT_ID_OMB_OMBTASK     0x00600000L

/** OMB task (Open Modbus TCP) Application task. */
#define HIL_COMPONENT_ID_OMB_OMBAPTASK   0x00610001L

/***********************************************************************************/
/* DNS task identifiers (DeviceNet Slave) */
/***********************************************************************************/
/** DeviceNet Slave FAL Task (Fieldbus Application Layer). */
#define HIL_COMPONENT_ID_DNS_FAL         0x00620000L

/** DeviceNet Slave AP Task (Dualport Application). */
#define HIL_COMPONENT_ID_DNS_AP          0x00630000L

/***********************************************************************************/
/* EtherCAT Master AP Task identifiers (EtherCAT Master AP Task) */
/***********************************************************************************/
/** EtherCAT Master AP Task. */
#define HIL_COMPONENT_ID_ETHERCAT_MASTER_AP 0x00640000L

/** EtherCAT Master Stack Task. */
#define HIL_COMPONENT_ID_ETHERCAT_MASTER 0x00650000L

/***********************************************************************************/
/* Ethernet Analyzer Transfer Task identifiers (Ethernet Analyzer Transfer Task) */
/***********************************************************************************/
/** Ethernet Analyzer Transfer Task. */
#define HIL_COMPONENT_ID_AN_TRANS        0x00660000L

/***********************************************************************************/
/* PROFIBUS MPI */
/***********************************************************************************/
/** PROFIBUS MPI Task. */
#define HIL_COMPONENT_ID_PROFIBUS_MPI    0x00670000L

/** PROFIBUS MPI Application Task. */
#define HIL_COMPONENT_ID_PROFIBUS_MPI_AP 0x00680000L

/** PROFIBUS MPI Task. */
#define HIL_COMPONENT_ID_PROFIBUS_MPI_RFC 0x00730000L

/***********************************************************************************/
/* PROFIBUS FSPMM2 */
/***********************************************************************************/
/** PROFIBUS FSPMM2 Task. */
#define HIL_COMPONENT_ID_PROFIBUS_FSPMM2 0x00690000L

/***********************************************************************************/
/* CC-Link Slave */
/***********************************************************************************/
/** CC-Link Slave Task. */
#define HIL_COMPONENT_ID_CCLINK_SLAVE    0x006A0000L

/** CC-Link Slave Application Task. */
#define HIL_COMPONENT_ID_CCLINK_APS      0x006B0000L

/***********************************************************************************/
/* Lenze EtherCAT Slave Anbindung */
/***********************************************************************************/
/** Lenze EtherCAT slave interface for 9400, main task. */
#define HIL_COMPONENT_ID_ECS_LENZE_MAIN  0x006C0000L

/** Lenze EtherCAT slave interface for 9400, bus bridge. */
#define HIL_COMPONENT_ID_ECS_LENZE_BRIDGE 0x006C0001L

/***********************************************************************************/
/* IO-Link Master Task identifiers */
/***********************************************************************************/
/** IO-Link Master Task. */
#define HIL_COMPONENT_ID_IOLINK_MASTER   0x006D0000L

/** IO-Link Application and Application Layer Task. */
#define HIL_COMPONENT_ID_IOLINK_APPLICATION 0x00A50000L

/** Task to communicate with the IO-Link Master with TMG IO protocol. */
#define HIL_COMPONENT_ID_IOLINK_TMG_IO   0x00A50001L

/** Task to access the IO-Link master with test protocol. */
#define HIL_COMPONENT_ID_IOLINK_TEST_PROTOCOL 0x00A50002L

/***********************************************************************************/
/* MODBUS RTU */
/***********************************************************************************/
/** MODBUS RTU Task. */
#define HIL_COMPONENT_ID_MODBUS_RTU      0x006E0000L

/***********************************************************************************/
/* MODBUS RTU AP */
/***********************************************************************************/
/** MODBUS RTU Application Task. */
#define HIL_COMPONENT_ID_MODBUS_RTU_AP   0x006F0000L

/***********************************************************************************/
/* Sercos Master stack task identifiers */
/***********************************************************************************/
/** Sercos Communication Phase Task */
#define HIL_COMPONENT_ID_SIII_MA_CP      0x00700000L

/***********************************************************************************/
/* Sercos Master stack task identifiers */
/***********************************************************************************/
/** Sercos DPM AP Task */
#define HIL_COMPONENT_ID_SIII_MA_AP      0x00720000L

/***********************************************************************************/
/* Sercos Master stack task identifiers */
/***********************************************************************************/
/** Sercos Service Channel Task */
#define HIL_COMPONENT_ID_SERCOSIIIMASTER_SVC 0x00710000L

/***********************************************************************************/
/* Sercos Master stack task identifiers */
/***********************************************************************************/
/** Sercos Auto-Configure Task */
#define HIL_COMPONENT_ID_SERCOSIIIMASTER_ACFG 0x00B70000L

/***********************************************************************************/
/* Sercos Master stack task identifiers */
/***********************************************************************************/
/** Sercos S/IP Client Task */
#define HIL_COMPONENT_ID_SERCOSIIIMASTER_SIP_CLIENT 0x00BA0000L

/** Sercos S/IP Server Task */
#define HIL_COMPONENT_ID_SERCOSIIIMASTER_SIP_SERVER 0x00BA0001L

/***********************************************************************************/
/* SSIO task identifiers */
/***********************************************************************************/
/** Serial Shift IO Task */
#define HIL_COMPONENT_ID_SSIO            0x00750000L

/***********************************************************************************/
/* SSIO AP task identifiers */
/***********************************************************************************/
/** Serial Shift IO Application Task */
#define HIL_COMPONENT_ID_SSIO_AP         0x00760000L

/***********************************************************************************/
/* MAP NIC task identifiers */
/***********************************************************************************/
/** netIC Mapping Task */
#define HIL_COMPONENT_ID_MEMORY_MAP      0x00770000L

/***********************************************************************************/
/* MPI Gateway task identifiers */
/***********************************************************************************/
/** MPI Gateway Task */
#define HIL_COMPONENT_ID_MPI_GATEWAY     0x00780000L

/***********************************************************************************/
/* Sercos Master stack NRT task identifiers */
/***********************************************************************************/
/** Sercos NRT Channel Task */
#define HIL_COMPONENT_ID_SERCOSIIIMASTER_NRT 0x00790000L

/***********************************************************************************/
/* AS-Interface Master task identifier */
/***********************************************************************************/
/** AS-Interface Master Task (AS-Interface Master stack). */
#define HIL_COMPONENT_ID_ASI_MASTER      0x007A0000L

/***********************************************************************************/
/* AS-Interface Master application task identifier */
/***********************************************************************************/
/** AS-Interface Master Application Task. */
#define HIL_COMPONENT_ID_ASI_APM         0x007B0000L

/***********************************************************************************/
/* CompoNet Slave task identifier */
/***********************************************************************************/
/** CompoNet Slave Task. */
#define HIL_COMPONENT_ID_COMPONET_SLAVE  0x007C0000L

/** CompoNet Slave Application Task. */
#define HIL_COMPONENT_ID_COMPONET_SLAVE_AP 0x007D0000L

/***********************************************************************************/
/* ASCII task identifier */
/***********************************************************************************/
/** ASCII Protocol Task. */
#define HIL_COMPONENT_ID_ASCII           0x007E0000L

/** ASCII Application Task. */
#define HIL_COMPONENT_ID_ASCII_AP        0x007F0000L

/***********************************************************************************/
/* netscript task identifier */
/***********************************************************************************/
/** netScript Task. */
#define HIL_COMPONENT_ID_NETSCRIPT       0x00800000L

/***********************************************************************************/
/* Marshaller task identifier */
/***********************************************************************************/
/** Marshaller: Main Task. */
#define HIL_COMPONENT_ID_MARSHALLER      0x00820000L

/** Marshaller: Packet Router Task. */
#define HIL_COMPONENT_ID_PACKET_ROUTER   0x00830000L

/** Marshaller: TCP Connection Task. */
#define HIL_COMPONENT_ID_TCP_CONNECTOR   0x00860000L

/***********************************************************************************/
/* netTAP 100 Gateway task identifier */
/***********************************************************************************/
/** netTAP Gateway Task. */
#define HIL_COMPONENT_ID_NT100_GATEWAY   0x00840000L

/***********************************************************************************/
/* Item Server task identifier */
/***********************************************************************************/
/** Item Server Task. */
#define HIL_COMPONENT_ID_ITEM_SERVER     0x00870000L

/***********************************************************************************/
/* ISaGRAF task identifiers */
/***********************************************************************************/
/** telnet task for ISaGRAF log. */
#define HIL_COMPONENT_ID_ISAGRAF_LOG     0x008C0000L

/** Virtual Machine ISaGRAF task. */
#define HIL_COMPONENT_ID_ISAGRAF_VM      0x008B0000L

/** ISaGRAF command task. */
#define HIL_COMPONENT_ID_ISAGRAF         0x008A0000L

/** Ethernet ISaGRAF interface. */
#define HIL_COMPONENT_ID_ISAGRAF_ETCP    0x00890000L

/** RS232 ISaGRAF interface. */
#define HIL_COMPONENT_ID_ISAGRAF_ISARSI  0x00880000L

/** ISaGRAF maintenance task. */
#define HIL_COMPONENT_ID_ISAGRAF_MAINTENANCE 0x008A0001L

/** ISaGRAF NTP task. */
#define HIL_COMPONENT_ID_ISAGRAF_NTP     0x008A0002L

/** ISaGRAF AP task. */
#define HIL_COMPONENT_ID_ISAGRAF_AP      0x008A0003L

/** ISaGRAF FTP task. */
#define HIL_COMPONENT_ID_ISAGRAF_FTP     0x008A0004L

/** ISaGRAF DS task. */
#define HIL_COMPONENT_ID_ISAGRAF_DS      0x008A0005L

/***********************************************************************************/
/* DF1 task identifier */
/***********************************************************************************/
/** DF1 Protocol Task. */
#define HIL_COMPONENT_ID_DF1             0x008D0000L

/** DF1 Application Task. */
#define HIL_COMPONENT_ID_DF1_AP          0x008E0000L

/***********************************************************************************/
/* 3964R task identifier */
/***********************************************************************************/
/** 3964R Protocol Task. */
#define HIL_COMPONENT_ID_3964R           0x008F0000L

/** 3964R Application Task. */
#define HIL_COMPONENT_ID_3964R_AP        0x00900000L

/***********************************************************************************/
/* IO Signals task identifier */
/***********************************************************************************/
/** IO Signal Task. */
#define HIL_COMPONENT_ID_IO_SIGNAL       0x00910000L

/***********************************************************************************/
/* ServX task identifier */
/***********************************************************************************/
/** servX HTTP Server. */
#define HIL_COMPONENT_ID_SERVX           0x00920000L

/***********************************************************************************/
/* TCPIP Application task identifiers */
/***********************************************************************************/
/** TCPUDP application task (TCP/IP stack). */
#define HIL_COMPONENT_ID_TCPIP_TCP_AP    0x00940000L

/***********************************************************************************/
/* DLR task identifier */
/***********************************************************************************/
/** EthernetIP DLR task. */
#define HIL_COMPONENT_ID_EIP_DLR         0x00950000L

/***********************************************************************************/
/* FODMI task identifier */
/***********************************************************************************/
/** Fibre optic diagnostic monitoring interface task. */
#define HIL_COMPONENT_ID_FODMI           0x00960000L

/***********************************************************************************/
/* PROFIDRIVE task identifier */
/***********************************************************************************/
/** PROFIDRIVE task. */
#define HIL_COMPONENT_ID_PROFIDRIVE      0x00970000L

/***********************************************************************************/
/*****/
/***********************************************************************************/
/** PROFIDRIVE parameter Access interface task. */
#define HIL_COMPONENT_ID_PROFIDRIVE_PA   0x00980000L

/***********************************************************************************/
/*****/
/***********************************************************************************/
/** PROFIDRIVE user application task. */
#define HIL_COMPONENT_ID_PROFIDRIVE_AP   0x009A0000L

/***********************************************************************************/
/*****/
/***********************************************************************************/
/** PROFIDRIVE Object Dictionary task. */
#define HIL_COMPONENT_ID_PROFIDRIVE_OD   0x00990000L

/***********************************************************************************/
/* CANopen Object Dictionary */
/***********************************************************************************/
/** CANopen Object Dictionary task. */
#define HIL_COMPONENT_ID_ODV3            0x009B0000L

/***********************************************************************************/
/* VARAN Client Application */
/***********************************************************************************/
/** VARAN Client application task. */
#define HIL_COMPONENT_ID_VARAN_CLIENT_AP 0x009D0000L

/***********************************************************************************/
/* VARAN Client */
/***********************************************************************************/
/** VARAN Client task. */
#define HIL_COMPONENT_ID_VARAN_CLIENT    0x009C0000L

/***********************************************************************************/
/* Modbus RTU Peripheral Task */
/***********************************************************************************/
/** Modbus RTU Peripheral Task. */
#define HIL_COMPONENT_ID_MODBUS_RTU_PERIPH 0x009E0000L

/***********************************************************************************/
/* PROFINET RTA Task */
/***********************************************************************************/
/** PROFINET RTA (RealTime Acyclic) Task. */
#define HIL_COMPONENT_ID_PROFINET_RTA    0x009F0000L

/***********************************************************************************/
/* Sercos Internet Protocol Service(SIP) task identifier */
/***********************************************************************************/
/** Sercos Internet Protocol Service Task. */
#define HIL_COMPONENT_ID_SIII_SIP        0x00A00000L

/***********************************************************************************/
/* 3S CodeSys PLC Handler task identifier */
/***********************************************************************************/
/** 3S CodeSys PLC Handler Task. */
#define HIL_COMPONENT_ID_3S_PLC_HANDLER  0x00A10000L

/***********************************************************************************/
/* 3S CodeSys PLC Handler AP task identifier */
/***********************************************************************************/
/** 3S CodeSys PLC Handler AP Task. */
#define HIL_COMPONENT_ID_3S_PLC_HANDLER_AP 0x00A20000L

/***********************************************************************************/
/* netPLC I/O Handler task identifier */
/***********************************************************************************/
/** netPLC I/O Handler. */
#define HIL_COMPONENT_ID_NETPLC_IO_HANDLER 0x00A30000L

/***********************************************************************************/
/* PowerLink MN task identifiers */
/***********************************************************************************/
/** PowerLink MN Task */
#define HIL_COMPONENT_ID_POWERLINK_MN_TASK 0x00A40000L

/** PowerLink MN AP Task */
#define HIL_COMPONENT_ID_POWERLINK_MN_AP_TASK 0x00A40001L

/** PowerLink MN NewData Task */
#define HIL_COMPONENT_ID_POWERLINK_MN_NEW_DATA_TASK 0x00A40002L

/** PowerLink MN SDO UDP Socket Task */
#define HIL_COMPONENT_ID_POWERLINK_MN_SDO_UDP_SOCK_TASK 0x00A40003L

/** PowerLink MN VirtualEthernet Task */
#define HIL_COMPONENT_ID_POWERLINK_MN_VETH_TASK 0x00A40004L

/** PowerLink MN JobToken Task */
#define HIL_COMPONENT_ID_POWERLINK_MN_JOB_TOKEN_TASK 0x00A40005L

/***********************************************************************************/
/* Recording task identifier */
/***********************************************************************************/
/** Recording and logging Task. */
#define HIL_COMPONENT_ID_RECORDING       0x00A60000L

/***********************************************************************************/
/* Insight Debug task identifier */
/***********************************************************************************/
/** Insight task. */
#define HIL_COMPONENT_ID_INSIGHT         0x00AC0000L

/***********************************************************************************/
/* SmartWire Master Task identifier */
/***********************************************************************************/
/** SmartWire master task. */
#define HIL_COMPONENT_ID_SMARTWIRE_MASTER 0x00AD0000L

/***********************************************************************************/
/* EtherCAT Slave V4 Task identifier */
/***********************************************************************************/
/** EtherCAT Slave V4 ESM Task. */
#define HIL_COMPONENT_ID_ECSV4_ESM       0x00AF0000L

/** EtherCAT Slave V4 MBX Task. */
#define HIL_COMPONENT_ID_ECSV4_MBX       0x00B00000L

/** EtherCAT Slave V4 COE Task. */
#define HIL_COMPONENT_ID_ECSV4_COE       0x00B10000L

/** EtherCAT Slave V4 SDO Task. */
#define HIL_COMPONENT_ID_ECSV4_SDO       0x00B10001L

/** EtherCAT Slave V4 EOE Task. */
#define HIL_COMPONENT_ID_ECSV4_EOE       0x00B20000L

/** EtherCAT Slave V4 FOE Task. */
#define HIL_COMPONENT_ID_ECSV4_FOE       0x00B30000L

/** EtherCAT Slave V4 FOE FH Task. */
#define HIL_COMPONENT_ID_ECSV4_FOE_FH    0x00C40000L

/** EtherCAT Slave V4 SOE_SSC Task. */
#define HIL_COMPONENT_ID_ECSV4_SOE_SSC   0x00B40000L

/** EtherCAT Slave V4 SOE_IDN Task. */
#define HIL_COMPONENT_ID_ECSV4_SOE_IDN   0x00B40001L

/** EtherCAT Slave V4 DPM Task. */
#define HIL_COMPONENT_ID_ECSV4_DPM       0x00AE0000L

/** netPROXY. */
#define HIL_COMPONENT_ID_NET_PROXY       0x00B50000L

/** EtherCAT Slave V4 AoE Task. */
#define HIL_COMPONENT_ID_ECSV4_AOE       0x00BB0000L

/***********************************************************************************/
/* netSMART Gateway Task identifier */
/***********************************************************************************/
/** netSMART Gateway. */
#define HIL_COMPONENT_ID_NET_SMART_GATEWAY 0x00B60000L

/***********************************************************************************/
/* TFTP Stack Task identifier */
/***********************************************************************************/
/** TFTP Stack Task. */
#define HIL_COMPONENT_ID_TFTP            0x00B80000L

/***********************************************************************************/
/* TFTP App Task identifier */
/***********************************************************************************/
/** TFTP Application Task. */
#define HIL_COMPONENT_ID_TFTP_AP         0x00B90000L

/***********************************************************************************/
/* PTP Task identifier */
/***********************************************************************************/
/** PTP Task. */
#define HIL_COMPONENT_ID_PTP             0x00BD0000L

/***********************************************************************************/
/* User Area */
/***********************************************************************************/
/** User Area. */
#define HIL_COMPONENT_ID_USER_AREA       0x0FF00000L

/***********************************************************************************/
/* User Area */
/***********************************************************************************/
/** S3S netProxy adaptation task */
#define HIL_COMPONENT_ID_NETPROXY_S3S    0x00BC0000L

/***********************************************************************************/
/* EtherNet/IP DLR IRQ Task identifier */
/***********************************************************************************/
/** PTP Task. */
#define HIL_COMPONENT_ID_EIP_DLR_IRQ     0x00BE0000L

/***********************************************************************************/
/* FTP Server Stack Task */
/***********************************************************************************/
/** FTP Stack Task */
#define HIL_COMPONENT_ID_FTP_SERVER      0x00BF0000L

/***********************************************************************************/
/* FTP Server Application Task */
/***********************************************************************************/
/** FTP Application Task. */
#define HIL_COMPONENT_ID_FTP_SERVER_AP   0x00C00000L

/***********************************************************************************/
/* netPROXY HIF Adapter Task */
/***********************************************************************************/
/** netPROXY HIF Adapter Task. */
#define HIL_COMPONENT_ID_NPXHIFADAPTER   0x00C20000L

/***********************************************************************************/
/* netPROXY package EtherNetIP Slave */
/***********************************************************************************/
/** netPROXY package EtherNetIP Slave. */
#define HIL_COMPONENT_ID_NPXPACKAGE_EIS  0x00C30000L

/***********************************************************************************/
/* netPROXY package EtherNetIP Slave to IO-Link Master */
/***********************************************************************************/
/** netPROXY package EtherNetIP Slave to IO-Link Master. */
#define HIL_COMPONENT_ID_NPXPACKAGE_EIS_IOLM 0x01260000L

/***********************************************************************************/
/* Hilscher mbedTLS Component (PSMBEDTLS) */
/***********************************************************************************/
/** Hilscher PSMBEDTLS Component. */
#define HIL_COMPONENT_ID_PSMBEDTLS       0x01290000L

/***********************************************************************************/
/* netHost task identifier */
/***********************************************************************************/
/** netHost Task. */
#define HIL_COMPONENT_ID_NETHOST         0x00C50000L

/***********************************************************************************/
/* CIP Handler task identifier */
/***********************************************************************************/
/** CIP Handler Task. */
#define HIL_COMPONENT_ID_CIP_HANDLER     0x00C60000L

/***********************************************************************************/
/* CPX Driver task identifier */
/***********************************************************************************/
/** CPX Driver Task. */
#define HIL_COMPONENT_ID_CPX_DRIVER      0x00C70000L

/***********************************************************************************/
/* CPX Exchange task identifier */
/***********************************************************************************/
/** CPX Exchange Task. */
#define HIL_COMPONENT_ID_CPX_EXCHANGE    0x00C80000L

/***********************************************************************************/
/* OSAL */
/***********************************************************************************/
/** CPX Exchange Task. */
#define HIL_COMPONENT_ID_CPX_OSAL        0x00CA0000L

/***********************************************************************************/
/* PROFINET IO CONTROLLER V3 */
/***********************************************************************************/
/** PROFINET IO Controller V3 AP Task. */
#define HIL_COMPONENT_ID_PNM_AP          0x00CB0000L

/***********************************************************************************/
/* ECMv4 */
/***********************************************************************************/
/** EtherCAT Master AP Task. */
#define HIL_COMPONENT_ID_ECMV4_AP        0x00D70000L

/** EtherCAT Master Interface Task. */
#define HIL_COMPONENT_ID_ECMV4_IF        0x00D60000L

/** LLD Scheduler. */
#define HIL_COMPONENT_ID_ECMV4_LLD_SCHED 0x00CC0000L

/** LLD IF. */
#define HIL_COMPONENT_ID_ECMV4_LLD_IF    0x00CC0001L

/** EtherCAT Master Control Task. */
#define HIL_COMPONENT_ID_ECMV4_EMC       0x00CD0000L

/** EtherCAT Master Control Task LLD IF. */
#define HIL_COMPONENT_ID_ECMV4_EMC_LLD_IF 0x00CD0001L

/** SDO Client Task. */
#define HIL_COMPONENT_ID_ECMV4_COE_SDOCLIENT 0x00CF0000L

/** SDOINFO Client Task. */
#define HIL_COMPONENT_ID_ECMV4_COE_SDOINFOCLIENT 0x00CF0001L

/** SDO Server Task. */
#define HIL_COMPONENT_ID_ECMV4_COE_SDOSERVER 0x00CF0002L

/** SDOINFO Server Task. */
#define HIL_COMPONENT_ID_ECMV4_COE_SDOINFOSERVER 0x00CF0003L

/** EoE Task. */
#define HIL_COMPONENT_ID_ECMV4_EOE       0x00D00000L

/** EoE Interface Task. */
#define HIL_COMPONENT_ID_ECMV4_EOE_IF    0x00D00001L

/** SoE Client Task. */
#define HIL_COMPONENT_ID_ECMV4_SOE_CLIENT 0x00D20000L

/** FoE Client Task. */
#define HIL_COMPONENT_ID_ECMV4_FOE_CLIENT 0x00D10000L

/** AoE Router Task. */
#define HIL_COMPONENT_ID_ECMV4_AOE_ROUTER 0x00CE0000L

/** AoE Client Task. */
#define HIL_COMPONENT_ID_ECMV4_AOE_CLIENT 0x00CE0001L

/** MbxRouter Task. */
#define HIL_COMPONENT_ID_ECMV4_MBXROUTER 0x00DD0000L

/** ACfg Task. */
#define HIL_COMPONENT_ID_ECMV4_ACFG      0x00DE0000L

/***********************************************************************************/
/* netPROXY package ODV3 */
/***********************************************************************************/
/** netPROXY package ODV3. */
#define HIL_COMPONENT_ID_NPXPACKAGE_ODV3 0x00D80000L

/***********************************************************************************/
/* netPROXY package EtherCAT Slave */
/***********************************************************************************/
/** netPROXY package EtherCAT Slave. */
#define HIL_COMPONENT_ID_NPXPACKAGE_ECS  0x00D90000L

/***********************************************************************************/
/* netPROXY package IO-Link Master */
/***********************************************************************************/
/** netPROXY package IO-Link Master. */
#define HIL_COMPONENT_ID_NPXPACKAGE_IOLM 0x00DA0000L

/***********************************************************************************/
/* netPROXY Management task */
/***********************************************************************************/
/** netPROXY management task. */
#define HIL_COMPONENT_ID_NPXMGMT         0x00E00000L

/***********************************************************************************/
/* Command Table task identifier */
/***********************************************************************************/
/** Command Table Task. */
#define HIL_COMPONENT_ID_CMD_TABLE       0x00E20000L

/***********************************************************************************/
/* netTap DPM Bridge Task */
/***********************************************************************************/
/** netTap DPM Bridge Task. */
#define HIL_COMPONENT_ID_NT_DPM_BRIDGE   0x00E40000L

/***********************************************************************************/
/* netPROXY LED Handler Task */
/***********************************************************************************/
/** netPROXY LED Handler Task. */
#define HIL_COMPONENT_ID_NPXLEDHNDLR     0x00E50000L

/***********************************************************************************/
/* LWIP Task */
/***********************************************************************************/
/** LWIP Task. */
#define HIL_COMPONENT_ID_LWIP            0x00E90000L

/** LWIP packet socket API for GCI. */
#define HIL_COMPONENT_ID_LWIP_GCI_SOCKIF 0x00E90001L

/***********************************************************************************/
/* OSAL Worker Thread */
/***********************************************************************************/
/** OSAL Worker Thread. */
#define HIL_COMPONENT_ID_WORKER_THREAD   0x00EA0000L

/***********************************************************************************/
/* netPROXY package PROFINET IO-Device V4 */
/***********************************************************************************/
/** netPROXY package PROFINET IO-Device. */
#define HIL_COMPONENT_ID_NPXPACKAGE_PNS  0x00EB0000L

/***********************************************************************************/
/* Base Firmware Application Task */
/***********************************************************************************/
/** Base Firmware Application Task. */
#define HIL_COMPONENT_ID_BASEFW_AP       0x00EC0000L

/***********************************************************************************/
/* Config Manager Task */
/***********************************************************************************/
/** Config Manager Task. */
#define HIL_COMPONENT_ID_CONFIG_MANAGER  0x00ED0000L

/***********************************************************************************/
/* netPROXY package Remanent */
/***********************************************************************************/
/** netPROXY package Remanent. */
#define HIL_COMPONENT_ID_NPXPACKAGE_REMANENT 0x00F20000L

/***********************************************************************************/
/* CC-Link IE Field Slave */
/***********************************************************************************/
/** CC-Link IE Slave Task. */
#define HIL_COMPONENT_ID_CCLIES          0x00F30000L

/** CC-Link IE Slave IF Task. */
#define HIL_COMPONENT_ID_CCLIES_IF       0x00F40000L

/** CC-Link IE Slave AP Task. */
#define HIL_COMPONENT_ID_CCLIES_AP       0x00F50000L

/***********************************************************************************/
/* Authentication manager */
/***********************************************************************************/
/** Authentication manager component for user authentication. */
#define HIL_COMPONENT_ID_AUTH_MANAGER    0x00FB0000L

/** Authentication manager component for digital (X.509) certificates handling. */
#define HIL_COMPONENT_ID_AUTH_MANAGER_X_509 0x00FB0001L

/***********************************************************************************/
/* CC-Link IE Field Basic */
/***********************************************************************************/
/** CC-Link IE Field Basic Slave Task. */
#define HIL_COMPONENT_ID_CCLIEFB         0x00FE0000L

/** CC-Link IE Field Basic Master/Slave Task. */
#define HIL_COMPONENT_ID_CCLIEFBM        0x00FE0001L

/** CC-Link IE Field Basic IF Task. */
#define HIL_COMPONENT_ID_CCLIEFB_IF      0x00FF0000L

/** CC-Link IE Field Basic AP Task. */
#define HIL_COMPONENT_ID_CCLIEFB_AP      0x01000000L

/***********************************************************************************/
/* netPROXY package MQTT Client */
/***********************************************************************************/
/** netPROXY package MQTT Client. */
#define HIL_COMPONENT_ID_NPXPACKAGE_MQTT_CLIENT 0x00F90000L

/***********************************************************************************/
/* Generic AP */
/***********************************************************************************/
/** Generic AP Task. */
#define HIL_COMPONENT_ID_GENERIC_AP      0x01090000L

/***********************************************************************************/
/* netPROXY package OPCUA */
/***********************************************************************************/
/** netPROXY package OPC UA. */
#define HIL_COMPONENT_ID_NPXPACKAGE_OPCUA 0x010A0000L

/***********************************************************************************/
/* netPROXY package Profibus DP Slave */
/***********************************************************************************/
/** netPROXY package Profibus DP Slave. */
#define HIL_COMPONENT_ID_NPXPACKAGE_DPS  0x010C0000L

/***********************************************************************************/
/* ECS V5 GCI */
/***********************************************************************************/
/** EtherCAT Slave V5 GCI. */
#define HIL_COMPONENT_ID_ECSV5_GCI       0x010D0000L

/***********************************************************************************/
/* netPROXY package OPCUA to IO-Link Master */
/***********************************************************************************/
/** netPROXY package OPC UA to IO-Link Master. */
#define HIL_COMPONENT_ID_NPXPACKAGE_OPCUA_IOL 0x010F0000L

/***********************************************************************************/
/* netPROXY package OPCUA to netIOL */
/***********************************************************************************/
/** netPROXY package OPC UA to netIOL. */
#define HIL_COMPONENT_ID_NPXPACKAGE_OPCUA_NETIOL 0x01100000L

/***********************************************************************************/
/* netPROXY package netIOL */
/***********************************************************************************/
/** netPROXY package netIOL. */
#define HIL_COMPONENT_ID_NPXPACKAGE_NETIOL 0x01130000L

/***********************************************************************************/
/* MQTT Client Stack */
/***********************************************************************************/
/** MQTT Client Stack. */
#define HIL_COMPONENT_ID_MQTT_CLIENT     0x01150000L

/***********************************************************************************/
/* netPROXY package OPCUA TLV Parser */
/***********************************************************************************/
/** netPROXY package OPC UA TLV Parser. */
#define HIL_COMPONENT_ID_NPXPACKAGE_OPCUATLVPARSER 0x01160000L

/***********************************************************************************/
/* netPROXY package OPCUA AddOn */
/***********************************************************************************/
/** netPROXY package OPC AddOn. */
#define HIL_COMPONENT_ID_NPXPACKAGE_OPCUA_ADDON 0x01170000L

/***********************************************************************************/
/* Driver Phy */
/***********************************************************************************/
/** Driver Phy. */
#define _HIL_UNQ_COMPONENT_ID_DRV_PHY    0x01180000L

/***********************************************************************************/
/* DeviceNet Core */
/***********************************************************************************/
/** DeviceNet Core. */
#define HIL_COMPONENT_ID_DEVNET_CORE     0x01190000L

/***********************************************************************************/
/* DeviceNet Object Library */
/***********************************************************************************/
/** DeviceNet Object Library. */
#define HIL_COMPONENT_ID_DEVNET_OBJECT   0x011A0000L

/***********************************************************************************/
/* DeviceNet GCI Adapter Slave */
/***********************************************************************************/
/** DeviceNet GCI Adapter Slave. */
#define HIL_COMPONENT_ID_DEVNET_GCI_SLAVE 0x011B0000L

/***********************************************************************************/
/* HTTP Web Server */
/***********************************************************************************/
/** HTTP and HTTPS Web Server. */
#define HIL_COMPONENT_ID_HTTP_SERVER     0x011C0000L

/***********************************************************************************/
/* netPROXY package GCI */
/***********************************************************************************/
/** netPROXY package GCI. */
#define HIL_COMPONENT_ID_NPXPACKAGE_GCI  0x011D0000L

/***********************************************************************************/
/* Protocol Detect */
/***********************************************************************************/
/** Protocol Detect. */
#define HIL_COMPONENT_ID_PDETECT         0x011F0000L

/***********************************************************************************/
/* OpenModbus GCI Adapter */
/***********************************************************************************/
/** OpenModbus GCI Adapter. */
#define HIL_COMPONENT_ID_OPEN_MODBUS_GCI 0x01200000L

/***********************************************************************************/
/* IEEE 802.1AS */
/***********************************************************************************/
/** IEEE 802.1AS. */
#define HIL_COMPONENT_ID_IEEE802_1AS     0x01210000L

/***********************************************************************************/
/* TSN Core */
/***********************************************************************************/
/** TSN Core. */
#define HIL_COMPONENT_ID_TSN_CORE        0x01220000L

/***********************************************************************************/
/* NTP */
/***********************************************************************************/
/** Network Time Protocol. */
#define HIL_COMPONENT_ID_NTP             0x01240000L

/***********************************************************************************/
/* TFTP GCI Adapter */
/***********************************************************************************/
/** Tftp GCI Adapter. */
#define HIL_COMPONENT_ID_TFTP_GCI        0x01250000L

/***********************************************************************************/
/* Embedded GCOV GCI Adapter */
/***********************************************************************************/
/** Embedded GCOV GCI Adapter. */
#define HIL_COMPONENT_ID_EMB_GCOV        0x012B0000L

/***********************************************************************************/
/* Firmware Diagnosis */
/***********************************************************************************/
/** Firmware Diagnosis. */
#define HIL_COMPONENT_ID_DIAG            0x012C0000L

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

