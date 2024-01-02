/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: TKitUser_Custom.c 14564 2022-07-26 13:28:44Z RMayer $:

  Description:
    USER implemented functions called by the cifX Toolkit.

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2022-06-14  Added USER_GetCachedIOBufferMode() function template
    2021-08-13  Add a new line handling to USER_Trace() if necessary
    2006-08-07  initial version

**************************************************************************************/

/*****************************************************************************/
/*! \file TKitUser_Custom.c                                                         *
*    USER implemented functions called by the cifX Toolkit                   */
/*****************************************************************************/

#include "cifXToolkit.h"
#include "cifXErrors.h"

#error "Implement target system specifc user functions in this file"

/*****************************************************************************/
/*!  \addtogroup CIFX_TK_USER User specific implementation
*    \{                                                                      */
/*****************************************************************************/

/*****************************************************************************/
/*! Returns OS file information for the given device/channel and Idx
*   passed as argument. This is needed for module downloading
*   \param ptDevInfo  DeviceInfo for which the
*                     firmware file should be delivered
*   \param ptFileInfo Short and full file name of the firmware. Used in
*                     calls to OS_OpenFile()
*   \return != 0 on success                                                   */
/*****************************************************************************/
int USER_GetOSFile(PCIFX_DEVICE_INFORMATION ptDevInfo, PCIFX_FILE_INFORMATION ptFileInfo)
{
}

/*****************************************************************************/
/*! Returns the number of firmware files associated with the card/channel,
*   passed as argument.
*   \param ptDevInfo DeviceInfo including channel number, for which the
*                    firmware file count should be read
*   \return number for firmware files, to download. The returned value will
*           be used as maximum value for ulIdx in calls to
*           USER_GetFirmwareFile                                             */
/*****************************************************************************/
uint32_t USER_GetFirmwareFileCount(PCIFX_DEVICE_INFORMATION ptDevInfo)
{
}

/*****************************************************************************/
/*! Returns firmware file information for the given device/channel and Idx
*   passed as argument.
*   \param ptDevInfo  DeviceInfo including channel number, for which the
*                     firmware file should be delivered
*   \param ulIdx      Index of the returned file
*                     (0..USER_GetFirmwareFileCount() - 1)
*   \param ptFileInfo Short and full file name of the firmware. Used in
*                     calls to OS_OpenFile()
*   \return !=0 on success                                                   */
/*****************************************************************************/
int USER_GetFirmwareFile(PCIFX_DEVICE_INFORMATION ptDevInfo, uint32_t ulIdx, PCIFX_FILE_INFORMATION ptFileInfo)
{
}

/*****************************************************************************/
/*! Returns the number of configuration files associated with the card/
*   channel, passed as argument.
*   \param ptDevInfo DeviceInfo including channel number, for which the
*                    configuration file count should be read
*   \return number for configuration files, to download. The returned value
*           will be used as maximum value for ulIdx in calls to
*           USER_GetConfgirationFile                                         */
/*****************************************************************************/
uint32_t USER_GetConfigurationFileCount(PCIFX_DEVICE_INFORMATION ptDevInfo)
{
}

/*****************************************************************************/
/*! Returns configuration file information for the given device/channel and
*   Idx passed as argument.
*   \param ptDevInfo  DeviceInfo including channel number, for which the
*                     configuration file should be delivered
*   \param ulIdx      Index of the returned file
*                     (0..USER_GetConfigurationFileCount() - 1)
*   \param ptFileInfo Short and full file name of the configuration. Used in
*                     calls to OS_OpenFile()
*   \return !=0 on success                                                   */
/*****************************************************************************/
int USER_GetConfigurationFile(PCIFX_DEVICE_INFORMATION ptDevInfo, uint32_t ulIdx, PCIFX_FILE_INFORMATION ptFileInfo)
{
}

/*****************************************************************************/
/*! Retrieve the full file name of the cifX ROM loader binary image
*   \param ptDevInstance  Pointer to the device instance
*   \param ptFileInfo Short and full file name of the bootloader. Used in
*                     calls to OS_OpenFile()                                 */
/*****************************************************************************/
void USER_GetBootloaderFile(PDEVICEINSTANCE ptDevInstance, PCIFX_FILE_INFORMATION ptFileInfo)
{
}

/*****************************************************************************/
/*! Read the warmstart data from a given warmstart file
*   \param ptDevInfo Device- and Serial number of the card
*   \param ptPacket  Buffer for the warmstart packet
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
int USER_GetWarmstartParameters(PCIFX_DEVICE_INFORMATION ptDevInfo, CIFX_PACKET* ptPacket)
{
}

/*****************************************************************************/
/*! Retrieve the alias name of a cifX Board depending on the Device and
*   Serialnumber passed during this call
*   \param ptDevInfo Device- and Serial number of the card
*   \param ulMaxLen  Maximum length of alias
*   \param szAlias   Buffer to copy alias to. A string of length 0 means
*                    no alias                                                */
/*****************************************************************************/
void USER_GetAliasName(PCIFX_DEVICE_INFORMATION ptDevInfo, uint32_t ulMaxLen, char* szAlias)
{
}

/*****************************************************************************/
/*! Check if interrupts should be enabled for this device
*   \param ptDevInfo  Device Information
*   \return !=0 to enable interrupts                                         */
/*****************************************************************************/
int USER_GetInterruptEnable(PCIFX_DEVICE_INFORMATION ptDevInfo)
{
}

/*****************************************************************************/
/*! Get actual I/O buffer caching mode for the given device
*   \param ptDevInfo  Device Information
*   \return eCACHED_MODE_ON to enable caching                                */
/*****************************************************************************/
int USER_GetCachedIOBufferMode(PCIFX_DEVICE_INFORMATION ptDevInfo)
{
}

#ifdef CIFX_TOOLKIT_DMA
/*****************************************************************************/
/*! Check if dma should be enabled for this device
*   \param ptDevInfo  Device Information
*   \return CIFX_DMA_STATE_ON/CIFX_DMA_STATE_OFF                             */
/*****************************************************************************/
int USER_GetDMAMode(PCIFX_DEVICE_INFORMATION ptDevInfo)
{
  /* we cannot handle dma in win32 user mode */
  UNREFERENCED_PARAMETER(ptDevInfo);

  return CIFX_DMA_STATE_OFF;
}
#endif

/*****************************************************************************/
/*! User trace function
*   right while cifXTKitAddDevice is being processed
*   \param ptDevInstance  Device instance
*   \param ulTraceLevel   Trace level
*   \param szFormat       Format string                                      */
/*****************************************************************************/
void USER_Trace(PDEVICEINSTANCE ptDevInstance, uint32_t ulTraceLevel, const char* szFormat, ...)
{
  /* Add an new line on the end of the trace strings if necessary */
  /* e.g. printf("\r\n\"); */
}


/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/
