/* SPDX-License-Identifier: MIT */
/**************************************************************************************
 *
 * Copyright (c) 2024, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 * Description: Header file of the netX virtual network interface.
 *
 * Changes:
 *
 *   Version   Date        Author   Description
 *   ----------------------------------------------------------------------------------
 *   1        02.01.24    SD        changed licensing terms
 *
 **************************************************************************************/

#ifndef __NETX_TAP__H
#define __NETX_TAP__H

#ifdef CIFXETHERNET

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <stdint.h>

#include <cifxlinux.h>

#define DEFAULT_POLL_TIMEOUT 10 /* polling interval of receiver thread in ms */

/* symbols used when requesting information of the extended status block (->cifXEthGetExentedStatusInformation()) */
#define EXT_INFO_INTF_CONFIG      1  /* request for interface configuration (returns EXT_INFO_MACADDR, EXT_INFO_IPADDR, EXT_INFO_NETMASK, EXT_INFO_GATEWAY) */
#define EXT_INFO_LINKSTATE        2
#define EXT_INFO_MACADDR          3
#define EXT_INFO_IPADDR           4
#define EXT_INFO_NETMASK          5
#define EXT_INFO_GATEWAY          6
#define EXT_INFO_NO_RECVPKT       7
#define EXT_INFO_NO_RCVPKT_DROP   8
#define EXT_INFO_NO_SENDPKT       9
#define EXT_INFO_NO_SENDPKT_DROP 10
#define EXT_STATISTICS           11

typedef struct NETX_ETH_DEV_CFG_Ttag
{
  char             cifx_name[16];
  char             eth_dev_name[IFNAMSIZ];

} NETX_ETH_DEV_CFG_T;

#pragma pack(1)
typedef struct IFCONFIG_Ttag /* configuration of the hardware, indicated by the ETH_INTF_RECV_IP_CONFIG_IND packet */
{
  uint8_t  abEthernetMACAddr[6];

} IFCONFIG_T, *PIFCONFIG_T;
#pragma pack()

typedef struct STATISTIC_Ttag /* ethernet statistic information structure (located in the extended information field) */
{
  uint64_t   ullIfInPkts;      /* the number of packets received                                                         */
  uint64_t   ullIfInDiscards;  /* The number of received packets not passed to the host because of packet queue overflow */
  uint64_t   ullIfOutPkts;     /* The number of sent ethernet frames                                                     */
  uint64_t   ullIfOutDiscards; /* The number of ethernet frames dropped because of no ethernet frame buffer available    */
  uint64_t   ullIfInBytes;     /* the number of bytes received                                                           */
  uint64_t   ullIfOutBytes;    /* the number of bytes transmitted                                                        */

} STATISTIC_T, *PSTATISTIC_T;

typedef struct LINK_STATE_Ttag
{
  uint8_t bLinkState;
} LINK_STATE_T, *PLINK_STATE_T;

void* cifxeth_create_device( NETX_ETH_DEV_CFG_T* config);
void  cifxeth_remove_device( void* handle, NETX_ETH_DEV_CFG_T* config);
void  cifxeth_sys_cleanup(void);

#endif //CIFXETHERNET

#endif /* __NETX_TAP__H */
