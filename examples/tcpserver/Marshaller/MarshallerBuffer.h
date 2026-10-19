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
    Marshaller internal buffer definitions

   Changes:

   Version   Date        Author   Description
   ----------------------------------------------------------------------------------
   1         2025-08-05  MSC      initial version

**************************************************************************************/

#ifndef __MARSHALLERBUFFER__H
#define __MARSHALLERBUFFER__H

#include "MarshallerConfig.h"
#include <sys/queue.h>

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
    uint32_t         ulConnectorIdx; /*!< Connector number of this buffer */
    HIL_MARSHALLER_H hMarshaller;    /*!< Marshaller object this buffer has been allocated by */

    MARSHALLER_BUFFER_TYPE_E eType; /*!< Type of buffer */

    uint32_t ulDataBufferLen;     /*!< Total length of buffer (only counting the size of abData[] */
    uint32_t ulUsedDataBufferLen; /*!< Number of used bytes in abData[] */

    uint32_t ulActualSendOffset; /*!< Can be used by connector for fragmented sending   */
  } tMgmt;

  HIL_TRANSPORT_HEADER tTransport; /*!< Transport header */
  uint8_t              abData[1];  /*!< User data        */
};

typedef struct HIL_MARSHALLER_BUFFER_Ttag HIL_MARSHALLER_BUFFER_T;

STAILQ_HEAD(MARSHALLER_BUFFER_HEAD, HIL_MARSHALLER_BUFFER_Ttag);

#endif /* __MARSHALLERBUFFER__H */
