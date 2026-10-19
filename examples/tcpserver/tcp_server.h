/* SPDX-License-Identifier: MIT */
/**************************************************************************************
 *
 * Copyright (c) 2025, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 **************************************************************************************/

#ifndef __TCPSERVER__H
#define __TCPSERVER__H

#ifdef __cplusplus
  extern "C" {
#endif

#include "MarshallerConfig.h"


/*****************************************************************************/
/*! Internal UART connector data                                             */
/*****************************************************************************/
typedef struct TCP_CONN_INTERNAL_Ttag
{
uint32_t   ulConnectorIdx;
void*      pvMarshaller;

int        fRunning;

SOCKET     hListen;
pthread_t  hServerThread;

SOCKET        hClient;
pthread_t     hClientThread;
unsigned long ulRxCount;
unsigned long ulTxCount;

HIL_MARSHALLER_CONNECTOR_IF_H hMarshallerIf;

} TCP_CONN_INTERNAL_T;



void        TrafficTimer                (void* dwUser);
void        MarshallerTimer             (int iSignal);
int32_t     APIENTRY xSysdeviceOpenWrap (CIFXHANDLE  hDriver, char*   szBoard, CIFXHANDLE* phSysdevice);
int32_t     APIENTRY xSysdeviceOpenWrap (CIFXHANDLE  hDriver, char*   szBoard, CIFXHANDLE* phSysdevice);
int32_t     APIENTRY xChannelOpenWrap   (CIFXHANDLE  hDriver,  char* szBoard, uint32_t ulChannel, CIFXHANDLE* phChannel);
int32_t     APIENTRY xChannelCloseWrap  (CIFXHANDLE  hChannel);
void        MarshallerRequest           (HIL_MARSHALLER_H pvMarshaller, HIL_MARSHALLER_USERDATA_H hUser);
uint32_t    InitMarshaller              (void);
void        DeinitMarshaller            (void);

#ifdef __cplusplus
  }
#endif

#endif /* __TCPSERVER__H */









