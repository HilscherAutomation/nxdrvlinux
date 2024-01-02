/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_CommandRange.h $: *//*!

  \file Hil_CommandRange.h

  The file is the central point where Hilscher packet commands numbers, or ranges,
  will be maintained.

  A range normally contains of 256 (0x100) entries. To reserve a range add a start
  command entry with a gap of 0x100 hex to the previous one start boundary e.g.:
    FOO_BAR_COMMAND_START         = 0x00AFFE00

**************************************************************************************/
#ifndef HIL_COMMANDRANGE_H_
#define HIL_COMMANDRANGE_H_

enum HIL_COMMAND_Etag
{
  /** Illegal command number */
  HIL_COMMAND_INVALID                                 = 0x00000000,

  /* Command numbers 0x00000000 to 0x0000007F reserved, don't use it */

  /** Hilscher global commands used in some TLR tasks */
  HIL_PACKET_COMMAND_START                            = 0x00000080,
      HIL_CMD_END_PROCESS_REQ                             = 0x00000080,
      HIL_CMD_END_PROCESS_CNF                             = 0x00000081,
      HIL_CMD_START_PROCESS_REQ                           = 0x00000082,
      HIL_CMD_START_PROCESS_CNF                           = 0x00000083,
      HIL_CMD_STOP_PROCESS_REQ                            = 0x00000084,
      HIL_CMD_STOP_PROCESS_CNF                            = 0x00000085,
      HIL_CMD_CYCLE_EVENT_REQ                             = 0x00000086,
      HIL_CMD_CYCLE_EVENT_CNF                             = 0x00000087,

  /** Profibus DL (Datalink Layer Protocol) service commands */
  PROFIBUS_DL_PACKET_COMMAND_START                    = 0x00000100,

  /** TCP/IP Stack - IP service commands */
  TCPIP_IP_PACKET_COMMAND_START                       = 0x00000200,

  /** TCP/IP Stack - TCP and UDP service commands */
  TCPIP_TCP_UDP_PACKET_COMMAND_START                  = 0x00000300,

  /** Profibus FSPMS (Fieldbus Service Protocol Machine - Slave) service commands */
  PROFIBUS_FSPMS_PACKET_COMMAND_START                 = 0x00000400,

  /** RPC (Remote procedure calls) service commands */
  RPC_PACKET_COMMAND_START                            = 0x00000500,

  /** PROFINET IO common service commands */
  PNIO_COMMON_PACKET_COMMAND_START                    = 0x00000600,

  /** PROFINET ACP service commands */
  PNIO_ACP_PACKET_COMMAND_START                       = 0x00000800,

  /** PROFINET DCP service commands */
  PNIO_DCP_PACKET_COMMAND_START                       = 0x00000900,

  /** PROFINET EDD service commands */
  PNIO_EDD_PACKET_COMMAND_START                       = 0x00000A00,

  /** Lenze PROFINET IO-Device service commands */
  PNIOD_LENZE_PACKET_COMMAND_START                    = 0x00000B00,
      PNIOD_LENZE_CMD_END_PROCESS_REQ                     = 0x0000B00,
      PNIOD_LENZE_CMD_END_PROCESS_CNF                     = 0x0000B01,

  /** Lenze PROFINET IO-Device service commands */
  PNIOD_LENZE_INIT_PACKET_COMMAND_START               = 0x00000B80,

  /** PROFINET IO-Controller application service commands */
  PNIO_APCTL_PACKET_COMMAND_START                     = 0x00000C00,

  /** PROFINET IO-Device application service commands */
  PNIO_APDEV_PACKET_COMMAND_START                     = 0x00000D00,

  /**  PROFINET CMCTL service commands */
  PNIO_CMCTL_PACKET_COMMAND_START                     = 0x00000E00,

  /**  PROFINET CMDEV service commands */
  PNIO_CMDEV_PACKET_COMMAND_START                     = 0x00000F00,

  /** POWERLINK EPL PDO service commands */
  EPL_PDO_PACKET_COMMAND_START                        = 0x00001000,

  /** POWERLINK EPL SDO service commands */
  EPL_SDO_PACKET_COMMAND_START                        = 0x00001100,

  /** POWERLINK EPL MN service commands */
  EPL_MN_PACKET_COMMAND_START                         = 0x00001200,

  /** POWERLINK EPL NMT service commands */
  EPL_NMT_PACKET_COMMAND_START                        = 0x00001300,

  /** POWERLINK MN Packet Timer service commands */
  EPL_MN_TIMER_PACKET_COMMAND_START                   = 0x00001400,

  /** PROFINET IO-Device DPM Interface service commands */
  PNIOD_DPMIF_PACKET_COMMAND_START                    = 0x00001500,
      PNIOD_DPMIF_CMD_END_PROCESS_REQ                     = 0x00001500,
      PNIOD_DPMIF_CMD_END_PROCESS_CNF                     = 0x00001501,
      PNIOD_DPMIF_PROCESS_ALARM_REQ                       = 0x00001502,
      PNIOD_DPMIF_PROCESS_ALARM_CNF                       = 0x00001503,
      PNIOD_DPMIF_ADD_CHANNEL_DIAG_REQ                    = 0x00001504,
      PNIOD_DPMIF_ADD_CHANNEL_DIAG_CNF                    = 0x00001505,
      PNIOD_DPMIF_ADD_GENERIC_DIAG_REQ                    = 0x00001506,
      PNIOD_DPMIF_ADD_GENERIC_DIAG_CNF                    = 0x00001507,
      PNIOD_DPMIF_REMOVE_DIAG_REQ                         = 0x00001508,
      PNIOD_DPMIF_REMOVE_DIAG_CNF                         = 0x00001509,
      PNIOD_DPMIF_SET_CONFIG_REQ                          = 0x0000150a,
      PNIOD_DPMIF_SET_CONFIG_CNF                          = 0x0000150b,
      PNIOD_DPMIF_CBF_READ_RECORD_REQ                     = 0x00001580,
      PNIOD_DPMIF_CBF_READ_RECORD_CNF                     = 0x00001581,
      PNIOD_DPMIF_CBF_WRITE_RECORD_REQ                    = 0x00001582,
      PNIOD_DPMIF_CBF_WRITE_RECORD_CNF                    = 0x00001583,
      PNIOD_DPMIF_ALARM_IND                               = 0x00001584,
      PNIOD_DPMIF_ALARM_RES                               = 0x00001585,
      PNIOD_DPMIF_CBF_STATION_NAME_IND                    = 0x00001586,
      PNIOD_DPMIF_CBF_STATION_NAME_RES                    = 0x00001587,
      PNIOD_DPMIF_CBF_STATION_TYPE_IND                    = 0x00001588,
      PNIOD_DPMIF_CBF_STATION_TYPE_RES                    = 0x00001589,

  /** UDP Debug Client service commands */
  DEBUG_CLIENT_PACKET_COMMAND_START                   = 0x00001600,
      DEBUG_CLIENT_CMD_END_PROCESS_REQ                    = 0x00001600,
      DEBUG_CLIENT_CMD_END_PROCESSS_CNF                   = 0x00001601,
      DEBUG_CLIENT_CMD_SEND_DEBUG_STRING_REQ              = 0x00001602,
      DEBUG_CLIENT_CMD_SEND_DEBUG_STRING_CNF              = 0x00001603,

  /** Example task 1 service commands */
  EXAMPLETASK1_PACKET_COMMAND_START                   = 0x00001700,
      EXAMPLETASK1_CMD_END_PROCESS_REQ                    = 0x00001700,
      EXAMPLETASK1_CMD_END_PROCESS_CNF                    = 0x00001701,
      EXAMPLETASK1_CMD_TEST_REQ                           = 0x00001702,
      EXAMPLETASK1_CMD_TEST_CNF                           = 0x00001703,

  /** Example task 2 service commands */
  EXAMPLETASK2_PACKET_COMMAND_START                   = 0x00001720,
      EXAMPLETASK2_CMD_END_PROCESS_REQ                    = 0x00001720,
      EXAMPLETASK2_CMD_END_PROCESS_CNF                    = 0x00001721,
      EXAMPLETASK2_CMD_TEST_REQ                           = 0x00001722,
      EXAMPLETASK2_CMD_TEST_CNF                           = 0x00001723,

  /** Example task 3 service commands */
  EXAMPLETASK3_PACKET_COMMAND_START                   = 0x00001740,
      EXAMPLETASK3_CMD_END_PROCESS_REQ                    = 0x00001740,
      EXAMPLETASK3_CMD_END_PROCESS_CNF                    = 0x00001741,
      EXAMPLETASK3_CMD_TEST_REQ                           = 0x00001742,
      EXAMPLETASK3_CMD_TEST_CNF                           = 0x00001743,

  /** Ethernet/IP Encapsulation task service commands */
  EIP_ENCAP_PACKET_COMMAND_START                      = 0x00001800,

  /** EtherCAT service commands (Part 1) */
  ECAT_PACKET_COMMAND_START                           = 0x00001900,

  /** Ethernet/IP Object task service commands */
  EIP_OBJECT_PACKET_COMMAND_START                     = 0x00001A00,

  /** EtherCAT service commands (Part 2) */
  ECAT_2_PACKET_COMMAND_START                         = 0x00001B00,

  /** iCon-L main task commands */
  ICONL_RUN_PACKET_COMMAND_START                      = 0x00001C00,
      ICONL_RUN_DSPSRUN_REQ                               = 0x00001C00,
      ICONL_RUN_DSPSRUN_CNF                               = 0x00001C01,

  /** iCon-L Timer task commands */
  ICONL_TIMER_PACKET_COMMAND_START                    = 0x00001D00,
      ICONL_TIMER_DSPSTIMER_REQ                           = 0x00001D00,
      ICONL_TIMER_DSPSTIMER_CNF                           = 0x00001D01,

  /** Middle ware system task commands.
   * The commands are described in the Hil_SystemCmd.h file. */
  MID_SYS_PACKET_COMMAND_START                        = 0x00001E00,

  /* PROFINET IO-Device Interface task service commands */
  PNS_IF_PACKET_COMMAND_START                         = 0x00001F00,

  /* Middle ware task (back end task) */
  MID_DBG_PACKET_COMMAND_START                        = 0x00002000,
      MID_DBG_READ_MEMORY_REQ                             = 0x00002000,
      MID_DBG_READ_MEMORY_CNF                             = 0x00002001,
      MID_DBG_WRITE_MEMORY_REQ                            = 0x00002002,
      MID_DBG_WRITE_MEMORY_CNF                            = 0x00002003,

      MID_DBG_CALL_FUNC_REQ                               = 0x00002020,
      MID_DBG_CALL_FUNC_CNF                               = 0x00002021,

      MID_DBG_SET_SW_BREAKPOINT_REQ                       = 0x00002040,
      MID_DBG_SET_SW_BREAKPOINT_CNF                       = 0x00002041,
      MID_DBG_CLR_SW_BREAKPOINT_REQ                       = 0x00002042,
      MID_DBG_CLR_SW_BREAKPOINT_CNF                       = 0x00002043,
      MID_DBG_SET_HW_BREAKPOINT_REQ                       = 0x00002044,
      MID_DBG_SET_HW_BREAKPOINT_CNF                       = 0x00002045,
      MID_DBG_CLR_HW_BREAKPOINT_REQ                       = 0x00002046,
      MID_DBG_CLR_HW_BREAKPOINT_CNF                       = 0x00002047,
      MID_DBG_GET_SW_BREAKPOINT_IDX_REQ                   = 0x00002048,
      MID_DBG_GET_SW_BREAKPOINT_IDX_CNF                   = 0x00002049,
      MID_DBG_GET_HW_BREAKPOINT_IDX_REQ                   = 0x0000204A,
      MID_DBG_GET_HW_BREAKPOINT_IDX_CNF                   = 0x0000204B,
      MID_DBG_REACHED_HW_BREAKPOINT_IND                   = 0x0000204C,
      MID_DBG_REACHED_HW_BREAKPOINT_RES                   = 0x0000204D,
      MID_DBG_REACHED_SW_BREAKPOINT_IND                   = 0x0000204E,
      MID_DBG_REACHED_SW_BREAKPOINT_RES                   = 0x0000204F,

      MID_DBG_READ_REGS_REQ                               = 0x00002060,
      MID_DBG_READ_REGS_CNF                               = 0x00002061,
      MID_DBG_WRITE_REG_REQ                               = 0x00002062,
      MID_DBG_WRITE_REG_CNF                               = 0x00002063,

      MID_DBG_SUSPEND_TASK_REQ                            = 0x00002080,
      MID_DBG_SUSPEND_TASK_CNF                            = 0x00002081,
      MID_DBG_CONTINUE_TASK_REQ                           = 0x00002082,
      MID_DBG_CONTINUE_TASK_CNF                           = 0x00002083,

      MID_DBG_CONNECT_REQ                                 = 0x000020A0,
      MID_DBG_CONNECT_CNF                                 = 0x000020A1,
      MID_DBG_DISCONNECT_REQ                              = 0x000020A2,
      MID_DBG_DISCONNECT_CNF                              = 0x000020A3,

  /** AS-Interface ECTRL task */
  ASI_ECTRL_PACKET_COMMAND_START                      = 0x00002100,
      ASI_ECTRL_CMD_SET_OFF_PHASE_REQ                     = 0x00002100,
      ASI_ECTRL_CMD_SET_OFF_PHASE_CNF                     = 0x00002101,
      ASI_ECTRL_CMD_SET_OP_MODE_REQ                       = 0x00002102,
      ASI_ECTRL_CMD_SET_OP_MODE_CNF                       = 0x00002103,
      ASI_ECTRL_CMD_SET_DATA_EXCH_REQ                     = 0x00002104,
      ASI_ECTRL_CMD_SET_DATA_EXCH_CNF                     = 0x00002105,
      ASI_ECTRL_CMD_SET_AUTO_ADDR_REQ                     = 0x00002106,
      ASI_ECTRL_CMD_SET_AUTO_ADDR_CNF                     = 0x00002107,
      ASI_ECTRL_CMD_WRITE_PARAM_REQ                       = 0x00002108,
      ASI_ECTRL_CMD_WRITE_PARAM_CNF                       = 0x00002109,
      ASI_ECTRL_CMD_WRITE_ID1_CODE_REQ                    = 0x0000210A,
      ASI_ECTRL_CMD_WRITE_ID1_CODE_CNF                    = 0x0000210B,
      ASI_ECTRL_CMD_CHANGE_ADDR_REQ                       = 0x0000210C,
      ASI_ECTRL_CMD_CHANGE_ADDR_CNF                       = 0x0000210D,
      ASI_ECTRL_CMD_EXECUTE_CMD_REQ                       = 0x0000210E,
      ASI_ECTRL_CMD_EXECUTE_CMD_CNF                       = 0x0000210F,
      ASI_ECTRL_CMD_GET_STATE_REQ                         = 0x00002110,
      ASI_ECTRL_CMD_GET_STATE_CNF                         = 0x00002111,
      ASI_ECTRL_CMD_GET_ACT_CONFIG_REQ                    = 0x00002112,
      ASI_ECTRL_CMD_GET_ACT_CONFIG_CNF                    = 0x00002113,
      ASI_ECTRL_CMD_GET_PERM_CONFIG_REQ                   = 0x00002114,
      ASI_ECTRL_CMD_GET_PERM_CONFIG_CNF                   = 0x00002115,
      ASI_ECTRL_CMD_SET_PERM_PARAM_REQ                    = 0x00002116,
      ASI_ECTRL_CMD_SET_PERM_PARAM_CNF                    = 0x00002117,
      ASI_ECTRL_CMD_STORE_ACT_PARAM_REQ                   = 0x00002118,
      ASI_ECTRL_CMD_STORE_ACT_PARAM_CNF                   = 0x00002119,
      ASI_ECTRL_CMD_SET_PERM_CONFIG_REQ                   = 0x0000211A,
      ASI_ECTRL_CMD_SET_PERM_CONFIG_CNF                   = 0x0000211B,
      ASI_ECTRL_CMD_STORE_ACT_CONFIG_REQ                  = 0x0000211C,
      ASI_ECTRL_CMD_STORE_ACT_CONFIG_CNF                  = 0x0000211D,
      ASI_ECTRL_CMD_READ_ID_STR_REQ                       = 0x0000211E,
      ASI_ECTRL_CMD_READ_ID_STR_CNF                       = 0x0000211F,
      ASI_ECTRL_CMD_READ_PARAM_STR_REQ                    = 0x00002120,
      ASI_ECTRL_CMD_READ_PARAM_STR_CNF                    = 0x00002121,
      ASI_ECTRL_CMD_READ_DIAG_STR_REQ                     = 0x00002122,
      ASI_ECTRL_CMD_READ_DIAG_STR_CNF                     = 0x00002123,
      ASI_ECTRL_CMD_WRITE_PARAM_STR_REQ                   = 0x00002124,
      ASI_ECTRL_CMD_WRITE_PARAM_STR_CNF                   = 0x00002125,
      ASI_ECTRL_CMD_READ_IN_DATA_REQ                      = 0x00002126,
      ASI_ECTRL_CMD_READ_IN_DATA_CNF                      = 0x00002127,
      ASI_ECTRL_CMD_WRITE_OUT_DATA_REQ                    = 0x00002128,
      ASI_ECTRL_CMD_WRITE_OUT_DATA_CNF                    = 0x00002129,
      ASI_ECTRL_CMD_READ_ANLG_IN_DATA_REQ                 = 0x0000212A,
      ASI_ECTRL_CMD_READ_ANLG_IN_DATA_CNF                 = 0x0000212B,
      ASI_ECTRL_CMD_WRITE_ANLG_OUT_DATA_REQ               = 0x0000212C,
      ASI_ECTRL_CMD_WRITE_ANLG_OUT_DATA_CNF               = 0x0000212D,
      ASI_ECTRL_CMD_READ_SERIAL_STRING_REQ                = 0x0000212E,
      ASI_ECTRL_CMD_READ_SERIAL_STRING_CNF                = 0x0000212F,
      ASI_ECTRL_CMD_WRITE_SERIAL_STRING_REQ               = 0x00002130,
      ASI_ECTRL_CMD_WRITE_SERIAL_STRING_CNF               = 0x00002131,

      ASI_ECTRL_CMD_CYCLE_EVENT_REQ                       = 0x000021F0,
      ASI_ECTRL_CMD_CYCLE_EVENT_CNF                       = 0x000021F1,

  /**  Profibus FSPMM (Fieldbus Service Protocol Machine - Master) service commands */
  PROFIBUS_FSPMM_PACKET_COMMAND_START                 = 0x00002200,

  /** LLDP service commands */
  LLDP_PACKET_COMMAND_START                           = 0x00002300,

  /** MibDatabase task (part of the SNMP-Implementation) service commands */
  MIB_DATABASE_COMMAND_START                          = 0x00002400,

  /** SnmpServer task (part of the SNMP-Implementation) service commands */
  SNMP_SERVER_COMMAND_START                           = 0x00002500,

  /** Ecat Cyclic service commands */
  ECAT_CYCLIC_COMMAND_START                           = 0x00002600,

  /** DDL ENPDDL task service commands. OEM Project: J060219 */
  DDL_ENPDDL_PACKET_COMMAND_START                     = 0x00002700,
      DDL_ENPDDL_CMD_CYCLE_EVENT_REQ                      = 0x00002700,
      DDL_ENPDDL_CMD_CYCLE_EVENT_CNF                      = 0x00002701,
      DDL_ENPDDL_CMD_TIMEOUT_AUTO_ADR_REQ                 = 0x00002702,
      DDL_ENPDDL_CMD_TIMEOUT_AUTO_ADR_CNF                 = 0x00002703,
      DDL_ENPDDL_CMD_TIMEOUT_MANUAL_ADR_REQ               = 0x00002704,
      DDL_ENPDDL_CMD_TIMEOUT_MANUAL_ADR_CNF               = 0x00002705,
      DDL_ENPDDL_CMD_TIMEOUT_PARAMETER_REQ                = 0x00002706,
      DDL_ENPDDL_CMD_TIMEOUT_PARAMETER_CNF                = 0x00002707,
      DDL_ENPDDL_CMD_TIMEOUT_DATA_REQ                     = 0x00002708,
      DDL_ENPDDL_CMD_TIMEOUT_DATA_CNF                     = 0x00002709,
      DDL_ENPDDL_CMD_TIMEOUT_WAIT_SEND_REQ                = 0x0000270A,
      DDL_ENPDDL_CMD_TIMEOUT_WAIT_SEND_CNF                = 0x0000270B,
      DDL_ENPDDL_CMD_TIMEOUT_ERROR_REQ                    = 0x0000270C,
      DDL_ENPDDL_CMD_TIMEOUT_ERROR_CNF                    = 0x0000270D,
      DDL_ENPDDL_CMD_CAN_DL_BUS_OFF_DELAY_REQ             = 0x0000270E,
      DDL_ENPDDL_CMD_CAN_DL_BUS_OFF_DELAY_CNF             = 0x0000270F,


  /** CANopen Master task service commands */
  CANOPEN_MASTER_PACKET_COMMAND_START                 = 0x00002800,

  /** CANopen Slave task service commands */
  CANOPEN_SLAVE_PACKET_COMMAND_START                  = 0x00002900,
      CANOPEN_SLAVE_CMD_CYCLE_EVENT_REQ                   = 0x000029F0,
      CANOPEN_SLAVE_CMD_CYCLE_EVENT_CNF                   = 0x000029F1,

  /** CAN DL task service commands */
  CAN_DL_PACKET_COMMAND_START                         = 0x00002A00,

  /** Commands used by Mid_Sys and routers to specify message routing for applications not knowing the instance */
  MID_SYS_LOG_PACKET_COMMAND_START                    = 0x00002B00,

  /** Commands used by the Object Dictionary DPM adapter to initialize the link between stack and DPM (used by EcatDPM task as well) */
  DPM_OD2_PACKET_COMMAND_START                        = 0x00002C00,

  /** DeviceNet Slave Filedbus application layer task */
  DNS_FAL_PACKET_COMMAND_START                        = 0x00002D00,

  /** CANopen Slave application task */
  CANOPEN_APS_PACKET_COMMAND_START                    = 0x00002E00,
      CANOPEN_APS_CMD_CYCLE_EVENT_REQ                     = 0x00002E00,
      CANOPEN_APS_CMD_CYCLE_EVENT_CNF                     = 0x00002E01,

  /** Common application packets
   * The commands are described in the Hil_ApplicationCmd.h file. */
  DIAG_INFO_PACKET_COMMAND_START                      = 0x00002F00,

  /** Profibus APM task commands */
  PROFIBUS_APM_PACKET_COMMAND_START                   = 0x00003000,

  /** Profibus APS task commands */
  PROFIBUS_APS_PACKET_COMMAND_START                   = 0x00003100,

  /** TBD */
  SERCOSIII_SL_COM_PACKET_COMMAND_START               = 0x00003200,

  /** TBD */
  SERCOSIII_SL_RTD_PACKET_COMMAND_START               = 0x00003300,

  /** TBD */
  SERCOSIII_SL_SVC_PACKET_COMMAND_START               = 0x00003400,

  /** TBD */
  SERCOSIII_SL_AP_PACKET_COMMAND_START                = 0x00003500,

  /** Ethernet/IP APS (Slave/Adapter) task service commands */
  EIP_APS_PACKET_COMMAND_START                        = 0x00003600,

  /** Ethernet/IP APM (Master/Scanner) task service commands */
  EIP_APM_PACKET_COMMAND_START                        = 0x00003700,

  /** DeviceNet FAL task service commands */
  DEVNET_FAL_PACKET_COMMAND_START                     = 0x00003800,

  /** DeviceNet APM task service commands */
  DEVNET_AP_PACKET_COMMAND_START                      = 0x00003900,

  /** CANopen Master application task service commands */
  CANOPEN_APM_PACKET_COMMAND_START                    = 0x00003A00,

  /** Ethernet Interface task service commands */
  ETH_INTF_PACKET_COMMAND_START                       = 0x00003B00,

  /** PNS_32BITIO AP task service commands */
  PNS_32BITIO_PACKET_COMMAND_START                    = 0x00003C00,

  /** PNS_4BITIO AP task service commands */
  PNS_4BITIO_PACKET_COMMAND_START                     = 0x00003D00,

  /** Mid Startup task service commands */
  MID_STARTUP_PACKET_COMMAND_START                    = 0x00003E00,

  /** Open modbus task service commands */
  OMB_OMBTASK_PACKET_COMMAND_START                    = 0x00003F00,

  /** Start value of OMB OMBAPTASK service commands */
  OMB_OMBAPTASK_PACKET_COMMAND_START                  = 0x00004000,

  /** DeviceNet Slave application layer task service commands */
  DNS_APS_PACKET_COMMAND_START                        = 0x00004100,

  /** Profibus MPI application layer task service commands */
  PROFIBUS_MPI_AP_PACKET_COMMAND_START                = 0x00004200,

  /** Profibus MPI layer task service commands */
  PROFIBUS_MPI_PACKET_COMMAND_START                   = 0x00004300,

  /** Profibus FSPMM2 layer task service commands */
  PROFIBUS_FSPMM2_PACKET_COMMAND_START                = 0x00004400,

  /** CC-Link Slave task service commands */
  CCLINK_SLAVE_PACKET_COMMAND_START                   = 0x00004500,

  /** CC-Link Slave application task service commands */
  CCLINK_APS_PACKET_COMMAND_START                     = 0x00004600,

  /** Modbus RTU task */
  MODBUS_RTU_PACKET_COMMAND_START                     = 0x00004700,

  /** Sercos III Master CP task */
  SIII_MA_CP_PACKET_COMMAND_START                     = 0x00004800,

  /** Sercos III Master Svc task */
  SIII_MA_SVC_PACKET_COMMAND_START                    = 0x00004900,

  /** Sercos III Master AP task */
  SIII_MA_AP_PACKET_COMMAND_START                     = 0x00004A00,

  /** Summary of module load packets */
  RCX_MODLOAD_PACKET_COMMAND_START                    = 0x00004B00,

  /** Summary of SSIO packets */
  SSIO_COMMAND_START                                  = 0x00004C00,

  /** Summary of SSIO packets */
  SSIO_AP_COMMAND_START                               = 0x00004D00,

  /** Summary of Memory Mapping packets */
  MEMORY_MAP_COMMAND_START                            = 0x00004E00,

  /** Summary of TCPIP SOCKIF packets */
  TCPIP_SOCKIF_PACKET_COMMAND_START                   = 0x00004F00,

  /** Sercos III Master NRT task */
  SIII_MA_NRT_PACKET_COMMAND_START                    = 0x00005000,

  /** NetScript task */
  NETSCRIPT_COMMAND_START                             = 0x00005100,

  /** AS-Interface Master task */
  ASI_MASTER_PACKET_COMMAND_START                     = 0x00005200,

  /** AS-Interface Master application task */
  ASI_APM_PACKET_COMMAND_START                        = 0x00005300,

  /** CompoNet Slave task */
  COMPONET_SLAVE_COMMAND_START                        = 0x00005400,

  /** CompoNet Slave application task */
  COMPONET_SLAVE_AP_COMMAND_START                     = 0x00005500,

  /** ASCII Protocol task */
  ASCII_COMMAND_START                                 = 0x00005600,

  /** ASCII application task */
  ASCII_AP_COMMAND_START                              = 0x00005700,

  /** Summary of Ecs SoE commands */
  ECAT_SOE_COMMAND_START                              = 0x00005800,

  /** Summary of netPLC (Codesys Variant) commands */
  NPLC_CODESYS_AP_COMMAND_START                       = 0x00005900,

  /** Summary of SercosIII Slave Stack IDN commands */
  SERCOSIII_SL_IDN_PACKET_COMMAND_START               = 0x00005A00,

  /** Summary of Item Server task commands */
  ITEM_SERVER_PACKET_COMMAND_START                    = 0x00005B00,

  /** Summary of DF1 stack task commands */
  DF1_PACKET_COMMAND_START                            = 0x00005C00,

  /** Summary of DF1 AP task commands */
  DF1_AP_PACKET_COMMAND_START                         = 0x00005D00,

  /** Summary of 3964R stack task commands */
  P3964R_PACKET_COMMAND_START                         = 0x00005E00,

  /** Summary of 3964R AP task commands */
  P3964R_AP_PACKET_COMMAND_START                      = 0x00005F00,

  /** Summary of ISAGraf AP task commands */
  ISAGRAF_AP_PACKET_COMMAND_START                     = 0x00006000,

  /** Summary of IO Signals task commands */
  IO_SIGNALS_PACKET_COMMAND_START                     = 0x00006100,

  /** RTR UART task commands */
  RTR_UART_PACKET_COMMAND_START                       = 0x00006200,

  /** RFC1006 task commands */
  RFC1006_AP_PACKET_COMMAND_START                     = 0x00006300,

  /** RFC1006 task commands */
  RFC1006_STACK_PACKET_COMMAND_START                  = 0x00006400,

  /** Ethernet/IP DLR task commands */
  EIP_DLR_PACKET_COMMAND_START                        = 0x00006500,

  /** SERCOS III Slave NRT task commands */
  SERCOSIII_SL_NRT_PACKET_COMMAND_START               = 0x00006600,

  /** OEM Device - can be used for all customer specific tasks */
  OEM_DEVICE_PACKET_COMMAND_START                     = 0x00006700,

  /** TCPIP AP task */
  TCPIP_AP_PACKET_COMMAND_START                       = 0x00006800,

  /** FODMI task */
  FODMI_PACKET_COMMAND_START                          = 0x00006900,

  /** ODv3 task */
  ODV3_PACKET_COMMAND_START                           = 0x00006A00,

  /** PROFIDRIVE - GSM  task */
  PROFIDRIVE_PACKET_COMMAND_START                     = 0x00006B00,

  /** PROFIDRIVE - PA  task */
  PROFIDRIVE_PA_PACKET_COMMAND_START                  = 0x00006C00,

  /** PROFIDRIVE - OD  task */
  PROFIDRIVE_OD_PACKET_COMMAND_START                  = 0x00006D00,

  /** PROFIDRIVE - AP task */
  PROFIDRIVE_AP_PACKET_COMMAND_START                  = 0x00006E00,

  /** VARAN Client - task */
  VARAN_CLIENT_PACKET_COMMAND_START                   = 0x00006F00,

  /** VARAN Client - AP task */
  VARAN_CLIENT_AP_PACKET_COMMAND_START                = 0x00007000,

  /** PROFINET RTA task */
  PROFINET_RTA_PACKET_COMMAND_START                   = 0x00007100,

  /** Modbus RTU Peripheral task */
  MBR_PERIPH_PACKET_COMMAND_START                     = 0x00007200,

  /** CODE SYS PLC Handler AP task */
  CODESYS_AP_PLCHANDLER_PACKET_COMMAND_START          = 0x00007300,

  /** CODE SYS PLC Handler task */
  CODESYS_PLCHANDLER_PACKET_COMMAND_START             = 0x00007400,

  /** PNS INX AP task */
  PNSINX_AP_PACKET_COMMAND_START                      = 0x00007F00,

  /** SercosIII SIP task */
  SIII_SIP_PACKET_COMMAND_START                       = 0x00008000,

  /** Packets for sercos test master firmware */
  SIII_MA_TEST_PACKET_COMMAND_START                   = 0x00008100,

  /** Packets for Powerlink MN Packet task */
  EPLMN_PCK_PACKET_COMMAND_START                      = 0x00008200,

  /** Packets for Powerlink MN AP task */
  EPLMN_AP_PACKET_COMMAND_START                       = 0x00008300,

  /** Packets for SmartWire Master task */
  SMARTWIRE_MASTER_PACKET_COMMAND_START               = 0x00008400,

  /** Packets for POWERLINK TestMaster */
  POWERLINK_TEST_MASTER_PACKET_START                  = 0x00008500,

  /** Packets for PROFINET IO common */
  PNIO_COMMON_PACKET_START                            = 0x00008600,

  /** Packets for Trivial File Server API */
  TRIVIAL_FILE_SERVER_API_PACKET_START                = 0x00008700,

  /** Packets for netProxy API */
  NPX_API_PACKET_START                                = 0x00008800,

  /** Second set of packets for sercos master CP task */
  SIII_MA_CP_PACKET_2ND_SET_COMMAND_START             = 0x00008900,

  /** Commands for sercos master Auto configure task */
  SIII_MA_ACFG_PACKET_START                           = 0x00008A00,

  /** Commands for TFTP Stack task */
  TFTP_STACK_PACKET_START                             = 0x00008B00,

  /** Commands for TFTP application task */
  TFTP_APP_PACKET_START                               = 0x00008C00,

  /** Commands for ECS AOE task */
  ECS_AOE_PACKET_START                                = 0x00008D00,

  /**  Commands for the PTP stack of EtherNet/IP */
  EIP_PTP_PACKET_COMMAND_START                        = 0x00008E00,

  /** Commands for sercos master S/IP client task */
  SIII_MA_SIP_PACKET_START                            = 0x00008F00,

  /** Commands for sercos master SMP task */
  SIII_MA_SMP_PACKET_START                            = 0x00009000,

  /** Commands for IO-Link master DL task */
  IOLM_DL_PACKET_START                                = 0x00009100,

  /** Commands for IO-Link master AL task */
  IOLM_AL_PACKET_START                                = 0x00009200,

  /** Commands for SIF */
  SIF_PACKET_START                                    = 0x00009300,

  /** Commands for Profinet IO-Controller AP task */
  PNM_AP_CFG_PACKET_COMMAND_START                     = 0x00009400,

  /** Commands for ECS FoE */
  ECS_FOE_PACKET_COMMAND_START                        = 0x00009500,

  /** Socket Api Commands */
  SOCK_PACKET_COMMAND_START                           = 0x00009600,

  /** ECMv4 API */
  ECM_COMMAND_START                                   = 0x00009700,

  /** ECMv4 API - MBX */
  ECM_MBX_COMMAND_START                               = 0x00009800,

  /** ECMv4 API - FoE */
  ECM_FOE_COMMAND_START                               = 0x00009900,

  /** ECMv4 API - CoE */
  ECM_COE_COMMAND_START                               = 0x00009A00,

  /** ECMv4 API - SoE */
  ECM_SOE_COMMAND_START                               = 0x00009B00,

  /** ECMv4 API - EoE */
  ECM_EOE_COMMAND_START                               = 0x00009C00,

  /** ECMv4 API - AoE */
  ECM_AOE_COMMAND_START                               = 0x00009D00,

  /** ECMv4 API - Interface */
  ECM_IF_COMMAND_START                                = 0x00009E00,

  /** ECMv4 API - AP */
  ECM_AP_COMMAND_START                                = 0x00009F00,

  /** Ethernet/IP Class1 task service commands */
  EIP_CL1_PACKET_COMMAND_START                        = 0x0000A000,

  /** PLS AP task service commands */
  PLS_AP_PACKET_COMMAND_START                         = 0x0000A100,

  /** PLS IF task service commands */
  PLS_IF_PACKET_COMMAND_START                         = 0x0000A200,

  /** Command Table task service commands */
  CMDTBL_COMMAND_START                                = 0x0000A300,

  /** DPM Bridge task service commands */
  DPM_BRIDGE_PACKET_COMMAND_START                     = 0x0000A400,

  /** Base Firmware application task service commands */
  BASEFW_AP_PACKET_COMMAND_START                      = 0x0000A500,

  /** CCLink IE service commands */
  CCLIES_COMMAND_START                                = 0x0000A600,

  /** CCLink IE interface service commands */
  CCLIES_IF_COMMAND_START                             = 0x0000A700,

  /** CCLink IE application task service commands */
  CCLIES_AP_COMMAND_START                             = 0x0000A800,

  /** IO-Link Test Protocol task service commands */
  IOLT_COMMAND_START                                  = 0x0000A900,

  /** CCLink IE Field Basic service commands */
  CCLIEFB_COMMAND_START                               = 0x0000AA00,

  /** CCLink IE Field Basic application task service commands */
  CCLIEFB_AP_COMMAND_START                            = 0x0000AB00,

  /** CC-Link IE Field Basic master service commands */
  CCLIEFBM_COMMAND_START                              = 0x0000AC00,

  /** Generic application task service commands */
  GENERIC_AP_TASK_COMMAND_START                       = 0x0000AD00,

  /** Generic communication interface service commands
   * The commands are described in the Hil_GenericCommunicationInterface.h file. */
  GENERIC_COMMUNICATION_INTERFACE_COMMAND_START       = 0x0000AE00,

  /** Web interface service commands */
  WEB_INTERFACE_COMMAND_START                         = 0x0000AF00,

  /** Authentication manager service commands  */
  AUTH_INTERFACE_COMMAND_START                        = 0x0000B000,

  /** DeviceNet Slave V4/V5 service commands  */
  DNS_COMMAND_START                                   = 0x0000B100,

  /** Protocol Detect commands */
  PDETECT_COMMAND_START                               = 0x0000B200,

  /** IEEE 802.1AS component commands */
  IEEE_802_1_AS_COMMAND_START                         = 0x0000B300,

  /** TSN Core component commands */
  TSN_CORE_COMMAND_START                              = 0x0000B400,



  /* ^^^^ Add new error codes above this line ^^^^ */

  /** Commands for EtherCAT master service commands */
  ETHERCAT_MASTER_V2_X_V3_X_AP_PACKET_START           = 0x00640000,
  ETHERCAT_MASTER_V2_X_V3_X_PACKET_START              = 0x00650000,

  /** Start value where a USER may define its own service commands */
  USER_PACKET_COMMAND_START                           = 0x01000000,
  USER_PACKET_COMMAND_END                             = 0x01FFFFFF,

  /* Command numbers from 0x02000000 are reserved, don't use it */

};

typedef enum HIL_COMMAND_Etag       HIL_COMMAND_E;

#endif  /* HIL_COMMANDRANGE_H_ */
