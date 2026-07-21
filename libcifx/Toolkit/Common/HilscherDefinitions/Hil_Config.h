/**************************************************************************************
  Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
  $HeadURL: https://subversion01/svn/HilscherDefinitions/netXFirmware/Headers/tags/20240319-00/includes/Hil_Compiler.h $: *//*!

  \file Hil_Config.h

  Configuration of the default settings

**************************************************************************************/
#ifndef HIL_CONFIG_H_
#define HIL_CONFIG_H_

#ifdef HIF_SUPPORT
  #define HIL_MAX_PACKET_SIZE_DEFAULT (2176)
#else
  #define HIL_MAX_PACKET_SIZE_DEFAULT (1596)
#endif

#endif /* HIL_CONFIG_H_ */
