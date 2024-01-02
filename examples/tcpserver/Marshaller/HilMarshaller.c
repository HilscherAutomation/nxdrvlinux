/**************************************************************************************
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $Id: HilMarshaller.c 14530 2022-06-27 12:01:46Z AMinor $:

Description:
    Hilscher Transport marshalling main module

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2022-06-27  Fix handling in HilMarshallerSetMode() to change mode of a single connector
    2020-11-12  Moved OS functions to separate implementation module
    2019-01-23  Bugfix:
                - Fixed wrong deinitialization order in HilMarshallerStop()
    2018-08-13  Change:
                - Removed pclint warnings
                - Updated header to new version
    2015-07-16  Change:
                - Adapted to new APIHeader directory and MarshallerFrame.h
    2013-05-06  Bugfix:
                - ACK's or zero length packets, may be send with a checksum (!=0)
    2010-06-07  Change:
                - Re-Added Tx buffers, which were never unused in Packet Transport (on rcX)
                - Added ulTxBufferSize / ulTxBufferSize on connector registration to setup
                  Tx Buffer usage (if needed)
    2009-09-23  Change:
                - Removed unused Tx buffers and doubled number of Rx buffers to avoid resource errors without increasing memory footprint.
    2009-09-22  Addon:
                - Added HilMarshallerSetMode(), mode evaluation in HilMarshallerConnRxData().
    2009-09-04  Bugfix:
                - Do not discard "superfluous" data on reception of a transport ACK from the host. Instead, start a new receive cycle.
    2009-09-02  Change:
                - HilMarshallerMain() returns TLR_E_FAIL if no message can be retrieved from the pending requests list.
    2009-09-01  Bugfix:
                - Removed redundant calls to OS_FREE() following call to HilMarshallerStop() after failure in HilMarshallerStart().
    2009-08-26  Change:
                - Use HilMarshallerGetBuffer() and HilMarshallerFreeBuffer() in SendAcknowledge() instead of STAILQ_xxx macros.
    2009-08-25  Bugfix:
                - No need to perform buffer handling in case of incoming Tx Ack.
    2009-08-19  Bugfixes:
                - Corrected calculation of the used buffer length for a QUERYSERVER response.
                - Zero ulActualSendOffset when allocating buffer.
    2009-08-13  Bugfix:
                - Set fAnswer flag to trigger response transmission also for regular Keep Alive message.
    2009-08-06  Bugfix:
                - Changed creation of the QUERYSERVER response to be in line with the specification.
    2009-08-04  Bugfix:
                - Keep Alive response: Prevent buffer from being freed by ResetRxStateMachine() while not transmitted completely.
    2009-07-28  Change:
                - Moved Marshaller version constants to MarshallerVersion.h
    2009-07-21  Bugfix:
                - Need to initialize Connectors before Transports because
                  Transport initialization uses Connector data
    2009-06-02  Bugfix:
                - Buffer handling for Connectors with Idx > 0 did not work
                  (Connectors will run out of buffers)
    2009-05-25  initial version

**************************************************************************************/

/*****************************************************************************/
/*! \file HilMarshaller.c
*   netX Marshaller implementation                                           */
/*****************************************************************************/

#include "OS_Dependent.h"
#include "MarshallerConfig.h"
#include "MarshallerInternal.h"
#include "MarshallerFrame.h"
#include "MarshallerVersion.h"


#ifndef min
  #define min(a,b)  ((a > b)? b : a)
#endif

/*****************************************************************************/
/*! \addtogroup NETX_MARSHALLER_MAIN
*   \{                                                                       */
/*****************************************************************************/

/*****************************************************************************/
/*! Calculate CCITT/ITU CRC-16 checksum.
*   Polynomial  : x^16 + x^12 + x^5 + 1 (0x1021)
*   Start value : 0xFFFF
*   Result      : XORed with 0xFFFF
*    \param pbData        Buffer to calculate CRC for
*    \param ulDataLength  Length of buffer
*    \return CRC-16 checksum                                                 */
/*****************************************************************************/
static uint16_t CalculateCRC16 (const uint8_t* pbData, uint32_t ulDataLength)
{
  uint16_t usCRC;
  uint32_t ulOffset;

  usCRC = 0xFFFF;
  if (pbData != NULL && ulDataLength > 0)
  { /* Buffer address and data length are valid. */
    for (ulOffset = 0; ulOffset < ulDataLength; ++ulOffset)
    { /* Update CRC value for each byte in the buffer. */
      usCRC =  (uint16_t)((usCRC >> 8) | (usCRC << 8));
      usCRC =  (uint16_t)(usCRC ^ pbData[ulOffset]);
      usCRC ^= (uint16_t)((usCRC & 0xFF) >> 4);
      usCRC ^= (uint16_t)((usCRC << 8) << 4);
      usCRC ^= (uint16_t)(((usCRC & 0xFF) << 4) << 1);
    }
    usCRC = (uint16_t) (~usCRC);
  }
  return (usCRC);
}

/*****************************************************************************/
/*! Allocates the requested data buffers and inserts them in free list
*   of connector
*    \param pvMarshaller   Marshaller handle
*    \param ptConnector    Connector to allocate buffers for
*    \param ulRxBufferCnt  Number of buffers to allocate (equals number of parallel services)
*    \param ulRxBufferSize Size of the buffers in bytes
*    \param ulTxBufferCnt  Number of transmit buffers to allocate (for unsolicited/indication data)
*    \param ulTxBufferSize Size of the transmit buffers in bytes
*    \return true on success                                                 */
/*****************************************************************************/
static bool AllocateBuffers(void* pvMarshaller, CONNECTOR_DATA_T* ptConnector, uint32_t ulRxBufferCnt, uint32_t ulRxBufferSize, uint32_t ulTxBufferCnt, uint32_t ulTxBufferSize)
{
  uint32_t  ulIdx;
  bool fRet = true;

  STAILQ_INIT(&ptConnector->tRxBuffer);
  STAILQ_INIT(&ptConnector->tTxBuffer);
  STAILQ_INIT(&ptConnector->tAckBuffer);
  STAILQ_INIT(&ptConnector->tKeepAliveBuffer);

  /* Use twice the number of requested data buffers for Rx data,
     to make sure we have some reserve in case a buffer has not yet been released after the confirmation has been sent */
  for(ulIdx = 0; ulIdx < ulRxBufferCnt; ++ulIdx)
  {
    HIL_MARSHALLER_BUFFER_T* ptBuffer;

    if(NULL == (ptBuffer = OS_Malloc((uint32_t)sizeof(*ptBuffer) + ulRxBufferSize)))
    {
      fRet = false;
      break;
    } else
    {
      ptBuffer->tMgmt.ulConnectorIdx      = ptConnector->ulConnectorIdx;
      ptBuffer->tMgmt.pvMarshaller        = pvMarshaller;
      ptBuffer->tMgmt.ulDataBufferLen     = ulRxBufferSize;
      ptBuffer->tMgmt.eType               = eMARSHALLER_RX_BUFFER;
      ptBuffer->tMgmt.ulUsedDataBufferLen = 0;

      /* Enqueue in linked list */
      STAILQ_INSERT_TAIL(&ptConnector->tRxBuffer, ptBuffer, tList);
    }
  }

  for(ulIdx = 0; ulIdx < ulTxBufferCnt; ++ulIdx)
  {
    HIL_MARSHALLER_BUFFER_T* ptBuffer;

    if(NULL == (ptBuffer = OS_Malloc((uint32_t)sizeof(*ptBuffer) + ulTxBufferSize)))
    {
      fRet = false;
      break;
    } else
    {
      ptBuffer->tMgmt.ulConnectorIdx      = ptConnector->ulConnectorIdx;
      ptBuffer->tMgmt.pvMarshaller        = pvMarshaller;
      ptBuffer->tMgmt.ulDataBufferLen     = ulTxBufferSize;
      ptBuffer->tMgmt.eType               = eMARSHALLER_TX_BUFFER;
      ptBuffer->tMgmt.ulUsedDataBufferLen = 0;
      /* Enqueue in linked list */
      STAILQ_INSERT_TAIL(&ptConnector->tTxBuffer, ptBuffer, tList);
    }
  }

  /* Use one additional Buffer per direction for acknowledgement,
     to make sure we can answer while we don't have databuffers
     ready (number of parallel services exceeded) */
  for(ulIdx = 0; ulIdx < ulRxBufferCnt + ulTxBufferCnt + 2; ++ulIdx)
  {
    HIL_MARSHALLER_BUFFER_T* ptBuffer;

    if(NULL == (ptBuffer = OS_Malloc(sizeof(*ptBuffer))))
    {
      fRet = false;
      break;
    } else
    {
      ptBuffer->tMgmt.ulConnectorIdx      = ptConnector->ulConnectorIdx;
      ptBuffer->tMgmt.pvMarshaller        = pvMarshaller;
      ptBuffer->tMgmt.ulDataBufferLen     = 0;
      ptBuffer->tMgmt.eType               = eMARSHALLER_ACK_BUFFER;
      ptBuffer->tMgmt.ulUsedDataBufferLen = 0;

      /* Enqueue in linked list */
      STAILQ_INSERT_TAIL(&ptConnector->tAckBuffer, ptBuffer, tList);
    }
  }

  if(fRet)
  {
    HIL_MARSHALLER_BUFFER_T* ptBuffer;

    if(NULL == (ptBuffer = OS_Malloc(sizeof(*ptBuffer) + sizeof(HIL_TRANSPORT_KEEPALIVE_DATA_T))))
    {
      fRet = false;
    } else
    {
      ptBuffer->tMgmt.ulConnectorIdx      = ptConnector->ulConnectorIdx;
      ptBuffer->tMgmt.pvMarshaller        = pvMarshaller;
      ptBuffer->tMgmt.ulDataBufferLen     = sizeof(HIL_TRANSPORT_KEEPALIVE_DATA_T);
      ptBuffer->tMgmt.eType               = eMARSHALLER_KEEPALIVE_BUFFER;
      ptBuffer->tMgmt.ulUsedDataBufferLen = 0;

      /* Enqueue in linked list */
      STAILQ_INSERT_TAIL(&ptConnector->tKeepAliveBuffer, ptBuffer, tList);
    }
  }

  return fRet;
}

/*****************************************************************************/
/*! Deallocates all data buffers from a connector
*    \param pvMarshaller Marshaller handle
*    \param ptConnector  Connector to free buffers for                      */
/*****************************************************************************/
static void DeAllocateBuffers(void* pvMarshaller, CONNECTOR_DATA_T* ptConnector)
{
  /* Deallocate all buffers in linked lists */
  HIL_MARSHALLER_BUFFER_T* ptBuffer;

  while(NULL != (ptBuffer = STAILQ_FIRST(&ptConnector->tAckBuffer)))
  {
    STAILQ_REMOVE(&ptConnector->tAckBuffer, ptBuffer, HIL_MARSHALLER_BUFFER_Ttag, tList);

    OS_Free(ptBuffer);
  }

  while(NULL != (ptBuffer = STAILQ_FIRST(&ptConnector->tRxBuffer)))
  {
    STAILQ_REMOVE(&ptConnector->tRxBuffer, ptBuffer, HIL_MARSHALLER_BUFFER_Ttag, tList);

    OS_Free(ptBuffer);
  }

  while(NULL != (ptBuffer = STAILQ_FIRST(&ptConnector->tTxBuffer)))
  {
    STAILQ_REMOVE(&ptConnector->tTxBuffer, ptBuffer, HIL_MARSHALLER_BUFFER_Ttag, tList);

    OS_Free(ptBuffer);
  }

  while(NULL != (ptBuffer = STAILQ_FIRST(&ptConnector->tKeepAliveBuffer)))
  {
    STAILQ_REMOVE(&ptConnector->tKeepAliveBuffer, ptBuffer, HIL_MARSHALLER_BUFFER_Ttag, tList);

    OS_Free(ptBuffer);
  }
}

/*****************************************************************************/
/*! Free a buffer
*    \param ptBuffer Buffer to free                                         */
/*****************************************************************************/
void HilMarshallerFreeBuffer(HIL_MARSHALLER_BUFFER_T* ptBuffer)
{
  HIL_MARSHALLER_DATA_T* ptMarshaller = (HIL_MARSHALLER_DATA_T*)ptBuffer->tMgmt.pvMarshaller;
  CONNECTOR_DATA_T*      ptConn       = &ptMarshaller->atConnectors[ptBuffer->tMgmt.ulConnectorIdx];
  int                    iLock;

  ptBuffer->tMgmt.ulUsedDataBufferLen = 0;

  iLock = OS_Lock();

  switch(ptBuffer->tMgmt.eType)
  {
  case eMARSHALLER_KEEPALIVE_BUFFER:
    STAILQ_INSERT_TAIL(&ptConn->tKeepAliveBuffer, ptBuffer, tList);
    break;

  case eMARSHALLER_ACK_BUFFER:
    STAILQ_INSERT_TAIL(&ptConn->tAckBuffer, ptBuffer, tList);
    break;

  case eMARSHALLER_RX_BUFFER:
    STAILQ_INSERT_TAIL(&ptConn->tRxBuffer, ptBuffer, tList);
    break;

  case eMARSHALLER_TX_BUFFER:
    STAILQ_INSERT_TAIL(&ptConn->tTxBuffer, ptBuffer, tList);
    break;

  default:
    /* NOTE: This should never happen, only if someone free's a wrong buffer,
             or destroyed the buffer management area (programming error). */
    ptConn = ptConn;
    break;
  }

  OS_Unlock(iLock);
} /*lint !e438 : Last value assigned to variable 'ptConn' (defined at line 276) not used */

/*****************************************************************************/
/*! Get a buffer for incoming data from connector
*    \param pvMarshaller Marshaller handle
*    \param eType        Type of buffer to acquire
*    \param ulConnector  Connector index
*    \return NULL if no buffer is available, valid buffer otherwise          */
/*****************************************************************************/
HIL_MARSHALLER_BUFFER_T* HilMarshallerGetBuffer(void* pvMarshaller, MARSHALLER_BUFFER_TYPE_E eType, uint32_t ulConnector)
{
  HIL_MARSHALLER_DATA_T*   ptMarshaller     = (HIL_MARSHALLER_DATA_T*)pvMarshaller;
  CONNECTOR_DATA_T*        ptConn           = &ptMarshaller->atConnectors[ulConnector];
  HIL_MARSHALLER_BUFFER_T* ptBuffer         = NULL;
  struct MARSHALLER_BUFFER_HEAD* ptListHead = NULL;
  int                      iLock;

  switch(eType)
  {
  case eMARSHALLER_RX_BUFFER:
    ptListHead = &ptConn->tRxBuffer;
    break;

  case eMARSHALLER_TX_BUFFER:
    ptListHead = &ptConn->tTxBuffer;
    break;

  case eMARSHALLER_ACK_BUFFER:
    ptListHead = &ptConn->tAckBuffer;
    break;

  case eMARSHALLER_KEEPALIVE_BUFFER:
    ptListHead = &ptConn->tKeepAliveBuffer;
    break;

  default:
    /* NOTE: This should never happen, only if someone free's a wrong buffer,
             or destroyed the buffer management area (programming error). */
    break;
  }

  iLock = OS_Lock();

  if( (NULL != ptListHead) &&
      (NULL != (ptBuffer = STAILQ_FIRST(ptListHead))) )
  {
    STAILQ_REMOVE(ptListHead, ptBuffer, HIL_MARSHALLER_BUFFER_Ttag, tList);
    ptBuffer->tMgmt.ulActualSendOffset  = 0;
    ptBuffer->tMgmt.ulUsedDataBufferLen = 0;
  }

  OS_Unlock(iLock);

  return ptBuffer;
}

/*****************************************************************************/
/*! Find a transport layer which can handle the requested data type
*    \param ptMarshaller Marshaller handle
*    \param usDataType   Datatype the transport layer is requested for
*    \return NULL if no transport is available, valid transport otherwise    */
/*****************************************************************************/
static TRANSPORT_LAYER_DATA_T* FindTransportLayer(const HIL_MARSHALLER_DATA_T* ptMarshaller, const uint16_t usDataType)
{
  TRANSPORT_LAYER_DATA_T* ptRet = NULL;
  uint32_t                ulIdx;

  for(ulIdx = 0; ulIdx < ptMarshaller->ulTransports; ++ulIdx)
  {
    if(usDataType == ptMarshaller->ptTransports[ulIdx].usDataType)
    {
      ptRet = &ptMarshaller->ptTransports[ulIdx];
      break;
    }
  }

  return ptRet;
}

/*****************************************************************************/
/*! Send an HIL Transport acknowledge
*    \param ptMarshaller Marshaller handle
*    \param ulConnector  Connector to send ACK to
*    \param ptHeader     Header of incoming packet
*    \param bState       State to set in acknowledge                        */
/*****************************************************************************/
static void SendAcknowledge(HIL_MARSHALLER_DATA_T* ptMarshaller, uint32_t ulConnector, HIL_TRANSPORT_HEADER* ptHeader, unsigned char bState)
{
  HIL_MARSHALLER_BUFFER_T* ptBuffer;
  /* If we don't get a buffer, the number of parallel services is exceeded.
     This is a host error, as it is sending data too fast and we are not able to do any acknowledge */
  ptBuffer = HilMarshallerGetBuffer(ptMarshaller, eMARSHALLER_ACK_BUFFER, ulConnector);
  if(NULL != ptBuffer)
  {
    ptBuffer->tTransport                = *ptHeader;
    ptBuffer->tTransport.bState         = bState;
    ptBuffer->tTransport.usDataType     = HIL_TRANSPORT_TYPE_ACKNOWLEDGE;
    ptBuffer->tMgmt.ulUsedDataBufferLen = 0;
    if(HIL_MARSHALLER_E_SUCCESS != HilMarshallerConnTxData(ptMarshaller, ulConnector, ptBuffer))
    {
      /* Internal error, this should never happen, but if it happens, we need to free the Ack buffer again */
      HilMarshallerFreeBuffer(ptBuffer);
    }
  }
}

/*****************************************************************************/
/*! Reset internal connector receive state machine
*    \param ptConnector  Connector to reset                                 */
/*****************************************************************************/
static void ResetRxStateMachine(CONNECTOR_DATA_T* ptConnector)
{

  if((ptConnector->eScanState != HIL_SEARCH_TELEGRAM_COOKIE) &&
     (ptConnector->eScanState != HIL_SCAN_DONE)              )
  {
    ptConnector->eScanState = ptConnector->eScanState;
  }

  ptConnector->fMonitorTimeout    = false;
  ptConnector->ulElapsedTime      = 0;
  ptConnector->tRxHeader.ulCookie = 0;
  ptConnector->ulRxOffset         = 0;
  if(NULL != ptConnector->ptCurrentRxBuffer)
  {
    HilMarshallerFreeBuffer(ptConnector->ptCurrentRxBuffer);
  }

  ptConnector->ptCurrentRxBuffer  = NULL;
  ptConnector->eScanState         = HIL_SEARCH_TELEGRAM_COOKIE;
}


/*****************************************************************************/
/*! Startup marshaller
*    \param ptParams       Marshaller parameters
*    \param ppvMarshHandle Returned Marshaller handle
*    \param pfnRequest     Function to call, when MarshallerMain should be called
*    \param pvUser         User parameter to pass on pfnRequest call
*    \return HIL_MARSHALLER_E_SUCCESS on success                             */
/*****************************************************************************/
uint32_t HilMarshallerStart(const HIL_MARSHALLER_PARAMS_T* ptParams, void** ppvMarshHandle, PFN_MARSHALLER_REQUEST pfnRequest, void* pvUser)
{
  uint32_t                                 eRet         = HIL_MARSHALLER_E_SUCCESS;
  HIL_MARSHALLER_DATA_T*                   ptMarshaller = NULL;
  uint32_t                                 ulIdx        = 0;
  const TRANSPORT_LAYER_CONFIG_T*          ptTransport  = NULL;
  const HIL_MARSHALLER_CONNECTOR_PARAMS_T* ptConnParams = NULL;

  if(NULL == ppvMarshHandle)
    eRet = HIL_MARSHALLER_E_INVALIDPARAMETER;
  else
  {
    *ppvMarshHandle = NULL;
    if (NULL == (ptMarshaller = OS_Malloc(sizeof(*ptMarshaller))))
      eRet = HIL_MARSHALLER_E_OUTOFMEMORY;
    else
    {
      OS_Memset( ptMarshaller, 0, sizeof(*ptMarshaller));
      STAILQ_INIT(&ptMarshaller->tPendingRequests);
      ptMarshaller->pfnRequest      = pfnRequest;
      ptMarshaller->pvUser          = pvUser;
      ptMarshaller->ulMaxConnectors = ptParams->ulMaxConnectors;
      ptMarshaller->ulTransports    = ptParams->ulTransportCnt;
      OS_Memcpy(ptMarshaller->szServerName, (void*)ptParams->szServerName, sizeof(ptMarshaller->szServerName));
      if(NULL == (ptMarshaller->atConnectors = OS_Malloc(ptParams->ulMaxConnectors * (uint32_t)sizeof(CONNECTOR_DATA_T))))
        eRet = HIL_MARSHALLER_E_OUTOFMEMORY;
      else
      {
        OS_Memset(ptMarshaller->atConnectors, 0, ptParams->ulMaxConnectors * sizeof(CONNECTOR_DATA_T));
        if(NULL == (ptMarshaller->ptTransports = OS_Malloc(ptParams->ulTransportCnt * (uint32_t)sizeof(TRANSPORT_LAYER_DATA_T))))
          eRet = HIL_MARSHALLER_E_OUTOFMEMORY;
        else
        {
          OS_Memset(ptMarshaller->ptTransports, 0, ptParams->ulTransportCnt * sizeof(TRANSPORT_LAYER_DATA_T));
          for(ulIdx = 0; eRet == HIL_MARSHALLER_E_SUCCESS && ulIdx < ptParams->ulConnectorCnt; ++ulIdx)
          {
            ptConnParams = &ptParams->ptConnectors[ulIdx];
            if(ptConnParams->pfnConnectorInit)
              eRet = ptConnParams->pfnConnectorInit(ptConnParams, ptMarshaller);
          }
          for(ulIdx = 0; eRet == HIL_MARSHALLER_E_SUCCESS && ulIdx < ptParams->ulTransportCnt; ++ulIdx)
          {
            ptTransport = &ptParams->atTransports[ulIdx];
            if(ptTransport->pfnInit)
              eRet = ptTransport->pfnInit(ptMarshaller, ptTransport->pvConfig);
          }
        }
      }
    }
    if(HIL_MARSHALLER_E_SUCCESS == eRet)
      *ppvMarshHandle = ptMarshaller;
    else
      HilMarshallerStop(ptMarshaller);
  }
  return eRet;   /*lint !e593 : 'ptMarshaller' possibly not freed or returned */
}

/*****************************************************************************/
/*! Stop marshaller
*    \param pvMarshHandle  Marshaller handle                                 */
/*****************************************************************************/
void HilMarshallerStop(void* pvMarshHandle)
{
  HIL_MARSHALLER_DATA_T* ptMarshaller = (HIL_MARSHALLER_DATA_T*)pvMarshHandle;

  if(NULL != ptMarshaller)
  {
    if(NULL != ptMarshaller->ptTransports)
    {
      uint32_t ulTrans;

      for(ulTrans = 0; ulTrans < ptMarshaller->ulTransports; ++ulTrans)
      {
        TRANSPORT_LAYER_DATA_T* ptTransport = &ptMarshaller->ptTransports[ulTrans];

        if(ptTransport->pfnDeinit)
        {
          ptTransport->pfnDeinit(ptTransport->pvUser);
        }
      }

      OS_Free(ptMarshaller->ptTransports);
      ptMarshaller->ptTransports = NULL;
    }

    if(NULL != ptMarshaller->atConnectors)
    {
      uint32_t ulConn;

      for(ulConn = 0; ulConn < ptMarshaller->ulMaxConnectors; ++ulConn)
      {
        CONNECTOR_DATA_T* ptConn = &ptMarshaller->atConnectors[ulConn];

        if(ptConn->fInUse)
        {
          ptConn->tConn.pfnDeinit(ptConn->tConn.pvUser);
        }
      }

      OS_Free(ptMarshaller->atConnectors);
      ptMarshaller->atConnectors = NULL;
    }

    OS_Free(ptMarshaller);
  }
}

/*****************************************************************************/
/*! Register a connector at marshaller
*    \param pvMarshaller     Marshaller handle
*    \param pulConnectorIdx  Returned connector number
*    \param ptConn           Connector data
*    \return HIL_MARSHALLER_E_SUCCESS on success                             */
/*****************************************************************************/
uint32_t HilMarshallerRegisterConnector(void* pvMarshaller, uint32_t* pulConnectorIdx, HIL_MARSHALLER_CONNECTOR_T* ptConn)
{
  HIL_MARSHALLER_DATA_T* ptMarshaller    = (HIL_MARSHALLER_DATA_T*)pvMarshaller;
  uint32_t               eRet            = HIL_MARSHALLER_E_SUCCESS;
  CONNECTOR_DATA_T*      ptConnectorData = NULL;
  uint32_t               ulConnIdx;

  /* Search free connector space */
  for(ulConnIdx = 0; ulConnIdx < ptMarshaller->ulMaxConnectors; ++ulConnIdx)
  {
    if(!ptMarshaller->atConnectors[ulConnIdx].fInUse)
    {
      ptConnectorData = &ptMarshaller->atConnectors[ulConnIdx];
      ptConnectorData->ulConnectorIdx  = ulConnIdx;
      break;
    }
  }

  if(NULL == ptConnectorData)
  {
    /* No more connectors allowed */
    eRet = HIL_MARSHALLER_E_OUTOFRESOURCES;

  } else if(!AllocateBuffers(pvMarshaller,
                             ptConnectorData,
                             ptConn->ulDataBufferCnt,
                             ptConn->ulDataBufferSize,
                             ptConn->ulTxBufferCnt,
                             ptConn->ulTxBufferSize))
  {
    /* Out of memory */
    eRet = HIL_MARSHALLER_E_OUTOFMEMORY;
  } else
  {
    /* Initialize RX state machine */
    ResetRxStateMachine(ptConnectorData);

    /* Initialize conector data */
    ptConnectorData->fInUse          = true;
    ptConnectorData->tConn           = *ptConn;
    *pulConnectorIdx                 = ulConnIdx;
  }

  return eRet;
}

/*****************************************************************************/
/*! Unregister a connector at marshaller
*    \param pvMarshaller     Marshaller handle
*    \param ulConnectorIdx   Connector number                               */
/*****************************************************************************/
void HilMarshallerUnregisterConnector(void* pvMarshaller, uint32_t ulConnectorIdx)
{
  HIL_MARSHALLER_DATA_T* ptMarshaller = (HIL_MARSHALLER_DATA_T*)pvMarshaller;

  if(ulConnectorIdx < ptMarshaller->ulMaxConnectors)
  {
    CONNECTOR_DATA_T* ptConnectorData = &ptMarshaller->atConnectors[ulConnectorIdx];

    DeAllocateBuffers(pvMarshaller,
                      ptConnectorData);

    ptConnectorData->fInUse = false;
    OS_Memset(&ptConnectorData->tConn, 0, sizeof(ptConnectorData->tConn));
  }
}

/*****************************************************************************/
/*! Called by connector when new data has arrived
*    \param pvMarshaller     Marshaller handle
*    \param ulConnector      Connector number
*    \param pbData           Incoming data
*    \param ulDataCnt        Number of bytes in incoming data buffer
*    \return HIL_MARSHALLER_E_SUCCESS on success                             */
/*****************************************************************************/
uint32_t HilMarshallerConnRxData(void* pvMarshaller, uint32_t ulConnector, uint8_t* pbData, uint32_t ulDataCnt)
{
  HIL_MARSHALLER_DATA_T* ptMarshaller = (HIL_MARSHALLER_DATA_T*)pvMarshaller;
  uint32_t               eRet         = HIL_MARSHALLER_E_INVALIDPARAMETER;

  if (ulConnector < ptMarshaller->ulMaxConnectors
  &&  ptMarshaller->atConnectors[ulConnector].ulMode != HIL_MARSHALLER_MODE_DISABLED)
  {
    CONNECTOR_DATA_T* ptConnector = &ptMarshaller->atConnectors[ulConnector];
    int               fDone       = 0;    /* Leave handler */
    uint32_t          ulRxDataIdx = 0;

    eRet = HIL_MARSHALLER_E_SUCCESS;
    ptConnector->ulElapsedTime    = 0;      /* Reschedule timeout handling */

    /*-----------------------------------------*/
    /* Start scanning of the input data        */
    /*-----------------------------------------*/
    do
    {
      switch(ptConnector->eScanState)
      {
        case HIL_SEARCH_TELEGRAM_COOKIE:
        {
          /* Parse input buffer */
          unsigned char* pabCookie = (unsigned char*)&ptConnector->tRxHeader.ulCookie;

          for ( ulRxDataIdx = 0; ulRxDataIdx < ulDataCnt; ulRxDataIdx++)
          {
            pabCookie[ptConnector->ulRxOffset] = pbData[ulRxDataIdx];
            if( ++ptConnector->ulRxOffset >= sizeof(ptConnector->tRxHeader.ulCookie))
            {
              /* Check if we have a complete cookie */
              if( HIL_TRANSPORT_COOKIE == ptConnector->tRxHeader.ulCookie)
              {
                /* Yes, store the cookie in the header */
                ulDataCnt = ulDataCnt - (ulRxDataIdx + 1);
                pbData    = &pbData[ulRxDataIdx + 1];

                /* Set next state */
                ptConnector->eScanState      = HIL_SEARCH_TELEGRAM_HEADER;
                ptConnector->fMonitorTimeout = true;
                break;
              }else
              {
                  /* Still no cookie, move the data and insert next character. */
                /* Set cookie buffer index to next insertable character. */
                  memmove(pabCookie, &pabCookie[1], 3);
                  ptConnector->ulRxOffset = 3;
              }
            }
          }

          if( HIL_SEARCH_TELEGRAM_COOKIE == ptConnector->eScanState)
              fDone = 1;

          break;
        }

        case HIL_SEARCH_TELEGRAM_HEADER:
        {
          /* We searching a telegram header, cookie is already available */
          /* Check if we have more date */
          if ( 0 == ulDataCnt)
          {
            fDone = 1;
          } else
          {
            /* Append at least the header length if available */
            uint32_t       ulCopyLength = 0;
            unsigned char* pbHeader     = (unsigned char*)&ptConnector->tRxHeader;

            ulCopyLength = min(ulDataCnt,
                               (sizeof(HIL_TRANSPORT_HEADER) - ptConnector->ulRxOffset));

            OS_Memcpy( &pbHeader[ptConnector->ulRxOffset],
                       pbData,
                       ulCopyLength);

            ptConnector->ulRxOffset += ulCopyLength;

            /* Check if we have more data */
            if(ptConnector->ulRxOffset < sizeof(HIL_TRANSPORT_HEADER))
            {
              /* We need more data */
              fDone = 1;

            } else if (HIL_TRANSPORT_TYPE_ACKNOWLEDGE == ptConnector->tRxHeader.usDataType)
            {
              /* Update the Rx data count and Rx data pointer for handling the remaining bytes */
              ulDataCnt -= ulCopyLength;
              pbData     = &pbData[ulCopyLength];
              ptConnector->ptCurrentRxBuffer = NULL;
              ResetRxStateMachine(ptConnector);
              /* If there are bytes left, start a new cookie search using these bytes. Otherwise terminate the receive cycle. */
              if (ulDataCnt == 0)
                fDone = 1;
              break;
            } else
            {
              /* We need a buffer */

              HIL_MARSHALLER_BUFFER_T* ptBuffer;

              if(HIL_TRANSPORT_TYPE_KEEP_ALIVE == ptConnector->tRxHeader.usDataType)
              {
                ptBuffer = HilMarshallerGetBuffer(ptMarshaller, eMARSHALLER_KEEPALIVE_BUFFER, ulConnector);

              } else
              {
                ptBuffer = HilMarshallerGetBuffer(ptMarshaller, eMARSHALLER_RX_BUFFER, ulConnector);
              }

              if(NULL == ptBuffer)
              {
                /* Send negative Ack */
                SendAcknowledge(ptMarshaller, ulConnector, &ptConnector->tRxHeader, HIL_TSTATE_RESOURCE_ERROR);
                ResetRxStateMachine(ptConnector);

              } else if(ptBuffer->tMgmt.ulDataBufferLen < ptConnector->tRxHeader.ulLength)
              {
                /* Send negative ACK (telegram too long for buffer) */
                SendAcknowledge(ptMarshaller, ulConnector, &ptConnector->tRxHeader, HIL_TSTATE_BUFFEROVERFLOW_ERROR);

                /* Release Buffer */
                HilMarshallerFreeBuffer(ptBuffer);
                ResetRxStateMachine(ptConnector);

              } else
              {
                /* We have a complete Header, wait for packet data complete */
                ulDataCnt -= ulCopyLength;
                pbData    = &pbData[ulCopyLength];

                /* Store header */
                ptBuffer->tTransport                = ptConnector->tRxHeader;
                ptBuffer->tMgmt.ulUsedDataBufferLen = 0; /* Set actual telegram data length */

                /* Set next scanner state */
                ptConnector->ptCurrentRxBuffer = ptBuffer;
                ptConnector->eScanState        = HIL_WAIT_TELEGRAM_DATA;
              }
            }
          }
        }
        break;

        case HIL_WAIT_TELEGRAM_DATA:
        {
          /* We waiting for telegram data, wait until all data are available */
          uint32_t              ulCopyLen    = 0;
          HIL_TRANSPORT_HEADER* ptHeader     = &ptConnector->ptCurrentRxBuffer->tTransport;
          uint32_t              ulDataOffset = ptConnector->ulRxOffset - (uint32_t)sizeof(HIL_TRANSPORT_HEADER);

          if(0 == ptHeader->ulLength)
          {
            /* We have all data */
            ptConnector->eScanState = HIL_CHECK_TELEGRAM;

          /* We have packet data, wait until all data are available */
          } else if(0 != ulDataCnt)
          {
            HIL_MARSHALLER_BUFFER_T* ptBuffer = ptConnector->ptCurrentRxBuffer;

              /* Check if all data must be copied or if we have more data than necessary */
            ulCopyLen = ulDataCnt;
              if( (ulDataCnt + ulDataOffset) > ptHeader->ulLength)
              {
                  /* Just copy the necessary data and keep the rest for a new cookie ssearch */
              ulCopyLen = ptHeader->ulLength - ulDataOffset;
              }

            OS_Memcpy(&ptBuffer->abData[ulDataOffset], pbData, ulCopyLen);
            ptConnector->ulRxOffset             += ulCopyLen;
            ptBuffer->tMgmt.ulUsedDataBufferLen += ulCopyLen;
          }

          if(ptConnector->ulRxOffset < (ptHeader->ulLength + sizeof(*ptHeader)))
          {
              /* We have to wait for more data */
              fDone = 1;
          } else
          {
              /* We have a complete telegram */

              /* Check if we have data left */
              ulDataCnt -= ulCopyLen;
              pbData    = &pbData[ulCopyLen];

            /* We have all data */
            ptConnector->eScanState = HIL_CHECK_TELEGRAM;
          }
        }
        break;

        case HIL_CHECK_TELEGRAM:
        {
          HIL_TRANSPORT_HEADER*    ptHeader = &ptConnector->ptCurrentRxBuffer->tTransport;
          HIL_MARSHALLER_BUFFER_T* ptBuffer = ptConnector->ptCurrentRxBuffer;

          if( (ptHeader->ulLength > 0)    &&
              (ptHeader->usChecksum != 0) &&
              (ptHeader->usChecksum != CalculateCRC16(ptBuffer->abData,
                                                      ptConnector->tRxHeader.ulLength)) )
          {
            SendAcknowledge(ptMarshaller, ulConnector, &ptConnector->tRxHeader, HIL_TSTATE_CHECKSUM_ERROR);
          } else
          {
            /* We have a complete telegram */
            /* Check for Acknowledge */
            switch(ptHeader->usDataType)
            {
            case HIL_TRANSPORT_TYPE_ACKNOWLEDGE:
              break;

            case HIL_TRANSPORT_TYPE_QUERYSERVER:
            {
              if(NULL == ptBuffer)
                ptBuffer = HilMarshallerGetBuffer(pvMarshaller, eMARSHALLER_TX_BUFFER, ulConnector);

              if(NULL == ptBuffer)
              {
                SendAcknowledge(ptMarshaller, ulConnector, &ptConnector->tRxHeader, HIL_TSTATE_RESOURCE_ERROR);

              } else
              {
                PHIL_TRANSPORT_ADMIN_QUERYSERVER_DATA_T ptServerData = (PHIL_TRANSPORT_ADMIN_QUERYSERVER_DATA_T)ptBuffer->abData;
                uint32_t                                ulIdx;

                SendAcknowledge(ptMarshaller, ulConnector, &ptConnector->tRxHeader, HIL_TRANSPORT_STATE_OK);

                /* Fill in data */
                ptBuffer->tTransport = ptConnector->ptCurrentRxBuffer->tTransport;

                ptServerData->ulStructVersion    = 1;
                OS_Memcpy(ptServerData->szServerName, ptMarshaller->szServerName, sizeof(ptServerData->szServerName));
                ptServerData->ulVersionMajor     = MARSHALLER_VERSION_MAJOR;
                ptServerData->ulVersionMinor     = MARSHALLER_VERSION_MINOR;
                ptServerData->ulVersionBuild     = MARSHALLER_VERSION_BUILD;
                ptServerData->ulVersionRevision  = MARSHALLER_VERSION_REVISION;
#if defined(HIL_MARSHALLER_PERMANENT_CONNECTION)
                ptServerData->ulFeatures         = HIL_TRANSPORT_FEATURES_KEEPALIVE |
                                                   HIL_TRANSPORT_FEATURES_PERMANENT_CONNECTION;
#else
                ptServerData->ulFeatures         = HIL_TRANSPORT_FEATURES_KEEPALIVE;
#endif
                ptServerData->ulParallelServices = ptConnector->tConn.ulDataBufferCnt;
                ptServerData->ulBufferSize       = ptConnector->tConn.ulDataBufferSize;
                ptServerData->ulDatatypeCnt      = ptMarshaller->ulTransports;

                for(ulIdx = 0; ulIdx < ptMarshaller->ulTransports; ++ulIdx)
                  ptServerData->ausDataTypes[ulIdx] = ptMarshaller->ptTransports[ulIdx].usDataType;

                /* Add the Keep Alive transport type to the list. */
                ptServerData->ausDataTypes[ulIdx] = HIL_TRANSPORT_TYPE_KEEP_ALIVE;  /*lint !e661 : see declaration of ptServerData */
                ptServerData->ulDatatypeCnt++;

                /* Calculate the actual response data length. */
                ptBuffer->tMgmt.ulUsedDataBufferLen = (uint32_t)((uint8_t*) ptServerData->ausDataTypes - (uint8_t*) ptServerData
                                                        + (ptServerData->ulDatatypeCnt) * sizeof (ptServerData->ausDataTypes[0]));

                if(HIL_MARSHALLER_E_SUCCESS != HilMarshallerConnTxData(pvMarshaller,
                                                                       ulConnector,
                                                                       ptBuffer))
                {
                  HilMarshallerFreeBuffer(ptBuffer);
                  ptConnector->ptCurrentRxBuffer = NULL;
                } else
                {
                  /* Prevent buffer from being freed by ResetRxStateMachine() while not transmitted completely. */
                  ptConnector->ptCurrentRxBuffer = NULL;
                }
              }
            }
            break;

            /*TODO: Implement Administration commands (QUERY_DEVICE) */

            case HIL_TRANSPORT_TYPE_KEEP_ALIVE:
              {
                PHIL_TRANSPORT_KEEPALIVE_DATA_T ptKeepAlive = (PHIL_TRANSPORT_KEEPALIVE_DATA_T)ptConnector->ptCurrentRxBuffer->abData;
                unsigned char                   bState      = HIL_TRANSPORT_STATE_OK;
                bool                            fSendAnswer = false;

                if(ptHeader->ulLength != sizeof(*ptKeepAlive))
                {
                  /* Illegal length of keepalive packet */
                  bState = HIL_TSTATE_LENGTH_INCOMPLETE;

                } else if(0 == ptKeepAlive->ulComID)
                {
                  /* New Keepalive ID requested */
                  uint32_t ulNewId = OS_GetTickCount();

                  if(0 == ulNewId)
                    ++ptConnector->ulKeepaliveID;

                  if(ulNewId == ptConnector->ulKeepaliveID)
                  {
                    ptConnector->ulKeepaliveID = ~ulNewId;
                  } else
                  {
                    ptConnector->ulKeepaliveID = ulNewId;
                  }

                  ptKeepAlive->ulComID       = ptConnector->ulKeepaliveID;
                  fSendAnswer                = true;

                } else if(ptKeepAlive->ulComID != ptConnector->ulKeepaliveID)
                {
                  /* ComID does not match, so just return a negative Acknowledge */
                  bState = HIL_TSTATE_KEEP_ALIVE_ERROR;
                } else
                {
                  /* Everything is fine */
                  ptKeepAlive->ulComID       = ptConnector->ulKeepaliveID;
                  fSendAnswer                = true;
                  bState = HIL_TRANSPORT_STATE_OK;
                }

                SendAcknowledge(ptMarshaller, ulConnector, &ptConnector->tRxHeader, bState);

                if(!fSendAnswer)
                {
                  HilMarshallerFreeBuffer(ptConnector->ptCurrentRxBuffer);
                  ptConnector->ptCurrentRxBuffer = NULL;

                } else if(HIL_MARSHALLER_E_SUCCESS != HilMarshallerConnTxData(pvMarshaller,
                                                                              ulConnector,
                                                                              ptConnector->ptCurrentRxBuffer))
                {
                  HilMarshallerFreeBuffer(ptConnector->ptCurrentRxBuffer);
                  ptConnector->ptCurrentRxBuffer = NULL;
                } else
                {
                  /* Prevent buffer from being freed by ResetRxStateMachine() while not transmitted completely. */
                  ptConnector->ptCurrentRxBuffer = NULL;
                }
              }
              break;

            default:
              {
                TRANSPORT_LAYER_DATA_T* ptTransport = FindTransportLayer(ptMarshaller, ptHeader->usDataType);

                if(NULL == ptTransport)
                {
                  SendAcknowledge(ptMarshaller, ulConnector, &ptConnector->tRxHeader, HIL_TSTATE_DATA_TYPE_UNKNOWN);

                  HilMarshallerFreeBuffer(ptConnector->ptCurrentRxBuffer);
                  ptConnector->ptCurrentRxBuffer = NULL;

                } else
                {
                  int iLock;

                  SendAcknowledge(ptMarshaller, ulConnector, &ptConnector->tRxHeader, HIL_TRANSPORT_STATE_OK);

                  /* Enqueue this request into list, and let user handle it in it's own task
                     We need to set the current Rx Buffer to NULL, so that ResetRxStateMachine won't
                     free it. */
                  ptConnector->ptCurrentRxBuffer = NULL;

                  iLock = OS_Lock();
                  STAILQ_INSERT_TAIL(&ptMarshaller->tPendingRequests, ptBuffer, tList);
                  OS_Unlock(iLock);

                  ptMarshaller->pfnRequest(ptMarshaller, ptMarshaller->pvUser);
                }
              }
              break;
            }
          }

          /* Reset state machine */
          ResetRxStateMachine(ptConnector);

          /* Check if we have processed all incoming data */
          if( 0 == ulDataCnt)
          {
            /* Start with scan for cookie */
            fDone = 1;
          }
        }
        break;

        default:
          ;
        break;
      } /* end switch state */

    } while (0 == fDone);
  }

  return eRet;
}

/*****************************************************************************/
/*! Called by transport when new data should be send to line
*    \param pvMarshaller     Marshaller handle
*    \param ulConnector      Connector number
*    \param ptBuffer         Outgoing data
*    \return HIL_MARSHALLER_E_SUCCESS on success                             */
/*****************************************************************************/
uint32_t HilMarshallerConnTxData(void* pvMarshaller, uint32_t ulConnector, HIL_MARSHALLER_BUFFER_T* ptBuffer)
{
  HIL_MARSHALLER_DATA_T* ptMarshaller = (HIL_MARSHALLER_DATA_T*)pvMarshaller;
  CONNECTOR_DATA_T*      ptConn       = &ptMarshaller->atConnectors[ulConnector];

  ptBuffer->tTransport.ulLength = ptBuffer->tMgmt.ulUsedDataBufferLen;

  if(ptBuffer->tTransport.ulLength > 0)
  {
    ptBuffer->tTransport.usChecksum = CalculateCRC16(ptBuffer->abData,
                                                     ptBuffer->tTransport.ulLength);
  } else
  {
    ptBuffer->tTransport.usChecksum = 0;
  }

  return ptConn->tConn.pfnTransmit(ptBuffer, ptConn->tConn.pvUser);
}

/*****************************************************************************/
/*! Called by connector data has been sent
*    \param pvMarshaller     Marshaller handle
*    \param ulConnector      Connector number
*    \param ptBuffer         Outgoing data buffer
*    \return HIL_MARSHALLER_E_SUCCESS on success                             */
/*****************************************************************************/
uint32_t HilMarshallerConnTxComplete(void* pvMarshaller, uint32_t ulConnector, HIL_MARSHALLER_BUFFER_T* ptBuffer)
{
  HilMarshallerFreeBuffer(ptBuffer);

  return HIL_MARSHALLER_E_SUCCESS;
}

/*****************************************************************************/
/*! Called by transport layer, during initialization.
*    \param pvMarshaller     Marshaller handle
*    \param ptLayerData      Layer registration data
*    \return HIL_MARSHALLER_E_SUCCESS on success                             */
/*****************************************************************************/
uint32_t HilMarshallerRegisterTransport(void* pvMarshaller, const TRANSPORT_LAYER_DATA_T* ptLayerData)
{
  HIL_MARSHALLER_DATA_T* ptMarshaller = (HIL_MARSHALLER_DATA_T*)pvMarshaller;
  uint32_t eRet;

  if(0 == ptLayerData->usDataType)
  {
    eRet = HIL_MARSHALLER_E_INVALIDPARAMETER;

  } else if(NULL != FindTransportLayer(ptMarshaller, ptLayerData->usDataType))
  {
    /* Another Transport layer is already registered for this datatype (configuration error) */
    eRet = HIL_MARSHALLER_E_ALREADYREGISTERED;
  } else
  {
    uint32_t ulIdx;

    eRet = HIL_MARSHALLER_E_OUTOFRESOURCES;

    /* Search first free array element */
    for(ulIdx = 0; ulIdx < ptMarshaller->ulTransports; ++ulIdx)
    {
      TRANSPORT_LAYER_DATA_T* ptLayer = &ptMarshaller->ptTransports[ulIdx];

      if(0 == ptLayer->usDataType)
      {
        *ptLayer = *ptLayerData;
        eRet = HIL_MARSHALLER_E_SUCCESS;
        break;
      }
    }
  }

  return eRet;
}

/*****************************************************************************/
/*! Called by transport layer, during uninitialization.
*    \param pvMarshaller     Marshaller handle
*    \param usDataType       Datatype the layer was registered at           */
/*****************************************************************************/
void HilMarshallerUnregisterTransport(void* pvMarshaller, uint16_t usDataType)
{
  HIL_MARSHALLER_DATA_T*  ptMarshaller = (HIL_MARSHALLER_DATA_T*)pvMarshaller;
  TRANSPORT_LAYER_DATA_T* ptLayer      = FindTransportLayer(ptMarshaller, usDataType);

  if(ptLayer != NULL)
  {
    OS_Memset(ptLayer, 0, sizeof(*ptLayer));
  }
}

/*****************************************************************************/
/*! Cyclic timer event, which needs to be called by user for timeout management
*    \param pvMarshaller     Marshaller handle                              */
/*****************************************************************************/
void HilMarshallerTimer(void* pvMarshaller)
{
  uint32_t                ulIdx;
  HIL_MARSHALLER_DATA_T*  ptMarshaller = (HIL_MARSHALLER_DATA_T*)pvMarshaller;

  /* Check all transports for polling functions */
  for(ulIdx = 0; ulIdx < ptMarshaller->ulTransports; ++ulIdx)
  {
    TRANSPORT_LAYER_DATA_T* ptLayer = &ptMarshaller->ptTransports[ulIdx];

    if(ptLayer->pfnPoll)
      ptLayer->pfnPoll(ptLayer->pvUser);
  }

  /* Check all connectors for polling functions */
  for(ulIdx = 0; ulIdx < ptMarshaller->ulMaxConnectors; ++ulIdx)
  {
    CONNECTOR_DATA_T* ptConn = &ptMarshaller->atConnectors[ulIdx];

    /* Timeout management for all connector Rx state machines */
    /* Check if the RX state machine is active */
    if(ptConn->fMonitorTimeout)
    {
      /* Incremet the elapsed time */
      ptConn->ulElapsedTime += 10;

      if( ptConn->tConn.ulTimeout < ptConn->ulElapsedTime)
      {
        ResetRxStateMachine( ptConn);
      }
    }

    if(ptConn->fInUse && ptConn->tConn.pfnPoll)
      ptConn->tConn.pfnPoll(ptConn->tConn.pvUser);
  }
}

/*****************************************************************************/
/*! Main marshaller module. This must be called by user, every time it receives
*   the pfnRequest callback.
*    \param pvMarshaller     Marshaller handle
*    \return HIL_MARSHALLER_E_SUCCESS on success
*    \and HIL_MARSHALLER_E_FAIL if no message retrieved from the pending requests list  */
/*****************************************************************************/
uint32_t HilMarshallerMain(void* pvMarshaller)
{
  uint32_t                 eRet            = HIL_MARSHALLER_E_SUCCESS;
  HIL_MARSHALLER_DATA_T*   ptMarshaller    = (HIL_MARSHALLER_DATA_T*) pvMarshaller;
  int                      iLock           = 0;
  HIL_MARSHALLER_BUFFER_T* ptBuffer        = NULL;
  TRANSPORT_LAYER_DATA_T*  ptTransport     = NULL;
  CONNECTOR_DATA_T*        ptConn          = NULL;

  if (ptMarshaller == NULL)
    eRet = HIL_MARSHALLER_E_FAIL;
  else
  {
    iLock    = OS_Lock();
    ptBuffer = STAILQ_FIRST(&ptMarshaller->tPendingRequests);
    OS_Unlock(iLock);
    if (ptBuffer == NULL)
      eRet = HIL_MARSHALLER_E_FAIL;
    else
    {
      ptTransport = FindTransportLayer(ptMarshaller, ptBuffer->tTransport.usDataType);
      ptConn      = &ptMarshaller->atConnectors[ptBuffer->tMgmt.ulConnectorIdx];
      if (ptTransport == NULL || ptConn == NULL)
        eRet = HIL_MARSHALLER_E_FAIL;
      else
      {
        iLock = OS_Lock();
        STAILQ_REMOVE(&ptMarshaller->tPendingRequests, ptBuffer, HIL_MARSHALLER_BUFFER_Ttag, tList);
        OS_Unlock(iLock);

        ptTransport->pfnHandler(ptMarshaller, ptBuffer, ptTransport->pvUser);
      }
    }
  }
  return eRet;
}

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
                              uint32_t ulConnectorID)
{
  uint32_t               eRet         = HIL_MARSHALLER_E_SUCCESS;
  HIL_MARSHALLER_DATA_T* ptMarshaller = (HIL_MARSHALLER_DATA_T*)pvMarshaller;
  uint32_t               ulIndex;
  uint32_t               ulMinConnectorID = ulConnectorID;
  uint32_t               ulMaxConnectorID = ulConnectorID + 1;

  /* check the scope of the command (single connector / all connectors) */
  if (ulConnectorID == HIL_MARSHALLER_CONNECTORS_ALL)
  { /* change mode of all connectors */
    ulMinConnectorID = 0;
    ulMaxConnectorID = ptMarshaller->ulMaxConnectors;
  }
  else
  { /* check the given connector index */
    if (ulConnectorID > ptMarshaller->ulMaxConnectors)
      eRet = HIL_MARSHALLER_E_OUTOFRESOURCES;
  }
  if (eRet == HIL_MARSHALLER_E_SUCCESS)
  {
    switch (ulMode)
    {
      case HIL_MARSHALLER_MODE_ENABLED:
      { /* set mode for a single connector or for all connectors */
        for (ulIndex = ulMinConnectorID; ulIndex < ulMaxConnectorID; ulIndex++)
          ptMarshaller->atConnectors[ulIndex].ulMode = ulMode;
        break;
      }
      case HIL_MARSHALLER_MODE_DISABLED:
      { /* set mode for a single connector or for all connectors */
        for (ulIndex = ulMinConnectorID; ulIndex < ulMaxConnectorID; ulIndex++)
          ptMarshaller->atConnectors[ulIndex].ulMode = ulMode;
        break;
      }
      default:
      { /* invalid mode parameter */
        eRet = HIL_MARSHALLER_E_INVALIDPARAMETER;
      }
    }
  }
  return (eRet);
}


/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/
