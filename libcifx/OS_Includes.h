/* SPDX-License-Identifier: MIT */
/**************************************************************************************
 *
 * Copyright (c) 2024, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 * Description: Header file for Linux specific operating system includes.
 *
 * Changes:
 *
 *   Version   Date        Author   Description
 *   ----------------------------------------------------------------------------------
 *   1        02.01.24    SD        changed licensing terms
 *
 **************************************************************************************/

#ifndef __OS_INCLUDES__H
#define __OS_INCLUDES__H

#ifdef CIFXETHERNET
#ifdef APIRENAME    /* NOTE: allow api renaming only when ethernet support is enabled */
#define xSysdeviceReset     xSysdeviceResetTK
#define xSysdeviceResetEx   xSysdeviceResetExTK
#define xSysdeviceBootstart xSysdeviceBootstartTK
#define xChannelReset       xChannelResetTK
#endif
#endif

#define APIENTRY

#ifndef NULL
  #define NULL 0
#endif

#ifndef UNREFERENCED_PARAMETER
  #define UNREFERENCED_PARAMETER(a) (void)(a)
#endif

#ifdef CIFX_DRV_HWIF
  #define CIFX_TOOLKIT_HWIF
#endif

#endif /* __OS_INCLUDES__H */
