/* SPDX-License-Identifier: MIT */
/**************************************************************************************
 *
 * Copyright (c) 2024, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 * Changes:
 *
 *   Version   Date        Author   Description
 *   ----------------------------------------------------------------------------------
 *   1        02.01.24    SD        changed licensing terms
 *
 **************************************************************************************/

#ifndef __TCPCONNECTOR__H
#define __TCPCONNECTOR__H

#include "OS_Includes.h"
#include "MarshallerInternal.h"
#include "CifXTransport.h"
#include "tcp_server.h"


#ifdef __cplusplus
  extern "C" {
#endif



uint32_t  InitMarshaller   ( void);
void      DeinitMarshaller ( void);
uint32_t  TCPConnectorInit (const HIL_MARSHALLER_CONNECTOR_PARAMS_T* ptParams, void* pvMarshaller);


#ifdef __cplusplus
  }
#endif

#endif /* __TCPCONNECTOR__H */
