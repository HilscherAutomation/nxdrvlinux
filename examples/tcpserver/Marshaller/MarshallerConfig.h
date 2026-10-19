/**************************************************************************************

   Copyright (c) Hilscher GmbH. All Rights Reserved.

 **************************************************************************************

   Filename:
    $Id: MarshallerConfig.h 15187 2025-08-05 12:35:33Z MSchulz $
   Last Modification:
    $Author: MSchulz $
    $Date: 2025-08-05 14:35:33 +0200 (Di, 05 Aug 2025) $
    $Revision: 15187 $

   Targets:
     OS independent : yes

   Description:
    Public Hilscher transport marshaller definitions

   Changes:

     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
     6        05.08.2025   MSC      New Feature:
                                     - NXMECOS-31: Call Marshaller functions via clean
                                       interface from connector
     5        04.08.2025   MSC      Change:
                                     - Use opaque marshaller handle.
     4        06.07.2010   MT       Change:
                                     - Added new configuration elements per
                                       connector for configuration of TX Buffers (see
                                       HIL_MARSHALLER_CONNECTOR_PARAMS_T)
     3        22.09.2009   MS       Addon: Added HilMarshallerSetMode(), and mode constants
     2        02.09.2009   MS       Change:
                                    HilMarshallerMain() returns TLR_E_FAIL if no message can be
                                    retrieved from the pending requests list.
     1        25.05.2009   MT       initial version

**************************************************************************************/

#ifndef __MARSHALLERCONFIG__H
#define __MARSHALLERCONFIG__H

/*****************************************************************************/
/*! \addtogroup NETX_MARSHALLER_CONFIGURATION
 *   \{                                                                       */
/*****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "HilTransport.h"

/* Use generic Results errors for Marshaller errors */
#define HIL_MARSHALLER_E_SUCCESS          ((uint32_t) 0x00000000L) /* TLR_S_OK */
#define HIL_MARSHALLER_E_FAIL             ((uint32_t) 0xC0000001L) /* TLR_E_FAIL */
#define HIL_MARSHALLER_E_OUTOFMEMORY      ((uint32_t) 0xC0000003L) /* TLR_E_OUTOFMEMORY */
#define HIL_MARSHALLER_E_INVALIDPARAMETER ((uint32_t) 0xC0000009L) /* TLR_E_INVALID_PARAMETER */
#define HIL_MARSHALLER_E_OUTOFRESOURCES   ((uint32_t) 0xC0000100L) /* TLR_E_INIT_FAULT */
#define HIL_MARSHALLER_E_ALREADYREGISTERED                                                         \
  ((uint32_t) 0xC0000201L) /* TLR_E_APPLICATION_ALREADY_REGISTERED */

/* mode constants for Marshaller mode request */
#define HIL_MARSHALLER_MODE_ENABLED   0 /* no restrictions */
#define HIL_MARSHALLER_MODE_DISABLED  1 /* no access (connector blocked) */

/* special connector ID for Marshaller mode setup */
#define HIL_MARSHALLER_CONNECTORS_ALL 0xFFFFFFFF /* command applies to all connectors */

typedef struct HIL_MARSHALLER_DATA_Ttag*               HIL_MARSHALLER_H;
typedef struct HIL_MARSHALLER_USERDATA_Ttag*           HIL_MARSHALLER_USERDATA_H;
typedef const struct HIL_MARSHALLER_CONNECTOR_IF_Ttag* HIL_MARSHALLER_CONNECTOR_IF_H;

struct HIL_MARSHALLER_CONNECTOR_PARAMS_Ttag;

typedef uint32_t (*PFN_CONN_INIT)(const struct HIL_MARSHALLER_CONNECTOR_PARAMS_Ttag* ptParams,
                                  HIL_MARSHALLER_H                                   hMarshaller,
                                  HIL_MARSHALLER_CONNECTOR_IF_H                      hMarshallerIf);

/*****************************************************************************/
/*! Connector parameter dara (needed for registration at marshaller)         */
/*****************************************************************************/
struct HIL_MARSHALLER_CONNECTOR_PARAMS_Ttag
{
  PFN_CONN_INIT pfnConnectorInit; /*!< Array of function pointer to initialize Connectors    */

  uint32_t ulDataBufferSize; /*!< Size for the RX buffers in Bytes */
  uint32_t ulDataBufferCnt;  /*!< Number of RX buffers to allocate per Connector */

  uint16_t usFlags;      /*! connection management flags, see HIL_MARSHALLER_CONNECTION_FLAG_xxx */
  uint32_t ulTimeout;    /*!< timeout in ms, see HIL_MARSHALLER_CONNECTION_TIMEOUT_xxx */
  void*    pvConfigData; /*!< Depends on CONNECTION_TYPE */

  /* NOTE: These information were added to the end of the structure to be old
           compatible. When using the previous structure initializations these fields
           will automatically be set to 0 */
  uint32_t
    ulTxBufferSize; /*!< TX Buffer size for unsolicited packets / calls (indication / callbacks)  */
  uint32_t
    ulTxBufferCnt; /*!< TX Buffer count for unsolicited packets / calls (indication / callbacks) */
};

typedef struct HIL_MARSHALLER_CONNECTOR_PARAMS_Ttag HIL_MARSHALLER_CONNECTOR_PARAMS_T;

typedef uint32_t (*PFN_TRANSPORT_INIT)(HIL_MARSHALLER_H hMarshaller, void* pvConfig);

/*****************************************************************************/
/*! Transport layer configuration                                            */
/*****************************************************************************/
typedef struct TRANSPORT_LAYER_CONFIG_Ttag
{
  PFN_TRANSPORT_INIT pfnInit;  /*!< Initialization function */
  void*              pvConfig; /*!< Configuration data      */

} TRANSPORT_LAYER_CONFIG_T;

typedef void (*PFN_MARSHALLER_REQUEST)(HIL_MARSHALLER_H          hMarshaller,
                                       HIL_MARSHALLER_USERDATA_H hUser);

/*****************************************************************************/
/*! Marshaller startup parameters                                            */
/*****************************************************************************/
typedef struct HIL_MARSHALLER_PARAMS_Ttag
{
  char     szServerName[32];
  uint32_t ulMaxConnectors; /*!< Maximum number of connectors */

  uint32_t ulConnectorCnt; /*!< Number of connectors to automatically load at startup */
  const HIL_MARSHALLER_CONNECTOR_PARAMS_T* ptConnectors;

  /* Add transport layers, currently only cifX and rcX Packet support */
  uint32_t ulTransportCnt; /*!< Number of transports to automatically load at startup */
  const TRANSPORT_LAYER_CONFIG_T*
    atTransports; /*!< Array of function pointer to initialize Transports    */

} HIL_MARSHALLER_PARAMS_T;

/*****************************************************************************/
/*! Marshaller function prototypes                                           */
/*****************************************************************************/

/**
 * \brief Startup marshaller
 *
 * \param[in] ptParams       Marshaller parameters
 * \param[out] phMarshaller  Returned Marshaller handle
 * \param[in] pfnRequest     Function to call, when MarshallerMain should be called
 * \param[in] hUser          User parameter to pass on pfnRequest call
 * \return HIL_MARSHALLER_E_SUCCESS on success
 */
uint32_t HilMarshallerStart(const HIL_MARSHALLER_PARAMS_T* ptParams,
                            HIL_MARSHALLER_H*              phMarshaller,
                            PFN_MARSHALLER_REQUEST         pfnRequest,
                            HIL_MARSHALLER_USERDATA_H      hUser);

/**
 * \brief Stop marshaller
 *
 * \param[in] hMarshaller  Marshaller handle
 */
void HilMarshallerStop(HIL_MARSHALLER_H hMarshaller);

/**
 * \brief Cyclic timer event, which needs to be called by user for timeout management
 *
 * \param[in] hMarshaller     Marshaller handle
 */
void HilMarshallerTimer(HIL_MARSHALLER_H hMarshaller);

/**
 * \brief Main marshaller module.
 *
 * This must be called by user, every time it receives the pfnRequest callback.
 *
 * \param[in] hMarshaller     Marshaller handle
 *
 * \retval \c HIL_MARSHALLER_E_SUCCESS on success
 * \retval \c HIL_MARSHALLER_E_FAIL if no message retrieved from the pending requests list
 */
uint32_t HilMarshallerMain(HIL_MARSHALLER_H hMarshaller);

/**
 * \brief Set the mode (unrestricted / restricted / disabled)
 *
 * Set the mode either of a single connector or of all connectors.
 *
 * \param[in] hMarshaller     Marshaller handle
 * \param[in] ulMode           see MARSHALLER_MODE_xxx constants above
 * \param[in] ulConnectorID    connector index or MARSHALLER_CONNECTORS_ALL
 *
 * \retval \c HIL_MARSHALLER_E_SUCCESS     on success
 * \retval \c HIL_MARSHALLER_E_INVALIDPARAMETER  if mode invalid
 * \retval \c HIL_MARSHALLER_E_OUTOFRESOURCES    if connector reference invalid
 */
uint32_t
HilMarshallerSetMode(HIL_MARSHALLER_H hMarshaller, uint32_t ulMode, uint32_t ulConnectorID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/*****************************************************************************/
/*  \}                                                                       */
/*****************************************************************************/

#endif /* __MARSHALLERCONFIG__H */
