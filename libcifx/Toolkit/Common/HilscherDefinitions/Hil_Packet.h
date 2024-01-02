/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_Packet.h $: *//*!

  \file Hil_Packet.h

  Hilscher Packet Definition.

**************************************************************************************/
#ifndef HIL_PACKET_H_
#define HIL_PACKET_H_


#include <stdint.h>
#include "Hil_Compiler.h"

  /************************ General Packet Definitions *************************/

  /** total packet size definition */
  #define HIL_MAX_PACKET_SIZE             (1596)
  #define HIL_PACKET_HEADER_SIZE          40              /*!< Maximum size of the HIL packet header in bytes */
  #define HIL_MAX_DATA_SIZE               (HIL_MAX_PACKET_SIZE - HIL_PACKET_HEADER_SIZE) /*!< Maximum RCX packet data size */


  /* Logical Queue defines */
  #define HIL_PACKET_DEST_SYSTEM         (0)
  #define HIL_PACKET_DEST_CHANNEL_0      (1)
  #define HIL_PACKET_DEST_CHANNEL_1      (2)
  #define HIL_PACKET_DEST_CHANNEL_2      (3)
  #define HIL_PACKET_DEST_CHANNEL_3      (4)

  #define HIL_PACKET_DEST_DEFAULT_CHANNEL   0x00000020
  #define HIL_PACKET_COMM_CHANNEL_TOKEN     0x00000020

  #define HIL_MSK_PACKET_ANSWER             0x00000001      /*!< Packet answer bit */



  /*** Definitions for the Packet Extension Field ***/

  /* mask for sequence number and sequence control portions */
  #define HIL_PACKET_SEQ_NR_MASK          (0x0000003F)    /*!< used for filtering the sequence number */
  #define HIL_PACKET_SEQ_MASK             (0x000000C0)    /*!< used for filtering the sequence control code */

  /* sequence control codes */
  #define HIL_PACKET_SEQ_NONE             (0x00000000)    /*!< packet is not part of a packet sequence */
  #define HIL_PACKET_SEQ_LAST             (0x00000040)    /*!< last packet of a packet sequence */
  #define HIL_PACKET_SEQ_FIRST            (0x00000080)    /*!< first packet of a packet sequence */
  #define HIL_PACKET_SEQ_MIDDLE           (0x000000C0)    /*!< packet in the middle of a packet sequence */

  /* packet handling flags */
  #define HIL_PACKET_NOT_DELETE           (0x00000100)    /*!< packet must not be returned to a packet pool */
  #define HIL_PACKET_RETRY                (0x00000200)    /*!< packet will be resent based on a predefined retry mechanism */

  /* router flags */
  #define HIL_PACKET_NO_CNF_THRU_ROUTER   (0x00000400)    /*!< router must not send response/confirmation packet back */

  /*********************** Packet Structure Definitions ************************/

  /** packet header definition */
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PACKET_HEADER_Ttag
  {
    uint32_t  ulDest;   /*!< destination of the packet (task message queue reference) */
    uint32_t  ulSrc;    /*!< source of the packet (task message queue reference) */
    uint32_t  ulDestId; /*!< destination reference (internal use for message routing) */
    uint32_t  ulSrcId;  /*!< source reference (internal use for message routing) */
    uint32_t  ulLen;    /*!< length of packet data (starting from the end of the header) */
    uint32_t  ulId;     /*!< identification reference (internal use by the sender) */
    uint32_t  ulSta;    /*!< operation status code (error code, initialize with 0) */
    uint32_t  ulCmd;    /*!< operation command code */
    uint32_t  ulExt;    /*!< extension count (nonzero in multi-packet transfers) */
    uint32_t  ulRout;   /*!< router reference (internal use for message routing) */
  } HIL_PACKET_HEADER_T;


  /** definition of a packet with maximum size */
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_PACKET_Ttag
  {
    HIL_PACKET_HEADER_T   tHead;
    uint8_t               abData[HIL_MAX_PACKET_SIZE - sizeof (HIL_PACKET_HEADER_T)];
  } HIL_PACKET_T;


  /** definition of a packet with minimum size */
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_EMPTY_PACKET_Ttag
  {
    HIL_PACKET_HEADER_T   tHead;
  } HIL_EMPTY_PACKET_T;


#endif  /* HIL_PACKET_H_ */
