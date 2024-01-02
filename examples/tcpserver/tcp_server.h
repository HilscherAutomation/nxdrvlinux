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

#ifndef __TCPSERVER__H
#define __TCPSERVER__H

#ifdef __cplusplus
  extern "C" {
#endif


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

} TCP_CONN_INTERNAL_T;



void        TrafficTimer                (void* dwUser);
void        MarshallerTimer             (int iSignal);
int32_t     APIENTRY xSysdeviceOpenWrap (CIFXHANDLE  hDriver, char*   szBoard, CIFXHANDLE* phSysdevice);
int32_t     APIENTRY xSysdeviceOpenWrap (CIFXHANDLE  hDriver, char*   szBoard, CIFXHANDLE* phSysdevice);
int32_t     APIENTRY xChannelOpenWrap   (CIFXHANDLE  hDriver,  char* szBoard, uint32_t ulChannel, CIFXHANDLE* phChannel);
int32_t     APIENTRY xChannelCloseWrap  (CIFXHANDLE  hChannel);
void        MarshallerRequest           (void* pvMarshaller, void* pvUser);
uint32_t    InitMarshaller              (void);
void        DeinitMarshaller            (void);

#ifdef __cplusplus
  }
#endif

#endif /* __TCPSERVER__H */









