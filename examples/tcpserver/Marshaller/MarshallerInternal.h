/**************************************************************************************

   Copyright (c) Hilscher GmbH. All Rights Reserved.

 **************************************************************************************

   Filename:
    $Id: MarshallerInternal.h 13309 2019-11-05 12:03:27Z AlexanderMinor $
   Last Modification:
    $Author: AlexanderMinor $
    $Date: 2019-11-05 13:03:27 +0100 (Di, 05 Nov 2019) $
    $Revision: 13309 $

   Targets:
     OS independent : yes

   Description:
    Marshaller internal structure definitions

   Changes:

     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
     5        06.07.2010   MT       Change:
                                     - Re-Added Tx Buffers, as they were used by 
                                       Packet Transport
                                     - Added new configuration elements per 
                                       connector for configuration of TX Buffers (see HIL_MARSHALLER_CONNECTOR_T)
     4        23.09.2009   MS       Change:
                                     - Removed unused Tx buffers and doubled number of Rx buffers to avoid resource errors without increasing memory footprint.
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
#include "HilTransport.h"
#include <sys/queue.h>
#include <stdbool.h>

/*****************************************************************************/
/*! \addtogroup NETX_MARSHALLER_INTERNAL
*   \{                                                                       */
/*****************************************************************************/

#ifdef __cplusplus
  extern "C" {
#endif

typedef enum MARSHALLER_BUFFER_TYPE_Etag
{
  eMARSHALLER_RX_BUFFER,
  eMARSHALLER_TX_BUFFER,
  eMARSHALLER_ACK_BUFFER,
  eMARSHALLER_KEEPALIVE_BUFFER,

} MARSHALLER_BUFFER_TYPE_E;

/*****************************************************************************/
/*! Marshaller Send/Receive buffer structure                                 */
/*****************************************************************************/
struct HIL_MARSHALLER_BUFFER_Ttag
{
  /* Single linked list */
  STAILQ_ENTRY(HIL_MARSHALLER_BUFFER_Ttag) tList;

  struct
  {
    uint32_t  ulConnectorIdx;       /*!< Connector number of this buffer */
    void*       pvMarshaller;         /*!< Marshaller object this buffer has been allocated by */

    MARSHALLER_BUFFER_TYPE_E eType;   /*!< Type of buffer */

    uint32_t  ulDataBufferLen;      /*!< Total length of buffer (only counting the size of abData[] */
    uint32_t  ulUsedDataBufferLen;  /*!< Number of used bytes in abData[] */

    uint32_t  ulActualSendOffset;   /*!< Can be used by connector for fragmented sending   */
  } tMgmt;

  HIL_TRANSPORT_HEADER  tTransport;   /*!< Transport header */
  uint8_t     abData[1];            /*!< User data        */
};

typedef struct HIL_MARSHALLER_BUFFER_Ttag HIL_MARSHALLER_BUFFER_T;

typedef uint32_t(*PFN_CONN_TRANSMIT)(HIL_MARSHALLER_BUFFER_T* ptBuffer, void* pvUser);
typedef void(*PFN_CONN_DEINIT)(void* pvUser);
typedef void(*PFN_CONN_POLL)(void* pvUser);

/*****************************************************************************/
/*! Connector registration information (filled by Connector)                 */
/*****************************************************************************/
typedef struct HIL_MARSHALLER_CONNECTOR_Ttag
{
  PFN_CONN_TRANSMIT pfnTransmit;  /*!< Transmit function         */
  PFN_CONN_DEINIT   pfnDeinit;    /*!< Uninitialization function */
  PFN_CONN_POLL     pfnPoll;      /*!< Function to call, if the connector needs to be polled */

  uint32_t          ulDataBufferSize; /*!< Size for the RX buffers in Bytes */
  uint32_t          ulDataBufferCnt;  /*!< Number of RX buffers to allocate per Connector */
  uint32_t          ulTimeout;        /*!< Timeout for RX state machine */

  void*             pvUser;           /*!< User parameter to provide on function calls */

  /* NOTE: These information were added to the end of the structure to be old
           compatible. When using the previous structure initializations these fields
           will automatically be set to 0 */
  uint32_t          ulTxBufferSize;   /*!< TX Buffer size for unsolicited packets / calls (indication / callbacks)  */
  uint32_t          ulTxBufferCnt;    /*!< TX Buffer count for unsolicited packets / calls (indication / callbacks) */

} HIL_MARSHALLER_CONNECTOR_T;

typedef void(*PFN_TRANSPORT_HANDLER)(void* pvMarshaller, HIL_MARSHALLER_BUFFER_T* ptBuffer, void* pvUser);
typedef void(*PFN_TRANSPORT_DEINIT)(void* pvUser);
typedef void(*PFN_TRANSPORT_POLL)(void* pvUser);

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

/* This function is called by a connector when it receives data from the line */
uint32_t HilMarshallerConnRxData(void* pvMarshaller, uint32_t ulConnector, uint8_t* pbData, uint32_t ulDataCnt);

/* This function is called by a transport, to send an answer */
uint32_t HilMarshallerConnTxData(void* pvMarshaller, uint32_t ulConnector, HIL_MARSHALLER_BUFFER_T* ptBuffer);

/* This function is called by a connector, to signal transfer complete */
uint32_t HilMarshallerConnTxComplete(void* pvMarshaller, uint32_t ulConnector, HIL_MARSHALLER_BUFFER_T* ptBuffer);

/* Connector registration/deregistration */
uint32_t HilMarshallerRegisterConnector(void* pvMarshaller, uint32_t* pulConnectorIdx, HIL_MARSHALLER_CONNECTOR_T* ptConn);
void     HilMarshallerUnregisterConnector(void* pvMarshaller, uint32_t ulConnectorIdx);

/* Register data transport layers */
uint32_t HilMarshallerRegisterTransport(void* pvMarshaller, const TRANSPORT_LAYER_DATA_T* ptLayerData);
void     HilMarshallerUnregisterTransport(void* pvMarshaller, uint16_t usDataType);

void                     HilMarshallerFreeBuffer(HIL_MARSHALLER_BUFFER_T* ptBuffer);
HIL_MARSHALLER_BUFFER_T* HilMarshallerGetBuffer(void* pvMarshaller, MARSHALLER_BUFFER_TYPE_E eType, uint32_t ulConnector);


/*****************************************************************************/
/*! Enum for the receive data state machine                                  */
/*****************************************************************************/
typedef enum
{
  HIL_SCAN_DONE   = 0,
  HIL_SEARCH_TELEGRAM_COOKIE,
  HIL_SEARCH_TELEGRAM_HEADER,
  HIL_WAIT_TELEGRAM_DATA,
  HIL_CHECK_TELEGRAM
} HIL_SCAN_STATE_E;

STAILQ_HEAD(MARSHALLER_BUFFER_HEAD, HIL_MARSHALLER_BUFFER_Ttag);

/*****************************************************************************/
/*! Marshaller internal connector data storage                               */
/*****************************************************************************/
typedef struct CONNECTOR_DATA_Ttag
{
  bool                          fInUse;           /*!< TRUE if connector slot is in use (contains a registered connector) */
  uint32_t                      ulConnectorIdx;   /*!< Number of the connector */
  uint32_t                      ulMode;           /*!< connector mode, see MARSHALLER_MODE_xxx constants */

  /* Connector Registration data */
  HIL_MARSHALLER_CONNECTOR_T    tConn;            /*!< Registration data */

  /* Data for parsing RX telegram data */
  HIL_SCAN_STATE_E              eScanState;       /*!< Current state of receive state machine                       */
  HIL_TRANSPORT_HEADER          tRxHeader;        /*!< Transport header used by parser for start of frame detection */
  uint32_t                      ulRxOffset;       /*!< Current receive offset inside tRxHeader (used by parser)     */
  bool                          fMonitorTimeout;  /*!< TRUE if timeout should be monitored                          */
  uint32_t                      ulElapsedTime;    /*!< Time since last scanner process, used for RX data timeout    */

  uint32_t                      ulKeepaliveID;    /*!< Last valid keep alive ID */

  HIL_MARSHALLER_BUFFER_T*      ptCurrentRxBuffer;/*!< Current used RX buffer to collect data                       */

  /* RX / Ack Buffer list */
  struct MARSHALLER_BUFFER_HEAD tRxBuffer;
  struct MARSHALLER_BUFFER_HEAD tTxBuffer;
  struct MARSHALLER_BUFFER_HEAD tAckBuffer;
  struct MARSHALLER_BUFFER_HEAD tKeepAliveBuffer;

} CONNECTOR_DATA_T;

/*****************************************************************************/
/*! Marshaller data storage                                                  */
/*****************************************************************************/
typedef struct HIL_MARSHALLER_DATA_Ttag
{
  char                          szServerName[HIL_TRANSPORT_QUERYSERVER_NAMELEN];
  uint32_t                      ulMaxConnectors;  /*!< Maximum number of connectors (size of atConnectors[]) */
  CONNECTOR_DATA_T*             atConnectors;     /*!< Array of available connectors */

  /* cifX API Layer configuration */
  uint32_t                      ulTransports;     /*!< Number of available transport handler                 */
  TRANSPORT_LAYER_DATA_T*       ptTransports;     /*!< Array of available transport handler                  */

  struct MARSHALLER_BUFFER_HEAD tPendingRequests;

  PFN_MARSHALLER_REQUEST        pfnRequest;
  void*                         pvUser;

}  HIL_MARSHALLER_DATA_T;

#ifdef __cplusplus
  }
#endif


/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/

#endif /*  __MARSHALLERINTERNAL__H */
