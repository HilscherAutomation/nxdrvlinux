// SPDX-License-Identifier: MIT
/**************************************************************************************
 *
 * Copyright (c) 2024, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 * Description: Linux specific implementation for finding firmware files, etc.
 *
 * Changes:
 *
 *   Version   Date        Author   Description
 *   ----------------------------------------------------------------------------------
 *   1        02.01.24    SD        changed licensing terms
 *
 **************************************************************************************/

#include "cifXToolkit.h"
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include "WarmstartFile.h"
#include "cifxlinux_internal.h"

char* g_szDriverBaseDir = NULL;  /*!< Global base path to driver/cifX configuration data */

static const char* DEVICE_CONF_ALIAS_KEY    = "alias=";
static const char* DEVICE_CONF_IRQ_KEY      = "irq=";
static const char* DEVICE_CONF_IRQPRIO_KEY  = "irqprio=";
static const char* DEVICE_CONF_IRQSCHED_KEY = "irqsched=";
#ifdef CIFX_TOOLKIT_DMA
static const char* DEVICE_CONF_DMA          = "dma=";
#endif
#ifdef CIFXETHERNET
static const char* DEVICE_CONF_ETH          = "eth=";
#endif

uint8_t severity_mapping[] = {
  0xff, /* not used */
  1,    /* TRACE_LEVEL_DEBUG */
  2,    /* TRACE_LEVEL_INFO */
  0xff, /* not used */
  4,    /* TRACE_LEVEL_WARNING */
  0xff, /* not used */
  0xff, /* not used */
  0xff, /* not used */
  8,    /* TRACE_LEVEL_ERROR */
};

/*****************************************************************************/
/*! Print a trace message from cifX toolkit
*     \param ptDevInstance  Device instance the trace is coming from
*     \param ulTraceLevel   see TRACE_LVL_XXX defines
*     \param szFormat       printf style format string
*     \param ...            printf arguments                                 */
/*****************************************************************************/
void USER_Trace(PDEVICEINSTANCE ptDevInstance, uint32_t ulTraceLevel, const char* szFormat, ...)
{
  PCIFX_DEVICE_INTERNAL_T internaldev = (PCIFX_DEVICE_INTERNAL_T)ptDevInstance->pvOSDependent;
  va_list                 vaList;
  struct timeval          time;
  struct tm               *local_tm;

  gettimeofday(&time, NULL);
  local_tm = localtime(&time.tv_sec);

  va_start(vaList, szFormat);

  if(NULL != internaldev->log_file)
  {
    fprintf(internaldev->log_file,
            "<%u> %.2d.%.2d.%.4d %.2d:%.2d:%.2d.%.3ld.%.3ld: ",
            severity_mapping[ulTraceLevel],
            local_tm->tm_mday, local_tm->tm_mon + 1, local_tm->tm_year + 1900,
            local_tm->tm_hour, local_tm->tm_min, local_tm->tm_sec,
            time.tv_usec / 1000, time.tv_usec % 1000);

    /* log file is given, so add this trace to our logfile */
    vfprintf(internaldev->log_file, szFormat, vaList);
    fprintf(internaldev->log_file, "\n");
    fflush(internaldev->log_file);

  } else
  {
    printf("<%u> %.2d.%.2d.%.4d %.2d:%.2d:%.2d.%.3ld.%3ld: ",
           severity_mapping[ulTraceLevel],
           local_tm->tm_mday, local_tm->tm_mon + 1, local_tm->tm_year + 1900,
           local_tm->tm_hour, local_tm->tm_min, local_tm->tm_sec,
           time.tv_usec / 1000, time.tv_usec % 1000);
    /* No logfile, so print to console */
    vprintf(szFormat, vaList);
    printf("\n");
  }

  UNREFERENCED_PARAMETER(ulTraceLevel);
}

#define PARSER_BUFFER_SIZE  1024

char* ToLowerCase( char* str, int len) {
  int i = 0;
  if (str != NULL) {
    while ((str[i] != 0) && (i<len)) {
      str[i] = tolower(str[i]);
      i++;
    }
  }
  return str;
}

/*****************************************************************************/
/*! Reads a value from an config file (ini-file style)
*     \param szFile   Path to the ini file
*     \param szKey    Key to search for (including trailing '=')
*     \param szValue  Pointer to returned value (needs to be free'd by caller)
*     \return !=0 on success                                                 */
/*****************************************************************************/
static int GetDeviceConfigString(const char* szFile, const char* szKey, char** szValue)
{
  int   ret = 0;
  FILE* fd  = fopen(szFile, "r");

  if(NULL != fd)
  {
    /* File is open */
    char* buffer = malloc(PARSER_BUFFER_SIZE);

    /* Read file line by line */
    while(NULL != fgets(buffer, PARSER_BUFFER_SIZE, fd))
    {
      char* key;

      /* '#' marks a comment line in the device.conf file */
      if(buffer[0] == '#')
        continue;

      /* Search for key in the input buffer */
      key = strstr(ToLowerCase(buffer, strlen(szKey)), szKey);

      if(NULL != key)
      {
        /* We've found the key */
        int   allocsize  = strlen(key + strlen(szKey)) + 1;
        int   valuelen;
        char* tempstring = (char*)malloc(allocsize);

        strcpy(tempstring, key + strlen(szKey));
        valuelen = strlen(tempstring);

        /* strip all trailing whitespaces */
        while( (tempstring[valuelen - 1] == '\n') ||
               (tempstring[valuelen - 1] == '\r') ||
               (tempstring[valuelen - 1] == ' ') )
        {
          tempstring[valuelen - 1] = '\0';
          --valuelen;
        }

        *szValue = tempstring;
        ret = 1;
        break;
      }
    }

    free(buffer);
    fclose(fd);
  }

  return ret;
}

int path_exists(char* szPath)
{
  struct stat s;
  return stat(szPath, &s);
}

/*****************************************************************************/
/*! Internal helper function returning the path to a channel directory
*   on the given device (e.g. /opt/cifx/deviceconfig/1250100/20004/channel0/)
*     \param szPath         Pointer to returned path
*     \param iPathLen       Length of the buffer passed in szPath
*     \param ptDevInfo      Device information (DeviceNr, SerialNr, ChannelNr)*/
/*****************************************************************************/
static void GetChannelDir(char* szPath, size_t iPathLen,PCIFX_DEVICE_INFORMATION ptDevInfo)
{
        uint32_t ulSlotNr = ptDevInfo->ptDeviceInstance->ulSlotNumber;
        /* if the rotary switch is set != 0 */
        if (ulSlotNr)
        {
                snprintf(szPath, iPathLen,
                        "%s/deviceconfig/Slot_%d/channel%d/",
                        g_szDriverBaseDir,
                        (unsigned int)ulSlotNr,
                        (unsigned int)ptDevInfo->ulChannel);
                if (path_exists(szPath)==0)
                  goto exit;
        }

        snprintf(szPath, iPathLen,
        "%s/deviceconfig/%d/%d/channel%d/",
        g_szDriverBaseDir,
        (unsigned int)ptDevInfo->ulDeviceNumber,
        (unsigned int)ptDevInfo->ulSerialNumber,
        (unsigned int)ptDevInfo->ulChannel);

        if (path_exists(szPath)==0)
          goto exit;

        snprintf(szPath, iPathLen,
                  "%s/deviceconfig/%s/channel%d/",
                  g_szDriverBaseDir,
                  ptDevInfo->ptDeviceInstance->szName,
                  (unsigned int)ptDevInfo->ulChannel);

        if (path_exists(szPath)==0)
          goto exit;

        snprintf(szPath, iPathLen,
                "%s/deviceconfig/FW/channel%d/",
                g_szDriverBaseDir,
                (unsigned int)ptDevInfo->ulChannel);

exit:
  return;
}

/*****************************************************************************/
/*! Internal helper function returning the path to a channel directory
*   on the given device  (e.g. /opt/cifx/deviceconfig/1250100/20004/)
*     \param szPath         Pointer to returned path
*     \param iPathLen       Length of the buffer passed in szPath
*     \param ptDevInfo      Device information (DeviceNr, SerialNr, ChannelNr)*/
/*****************************************************************************/
static void GetDeviceDir(char* szPath, size_t iPathLen,PCIFX_DEVICE_INFORMATION ptDevInfo)
{
  uint32_t ulSlotNr = ptDevInfo->ptDeviceInstance->ulSlotNumber;
  /* if the rotary switch is set != 0 */
  if (ulSlotNr)
  {
    snprintf(szPath, iPathLen,
             "%s/deviceconfig/Slot_%d/",
             g_szDriverBaseDir,
             (unsigned int)ulSlotNr);

    if (path_exists(szPath)==0)
      goto exit;
  }
  snprintf(szPath, iPathLen,
           "%s/deviceconfig/%d/%d/",
           g_szDriverBaseDir,
           (unsigned int)ptDevInfo->ulDeviceNumber,
           (unsigned int)ptDevInfo->ulSerialNumber);

  if (path_exists(szPath)==0)
    goto exit;

  snprintf(szPath, iPathLen,
           "%s/deviceconfig/%s/",
           g_szDriverBaseDir,
           ptDevInfo->ptDeviceInstance->szName);

  if (path_exists(szPath)==0)
    goto exit;

  snprintf(szPath, iPathLen,
           "%s/deviceconfig/FW/",
           g_szDriverBaseDir);

exit:
  return;
}

/*****************************************************************************/
/*! Returns the number of firmware files to be downloaded on the given
*   device/channel
*     \param ptDevInfo      Device information (DeviceNr, SerialNr, ChannelNr)
*     \return Number of files (used for USER_GetFirmwareFile()               */
/*****************************************************************************/
uint32_t USER_GetFirmwareFileCount(PCIFX_DEVICE_INFORMATION ptDevInfo)
{
  char          szPath[CIFX_MAX_FILE_NAME_LENGTH];
  unsigned long ulRet = 0;
  DIR*          dir;

  GetChannelDir(szPath, sizeof(szPath), ptDevInfo);

  if(NULL != (dir = opendir(szPath)))
  {
    struct dirent* dirent;

    while(NULL != (dirent = readdir(dir)))
    {
      char* szExt = strstr(dirent->d_name, ".");
      if(NULL != szExt)
      {
        if( (0 == strncasecmp(szExt, HIL_FILE_EXTENSION_FIRMWARE, 4)) ||
            (0 == strncasecmp(szExt, HIL_FILE_EXTENSION_OPTION, 4)) )
        {
          ++ulRet;
        }
      }
    }
    closedir(dir);
  }

  return ulRet;
}

/*****************************************************************************/
/*! Returns filename of the file to be downloaded on the given device/channel
*     \param ptDevInfo      Device information (DeviceNr, SerialNr, ChannelNr)
*     \param ulIdx          Number of the file (0..USER_GetFirmwareFileCount)
*     \param ptFileInfo     Full and short filename of the file
*     \return !=0 on success                                                 */
/*****************************************************************************/
int USER_GetFirmwareFile(PCIFX_DEVICE_INFORMATION ptDevInfo, uint32_t ulIdx,  PCIFX_FILE_INFORMATION ptFileInfo)
{
  char          szPath[CIFX_MAX_FILE_NAME_LENGTH];
  int           ret   = 0;
  DIR*          dir;

  GetChannelDir(szPath, sizeof(szPath), ptDevInfo);

  if(NULL != (dir = opendir(szPath)))
  {
    struct dirent* dirent;
    unsigned long  ulFile = 0;

    while(NULL != (dirent = readdir(dir)))
    {
      char* szExt = strstr(dirent->d_name, ".");
      if(NULL != szExt)
      {
        if( (0 == strncasecmp(szExt, HIL_FILE_EXTENSION_FIRMWARE, 4)) ||
            (0 == strncasecmp(szExt, HIL_FILE_EXTENSION_OPTION, 4)) )
        {
          if(ulFile++ == ulIdx)
          {
            snprintf(ptFileInfo->szFullFileName, sizeof(ptFileInfo->szFullFileName),
                     "%s/%s", szPath, dirent->d_name);
            strncpy(ptFileInfo->szShortFileName, dirent->d_name,
                    sizeof(ptFileInfo->szShortFileName));
            ret = 1;
            break;
          }
        }
      }
    }
    closedir(dir);
  }

  return ret;
}

/*****************************************************************************/
/*! Returns the number of configuration files to be downloaded on the given
*   device/channel
*    \param ptDevInfo      Device information (DeviceNr, SerialNr, ChannelNr)
*    \return Number of files (used for USER_GetConfigurationFile()           */
/*****************************************************************************/
uint32_t USER_GetConfigurationFileCount(PCIFX_DEVICE_INFORMATION ptDevInfo)
{
  char          szPath[CIFX_MAX_FILE_NAME_LENGTH];
  unsigned long ulRet = 0;
  DIR*          dir;

  GetChannelDir(szPath, sizeof(szPath), ptDevInfo);

  if(NULL != (dir = opendir(szPath)))
  {
    struct dirent* dirent;

    while(NULL != (dirent = readdir(dir)))
    {
      char* szExt = strstr(dirent->d_name, ".");
      if(NULL != szExt)
      {
        if(0 == strncasecmp(szExt, HIL_FILE_EXTENSION_DATABASE, 4))
        {
          ++ulRet;
        }
      }
    }
    closedir(dir);
  }

  return ulRet;
}

/*****************************************************************************/
/*! Returns filename of the file to be downloaded on the given device/channel
*     \param ptDevInfo  Device information (DeviceNr, SerialNr, ChannelNr)
*     \param ulIdx      Number of the file (0..USER_GetConfigurationFileCount)
*     \param ptFileInfo Full and short filename of the file
*     \return !=0 on success                                                 */
/*****************************************************************************/
int USER_GetConfigurationFile(PCIFX_DEVICE_INFORMATION ptDevInfo, uint32_t ulIdx, PCIFX_FILE_INFORMATION ptFileInfo)
{
  char          szPath[CIFX_MAX_FILE_NAME_LENGTH];
  int           ret   = 0;
  DIR*          dir;

  GetChannelDir(szPath, sizeof(szPath), ptDevInfo);

  if(NULL != (dir = opendir(szPath)))
  {
    struct dirent* dirent;
    unsigned long  ulFile = 0;

    while(NULL != (dirent = readdir(dir)))
    {
      char* szExt = strstr(dirent->d_name, ".");
      if(NULL != szExt)
      {
        if(0 == strncasecmp(szExt, HIL_FILE_EXTENSION_DATABASE, 4))
        {
          if(ulFile++ == ulIdx)
          {
            snprintf(ptFileInfo->szFullFileName, sizeof(ptFileInfo->szFullFileName),
                     "%s/%s", szPath, dirent->d_name);
            strncpy(ptFileInfo->szShortFileName, dirent->d_name,
                    sizeof(ptFileInfo->szShortFileName));
            ret = 1;
            break;
          }
        }
      }
    }
    closedir(dir);
  }

  return ret;
}

/*****************************************************************************/
/*! Get warmstart parameters (for slave devices to be configured at startup)
*   This is usually done by the user application, and not the toolkit
*     \param ptDevInfo  Device information (DeviceNr, SerialNr, ChannelNr)
*     \param ptPacket   Packet to send after starting the firmware
*     \return !=0 if a packet is available                                   */
/*****************************************************************************/
int USER_GetWarmstartParameters(PCIFX_DEVICE_INFORMATION ptDevInfo, CIFX_PACKET* ptPacket)
{
  int           ret = 0;
  char          szFile[CIFX_MAX_FILE_NAME_LENGTH];
  void*         pvFile;
  uint32_t ulFileLen;

  GetChannelDir(szFile, sizeof(szFile), ptDevInfo);
  strcat(szFile, "warmstart.dat");

  pvFile = OS_FileOpen(szFile, &ulFileLen);

  if ( (pvFile == NULL)                   ||
        (ulFileLen < sizeof(CIFX_WS_FILEHEADER)) )
  {
    if (errno != ENOENT) /* do not print an error if file does not exists */
    {
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInfo->ptDeviceInstance,
                    TRACE_LEVEL_ERROR,
                    "Error opening warm start file! %s", strerror(errno));
      }
    }
  } else
  {
    CIFX_WS_FILEHEADER tHeader = {0};

    /* Read file header */
    if (OS_FileRead( pvFile, 0, sizeof(tHeader), &tHeader) != sizeof(tHeader))
    {
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInfo->ptDeviceInstance,
                    TRACE_LEVEL_ERROR,
                    "Error reading from warm start file!");
      }

    } else if( tHeader.ulCookie != CIFX_WS_WARMSTART_FILE_COOKIE)
    {
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInfo->ptDeviceInstance,
                    TRACE_LEVEL_ERROR,
                    "Invalid warm start file cookie!");
      }
    } else if( tHeader.ulDataLen > sizeof(*ptPacket))
    {
      if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
      {
        USER_Trace(ptDevInfo->ptDeviceInstance,
                    TRACE_LEVEL_ERROR,
                    "Invalid warm start file length!");
      }
    } else
    {
      /* Read file data */
      if (OS_FileRead( pvFile, sizeof(tHeader), tHeader.ulDataLen, (void*)ptPacket) != tHeader.ulDataLen)
      {
        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInfo->ptDeviceInstance,
                    TRACE_LEVEL_ERROR,
                    "Error reading user data from warm start file!");
        }
      } else
      {
        ret = 1;
      }
    }

    if( NULL != pvFile)
      OS_FileClose(pvFile);
  }

  return ret;
}

/*****************************************************************************/
/*! Retrieve the full and short filename of a bootloader file to download to
*   the given device
*     \param ptDevInstance Device Instance containing all device data
*     \param ptFileInfo    Pointer to returned file information              */
/*****************************************************************************/
void USER_GetBootloaderFile(PDEVICEINSTANCE ptDevInstance, PCIFX_FILE_INFORMATION ptFileInfo)
{
  switch(ptDevInstance->eChipType)
  {
  case eCHIP_TYPE_NETX500:
  case eCHIP_TYPE_NETX100:
    {
      FILE* fd;

      snprintf(ptFileInfo->szFullFileName, sizeof(ptFileInfo->szFullFileName),
              "%s/%s",
              g_szDriverBaseDir, "NETX100-BSL.bin");

      strncpy(ptFileInfo->szShortFileName,
              "NETX100-BSL.bin", sizeof(ptFileInfo->szShortFileName));

      if(NULL != (fd = fopen(ptFileInfo->szFullFileName, "r")))
      {
        /* New bootloader found */
        fclose(fd);

      } else
      {
        /* new bootloader not found. Assume user still has the "old" cifX bootloader */
        snprintf(ptFileInfo->szFullFileName, sizeof(ptFileInfo->szFullFileName),
                "%s/%s",
                g_szDriverBaseDir, "NXCIF50-RTE.bin");

        strncpy(ptFileInfo->szShortFileName,
                "NXCIF50-RTE.bin", sizeof(ptFileInfo->szShortFileName));
      }
    }
    break;

  case eCHIP_TYPE_NETX52:
    snprintf(ptFileInfo->szFullFileName, sizeof(ptFileInfo->szFullFileName),
             "%s/%s",
             g_szDriverBaseDir, "NETX52-BSL.bin");

    strncpy(ptFileInfo->szShortFileName,
            "NETX52-BSL.bin", sizeof(ptFileInfo->szShortFileName));
    break;

  case eCHIP_TYPE_NETX51:
    snprintf(ptFileInfo->szFullFileName, sizeof(ptFileInfo->szFullFileName),
             "%s/%s",
             g_szDriverBaseDir, "NETX51-BSL.bin");

    strncpy(ptFileInfo->szShortFileName,
            "NETX51-BSL.bin", sizeof(ptFileInfo->szShortFileName));
    break;

  case eCHIP_TYPE_NETX50:
      /* new bootloader for netX50 will default to NETX50-BSL.bin (which is the default toolkit delivery) */
      snprintf(ptFileInfo->szFullFileName, sizeof(ptFileInfo->szFullFileName),
              "%s/%s",
              g_szDriverBaseDir, "NETX50-BSL.bin");

      strncpy(ptFileInfo->szShortFileName,
              "NETX50-BSL.bin", sizeof(ptFileInfo->szShortFileName));
    break;

  default:
    /* This should never happen */
    break;
  }
}

/*****************************************************************************/
/*! Retrieve the alias name of a card. This name can be alternatively used
*   by a application in a call to xChannelOpen. This name must be unique
*     \param ptDevInfo  Device information (DeviceNr, SerialNr)
*     \param ulMaxLen   Maximum Length of alias
*     \param szAlias    Pointer to returned alias name (empty = no alias)    */
/*****************************************************************************/
void USER_GetAliasName(PCIFX_DEVICE_INFORMATION ptDevInfo, uint32_t ulMaxLen, char* szAlias)
{
  char  szFile[CIFX_MAX_FILE_NAME_LENGTH];
  char* szTempAlias = NULL;

  GetDeviceDir(szFile, sizeof(szFile), ptDevInfo);
  strcat(szFile, "/device.conf");

  /* Read alias from file */
  if(GetDeviceConfigString(szFile, DEVICE_CONF_ALIAS_KEY, &szTempAlias))
  {
    strncpy(szAlias, szTempAlias, ulMaxLen);
    free(szTempAlias);
  }
}

/*****************************************************************************/
/*! Returns OS file information for the given device/channel and Idx
*   passed as argument.
*   \param ptDevInfo  DeviceInfo including channel number, for which the
*                     firmware file should be delivered
*   \param ptFileInfo Short and full file name of the firmware. Used in
*                     calls to OS_OpenFile()
*   \return != 0 on success                                                   */
/*****************************************************************************/
int USER_GetOSFile(PCIFX_DEVICE_INFORMATION ptDevInfo, PCIFX_FILE_INFORMATION ptFileInfo)
{
  char          szPath[CIFX_MAX_FILE_NAME_LENGTH];
  int           ret   = 0;
  DIR*          dir;

  GetDeviceDir(szPath, sizeof(szPath), ptDevInfo);

  if(NULL != (dir = opendir(szPath)))
  {
    struct dirent* dirent;

    while(NULL != (dirent = readdir(dir)))
    {
      char* szExt = strstr(dirent->d_name, ".");
      if(NULL != szExt)
      {
        if(0 == strncasecmp(szExt, HIL_FILE_EXTENSION_FIRMWARE, 4))
        {
          snprintf(ptFileInfo->szFullFileName, sizeof(ptFileInfo->szFullFileName),
                  "%s/%s", szPath, dirent->d_name);
          strncpy(ptFileInfo->szShortFileName, dirent->d_name,
                  sizeof(ptFileInfo->szShortFileName));
          ret = 1;
          break;
        }
      }
    }
    closedir(dir);
  }

  return ret;
}


/*****************************************************************************/
/*! Check if the interrupts are to be enabled on this device
*     \param ptDevInstance Device Instance containing all device data
*     \return !=0 to enable IRQs                                             */
/*****************************************************************************/
int USER_GetInterruptEnable(PCIFX_DEVICE_INFORMATION ptDevInfo)
{
  char  szFile[CIFX_MAX_FILE_NAME_LENGTH];
  char* szTempIrq   = NULL;
  int   ret         = 0;

  GetDeviceDir(szFile, sizeof(szFile), ptDevInfo);
  strcat(szFile, "/device.conf");

  /* Read IRQ enable from file */
  if(GetDeviceConfigString(szFile, DEVICE_CONF_IRQ_KEY, &szTempIrq))
  {
    if(0 == strcasecmp("yes", szTempIrq))
    {
      PCIFX_DEVICE_INTERNAL_T internaldev = (PCIFX_DEVICE_INTERNAL_T)ptDevInfo->ptDeviceInstance->pvOSDependent;

      /* Check if we have a uio handle. If not, we cannot handle IRQs */
      if(-1 == internaldev->userdevice->uio_fd)
      {
        if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
        {
          USER_Trace(ptDevInfo->ptDeviceInstance,
                         TRACE_LEVEL_ERROR,
                         "Trying to activate IRQ's on a device, that does not have a uio connection. Fallback to polling mode!");
        }
      } else
      {
        char*    szTempData  = NULL;
        uint32_t ulEnableInt = 0;
        /* NOTE: We don't want do enable interrupts here! We just want to check if the device provides interrupt support! */
        /*       ret = ENOSYS => !to be backwards compatible! - verification not provided but device supports interrupt   */
        /*       ret = EIO    => interrupt not supported, the device did not register for any interrupt                   */
        /* NOTE: For SPIDEV devices, the IRQ trigger file is opened read only! In this case the uio_num is -1.            */
        if ((internaldev->userdevice->uio_num >= 0) && ((ret=write(internaldev->userdevice->uio_fd, (void*)&ulEnableInt, sizeof(uint32_t))) < 0) && (errno != ENOSYS))
        {
          ret = 0;
          if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
          {
            USER_Trace(ptDevInfo->ptDeviceInstance,
                         TRACE_LEVEL_ERROR,
                         "Error enabling interrupts (%s), fallback to polling mode!",
                         strerror(errno));
            if (errno == EIO)
            {
              if(g_ulTraceLevel & TRACE_LEVEL_ERROR)
              {
                USER_Trace(ptDevInfo->ptDeviceInstance,
                         TRACE_LEVEL_ERROR,
                         "No irq for device registered!");
              }
            }
          }
        } else
        {
          /* Check for custom priority and set them in CIFX_DEVICE_INTERNAL_T to use them in OS_EnableInterrupts() */
          if(GetDeviceConfigString(szFile, DEVICE_CONF_IRQPRIO_KEY, &szTempData))
          {
            internaldev->set_irq_prio = 1;
            internaldev->irq_prio     = atoi(szTempData);

            if(g_ulTraceLevel & TRACE_LEVEL_INFO)
            {
              USER_Trace(ptDevInfo->ptDeviceInstance,
                         TRACE_LEVEL_INFO,
                         "Using custom IRQ thread priority (%d).",
                         internaldev->irq_prio);
            }
            free(szTempData);
          } else
          {
            internaldev->set_irq_prio = 0;
          }

          /* Check for custom scheduling policy */
          if(GetDeviceConfigString(szFile, DEVICE_CONF_IRQSCHED_KEY, &szTempData))
          {
            internaldev->set_irq_scheduler_algo = 1;

            if(strcasecmp("fifo", szTempData) == 0)
            {
              /* SCHED_FIFO */
              internaldev->irq_scheduler_algo = SCHED_FIFO;
              if(g_ulTraceLevel & TRACE_LEVEL_INFO)
              {
                USER_Trace(ptDevInfo->ptDeviceInstance,
                           TRACE_LEVEL_INFO,
                           "Using custom IRQ thread scheduling algorithm (SCHED_FIFO).");
              }

            } else if(strcasecmp("rr", szTempData) == 0)
            {
              /* SCHED_RR */
              internaldev->irq_scheduler_algo = SCHED_RR;
              if(g_ulTraceLevel & TRACE_LEVEL_INFO)
              {
                USER_Trace(ptDevInfo->ptDeviceInstance,
                           TRACE_LEVEL_INFO,
                           "Using custom IRQ thread scheduling algorithm (SCHED_RR).");
              }

            } else
            {
              internaldev->set_irq_scheduler_algo = 0;
              if(g_ulTraceLevel & TRACE_LEVEL_INFO)
              {
                USER_Trace(ptDevInfo->ptDeviceInstance,
                           TRACE_LEVEL_INFO,
                           "Trying to override IRQ thread scheduling policy, but unknown policy was given (%s)", szTempData);
              }
            }
            free(szTempData);
          } else
          {
            internaldev->set_irq_scheduler_algo = 0;
          }
          ret = 1;
        }
      }
    }
    free(szTempIrq);
  }
  if(g_ulTraceLevel & TRACE_LEVEL_INFO)
  {
    USER_Trace( ptDevInfo->ptDeviceInstance, TRACE_LEVEL_INFO, "%s", (ret)?"IRQ-Mode enabled!":"Polling Mode enabled!");
  }

  return ret;
}

#ifdef CIFX_TOOLKIT_DMA
/*****************************************************************************/
/*! Check if the DMA mode is enabled
*     \param ptDevInstance Device Instance containing all device data
*     \return CIFX_TOOLKIT_DMA_MODE_E
*/
/*****************************************************************************/
int USER_GetDMAMode(PCIFX_DEVICE_INFORMATION ptDevInfo)
{
  char  szFile[CIFX_MAX_FILE_NAME_LENGTH];
  char* szTempDMA = NULL;

  GetDeviceDir(szFile, sizeof(szFile), ptDevInfo);
  strcat(szFile, "/device.conf");

  if(GetDeviceConfigString(szFile, DEVICE_CONF_DMA, &szTempDMA))
  {
    if(0 == strcasecmp("yes", szTempDMA))
    {
      if(g_ulTraceLevel & TRACE_LEVEL_INFO)
      {
        USER_Trace(ptDevInfo->ptDeviceInstance, 0, "DMA mode enabled!");
      }
      return eDMA_MODE_ON;
    }
    free(szTempDMA);
  }
  if(g_ulTraceLevel & TRACE_LEVEL_INFO)
  {
    USER_Trace(ptDevInfo->ptDeviceInstance, 0, "No DMA support!");
  }
  return eDMA_MODE_OFF;
}
#endif

#ifdef CIFXETHERNET
/*****************************************************************************/
/*! Check if the ethernet support is enabled
*     \param ptDevInstance Device Instance containing all device data
*     \return CIFX_TOOLKIT_DMA_MODE_E
*/
/*****************************************************************************/
int USER_GetEthernet(PCIFX_DEVICE_INFORMATION ptDevInfo)
{
  char  szFile[CIFX_MAX_FILE_NAME_LENGTH];
  char* szTempEth = NULL;
  int   ret       = 0;

  GetDeviceDir(szFile, sizeof(szFile), ptDevInfo);
  strcat(szFile, "/device.conf");

  if(GetDeviceConfigString(szFile, DEVICE_CONF_ETH, &szTempEth))
  {
    if(0 == strcasecmp("yes", szTempEth))
    {
      if(g_ulTraceLevel & TRACE_LEVEL_INFO)
      {
        USER_Trace(ptDevInfo->ptDeviceInstance, 0, "Ethernet support enabled!");
      }
      ret = 1;
    }
    free(szTempEth);
  } else
  {
    if(g_ulTraceLevel & TRACE_LEVEL_INFO)
    {
      USER_Trace(ptDevInfo->ptDeviceInstance, 0, "No ethernet support!");
    }
  }
  return ret;
}
#endif

/*****************************************************************************/
/*! Get actual I/O buffer caching mode for the given device
*   \param ptDevInfo  Device Information
*   \return eCACHED_MODE_ON to enable caching                                */
/*****************************************************************************/
int USER_GetCachedIOBufferMode(PCIFX_DEVICE_INFORMATION ptDevInfo)
{
  return eCACHED_MODE_OFF;
}
