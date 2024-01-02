/*******************************************************************************

   Copyright (c) Hilscher GmbH. All Rights Reserved.

 *******************************************************************************

   Filename:
    $Id: HilTransport.h 7029 2015-07-16 12:09:53Z Robert $
   Last Modification:
    $Author: Robert $
    $Date: 2015-07-16 14:09:53 +0200 (Do, 16 Jul 2015) $
    $Revision: 7029 $

   Targets:
    OSIndependent  : yes

   Description:
    Definition for the universal Hilscher Transport Header prepended, to all
    data, send via a communication channel like USB/TCP/Serial.

   Changes:

     Version    Date        Author   Description
     ---------------------------------------------------------------------------
      11        02.10.2013  MT       Added HIL_TRANSPORT_FEATURES_PERMANENT_CONNECTION
      10        23.05.2011  MS       Added transport types HIL_TRANSPORT_TYPE_INX, HIL_TRANSPORT_TYPE_NETANALYZER, HIL_TRANSPORT_TYPE_NETPLC
      9         28.06.2010  SD       New types for 64-bit support
      8         24.06.2009  MT       HIL_TRANSPORT_BASIC_FEATURES_U structure changed data type from uint32_t to unsigned int
      7         22.07.2008  PL       New transport type HIL_TRANSPORT_TYPE_KEEP_ALIVE added
      6         15.04.2008  MT       New error code (state) HIL_TSTATE_BUFFEROVERFLOW_ERROR added
      5         07.05.2007  MT       New header definitions adjusted to standard c data types
      4         04.04.2007  RM       Default state definitions included
      3         22.03.2007  RM       Definition changed to fit new data structure
      2         16.03.2007  RM       Header reworked
      1         06.03.2007  MT       created

*******************************************************************************/

#ifndef __HILTRANSPORT__H
#define __HILTRANSPORT__H

#include <stdint.h>

/*****************************************************************************/
/*! \file HilTransport.h
*  Definition for the universal Hilscher Transport Header                    */
/*****************************************************************************/

#define HIL_TRANSPORT_IP_PORT           50111           /**< IP Port number used for transport                  */

#define HIL_TRANSPORT_COOKIE            0xA55A5AA5UL    /**< Cookie for HIL_TRANSPORT_HEADER                    */
#define HIL_TRANSPORT_PACKETID          "netX"          /**< Packet ID                                          */

#define HIL_TRANSPORT_STATE_OK          0x00            /**< Packet received successfully                       */

#define HIL_TSTATE_CHECKSUM_ERROR       0x10            /**< Packet checksum error                              */
#define HIL_TSTATE_LENGTH_INCOMPLETE    0x11            /**< Packet length incomplete                           */
#define HIL_TSTATE_DATA_TYPE_UNKNOWN    0x12            /**< Packet data type unknown                           */
#define HIL_TSTATE_DEVICE_UNKNOWN       0x13            /**< Device unknown                                     */
#define HIL_TSTATE_CHANNEL_UNKNOWN      0x14            /**< Channel unknown                                    */
#define HIL_TSTATE_SEQUENCE_ERROR       0x15            /**< Sequence number out of sync                        */
#define HIL_TSTATE_BUFFEROVERFLOW_ERROR 0x16            /**< Buffer overflow in receiver                        */
#define HIL_TSTATE_RESOURCE_ERROR       0x17            /**< Out of internal resources
                                                             (e.g. parallel services exceeded)                  */
#define HIL_TSTATE_KEEP_ALIVE_ERROR     0x20            /**< Keep Alive ComID was incorrect                     */

/*****************************************************************************/
/* Transport data types                                                      */
/*****************************************************************************/
#define HIL_TRANSPORT_TYPE_QUERYSERVER  0x0000          /**< Admim command: query basic server information      */
#define HIL_TRANSPORT_TYPE_QUERYDEVICE  0x0001          /**< Admim command: query device specific information   */
#define HIL_TRANSPORT_TYPE_RCX_PACKET   0x0100          /**< Transport type for rcX packets                     */
#define HIL_TRANSPORT_TYPE_MARSHALLER   0x0200          /**< Transport type for cifX API calls                  */
#define HIL_TRANSPORT_TYPE_INX          0x0300          /**< Transport type for INX API calls                   */
#define HIL_TRANSPORT_TYPE_NETANALYZER  0x0400          /**< Transport type for netANALYZER API calls           */
#define HIL_TRANSPORT_TYPE_NETPLC       0x0500          /**< Transport type for netPLC API calls                */
#define HIL_TRANSPORT_TYPE_ACKNOWLEDGE  0x8000          /**< Transport type for acknowledging succesful sends   */
#define HIL_TRANSPORT_TYPE_KEEP_ALIVE   0xFFFF          /**< Transport type for connection keepalive transfers  */

#define HIL_TRANSPORT_TRANSACTION_DEVICE  0x8000        /**< Transaction ID was generated by Device/Target      */

/*****************************************************************************/
/*! Structure preceeding every data packet sent to netX based devices.
*   This structure is independent from the transmission channel.             */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_HEADERtag
{
  uint32_t ulCookie;                              /**< Magic cookie                             */
  uint32_t ulLength;                              /**< Length of following data (in Bytes)      */
  uint16_t usChecksum;                            /**< CRC16 over all data, 0 = no checksum     */
  uint16_t usDataType;                            /**< Type of data following                   */
  uint8_t  bDevice;                               /**< Device number                            */
  uint8_t  bChannel;                              /**< Channel number                           */
  uint8_t  bSequenceNr;                           /**< Sequence Nr. Increased with every packet */
  uint8_t  bState;                                /**< Transmission/Target state                */
  uint16_t usTransaction;                         /**< Transaction ID                           */
  uint16_t usReserved;                            /**< unused/reserved for future use           */
} HIL_TRANSPORT_HEADER, *PHIL_TRANSPORT_HEADER;


/*****************************************************************************/
/*! Administration datatype definitions                                      */
/*****************************************************************************/

/* Error codes */
#define HIL_ADMIN_S_OK                      0           /*!< No error */

/* Features flag definition (used in QueryServer command) */
#define HIL_TRANSPORT_FEATURES_KEEPALIVE            0x00000001  /*!< Device supports keep-alive         */
#define HIL_TRANSPORT_FEATURES_NXAPI                0x00000002  /*!< Device supports NXAPI marshalling  */
#define HIL_TRANSPORT_FEATURES_PERMANENT_CONNECTION 0x80000000  /*!< Device has a steady connection (does not drop on remote device resets) */
#define HIL_TRANSPORT_QUERYSERVER_NAMELEN   32

/*****************************************************************************/
/*! Administration : Query Server structure DataType = 0x0000                */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_ADMIN_QUERYSERVER_DATA_Ttag
{
  uint32_t ulStructVersion;    /*!< Structure Version, currently 1 */
  char     szServerName[HIL_TRANSPORT_QUERYSERVER_NAMELEN];   /*!< NULL Terminated Server name    */
  uint32_t ulVersionMajor;     /*!< Major Version of Server        */
  uint32_t ulVersionMinor;     /*!< Minor Version of Server        */
  uint32_t ulVersionBuild;     /*!< Build Version of Server        */
  uint32_t ulVersionRevision;  /*!< Major Version of Server        */
  uint32_t ulFeatures;         /*!< Features flag                  */
  uint32_t ulParallelServices; /*!< Number of parallel services       */
  uint32_t ulBufferSize;       /*!< Maximum of bytes in a data packet */
  uint32_t ulDatatypeCnt;      /*!< Number of supported data types */
  uint16_t ausDataTypes[1];    /*!< Array of supported data types  */

} HIL_TRANSPORT_ADMIN_QUERYSERVER_DATA_T, *PHIL_TRANSPORT_ADMIN_QUERYSERVER_DATA_T;

/*****************************************************************************/
/*! Administration : Query Server data packet DataType = 0x0000              */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_ADMIN_QUERYSERVER_Ttag
{
  HIL_TRANSPORT_HEADER                    tHeader;
  HIL_TRANSPORT_ADMIN_QUERYSERVER_DATA_T  tData;

} HIL_TRANSPORT_ADMIN_QUERYSERVER_T, *PHIL_TRANSPORT_ADMIN_QUERYSERVER_T;


/*****************************************************************************/
/* Query device definitions (Datatype=0x0001)                                */
/*****************************************************************************/

#define HIL_TRANSPORT_QUERYDEVICE_OPT_DEVICECNT   0 /*!< Query the number of available devices    */
#define HIL_TRANSPORT_QUERYDEVICE_OPT_CHANNELCNT  1 /*!< Query the number of channels on a device */
#define HIL_TRANSPORT_QUERYDEVICE_OPT_DEVICEINFO  2 /*!< Get Device information                   */
#define HIL_TRANSPORT_QUERYDEVICE_OPT_CHANNELINFO 3 /*!< Get channel information                  */

/*****************************************************************************/
/*! Administration : Query Device request data DataType = 0x0001             */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_ADMIN_QUERYDEVICE_REQ_DATA_Ttag
{
  uint16_t usDataType;  /*!< Data type the info is requested for */
  uint16_t usReserved;  /*!< unused/reserved, set to zero        */
  uint32_t ulOption;    /*!< Option to query                     */

} HIL_TRANSPORT_ADMIN_QUERYDEVICE_REQ_DATA_T, *PHIL_TRANSPORT_ADMIN_QUERYDEVICE_REQ_DATA_T;

/*****************************************************************************/
/*! Administration : Query Device request DataType = 0x0001                  */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_ADMIN_QUERYDEVICE_REQ_Ttag
{
  HIL_TRANSPORT_HEADER                        tHeader;
  HIL_TRANSPORT_ADMIN_QUERYDEVICE_REQ_DATA_T  tData;

} HIL_TRANSPORT_ADMIN_QUERYDEVICE_REQ_T, *PHIL_TRANSPORT_ADMIN_QUERYDEVICE_REQ_T;


/*****************************************************************************/
/*! Structure of query device count response data (administration command).  */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_ADMIN_QUERYDEVICECNT_CNF_DATA_Ttag
{
  uint16_t usDatatype;                            /**< Data type the request is made for  */
  uint16_t usReserved;                            /**< Reserved field                     */
  uint32_t ulOption;                              /**< Requested option                   */
  uint32_t ulError;                               /**< Error code for te request( If ulError is set -> following data will be supressed */
  uint32_t ulDeviceCnt;                           /**< Number of supported devices        */

} HIL_TRANSPORT_ADMIN_QUERYDEVICECNT_CNF_DATA_T, *PHIL_TRANSPORT_ADMIN_QUERYDEVICECNT_CNF_DATA_T;

/*****************************************************************************/
/*! Structure of query device count response (administration command).  */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_ADMIN_QUERYDEVICE_CNF_Ttag
{
  HIL_TRANSPORT_HEADER                           tHeader;
  HIL_TRANSPORT_ADMIN_QUERYDEVICECNT_CNF_DATA_T  tData;

} HIL_TRANSPORT_ADMIN_QUERYDEVICECNT_CNF_T, *PHIL_TRANSPORT_ADMIN_QUERYDEVICECNT_CNF_T;

/*****************************************************************************/
/*! Structure of query channel count response data (administration command). */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_ADMIN_QUERYCHANNELCNT_CNF_DATA_Ttag
{
  uint16_t usDatatype;                            /**< Data type the request is made for  */
  uint16_t usReserved;                            /**< Reserved field                     */
  uint32_t ulOption;                              /**< Requested option                   */
  uint32_t ulError;                               /**< Error code for te request( If ulError is set -> following data will be supressed */
  uint32_t ulChannelCnt;                          /**< Number of supported channels on device passed via bDevice */

} HIL_TRANSPORT_ADMIN_QUERYCHANNELCNT_CNF_DATA_T, *PHIL_TRANSPORT_ADMIN_QUERYCHANNELCNT_CNF_DATA_T;

/*****************************************************************************/
/*! Structure of query channel count response (administration command).      */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_ADMIN_QUERYCHANNELCNT_CNF_Ttag
{
  HIL_TRANSPORT_HEADER                            tHeader;
  HIL_TRANSPORT_ADMIN_QUERYCHANNELCNT_CNF_DATA_T  tData;

} HIL_TRANSPORT_ADMIN_QUERYCHANNELCNT_CNF_T, *PHIL_TRANSPORT_ADMIN_QUERYCHANNELCNT_CNF_T;

/*****************************************************************************/
/*! Structure of query device info response data (administration command).   */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_ADMIN_QUERYDEVICEINFO_CNF_DATA_Ttag
{
  uint16_t usDatatype;                            /**< Data type the request is made for  */
  uint16_t usReserved;                            /**< Reserved field                     */
  uint32_t ulOption;                              /**< Requested option                   */
  uint32_t ulError;                               /**< Error code for te request( If ulError is set -> following data will be supressed */
  uint32_t ulDeviceNr;                            /**< Device Number                      */
  uint32_t ulSerialNr;                            /**< Serial Number                      */
  uint16_t usMfgNr;                               /**< Manufacturer code                  */
  uint16_t usProdDate;                            /**< Production date                    */
  uint32_t ulLicenseFlag1;                        /**< License flags 1                    */
  uint32_t ulLicenseFlag2;                        /**< License flags 2                    */
  uint16_t usLicenseID;                           /**< License ID                         */
  uint16_t usLicenseFlags;                        /**< License flags                      */
  uint16_t usDeviceClass;                         /**< Device class                       */
  uint8_t  bHwRevision;                           /**< HW Revision                        */
  uint8_t  bHwCompatibility;                      /**< HW compatibility                   */

} HIL_TRANSPORT_ADMIN_QUERYDEVICEINFO_CNF_DATA_T, *PHIL_TRANSPORT_ADMIN_QUERYDEVICEINFO_CNF_DATA_T;

/*****************************************************************************/
/*! Structure of query device info response (administration command).        */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_ADMIN_QUERYDEVICEINFO_CNF_Ttag
{
  HIL_TRANSPORT_HEADER                            tHeader;
  HIL_TRANSPORT_ADMIN_QUERYDEVICEINFO_CNF_DATA_T  tData;

} HIL_TRANSPORT_ADMIN_QUERYDEVICEINFO_CNF_T, *PHIL_TRANSPORT_ADMIN_QUERYDEVICEINFO_CNF_T;


/*****************************************************************************/
/*! Structure of query channel info response data (administration command).   */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_ADMIN_QUERYCHANNELINFO_CNF_DATA_Ttag
{
  uint16_t usDatatype;                            /**< Data type the request is made for  */
  uint16_t usReserved;                            /**< Reserved field                     */
  uint32_t ulOption;                              /**< Requested option                   */
  uint32_t ulError;                               /**< Error code for te request( If ulError is set -> following data will be supressed */
  uint16_t usCommClass;                           /**< Communication class                */
  uint16_t usProtocolClass;                       /**< Protocol class                     */
  uint16_t usConformanceClass;                    /**< Conformance class                  */
  uint16_t usReserved1;                           /**< Reserved field 1                   */
  uint8_t  szFWName[64];                          /**< Null-terminated firmware name      */
  uint16_t usFWMajor;                             /**< Firmware version (major)           */
  uint16_t usFWMinor;                             /**< Firmware version (minor)           */
  uint16_t usFWBuild;                             /**< Firmware version (build)           */
  uint16_t usFWRev;                               /**< Firmware version (revision)        */
  uint16_t usYear;                                /**< Build data of firmware (year)      */
  uint8_t  bMonth;                                /**< Build data of firmware (month)     */
  uint8_t  bDay;                                  /**< Build data of firmware (day)       */

} HIL_TRANSPORT_ADMIN_QUERYCHANNELINFO_CNF_DATA_T, *PHIL_TRANSPORT_ADMIN_QUERYCHANNELINFO_CNF_DATA_T;

/*****************************************************************************/
/*! Structure of query device info response (administration command).        */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_ADMIN_QUERYCHANNELINFO_CNF_Ttag
{
  HIL_TRANSPORT_HEADER                            tHeader;
  HIL_TRANSPORT_ADMIN_QUERYCHANNELINFO_CNF_DATA_T tData;

} HIL_TRANSPORT_ADMIN_QUERYCHANNELINFO_CNF_T, *PHIL_TRANSPORT_ADMIN_QUERYCHANNELINFO_CNF_T;

/*****************************************************************************/
/* Keep alive definitions                                                    */
/*****************************************************************************/

#define HIL_TRANSPORT_KEEP_ALIVE_FIRST_COMID    0x00000000 /**< Default value for the Keep Alive ComID             */
#define HIL_TRANSPORT_KEEP_ALIVE_CLIENT_TIMEOUT        500 /**< Timeout(ms) for the Client-side of the Keep Alive  */
#define HIL_TRANSPORT_KEEP_ALIVE_SERVER_TIMEOUT       2000 /**< Timeout(ms) for the Server-side of the Keep Alive  */

/*****************************************************************************/
/*! Keepalive data structure DataType = 0xFFFF                               */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_KEEPALIVE_DATA_Ttag
{
  uint32_t         ulComID;  /*!< KeepAlive ID. 0 = Request new from device */
} HIL_TRANSPORT_KEEPALIVE_DATA_T, *PHIL_TRANSPORT_KEEPALIVE_DATA_T;

/*****************************************************************************/
/*! Keepalive data packet DataType = 0xFFFF                                  */
/*****************************************************************************/
typedef struct HIL_TRANSPORT_KEEPALIVE_Ttag
{
  HIL_TRANSPORT_HEADER            tHeader;
  HIL_TRANSPORT_KEEPALIVE_DATA_T  tData;

} HIL_TRANSPORT_KEEPALIVE_T, *PHIL_TRANSPORT_KEEPALIVE_T;

#endif /*  __HILTRANSPORT__H */
