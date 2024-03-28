// SPDX-License-Identifier: MIT
/**************************************************************************************
 *
 * Copyright (c) 2024, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 * Description: Implementation of the netX virtual network interface.
 *
 * Changes:
 *
 *   Version   Date        Author   Description
 *   ----------------------------------------------------------------------------------
 *   1        02.01.24    SD        changed licensing terms
 *
 **************************************************************************************/

#ifdef CIFXETHERNET

#include <sys/ioctl.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <time.h>
#include <sys/queue.h>
#include <netlink/cli/utils.h>
#include <netlink/cli/link.h>

#include "Hil_Packet.h"
#include "Hil_Results.h"
#include "Hil_ApplicationCmd.h"
#include "DrvEth_GCI_API.h"
#include "cifXUser.h"
#include "cifXErrors.h"
#include "cifXHWFunctions.h"
#include "netx_tap.h"

#define TUNTAP_DEVICEPATH    "/dev/net/tun"

#define SEND_RETRIES 0

#define LINK_STATE_POLL_INTERVAL 5 /* in seconds */

void* g_eth_list_lock;

typedef struct NETX_ETH_DEV_Ttag
{
  TAILQ_ENTRY(NETX_ETH_DEV_Ttag) lentry;
  int                eth_fd;
  char               cifxeth_name[CIFX_MAX_FILE_NAME_LENGTH];
  char               event_path[CIFX_MAX_FILE_NAME_LENGTH];
  NETX_ETH_DEV_CFG_T config;
  CIFXHANDLE         cifx_driver;
  CIFXHANDLE         cifx_channel;
  PDEVICEINSTANCE    devinst;
  uint32_t           channel_no;
  pthread_t          eth_to_cifx_thread;
  pthread_t          cifx_to_eth_thread;
  int                stop_to_eth;
  int                stop_to_cifx;
  void*              com_lock;
  void*              send_event;
  uint32_t           active_sends;
  uint32_t           send_packets;
  uint32_t           recv_packets;
  int                link_up;
  void*              link_event;

} NETX_ETH_DEV_T;

static TAILQ_HEAD(, NETX_ETH_DEV_Ttag) s_DeviceList = TAILQ_HEAD_INITIALIZER( s_DeviceList);

static int32_t cifxeth_search_eth_channel   ( char* szDeviceName, uint32_t ulSearchIdx, uint32_t* pulChannelNumber);
static int     cifxeth_allocate_tap         ( NETX_ETH_DEV_T* internal_dev, char* prefix);
static void    cifxeth_free_tap             ( NETX_ETH_DEV_T* internal_dev, char* name);
static void    cifxeth_delete_device        ( NETX_ETH_DEV_T* internal_dev);
static int32_t cifxeth_register_app         ( NETX_ETH_DEV_T* internal_dev, int fRegister);
static int     cifxeth_create_com_thread    ( NETX_ETH_DEV_T* internal_dev);
static int     cifxeth_create_cifx_thread   ( NETX_ETH_DEV_T* internal_dev);
static int32_t cifxeth_update_device_config ( NETX_ETH_DEV_T* internal_dev);
static int32_t cifxeth_update_link_state    ( NETX_ETH_DEV_T* internal_dev);
static int32_t cifxeth_get_extended_info    ( NETX_ETH_DEV_T* internal_dev, uint32_t ulInformationRequest, void* pvBuffer, uint32_t ulBufLen);
static void*   eth_to_cifx_thread           ( void* arg);
static void*   cifx_to_eth_thread           ( void* arg);
static NETX_ETH_DEV_T* find_device          ( char* name);

struct nl_link_arg {
  struct rtnl_link *change;
  struct nl_sock *sock;
  NETX_ETH_DEV_T* priv;
};

static void set_cb(struct nl_object *obj, void *arg)
{
  struct rtnl_link *link = nl_object_priv(obj);
  struct nl_link_arg *link_arg = arg;

  if (rtnl_link_change(link_arg->sock, link, link_arg->change, 0))
  {
    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      USER_Trace( link_arg->priv->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Link-update failed for %s", link_arg->priv->cifxeth_name);
    }
  }
}

void nl_signal_link_change( NETX_ETH_DEV_T* internal_dev, int state) {
  struct nl_sock *sock;
  struct nl_cache *link_cache;
  struct rtnl_link *link, *change;
  struct nl_link_arg link_arg;

  if (NULL == (sock = nl_cli_alloc_socket()))
  {
    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Link-update failed - failed to allocate socket", internal_dev->cifxeth_name);
    }
    return;
  }
  if (0 == nl_cli_connect(sock, NETLINK_ROUTE))
  {
    link_cache = nl_cli_link_alloc_cache(sock);
    link = nl_cli_link_alloc();
    change = nl_cli_link_alloc();

    nl_cli_link_parse_name(link, internal_dev->cifxeth_name);
    if (state != 0) {
      rtnl_link_set_flags(change, IFF_UP);
    } else {
      rtnl_link_unset_flags(change, IFF_UP);
    }

    link_arg.sock = sock;
    link_arg.change = change;
    link_arg.priv = internal_dev;
    nl_cache_foreach_filter(link_cache, OBJ_CAST(link), set_cb, &link_arg);
  } else
  {
    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Link-update failed - failed to connect socket", internal_dev->cifxeth_name);
    }
  }
}

/*****************************************************************************/
/*! This function creates a netX based ethernet interface
*   \param config  pointer to configuration structure
*   \return != NULL on success                                               */
/*****************************************************************************/
void* cifxeth_create_device(NETX_ETH_DEV_CFG_T* config)
{
  NETX_ETH_DEV_T* internal_dev = NULL;
  void*           ret          = NULL;
  int32_t         cifx_error   = CIFX_NO_ERROR;
  int32_t         search_error = CIFX_NO_ERROR;
  uint32_t        channel_no   = 0;
  uint32_t        eth_no       = 0;
  int             err          = 0;

  if (NULL == config)
    goto exit;

  if (NULL != find_device( config->cifx_name))
    return NULL;

  /* try to find a channel providing an ethernet interface */
  while (CIFX_NO_ERROR == (search_error = cifxeth_search_eth_channel( config->cifx_name, channel_no, &channel_no)))
  {
    if(NULL != (internal_dev = (NETX_ETH_DEV_T*)OS_Memalloc( sizeof(*internal_dev))))
    {
      OS_Memset( internal_dev, 0, sizeof(NETX_ETH_DEV_T));
      if (NULL == (internal_dev->com_lock = OS_CreateLock()))
      {
        OS_Memfree( internal_dev);
        goto exit;

      } else if (NULL == (internal_dev->send_event = OS_CreateEvent()))
      {
        OS_DeleteLock( internal_dev->com_lock);
        OS_Memfree( internal_dev);
        goto exit;

      } else if (NULL == (internal_dev->link_event = OS_CreateEvent()))
      {
        OS_DeleteEvent( internal_dev->send_event);
        OS_DeleteLock( internal_dev->com_lock);
        OS_Memfree( internal_dev);
        goto exit;
      }
      internal_dev->channel_no = channel_no;

      if(CIFX_NO_ERROR != (cifx_error = xDriverOpen( &internal_dev->cifx_driver)))
      {
        fprintf(stderr, "Ethernet-IF Error: %s: Error opening cifX Device Driver (Ret=0x%08X)\n", __TIME__, cifx_error);

      } else if(CIFX_NO_ERROR == (cifx_error = xChannelOpen(internal_dev->cifx_driver,
                                                            config->cifx_name,
                                                            internal_dev->channel_no,
                                                            &internal_dev->cifx_channel)))
      {
        char             prefix[CIFX_MAX_FILE_NAME_LENGTH];
        CIFX_PACKET      tDummy;
        uint32_t         i        = 0;
        PCHANNELINSTANCE ptdevice = (PCHANNELINSTANCE)internal_dev->cifx_channel;
        internal_dev->devinst     = (PDEVICEINSTANCE)ptdevice->pvDeviceInstance;

        /* in case there are remaining packages try to empty mailbox */
        for(i=0;i<16;i++) {
          if(CIFX_NO_ERROR != xChannelGetPacket(internal_dev->cifx_channel, sizeof(tDummy), &tDummy, 50))
          break;
        }

        OS_Memcpy( &internal_dev->config, config, sizeof(internal_dev->config));
        strcpy( internal_dev->config.cifx_name, config->cifx_name);
        strcpy( prefix, config->cifx_name);
        for (i=0; i<strlen(config->cifx_name); i++)
          prefix[i] = tolower(prefix[i]);

        internal_dev->eth_fd = -1;
        if( (internal_dev->eth_fd = cifxeth_allocate_tap( internal_dev, prefix)) < 0)
        {
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Error allocating tap device for '%s'. Error=%d", config->cifx_name, internal_dev->eth_fd);
          }
        } else
        {
          /* signal link down since TAP device is up by default */
          nl_signal_link_change( internal_dev, 0);
          eth_no++;
          strcpy( config->eth_dev_name, internal_dev->cifxeth_name);

          if(g_ulTraceLevel & TRACE_LEVEL_INFO)
          {
            USER_Trace( internal_dev->devinst, TRACE_LEVEL_INFO, "Ethernet-IF Info: Successfully created '%s' at channel %d on device '%s'", internal_dev->cifxeth_name, channel_no, config->cifx_name);
          }
          /* de-register application since may not be de-registered */
          cifxeth_register_app( internal_dev, 0);
          /* Register for ethernet service on device */
          if (CIFX_NO_ERROR == (cifxeth_register_app( internal_dev, 1)))
          {
            /* Create threads for packet exchange */
            if(cifxeth_create_cifx_thread( internal_dev) != 0)
            {
              if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
              {
                USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Error creating cifX ethernet channel thread for %s.", internal_dev->cifxeth_name);
              }
            } else
            {
              /* Set device MAC address from response packet */
              //cifxeth_update_device_config( internal_dev);

              /* Device successfully created */
              OS_EnterLock( g_eth_list_lock);
              TAILQ_INSERT_TAIL( &s_DeviceList, internal_dev, lentry);
              ret = internal_dev;
              OS_LeaveLock( g_eth_list_lock);
              return ret;
            }
          }
        }
      } else
      {
        fprintf(stderr, "Ethernet-IF Error: %s: Error opening cifX Ethernet Channel (Board=%s, Channel=%u, Errror=0x%08X)\n", __TIME__,
                config->cifx_name, internal_dev->channel_no, cifx_error);
      }
      if(NULL == ret)
      {
        cifxeth_delete_device( internal_dev);
      }
    } else
    {
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Not enough memory to create cifx virtual ethernet interface!");
      }
    }
    channel_no++;
  }
exit:
  return ret;
}

/*****************************************************************************/
/*! This removes a previously with cifxeth_create_device() created netX based ethernet interface
 * The function requires either a handle or config structure containing the name of the cifX device
*   \param devicehandle pointer to handle returned by cifxeth_create_device()
*   \param config       pointer to configuration structure
*   \return != NULL on success                                               */
/*****************************************************************************/
void cifxeth_remove_device( void* devicehandle, NETX_ETH_DEV_CFG_T* config)
{
  NETX_ETH_DEV_T* internal_dev = (NETX_ETH_DEV_T*)devicehandle;

  if ((NULL == internal_dev) && (NULL == config))
    return;

  if ((NULL != internal_dev) || (NULL != (internal_dev = find_device( config->cifx_name))))
  {
    cifxeth_delete_device( internal_dev);
  }
}

/*****************************************************************************/
/*! This function empties mailbox
*   \param internal_dev pointer to internal netx-ethernet device             */
/*****************************************************************************/
void empty_mailbox( NETX_ETH_DEV_T* internal_dev) {
  uint32_t ulRecvPktCount = 0;
  uint32_t ulSendPktCount = 0;
  int32_t  lRet           = 0;

  if ((internal_dev == NULL) || (internal_dev->cifx_channel == NULL))
    return;

  lRet = xChannelGetMBXState( internal_dev->cifx_channel, &ulRecvPktCount, &ulSendPktCount);
  while((lRet == CIFX_NO_ERROR) && (ulRecvPktCount > 0)) {
    CIFX_PACKET cifx_packet;

    lRet = xChannelGetPacket( internal_dev->cifx_channel, sizeof(cifx_packet), &cifx_packet, CIFX_TO_CONT_PACKET);
    if (lRet == CIFX_NO_ERROR)
      lRet = xChannelGetMBXState( internal_dev->cifx_channel, &ulRecvPktCount, &ulSendPktCount);
  }
}

/*****************************************************************************/
/*! This function deletes netX based ethernet interface
*   \param internal_dev pointer to internal netx-ethernet device
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static void cifxeth_delete_device( NETX_ETH_DEV_T* internal_dev)
{
  if(NULL != internal_dev)
  {
    if (0 != internal_dev->eth_to_cifx_thread) {
      internal_dev->stop_to_cifx = 1;
      pthread_join( internal_dev->eth_to_cifx_thread, NULL);
    }

    if (0 != internal_dev->cifx_to_eth_thread) {
      internal_dev->stop_to_eth = 1;
      pthread_join( internal_dev->cifx_to_eth_thread, NULL);
    }

    if (internal_dev->cifx_channel != NULL)
      cifxeth_register_app( internal_dev, 0);

    /* remove any pending packets */
    empty_mailbox( internal_dev);

    cifxeth_free_tap(internal_dev,internal_dev->cifxeth_name);

    if (NULL != internal_dev->link_event)
      OS_DeleteEvent( internal_dev->link_event);

    if (NULL != internal_dev->com_lock)
      OS_DeleteLock( internal_dev->com_lock);

    if (NULL != internal_dev->send_event)
      OS_DeleteEvent( internal_dev->send_event);

    if(NULL != internal_dev->cifx_channel)
      xChannelClose(internal_dev->cifx_channel);

    if(NULL != internal_dev->cifx_driver)
      xDriverClose(internal_dev->cifx_driver);

    OS_EnterLock( g_eth_list_lock);
    if (!TAILQ_EMPTY( &s_DeviceList)) {
      TAILQ_REMOVE( &s_DeviceList, internal_dev, lentry);
      OS_Memfree(internal_dev);
    }
    OS_LeaveLock( g_eth_list_lock);
  }
}

/*****************************************************************************/
/*! removes all cifx tap devices reside in /sys/class/net                    */
/*****************************************************************************/
void cifxeth_sys_cleanup(void) {
  struct dirent**       namelist;
  int                   num_virt_eth;

  num_virt_eth = scandir("/sys/class/net/", &namelist, 0, alphasort);
  if(num_virt_eth > 0)
  {
    int currenteth;
    for(currenteth = 0; currenteth < num_virt_eth; ++currenteth)
    {
      if (0 == strncmp("cifx",namelist[currenteth]->d_name,4)) {
        cifxeth_free_tap(NULL,namelist[currenteth]->d_name);
      }
      free(namelist[currenteth]);
    }
    free(namelist);
  }
}

static int is_ethernet_channel(HIL_DPM_CHANNEL_INFO_BLOCK_T* ptChannel) {
  int ret = 0;

  if (HIL_COMM_CLASS_MESSAGING == ptChannel->tCom.usCommunicationClass) {
    if (HIL_PROT_CLASS_ETHERNET == ptChannel->tCom.usProtocolClass) {
      /* old identification */
      ret = 1;
    } else if( (HIL_PROT_CLASS_NETWORK_SERVICES == ptChannel->tCom.usProtocolClass) &&
               (ptChannel->tCom.usProtocolConformanceClass & HIL_CONF_CLASS_FLAG_NDIS_AWARE) ) {
      /* new identification */
      ret = 1;
    }
  }

  return ret;
}

/*****************************************************************************/
/*! This function searches a cifx device for an existing Ethernet channel
*   \param szDeviceName     name of the cifX device
*   \param ulSearchIdx      Start index for channel information search
*   \param ulSearchIdx      pointer to channel providing ethernet interface
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t cifxeth_search_eth_channel( char*  szDeviceName,
                                 uint32_t  ulSearchIdx,
                                 uint32_t* pulChannelNumber)
{
  int32_t          lRet       = CIFX_NO_ERROR;
  CIFXHANDLE       hSysdevice = NULL;
  CIFXHANDLE       hDriver    = NULL;
  PCHANNELINSTANCE ptdevice   = NULL;
  PDEVICEINSTANCE  ptDevInst  = NULL;

  if (ulSearchIdx>=CIFX_MAX_NUMBER_OF_CHANNELS)
    return CIFX_INVALID_PARAMETER;

  if (CIFX_NO_ERROR != xDriverOpen(&hDriver))
  {
    fprintf( stderr, "Ethernet-IF Error: %s: Error opening driver to for ethernet interface (lRet=0x%08X)\n",__TIME__,lRet);
    /* Check if we have a channel that might be used for Ethernet / NDIS */
  } else if(CIFX_NO_ERROR != (lRet = xSysdeviceOpen( hDriver,
                                              szDeviceName,
                                              &hSysdevice)))
  {
    fprintf( stderr, "Ethernet-IF Error: %s: Error opening system device to read channel info block to detect channels usable for ethernet interface (lRet=0x%08X). - %s\n",__TIME__,lRet, szDeviceName);
  } else
  {
    /* Read channel information block */
    SYSTEM_CHANNEL_CHANNEL_INFO_BLOCK tChannelInfoBlock = {{{0}}};

    ptdevice  = (PCHANNELINSTANCE)hSysdevice;
    ptDevInst = (PDEVICEINSTANCE)ptdevice->pvDeviceInstance;

    if(CIFX_NO_ERROR != (lRet = xSysdeviceInfo( hSysdevice,
                                                CIFX_INFO_CMD_SYSTEM_CHANNEL_BLOCK,
                                                sizeof(tChannelInfoBlock),
                                                &tChannelInfoBlock)))
    {
      /* Error reading system info block */
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInst,
                   TRACE_LEVEL_ERROR,
                   "Ethernet-IF Error: Error reading channel info block to detect channels usable for ethernet interface (lRet=0x%08X).",
                   lRet);
      }

    } else
    {
      uint32_t ulStartIndex     = 0;
      int      fComChannelFound = 0;

      if (ulSearchIdx>=ptDevInst->ulCommChannelCount)
        return CIFX_INVALID_PARAMETER;

      for( ; ulStartIndex < CIFX_MAX_NUMBER_OF_CHANNELS; ++ulStartIndex)
      {
        HIL_DPM_CHANNEL_INFO_BLOCK_T* ptChannel = (HIL_DPM_CHANNEL_INFO_BLOCK_T*)&tChannelInfoBlock.abInfoBlock[ulStartIndex][0];

        if( (HIL_CHANNEL_TYPE_COMMUNICATION == ptChannel->tHandshake.bChannelType))
        {
          if (DEV_IsReady( ptDevInst->pptCommChannels[ulSearchIdx]))
          {
            fComChannelFound = 1;
            lRet             = CIFX_NO_ERROR;
            break;
          } else
          {
            lRet = CIFX_DEV_NOT_READY;
          }
        }
      }
      lRet = CIFX_INVALID_BOARD;
      /* Check system info block entries */
      for( ; (fComChannelFound && ((ulSearchIdx + ulStartIndex) < CIFX_MAX_NUMBER_OF_CHANNELS)); ++ulSearchIdx)
      {
        HIL_DPM_CHANNEL_INFO_BLOCK_T* ptChannel = (HIL_DPM_CHANNEL_INFO_BLOCK_T*)&tChannelInfoBlock.abInfoBlock[ulSearchIdx + ulStartIndex][0];

        if(is_ethernet_channel(ptChannel))
        {
          /* only create a NDIS interface if the interface is not created already */
          if (NULL != pulChannelNumber)
            *pulChannelNumber = ulSearchIdx;

          lRet = CIFX_NO_ERROR;
          break;
        }
      }
    }
    /* Close system device */
    xSysdeviceClose(hSysdevice);
  }
  xDriverClose(hDriver);

  return lRet;
}

/*****************************************************************************/
/*! This function allocates and initializes a tap device
*   \param internal_dev pointer to internal netx-ethernet device
*   \param prefix       prefix of the device (e.g. "cifX" -> cifX[x])
*   \param dev          returns the name of created device (-> cifX[x])
*   \return a valid file descriptor to the device - on success (>=0)         */
/*****************************************************************************/
static int cifxeth_allocate_tap( NETX_ETH_DEV_T* internal_dev, char* prefix)
{
  struct ifreq ifr;
  int          ret;

  if( (ret = open( TUNTAP_DEVICEPATH, O_RDWR)) >= 0 )
  {
    int err;

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = (IFF_TAP | IFF_NO_PI);

    if(prefix)
      strncpy( ifr.ifr_name, prefix, IFNAMSIZ);

    if( (err = ioctl(ret, TUNSETIFF, (void *) &ifr)) < 0 )
    {
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Error creating tap device (TUNSETIFF) '%s'.    Error=%d", prefix, errno);
      }
      close(ret);
      ret = err;

    } else
    {
      strcpy( internal_dev->cifxeth_name, prefix);
      sprintf(internal_dev->event_path,"/sys/class/net/%s/uevent",prefix);
      internal_dev->eth_fd = ret; /* set temp. since cifxeth_update_device_config() deals with that handle */
      cifxeth_update_device_config(internal_dev);
    }
  } else
  {
    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Error opening tun interface '%s'. Error=%d", TUNTAP_DEVICEPATH, errno);
    }
    ret = -errno;
  }
  return ret;
}

/*****************************************************************************/
/*! This function frees a tap device
 *   \param internal_dev pointer to internal netx-ethernet device
 *   \param name         name of device in case device is already closed (->link down) */
/*****************************************************************************/
void cifxeth_free_tap(NETX_ETH_DEV_T* internal_dev, char* name) {

  if (NULL != internal_dev) {
    if (internal_dev->eth_fd>=0) {
      ioctl(internal_dev->eth_fd, TUNSETPERSIST, 0);
      close(internal_dev->eth_fd);
      internal_dev->eth_fd = -1;
    }
  }
  /* we have also check the name, in case of link down the handle is '-1' */
  if (name != NULL) {
    struct ifreq ifr;
    int          ret;

    if ((ret = open( TUNTAP_DEVICEPATH, O_RDWR))) {
      memset(&ifr, 0, sizeof(ifr));
      ifr.ifr_flags = (IFF_TAP | IFF_NO_PI);
      strncpy( ifr.ifr_name, name, IFNAMSIZ);
      ioctl(ret, TUNSETIFF, (void *) &ifr);
      ioctl(ret, TUNSETPERSIST, 0);
      close(ret);
    }
  }
}

/*****************************************************************************/
/*! This function creates send/receivce thread
 *   \param internal_dev pointer to internal cifx eth channel
 *   \return >0 on success                                                    */
/*****************************************************************************/
static int cifxeth_create_cifx_thread(NETX_ETH_DEV_T* internal_dev)
{
  int            ret = -1;
  pthread_attr_t attr;

  internal_dev->stop_to_eth = 0;
  if(0 == (ret = pthread_attr_init(&attr)))
  {
    ret = pthread_create(&internal_dev->cifx_to_eth_thread,
                          &attr,
                          cifx_to_eth_thread,
                          internal_dev);
  }
  return ret;
}

/*****************************************************************************/
/*! This function creates send/receivce thread
*   \param internal_dev pointer to internal netx-ethernet device
*   \return >0 on success                                                    */
/*****************************************************************************/
static int cifxeth_create_com_thread(NETX_ETH_DEV_T* internal_dev)
{
  int            ret = -1;
  pthread_attr_t attr;

  internal_dev->stop_to_cifx = 0;
  if(0 == (ret = pthread_attr_init(&attr)))
  {
    ret = pthread_create(&internal_dev->eth_to_cifx_thread,
                                 &attr,
                                 eth_to_cifx_thread,
                                 internal_dev);
  }

  return ret;
}

/*****************************************************************************/
/*! Send thread: processes eth packets from tapX to cifX device
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static void* eth_to_cifx_thread(void* arg)
{
  NETX_ETH_DEV_T*                 internal_dev = (NETX_ETH_DEV_T*)arg;
  int                             fd           = internal_dev->eth_fd;
  char                            buffer[1514];
  DRVETH_GCI_SEND_ETH_FRAME_PCK_T cifx_packet;
  int                             select_ret = 0;

  memset(&cifx_packet.tReq.tHead, 0, sizeof(cifx_packet.tReq.tHead));

  cifx_packet.tReq.tHead.ulCmd  = DRVETH_GCI_CMD_SEND_ETH_FRAME_REQ;
  cifx_packet.tReq.tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL;

  while(1)
  {
    fd_set readfds, exceptfds;
    struct timeval  timeout = {0};
    timeout.tv_sec  = 0;
    timeout.tv_usec = 500 * 1000; /* Default wait timeout = 500ms */

    FD_ZERO(&readfds);
    FD_ZERO(&exceptfds);
    FD_SET(fd, &readfds);
    FD_SET(fd, &exceptfds);

    /* check link state */
    while (0 == internal_dev->link_up)
    {
      OS_WaitEvent( internal_dev->link_event, 10);
      if (internal_dev->stop_to_cifx == 1)
        break;
    }

    if((select_ret = select( fd+1, &readfds, NULL, &exceptfds, &timeout)) > 0)
    {
      if(FD_ISSET(fd, &exceptfds))
      {
        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Exception on Ethernet Device file descriptor, exiting thread");
        }
        break;
      }

      if(FD_ISSET(fd, &readfds))
      {
        int32_t cifx_error;
        ssize_t recv_len;
        int retry = 0;

        recv_len = read(fd, buffer, sizeof(buffer));

        while (internal_dev->active_sends>0x08)
        {
          OS_WaitEvent( internal_dev->send_event, 10);
          if (internal_dev->stop_to_cifx == 1)
            break;
        }
        if (internal_dev->stop_to_cifx == 1)
          break;

        if(recv_len > 0)
        {
          memcpy( &cifx_packet.tReq.tData, buffer, recv_len);
          cifx_packet.tReq.tHead.ulLen = recv_len;
          if (recv_len<60)
          {
            memset( (cifx_packet.tReq.tData.abData + recv_len), 0, (60 - recv_len));
            cifx_packet.tReq.tHead.ulLen = 60;
          }
          retry = SEND_RETRIES;
          do {
            cifx_error = xChannelPutPacket( internal_dev->cifx_channel, (CIFX_PACKET*)&cifx_packet, CIFX_TO_CONT_PACKET);
            if (cifx_error == CIFX_NO_ERROR) {
              OS_EnterLock( internal_dev->com_lock);
              internal_dev->active_sends++;
              OS_LeaveLock( internal_dev->com_lock);
            }
          } while ((cifx_error == CIFX_DEV_MAILBOX_FULL) && (retry-->0));

          if ((g_ulTraceLevel & TRACE_LEVEL_DEBUG) && ((SEND_RETRIES-retry) != SEND_RETRIES))
          {
            USER_Trace( internal_dev->devinst, TRACE_LEVEL_DEBUG, "Ethernet-IF Error: Sending a packet took %d-%dms)!", (SEND_RETRIES-retry+1)*CIFX_TO_CONT_PACKET, (SEND_RETRIES-retry)*CIFX_TO_CONT_PACKET);
          }
          if (CIFX_NO_ERROR != cifx_error)
          {
            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Error sending packet to cifX Device. (Error=0x%08X)", cifx_error);
            }
          }
        }
      }
    } else if (0 == select_ret)
    {
      //continue;
    } else
    {
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Error on select/or stop requested for Ethernet Device file descriptor, exiting thread");
      }
      break;
    }
    if (internal_dev->stop_to_cifx == 1)
      break;
  }

  return NULL;
}

/*****************************************************************************/
/*! send confirmation of the packet
 *   \param internal_dev Pointer to internal device
 *   \param ptPacket Pointer to packet to handle
 *   \param ulTimeout timeout
 *   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t send_confirmation( NETX_ETH_DEV_T* internal_dev, CIFX_PACKET* ptPacket, uint32_t ulState, uint32_t ulTimeout) {
  int32_t ret = CIFX_NO_ERROR;

  if (ptPacket == NULL)
    return CIFX_NO_ERROR;

  if ((ptPacket->tHeader.ulCmd & CIFX_MSK_PACKET_ANSWER) == 0) {
    uint8_t bRetry = 3;

    ptPacket->tHeader.ulCmd   |= CIFX_MSK_PACKET_ANSWER;
    ptPacket->tHeader.ulState  = ulState;
    ptPacket->tHeader.ulLen    = 0;

    do {
      ret = xChannelPutPacket(internal_dev->cifx_channel, ptPacket, ulTimeout);
    } while((ret != CIFX_NO_ERROR) && (bRetry-- > 0));
    if ((ret != CIFX_NO_ERROR) && (g_ulTraceLevel & TRACE_LEVEL_ERROR)) {
      USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Failed to send confirmation - Error=0x%X (ulCmd=0x%X / ulState=0x%X)!\n",
                 ret,
                 ptPacket->tHeader.ulCmd,
                 ptPacket->tHeader.ulState);
    }
  }
  return ret;
}

/*****************************************************************************/
/*! handles incoming packets
 *   \param internal_dev Pointer to internal device
 *   \param ptPacket Pointer to packet to handle
 *   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
void handle_incoming_packet( NETX_ETH_DEV_T* internal_dev, CIFX_PACKET* ptPacket) {
  uint32_t ulState = 0;

  switch(ptPacket->tHeader.ulCmd) {
    case DRVETH_GCI_CMD_SEND_ETH_FRAME_CNF:
      /* Send response */
      OS_EnterLock( internal_dev->com_lock);
      internal_dev->active_sends--;
      OS_SetEvent(internal_dev->send_event);
      OS_LeaveLock( internal_dev->com_lock);

      if (ptPacket->tHeader.ulState != CIFX_NO_ERROR) {
        if(g_ulTraceLevel & TRACE_LEVEL_WARNING) {
          USER_Trace( internal_dev->devinst, TRACE_LEVEL_WARNING, "Ethernet-IF Error: Error signaled by confirmation packet (0x%X)\n", ptPacket->tHeader.ulState);
        }
      }
    break;

    case DRVETH_GCI_CMD_RECV_ETH_FRAME_IND:
    {
      if (internal_dev->link_up) {
        uint32_t data_len = ptPacket->tHeader.ulLen;
        ssize_t  send_res = data_len;
        int      ret = 0;

        /* New RX packet */
        if(send_res != (ret = write(internal_dev->eth_fd, ptPacket->abData, data_len))) {
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR) {
            USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Error sending incoming data to ethernet device (%d)\n", ret);
          }
        }
      }
    }
    break;

    case DRVETH_GCI_CMD_EVENT_IND:
    {
      /* ignore package since we poll the link state */
    }
    break;

    default:
    {
      ulState = ERR_HIL_UNKNOWN_COMMAND;
      if(g_ulTraceLevel & TRACE_LEVEL_INFO) {
        USER_Trace( internal_dev->devinst, TRACE_LEVEL_INFO, "Ethernet-IF Error: Error receiving unknown packet cmd=0x%X\n", ptPacket->tHeader.ulCmd);
      }
    }
    break;
  }
  send_confirmation( internal_dev, ptPacket, ulState, CIFX_TO_CONT_PACKET);
}

/*****************************************************************************/
/*! Receiver thread: processes eth packets from cifX to tapX device
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static void* cifx_to_eth_thread(void* arg)
{
  NETX_ETH_DEV_T* internal_dev = (NETX_ETH_DEV_T*)arg;
  CIFX_PACKET     cifx_packet;
  uint32_t        ulRecvPktCount   = 0;
  uint32_t        ulSendPktCount   = 0;
  time_t          last_update      = 0;
  int32_t         lRet;

  while(1)
  {
    if (internal_dev->stop_to_eth == 1)
      break;

    ulRecvPktCount = 1;
    while(ulRecvPktCount > 0) {
      if (CIFX_NO_ERROR == (lRet = xChannelGetPacket( internal_dev->cifx_channel, sizeof(cifx_packet), &cifx_packet, CIFX_TO_CONT_PACKET))) {
        handle_incoming_packet( internal_dev, &cifx_packet);
      }
      if (lRet == CIFX_DEV_GET_NO_PACKET) {
        /* in some case the firmware may deliver wrong mailbox state. */
        /* In case of false state, break here to update link state or */
        /* to be able to interrupt/stop the running thread.           */
        break;
      }
      if (CIFX_NO_ERROR != (lRet = xChannelGetMBXState( internal_dev->cifx_channel, &ulRecvPktCount, &ulSendPktCount))) {
        break;
      }
    }
    if (difftime( time(NULL), last_update) > LINK_STATE_POLL_INTERVAL) {
      cifxeth_update_link_state( internal_dev);
      last_update = time(NULL);
    }
  }
  return NULL;
}

/*! **************************************************************************
* Function retrieves the device link state and updates the corresponding
* tap device
*   \param internal_dev Pointer to internal device
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifxeth_update_link_state( NETX_ETH_DEV_T* internal_dev)
{
  LINK_STATE_T tLinkState = {0};
  int32_t      lRet       = CIFX_NO_ERROR;

  if (CIFX_NO_ERROR == (lRet = cifxeth_get_extended_info( internal_dev, EXT_INFO_LINKSTATE, &tLinkState, sizeof(tLinkState))))
  {
    int fSkipUpdate = 0;
    if ((internal_dev->link_up > 0) && (tLinkState.bLinkState)) {
      /* device is already online */
      fSkipUpdate = 1;
    }
    if ((internal_dev->link_up <= 0) && (tLinkState.bLinkState == 0)) {
      /* already offline... skip handling */
      fSkipUpdate = 1;
    }
    if (fSkipUpdate == 0) {
      if (tLinkState.bLinkState)
      {
        if(g_ulTraceLevel & TRACE_LEVEL_DEBUG) {
          USER_Trace( internal_dev->devinst, TRACE_LEVEL_DEBUG, "Link up on '%s'", internal_dev->cifxeth_name);
        }
        /* notify link state change */
        internal_dev->link_up = 1;
        OS_SetEvent( internal_dev->link_event);

        if (cifxeth_create_com_thread( internal_dev) != 0) {
          lRet = CIFX_FUNCTION_FAILED;
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Error creating cifX Ethernet communication thread for %s.",internal_dev->cifxeth_name);
          }
        } else {
          FILE *file = NULL;

          nl_signal_link_change( internal_dev, 1);

          /* notify link up via uevent */
          if (NULL != (file = fopen(internal_dev->event_path,"r+"))) {
            fprintf(file, "online");
            fclose(file);
          } else {
            lRet = CIFX_FUNCTION_FAILED;
            if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
            {
              USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Error opening event path of cifX Ethernet IF %s (online event).",internal_dev->cifxeth_name);
            }
          }
        }
      } else
      {
        FILE *file = NULL;
        internal_dev->link_up = 0;

        if(g_ulTraceLevel & TRACE_LEVEL_DEBUG) {
          USER_Trace( internal_dev->devinst, TRACE_LEVEL_DEBUG, "Link down on '%s'", internal_dev->cifxeth_name);
        }

        /* stop eth-if to cifx communication since we we will remove the handle */
        internal_dev->stop_to_cifx = 1;

        if (internal_dev->eth_to_cifx_thread != 0) {
          pthread_join( internal_dev->eth_to_cifx_thread, 0);
          internal_dev->eth_to_cifx_thread = 0;
        }

        nl_signal_link_change( internal_dev, 0);

        /* notify link down via uevent */
        if (NULL != (file = fopen(internal_dev->event_path,"r+"))) {
          fprintf(file, "offline");
          fclose(file);
        } else {
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Error opening event path of cifX Ethernet IF %s (offline event).",internal_dev->cifxeth_name);
          }
          lRet = CIFX_FUNCTION_FAILED;
        }
      }
    }
  }
  return lRet;
}

/*! **************************************************************************
* Function retrieves the device configuration and initialize the corresponding
* tap device
*   \param internal_dev Pointer to internal device
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifxeth_update_device_config( NETX_ETH_DEV_T* internal_dev)
{
  IFCONFIG_T tExtInfo = {0};
  int32_t    lRet     = CIFX_NO_ERROR;

  if (CIFX_NO_ERROR == (lRet = cifxeth_get_extended_info( internal_dev, EXT_INFO_INTF_CONFIG, &tExtInfo, sizeof(tExtInfo))))
  {
    struct ifreq ifr;
    memset( &ifr, 0, sizeof(ifr));

    memcpy( ifr.ifr_hwaddr.sa_data, tExtInfo.abEthernetMACAddr, 6);
    ifr.ifr_hwaddr.sa_family = 1;

    if( (ioctl( internal_dev->eth_fd, SIOCSIFHWADDR, (void *) &ifr)) < 0 )
    {
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Failed to set MAC address %02x:%02x:%02x:%02x:%02x:%02x of %s (%d)",
                    tExtInfo.abEthernetMACAddr[0], tExtInfo.abEthernetMACAddr[1], tExtInfo.abEthernetMACAddr[2],
                    tExtInfo.abEthernetMACAddr[3], tExtInfo.abEthernetMACAddr[4], tExtInfo.abEthernetMACAddr[5],
                    internal_dev->cifxeth_name, errno);
      }
    } else
    {
      if(g_ulTraceLevel & TRACE_LEVEL_DEBUG)
      {
        USER_Trace( internal_dev->devinst, TRACE_LEVEL_DEBUG, "Ethernet-IF: Successfully set MAC address to %02x:%02x:%02x:%02x:%02x:%02x on %s",
                    tExtInfo.abEthernetMACAddr[0], tExtInfo.abEthernetMACAddr[1], tExtInfo.abEthernetMACAddr[2],
                    tExtInfo.abEthernetMACAddr[3], tExtInfo.abEthernetMACAddr[4], tExtInfo.abEthernetMACAddr[5],
                    internal_dev->cifxeth_name);
      }
    }
  }
  return lRet;
}

/*! *************************************************************************
 * Receive callback function. requried to handle incoming packets during admin
 * communication
 *   \param ptRecvPkt
 *   \param pvUser                */
/****************************************************************************/
void PacketRecvCallBack( CIFX_PACKET* ptRecvPkt, void* pvUser)
{
  NETX_ETH_DEV_T* internal_dev = (NETX_ETH_DEV_T*)pvUser;

  handle_incoming_packet( internal_dev, ptRecvPkt);
}

/*! *************************************************************************
* Register an application, so the Ethernet Stack will send indications
*   \param internal_dev Pointer to internal device
*   \param fRegister    1= register, 0 = unregister  application            */
/****************************************************************************/
static int32_t cifxeth_register_app( NETX_ETH_DEV_T* internal_dev, int fRegister)
{
  uint32_t               lRet      = CIFX_NO_ERROR;
  CIFXHANDLE             hChannel  = internal_dev->cifx_channel;
  HIL_REGISTER_APP_REQ_T tSendPkt  = {{0}};
  CIFX_PACKET            tRecvPkt  = {{0}};

  tSendPkt.tHead.ulDest = HIL_PACKET_DEST_DEFAULT_CHANNEL; /* Destination of packet, process queue */
  tSendPkt.tHead.ulSrc  = 0;                               /* Source of packet, process queue      */
  tSendPkt.tHead.ulLen  = 0;
  tSendPkt.tHead.ulId   = 0x00;                            /* Identification handle of sender      */

  if (fRegister)
  {
    tSendPkt.tHead.ulCmd = HIL_REGISTER_APP_REQ;          /* Packet command                       */
  }else
  {
    tSendPkt.tHead.ulCmd = HIL_UNREGISTER_APP_REQ;        /* Packet command                       */
  }

  lRet = DEV_TransferPacket( hChannel,
                             (CIFX_PACKET*)&tSendPkt,
                             &tRecvPkt,
                             sizeof(tRecvPkt),
                             CIFX_TO_SEND_PACKET,
                             PacketRecvCallBack, internal_dev);
  if( CIFX_NO_ERROR != lRet)
  {
    /* This is a transport error */
    if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
    {
      /* This is a transport error */
      USER_Trace(internal_dev->devinst,
                TRACE_LEVEL_ERROR,
                 "Ethernet-IF Error: Error in cifXEthTransferPacket()(lRet=0x%08X).",
                lRet);
    }
  } else
  {
    /* in case of de-register ignore return value since we might not be registered */
    if (fRegister) {
      /* Check if we have a state error from the stack */
      if(SUCCESS_HIL_OK != (lRet = tRecvPkt.tHeader.ulState)) {
        if(g_ulTraceLevel & TRACE_LEVEL_ERROR) {
          USER_Trace(internal_dev->devinst,
                     TRACE_LEVEL_ERROR,
                     "Ethernet-IF Error: Error sending Register-Application-Request (lRet=0x%08X).",
                     lRet);
        }
      }
    }
  }
  return lRet;
}

/*! **************************************************************************
* Function reads extended status block an returns requested information
*   \param internal_dev Pointer to internal device
*   \param ulInformationRequest Command which information is retrieved
*   \param pvBuffer             Pointer to information buffer
*   \param ulBufLen             Size of pvBuffer
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
static int32_t cifxeth_get_extended_info( NETX_ETH_DEV_T* internal_dev, uint32_t ulInformationRequest, void* pvBuffer, uint32_t ulBufLen)
{
  int32_t                     lRet     = CIFX_NO_ERROR;
  CIFXHANDLE                  hChannel = internal_dev->cifx_channel;
  DRVETH_GCI_EXTENDED_STATE_T tExtStatusInfo;

  /* check parameter */
  if ((pvBuffer == NULL) || (ulBufLen == 0))
    return CIFX_INVALID_PARAMETER;

  lRet = xChannelExtendedStatusBlock( hChannel, CIFX_CMD_READ_DATA, 0, sizeof(tExtStatusInfo), &tExtStatusInfo);
  if( CIFX_NO_ERROR == lRet)
  {
    /* check which information is requested */
    switch (ulInformationRequest)
    {
      /* return MAC address */
      case EXT_INFO_INTF_CONFIG:
      {
        if ( sizeof(IFCONFIG_T) != ulBufLen) {
          lRet = CIFX_INVALID_BUFFERSIZE;
        } else {
          PIFCONFIG_T ptIPconfig = (PIFCONFIG_T)pvBuffer;

          OS_Memcpy( (void*)ptIPconfig->abEthernetMACAddr, (void*)tExtStatusInfo.abMacAddress, DRVETH_GCI_ETH_ADDR_SIZE);

          lRet = CIFX_NO_ERROR;
        }
      }
      break;

      /* return Link State */
      case EXT_INFO_LINKSTATE:
      {
        if ( sizeof(LINK_STATE_T) != ulBufLen) {
          lRet = CIFX_INVALID_BUFFERSIZE;
        } else {
          PLINK_STATE_T ptLinkState = (PLINK_STATE_T)pvBuffer;

          ptLinkState->bLinkState = tExtStatusInfo.bMautype;

          lRet = CIFX_NO_ERROR;
        }
      }
      break;

      case EXT_STATISTICS:
      {
        if ( sizeof(STATISTIC_T) != ulBufLen) {
          lRet = CIFX_INVALID_BUFFERSIZE;
        } else {
          STATISTIC_T* ptStatistic      = (STATISTIC_T*)pvBuffer;

          ptStatistic->ullIfInPkts      = tExtStatusInfo.ullIfInPkts;
          ptStatistic->ullIfInDiscards  = tExtStatusInfo.ullIfInDiscards;
          ptStatistic->ullIfOutPkts     = tExtStatusInfo.ullIfOutPkts;
          ptStatistic->ullIfOutDiscards = tExtStatusInfo.ullIfOutDiscards;
          ptStatistic->ullIfInBytes     = tExtStatusInfo.ullIfInBytes;
          ptStatistic->ullIfOutBytes    = tExtStatusInfo.ullIfOutBytes;

          lRet = CIFX_NO_ERROR;
        }
      }
      break;

      case EXT_INFO_MACADDR:
      case EXT_INFO_IPADDR:
      case EXT_INFO_NETMASK:
      case EXT_INFO_GATEWAY:
      case EXT_INFO_NO_RECVPKT:
      case EXT_INFO_NO_RCVPKT_DROP:
      case EXT_INFO_NO_SENDPKT:
      case EXT_INFO_NO_SENDPKT_DROP:
      default:
        lRet = CIFX_INVALID_COMMAND;
      break;
    }
  }
  if (CIFX_NO_ERROR != lRet) {
    if(g_ulTraceLevel & TRACE_LEVEL_ERROR) {
      USER_Trace( internal_dev->devinst, TRACE_LEVEL_ERROR, "Ethernet-IF Error: Failed to retrieve extended info of %s (0x%X)", internal_dev->cifxeth_name, lRet);
    }
  }
  return lRet;
}

/*****************************************************************************/
/*! Searchs for internal device structure of the device given by name
*   \param name Name of the requested cifX device
*   \return pointer to internal device (!= NULL on success)                  */
/*****************************************************************************/
static NETX_ETH_DEV_T* find_device(char* name)
{
  NETX_ETH_DEV_T* internal_dev = NULL;

  OS_EnterLock( g_eth_list_lock);
  if (!TAILQ_EMPTY( &s_DeviceList))
  {
    NETX_ETH_DEV_T* item;
    TAILQ_FOREACH( item, &s_DeviceList, lentry) {
      if (0 == strcmp( item->config.cifx_name, name))
      {
        internal_dev = item;
        break;
      }
    }
  }
  OS_LeaveLock( g_eth_list_lock);

  return internal_dev;
}

/*****************************************************************************/
/*! NOTE: xSysdeviceReset() is a cifx toolkit function. In case of an ethernet
 * interface a function wrapper is required since xSysdeviceReset() will reset
 * the whole device including the raw ethernet channel. So we need a wrap
 * around to remove all previously registered ethernet interfaces.           */
/*****************************************************************************/
extern int32_t APIENTRY xSysdeviceResetTK( CIFXHANDLE hSysdevice, uint32_t ulTimeout);
/*****************************************************************************/
/*! Hard resets a complete device via system channel
*   \param hSysdevice Handle to system device
*   \param ulTimeout  Timeout to wait for card to finish reset
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xSysdeviceReset( CIFXHANDLE hSysdevice, uint32_t ulTimeout)
{
  int32_t            ret          = CIFX_NO_ERROR;
  PCHANNELINSTANCE   ptdevice     = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE    ptDevInst    = (PDEVICEINSTANCE)ptdevice->pvDeviceInstance;
  NETX_ETH_DEV_T*    internal_dev = find_device( ptDevInst->szName);
  NETX_ETH_DEV_CFG_T config       = {{0}};
  int                ethdevice    = 0;

  if (NULL != internal_dev)
  {
    ethdevice = 1;
    strcpy( config.cifx_name, internal_dev->config.cifx_name);
    cifxeth_delete_device( internal_dev);
  }

  if (CIFX_NO_ERROR == (ret = xSysdeviceResetTK( hSysdevice, ulTimeout)))
  {
    if (1 == ethdevice)
      cifxeth_create_device( &config);
  }
  return ret;
}

extern int32_t APIENTRY xSysdeviceResetExTK(CIFXHANDLE hSysdevice, uint32_t ulTimeout, uint32_t ulMode);
/*****************************************************************************/
/*! Hard resets a complete device via system channel with reset parameter
*   \param hSysdevice Handle to system device
*   \param ulTimeout  Timeout to wait for card to finish reset
*   \param ulMode     Reset mode with parameter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xSysdeviceResetEx(CIFXHANDLE hSysdevice, uint32_t ulTimeout, uint32_t ulMode)
{
  int32_t            ret          = CIFX_NO_ERROR;
  PCHANNELINSTANCE   ptdevice     = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE    ptDevInst    = (PDEVICEINSTANCE)ptdevice->pvDeviceInstance;
  NETX_ETH_DEV_T*    internal_dev = find_device( ptDevInst->szName);
  NETX_ETH_DEV_CFG_T config       = {{0}};
  int                ethdevice    = 0;

  switch(ulMode)
  {
    case CIFX_RESETEX_SYSTEMSTART:
    case CIFX_RESETEX_BOOTSTART:
    case CIFX_RESETEX_UPDATESTART:
    {
      if (NULL != internal_dev)
      {
        ethdevice = 1;
        strcpy( config.cifx_name, internal_dev->config.cifx_name);
        cifxeth_delete_device( internal_dev);
      }
      if (CIFX_NO_ERROR == (ret = xSysdeviceResetExTK( hSysdevice, ulTimeout, ulMode)))
      {
        if ((1 == ethdevice) && (ulMode != CIFX_RESETEX_BOOTSTART))
          cifxeth_create_device( &config);
      }
    }
    break;
    default:
    {
      ret = CIFX_INVALID_PARAMETER;
    }
    break;
  }
  return ret;
}

extern int32_t APIENTRY xSysdeviceBootstartTK(CIFXHANDLE hSysdevice, uint32_t ulTimeout);
/*****************************************************************************/
/*! Boot start reset to via system channel
*   \param hSysdevice Handle to system device
*   \param ulTimeout  Timeout to wait for card to finish reset
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xSysdeviceBootstart(CIFXHANDLE hSysdevice, uint32_t ulTimeout)
{
  int32_t            ret          = CIFX_NO_ERROR;
  PCHANNELINSTANCE   ptdevice     = (PCHANNELINSTANCE)hSysdevice;
  PDEVICEINSTANCE    ptDevInst    = (PDEVICEINSTANCE)ptdevice->pvDeviceInstance;
  NETX_ETH_DEV_T*    internal_dev = find_device( ptDevInst->szName);
  NETX_ETH_DEV_CFG_T config       = {{0}};

  if (NULL != internal_dev)
  {
    strcpy( config.cifx_name, internal_dev->config.cifx_name);
    cifxeth_delete_device( internal_dev);
  }
  return xSysdeviceBootstartTK( hSysdevice, ulTimeout);
}

/*****************************************************************************/
/*! NOTE: xChannelReset() is a cifx toolkit function. In case of an ethernet
 * interface a function wrapper is required since xChannelReset() (with mode set
 * to CIFX_SYSTEMSTART) will reset the whole device including the raw ethernet
 * channel. So we need a wrap around to remove all previously registered ethernet
 * interfaces.                                                               */
/*****************************************************************************/
extern int32_t APIENTRY xChannelResetTK( CIFXHANDLE hChannel, uint32_t ulResetMode, uint32_t ulTimeout);
/*****************************************************************************/
/*! Hard resets a complete device via system channel
*   \param hChannel Handle to system device
*   \param ulResetMode Reset Mode
*   \param ulTimeout  Timeout to wait for card to finish reset
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xChannelReset(CIFXHANDLE  hChannel, uint32_t ulResetMode, uint32_t ulTimeout)
{
  int32_t            ret          = CIFX_NO_ERROR;
  PCHANNELINSTANCE   ptdevice     = (PCHANNELINSTANCE)hChannel;
  PDEVICEINSTANCE    ptDevInst    = (PDEVICEINSTANCE)ptdevice->pvDeviceInstance;
  NETX_ETH_DEV_T*    internal_dev = find_device( ptDevInst->szName);
  NETX_ETH_DEV_CFG_T config       = {{0}};
  int                ethdevice    = 0;

  switch(ulResetMode)
  {
    case CIFX_SYSTEMSTART:
    {
      if (NULL != internal_dev)
      {
        ethdevice = 1;
        strcpy( config.cifx_name, internal_dev->config.cifx_name);
        cifxeth_delete_device( internal_dev);
      }

      if (CIFX_NO_ERROR == (ret = xChannelResetTK( hChannel, ulResetMode, ulTimeout)))
      {
        if (1 == ethdevice)
          cifxeth_create_device( &config);
      }
    }
    break;
    default:
    {
      ret = xChannelResetTK( hChannel, ulResetMode, ulTimeout);
    }
    break;
  }
  return ret;
}

#endif //CIFXETHERNET
