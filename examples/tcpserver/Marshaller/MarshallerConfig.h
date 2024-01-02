/**************************************************************************************

   Copyright (c) Hilscher GmbH. All Rights Reserved.

 **************************************************************************************

   Filename:
    $Id: MarshallerConfig.h 13309 2019-11-05 12:03:27Z AlexanderMinor $
   Last Modification:
    $Author: AlexanderMinor $
    $Date: 2019-11-05 13:03:27 +0100 (Di, 05 Nov 2019) $
    $Revision: 13309 $

   Targets:
     OS independent : yes

   Description:
    Public Hilscher transport marshaller definitions

   Changes:

     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
     4        06.07.2010   MT       Change:
                                     - Added new configuration elements per 
                                       connector for configuration of TX Buffers (see HIL_MARSHALLER_CONNECTOR_PARAMS_T)
     3        22.09.2009   MS       Addon:
                                    Added HilMarshallerSetMode(), and mode constants
     2        02.09.2009   MS       Change:
                                    HilMarshallerMain() returns TLR_E_FAIL if no message can be retrieved from the pending requests list.
     1        25.05.2009   MT       initial version

**************************************************************************************/

#ifndef __MARSHALLERCONFIG__H
#define __MARSHALLERCONFIG__H

/*****************************************************************************/
/*! \addtogroup NETX_MARSHALLER_CONFIGURATION
*   \{                                                                       */
/*****************************************************************************/

#ifdef __cplusplus
  extern "C" {
#endif  /* __cplusplus */

#include "HilTransport.h"

/* Use generic Results errors for Marshaller errors */
#define  HIL_MARSHALLER_E_SUCCESS             ((uint32_t)0x00000000L) /* TLR_S_OK */
#define  HIL_MARSHALLER_E_FAIL                ((uint32_t)0xC0000001L) /* TLR_E_FAIL */
#define  HIL_MARSHALLER_E_OUTOFMEMORY         ((uint32_t)0xC0000003L) /* TLR_E_OUTOFMEMORY */
#define  HIL_MARSHALLER_E_INVALIDPARAMETER    ((uint32_t)0xC0000009L) /* TLR_E_INVALID_PARAMETER */
#define  HIL_MARSHALLER_E_OUTOFRESOURCES      ((uint32_t)0xC0000100L) /* TLR_E_INIT_FAULT */
#define  HIL_MARSHALLER_E_ALREADYREGISTERED   ((uint32_t)0xC0000201L) /* TLR_E_APPLICATION_ALREADY_REGISTERED */

/* mode constants for Marshaller mode request */
#define HIL_MARSHALLER_MODE_ENABLED           0          /* no restrictions */
#define HIL_MARSHALLER_MODE_DISABLED          1          /* no access (connector blocked) */

/* special connector ID for Marshaller mode setup */
#define HIL_MARSHALLER_CONNECTORS_ALL         0xFFFFFFFF  /* command applies to all connectors */


struct HIL_MARSHALLER_CONNECTOR_PARAMS_Ttag;

typedef uint32_t(*PFN_CONN_INIT)(const struct HIL_MARSHALLER_CONNECTOR_PARAMS_Ttag* ptParams, void* pvMarshaller);

/*****************************************************************************/
/*! Connector parameter dara (needed for registration at marshaller)         */
/*****************************************************************************/
struct HIL_MARSHALLER_CONNECTOR_PARAMS_Ttag
{
  PFN_CONN_INIT  pfnConnectorInit; /*!< Array of function pointer to initialize Connectors    */

  uint32_t       ulDataBufferSize; /*!< Size for the RX buffers in Bytes */
  uint32_t       ulDataBufferCnt;  /*!< Number of RX buffers to allocate per Connector */

  uint16_t       usFlags;          /*! connection management flags, see HIL_MARSHALLER_CONNECTION_FLAG_xxx */
  uint32_t       ulTimeout;        /*!< timeout in ms, see HIL_MARSHALLER_CONNECTION_TIMEOUT_xxx */
  void*          pvConfigData;     /*!< Depends on CONNECTION_TYPE */

  /* NOTE: These information were added to the end of the structure to be old
           compatible. When using the previous structure initializations these fields
           will automatically be set to 0 */
  uint32_t       ulTxBufferSize;   /*!< TX Buffer size for unsolicited packets / calls (indication / callbacks)  */
  uint32_t       ulTxBufferCnt;    /*!< TX Buffer count for unsolicited packets / calls (indication / callbacks) */
};

typedef struct HIL_MARSHALLER_CONNECTOR_PARAMS_Ttag HIL_MARSHALLER_CONNECTOR_PARAMS_T;

typedef uint32_t(*PFN_TRANSPORT_INIT)(void* pvMarshaller, void* pvConfig);

/*****************************************************************************/
/*! Transport layer configuration                                            */
/*****************************************************************************/
typedef struct TRANSPORT_LAYER_CONFIG_Ttag
{
  PFN_TRANSPORT_INIT pfnInit;  /*!< Initialization function */
  void*              pvConfig; /*!< Configuration data      */

} TRANSPORT_LAYER_CONFIG_T;

typedef void(*PFN_MARSHALLER_REQUEST)(void* pvMarshaller, void* pvUser);

/*****************************************************************************/
/*! Marshaller startup parameters                                            */
/*****************************************************************************/
typedef struct HIL_MARSHALLER_PARAMS_Ttag
{
  char          szServerName[32];
  uint32_t      ulMaxConnectors;               /*!< Maximum number of connectors */

  uint32_t      ulConnectorCnt;                /*!< Number of connectors to automatically load at startup */
  const HIL_MARSHALLER_CONNECTOR_PARAMS_T* ptConnectors;

  /* Add transport layers, currently only cifX and rcX Packet support */
  uint32_t      ulTransportCnt;                /*!< Number of transports to automatically load at startup */
  const TRANSPORT_LAYER_CONFIG_T* atTransports;  /*!< Array of function pointer to initialize Transports    */

} HIL_MARSHALLER_PARAMS_T;


/*****************************************************************************/
/*! Marshaller function prototypes                                           */
/*****************************************************************************/

uint32_t HilMarshallerStart (const HIL_MARSHALLER_PARAMS_T* ptParams, void** ppvMarshHandle, PFN_MARSHALLER_REQUEST pfnRequest, void* pvUser);
void     HilMarshallerStop  (void* pvMarshHandle);
void     HilMarshallerTimer (void* pvMarshaller);
uint32_t HilMarshallerMain  (void* pvMarshaller);


/*****************************************************************************/
/*! HilMarshallerSetMode() sets the mode (unrestricted / restricted / disabled)
*   either of a single connector or of all connectors.
*   \param pvMarshaller     Marshaller handle
*   \param ulMode           see MARSHALLER_MODE_xxx constants above
*   \param ulConnectorID    connector index or MARSHALLER_CONNECTORS_ALL
*   \return HIL_MARSHALLER_E_SUCCESS     on success
*   \ HIL_MARSHALLER_E_INVALIDPARAMETER  if mode invalid
*   \ HIL_MARSHALLER_E_OUTOFRESOURCES    if connector reference invalid      */
/*****************************************************************************/

uint32_t HilMarshallerSetMode(void*    pvMarshaller,
                              uint32_t ulMode,
                              uint32_t ulConnectorID);

#ifdef __cplusplus
  }
#endif  /* __cplusplus */

/*****************************************************************************/
/*  \}                                                                       */
/*****************************************************************************/

#endif /* __MARSHALLERCONFIG__H */
