/* SPDX-License-Identifier: MIT */
/**************************************************************************************
 *
 * Copyright (c) 2025, Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
 *
 **************************************************************************************/

#ifndef __CIFXLINUX_DBG__H
#define __CIFXLINUX_DBG__H

#include "USER_Dependent.h"

#define FORMAT_STR(type,fmt) type "%s: " fmt
#define ERR(fmt, ...)  do { \
                         if (g_ulTraceLevel & CIFX_TRACE_LEVEL_ERROR) { \
                           fprintf( stderr, FORMAT_STR("ERR:",fmt), __func__, ##__VA_ARGS__); \
                         } \
                        } while (0)
#if defined(VERBOSE) || defined(DEBUG)
  #define DBG(fmt, ...)  do { \
                           if (g_ulTraceLevel & CIFX_TRACE_LEVEL_DEBUG) { \
                             fprintf( stdout, FORMAT_STR("DBG:",fmt), __func__, ##__VA_ARGS__); \
                           } \
                         } while (0)
#else
  #define DBG(fmt, ...)
#endif

#endif /* __CIFXLINUX_DBG__H */
