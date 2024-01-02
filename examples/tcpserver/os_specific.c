// SPDX-License-Identifier: MIT
/**************************************************************************************
 *
 * Copyright (c) 2024, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 * Description: Linux OS Abstraction Layer implementation
 *
 * Changes:
 *
 *   Version   Date        Author   Description
 *   ----------------------------------------------------------------------------------
 *   1        02.01.24    SD        changed licensing terms
 *
 **************************************************************************************/

#include "OS_Includes.h"
#include <pthread.h>

/* NOTE: The current marshaller implementation does not use lock create/delete functions. */
/*       The mutex we are using here will be created by TCPServer.c.                      */
extern pthread_mutex_t* g_ptMutex;

void OS_EnterLock(void* pvLock);
void OS_LeaveLock(void* pvLock);

/*****************************************************************************/
/*! Get Millisecond counter value (used for timeout handling)
*     \return Counter value with a resolution of 1ms                         */
/*****************************************************************************/
uint32_t OS_GetTickCount(void) {
  struct timespec ts_get_milli;
  unsigned int    msec_count;

#ifdef VERBOSE_2
  printf("%s() called\n", __FUNCTION__);
#endif
  if( clock_gettime( CLOCK_MONOTONIC, &ts_get_milli ) != 0 )
  {
    perror("gettime failed");
    return 0;
  }
  msec_count = ts_get_milli.tv_sec * 1000;
  msec_count += ts_get_milli.tv_nsec / 1000 / 1000;

  return msec_count;
}

/*****************************************************************************/
/*! Acquire a lock
*     \param pvLock Handle to lock                                           */
/*****************************************************************************/
int OS_Lock(void) {
#ifdef VERBOSE_2
  printf("%s() called\n", __FUNCTION__);
#endif
    OS_EnterLock( g_ptMutex);
    return 0;
}

/*****************************************************************************/
/*! Release a lock
*     \param pvLock Handle to lock                                           */
/*****************************************************************************/
void OS_Unlock(int iLock) {
#ifdef VERBOSE_2
  printf("%s() called\n", __FUNCTION__);
#endif
    (void)iLock;/* we can ignore this parameter since the lock is one static reference */
    OS_LeaveLock( g_ptMutex);
}
