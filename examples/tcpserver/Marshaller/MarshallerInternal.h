/**************************************************************************************

   Copyright (c) Hilscher GmbH. All Rights Reserved.

 **************************************************************************************

   Filename:
    $Id: MarshallerInternal.h 15613 2026-10-01 10:31:00Z AMinor $
   Last Modification:
    $Author: AMinor $
    $Date: 2026-10-01 12:31:00 +0200 (Do, 01 Okt 2026) $
    $Revision: 15613 $

   Targets:
     OS independent : yes

   Description:
    Marshaller internal structure definitions

   Changes:

     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
     7        05.08.2025   MSC      New Feature:
                                     - NXMECOS-31: Call Marshaller functions via clean
                                       interface from connector
     6        04.08.2025   MSC      Change:
                                     - Use opaque marshaller handle.
     5        06.07.2010   MT       Change:
                                     - Re-Added Tx Buffers, as they were used by
                                       Packet Transport
                                     - Added new configuration elements per
                                       connector for configuration of TX Buffers (see
                                       HIL_MARSHALLER_CONNECTOR_T)
     4        23.09.2009   MS       Change:
                                     - Removed unused Tx buffers and doubled number of Rx buffers to
                                       avoid resource errors without increasing memory footprint.
     3        22.09.2009   MS       Addon:
                                     - Added member ulMode to CONNECTOR_DATA_T.
     2        28.07.2009   MS       Change:
                                     - Moved Marshaller version constants to MarshallerVersion.h.
     1        25.05.2009   MT       initial version

**************************************************************************************/

/*****************************************************************************/
/*! \file MarshallerInternal.h
 *   Internal structure and API definitions of the netX Marshaller            */
/*****************************************************************************/

#ifndef __MARSHALLERINTERNAL__H
#define __MARSHALLERINTERNAL__H

#include "cifXAPI_Wrapper.h"
#include "MarshallerConfig.h"
#include "MarshallerConnectorIf.h"
#include "MarshallerBuffer.h"
#include "HilTransport.h"
#include <sys/queue.h>
#include <stdbool.h>

/*****************************************************************************/
/*! \addtogroup NETX_MARSHALLER_INTERNAL
 *   \{                                                                       */
/*****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/*! Enum for the receive data state machine                                  */
/*****************************************************************************/
typedef enum
{
  HIL_SCAN_DONE = 0,
  HIL_SEARCH_TELEGRAM_COOKIE,
  HIL_SEARCH_TELEGRAM_HEADER,
  HIL_WAIT_TELEGRAM_DATA,
  HIL_CHECK_TELEGRAM
} HIL_SCAN_STATE_E;

/*****************************************************************************/
/*! Marshaller internal connector data storage                               */
/*****************************************************************************/

typedef struct CONNECTOR_DATA_Ttag
{
  bool     fInUse; /*!< TRUE if connector slot is in use (contains a registered connector) */
  uint32_t ulConnectorIdx; /*!< Number of the connector */
  uint32_t ulMode;         /*!< connector mode, see MARSHALLER_MODE_xxx constants */

  /* Connector Registration data */
  HIL_MARSHALLER_CONNECTOR_T tConn; /*!< Registration data */

  /* Data for parsing RX telegram data */
  HIL_SCAN_STATE_E eScanState; /*!< Current state of receive state machine                       */
  HIL_TRANSPORT_HEADER tRxHeader; /*!< Transport header used by parser for start of frame detection */
  uint32_t ulRxOffset;      /*!< Current receive offset inside tRxHeader (used by parser)     */
  bool     fMonitorTimeout; /*!< TRUE if timeout should be monitored                          */
  uint32_t ulElapsedTime;   /*!< Time since last scanner process, used for RX data timeout    */

  uint32_t ulKeepaliveID; /*!< Last valid keep alive ID */

  HIL_MARSHALLER_BUFFER_T* ptCurrentRxBuffer; /*!< Current used RX buffer to collect data */

  /* RX / Ack Buffer list */
  struct MARSHALLER_BUFFER_HEAD tRxBuffer;
  struct MARSHALLER_BUFFER_HEAD tTxBuffer;
  struct MARSHALLER_BUFFER_HEAD tAckBuffer;
  struct MARSHALLER_BUFFER_HEAD tKeepAliveBuffer;
} CONNECTOR_DATA_T;

/* functions to be implemented by transport */
typedef void (*PFN_TRANSPORT_HANDLER)(HIL_MARSHALLER_H         hMarshaller,
                                      HIL_MARSHALLER_BUFFER_T* ptBuffer,
                                      void*                    pvUser);
typedef void (*PFN_TRANSPORT_DEINIT)(void* pvUser);
typedef void (*PFN_TRANSPORT_POLL)(void* pvUser);

/*****************************************************************************/
/*! Transport layer registration information                                 */
/*****************************************************************************/
typedef struct TRANSPORT_LAYER_DATA_Ttag
{
  uint16_t              usDataType;
  uint16_t              usReserved;
  PFN_TRANSPORT_HANDLER pfnHandler;
  PFN_TRANSPORT_DEINIT  pfnDeinit;
  PFN_TRANSPORT_POLL    pfnPoll;
  void*                 pvUser;
} TRANSPORT_LAYER_DATA_T;

/*****************************************************************************/
/* functions offered by marshaller to transports                             */
/*****************************************************************************/

/**
 * \brief Called by transport layer, during initialization.
 *
 * \param[in] hMarshaller      Marshaller handle
 * \param[in] ptLayerData      Layer registration data
 *
 * \return HIL_MARSHALLER_E_SUCCESS on success
 */
uint32_t HilMarshallerRegisterTransport(HIL_MARSHALLER_H              hMarshaller,
                                        const TRANSPORT_LAYER_DATA_T* ptLayerData);

/**
 * \brief Called by transport layer, during uninitialization.
 *
 * \param[in] hMarshaller      Marshaller handle
 * \param[in] usDataType       Datatype the layer was registered at
 */
void HilMarshallerUnregisterTransport(HIL_MARSHALLER_H hMarshaller, uint16_t usDataType);

/**
 * \brief Called by transport when new data should be send to line
 *
 * \note This function is called by a transport, to send an answer
 *
 * \param[in] hMarshaller      Marshaller handle
 * \param[in] ulConnector      Connector number
 * \param[in] ptBuffer         Outgoing data
 *
 * \return HIL_MARSHALLER_E_SUCCESS on success
 */
uint32_t HilMarshallerConnTxData(HIL_MARSHALLER_H         hMarshaller,
                                 uint32_t                 ulConnector,
                                 HIL_MARSHALLER_BUFFER_T* ptBuffer);

/**
 * \brief Free a buffer
 *
 * \param[in] ptBuffer Buffer to free
 */
void HilMarshallerFreeBuffer(HIL_MARSHALLER_BUFFER_T* ptBuffer);

/**
 * \brief Get a buffer for incoming data from connector
 *
 * \param[in] hMarshaller  Marshaller handle
 * \param[in] eType        Type of buffer to acquire
 * \param[in] ulConnector  Connector index
 *
 * \return NULL if no buffer is available, valid buffer otherwise
 */
HIL_MARSHALLER_BUFFER_T* HilMarshallerGetBuffer(HIL_MARSHALLER_H         hMarshaller,
                                                MARSHALLER_BUFFER_TYPE_E eType,
                                                uint32_t                 ulConnector);

/**
 * \brief Get all registered connectors
 *
 * \param[in] hMarshaller       Marshaller handle
 * \param[out] pulMaxConnectors Maximum number of connectors (size of atConnectors[])
 * \param[out] patConnectors    Array of available connectors
 */
void HilMarshallerGetConnectors(HIL_MARSHALLER_H   hMarshaller,
                                uint32_t*          pulMaxConnectors,
                                CONNECTOR_DATA_T** patConnectors);

#ifdef __cplusplus
}
#endif

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/

#endif /*  __MARSHALLERINTERNAL__H */
