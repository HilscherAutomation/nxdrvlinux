// SPDX-License-Identifier: MIT
/**************************************************************************************
 *
 * Copyright (c) 2024, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 * Description: marshaller server handling
 *
 * Changes:
 *
 *   Version   Date        Author   Description
 *   ----------------------------------------------------------------------------------
 *   1        02.01.24    SD        changed licensing terms
 *
 **************************************************************************************/

#define _XOPEN_SOURCE 700

#include <ifaddrs.h>
#include <pthread.h>
#include <stdio.h>
#include <strings.h>
#include <errno.h>
#include <sys/stat.h>

#include "tcp_connector.h"
#include "tcp_server.h"
#include "cifxlinux.h"
#include "cifx_download_hook.h"
#include "MarshallerErrors.h"

/* Driver/Config base directory */
extern char* g_szDriverBaseDir;

/* marshaller handle */
void*             g_pvMarshaller = NULL;
/* flag for main loop */
int               g_fRunning      = 0;
/* flag to display once the network traffic */
int               g_fTrafficOnce  = 0;

unsigned short    g_usPortNumber = HIL_TRANSPORT_IP_PORT;

struct CIFX_LINUX_INIT  g_tInit = {0};

pthread_mutex_t*  g_ptMutex = NULL;

typedef struct INIT_PARAM_Ttag
{
  int fUseSingleCard;
  int  iCardNumber;
  int  iPortNumber;

} INIT_PARAM_T;

INIT_PARAM_T g_tInitParam = {0};

/* use the functions of the linked library libcifx */
void OS_DeleteLock(void* pvLock);
void* OS_CreateLock(void);

/*****************************************************************************/
/*! Function to demonstrate the board/channel enumeration
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t EnumBoardDemo(void)
{
  CIFXHANDLE hDriver = NULL;
  long   lRet    = xDriverOpen(&hDriver);

  printf("---------- Available Cards ----------\r\n");

  if(CIFX_NO_ERROR == lRet)
  {
    /* Driver/Toolkit successfully opened */
    unsigned long     ulBoard    = 0;
    BOARD_INFORMATION tBoardInfo = {0};

    /* Iterate over all boards */
    while(CIFX_NO_ERROR == xDriverEnumBoards(hDriver, ulBoard, sizeof(tBoardInfo), &tBoardInfo))
    {
      printf("%d.: %s\r\n", tBoardInfo.ulBoardID +1, tBoardInfo.abBoardName);
      if(strlen( (char*)tBoardInfo.abBoardAlias) != 0)
        printf("    Alias        : %s\r\n", tBoardInfo.abBoardAlias);

      printf("    DeviceNumber : %lu\r\n",(long unsigned int)tBoardInfo.tSystemInfo.ulDeviceNumber);
      printf("    SerialNumber : %lu\r\n",(long unsigned int)tBoardInfo.tSystemInfo.ulSerialNumber);

      unsigned long       ulChannel    = 0;
      CHANNEL_INFORMATION tChannelInfo = {{0}};

      /* iterate over all channels on the current board */
      while(CIFX_NO_ERROR == xDriverEnumChannels(hDriver, ulBoard, ulChannel, sizeof(tChannelInfo), &tChannelInfo))
      {
        printf("    - Channel %lu:\r\n", ulChannel);
        printf("      Firmware : %s\r\n", tChannelInfo.abFWName);
        printf("      Version  : %u.%u.%u build %u\r\n",
               tChannelInfo.usFWMajor,
               tChannelInfo.usFWMinor,
               tChannelInfo.usFWRevision,
               tChannelInfo.usFWBuild);

        ++ulChannel;
      }

      ++ulBoard;
      printf("----------------------------------------------------\r\n");
    }

    /* close previously opened driver */
    xDriverClose(hDriver);
  }

  return lRet;
}

/*****************************************************************************/
/*! Function display IP of the available network adapter                     */
/*****************************************************************************/
void DisplayIP(void)
{
  struct ifaddrs * ifAddrStruct=NULL;
  struct ifaddrs * ifa=NULL;
  void * tmpAddrPtr=NULL;

  getifaddrs(&ifAddrStruct);

  if (ifAddrStruct != NULL) {
    printf("Interface Name\t: IP\n");
    printf("-------------------------------\n");
  }

  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
  {
    /* if IP4 */
    if ( (ifa->ifa_addr != NULL) && (ifa->ifa_addr->sa_family == AF_INET) )
    {
      if (OS_Strnicmp("lo", ifa->ifa_name, strlen(ifa->ifa_name)))
      {
        tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
        char addressBuffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

        printf("%-16s: %s\n", ifa->ifa_name, addressBuffer);
      }
    }
  }

  printf("\n");

  if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
}


/*****************************************************************************/
/*! Function display optional arguments                                      */
/*****************************************************************************/
void DisplayHelp(void)
{
  printf("The cifx_tcpserver application is a demo application for remote access of a cifX device under Linux!");
  printf("The application provides a file transfer mechanism, to be able to remotely configure a device (e.g. via Communication Studio/SYCON.net)\n");
  printf("For custom purposes the application needs to be adapted!\n");
  printf("Available options:\n");
  printf("[-n <n>] initialize only a specific card specified by 'n'.\n");
  printf("[-p <n>] use port number specified by 'n'.\n");
  printf("[-d] display IP adress of the active adapter and return.\n");
  printf("[-a] display available cards and return.\n");
  printf("[-h] display this help.\n");

  printf("Example:\n");
  printf("cifXTCPServer -n 0 -p 51234\n");
}


/*****************************************************************************/
/*! Function evaluates input arguments                                       */
/*****************************************************************************/
int ValidateArgs(int argc, char* argv[])
{
  int fRet    = 1;
  int iArgCnt = 0;

  if (argc > 1)
  {
    /* ignore if help is not first param */
    if (0 == strcasecmp("-h", argv[1]))
    {
      DisplayHelp();
      return 0;
    }

    /* iterate over options check only -d & -a  */
    for (iArgCnt = 1; iArgCnt<argc; iArgCnt++)
    {
      if (0 == strcasecmp("-d", argv[iArgCnt]))
      {
        /* only display the active adapter and its IP */
        fRet = 0;

        /* display ip of the active adapter */
        DisplayIP();

      }else if (0 == strcasecmp("-a", argv[iArgCnt]))
      {
        /* only display all available cards */
        fRet = 0;

        /* struct to initialize toolkit */
        g_tInit.init_options        = CIFX_DRIVER_INIT_AUTOSCAN;
        g_tInit.iCardNumber         = 0;
        g_tInit.fEnableCardLocking  = 0;
        g_tInit.base_dir            = NULL;
        g_tInit.poll_interval       = 0;
        g_tInit.poll_StackSize      = 0;
        g_tInit.trace_level         = 255;
        g_tInit.user_card_cnt       = 0;

        /* First of all initialize toolkit */
        if (CIFX_NO_ERROR == cifXDriverInit(&g_tInit))
        {
          /* display ip of the active adapter */
          EnumBoardDemo();
        }
      }
    }
    /* if -d or -a option is used return */
    if (fRet == 0)
      return fRet;

    /* check options */
    for (iArgCnt = 1; ((iArgCnt<argc) && (fRet == 1)); iArgCnt++)
    {
      if (0 == strcasecmp( "-n", argv[iArgCnt]))
      {
        g_tInitParam.fUseSingleCard = 1;
        iArgCnt++;
        if ((iArgCnt) < argc)
        {
          g_tInitParam.iCardNumber = atoi( argv[iArgCnt]);
          printf("Select card: cifX%d!\n", g_tInitParam.iCardNumber);

        } else
        {
          fRet = 0;
        }
      }else if (0 == strcasecmp("-p", argv[iArgCnt]))
      {
        iArgCnt++;
        if ((iArgCnt) < argc)
        {
          g_usPortNumber = atoi( argv[iArgCnt]);
          printf("Use port number: %d!\n", g_usPortNumber);

        } else
        {
          fRet = 0;
        }
      }else if ((0 == strcasecmp("-d", argv[iArgCnt])) || (0 == strcasecmp("-a", argv[iArgCnt])) || (0 == strcasecmp("-h", argv[iArgCnt])))
      {
        /* already done */
      }else
      {
        fRet = 0;
      }
    }
    if (0 == fRet)
    {
      printf("Invalid argument!\n");
      DisplayHelp();
    }
  }

  return fRet;
}

/*****************************************************************************/
/*! Timer refreshing TCP traffic                                             */
/*****************************************************************************/
void TrafficTimer(void* dwUser)
{
  TCP_CONN_INTERNAL_T* ptTcpData = (TCP_CONN_INTERNAL_T*)dwUser;
  g_fTrafficOnce = 1;

  /* insert a handling to write down or display the traffic */

  printf("\nRX[Bytes]: %lu\nTX[Bytes]: %lu\n", ptTcpData->ulRxCount, ptTcpData->ulTxCount);
}

/*****************************************************************************/
/*! This function is called after each interval specified in the setitimer
 *  function (see InitMarshaller) used to install a timer.
 *    \param iSignal   Signal which caused the call                          */
/*****************************************************************************/
void MarshallerTimer(int iSignal)
{
  HilMarshallerTimer(g_pvMarshaller);
}

/*****************************************************************************/
/*! Wrapper for xSysdeviceOpen to track sysdevice access
*   \param hDriver      Driver handle
*   \param szBoard      Name of the board to open
*   \param phSysdevice  Returned handle to the System device area
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xSysdeviceOpenWrap ( CIFXHANDLE  hDriver, char*   szBoard, CIFXHANDLE* phSysdevice)
{
  int32_t lRet  = CIFX_NO_ERROR;

  if (CIFX_NO_ERROR == (lRet = xSysdeviceOpen( hDriver, szBoard, phSysdevice)))
  {
    /* add here additional code for print out or display handling */

    printf("Open system device \"%s\" (handle %p)\n", szBoard, *phSysdevice);

  }

  return lRet;
}

/*****************************************************************************/
/*! Wrapper for xSysdeviceClose to track sysdevice access
*   \param hSysdevice  Handle to the System device to close
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xSysdeviceCloseWrap ( CIFXHANDLE  hSysdevice)
{
  int32_t lRet  = CIFX_NO_ERROR;

  if (CIFX_NO_ERROR == (lRet = xSysdeviceClose( hSysdevice)))
  {
    /* add here additional code for print out or display handling */
    printf("Close system device (handle %p)\n", hSysdevice);
  }

  return lRet;
}

/*****************************************************************************/
/*! Wrapper for xChannelOpen to track channel access
*   \param hDriver    Driver handle
*   \param szBoard    DOS Device Name of the Board to open
*   \param ulChannel  Channel number to open (0..n)
*   \param phChannel  Returned handle to the channel (Needed for all channel
*                     specific operations)
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xChannelOpenWrap  ( CIFXHANDLE  hDriver,  char* szBoard, uint32_t ulChannel, CIFXHANDLE* phChannel)
{
  int32_t lRet  = CIFX_NO_ERROR;

  if (CIFX_NO_ERROR == (lRet = xChannelOpen( hDriver, szBoard, ulChannel, phChannel)))
  {
    /* add here additional code for print out or display handling */
    printf("Open channel %d of device \"%s\" (handle %p)\n", ulChannel, szBoard, *phChannel);

  }

  return lRet;
}

/*****************************************************************************/
/*! Wrapper for xChannelClose to track channel access
*   \param hChannel  Handle to the channel device to close
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int32_t APIENTRY xChannelCloseWrap ( CIFXHANDLE  hChannel)
{
  int32_t lRet  = CIFX_NO_ERROR;

  if (CIFX_NO_ERROR == (lRet = xChannelClose(hChannel)))
  {
    /* add here additional code for print out or display handling */

    printf("Close channel (handle %p)\n", hChannel);
  }

  return lRet;
}

/*****************************************************************************/
/*! Function for Marshaller Request                                          */
/*****************************************************************************/
void MarshallerRequest(void* pvMarshaller, void* pvUser)
{
  UNREFERENCED_PARAMETER(pvUser);
  HilMarshallerMain(pvMarshaller);
}

/*****************************************************************************/
/* Destroy marshallar and deinit driver
 * if SIGINT (ctrl +c)                                                       */
/*****************************************************************************/
void DeInitServer(int iSignal)
{
  DeinitMarshaller();
  cifXDriverDeinit();

  OS_DeleteLock(g_ptMutex);
  g_fRunning = 0;
}

/*****************************************************************************/
/*! Checks if the given path exists in the file system.
*     \param  szPath  Pointer to path                                        */
/*****************************************************************************/
static int path_exists(char* szPath)
{
  struct stat s;
  return stat(szPath, &s);
}

/*****************************************************************************/
/*! Helper function returning the path to a channel directory
*   on the given device (e.g. /opt/cifx/deviceconfig/1250100/20004/channel0/).
*   This function exists in the USER_Linux.c almost exactly like this one.
*     \param szPath         Pointer to returned path
*     \param iPathLen       Length of the buffer passed in szPath
*     \param ulChannel      Channel number
*     \param ptDevInfo      Device information (DeviceNr, SerialNr, ChannelNr)*/
/*****************************************************************************/
static void GetChannelDir(char* szPath, size_t iPathLen, uint32_t ulChannel, BOARD_INFORMATION* ptDevInfo)
{
  uint32_t ulSlotNr = ptDevInfo->tSystemInfo.bDevIdNumber;

  /* If the rotary switch is set != 0 */
  if (ulSlotNr)
  {
    snprintf(szPath, iPathLen,
            "%s/deviceconfig/Slot_%d/channel%d/",
            g_szDriverBaseDir,
            (unsigned int)ulSlotNr,
            (unsigned int)ulChannel);
    if (path_exists(szPath)==0)
      return;
  }

  /* Check if a path with <device-nr>/<serial-nr> exists */
  snprintf(szPath, iPathLen,
           "%s/deviceconfig/%d/%d/channel%d/",
           g_szDriverBaseDir,
           (unsigned int)ptDevInfo->tSystemInfo.ulDeviceNumber,
           (unsigned int)ptDevInfo->tSystemInfo.ulSerialNumber,
           (unsigned int)ulChannel);

  if (path_exists(szPath)==0)
    return;

  /* Check if a path with the <board-name> exists */
  snprintf(szPath, iPathLen,
           "%s/deviceconfig/%s/channel%d/",
           g_szDriverBaseDir,
           ptDevInfo->abBoardName,
           (unsigned int)ulChannel);

  if (path_exists(szPath)==0)
    return;

  /* Check if a single-directory path exists */
  snprintf(szPath, iPathLen,
           "%s/deviceconfig/FW/channel%d/",
           g_szDriverBaseDir,
           (unsigned int)ulChannel);

  return;
}

/*****************************************************************************/
/*! File storage callback (uses NX-API)                                      */
/*****************************************************************************/
static int32_t HandleFileStorage(BOARD_INFORMATION* ptBoardInfo,
                                 char* pszFileName, uint32_t ulFileSize,
                                 uint8_t* pabFileData, uint32_t ulChannel,
                                 uint32_t ulDownloadMode, void* pvUser)
{
  int32_t lRet = CIFX_FUNCTION_FAILED;
  char    abFileName[FILENAME_MAX];
  FILE*   iFd;

  GetChannelDir(abFileName, FILENAME_MAX, ulChannel, ptBoardInfo);
  strcat(abFileName, pszFileName);

  printf("Store file: %s\n", abFileName);
  if ( NULL != (iFd = fopen( abFileName, "w+")))
  {
    int iFileSize;
    if ( 0 < (iFileSize = fwrite( pabFileData, 1, ulFileSize, iFd)))
    {
      if ((((unsigned int)iFileSize) == ulFileSize))
        lRet = CIFX_NO_ERROR;
    } else
    {
      printf("File storing failed %d\n", iFileSize);
    }
    fclose( iFd);
  } else
  {
    printf("File open failed (error=%d)!\n",errno);
    printf("Please create directory in case path does not exist!\n");
    lRet = CIFX_FILE_OPEN_FAILED;
  }
  if (( DOWNLOAD_MODE_FIRMWARE == ulDownloadMode) && (lRet == CIFX_NO_ERROR))
  {
    /* if download succeeded restart device */
    xDriverRestartDevice ( NULL, ptBoardInfo->abBoardName, NULL);
  }

  return lRet;
}

/*****************************************************************************/
/*! Initialization the Marshallar                                         */
/*****************************************************************************/
uint32_t InitMarshaller(void)
{
  HIL_MARSHALLER_PARAMS_T           tParams        = {{0}};
  HIL_MARSHALLER_CONNECTOR_PARAMS_T tTCPConnector  = {0};

  tTCPConnector.pfnConnectorInit = TCPConnectorInit;
  tTCPConnector.pvConfigData     = NULL;
  tTCPConnector.ulDataBufferCnt  = 1;
  tTCPConnector.ulDataBufferSize = 6000;
  tTCPConnector.ulTimeout        = 1000;

  TRANSPORT_LAYER_CONFIG_T          tCifXTransport = {0};
  CIFX_TRANSPORT_CONFIG             tCifXConfig    = {{0}};

  tCifXConfig.tDRVFunctions.pfnxDriverOpen                 = xDriverOpen;
  tCifXConfig.tDRVFunctions.pfnxDriverClose                = xDriverClose;
  tCifXConfig.tDRVFunctions.pfnxDriverGetInformation       = xDriverGetInformation;
  tCifXConfig.tDRVFunctions.pfnxDriverGetErrorDescription  = xDriverGetErrorDescription;
  tCifXConfig.tDRVFunctions.pfnxDriverEnumBoards           = xDriverEnumBoards;
  tCifXConfig.tDRVFunctions.pfnxDriverEnumChannels         = xDriverEnumChannels;
  tCifXConfig.tDRVFunctions.pfnxDriverMemoryPointer        = xDriverMemoryPointer;
  tCifXConfig.tDRVFunctions.pfnxSysdeviceOpen              = xSysdeviceOpenWrap;
  tCifXConfig.tDRVFunctions.pfnxSysdeviceClose             = xSysdeviceCloseWrap;
  tCifXConfig.tDRVFunctions.pfnxSysdeviceReset             = xSysdeviceReset;
  tCifXConfig.tDRVFunctions.pfnxSysdeviceResetEx           = xSysdeviceResetEx;
  tCifXConfig.tDRVFunctions.pfnxSysdeviceGetMBXState       = xSysdeviceGetMBXState;
  tCifXConfig.tDRVFunctions.pfnxSysdevicePutPacket         = xSysdevicePutPacket;
  tCifXConfig.tDRVFunctions.pfnxSysdeviceGetPacket         = xSysdeviceGetPacket;
  tCifXConfig.tDRVFunctions.pfnxSysdeviceDownload          = xSysdeviceDownload;
  tCifXConfig.tDRVFunctions.pfnxSysdeviceInfo              = xSysdeviceInfo;
  tCifXConfig.tDRVFunctions.pfnxSysdeviceFindFirstFile     = xSysdeviceFindFirstFile;
  tCifXConfig.tDRVFunctions.pfnxSysdeviceFindNextFile      = xSysdeviceFindNextFile;
  tCifXConfig.tDRVFunctions.pfnxChannelOpen                = xChannelOpenWrap;
  tCifXConfig.tDRVFunctions.pfnxChannelClose               = xChannelCloseWrap;
  tCifXConfig.tDRVFunctions.pfnxChannelDownload            = xChannelDownload;
  tCifXConfig.tDRVFunctions.pfnxChannelGetMBXState         = xChannelGetMBXState;
  tCifXConfig.tDRVFunctions.pfnxChannelPutPacket           = xChannelPutPacket;
  tCifXConfig.tDRVFunctions.pfnxChannelGetPacket           = xChannelGetPacket;
  tCifXConfig.tDRVFunctions.pfnxChannelGetSendPacket       = xChannelGetSendPacket;
  tCifXConfig.tDRVFunctions.pfnxChannelConfigLock          = xChannelConfigLock;
  tCifXConfig.tDRVFunctions.pfnxChannelReset               = xChannelReset;
  tCifXConfig.tDRVFunctions.pfnxChannelInfo                = xChannelInfo;
  tCifXConfig.tDRVFunctions.pfnxChannelWatchdog            = xChannelWatchdog;
  tCifXConfig.tDRVFunctions.pfnxChannelHostState           = xChannelHostState;
  tCifXConfig.tDRVFunctions.pfnxChannelBusState            = xChannelBusState;
  tCifXConfig.tDRVFunctions.pfnxChannelIORead              = xChannelIORead;
  tCifXConfig.tDRVFunctions.pfnxChannelIOWrite             = xChannelIOWrite;
  tCifXConfig.tDRVFunctions.pfnxChannelIOReadSendData      = xChannelIOReadSendData;
  tCifXConfig.tDRVFunctions.pfnxChannelControlBlock        = xChannelControlBlock;
  tCifXConfig.tDRVFunctions.pfnxChannelCommonStatusBlock   = xChannelCommonStatusBlock;
  tCifXConfig.tDRVFunctions.pfnxChannelExtendedStatusBlock = xChannelExtendedStatusBlock;
  tCifXConfig.tDRVFunctions.pfnxChannelPLCMemoryPtr        = xChannelPLCMemoryPtr;
  tCifXConfig.tDRVFunctions.pfnxChannelPLCIsReadReady      = xChannelPLCIsReadReady;
  tCifXConfig.tDRVFunctions.pfnxChannelPLCIsWriteReady     = xChannelPLCIsWriteReady;
  tCifXConfig.tDRVFunctions.pfnxChannelPLCActivateWrite    = xChannelPLCActivateWrite;
  tCifXConfig.tDRVFunctions.pfnxChannelPLCActivateRead     = xChannelPLCActivateRead;
  tCifXConfig.tDRVFunctions.pfnxChannelFindFirstFile       = xChannelFindFirstFile;
  tCifXConfig.tDRVFunctions.pfnxChannelFindNextFile        = xChannelFindNextFile;

  /* Install download hook */
  xDownloadHook_Install(&tCifXConfig.tDRVFunctions, HandleFileStorage, &g_tInit);

  tCifXTransport.pfnInit  = cifXTransportInit;
  tCifXTransport.pvConfig = &tCifXConfig;

  tParams.ulMaxConnectors = 1;
  tParams.atTransports    = &tCifXTransport;
  tParams.ulTransportCnt  = 1;

  tParams.ptConnectors    = &tTCPConnector;
  tParams.ulConnectorCnt  = 1;

  uint32_t eRet = HilMarshallerStart(&tParams, &g_pvMarshaller, MarshallerRequest, 0);

  if ( MARSHALLER_NO_ERROR == eRet)
  {
    /* interval = 10ms */
    struct itimerval pITimerVal;
    pITimerVal.it_value.tv_sec     = 0;
    pITimerVal.it_value.tv_usec    = 10000;
    pITimerVal.it_interval.tv_sec  = 0;
    pITimerVal.it_interval.tv_usec = 10000;

    setitimer(ITIMER_REAL, &pITimerVal,NULL);

    struct sigaction tSigTimer = {.sa_handler = MarshallerTimer};

    sigemptyset( &tSigTimer.sa_mask);

    /* timer event for timeout */
    tSigTimer.sa_flags = 0;

    sigaction(SIGALRM, &tSigTimer, NULL);
  }

  return eRet;
}

/*****************************************************************************/
/* Destroy the Marshallar */
/*****************************************************************************/
void DeinitMarshaller()
{
  setitimer(ITIMER_REAL,NULL,NULL);

  if(NULL != g_pvMarshaller)
  {
    printf("\nWaiting for all process to end...\n");
    HilMarshallerStop(g_pvMarshaller);
  }
}

/*****************************************************************************/
/* Displays information of the server (cards under control and IP)           */
/*****************************************************************************/
void PrintServerInformation(void)
{
  CIFXHANDLE hDriver = NULL;
  int32_t    lRet    = xDriverOpen(&hDriver);

  printf("\n******************************************************************************\n");

  if(CIFX_NO_ERROR != lRet)
  {
    printf("\nFAILED to open CIFX driver, error: 0x%.8X\n", lRet);
  }else
  {
    /* Driver/Toolkit successfully opened */
    unsigned long     ulBoard    = 0;
    unsigned long     fFound     = 0;
    BOARD_INFORMATION tBoardInfo = {0};

    printf("The following card(s) are now accessible via TCP/IP remote connection:\n");

    /* Iterate over all boards */
    while(CIFX_NO_ERROR == xDriverEnumBoards(hDriver, ulBoard, sizeof(tBoardInfo), &tBoardInfo))
    {
      printf("-> %d.: %s\r\n", tBoardInfo.ulBoardID +1, tBoardInfo.abBoardName);
      ++ulBoard;
      fFound = 1;
    }
    xDriverClose(hDriver);

    if( 0 == fFound)
    {
      printf("\n!!! NO CIFX boards found !!!! \n");
    }
  }
  printf("\nThe server is reachable under:\n");
  DisplayIP();
  printf("******************************************************************************\n");
}


int main( int argc, char* argv[])
{
  long             lRet = CIFX_NO_ERROR;
  struct sigaction tSigTerm;

  if (0 == ValidateArgs(argc, argv))
    return 0;

  if ( (g_ptMutex = OS_CreateLock()) == NULL ) {
    printf("Error creating required resources - quiting server!\n");
    return -1;
  }

  sigemptyset(&tSigTerm.sa_mask);
  tSigTerm.sa_handler = DeInitServer;
  tSigTerm.sa_flags = 0;

  /* catch "ctrl + c" signal */
  sigaction(SIGINT,&tSigTerm,NULL);

  /* set to default values */
  g_tInit.init_options        = CIFX_DRIVER_INIT_AUTOSCAN;
  g_tInit.iCardNumber         = 0;
  g_tInit.fEnableCardLocking  = 0;
  g_tInit.base_dir            = NULL;
  g_tInit.poll_interval       = 0;
  g_tInit.poll_StackSize      = 0;
  g_tInit.trace_level         = 255;
  g_tInit.user_card_cnt       = 0;

  if (g_tInitParam.fUseSingleCard)
  {
    /* initialize toolkit to scan only for the specified card */
    g_tInit.init_options   = CIFX_DRIVER_INIT_CARDNUMBER;
    g_tInit.iCardNumber    = g_tInitParam.iCardNumber;
  }

  printf("cifXDriverInit...\n");
  /* First of all initialize toolkit */
  lRet = cifXDriverInit(&g_tInit);

  if(CIFX_NO_ERROR == lRet)
  {
      if (MARSHALLER_NO_ERROR == (lRet = InitMarshaller()))
      {
            printf("Press ctrl+'c' to quit!\n");
            g_fRunning = 1;

            /* display the current server settings */
            PrintServerInformation();
          } else
      {
        printf("Marshaller initialization failed!\n");
      }
  } else
  {
    printf("CifXDriver initialization failed!\n");
  }

  if (MARSHALLER_NO_ERROR != lRet)
  {
    if( NULL != g_pvMarshaller)
      kill(0,SIGINT);
  }

  /* main loop */
  while( g_fRunning)
  {
    OS_Sleep( 250);
  };

  return 0;
}
