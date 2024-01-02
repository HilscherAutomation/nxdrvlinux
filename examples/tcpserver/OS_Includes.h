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

#ifndef __OS_INCLUDES__H
#define __OS_INCLUDES__H

#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include <sys/time.h>
#include <signal.h>

#include "OS_Dependent.h"


#ifndef NULL
  #define NULL  ((void*)0)
#endif

#ifndef UNREFERENCED_PARAMETER
  #define UNREFERENCED_PARAMETER(a) (a=a)
#endif

#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define TYPE_FD_SET fd_set
#define BOOL bool
#define TRUE true

#define          OS_MEMCPY      memcpy
#define          OS_STRNCPY     strncpy

#define          OS_Free(x)   OS_Memfree(x)
#define          OS_Malloc(x) OS_Memalloc(x)

/* these function are part of the libcifx library */
void*            OS_Memalloc(uint32_t ulSize);
void             OS_Memfree(void* pvMem);
void             OS_Sleep       (uint32_t ulSleepTimeMs);
/* impl. in OS_Specific.c */
uint32_t         OS_GetTickCount(void);


#ifdef __cplusplus
}
#endif

#endif /* __OS_INCLUDES__H */
