/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20230403-00/includes/Hil_GenericCommunicationInterface.h $: *//*!

  \file Hil_GenericCommunicationInterface.h

  Hilscher's Generic Communication Interface (GCI) Definition.

**************************************************************************************/
#ifndef HIL_GENERICCOMMUNICATIONINTERFACE_H_
#define HIL_GENERICCOMMUNICATIONINTERFACE_H_

/*! Version of the Generic Communication Interface. */
#define HIL_GCI_VERSION_21   (21)

#include <stdint.h>
#include "Hil_Compiler.h"
#include "Hil_Packet.h"


/*! Handle of the GCI-Component.
 * This is the GCI-Component handle which must be passed as first parameter into the GCI-Component's functions.
 *
 * \note The actual structure definition is GCI-Component specific. It has to be defined in a private header,
 *       and is not public to the GCI-Application or other GCI-Components.
 */
typedef struct HIL_GCI_COMP_Ttag* HIL_GCI_COMP_H;

/*! Handle of the GCI-Application.
 * The handle is instantiated by the GCI-Application and must be passed as first parameter into the GCI-Application's functions.
 *
 * \note The actual structure definition is GCI-Application specific. It has to be defined in a private header,
 *       and is not public to the GCI-Components or other GCI-Components.
 */
typedef struct HIL_GCI_APP_Ttag* HIL_GCI_APP_H;



/*! \defgroup HIL_GCI_AcyclicServices Acyclic Services
 * The Generic Communication Interface (GCI) defines a acyclic bidirectional communication interface
 * between a GCI-Application and a GCI-Component.
 *
 * The GCI-Component and GCI-Application interact by means of services. Each of this services has a defined data structure
 * which is passed along with the service.
 *
 * Services are symmetrically defined by the GCI-Application and the GCI-Component:
 *  - The GCI-Component implements services which are called "requests" (*_REQ) to be used by the GCI-Application and for
 *    each of them, the GCI-Application correspondingly implements a service which is called "confirmation" (*_CNF).
 *    The confirmation is the acknowledgment of the request.
 *
 *  - In opposite direction, the GCI-Application implements services which are called "indications" (*_IND) to be used
 *    by the GCI-Component and for each of them, the GCI-Component corresponding implements a service which is called
 *    "response" (*_RES). The response is the acknowledgment of the indication.
 *
 * Each service is assigned an unique number called the command code.
 *
 * Per definition, all service initiated by the GCI-Application are named requests and all services initiated by the
 * GCI-Component are named indications.
 *
 * Based on this interface, the GCI allows to define such pairs of services to implement interaction in a call and
 * reply fashion for, e.g., the following purposes:
 *    - Configuration
 *    - Exchange of acyclic data
 *    - Diagnostics
 *
 *\{*/

/*! Handle of an acyclic service.
 * The handle is instantiated by the GCI-Component for each service request towards the GCI-Application and uniquely
 * identifies the transaction between the GCI-Application and the GCI-Component related to this service.
 *
 * \note The actual structure definition is GCI-Component specific. It has to be defined in a private header,
 *       and is not public to the GCI-Application or other GCI-Components.
 */
typedef struct HIL_GCI_COMP_SERVICE_Ttag*    HIL_GCI_COMP_SERVICE_H;

/*! Announce a service from the GCI-Application towards the GCI-Component.
 * This function passes a new request (*_REQ), or confirms an indication (*_RES), at the GCI-Component.
 * On successful return, the packet header as well as the data have been consumed by the GCI-Component and
 * can be released by the GCI-Application.
 *
 * \note The function shall be called from one context only (IRQ context not allowed).
 * \note The function shall NOT wait for other events (e.g. on network),
 *       but may call task synchronization functions (e.g. Mutex, Triple buffer exchange)
 * \note The \ref HIL_GCI_APP_SERVICE_AVAILABLE_FN from the GCI-Application may be called before the function returns.
 * \note The number of supported parallel services which can be processed by the GCI-Component is implementation specific.
 *       If the number of parallel data handling is exceeded, the function returns with an error code.
 *       At least 8 parallel service requests are recommended.
 * \note This interface does not support fragmentation of the individual service.
 *       The GCI-Application has to ensure to collect all data before this function is called.
 *
 * \param hComponent      [in] Handle of the GCI-Component.
 * \param ptPacketHeader  [in] Packet header information for the passed data in pabPacketData.
 *                             \note This pointer shall point to a DWORD aligned address.
 * \param pabPacketData   [in] Pointer to acyclic data buffer with length ptPacketHeader->ulLen.
 *                             \note The content is only valid until the function returns.
 *                             \note This pointer shall point to a DWORD aligned address.
 *
 * \return SUCCESS_HIL_OK in case the GCI-Component has accepted the service and consumed all data,
 *         otherwise an error code is returned.
 *         The error returned at this point is an interface issue, the service has not be accepted
 *         by GCI-Component. In case the GCI-Component is busy (e.g. ERR_HIL_COMPONENT_BUSY was returned)
 *         the GCI-Application shall retry the service later in time (recommend retry time is 5ms).
 *
 *         \note The error code shall only be used by the GCI-Component in case the GCI-Component is not able to accept the service,
 *               e.g. no more resources left to accept service or invalid function arguments.
 *
 *         \note GCI-Component implementation note. The error code shall not be a result from the processing of the service.
 *               The content of the packet header (ptPacketHeader) and packet data (pabPacketData) shall not be considered
 *               as error code, e.g. invalid packet length, invalid parameter or unknown command. In this case, the service
 *               shall be accepted anyway. In case of a request (*_REQ) the error code shall be passed by the confirmation
 *               (*_CNF) packet status.
 */
typedef uint32_t (*HIL_GCI_COMP_SERVICE_AVAILABLE_FN)
(
  HIL_GCI_COMP_H              hComponent,
  const HIL_PACKET_HEADER_T*  ptPacketHeader,
  const uint8_t*              pabPacketData
);

/*! Announce a service from the GCI-Component towards the GCI-Application.
 * The service announces a new indication (*_IND) or confirmation (*_CNF) to a previous request, at the GCI-Application.
 * On successful return (or during function execution), the GCI-Application will call the \ref HIL_GCI_COMP_SERVICE_READ_PACKET_DATA_FN
 * function to consume the announced service data.
 *
 * \note This callback can be called from any context (except IRQ).
 * \note The service data read function callback (\ref HIL_GCI_COMP_SERVICE_READ_PACKET_DATA_FN) may be called before this function returns.
 * \note The GCI-Component has to ensure to collect all announced data before calling this function.
 * \note The GCI-Component has to hold the acyclic data until the GCI-Application has consumed all data by means of one or multiple calls
 *       to \ref HIL_GCI_COMP_SERVICE_READ_PACKET_DATA_FN.
 *
 * \param hApplication    [in] Handle of the GCI-Application.
 * \param hService        [in] Service handle to uniquely identify the transaction of service data from the GCI-Component to the GCI-Application
 *                             in subsequent calls to the \ref HIL_GCI_COMP_SERVICE_READ_PACKET_DATA_FN function.
 * \param ptPacketHeader  [in] Packet header information for the announced data.
 *
 * \return SUCCESS_HIL_OK in case GCI-Application accepts the service, otherwise an error code is returned.
 *         \note The error code shall only be used by the GCI-Application in case the GCI-Application is not able to accept the service,
 *               e.g. no more resources left to accept service, or ptPacketHeader is NULL. The error code shall not be a result from
 *               the processing of the service.
 *         \note The content of the packet header (ptPacketHeader) shall not be considered as error code, e.g. invalid packet length
 *               or unknown command. In this case, the service shall be accepted anyway. In case of an indication (*_IND) the error code
 *               shall be passed by the response (*_RES) packet status.
 *         \note In error case the GCI-Application will not read out the data. The GCI-Component may retry to announce the service, where
 *               the retry interval and the number of retries is implementation-specific. If the GCI-Component forfeits the retry, it will
 *               call GCI-Application's error callback and enter faulty state.
 */
typedef uint32_t (*HIL_GCI_APP_SERVICE_AVAILABLE_FN)
(
  HIL_GCI_APP_H                 hApplication,
  HIL_GCI_COMP_SERVICE_H        hService,
  const HIL_PACKET_HEADER_T*    ptPacketHeader
);

/*! The minimum read buffer size in bytes that shall be used during read of the service data from the GCI-Component. */
#define HIL_GCI_SERVICE_READ_PACKET_DATA_BUFFER_SIZE_MIN       (64)

/*! Read a portion of announced service data from the GCI-Component.
 * The function will read from the GCI-Component a subset or all of the data previously announced by the GCI-Component via
 * \ref HIL_GCI_APP_SERVICE_AVAILABLE_FN. Typically, The transaction of service data is completed when all announced data has been
 * read from the GCI-Component.
 *
 * If the GCI-Application is not interested in the announced service data, or there is no data available, the transaction has to be
 * completed by calling this function once with a NULL Pointer for the pvBuffer parameter.
 *
 * Announced data can be read from the GCI-Component in a arbitrary amount of calls. The read buffer size of each call has to be at
 * least 64 bytes, unless the remaining size to read is lower on the very last call. The function will never return with less data
 * than requested, unless all data has been read.
 *
 * Pending transactions of announced but not yet read data may occupy resources in the GCI-Component.
 * When the transaction is completed because all data has been read or the GCI-Application has called with a NULL buffer, the service
 * data handle is invalid, and must not be used any further.
 *
 * Three conditions are defined to indicate completion of a transaction of announced service data:
 *  - The function returns with 0
 *  - The sum of the returned sizes of all consecutive call for a transaction reaches the announced data size ptPacketHeader.ulLen.
 *  - The function is called with NULL pointer for the parameter pvBuffer, regardless if nothing or only a part was read before
 *    (e.g. first 64bytes have been read, then the service shall be quit). This allows the GCI-Application to abort a transaction.
 *
 * \note The function shall NOT wait for other events (e.g. on network), but may call task synchronization functions (e.g. Mutex, Triple buffer exchange)
 * \note The GCI-Application has to provide a proper data buffer to store the announced data at once
 * \note The GCI-Component will copy the raw packet data to the provided buffer (without the packet header) and keeps track of the current offset
 * \note The function has an iterative property. Thus, the GCI-Component has to hold the data until completely read by the GCI-Application.
 *
 * \param hComponent              [in]   Handle of the GCI-Component.
 * \param hService                [in]   Service data handle as previously announced to uniquely identify the service data transaction.
 * \param pvPacketDataBuffer      [out]  Destination buffer (4 byte aligned) for the GCI-Component to write the requested data portion into
 *                                       \note Pointer can also be NULL, in this case the GCI-Component can release the data/abort the transaction
 * \param ulReadSize              [in]   Size of the given destination buffer, number of bytes to read. At least 64 bytes have to be requested with
 *                                       each call except on the very last call, the remaining data size can be lower.
 *
 * \return Size in bytes that have been read. The size will always be equal to ulReadSize, unless all data has been read.
 *         Example: 150 Bytes have to be read and the read buffer size is ulReadSize = 128 bytes.
 *                  1. iteration: \ref HIL_GCI_COMP_SERVICE_READ_PACKET_DATA_FN call with ulReadSize = 128 bytes
 *                                -> function returns with 128 bytes
 *                  2. iteration: \ref HIL_GCI_COMP_SERVICE_READ_PACKET_DATA_FN call with ulReadSize = 128 bytes
 *                                -> function returns with 22 bytes
 */
typedef uint32_t (*HIL_GCI_COMP_SERVICE_READ_PACKET_DATA_FN)
(
  HIL_GCI_COMP_H         hComponent,
  HIL_GCI_COMP_SERVICE_H hService,
  void*                  pvPacketDataBuffer,
  uint32_t               ulReadSize
);

/*! This structure defines a command range for acyclic services.
 * \note This structure is used by the GCI-Component during initialization phase to provide
 * which command codes it will accept (see \ref HIL_GCI_COMP_INTERFACE_INITIALIZE_FN).
 */
typedef struct HIL_GCI_COMP_SERVICE_COMMAND_RANGE_Ttag
{
  /*! Starting command number within the command range. */
  uint32_t ulCommandDomain;

  /*! The mask represents the command number filter to determine the membership within a domain.
   * Example: The command range shall be [0x2F30, 0x2F33]
   * The range is determined by the two lower bits. Neglecting the two lower bits is done by Mask = 0xFFFC.
   * After applying the filter (command number & mask), the domain is determined to be Domain = 0x2F30.
   * */
  uint32_t ulCommandMask;

  /*! This flag indicates to the GCI-Application if the services within the command range can be announced
   * in fragments to the GCI-Component by means of one or multiple calls to \ref HIL_GCI_COMP_SERVICE_AVAILABLE_FN.
   * This is enabled by the GCI-Component if it can not be ensured that the packet data can be provided in
   * one buffer, especially in environments with limited memory space.
   * values: true in case service can be forwarded in fragments to the GCI-Component, otherwise false.
   *
   * \note The mechanism to provide a service in fragments is GCI-Component specific.
   * \note This flag is set to true in rare cases. */
  uint8_t bDisableFragmentation;

} HIL_GCI_COMP_SERVICE_COMMAND_RANGE_T;

/*! The acyclic service interface provided by the GCI-Component for the GCI-Application.
 *  This is part of the GCI-Component Interface structure.
 */
typedef struct HIL_GCI_COMP_SERVICE_INTERFACE_Ttag
{
  /*! Service available callback. */
  HIL_GCI_COMP_SERVICE_AVAILABLE_FN         fnAvailable;

  /*! Service read packet data callback. */
  HIL_GCI_COMP_SERVICE_READ_PACKET_DATA_FN  fnRead;

  /*! GCI service command ranges that the GCI-Component is interested on.
   *  If (ulCommand & ulCommandMask) == ulCommandDomain is valid, the acyclic service shall be handled by this interface. */
  const HIL_GCI_COMP_SERVICE_COMMAND_RANGE_T* patCommandRanges;
  /*! The total amount of different command ranges stored in patCommandRanges. */
  uint32_t                                    ulNumOfCommandRanges;

  /*! The maximum unfragmented service data length in bytes that the GCI-Component can receive from GCI-Application. */
  uint32_t ulMaxUnfragDataSize;

} HIL_GCI_COMP_SERVICE_INTERFACE_T;

/*! The acyclic service interface provided by the GCI-Application for the GCI-Component.
 *  This is part of the GCI-Application interface structure.
 */
typedef struct HIL_GCI_APP_SERVICE_INTERFACE_Ttag
{
  /*! Acyclic service available callback. */
  HIL_GCI_APP_SERVICE_AVAILABLE_FN fnAvailable;

} HIL_GCI_APP_SERVICE_INTERFACE_T;

/*!  \} */







/*! \defgroup HIL_GCI_CyclicData Cyclic Data
 *  The GCI provides an interface to exchange cyclic data.
 *  \{ */

/*! Update cyclic provider data.
 * The GCI-Application calls this function when provider data was updated by GCI-Application.
 *
 * After calling this function the provider data (pabProviderData given in HIL_GCI_APPLICATION_INTERFACE_T)
 * is eligible to be accessed and processed by the GCI-Component. When the GCI-Component has finished access, it will call
 * \ref HIL_GCI_APP_CYCLIC_PROVIDER_DATA_UPDATE_DONE_FN to give control over the provider data area back to the GCI-Application.
 * Accessing the provider data from each side when not in control leads to undefined behavior.
 *
 * \note The function needs to be implemented by the GCI-Component and is called by the GCI-Application.
 *
 * \note This callback can be called from any context (including IRQ). Thus, the GCI-Component takes the responsibility
 *       to enforce a context switch if necessary. It is not allowed to call blocking functions within this callback.
 *
 * \param hComponent [in] Handle of the GCI-Component.
 *
 * \return SUCCESS_HIL_OK in case GCI-Component accepts the update request, otherwise an error code is returned.
 *         \note  If accepted, the GCI-Component calls \ref HIL_GCI_APP_CYCLIC_PROVIDER_DATA_UPDATE_DONE_FN as soon as provider
 *                data has been consumed. The GCI-Application will not access provider data in the meantime.
*/
typedef uint32_t (*HIL_GCI_COMP_CYCLIC_PROVIDER_DATA_UPDATE_REQUEST_FN)
(
  HIL_GCI_COMP_H  hComponent
);

/*! Update cyclic provider data done.
 * The GCI-Component calls this function after provider data was processed by the GCI-Component to pass back
 * control over the provider data area back to the GCI-Application. Subsequently, the GCI-Application is allowed to
 * access the provider data again, i.e. to do another update and, in turn, pass it to the GCI-Component by means of
 * \ref HIL_GCI_COMP_CYCLIC_PROVIDER_DATA_UPDATE_REQUEST_FN.
 *
 * \note This function has to be implemented by the GCI-Application and is called by the GCI-Component.
 *
 * \param hApplication [in] Handle of the GCI-Application.
 *
 * \return This function has no return code
*/
typedef void (*HIL_GCI_APP_CYCLIC_PROVIDER_DATA_UPDATE_DONE_FN)
(
  HIL_GCI_APP_H hApplication
);

/*! Update cyclic consumer data.
 * The GCI-Application calls this function when the consumer data shall be updated by the GCI-Component.
 *
 * After calling this function the consumer data (pabConsumerData given in HIL_GCI_APPLICATION_INTERFACE_T)
 * is eligible to be accessed and processed by the GCI-Component. When the GCI-Component has finished access, it will call
 * \ref HIL_GCI_APP_CYCLIC_CONSUMER_DATA_UPDATE_DONE_FN to give control over the consumer data area back to the GCI-Application.
 * Accessing the consumer data from each side when not in control leads to undefined behavior.
 *
 * \note The function needs to be implemented by the GCI-Component and is called by the GCI-Application.
 *
 * \note This callback can be called from any context (including IRQ).
 *       Thus, the GCI-Component takes the responsibility to enforce a context switch in case necessary.
 *
 * \param hComponent  [in] Handle of the GCI-Component.
 *
 * \return SUCCESS_HIL_OK in case GCI-Component accepts the update request, otherwise an error code is returned.
 *         \note  If accepted, the GCI-Component calls \ref HIL_GCI_APP_CYCLIC_CONSUMER_DATA_UPDATE_DONE_FN as soon as consumer
 *                data has been updated. The GCI-Application will not access consumer data in the meantime.
*/
typedef uint32_t (*HIL_GCI_COMP_CYCLIC_CONSUMER_DATA_UPDATE_REQUEST_FN)
(
  HIL_GCI_COMP_H hComponent
);

/*! Update cyclic consumer data done.
 * The GCI-Component calls this function after consumer data was updated by the GCI-Component to pass back
 * control over the consumer data area back to the GCI-Application. Subsequently, the GCI-Application is allowed to
 * access the consumer data again, i.e. to read it and pass control back to the GCI-Component by means of
 * \ref HIL_GCI_COMP_CYCLIC_CONSUMER_DATA_UPDATE_REQUEST_FN.
 *
 * \note This function has to be implemented by the GCI-Application and is called by the GCI-Component.
 *
 * \param hApplication [in] Handle of the GCI-Application.
 *
 * \return This function has no return code
*/
typedef void (*HIL_GCI_APP_CYCLIC_CONSUMER_DATA_UPDATE_DONE_FN)
(
  HIL_GCI_APP_H hApplication
);

/*! Update provider and consumer data size.
 * The GCI-Component calls this function when data size of provider and/or consumer has changed.
 *
 * \param hComponent     [in] Handle of the GCI-Application.
 * \param ulProviderSize [in] Provider size in bytes.
 * \param ulConsumerSize [in] Consumer size in bytes.
 *
 * \return SUCCESS_HIL_OK in case the GCI-Application accepts data sizes, otherwise at least one data size is not accepted.
*/
typedef uint32_t (*HIL_GCI_APP_CONSUMER_PROVIDER_DATA_SIZE_UPDATE_FN)
(
  HIL_GCI_APP_H   hApplication,
  uint32_t        ulProviderSize,
  uint32_t        ulConsumerSize
);

/*! The cyclic service interface provided by the GCI-Application for the GCI-Component.
 *  This is part of the GCI-Application interface structure.
 */
typedef struct HIL_GCI_APP_CYCLIC_INTERFACE_Ttag
{
  /*! Cyclic consumer data update done callback.
    \note The GCI-Application will be informed about finished consumer data update. */
  HIL_GCI_APP_CYCLIC_CONSUMER_DATA_UPDATE_DONE_FN fnConsumerDataUpdateDone;

  /*! Pointer to consumer data buffer.
   * \note  The GCI-Application has to ensure to not change the consumer buffer.
   *        Otherwise inconsistent/invalid data might be observed.
   * \note  This pointer shall point to a DWORD aligned address. */
  uint8_t*  pabConsumerData;
  /*! Consumer data maximum size in bytes. */
  uint32_t  ulConsumerDataMaxSize;
  /*! Pointer to consumer data handshake error counter.
      In case not NULL, the GCI-Component has to update the value synchronous to IO exchange.
      The value is incremented by one for each I/O cycle in which update of consumer data
      by the GCI-Component took place but the GCI-Application failed to consume the data.
      Saturates at value 255. */
  uint8_t*  pbConsumerDataErrorCnt;

  /*! Cyclic provider data update done callback.
    \note The GCI-Application will be informed about finished provider data update. */
  HIL_GCI_APP_CYCLIC_PROVIDER_DATA_UPDATE_DONE_FN fnProviderDataUpdateDone;

  /*! Pointer to provider data buffer.
   * \note  This pointer shall point to a DWORD aligned address.*/
  uint8_t*  pabProviderData;
  /*! Provider data maximum size in bytes. */
  uint32_t  ulProviderDataMaxSize;
  /*! Pointer to provider data handshake error counter.
      In case not NULL, the GCI-Component updates the value synchronous to IO exchange.
      The value is incremented by one for each I/O cycle in which no update of provider data
      by the GCI-Application took place but the GCI-Component processed the data.
      Saturates at value 255. */
  uint8_t*  pbProviderDataErrorCnt;

  /*! Update provider and consumer data size callback.
    \note The GCI-Application will be informed about changed consumer/provider data size. */
  HIL_GCI_APP_CONSUMER_PROVIDER_DATA_SIZE_UPDATE_FN fnConsumerProviderUpdateDataSize;

} HIL_GCI_APP_CYCLIC_INTERFACE_T;

/*! The cyclic service interface provided by the GCI-Component for the GCI-Application.
 *  This is part of the GCI-Component interface structure.
 */
typedef struct HIL_GCI_COMP_CYCLIC_INTERFACE_Ttag
{
  /*! Update cyclic consumer data callback. */
  HIL_GCI_COMP_CYCLIC_CONSUMER_DATA_UPDATE_REQUEST_FN fnConsumerDataUpdateRequest;

  /*! Update cyclic provider data callback. */
  HIL_GCI_COMP_CYCLIC_PROVIDER_DATA_UPDATE_REQUEST_FN fnProviderDataUpdateRequest;

} HIL_GCI_COMP_CYCLIC_INTERFACE_T;

/*!  \} */







/*! \defgroup HIL_GCI_Sync Synchronization
 *   The GCI provides a unique interface to synchronize both the GCI-Application and GCI-Component with each other.
 *  \{ */

/*! Request synchronization between GCI-Component and GCI-Application.
 * Request synchronization with the GCI-Component. The GCI-Component will call \ref HIL_GCI_APP_SYNC_EVENT_DONE_FN
 * to synchronize the GCI-Application.
 *
 * The specific semantics of the synchronization function is specific to the GCI-Component and is configurable
 * with the dedicated services HIL_GCI_SET_TRIGGER_TYPE_REQ/CNF.
 *
 * Based on this functions, two basic mechanisms are implemented:
 * 1. To synchronize the GCI-Application to an event to occur in the GCI-Component (e.g. a new I/O frame is received)
 *      In this case the GCI-Application has to call this function to enter the wait phase for the expected event.
 *      The GCI-Component calls back into \ref HIL_GCI_APP_SYNC_EVENT_DONE_FN synchronous to the event. The GCI-Application then may
 *      call this function again to wait for the next event. In overload scenarios, where the GCI-Application or GCI-Component
 *      fail to call in time, synchronization events may be missed.
 *
 * 2. To synchronize the GCI-Component to an event to occur in the GCI-Application (e.g. Send I/O frame at an
 *    application-controlled point in time.
 *      In this case the GCI-Application has to call this function synchronous to the event.
 *      The GCI-Component will process the event in a best-effort manner and call back into \ref HIL_GCI_APP_SYNC_EVENT_DONE_FN
 *      after completion. On the next event, the GCI-Application will call this function again.
 *
 * \note \ref HIL_GCI_COMP_SYNC_EVENT_REQUEST_FN and \ref HIL_GCI_APP_SYNC_EVENT_DONE_FN must always be called in a well-formed
 *       successive fashion.
 * \note The function is implemented by the GCI-Component and is called by the GCI-Application.
 *
 * \param hComponent [in] Handle of the GCI-Component.
 *
 * \return SUCCESS_HIL_OK in case GCI-Component accepts the request, otherwise an error code is returned.
 *         \note If accepted, the GCI-Component calls \ref HIL_GCI_APP_SYNC_EVENT_DONE_FN after event has been processed.
*/
typedef uint32_t (*HIL_GCI_COMP_SYNC_EVENT_REQUEST_FN)
(
  HIL_GCI_COMP_H hComponent
);

/*! Synchronization between GCI-Component and GCI-Application completed.
 *
 * This function is called by the GCI-Component to complete the GCI-Application-requested synchronization.
 *
 * Depending on the particular GCI-Component and its configuration, this function is called to synchronize the GCI-Application
 * to an event which occurred in the GCI-Component or, alternatively, to just to report completion of an synchronous event
 * which was triggered by the GCI-Application by means of \ref HIL_GCI_COMP_SYNC_EVENT_REQUEST_FN.
 *
 * \note This function is implemented by the GCI-Application and is called by the GCI-Component.
 *
 * \param hApplication [in] Handle of the GCI-Application.
*/
typedef void (*HIL_GCI_APP_SYNC_EVENT_DONE_FN)
(
  HIL_GCI_APP_H hApplication
);

/*! The synchronization service interface provided by the GCI-Component for the GCI-Application.
 *  This is part of the GCI-Component interface structure.
 */
typedef struct HIL_GCI_COMP_SYNC_INTERFACE_Ttag
{
  /*! Process SYNC event callback.  */
  HIL_GCI_COMP_SYNC_EVENT_REQUEST_FN fnSyncRequest;

} HIL_GCI_COMP_SYNC_INTERFACE_T;

/*! The synchronization service interface provided by the GCI-Application for the GCI-Component.
 *  This is part of the GCI-Application interface structure.
 */
typedef struct HIL_GCI_APP_SYNC_INTERFACE_Ttag
{
  /*! SYNC event process done callback. */
  HIL_GCI_APP_SYNC_EVENT_DONE_FN fnSyncDone;

  /*! Pointer to synchronization handshake error counter.
      In case not NULL, the GCI-Component updates the value synchronous to the Synchronization exchange
      via \ref HIL_GCI_APP_SYNC_EVENT_DONE_FN. The value is incremented by one for each Synchronization cycle in
      which no handshaking with the GCI-Application took place. Saturates at value 255. */
  uint8_t*  pbSyncEventErrorCnt;

} HIL_GCI_APP_SYNC_INTERFACE_T;

/*!  \} */







/*! \defgroup HIL_GCI_Error Error
 * GCI provides a bidirectional interface for error report.
 *  \{ */

/*! Report an faulty GCI-Component.
 * The GCI-Component calls this function in case it is no longer possible to continue normal operation
 * process, e.g. a critical problem is detected, specifically, critical runtime failures and
 * invalid use of the GCI interface. The GCI-Application can no longer expect proper functionality
 * of the GCI-Component, i.e. the GCI-Application shall switch to a safe state.
 *
 * \note The function needs to be implemented by the GCI-Application and is called by the GCI-Component.
 *
 * \param hApplication   [in] Handle of the GCI-Application.
 * \param ulErrorCode    [in] The error code indicating the problem occurred.
*/
typedef void (*HIL_GCI_APP_COMPONENT_ERROR_OCCURED_FN)
(
  HIL_GCI_APP_H         hApplication,
  uint32_t              ulErrorCode
);

/*! The error service interface provided by the GCI-Application for the GCI-Component.
 *  This is part of the GCI-Application interface structure.
 */
typedef struct HIL_GCI_APP_ERROR_INTERFACE_Ttag
{
  /*! Error occurred callback. */
  HIL_GCI_APP_COMPONENT_ERROR_OCCURED_FN fnErrorOccured;

} HIL_GCI_APP_ERROR_INTERFACE_T;

/*! Report a GCI-Application error to the GCI-Component.
 *
 * Using this callback, the GCI-Application requests the GCI-Component to transit into failure state
 * and stop communication. In this state any start of network commands shall be rejected until
 * the GCI-Component is reinitialized (HIL_CHANNEL_INIT_REQ).
 *
 * The GCI-Component will send a HIL_GCI_SET_COMMUNICATION_STATUS_IND to acknowledge the new state:
 *  ulCommunicationState = HIL_COMM_STATE_STOP
 *  ulError              = ulErrorCode
 *  ulErrorCounter       = Total number of errors detected since last reset.
 *  bCommunicating       = 0 (false)
 *
 *
 * \param hComponent     [in] Handle of the GCI-Component.
 * \param ulErrorCode    [in] The error code indicating the problem occurred, e.g. ERR_HIL_WATCHDOG_TIMEOUT
 */
typedef void (*HIL_GCI_COMP_APPLICATION_ERROR_OCCURED_FN)
(
  HIL_GCI_COMP_H hComponent,
  uint32_t       ulErrorCode
);

/*! The error service interface provided by the GCI-Component for the GCI-Application.
 *  This is part of the GCI-Component interface structure.
 */
typedef struct HIL_GCI_COMP_ERROR_INTERFACE_Ttag
{
  /*! Error occurred callback. */
  HIL_GCI_COMP_APPLICATION_ERROR_OCCURED_FN   fnErrorOccured;

} HIL_GCI_COMP_ERROR_INTERFACE_T;

/*!  \} */







/*! \defgroup HIL_GCI_Startup Startup
 *   The GCI provides a unique interface to initialize and start the GCI-Component.
 *  \{ */

/*! The GCI-Component information interface provided by the GCI-Component for the GCI-Application.
 *  This is part of the GCI-Component interface structure.
 */
typedef struct HIL_GCI_COMP_INFO_Ttag
{
  /*! Unique GCI-Component identifier HIL_COMPONENT_ID_*. */
  uint32_t ulComponentId;
  /*! Total amount of required remanent data in bytes. In case the remanent data size is zero,
   * the GCI-Component does not need remanent data at all.
   * \note The remanent data size shall be exact the size used during HIL_STORE_REMANENT_DATA_IND. */
  uint32_t ulRemanentDataSize;
  /*! Major version */
  uint16_t usVersionMajor;
  /*! Minor version */
  uint16_t usVersionMinor;
  /*! Build version */
  uint16_t usVersionBuild;
  /*! Revision version */
  uint16_t usVersionRevision;
} HIL_GCI_COMP_INFO_T;

/*! Start the GCI Interface of a specific GCI-Component.
 *
 * This function is called by the GCI-Application to signal the GCI-Component that the interface shall be started.
 * At that point in time both the GCI-Application and the GCI-Component are allowed to communicate with each other
 * using the previously exchanged interfaces (see \ref HIL_GCI_COMP_INTERFACE_INITIALIZE_FN).

 * \param hComponent [in] Handle of the GCI-Component. Obtained with GCI-Component's interface,
 *                        see \ref HIL_GCI_COMP_INTERFACE_INITIALIZE_FN.
 */
typedef void (*HIL_GCI_COMP_INTERFACE_START_FN)
(
  HIL_GCI_COMP_H hComponent
);

/*!
 *  GCI-Application interface
 *  This interface structure is filled by the GCI-Application and provided to the GCI-Component.
 */
typedef struct HIL_GCI_APP_INTERFACE_Ttag
{
  /*! GCI version number that is used by GCI-Application. The version is of type HIL_GCI_VERSION_*. */
  uint32_t ulVersionNumber;

  /*! GCI-Application handle. */
  HIL_GCI_APP_H  hApplication;

  /*! The acyclic service interface. */
  HIL_GCI_APP_SERVICE_INTERFACE_T   tService;

  /*! The cyclic service interface. */
  HIL_GCI_APP_CYCLIC_INTERFACE_T    tCyclic;

  /*! The synchronization service interface. */
  HIL_GCI_APP_SYNC_INTERFACE_T      tSync;

  /*! The error service interface. */
  HIL_GCI_APP_ERROR_INTERFACE_T     tError;

  /*! Handle of Log book.
   * The GCI-Application is responsible for maintaining the Logbook. The handle is provided to the
   * GCI-Component for recording events, which may help during commissioning or debugging of the device.
   * If the GCI-Component does not support the PS Toolbox logging, this handle can be ignored.
   * If no Logbook is available, this member is set to NULL.
   */
  struct PS_LOGBOOK_Ttag* hLogbook;

  /*! Pointer to the Extended Status data buffer. If provided by the GCI-Application, the GCI-Component
   * will write additional status information into this buffer.
   * \note The communication data flow is unidirectional (GCI-Component -> GCI-Application).
   * \note The structure, semantics, size and times of access are specific to the GCI-Component
   *       and have to be obtained from the corresponding manual.
   * */
  uint8_t*  pabExtendedStatusArea;

  /*! Extended Status Area total size in bytes. Typically, 172 bytes are provided.
   *  In case the size is not supported by the GCI-Component, the \ref HIL_GCI_COMP_INTERFACE_INITIALIZE_FN
   *  has to return an appropriate error code. */
  uint32_t  ulExtendedStatusAreaSize;

} HIL_GCI_APP_INTERFACE_T;

/*!
 * GCI-Component interface
 * This interface structure is filled by the GCI-Component.
 */
typedef struct HIL_GCI_COMP_INTERFACE_Ttag
{
  /*! GCI version number that is used by the GCI-Component. The version is of type HIL_GCI_VERSION_*. */
  uint32_t ulVersionNumber;

  /*! Handle of the GCI-Component.  */
  HIL_GCI_COMP_H      hComponent;

  /*! Start the GCI-Interface for both GCI-Application and GCI-Component side. */
  HIL_GCI_COMP_INTERFACE_START_FN fnStart;

  /*! The acyclic service interface */
  HIL_GCI_COMP_SERVICE_INTERFACE_T tService;

  /*! The cyclic service interface */
  HIL_GCI_COMP_CYCLIC_INTERFACE_T tCyclic;

  /*! The synchronization service interface */
  HIL_GCI_COMP_SYNC_INTERFACE_T   tSync;

  /*! The error service interface. */
  HIL_GCI_COMP_ERROR_INTERFACE_T  tError;

  /*! The GCI-Component information interface. */
  HIL_GCI_COMP_INFO_T             tInfo;

} HIL_GCI_COMP_INTERFACE_T;

/*! Initialize the GCI Interface of a specific GCI-Component.
 *
 * The GCI-Application provides its interface to the GCI-Component together with GCI-Component specific initialization parameters.
 * On function return the GCI-Component has set up its interface for the GCI-Application.
 *
 * After the interfaces have been exchanged and the start function of the GCI-Component has been called, the interfaces are ready
 * to be used by each side. Using them before the start function has been called leads to undefined behavior.
 *
 * \note The function needs to be implemented by GCI-Component and is called by GCI-Application.
 *
 * \param ptApplicationInterface [in]  GCI-Application interface that will be used by the GCI-Component.
 *                                     The GCI-Component has to copy-out the provided interface data,
 *                                     as the pointer might not be valid anymore as soon as the function returns.
 * \param ptComponentInterface   [out] GCI-Component interface set up after the function successfully has returned.
 * \param pvComponentParameter   [in]  GCI-Component specific initialization parameter.
 *                                     The GCI-Component has to copy-out the provided parameter,
 *                                     as the pointer might not be valid anymore as soon as the function returns.
 *
 * \return SUCCESS_HIL_OK in case the GCI-Component successfully initialized.
 *         Otherwise an error code is returned. In the error case also the content of
 *         ptComponentInterface must be considered invalid.
 */
typedef uint32_t (*HIL_GCI_COMP_INTERFACE_INITIALIZE_FN)
(
  const HIL_GCI_APP_INTERFACE_T* ptApplicationInterface,
  HIL_GCI_COMP_INTERFACE_T*      ptComponentInterface,
  void*                          pvComponentParameter
);

/*!  \} */


/******************************************************************************/
/*! \defgroup group_HIL_GCI_Packets Generic Services
 * \{*/



/*! \defgroup HIL_GCI_SET_COMMUNICATION_STATUS_doc   Set Communication Status
 * \{ */
#define HIL_GCI_SET_COMMUNICATION_STATUS_IND                   0x0000AE00  /*!< Set Communication Status indication */
#define HIL_GCI_SET_COMMUNICATION_STATUS_RES                   0x0000AE01  /*!< Set Communication Status response */
/*! \} */

/*! \defgroup HIL_GCI_SET_MASTER_STATUS_doc   Set Master Status
 * \{ */
#define HIL_GCI_SET_MASTER_STATUS_IND                         0x0000AE02  /*!< Set Master status indication */
#define HIL_GCI_SET_MASTER_STATUS_RES                         0x0000AE03  /*!< Set Master status response */
/*! \} */

/*! \defgroup HIL_GCI_SET_IO_MIN_UPDATE_INTERVAL_doc    Set minimum process data update interval
 * \{ */
#define HIL_GCI_SET_IO_MIN_UPDATE_INTERVAL_IND                0x0000AE04  /*!< Set minimum process data update interval indication */
#define HIL_GCI_SET_IO_MIN_UPDATE_INTERVAL_RES                0x0000AE05  /*!< Set minimum process data update interval response */
/*! \} */

/*! \defgroup HIL_GCI_SET_COM_LEDS_doc   Set Communication LEDs
 * \{ */
#define HIL_GCI_SET_COM_LEDS_IND                              0x0000AE06  /*!< Set Communication LEDs indication */
#define HIL_GCI_SET_COM_LEDS_RES                              0x0000AE07  /*!< Set Communication LEDs response */
/*! \} */

/*! \defgroup HIL_GCI_SET_STATE_FIELD_doc   Set state information field
 * \{ */
#define HIL_GCI_SET_STATE_FIELD_IND                           0x0000AE08  /*!< Set state information field indication */
#define HIL_GCI_SET_STATE_FIELD_RES                           0x0000AE09  /*!< Set state information field response */
/*! \} */

/*! \defgroup HIL_GCI_RESET_STATE_FIELDS_doc   Reset state information fields
 * \{ */
#define HIL_GCI_RESET_STATE_FIELDS_IND                        0x0000AE0A  /*!< Reset state information fields indication */
#define HIL_GCI_RESET_STATE_FIELDS_RES                        0x0000AE0B  /*!< Reset state information fields response */
/*! \} */

/******************************************************************************/
/*! \addtogroup HIL_GCI_SET_COMMUNICATION_STATUS_doc
 *
 * This service provides a basic but general status of the GCI-ProtocolStack. Note that
 * the meaning of the fields is still GCI-ProtocolStack specific.
 *
 * \{*/

/*! Communication status indication data structure. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GCI_SET_COMMUNICATION_STATUS_IND_DATA_Ttag
{
  /*! Communication state.
   * Information about the current network state.
   *  - HIL_COMM_STATE_NOT_CONFIGURED = No valid configuration / no network communication
   *  - HIL_COMM_STATE_STOP           = Communication stopped by the user GCI-Application or
   *                                    an error during network communication
   *  - HIL_COMM_STATE_IDLE           = GCI-ProtocolStack is configured but not yet operational.
   *                                    No cyclic data exchange on the bus system
   *  - HIL_COMM_STATE_OPERATE        = Network communication is active, data exchange on the network
   *                                    is activated
   * \note Depending on the GCI-ProtocolStack, not all of the above states are available or may deviate
   *       in meaning.
   */
  uint32_t ulCommunicationState;

  /*! GCI-ProtocolStack specific error code or SUCCESS_HIL_OK on status changes in the "good case" */
  uint32_t ulError;

  /*! Error Counter.
   * Total number of errors detected by the GCI-ProtocolStack since start or reset, respectively.
   * A GCI-ProtocolStack counts all sorts of errors in this field no matter if they were network
   * related or caused internally. */
  uint32_t ulErrorCounter;

  /*! Communicating Status.
   * This flag is set if the GCI-ProtocolStack has successfully opened a connection
   * to at least ONE of the configured network slaves (for GCI-MasterStack),
   * respectively has an open connection to the network master (for GCI-SlaveStack).
   * If cleared, the cyclic input data should not be evaluated, because it may be invalid,
   * old or both.
   *  - 0 = No communication (No connection).
   *  - 1 = Communicating (At least one connection).
   * \note Even if the GCI-ProtocolStack reports "Communicating" the process data may not be valid. E.g.
   *       Open Modbus reports "Communicating" when a TCP connection is established, because
   *       this system works message based, no data must be transmitted. In addition, most GCI-ProtocolStacks
   *       provide a dedicated validation information together with the process data.
   * \note On slave systems it is highly recommended that the process data which is generated by the
   *       GCI-Application is updated even if the connection is not active. */
  uint8_t  bCommunicating;

  /*! Bus state.
   * Actual bus state of the GCI-ProtocolStack stack. The BUS on/off only represent
   * the cyclic I/O bus status of the protocol. If the bus is off the communication
   * stack will not accept application related connections (I/O connections) e.g. from
   * a PLC.
   * The Bus state follows the requested state from the application. A wrong
   * configuration may prevent the GCI-ProtocolStack that this will happen.
   *  - 0 = No cyclic connections will be accepted.
   *  - 1 = cyclic connections will be accepted.
   *
   * \note Depending on protocol the communication on the network is still active,
   *       particular stack with possible daisy chain topology may enable the bus
   *       PHYs regardless of the requested bus signal from application.
   */
  uint8_t  bBusState;

} HIL_GCI_SET_COMMUNICATION_STATUS_IND_DATA_T;

/*! Communication status indication structure. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GCI_SET_COMMUNICATION_STATUS_IND_Ttag
{
  HIL_PACKET_HEADER_T                           tHead;  /*!< Packet header. */
  HIL_GCI_SET_COMMUNICATION_STATUS_IND_DATA_T   tData;  /*!< Packet data. */
} HIL_GCI_SET_COMMUNICATION_STATUS_IND_T;

/*! Packet data size. */
#define HIL_GCI_SET_COMMUNICATION_STATUS_IND_SIZE  (sizeof(HIL_GCI_SET_COMMUNICATION_STATUS_IND_DATA_T))


/*! Communication status response structure. */
typedef HIL_EMPTY_PACKET_T      HIL_GCI_SET_COMMUNICATION_STATUS_RES_T;

/*! Packet data size. */
#define HIL_GCI_SET_COMMUNICATION_STATUS_RES_SIZE  (0)

/*! \}*************************************************************************/




/******************************************************************************/
/*! \addtogroup HIL_GCI_SET_MASTER_STATUS_doc
 *
 * The master status indication service offers common information over all
 * configured slaves for all GCI-MasterStacks. This indication will be
 * generated by the GCI-MasterStack if the status of one or more devices has changed.
 *
 * \note This service is only available for the GCI-MasterStack.
 *
 * \{*/

/*! Master status indication data structure. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GCI_SET_MASTER_STATUS_IND_DATA_Ttag
{
  /*! Number of configured slave devices in the master configuration. */
  uint32_t ulNumOfConfigSlaves;
  /*! Number of activated slave devices, the master has an open connection to. */
  uint32_t ulNumOfActiveSlaves;
  /*! Number of slaves reporting diagnostic issues. */
  uint32_t ulNumOfDiagSlaves;
} HIL_GCI_SET_MASTER_STATUS_IND_DATA_T;

/*! Master status indication structure. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GCI_SET_MASTER_STATUS_IND_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;  /*!< Packet header. */
  HIL_GCI_SET_MASTER_STATUS_IND_DATA_T  tData;  /*!< Packet data. */
} HIL_GCI_SET_MASTER_STATUS_IND_T;

/*! Packet data size. */
#define HIL_GCI_SET_MASTER_STATUS_IND_SIZE (sizeof(HIL_GCI_SET_MASTER_STATUS_IND_DATA_T))


/*! Master status response structure. */
typedef HIL_EMPTY_PACKET_T      HIL_GCI_SET_MASTER_STATUS_RES_T;

/*! Packet data size. */
#define HIL_GCI_SET_MASTER_STATUS_RES_SIZE   (0)
/*! \}*************************************************************************/




/******************************************************************************/
/*! \addtogroup HIL_GCI_SET_IO_MIN_UPDATE_INTERVAL_doc
 *
 * This service is used by the GCI-ProtocolStack to configure the minimum process data
 * update interval in free-run mode. The GCI-Application shall consider to not update
 * process data faster than this interval. In case the service is not used, the
 * GCI-Application shall assume a default value of 1000us.
 *
 * \note The service can be used at runtime (dynamic PDO mapping).
 * \note The update interval is restored to default value on delete config.
 * \note This service is intended to be used firmware internal only.
 *
 * \{*/

#define HIL_GCI_SET_IO_MIN_UPDATE_INTERVAL_MIN_FREERUN_UPDATE_TIME  32  /*!< Minimum allowed update time in us. */

/*! Set process data update interval structure. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GCI_SET_IO_MIN_UPDATE_INTERVAL_IND_DATA_Ttag
{
  /*! Minimal Update interval used for overload protection in free-run mode.
   * Value in microseconds, default value is 1000us, value 0-31 is not valid. */
  uint16_t usMinFreeRunUpdateInterval;
} HIL_GCI_SET_IO_MIN_UPDATE_INTERVAL_IND_DATA_T;

/*! Set process data update interval indication structure. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GCI_SET_IO_MIN_UPDATE_INTERVAL_IND_Ttag
{
  HIL_PACKET_HEADER_T                             tHead;  /*!< Packet header. */
  HIL_GCI_SET_IO_MIN_UPDATE_INTERVAL_IND_DATA_T   tData;  /*!< Packet data. */
} HIL_GCI_SET_IO_MIN_UPDATE_INTERVAL_IND_T;

/*! Packet data size. */
#define HIL_GCI_SET_IO_MIN_UPDATE_INTERVAL_IND_SIZE (sizeof(HIL_GCI_SET_IO_MIN_UPDATE_INTERVAL_IND_DATA_T))


/*! Set process data update interval response structure. */
typedef HIL_EMPTY_PACKET_T      HIL_GCI_SET_IO_MIN_UPDATE_INTERVAL_RES_T;

/*! Packet data size. */
#define HIL_GCI_SET_IO_MIN_UPDATE_INTERVAL_RES_SIZE   (0)
/*! \}*************************************************************************/

/******************************************************************************/
/*! \addtogroup HIL_GCI_SET_COM_LEDS_doc
 * This service coming from the GCI-ProtocolStack indicates protocol specific
 * COM Led states.
 *
 * For some GCI-ProtocolStack this service has to be handled by the GCI-Application
 * to be able to fulfill certification requirements.
 *
 * \note This service shall not be used to control GCI-Application behavior.
 *       It's purpose is to transport status information to be displayed on LEDs
 *       or an alternative status indicator.
 * \{*/

/*! This status indicates that there is no active LED status  */
#define HIL_GCI_COM_LED_STATUS_NO_STATUS_INFORMATION              0x0000

/*! \defgroup HIL_GCI_COM_LED_STATUS_IND_PNS Profinet device status indications
 * \{*/
#define HIL_GCI_COM_LED_STATUS_PNS_DCP_SIGNAL                     0x0101  /*!< DCP signal service is initiated via the bus. */
#define HIL_GCI_COM_LED_STATUS_PNS_WATCHDOG_TIMEOUT               0x0102  /*!< Watchdog timeout; channel, generic or extended diagnosis present; system error. */
#define HIL_GCI_COM_LED_STATUS_PNS_NO_DATA_EXCHANGE               0x0103  /*!< No data exchange */
#define HIL_GCI_COM_LED_STATUS_PNS_NO_CONFIG                      0x0104  /*!< No configuration; or low speed physical link; or no physical link. */
/*! \} */

/*! \defgroup HIL_GCI_COM_LED_STATUS_IND_EIS Ethernet/IP adapter status indications
 * \{*/
#define HIL_GCI_COM_LED_STATUS_EIS_OPERATIONAL                    0x0201  /*!< The device is operating correctly. */
#define HIL_GCI_COM_LED_STATUS_EIS_STANDBY                        0x0202  /*!< The device is not configured. */
#define HIL_GCI_COM_LED_STATUS_EIS_SELF_TEST                      0x0203  /*!< The device is performing its power up testing. */
#define HIL_GCI_COM_LED_STATUS_EIS_MINOR_FAULT                    0x0204  /*!< The device detected a recoverable minor fault, e.g. incorrect or inconsistent configuration. */
#define HIL_GCI_COM_LED_STATUS_EIS_MAJOR_FAULT                    0x0205  /*!< The device detected a non-recoverable major fault. */
#define HIL_GCI_COM_LED_STATUS_EIS_CONNECTED                      0x0206  /*!< The device has at least one established connection. */
#define HIL_GCI_COM_LED_STATUS_EIS_NO_CONNECTIONS                 0x0207  /*!< The device has no established connections, but has obtained an IP address. */
#define HIL_GCI_COM_LED_STATUS_EIS_CONNECTION_TIMEOUT             0x0208  /*!< One or more connections have timed out. */
#define HIL_GCI_COM_LED_STATUS_EIS_DUPLICATE_IP                   0x0209  /*!< The device detected that its IP address is already in use. */
/*! \} */

/*! \defgroup HIL_GCI_COM_LED_STATUS_IND_ECS EtherCAT slave status indications
 * \{*/
#define HIL_GCI_COM_LED_STATUS_ECS_INIT                           0x0301  /*!< The device is in initialization state. */
#define HIL_GCI_COM_LED_STATUS_ECS_PRE_OPERATIONAL                0x0302  /*!< The device is in pre-operational state. */
#define HIL_GCI_COM_LED_STATUS_ECS_SAFE_OPERATIONAL               0x0303  /*!< The device is in safe-operational state. */
#define HIL_GCI_COM_LED_STATUS_ECS_OPERATIONAL                    0x0304  /*!< The device is in operational state. */
#define HIL_GCI_COM_LED_STATUS_ECS_NO_ERROR                       0x0305  /*!< The communication of the device is in working condition. */
#define HIL_GCI_COM_LED_STATUS_ECS_INVALID_CONFIGURATION          0x0306  /*!< General configuration error. */
#define HIL_GCI_COM_LED_STATUS_ECS_LOCAL_ERROR                    0x0307  /*!< Slave device application has changed the EtherCAT state autonomously. */
#define HIL_GCI_COM_LED_STATUS_ECS_APPLICATION_WATCHDOG_TIMEOUT   0x0308  /*!< An application watchdog timeout has occurred. */
/*! \} */

/*! \defgroup HIL_GCI_COM_LED_STATUS_IND_SCS Sercos slave status indications
 * \{*/
#define HIL_GCI_COM_LED_STATUS_SCS_NRT_MODE                       0x0401  /*!< No Sercos communication (Non RealTime Mode). */
#define HIL_GCI_COM_LED_STATUS_SCS_LOOPBACK                       0x0402  /*!< The network state has changed from "fastforward" to "loopback". */
#define HIL_GCI_COM_LED_STATUS_SCS_CP0                            0x0403  /*!< Communication phase 0. */
#define HIL_GCI_COM_LED_STATUS_SCS_CP1                            0x0404  /*!< Communication phase 1. */
#define HIL_GCI_COM_LED_STATUS_SCS_CP2                            0x0405  /*!< Communication phase 2. */
#define HIL_GCI_COM_LED_STATUS_SCS_CP3                            0x0406  /*!< Communication phase 3. */
#define HIL_GCI_COM_LED_STATUS_SCS_CP4                            0x0407  /*!< Communication phase 4: Normal operation, no error. */

#define HIL_GCI_COM_LED_STATUS_SCS_HP0                            0x0408  /*!< Hot-plug mode 0. */
#define HIL_GCI_COM_LED_STATUS_SCS_HP1                            0x0409  /*!< Hot-plug mode 1. */
#define HIL_GCI_COM_LED_STATUS_SCS_HP2                            0x040A  /*!< Hot-plug mode 2. */

#define HIL_GCI_COM_LED_STATUS_SCS_IDENTIFICATION                 0x040D  /*!< Remote address allocation or configuration errors between Master and Slaves. */
#define HIL_GCI_COM_LED_STATUS_SCS_MST_LOSSES                     0x040E  /*!< Communication warning (Master SYNC telegrams not received). */
#define HIL_GCI_COM_LED_STATUS_SCS_APPLICATION_ERROR              0x040F  /*!< See GDP & FSP Status codes class error (API manual). */
#define HIL_GCI_COM_LED_STATUS_SCS_COMMUNICATION_ERROR            0x0410  /*!< See SCP Status codes class error (API manual). */
#define HIL_GCI_COM_LED_STATUS_SCS_WATCHDOG_ERROR                 0x0411  /*!< A watch dog error has been detected */
/*! \} */

/*! \defgroup HIL_GCI_COM_LED_STATUS_IND_PLS POWERLINK Controlled Node status indications
 * \{*/
#define HIL_GCI_COM_LED_STATUS_PLS_NMT_OFF                        0x0501  /*!< The device is not initialized yet */
#define HIL_GCI_COM_LED_STATUS_PLS_NMT_INITIALISATION             0x0502  /*!< The device is in initialization state */
#define HIL_GCI_COM_LED_STATUS_PLS_NMT_NOT_ACTIVE                 0x0503  /*!< The device is in initialized and waiting for EPL communication */
#define HIL_GCI_COM_LED_STATUS_PLS_NMT_BASIC_ETHERNET             0x0504  /*!< The device is running in Basic Ethernet mode */
#define HIL_GCI_COM_LED_STATUS_PLS_NMT_PRE_OPERATIONAL_1          0x0505  /*!< The device is in pre-operational 1 state. EPL communication with SoA cycle is established */
#define HIL_GCI_COM_LED_STATUS_PLS_NMT_PRE_OPERATIONAL_2          0x0506  /*!< The device is in pre-operational 2 state. EPL communication with SoC cycle is established */
#define HIL_GCI_COM_LED_STATUS_PLS_NMT_READY_TO_OPERATE           0x0507  /*!< The device is in ready to operate state. EPL communication is established, the configuration was done and the device is ready for cyclic data exchange */
#define HIL_GCI_COM_LED_STATUS_PLS_NMT_OPERATIONAL                0x0508  /*!< The device is in operational state. EPL communication is established and cyclic data exchange is running */
#define HIL_GCI_COM_LED_STATUS_PLS_NMT_STOPPED                    0x0509  /*!< The device is in stopped state. EPL communication is established and cyclic data exchange is stopped */
#define HIL_GCI_COM_LED_STATUS_PLS_NMT_ERROR                      0x050A  /*!< The device has an error */
/*! \} */

/*! \defgroup HIL_GCI_COM_LED_STATUS_IND_COS CANopen Slave status indications
* \{*/

/* Definitions if RUN and ERROR LEDs are visually separated from each other */
#define HIL_GCI_COM_LED_STATUS_COS_RUN_NMT_RESET                                        0x0601 /*!< NMT is in reset state. */
#define HIL_GCI_COM_LED_STATUS_COS_RUN_NMT_OPERATIONAL                                  0x0602 /*!< NMT is in operational state. */
#define HIL_GCI_COM_LED_STATUS_COS_RUN_NMT_PREOPERATIONAL                               0x0603 /*!< NMT is in pre-operational state. */
#define HIL_GCI_COM_LED_STATUS_COS_RUN_NMT_STOP                                         0x0604 /*!< NMT is in stop state. */
#define HIL_GCI_COM_LED_STATUS_COS_RUN_NMT_BAUD_DETECTION                               0x060F /*!< CAN auto baud detection is in progress. */

#define HIL_GCI_COM_LED_STATUS_COS_ERROR_NOERROR                                        0x0610 /*!< No CAN error detected. */
#define HIL_GCI_COM_LED_STATUS_COS_ERROR_CAN_WARNING_LEVEL_REACHED                      0x0620 /*!< CAN warning level reached. */
#define HIL_GCI_COM_LED_STATUS_COS_ERROR_CO_ERROR_CONTROL_EVENT                         0x0630 /*!< CANopen error event occurred. */
#define HIL_GCI_COM_LED_STATUS_COS_ERROR_CAN_BUSOFF                                     0x0640 /*!< CAN is in bus-off state. */
#define HIL_GCI_COM_LED_STATUS_COS_ERROR_BAUD_DETECTION                                 0x06F0 /*!< CAN auto baud detection is in progress. */

/* Definitions if only one STATUS LED exist */
#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_RESET_CAN_NO_ERROR                        0x0611 /*!< NMT is in reset state, no CAN error detected. */
#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_RESET_CAN_WARNING_LEVEL_REACHED           0x0621 /*!< NMT is in reset state, CAN warning level reached. */
#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_RESET_CO_ERROR_CONTROL_EVENT              0x0631 /*!< NMT is in reset state, CANopen error event occurred. */
#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_RESET_CAN_BUSOFF                          0x0641 /*!< NMT is in reset state, CAN is in bus-off state. */

#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_OPERATIONAL_CAN_NO_ERROR                  0x0612 /*!< NMT is in operational state, no CAN error detected. */
#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_OPERATIONAL_CAN_WARNING_LEVEL_REACHED     0x0622 /*!< NMT is in operational state, CAN warning level reached. */
#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_OPERATIONAL_CO_ERROR_CONTROL_EVENT        0x0632 /*!< NMT is in operational state, CANopen error event occurred. */
#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_OPERATIONAL_CAN_BUSOFF                    0x0642 /*!< NMT is in operational state, CAN is in bus-off state. */

#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_PREOPERATIONAL_CAN_NO_ERROR               0x0613 /*!< NMT is in pre-operational state, no CAN error detected. */
#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_PREOPERATIONAL_CAN_WARNING_LEVEL_REACHED  0x0623 /*!< NMT is in pre-operational state, CAN warning level reached. */
#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_PREOPERATIONAL_CO_ERROR_CONTROL_EVENT     0x0633 /*!< NMT is in pre-operational state, CANopen error event occurred. */
#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_PREOPERATIONAL_CAN_BUSOFF                 0x0643 /*!< NMT is in pre-operational state, CAN is in bus-off state. */

#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_STOP_CAN_NO_ERROR                         0x0614 /*!< NMT is in stop state, no CAN error detected. */
#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_STOP_CAN_WARNING_LEVEL_REACHED            0x0624 /*!< NMT is in stop state, CAN warning level reached. */
#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_STOP_CO_ERROR_CONTROL_EVENT               0x0634 /*!< NMT is in stop state, CANopen error event occurred. */
#define HIL_GCI_COM_LED_STATUS_COS_STATUS_NMT_STOP_CAN_BUSOFF                           0x0644 /*!< NMT is in stop state, CAN is in bus-off state. */

#define HIL_GCI_COM_LED_STATUS_CAN_STATUS_AUTO_BAUD_DETECTION                           0x06FF /*!< CAN auto baud detection is in progress. */
/*! \} */


#define HIL_GCI_COM_LED_COLOR_OFF                                   0x00  /*!< Disable all LEDs, off. */
#define HIL_GCI_COM_LED_COLOR_RED                                   0x01  /*!< Red LED on. */
#define HIL_GCI_COM_LED_COLOR_GREEN                                 0x02  /*!< Green LED on. */
#define HIL_GCI_COM_LED_COLOR_YELLOW                                0x03  /*!< Yellow LED on (or green and red if duo LED). */

#define HIL_GCI_COM_LED_CYCLE_TIME_1HZ                                40  /*!< Speed 1 Hz,   1/(40*0.025s) */
#define HIL_GCI_COM_LED_CYCLE_TIME_2HZ                                20  /*!< Speed 2 Hz,   1/(20*0.025s) */
#define HIL_GCI_COM_LED_CYCLE_TIME_2_5HZ                              16  /*!< Speed 2.5 Hz, 1/(16*0.025s) */
#define HIL_GCI_COM_LED_CYCLE_TIME_5HZ                                 8  /*!< Speed 5 Hz,   1/( 8*0.025s) */
#define HIL_GCI_COM_LED_CYCLE_TIME_10HZ                                4  /*!< Speed 10 Hz,  1/( 4*0.025s) */
#define HIL_GCI_COM_LED_CYCLE_TIME_STATIC                           0x00  /*!< No blinking solid on */

#define HIL_GCI_COM_LED_DUTY_CYCLE_0                                0x00  /*!<   0% Duty cycle (solid background color) */
#define HIL_GCI_COM_LED_DUTY_CYCLE_25                               0x01  /*!<  25% Duty cycle  */
#define HIL_GCI_COM_LED_DUTY_CYCLE_50                               0x02  /*!<  50% Duty cycle  */
#define HIL_GCI_COM_LED_DUTY_CYCLE_75                               0x03  /*!<  75% Duty cycle  */
#define HIL_GCI_COM_LED_DUTY_CYCLE_100                              0x04  /*!< 100% Duty cycle (solid foreground color) */

#define HIL_GCI_COM_LED_REPETITION_RUN_TO_END_MSK                   0x80  /*!< Disable interruption of COM LED sequence. */

/*! COM LED data structure.
 *
 * Here are some examples:
 *  - PROFINET DCP Signaling
 *      bForeColor = HIL_GCI_COM_LED_COLOR_RED;       bBackColor = HIL_GCI_COM_LED_COLOR_OFF;
 *      bCycleTime  = HIL_GCI_COM_LED_CYCLE_TIME_1HZ;  bDutyCycle = HIL_GCI_COM_LED_DUTY_CYCLE_50;
 *      bRepetitions   = 3;                       bDelay = 255;
 *      usCOMStatusCode = HIL_GCI_COM_LED_STATUS_PNS_DCP_SIGNAL;
 *
 *  - Sercos CP2
 *      bForeColor = HIL_GCI_COM_LED_COLOR_GREEN;    bBackColor = HIL_GCI_COM_LED_COLOR_YELLOW;
 *      bCycleTime  = HIL_GCI_COM_LED_CYCLE_TIME_2HZ; bDutyCycle = HIL_GCI_COM_LED_DUTY_CYCLE_50;
 *      bRepetitions   = 2;                      bDelay = 80; (repeat after 2s pause)
 *      usCOMStatusCode = HIL_GCI_COM_LED_STATUS_SCS_CP2;
 *
 *  - EtherCAT in Operational state
 *      bForeColor = HIL_GCI_COM_LED_COLOR_GREEN;       bBackColor = HIL_GCI_COM_LED_COLOR_OFF; (don't care)
 *      bCycleTime  = HIL_GCI_COM_LED_CYCLE_TIME_STATIC; bDutyCycle = HIL_GCI_COM_LED_DUTY_CYCLE_100; (don't care)
 *      bRepetitions   = 0; (don't care)            bDelay = 0; (don't care)
 *      usCOMStatusCode = HIL_GCI_COM_LED_STATUS_ECS_OPERATIONAL;
 *
 * \note bDutyCycle == 0 && bDelay == 255 stop all previous codes including HIL_GCI_COM_LED_REPETITION_RUN_TO_END_MSK
 */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GCI_COM_LED_DATA_Ttag
{
  uint8_t   bForeColor; /*!< LED foreground color (see HIL_GCI_COM_LED_COLOR_*) */
  uint8_t   bBackColor; /*!< LED background color (see HIL_GCI_COM_LED_COLOR_*) */
  uint8_t   bCycleTime; /*!< Time of one full cycle in 25ms steps (see HIL_GCI_COM_LED_CYCLE_TIME_*) */
  uint8_t   bDutyCycle; /*!< Duty cycle between foreground color and background color (see HIL_GCI_COM_LED_DUTY_CYCLE_*) */

  uint8_t   bRepetitions; /*!< Repetition (cycles) times.  0 == no repeat, foreground color solid on. */
  uint8_t   bDelay;       /*!< Delay after blink sequence in 25ms steps. 0 == no delay. 255 == stops and background color stays on. */

  uint16_t  usCOMStatusCode; /*!< See definition list HIL_GCI_COM_LED_STATUS_* */
} HIL_GCI_COM_LED_DATA_T;


/*! COM LED control indication packet data. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GCI_SET_COM_LEDS_IND_DATA_Ttag
{
  /*! LED data for COM-LED 0 and COM-LED 1. */
  HIL_GCI_COM_LED_DATA_T atCOM[2];

} HIL_GCI_SET_COM_LEDS_IND_DATA_T;

/*! COM LED control indication packet. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GCI_SET_COM_LEDS_IND_Ttag
{
  HIL_PACKET_HEADER_T               tHead;  /*!< Packet header. */
  HIL_GCI_SET_COM_LEDS_IND_DATA_T   tData;  /*!< Packet data. */
} HIL_GCI_SET_COM_LEDS_IND_T;

/*! Packet data size. */
#define HIL_GCI_SET_COM_LEDS_IND_SIZE (sizeof(HIL_GCI_SET_COM_LEDS_IND_DATA_T))


/*! COM LED control response packet. */
typedef struct HIL_EMPTY_PACKET_Ttag  HIL_GCI_SET_COM_LEDS_RES_T;

/*! Packet data size. */
#define HIL_GCI_SET_COM_LEDS_RES_SIZE (0)

/*! \}*************************************************************************/

/******************************************************************************/
/*! \addtogroup HIL_GCI_SET_STATE_FIELD_doc
 *
 * This service offers notification of the GCI-Application about one or more new state
 * information fields in the Consumer/Provider Data Area. The GCI-Component can also
 * notify an updated state information field in case the number of state entries change.
 * In case the GCI-Application cannot recognize the state information field, the
 * response shall contain an appropriate error code.
 *
 * \note The GCI-Component can notify multiple state fields to the GCI-Application.
 * \note The packet data length is either one or multiple of the size HIL_GCI_STATE_FIELD_T.
 * \note In order to update the number of state entries of an already existing state
 *       information field, bStateArea, bStateTypeID and ulStateOffset have to match.
 *
 * \{*/

/*! State field located in Consumer Data Area. */
#define HIL_GCI_STATE_FIELD_LOCATION_CONSUMER         0
/*! State field located in Provider Data Area. */
#define HIL_GCI_STATE_FIELD_LOCATION_PROVIDER         8

/*! State field data structure. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GCI_STATE_FIELD_Ttag
{
  /*! Location of the state information field HIL_GCI_STATE_FIELD_LOCATION_*. */
  uint8_t bStateArea;
  /*! Type of status information. The supported types are GCI-Component specific and has to
   *  be obtained from the corresponding API manual. */
  uint8_t bStateTypeID;
  /*! Number of state entries of type bStateTypeID. */
  uint16_t usNumOfStateEntries;
  /*! Start offset of the status information field in the location bStateArea (Provider/Consumer data). */
  uint32_t ulStateOffset;
} HIL_GCI_STATE_FIELD_T;


/*! State information field data structure. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GCI_SET_STATE_FIELD_IND_DATA_Ttag
{
  /*! The number of state information fields in atData.
   * \note The value shall always be greater than zero. */
  uint32_t          ulNumDataElements;
  /*! List of one or multiple state entries. */
  HIL_GCI_STATE_FIELD_T atData[__HIL_VARIABLE_LENGTH_ARRAY];
} HIL_GCI_SET_STATE_FIELD_IND_DATA_T;

/*! State information field indication structure. */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_GCI_SET_STATE_FIELD_IND_Ttag
{
  HIL_PACKET_HEADER_T                   tHead;  /*!< Packet header. */
  HIL_GCI_SET_STATE_FIELD_IND_DATA_T    tData;  /*!< Packet data. */
} HIL_GCI_SET_STATE_FIELD_IND_T;

/*! Packet data size. */
#define HIL_GCI_SET_STATE_FIELD_IND_SIZE(numOfDataElements) (sizeof(uint32_t) +  (numOfDataElements) * sizeof(HIL_GCI_STATE_FIELD_T))


/*! State information field response structure. */
typedef HIL_EMPTY_PACKET_T      HIL_GCI_SET_STATE_FIELD_RES_T;

/*! Packet data size. */
#define HIL_GCI_SET_STATE_FIELD_RES_SIZE   (0)

/*! \}*************************************************************************/


/******************************************************************************/
/*! \addtogroup HIL_GCI_RESET_STATE_FIELDS_doc
 *
 * This service notifies the GCI-Application that all state information fields
 * in the Consumer and Provider Data Area have been reset by the GCI-Component.
 * It can be assumed that no state information fields exist anymore.
 *
 * \{*/

/*! Reset state information field indication structure. */
typedef HIL_EMPTY_PACKET_T      HIL_GCI_RESET_STATE_FIELDS_IND_T;

/*! Packet data size. */
#define HIL_GCI_RESET_STATE_FIELDS_IND_SIZE   (0)


/*! Reset state information field response structure. */
typedef HIL_EMPTY_PACKET_T      HIL_GCI_RESET_STATE_FIELDS_RES_T;

/*! Packet data size. */
#define HIL_GCI_RESET_STATE_FIELDS_RES_SIZE   (0)

/*! \}*************************************************************************/

/*! \}*************************************************************************/

/* Backward compatible definitions, those defines will be removed in the
 * next version of GCI. Currently we have HIL_GCI_VERSION_20 */
#ifndef DOXYGEN_SKIP_SECTION
  #define HIL_COMMUNICATION_STATE_IND           HIL_GCI_SET_COMMUNICATION_STATUS_IND
  #define HIL_COMMUNICATION_STATE_RES           HIL_GCI_SET_COMMUNICATION_STATUS_RES
  #define HIL_COMMUNICATION_STATE_IND_DATA_T    HIL_GCI_SET_COMMUNICATION_STATUS_IND_DATA_T
  #define HIL_COMMUNICATION_STATE_IND_T         HIL_GCI_SET_COMMUNICATION_STATUS_IND_T
  #define HIL_COMMUNICATION_STATE_RES_T         HIL_GCI_SET_COMMUNICATION_STATUS_RES_T
  #define HIL_COMMUNICATION_STATE_IND_SIZE      HIL_GCI_SET_COMMUNICATION_STATUS_IND_SIZE
  #define HIL_COMMUNICATION_STATE_RES_SIZE      HIL_GCI_SET_COMMUNICATION_STATUS_RES_SIZE

  #define HIL_COM_LED_CONTROL_IND               HIL_GCI_SET_COM_LEDS_IND
  #define HIL_COM_LED_CONTROL_RES               HIL_GCI_SET_COM_LEDS_RES
  #define HIL_COM_LED_CONTROL_IND_COM_DATA_T    HIL_GCI_COM_LED_DATA_T
  #define HIL_COM_LED_CONTROL_IND_DATA_T        HIL_GCI_SET_COM_LEDS_IND_DATA_T
  #define HIL_COM_LED_CONTROL_IND_T             HIL_GCI_SET_COM_LEDS_IND_T
  #define HIL_COM_LED_STATUS_RES_T              HIL_GCI_SET_COM_LEDS_RES_T
  #define HIL_COM_LED_CONTROL_IND_SIZE          HIL_GCI_SET_COM_LEDS_IND_SIZE
  #define HIL_COM_LED_CONTROL_RES_SIZE          HIL_GCI_SET_COM_LEDS_RES_SIZE

  #define HIL_STATE_FIELD_IND                   HIL_GCI_SET_STATE_FIELD_IND
  #define HIL_STATE_FIELD_RES                   HIL_GCI_SET_STATE_FIELD_RES
  #define HIL_STATE_FIELD_IND_DATA_T            HIL_GCI_SET_STATE_FIELD_IND_DATA_T
  #define HIL_STATE_FIELD_IND_T                 HIL_GCI_SET_STATE_FIELD_IND_T
  #define HIL_STATE_FIELD_RES_T                 HIL_GCI_SET_STATE_FIELD_RES_T
  #define HIL_STATE_FIELD_IND_SIZE              HIL_GCI_SET_STATE_FIELD_IND_SIZE
  #define HIL_STATE_FIELD_RES_SIZE              HIL_GCI_SET_STATE_FIELD_RES_SIZE

  #define HIL_STATE_FIELD_RESET_IND             HIL_GCI_RESET_STATE_FIELDS_IND
  #define HIL_STATE_FIELD_RESET_RES             HIL_GCI_RESET_STATE_FIELDS_RES
  #define HIL_STATE_FIELD_RESET_IND_T           HIL_GCI_RESET_STATE_FIELDS_IND_T
  #define HIL_STATE_FIELD_RESET_RES_T           HIL_GCI_RESET_STATE_FIELDS_RES_T
  #define HIL_STATE_FIELD_RESET_IND_SIZE        HIL_GCI_RESET_STATE_FIELDS_IND_SIZE
  #define HIL_STATE_FIELD_RESET_RES_SIZE        HIL_GCI_RESET_STATE_FIELDS_RES_SIZE
#endif /* DOXYGEN_SKIP_SECTION */



#endif /* HIL_GENERICCOMMUNICATIONINTERFACE_H_ */


