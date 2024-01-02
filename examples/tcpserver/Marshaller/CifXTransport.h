/**************************************************************************************
 
   Copyright (c) Hilscher GmbH. All Rights Reserved.
 
 **************************************************************************************
 
   Filename:
    $Workfile: CifXTransport.h $
   Last Modification:
    $Author: AlexanderMinor $
    $Modtime: $
    $Revision: 13309 $
   
   Targets:
     Win32/ANSI   : yes
     Win32/Unicode: yes (define _UNICODE)
     WinCE        : yes
 
   Description:
    Defines for the "CifX Modul" of the "Marshaller" device side 
       
   Changes:
 
     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
     1         25.05.2009  PL       intitial version
 
**************************************************************************************/

/*****************************************************************************/
/*! \file CifXTransport.h
*   cifX marshalling via Hilscher Transport Protocol                         */
/*****************************************************************************/

#ifndef __CIFXTRANSPORT__H
#define __CIFXTRANSPORT__H

/*****************************************************************************/
/*! \addtogroup NETX_MARSHALLER_CIFX
*   \{                                                                       */
/*****************************************************************************/

#ifdef __cplusplus
  extern "C" {
#endif  /* __cplusplus */

#include "MarshallerConfig.h"
#include "cifXAPI_Wrapper.h"

/* Init information for the cifX Transport */
typedef struct CIFX_TRANSPORT_CONFIG_Ttag
{
  DRIVER_FUNCTIONS  tDRVFunctions; /*!  Function list of CifX API   */  
} CIFX_TRANSPORT_CONFIG;

/* Initialize a Transport */
uint32_t cifXTransportInit(void* pvMarshaller, void* pvConfig);

#ifdef __cplusplus
  }
#endif  /* __cplusplus */

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/

#endif /* __CIFXTRANSPORT__H */
