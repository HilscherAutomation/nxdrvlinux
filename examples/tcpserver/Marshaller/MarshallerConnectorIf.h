/**************************************************************************************

   Copyright (c) Hilscher GmbH. All Rights Reserved.

 **************************************************************************************

   Filename:
    $Id: $
   Last Modification:
    $Author: $
    $Date: $
    $Revision: $

   Targets:
     OS independent : yes

   Description:
    Marshaller connector interface definitions

   Changes:

   Version   Date        Author   Description
   ----------------------------------------------------------------------------------
   1         2025-08-05  MSC      initial version

**************************************************************************************/

#ifndef __MARSHALLERCONNECTORIF__H
#define __MARSHALLERCONNECTORIF__H

#include "MarshallerConfig.h"
#include "MarshallerBuffer.h"

typedef struct HIL_MARSHALLER_CONNECTOR_USERDATA_Ttag* HIL_MARSHALLER_CONNECTOR_USERDATA_H;

/* functions to be implemented by connector */
typedef uint32_t (*PFN_CONN_TRANSMIT)(HIL_MARSHALLER_BUFFER_T*            ptBuffer,
                                      HIL_MARSHALLER_CONNECTOR_USERDATA_H hUser);
typedef void (*PFN_CONN_DEINIT)(HIL_MARSHALLER_CONNECTOR_USERDATA_H hUser);
typedef void (*PFN_CONN_POLL)(HIL_MARSHALLER_CONNECTOR_USERDATA_H hUser);

/*****************************************************************************/
/*! Connector registration information (filled by Connector)                 */
/*****************************************************************************/
typedef struct HIL_MARSHALLER_CONNECTOR_Ttag
{
  PFN_CONN_TRANSMIT pfnTransmit; /*!< Transmit function         */
  PFN_CONN_DEINIT   pfnDeinit;   /*!< Uninitialization function */
  PFN_CONN_POLL     pfnPoll;     /*!< Function to call, if the connector needs to be polled */

  uint32_t ulDataBufferSize; /*!< Size for the RX buffers in Bytes */
  uint32_t ulDataBufferCnt;  /*!< Number of RX buffers to allocate per Connector */
  uint32_t ulTimeout;        /*!< Timeout for RX state machine */

  HIL_MARSHALLER_CONNECTOR_USERDATA_H hUser; /*!< User parameter to provide on function calls */

  /* NOTE: These information were added to the end of the structure to be old
           compatible. When using the previous structure initializations these fields
           will automatically be set to 0 */
  uint32_t
    ulTxBufferSize; /*!< TX Buffer size for unsolicited packets / calls (indication / callbacks)  */
  uint32_t
    ulTxBufferCnt; /*!< TX Buffer count for unsolicited packets / calls (indication / callbacks) */

} HIL_MARSHALLER_CONNECTOR_T;

/*****************************************************************************/
/* functions offered by marshaller to connectors                             */
/*****************************************************************************/

/**
 * \brief Register a connector at marshaller
 *
 * \param[in] hMarshaller      Marshaller handle
 * \param[in] pulConnectorIdx  Returned connector number
 * \param[in] ptConn           Connector data
 *
 * \return HIL_MARSHALLER_E_SUCCESS on success
 */
typedef uint32_t (*PFN_MARSHALLER_REGISTER_CONNECTOR)(HIL_MARSHALLER_H            hMarshaller,
                                                      uint32_t*                   pulConnectorIdx,
                                                      HIL_MARSHALLER_CONNECTOR_T* ptConn);

/**
 * \brief Unregister a connector at marshaller
 *
 * \param[in] hMarshaller      Marshaller handle
 * \param[in] ulConnectorIdx   Connector number
 */
typedef void (*PFN_MARSHALLER_UNREGISTER_CONNECTOR)(HIL_MARSHALLER_H hMarshaller,
                                                    uint32_t         ulConnectorIdx);

/**
 * \brief Called by connector when new data has arrived
 *
 * \note This function is called by a connector when it receives data from the line
 *
 * \param[in] hMarshaller      Marshaller handle
 * \param[in] ulConnector      Connector number
 * \param[in] pbData           Incoming data
 * \param[in] ulDataCnt        Number of bytes in incoming data buffer
 *
 * \return HIL_MARSHALLER_E_SUCCESS on success
 */
typedef uint32_t (*PFN_MARSHALLER_CONN_RX_DATA)(HIL_MARSHALLER_H hMarshaller,
                                                uint32_t         ulConnector,
                                                uint8_t*         pbData,
                                                uint32_t         ulDataCnt);

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
typedef uint32_t (*PFN_MARSHALLER_CONN_TX_DATA)(HIL_MARSHALLER_H         hMarshaller,
                                                uint32_t                 ulConnector,
                                                HIL_MARSHALLER_BUFFER_T* ptBuffer);

/**
 * \brief Called by connector when data has been sent
 *
 * \note This function is called by a connector, to signal transfer complete
 *
 * \param[in] hMarshaller      Marshaller handle
 * \param[in] ulConnector      Connector number
 * \param[in] ptBuffer         Outgoing data buffer
 *
 * \return HIL_MARSHALLER_E_SUCCESS on success
 */
typedef uint32_t (*PFN_MARSHALLER_CONN_TX_COMPLETE)(HIL_MARSHALLER_H         hMarshaller,
                                                    uint32_t                 ulConnector,
                                                    HIL_MARSHALLER_BUFFER_T* ptBuffer);

/**
 * \brief Get user data pointer from marshaller handle
 *
 * \param[in] hMarshaller  Marshaller handle
 *
 * \return User data pointer
 */
typedef HIL_MARSHALLER_USERDATA_H (*PFN_MARSHALLER_GET_USER_DATA)(HIL_MARSHALLER_H hMarshaller);

/*****************************************************************************/
/*! Marshaller Interface to Connectors                                       */
/*****************************************************************************/
typedef struct HIL_MARSHALLER_CONNECTOR_IF_Ttag
{
  PFN_MARSHALLER_REGISTER_CONNECTOR pfnRegisterConnector; /*!< Function to register a connector */
  PFN_MARSHALLER_UNREGISTER_CONNECTOR
                                  pfnUnregisterConnector; /*!< Function to unregister a connector */
  PFN_MARSHALLER_CONN_RX_DATA     pfnRxData;     /*!< Function to call when new data has arrived */
  PFN_MARSHALLER_CONN_TX_COMPLETE pfnTxComplete; /*!< Function to call when data has been sent    */
  PFN_MARSHALLER_GET_USER_DATA pfnGetUserData;   /*!< Function to get user data from marshaller   */
} HIL_MARSHALLER_CONNECTOR_IF_T;

#endif /* __MARSHALLERCONNECTORIF__H */
