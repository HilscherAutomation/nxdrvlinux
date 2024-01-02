/**************************************************************************************
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $Id:  $: *//*!

  \file DrvEth_GCI_API.h
  DrvEth GCI Application Programming Interface

Changes:
  Date          Description
  -----------------------------------------------------------------------------------
  2018-08-8     created
**************************************************************************************/

#ifndef __DRVETH_GCI_API_H
#define __DRVETH_GCI_API_H

#include <stdint.h>
#include "Hil_Compiler.h"
#include "Hil_Packet.h"

/*! \defgroup drveth_gci_public_api Raw Ethernet Public API
 *
 * This section describes the Packet API between the host application and
 * DrvEth GCI Adapter.
 * @{
 */

/*! Packet Command Codes */
typedef enum DRVETH_GCI_CMD_Etag
{
  /*! Event indication */
  DRVETH_GCI_CMD_EVENT_IND =          0x00003B20,
  /*! Event response */
  DRVETH_GCI_CMD_EVENT_RSP =          0x00003B21,

  /*! Send ethernet frame request */
  DRVETH_GCI_CMD_SEND_ETH_FRAME_REQ = 0x00003B22,
  /*! Send ethernet frame confirmation */
  DRVETH_GCI_CMD_SEND_ETH_FRAME_CNF = 0x00003B23,

  /*! Received ethernet frame indication */
  DRVETH_GCI_CMD_RECV_ETH_FRAME_IND = 0x00003B24,
  /*! Received ethernet frame response */
  DRVETH_GCI_CMD_RECV_ETH_FRAME_RSP = 0x00003B25,

  /*! Register for multicast reception for certain group address request */
  DRVETH_GCI_CMD_SET_MULTICAST_SINGLE_REQ = 0x00003B26,
  /*! Register for multicast reception for certain group address confirmation */
  DRVETH_GCI_CMD_SET_MULTICAST_SINGLE_CNF = 0x00003B27,

  /*! Unregister for multicast reception for certain group address request */
  DRVETH_GCI_CMD_CLR_MULTICAST_SINGLE_REQ = 0x00003B28,
  /*! Unregister for multicast reception for certain group address confirmation */
  DRVETH_GCI_CMD_CLR_MULTICAST_SINGLE_CNF = 0x00003B29,
} DRVETH_GCI_CMD_E;


/*! Size of an ethernet mac address */
#define DRVETH_GCI_ETH_ADDR_SIZE          (6)

/*! Extended Status Area content. */
typedef __HIL_PACKED_PRE struct DRVETH_GCI_EXTENDED_STATE_Ttag
{
  /*!The mac address of the ethernet interface
   * This is set to zero if ethernet mac is not available. */
  uint8_t abMacAddress[DRVETH_GCI_ETH_ADDR_SIZE];
  /*! The current mau type according IANA */
  uint8_t bMautype;
  /*! alignment */
  uint8_t  bPadding;
  /*! Reserved. */
  uint32_t   ulReserved0;
  /*! Reserved. */
  uint32_t   ulReserved1;
  /*! Reserved. */
  uint32_t   ulReserved2;
  /*! The number of received packets passed to the host. */
  uint64_t   ullIfInPkts;
  /*! The number of received packets not passed to the host because of
   *  packet queue overflow. */
  uint64_t   ullIfInDiscards;
  /*! The number of sent ethernet frames. */
  uint64_t   ullIfOutPkts;
  /*! The number of ethernet frames dropped because of no
   *  ethernet frame buffer available. */
  uint64_t   ullIfOutDiscards;
  /*! The number of bytes received. */
  uint64_t   ullIfInBytes;
  /*! The number of bytes transmitted. */
  uint64_t   ullIfOutBytes;
} __HIL_PACKED_POST DRVETH_GCI_EXTENDED_STATE_T;


/*! Minimum Ethernet Frame length (without FCS). */
#define DRVETH_GCI_MIN_ETH_FRAME_SIZE  (60)
/*! Maximum Ethernet Frame length (without FCS). */
#define DRVETH_GCI_MAX_ETH_FRAME_SIZE  (1518)

/*! Ethernet frame data type */
typedef __HIL_PACKED_PRE struct DRVETH_GCI_ETH_FRAME_Ttag
{
  /*! Destination Mac Address */
  uint8_t abDestMacAddr[DRVETH_GCI_ETH_ADDR_SIZE];
  /*! Source Mac Address */
  uint8_t abSrcMacAddr[DRVETH_GCI_ETH_ADDR_SIZE];
  /*! Remaining Data */
  uint8_t  abData[DRVETH_GCI_MAX_ETH_FRAME_SIZE - 2 * DRVETH_GCI_ETH_ADDR_SIZE];
} __HIL_PACKED_POST DRVETH_GCI_ETH_FRAME_T;


/*! \defgroup drveth_gci_service_sendframe Send Ethernet Frame Service
 *
 * This service shall be used by the Host Application to send an ethernet
 * frame.
 *
 * @{
 */

/*! Send Ethernet frame request packet */
typedef __HIL_PACKED_PRE struct DRVETH_GCI_SEND_ETH_FRAME_REQ_Ttag
{
  /*! Packet header */
  HIL_PACKET_HEADER_T                 tHead;
  /*! Packet data */
  DRVETH_GCI_ETH_FRAME_T              tData;
} __HIL_PACKED_POST DRVETH_GCI_SEND_ETH_FRAME_REQ_T;

/*! Send Ethernet frame confirmation packet */
typedef HIL_EMPTY_PACKET_T  DRVETH_GCI_SEND_ETH_FRAME_CNF_T;

/*! Send Ethernet frame packet union */
typedef union DRVETH_GCI_SEND_ETH_FRAME_PCK_Ttag
{
  /*! Request */
  DRVETH_GCI_SEND_ETH_FRAME_REQ_T tReq;
  /*! Confirmation */
  DRVETH_GCI_SEND_ETH_FRAME_CNF_T tCnf;
} DRVETH_GCI_SEND_ETH_FRAME_PCK_T;

/*! @} */

/*! \defgroup drveth_gci_service_recvframe Receive Ethernet Frame Service
 *
 * This service is used by the DrvEth GCI Adapter to indicate
 * reception of an Ethernet Frame to the Host application. The
 * Host application must use the HIL_REGISTER_APP_REQ before this
 * service is enabled.
 * @{
 */

/*! Received Ethernet frame indication packet */
typedef __HIL_PACKED_PRE struct DRVETH_GCI_RECV_ETH_FRAME_IND_Ttag
{
  /*! Packet header */
  HIL_PACKET_HEADER_T       tHead;
  /*! Packet data */
  DRVETH_GCI_ETH_FRAME_T    tData;
} __HIL_PACKED_POST DRVETH_GCI_RECV_ETH_FRAME_IND_T;

/*! Send Ethernet frame confirmation packet */
typedef HIL_EMPTY_PACKET_T  DRVETH_GCI_RECV_ETH_FRAME_RSP_T;

/*! Receive Ethernet frame packet union */
typedef union DRVETH_GCI_RECV_ETH_FRAME_PCK_Ttag
{
  /*! Indication */
  DRVETH_GCI_RECV_ETH_FRAME_IND_T tInd;
  /*! Response */
  DRVETH_GCI_RECV_ETH_FRAME_RSP_T tRsp;
} DRVETH_GCI_RECV_ETH_FRAME_PCK_T;

/*! @} */


/*! \defgroup drveth_gci_service_event Event service
 *
 *  This service is used by the DrvEth GCI Adapter to
 *  notify the Host Application about occurring events. The
 *  service will be enabled after the Host Application used
 *  the HIL_REGISTER_APP_REQ.
 *
 *  The service uses a locking mechanism to avoid flooding the
 *  host application with events. After an event indication
 *  has been generated, the DrvEth GCI Adapter will count
 *  any subsequent events instead of sending a new packet. After
 *  the host returned the event response back, a new event
 *  indication will be generated if necessary.
 *
 * @{
 */

/*! Event counter enumeration */
typedef enum DRVETH_GCI_EVENT_Etag
{
  /*! Linkstatus changed event. */
  DRVETH_GCI_EVENT_LINKCHANGED = 0,
  /*! Maximum number of event types. */
  DRVETH_GCI_EVENT_MAX
} DRVETH_GCI_EVENT_E;

/*! Event Data */
typedef __HIL_PACKED_PRE struct DRVETH_GCI_EVENT_DATA_Ttag
{
  /*! Array of Counters counting the events defined by
   * DRVETH_GCI_EVENT_E */
  uint16_t  uiEventCnt[DRVETH_GCI_EVENT_MAX];
} __HIL_PACKED_POST DRVETH_GCI_EVENT_DATA_T;

/*! Event occurred indication packet */
typedef __HIL_PACKED_PRE struct DRVETH_GCI_EVENT_IND_Ttag
{
  /*! Packet header */
  HIL_PACKET_HEADER_T       tHead;
  /*! Packet data */
  DRVETH_GCI_EVENT_DATA_T   tData;
} __HIL_PACKED_POST DRVETH_GCI_EVENT_IND_T;

/*! Event occurred response packet */
typedef HIL_EMPTY_PACKET_T  DRVETH_GCI_EVENT_RSP_T;

/*! Event service packet union */
typedef union DRVETH_GCI_EVENT_PCK_Ttag
{
  /*! Indication */
  DRVETH_GCI_EVENT_IND_T tInd;
  /*! Response */
  DRVETH_GCI_EVENT_RSP_T tRsp;
} DRVETH_GCI_EVENT_PCK_T;
/*! @} */

/*! \defgroup drveth_gci_service_setmulticast Set Multicast Single Service
 *
 * This service shall be used to receive traffic from a specific IPv4
 * multicast group.
 *
 * @{
 */

/*! Register multicast group request packet */
typedef __HIL_PACKED_PRE struct DRVETH_GCI_SET_MULTICAST_SINGLE_REQ_Ttag
{
  /*! Packet header */
  HIL_PACKET_HEADER_T                 tHead;
  /*! Packet data */
  uint8_t abMacAddr[DRVETH_GCI_ETH_ADDR_SIZE];
} __HIL_PACKED_POST DRVETH_GCI_SET_MULTICAST_SINGLE_REQ_T;

/*! Register multicast group confirmation packet */
typedef HIL_EMPTY_PACKET_T  DRVETH_GCI_SET_MULTICAST_SINGLE_CNF_T;

/*! Register multicast group reception packet union */
typedef union DRVETH_GCI_SET_MULTICAST_SINGLE_PCK_Ttag
{
  /*! Request */
  DRVETH_GCI_SET_MULTICAST_SINGLE_REQ_T tReq;
  /*! Confirmation */
  DRVETH_GCI_SET_MULTICAST_SINGLE_CNF_T tCnf;
} DRVETH_GCI_SET_MULTICAST_SINGLE_PCK_T;

/*! @} */

/*! \defgroup drveth_gci_service_clearmulticast Clear Multicast Single Service
 *
 * This service shall be used to stop receiving traffic from a specific IPv4
 * multicast group.
 *
 * @{
 */

/*! Clear multicast group request packet */
typedef __HIL_PACKED_PRE struct DRVETH_GCI_CLR_MULTICAST_SINGLE_REQ_Ttag
{
  /*! Packet header */
  HIL_PACKET_HEADER_T                 tHead;
  /*! Packet data */
  uint8_t abMacAddr[DRVETH_GCI_ETH_ADDR_SIZE];
} __HIL_PACKED_POST DRVETH_GCI_CLR_MULTICAST_SINGLE_REQ_T;

/*! Clear multicast group confirmation packet */
typedef HIL_EMPTY_PACKET_T  DRVETH_GCI_CLR_MULTICAST_SINGLE_CNF_T;

/*! Unregister multicast group reception packet union */
typedef union DRVETH_GCI_CLR_MULTICAST_SINGLE_PCK_Ttag
{
  /*! Request */
  DRVETH_GCI_CLR_MULTICAST_SINGLE_REQ_T tReq;
  /*! Confirmation */
  DRVETH_GCI_CLR_MULTICAST_SINGLE_CNF_T tCnf;
} DRVETH_GCI_CLR_MULTICAST_SINGLE_PCK_T;

/*! @} */

/*! @} */

#endif /* #ifndef __DRVETH_GCI_API_H */
