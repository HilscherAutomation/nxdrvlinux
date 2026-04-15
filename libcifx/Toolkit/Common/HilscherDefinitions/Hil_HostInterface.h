/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: $: *//*!

  \file Hil_HostInterface.h

  Hilscher definitions and structures for dual ported memory.

**************************************************************************************/
#ifndef HIL_HOSTINTERFACE_H_
#define HIL_HOSTINTERFACE_H_

#include <stdint.h>
#include "Hil_Compiler.h"
#include "Hil_HostInterfaceDefines.h"

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_PACK_1(HIL_HOSTINTERFACE)
#endif


  /*===========================================================================*/
  /*                                                                           */
  /* DEFAULT HIF structure                                                     */
  /*                                                                           */
  /*===========================================================================*/
  /*                                                                           */
  /*   -------------------------    HIF Offset 0                               */
  /*  | System Area             |                                              */
  /*   -------------------------                                               */
  /*  | Communication Area      |                                              */
  /*   -------------------------                                               */
  /*  | Chip Control Area       |                                              */
  /*   -------------------------   HIF Offset xxxx                             */
  /*===========================================================================*/
  /*\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */


  /* Communication Channel Mailbox Count */
  #define HIL_HIF_LAYOUT_16K_MBX_CNT        (1)
  #define HIL_HIF_LAYOUT_32K_MBX_CNT        (2)
  #define HIL_HIF_LAYOUT_64K_MBX_CNT        (4)
  #define HIL_HIF_LAYOUT_256K_MBX_CNT       (14)


  /* Global definitions */
  #define HIL_HIF_MAX_SUPPORTED_CHANNELS     3               /*!< Maximum number of possible channels */
  #define HIL_HIF_PACKET_SIZE                2176            /*!< default packet size in bytes */

  /* Communication Channel Mailbox Count */
  #define HIL_HIF_MBX_CNT_16K         (1)
  #define HIL_HIF_MBX_CNT_32K         (2)
  #define HIL_HIF_MBX_CNT_64K         (4)
  #define HIL_HIF_MBX_CNT_256K        (14)

  #define HIL_HIF_SYSTEM_CHANNEL_INDEX       0               /*!< Index of the system channel, always 0 */
  #define HIL_HIF_COM_CHANNEL_INDEX          1               /*!< Index of communication channel        */
  #define HIL_HIF_CHIP_CONTROL_CHANNEL_INDEX 2               /*!< Index of the chip control channel     */

  /* Global system channel definitions */
  #define HIL_HIF_SYSTEM_MAILBOX_MIN_SIZE       128          /*!< Min Size of a system packet mailbox in bytes     */
  #define HIL_HIF_SYSTEM_MAILBOX_DEFAULT_SIZE   2176         /*!< Default Size of a system packet mailbox in bytes */
  #define HIL_HIF_SYSTEM_CHANNEL_BLOCK_NUM      7            /*!< Number of blocks at system channel               */

/* Global communication channel definitions */
  #define HIL_HIF_COM_MAILBOX_MIN_SIZE       1600            /*!< Min size of a channel packet mailbox in bytes    */
  #define HIL_HIF_COM_MAILBOX_DEFAULT_SIZE   2176            /*!< Default Size of a system packet mailbox in bytes */
  #define HIL_HIF_COM_CHANNEL_BLOCK_NUM      4               /*!< Number of blocks at system channel               */
  #define HIL_HIF_IO_DATA_MIN_SIZE           5760            /*!< Min size of I/O data                             */
  #define HIL_HIF_16K_IO_DATA_SIZE           5760            /*!< Size of I/O data for 16K HIF                     */
  #define HIL_HIF_32K_IO_DATA_SIZE           9216            /*!< Size of I/O data for 32K HIF                     */
  #define HIL_HIF_64K_IO_DATA_SIZE          20992            /*!< Size of I/O data for 64K HIF                     */
  #define HIL_HIF_256K_IO_DATA_SIZE         21760            /*!< Size of I/O data for 256K HIF                    */
  #define HIL_HIF_HOST_WINDOW_SIZE          65536            /*!< Size of Host Windos (AC/Rx/Tx)                   */

  #define HIL_HIF_HNDSHK_SYSTEM_CHANNEL      0
  #define HIL_HIF_HNDSHK_COMM_CHANNEL        1
  #define HIL_HIF_HNDSHK_MBX_FROM_HOST       2
  #define HIL_HIF_HNDSHK_MBX_TO_HOST         3

  #define HIL_HIF_MBX_IDX_MASK               0x3F
  #define HIL_HIF_MBX_WRAPAROUND             0x8000


  /* Global communication channel definitions */
  #define HIL_HIF_HANDSHAKE_SIZE             128             /*!< Size of HSC area               */

  #define HIL_HIF_CHANNEL_TBA                8               /*!< Number of TBA units */

  #define HIL_HIF_MAX_TBUF_CHANNELS          4               /*!< Maximum number of channels per single or
                                                                   tripple buffer in one direction (Rx & Tx)*/

  /* Handshake cells definitions */
  #define HIL_HIF_HSC_NETX_SYS            0
  #define HIL_HIF_HSC_NETX_COM            1
  #define HIL_HIF_HSC_NETX_SNDMBX         2
  #define HIL_HIF_HSC_NETX_RCVMBX         3
  #define HIL_HIF_HSC_HOST_SYS            8
  #define HIL_HIF_HSC_HOST_COM            9
  #define HIL_HIF_HSC_HOST_SNDMBX         10
  #define HIL_HIF_HSC_HOST_RCVMBX         11
  #define HIL_HIF_HSC_MAX                 16
  #define HIL_HIF_HSC_HOST_OFFSET         8

  /*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
  /*XX                                                           XXXXXXXXXXXXXX*/
  /*XX SYSTEM AREA LAYOUT                                        XXXXXXXXXXXXXX*/
  /*XX                                                           XXXXXXXXXXXXXX*/
  /*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/

  /*****************************************************************************/
  /*! Process Data Information (Size = 48 Byte)                                */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_PROCESS_DATA_INFO_Ttag
  {
    uint16_t  usSize;                                      /*!< 0x00 Size of the process data area */
    uint8_t   bType;                                       /*!< 0x02 Type of the process data area */
    uint8_t   bReserved;                                   /*!< 0x03 Reserved, set to zero         */
  } HIL_HIF_PROCESS_DATA_INFO_T;

  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_PROCESS_DATA_INFO_BLOCK_Ttag
  {
    HIL_HIF_PROCESS_DATA_INFO_T atProcessDataInfo[8];      /*!< Information about the process data area */
  } HIL_HIF_PROCESS_DATA_BLOCK_T;

  /*****************************************************************************/
  /*! System status block (Size = 48 Byte)                                     */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_SYSTEM_STATUS_BLOCK_Ttag
  {
    uint32_t  ulSystemStatus;                                      /*!< 0x00 Actual system state */
    uint32_t  ulSystemError;                                       /*!< 0x04 Actual system error */
    uint32_t  ulBootError;                                         /*!< 0x08 Bootup error (only set by 2nd Stage Bootloader) */
    uint32_t  ulTimeSinceStart;                                    /*!< 0x0c time since start in seconds */
    uint16_t  usCpuLoad;                                           /*!< 0x10 cpu load in 0,01% units (10000 => 100%) */
    uint16_t  usReserved;                                          /*!< 0x12 Reserved */
    uint32_t  ulHWFeatures;                                        /*!< 0x14 Hardware Features   */
    uint8_t   abReserved[24];                                      /*!< 0x18 Reserved */
  } HIL_HIF_SYSTEM_STATUS_BLOCK_T;

  /*****************************************************************************/
  /*! System watchdog block (Size = 12 Byte)                                   */
  /*****************************************************************************/
  typedef HIL_WATCHDOG_T HIL_HIF_SYSTEM_WATCHDOG_BLOCK_T;

  /*****************************************************************************/
  /*! Communication status block (Size 100 Byte)                               */
  /*****************************************************************************/
  #define HIL_HIF_COMM_STATE_UNKNOWN                              0x0000
  #define HIL_HIF_COMM_STATE_NOT_CONFIGURED                       0x0001
  #define HIL_HIF_COMM_STATE_STOP                                 0x0002
  #define HIL_HIF_COMM_STATE_IDLE                                 0x0003
  #define HIL_HIF_COMM_STATE_OPERATE                              0x0004

  #define HIL_HIF_COMM_STATE_DMA                                  0x0100
  #define HIL_HIF_COMM_STATE_BUS_ON                               0x0200

  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_COMMUNICATION_STATUS_BLOCK_Ttag
  {
    uint32_t  ulCommunicationState;                               /*!< 0x00 Actual communication state */
    uint32_t  ulCommunicationError;                               /*!< 0x04 Actual communication error */
    uint8_t   abReserved[92];                                     /*!< 0x08 Reserved                   */
  } HIL_HIF_COMMUNICATION_STATUS_BLOCK_T;

  /*****************************************************************************/
  /*! System packet mailbox (Min Size 576 Byte)                           */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_SYSTEM_MAILBOX_16K_Ttag
  {
    uint8_t   abMbx[HIL_HIF_SYSTEM_MAILBOX_MIN_SIZE];           /*!< Send mailbox packet buffer */
  } HIL_HIF_SYSTEM_MAILBOX_16K_T;

  /*****************************************************************************/
  /*! System packet mailbox default size (2172 Byte)                           */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_SYSTEM_MAILBOX_Ttag
  {
    uint8_t   abMbx[HIL_HIF_SYSTEM_MAILBOX_DEFAULT_SIZE];       /*!< Receive mailbox packet buffer */
  } HIL_HIF_SYSTEM_MAILBOX_T;


  /*****************************************************************************/
  /*! Structure of the whole default system channel (HIF) (Size 4608 Byte)     */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_SYSTEM_CHANNEL_Ttag
  {
    HIL_SYSTEM_INFO_T                    tSystemInfo;                                     /*!< 0x000:0x002F System information block  */
    HIL_HIF_PROCESS_DATA_BLOCK_T         tProcessDataInfo;                                /*!< 0x030:0x004F Process data info block   */
    uint8_t                              abReserved[16];                                  /*!< 0x050:0x005F reserved                  */
    HIL_HIF_SYSTEM_STATUS_BLOCK_T        tSystemState;                                    /*!< 0x060:0x008F System state block        */
    HIL_HIF_SYSTEM_WATCHDOG_BLOCK_T      tSystemWatchdog;                                 /*!< 0x090:0x009B System watchdog block     */
    HIL_HIF_COMMUNICATION_STATUS_BLOCK_T tCommunicationState;                             /*!< 0x09C:0x00FF Communication state block */
    HIL_HIF_SYSTEM_MAILBOX_T             tSystemMailboxFromHost;                          /*!< 0x100:0x097F mailbox from host         */
    HIL_HIF_SYSTEM_MAILBOX_T             tSystemMailboxToHost;                            /*!< 0x980:0x10FF mailbox to host           */
  } HIL_HIF_SYSTEM_CHANNEL_T;

  /*****************************************************************************/
  /*! Structure of the whole 16K system channel (HIF) (Size 512 Byte)          */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_SYSTEM_CHANNEL_16K_Ttag
  {
    HIL_SYSTEM_INFO_T                    tSystemInfo;                                     /*!< 0x000:0x002F System information block  */
    HIL_HIF_PROCESS_DATA_BLOCK_T         tProcessDataInfo;                                /*!< 0x030:0x004F Process data info block   */
    uint8_t                              abReserved[16];                                  /*!< 0x050:0x005F reserved                  */
    HIL_HIF_SYSTEM_STATUS_BLOCK_T        tSystemState;                                    /*!< 0x060:0x008F System state block        */
    HIL_HIF_SYSTEM_WATCHDOG_BLOCK_T      tSystemWatchdog;                                 /*!< 0x090:0x009B System watchdog block     */
    HIL_HIF_COMMUNICATION_STATUS_BLOCK_T tCommunicationState;                             /*!< 0x09C:0x00FF Communication state block */
    HIL_HIF_SYSTEM_MAILBOX_16K_T         tSystemMailboxFromHost;                          /*!< 0x100:0x017F mailbox from host         */
    HIL_HIF_SYSTEM_MAILBOX_16K_T         tSystemMailboxToHost;                            /*!< 0x180:0x01FF mailbox to host           */
  } HIL_HIF_SYSTEM_CHANNEL_16K_T;


  /*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
  /*XX                                                           XXXXXXXXXXXXXX*/
  /*XX COMMUNICATION CHANNEL LAYOUT                              XXXXXXXXXXXXXX*/
  /*XX                                                           XXXXXXXXXXXXXX*/
  /*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/

  /*****************************************************************************/
  /*! Channel packet mailbox block (Size 1600 Byte)                            */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_MAILBOX_BLOCK_16K_Ttag
  {
    uint8_t   aabMailbox[HIL_HIF_MBX_CNT_16K][HIL_HIF_COM_MAILBOX_MIN_SIZE];                    /*!< 0x00 mailbox packet buffer */
  } HIL_HIF_MAILBOX_BLOCK_16K_T;

  /*****************************************************************************/
  /*! Channel packet mailbox block (Size 2*2176 Byte)                            */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_MAILBOX_BLOCK_32K_Ttag
  {
    uint8_t   aabMailbox[HIL_HIF_MBX_CNT_32K][HIL_HIF_COM_MAILBOX_DEFAULT_SIZE];                /*!< 0x00 mailbox packet buffer */
  } HIL_HIF_MAILBOX_BLOCK_32K_T;

  /*****************************************************************************/
  /*! Channel packet mailbox block (Size 4*2176 Byte)                       */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_MAILBOX_BLOCK_64K_Ttag
  {
    uint8_t   aabMailbox[HIL_HIF_MBX_CNT_64K][HIL_HIF_COM_MAILBOX_DEFAULT_SIZE];                /*!< 0x00 mailbox packet buffer */
  } HIL_HIF_MAILBOX_BLOCK_64K_T;

  /*****************************************************************************/
  /*! Channel packet mailbox block (Size 14*2176 Byte)                       */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_MAILBOX_BLOCK_256K_Ttag
  {
    uint8_t   aabMailbox[HIL_HIF_MBX_CNT_256K][HIL_HIF_COM_MAILBOX_DEFAULT_SIZE];               /*!< 0x00 mailbox packet buffer */
  } HIL_HIF_MAILBOX_BLOCK_256K_T;

  /*****************************************************************************/
  /*! Process data block triple buffer                                         */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_PROCESSDATA_TRIBUF_Ttag
  {
    uint8_t   bAction;                                                                          /*!< 0x00 Action cell for input exchange */
    uint8_t   bStatus;                                                                          /*!< 0x01 Buffer state                   */
    uint8_t   abReserved[6];                                                                    /*!< 0x02 Reserved                       */
    uint8_t   abProcessData[__HIL_VARIABLE_LENGTH_ARRAY];                                       /*!< 0x08 process data                   */
  } HIL_HIF_PROCESSDATA_TRIBUF_T;

  /*****************************************************************************/
  /*! Channel process data block 16K layout                                    */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_PROCESSDATA_BLOCK_16K_Ttag
  {
    uint8_t   abProcessData[HIL_HIF_16K_IO_DATA_SIZE];
  } HIL_HIF_PROCESSDATA_BLOCK_16K_T;

  /*****************************************************************************/
  /*! Channel process data block 32K layout                                    */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_PROCESSDATA_BLOCK_32K_Ttag
  {
    uint8_t   abProcessData[HIL_HIF_32K_IO_DATA_SIZE];
  } HIL_HIF_PROCESSDATA_BLOCK_32K_T;

  /*****************************************************************************/
  /*! Channel process data block 64K layout                                    */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_PROCESSDATA_BLOCK_64K_Ttag
  {
    uint8_t   abProcessData[HIL_HIF_64K_IO_DATA_SIZE];
  } HIL_HIF_PROCESSDATA_BLOCK_64K_T;

  /*****************************************************************************/
  /*! Channel process data block 256K layout                                   */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_PROCESSDATA_BLOCK_256K_Ttag
  {
    uint8_t   abProcessData[HIL_HIF_256K_IO_DATA_SIZE];
  } HIL_HIF_PROCESSDATA_BLOCK_256K_T;


  /*****************************************************************************/
  /*! Structure of the 16K layout communication channel                        */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_COMM_CHANNEL_16K_Ttag
  {
    HIL_HIF_MAILBOX_BLOCK_16K_T      tMbxFromHost;                /*!< mailbox from host block    */
    HIL_HIF_MAILBOX_BLOCK_16K_T      tRecvMbxToHost;              /*!< mailbox to host block      */
    uint8_t                          abReserved[128];             /*!< reserved */
    HIL_HIF_PROCESSDATA_BLOCK_16K_T  tPdToHost;                   /*!< Process data 0 input area  */
    HIL_HIF_PROCESSDATA_BLOCK_16K_T  tPdFromHost;                 /*!< Process data 0 output area */
  } HIL_HIF_COMM_CHANNEL_16K_T;

  /*****************************************************************************/
  /*! Structure of the 32K layout communication channel                        */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_COMM_CHANNEL_32K_Ttag
  {
    HIL_HIF_MAILBOX_BLOCK_32K_T      tMbxFromHost;                /*!< mailbox from host block    */
    HIL_HIF_MAILBOX_BLOCK_32K_T      tRecvMbxToHost;              /*!< mailbox to host block      */
    HIL_HIF_PROCESSDATA_BLOCK_32K_T  tPdToHost;                   /*!< Process data 0 input area  */
    HIL_HIF_PROCESSDATA_BLOCK_32K_T  tPdFromHost;                 /*!< Process data 0 output area */
  } HIL_HIF_COMM_CHANNEL_32K_T;

  /*****************************************************************************/
  /*! Structure of the 64K layout communication channel                        */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_COMM_CHANNEL_64K_Ttag
  {
    HIL_HIF_MAILBOX_BLOCK_64K_T      tMbxFromHost;                /*!< mailbox from host block    */
    HIL_HIF_MAILBOX_BLOCK_64K_T      tRecvMbxToHost;              /*!< mailbox to host block      */
    HIL_HIF_PROCESSDATA_BLOCK_64K_T  tPdToHost;                   /*!< Process data 0 input area  */
    HIL_HIF_PROCESSDATA_BLOCK_64K_T  tPdFromHost;                 /*!< Process data 0 output area */
  } HIL_HIF_COMM_CHANNEL_64K_T;

  /*****************************************************************************/
  /*! Structure of the 256K layout communication channel                       */
  /*****************************************************************************/
  typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_HIF_COMM_CHANNEL_256K_Ttag
  {
    HIL_HIF_MAILBOX_BLOCK_256K_T     tMbxFromHost;                                                    /*!< mailbox from host block    */
    HIL_HIF_MAILBOX_BLOCK_256K_T     tRecvMbxToHost;                                                  /*!< mailbox to host block      */
    HIL_HIF_PROCESSDATA_BLOCK_256K_T tPdToHost;                                                       /*!< Process data 0 input area  */
    uint8_t                          abToHostReserved[HIL_HIF_HOST_WINDOW_SIZE -
                                                      sizeof(HIL_HIF_PROCESSDATA_BLOCK_256K_T)];      /*!< Reserved */
    HIL_HIF_PROCESSDATA_BLOCK_256K_T tPdFromHost;                                                     /*!< Process data 0 output area */
    uint8_t                          abFromHostReserved[HIL_HIF_HOST_WINDOW_SIZE -
                                                        sizeof(HIL_HIF_PROCESSDATA_BLOCK_256K_T)];    /*!< Reserved */
  } HIL_HIF_COMM_CHANNEL_256K_T;

  /*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
  /*XX                                                           XXXXXXXXXXXXXX*/
  /*XX CHIP CONTROL CHANNEL LAYOUT                               XXXXXXXXXXXXXX*/
  /*XX                                                           XXXXXXXXXXXXXX*/
  /*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/

  /*****************************************************************************/
  /*! Handshake block definition                                          */
  /*****************************************************************************/
  typedef struct HIL_HIF_HANDSHAKE_BLOCK_Ttag
  {
    uint32_t ulNetxSysFlags;                                      /*!< 0x00 netX system handshake flags   */
    uint32_t ulNetxCommFlags;                                     /*!< 0x04 netX channel handshake flags  */
    uint32_t ulNetxMbxFromHostIdx;                                /*!< 0x08 netX send mailbox index       */
    uint32_t ulNetxMbxToHostIdx;                                  /*!< 0x0C netX receive mailbox index    */
    uint32_t aulNetxHsc[4];                                       /*!< 0x10 netx optional handshake flags */
    uint32_t ulHostSysFlags;                                      /*!< 0x20 Host system handshake flags   */
    uint32_t ulHostCommFlags;                                     /*!< 0x24 Host channel handshake flags  */
    uint32_t ulHostMbxFromHostIdx;                                /*!< 0x28 Host send mailbox index       */
    uint32_t ulHostMbxToHostIdx;                                  /*!< 0x2C Host receive mailbox index    */
    uint32_t aulHostHsc[4];                                       /*!< 0x30 Host optional handshake flags */
    uint8_t  abReserved[64];                                      /*!< 0x40:0x7F Reserved                 */
  } HIL_HIF_HANDSHAKE_BLOCK_T;

  /*****************************************************************************/
  /*! Handshake set bit block definition                                       */
  /*****************************************************************************/
  typedef struct HIL_HIF_HANDSHAKE_SET_BLOCK_Ttag
  {
    uint32_t ulNetxSysFlags;                                      /*!< 0x00 netX system handshake flags   */
    uint32_t ulNetxCommFlags;                                     /*!< 0x04 netX channel handshake flags  */
    uint32_t ulNetxMbxFromHostIdx;                                /*!< 0x08 netX send mailbox index       */
    uint32_t ulNetxMbxToHostIdx;                                  /*!< 0x0C netX receive mailbox index    */
    uint32_t aulNetxHsc[4];                                       /*!< 0x10 netx optional handshake flags */
    uint32_t ulHostSysFlags;                                      /*!< 0x20 Host system handshake flags   */
    uint32_t ulHostCommFlags;                                     /*!< 0x24 Host channel handshake flags  */
    uint32_t ulHostMbxFromHostIdx;                                /*!< 0x28 Host send mailbox index       */
    uint32_t ulHostMbxToHostIdx;                                  /*!< 0x2C Host receive mailbox index    */
    uint32_t aulHostHsc[4];                                       /*!< 0x30 Host optional handshake flags */
    uint8_t  abReserved[64];                                      /*!< 0x40:0x7F Reserved                 */
  } HIL_HIF_HANDSHAKE_SET_BLOCK_T;

  /*****************************************************************************/
  /*! Handshake clear bit block definition                                     */
  /*****************************************************************************/
  typedef struct HIL_HIF_HANDSHAKE_CLR_BLOCK_Ttag
  {
    uint32_t ulNetxSysFlags;                                      /*!< 0x00 netX system handshake flags   */
    uint32_t ulNetxCommFlags;                                     /*!< 0x04 netX channel handshake flags  */
    uint32_t ulNetxMbxFromHostIdx;                                /*!< 0x08 netX send mailbox index       */
    uint32_t ulNetxMbxToHostIdx;                                  /*!< 0x0C netX receive mailbox index    */
    uint32_t aulNetxHsc[4];                                       /*!< 0x10 netx optional handshake flags */
    uint32_t ulHostSysFlags;                                      /*!< 0x20 Host system handshake flags   */
    uint32_t ulHostCommFlags;                                     /*!< 0x24 Host channel handshake flags  */
    uint32_t ulHostMbxFromHostIdx;                                /*!< 0x28 Host send mailbox index       */
    uint32_t ulHostMbxToHostIdx;                                  /*!< 0x2C Host receive mailbox index    */
    uint32_t aulHostHsc[4];                                       /*!< 0x30 Host optional handshake flags */
    uint8_t  abReserved[64];                                      /*!< 0x40:0x7F Reserved                 */
  } HIL_HIF_HANDSHAKE_CLR_BLOCK_T;

  /*****************************************************************************/
  /*! Handshake soft IRQ block definition                                     */
  /*****************************************************************************/
  typedef struct HIL_HIF_HANDSHAKE_IRQ_BLOCK_Ttag
  {
    uint32_t ulIrqSet;                                      /*!< 0x00 Handshake set soft IRQ   */
    uint32_t ulIrqClr;                                      /*!< 0x04 Handshake clear soft IRQ */
    uint32_t ulIrqState;                                    /*!< 0x08 Handshake soft IRQ state */
    uint8_t  abReserved[500];                               /*!< 0x0c:0x1FF Reserved           */
  } HIL_HIF_HANDSHAKE_IRQ_BLOCK_T;

  /*****************************************************************************/
  /*! Host IRQ block definition                                                */
  /*****************************************************************************/
  typedef struct HIL_HIF_HOST_IRQ_BLOCK_Ttag
  {
    uint32_t ulIrqPending;                                  /*!< 0x00 Raw IRQ register    */
    uint32_t ulIrqMasked;                                   /*!< 0x04 Masked IRQ register */
    uint32_t ulIrqLatchState;                               /*!< 0x08 Current latch state */
    uint32_t ulIrqMaskState;                                /*!< 0x0C IRQ Mask state      */
    uint32_t ulIrqLatchReset;                               /*!< 0x10 Reset latch state (confirms edge triggerd and software IRQ) */
    uint32_t ulIrqLatchSet;                                 /*!< 0x14 Set latch state (set soft IRQ)*/
    uint32_t ulIrqMaskReset;                                /*!< 0x18 Clear IRQ mask      */
    uint32_t ulIrqMaskSet;                                  /*!< 0x1C Set IRQ mask        */
    uint32_t ulIrqIsEdge;                                   /*!< 0x20 Configuration of edge triggerd IRQ */
    uint8_t  abReserved[92];                                /*!< 0x20:0x80 Reserved      */
  } HIL_HIF_HOST_IRQ_BLOCK_T;

  /*****************************************************************************/
  /*! DMA triple buffer Managment definition                                   */
  /*****************************************************************************/
  typedef struct HIL_DMA_TRIPLE_BUFFER_Ttag
  {
    uint32_t ulStatus;                                      /*!< 0x00 Triple buffer state    */
    uint32_t ulReserved1;                                   /*!< 0x04 Reserved               */
    uint32_t ulAction;                                      /*!< 0x08 Triple buffer exchange */
    uint32_t ulReserved2;                                   /*!< 0x0C Reserved               */
    uint32_t ulCtrl;                                        /*!< 0x10 Triple buffer settings */
    uint32_t aulReserved[3];                                /*!< 0x14:0x1F Reserved */
  } HIL_DMA_TRIPLE_BUFFER_T;

  /*****************************************************************************/
  /*! DMA triple buffer Managment block definition                             */
  /*****************************************************************************/
  typedef struct HIL_HIF_DMA_TRIPLE_BUFFER_BLOCK_Ttag
  {
    HIL_DMA_TRIPLE_BUFFER_T atTBA[HIL_HIF_CHANNEL_TBA];   /*!< 0x000:0x0FF DMA triple buffer managment units */
    uint8_t abReserved[3840];                               /*!< 0x100:0xFFF Reserved                          */
  } HIL_HIF_DMA_TRIPLE_BUFFER_BLOCK_T;

  /*****************************************************************************/
  /*! netX register block definition                             */
  /*****************************************************************************/
  typedef struct HIL_HIF_NETX_REGISTER_BLOCK_Ttag
  {
    uint32_t ulNetxReset;                                   /*!< 0x00 netX reset                  */
    uint32_t ulNetxResetNoPcie;                             /*!< 0x04 netX reset without PCIe     */
    uint32_t ulNetxOptions;                                 /*!< 0x08 available netX chip options */
  } HIL_HIF_NETX_REGISTER_BLOCK_T;

  /*****************************************************************************/
  /*! Chip control channel definition for parallel DPM usage                   */
  /*****************************************************************************/
  typedef struct HIL_HIF_CHIP_CONTROL_CHANNEL_PDPM_Ttag
  {
    HIL_HIF_HANDSHAKE_BLOCK_T         tHandshake;           /*!< 0x0000:0x007f Handshake block           */
    HIL_HIF_HANDSHAKE_SET_BLOCK_T     tHandshakeSet;        /*!< 0x0080:0x00FF Handshake set bit block   */

    /* NOTE: As we have not enough space in netX900 MPW parallel memory layouts to map Host IRQ area into
       the layouts, we just overwrite the handshakeclr area with the Host IRQ area by using dpm0 tunnel
       configuration!
       todo: change this on final chip! */
    //HIL_HIF_HANDSHAKE_CLR_BLOCK_T     tHandshakeClr;        /*!< 0x0100:0x017F Handshake clear bit block */
    //HIL_HIF_HANDSHAKE_IRQ_BLOCK_T     tHandshakeIrq;        /*!< 0x0200:0x03FF Handshake soft IRQ block  */
    HIL_HIF_HOST_IRQ_BLOCK_T            tHostIrq;               /*!< 0x0100:0x0180 Host IRQ block            */
    uint8_t                             abReservedSysTime[384]; /*!< 0x0180:0x0300 Reserved for systime area */
    uint8_t                             abDpmConfigWindow[256]; /*!< 0x0300:0x0400 Reserved for dpm config window */
  } HIL_HIF_CHIP_CONTROL_CHANNEL_PDPM_T;


  /*****************************************************************************/
  /*! Chip control channel definition                                          */
  /*****************************************************************************/
  typedef struct HIL_HIF_CHIP_CONTROL_CHANNEL_Ttag
  {
    HIL_HIF_HANDSHAKE_BLOCK_T         tHandshake;             /*!< 0x0000:0x007f Handshake block           */
    HIL_HIF_HANDSHAKE_SET_BLOCK_T     tHandshakeSet;          /*!< 0x0080:0x00FF Handshake set bit block   */
    HIL_HIF_HANDSHAKE_CLR_BLOCK_T     tHandshakeClr;          /*!< 0x0100:0x017F Handshake clear bit block */
    uint8_t                           abReserved1[128];       /*!< 0x0180:0x01FF Reserved                  */
    HIL_HIF_HANDSHAKE_IRQ_BLOCK_T     tHandshakeIrq;          /*!< 0x0200:0x03FF Handshake soft IRQ block  */
    HIL_HIF_HOST_IRQ_BLOCK_T          tHostIrq;               /*!< 0x0400:0x0480 Host IRQ block            */
    uint8_t                           abReservedSysTime[384]; /*!< 0x0480:0x0500 Reserved for systime area */
    uint8_t                           abReserved[0x500];      /*!< 0x0500:0x0FFF Reserved Area             */
    HIL_HIF_DMA_TRIPLE_BUFFER_BLOCK_T tDmaTripleBuffer;       /*!< 0x1000:0x1FFF Dma triple buffer block   */
    HIL_HIF_NETX_REGISTER_BLOCK_T     tNetxRegister;          /*!< 0x2000:0x200B NetX Register block       */
  } HIL_HIF_CHIP_CONTROL_CHANNEL_T;

  /*****************************************************************************/
  /*! Full HIF layout structures                                               */
  /*****************************************************************************/
  typedef struct HIL_HIF_16K_Ttag
  {
    HIL_HIF_SYSTEM_CHANNEL_16K_T         tSystemChannel;
    HIL_HIF_COMM_CHANNEL_16K_T           tCommChannel;
    HIL_HIF_CHIP_CONTROL_CHANNEL_PDPM_T  tChipControlChannel;
  } HIL_HIF_16K_T;

  typedef struct HIL_HIF_32K_Ttag
  {
    HIL_HIF_SYSTEM_CHANNEL_T             tSystemChannel;
    HIL_HIF_COMM_CHANNEL_32K_T           tCommChannel;
    HIL_HIF_CHIP_CONTROL_CHANNEL_PDPM_T  tChipControlChannel;
  } HIL_HIF_32K_T;

  typedef struct HIL_HIF_64K_Ttag
  {
    HIL_HIF_SYSTEM_CHANNEL_T             tSystemChannel;
    HIL_HIF_COMM_CHANNEL_64K_T           tCommChannel;
    HIL_HIF_CHIP_CONTROL_CHANNEL_PDPM_T  tChipControlChannel;
  } HIL_HIF_64K_T;

  typedef struct HIL_HIF_256K_Ttag
  {
    HIL_HIF_SYSTEM_CHANNEL_T        tSystemChannel;
    HIL_HIF_COMM_CHANNEL_256K_T     tCommChannel;
    HIL_HIF_CHIP_CONTROL_CHANNEL_T  tChipControlChannel;
  } HIL_HIF_256K_T;

  typedef union HIL_HIF_Utag
  {
    HIL_HIF_16K_T  t16kLayout;
    HIL_HIF_32K_T  t32kLayout;
    HIL_HIF_64K_T  t64kLayout;
    HIL_HIF_256K_T t256kLayout;
  } HIL_HIF_U;


  /*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */
  /* End of HIF Layout definition */

  /*===========================================================================*/
  /*                                                                           */
  /* Standardized Handshake Flags                                              */
  /*                                                                           */
  /*===========================================================================*/

  /* --------------------------------------------*/
  /* System Channel Handshake Flags              */
  /* --------------------------------------------*/
  /* HOST Flags */
  #define HIL_HIF_HSF_RESET                         0x0001                      /*!< Reset command bitmask               */
  #define HIL_HIF_HSF_BOOTSTART                     0x0002                      /*!< Set when device has a second stage loader, to enter bootloader mode after a system start */
  #define HIL_HIF_HSF_MBX_FROM_HOST_CMD             0x0010                      /*!< Send mailbox command bitmask        */
  #define HIL_HIF_HSF_MBX_TO_HOST_ACK               0x0020                      /*!< Receive mailbox acknowledge bitmask */

  /* HOST Flags as Bit number */
  #define HIL_HIF_HSF_RESET_BIT_NO                  0                           /*!< Reset command bitnumber               */
  #define HIL_HIF_HSF_BOOTLOADER_BIT_NO             1                           /*!< Bitnumber to be set when device has a second stage loader, to enter bootloader mode after a system start */
  #define HIL_HIF_HSF_MBX_FROM_HOST_CMD_BIT_NO      4                           /*!< Send mailbox command bitnumber        */
  #define HIL_HIF_HSF_MBX_TO_HOST_ACK_BIT_NO        5                           /*!< Receive mailbox acknowledge bitnumber */


  /* netX Flags */
  #define HIL_HIF_NSF_READY                         0x0001                      /*!< netX System READY bitmask        */
  #define HIL_HIF_NSF_ERROR                         0x0002                      /*!< General system error bitmask     */
  #define HIL_HIF_NSF_MBX_FROM_HOST_ACK             0x0010                      /*!< Send mailbox acknowledge bitmask */
  #define HIL_HIF_NSF_MBX_TO_HOST_CMD               0x0020                      /*!< Receive mailbox command bitmask  */

  /* netX Flags as Bit number */
  #define HIL_HIF_NSF_READY_BIT_NO                  0                           /*!< netX System READY bitnumber        */
  #define HIL_HIF_NSF_ERROR_BIT_NO                  1                           /*!< General system error bitnumber     */
  #define HIL_HIF_NSF_MBX_FROM_HOST_ACK_BIT_NO      4                           /*!< Send mailbox acknowledge bitnumber */
  #define HIL_HIF_NSF_MBX_TO_HOST_CMD_BIT_NO        5                           /*!< Receive mailbox command bitnumber  */

  /*--------------------------------------------*/
  /* Communication Channel Handshake Flags      */
  /*--------------------------------------------*/

  /* netX Communication Channel Flags */
  #define HIL_HIF_NCF_READY                         0x0001                      /*!< Channel is ready for packet exchange bitnumber */
  #define HIL_HIF_NCF_RUN                           0x0002                      /*!< Channel is configured bitnumber                */
  #define HIL_HIF_NCF_COMMUNICATING                 0x0004                      /*!< Channel has an active connection bitnumber     */
  #define HIL_HIF_NCF_ERROR                         0x0008                      /*!< Communication channel error bitnumber          */
  #define HIL_HIF_NCF_NEW_CONFIGURATION             0x0010                      /*!< New configuration is available bitnumber       */
  #define HIL_HIF_NCF_RESTART_REQUIRED              0x0020                      /*!< Reset required bitnumber                       */

  /* netX Communication Channel Flags as Bit number */
  #define HIL_HIF_NCF_READY_BIT_NO                  0                           /*!< Channel is ready for packet exchange bitnumber */
  #define HIL_HIF_NCF_RUN_BIT_NO                    1                           /*!< Channel is configured bitnumber                */
  #define HIL_HIF_NCF_COMMUNICATING_BIT_NO          2                           /*!< Channel has an active connection bitnumber     */
  #define HIL_HIF_NCF_ERROR_BIT_NO                  3                           /*!< Communication channel error bitnumber          */
  #define HIL_HIF_NCF_NEW_CONFIGURATION_BIT_NO      4                           /*!< New configuration is available bitnumber       */
  #define HIL_HIF_NCF_RESTART_REQUIRED_BIT_NO       5                           /*!< Reset required bitnumber                       */


  /*===========================================================================*/
  /*                                                                           */
  /* Channel block information                                                 */
  /*                                                                           */
  /*===========================================================================*/

  /*****************************************************************************/
  /*! Block configuration information                                          */
  /*****************************************************************************/
  typedef struct HIL_HIF_BLOCK_DEFINITION_Ttag
  {
    uint8_t   bChannelNumber;
    uint8_t   bBlockNumber;
    uint8_t   bBlockID;
    uint8_t   bPad;
    uint32_t  ulOffset;
    uint32_t  ulSize;
    uint32_t  aulAddInfo[2];
  } HIL_HIF_BLOCK_DEFINITION_T;

  typedef struct HIL_HIF_LAYOUT_Ttag{
    HIL_HIF_BLOCK_DEFINITION_T tSystemInfo;
//    HIL_HIF_BLOCK_DEFINITION_T tChannelInfo;
    HIL_HIF_BLOCK_DEFINITION_T tSystemStatus;
    HIL_HIF_BLOCK_DEFINITION_T tSystemWatchdog;
    HIL_HIF_BLOCK_DEFINITION_T tCommunicationStatus;
    HIL_HIF_BLOCK_DEFINITION_T tSystemMbxFromHost;
    HIL_HIF_BLOCK_DEFINITION_T tSystemMbxToHost;
    HIL_HIF_BLOCK_DEFINITION_T tChannelMbxFromHost;
    HIL_HIF_BLOCK_DEFINITION_T tChannelMbxToHost;
    HIL_HIF_BLOCK_DEFINITION_T tProcessDataFromHost;
    HIL_HIF_BLOCK_DEFINITION_T tProcessDataToHost;
    HIL_HIF_BLOCK_DEFINITION_T tChipControlHsc;
    HIL_HIF_BLOCK_DEFINITION_T tChipControlHscSet;
    HIL_HIF_BLOCK_DEFINITION_T tChipControlHscClr;
    HIL_HIF_BLOCK_DEFINITION_T tChipControlHscIrq;
    HIL_HIF_BLOCK_DEFINITION_T tChipControlHostIrq;
    HIL_HIF_BLOCK_DEFINITION_T tChipControlTba;
    HIL_HIF_BLOCK_DEFINITION_T tChipControlNetXRegister;
  }HIL_HIF_LAYOUT_T;

  typedef union HIL_HIF_LAYOUT_Utag
  {
    HIL_HIF_LAYOUT_T tLayout;
    HIL_HIF_BLOCK_DEFINITION_T atBlockDef[18];
  }HIL_HIF_LAYOUT_U;

#if 0
  /* Block ID */
  #define HIL_BLOCK_MASK                                      0x00FFL
  #define HIL_BLOCK_UNDEFINED                                 0x0000L
  #define HIL_BLOCK_SYSTEM_INFO                               0x0011L
  #define HIL_BLOCK_CHANNEL_INFO                              0x0012L
  #define HIL_BLOCK_SYSTEM_STATUS                             0x0013L
  #define HIL_BLOCK_SYSTEM_WATCHDOG                           0x0014L
  #define HIL_BLOCK_COMMUNICATION_STATUS                      0x0015L
  #define HIL_BLOCK_SYSTEM_MAILBOX_FROM_HOST                  0x0016L
  #define HIL_BLOCK_SYSTEM_MAILBOX_TO_HOST                    0x0017L
  #define HIL_BLOCK_CHANNEL_MAILBOX_FROM_HOST                 0x0018L
  #define HIL_BLOCK_CHANNEL_MAILBOX_TO_HOST                   0x0019L
  #define HIL_BLOCK_PROCESS_DATA_FROM_HOST                    0x001AL
  #define HIL_BLOCK_PROCESS_DATA_TO_HOST                      0x001BL
  #define HIL_BLOCK_CHIP_CONTROL_HSC                          0x001CL
  #define HIL_BLOCK_CHIP_CONTROL_HSC_SET                      0x001DL
  #define HIL_BLOCK_CHIP_CONTROL_HSC_CLEAR                    0x001EL
  #define HIL_BLOCK_CHIP_CONTROL_HSC_IRQ                      0x001FL
  #define HIL_BLOCK_CHIP_CONTROL_HOST_IRQ                     0x0020L
  #define HIL_BLOCK_CHIP_CONTROL_TBA                          0x0021L
  #define HIL_BLOCK_CHIP_CONTROL_NETX_REGISTER                0x0022L
#endif

#ifdef __HIL_PRAGMA_PACK_ENABLE
  #pragma __HIL_PRAGMA_UNPACK_1(HIL_HOSTINTERFACE)
#endif

//todo: new define for 4 Areas at IN/OUT required
#define HIL_HIF_DIRECTION_IN 0
#define HIL_HIF_DIRECTION_OUT 1

#define HIL_HIF_IO_DATA_OFFSET          8  /*!< Offset to beginning of actual memory of IO data
                                                  (first 8 bytes are reserved for action cells) */

/***************************************************************************
* Supported HIF Modes
***************************************************************************/
#define HIL_HIF_MODE_NONE         0   /*!< No configuration */
#define HIL_HIF_MODE_SINGLE_BUF   1   /*!< Single Buffer Mode (currently not supported) */
#define HIL_HIF_MODE_TRIPLE_BUF   2   /*!< Tripple Buffer Mode */
#define HIL_HIF_MODE_TRIPLE_INCUP 3   /*!< Tripple Buffer Mode with incremental update (DMA) */

#endif  /* HIL_HOSTINTERFACE_H_ */
