/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: OS_Dependent.h 13819 2020-11-25 09:36:58Z AMinor $:

  Description:
    OS Dependent function declaration. These functions must be implemented for the
    toolkit, to allow abstraction from the operating system

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2020-11-12  Created from the cifX Toolkit

**************************************************************************************/

#ifndef __OS_DEPENDENT__H
#define __OS_DEPENDENT__H

#include <stdint.h>
#include <OS_Includes.h>

#ifdef __cplusplus
extern "C"
{
#endif

void*    OS_Malloc(uint32_t ulSize);
void     OS_Free(void* pvMem);

void     OS_Memset(void* pvMem, uint8_t bFill, uint32_t ulSize);
void     OS_Memcpy(void* pvDest, void* pvSrc, uint32_t ulSize);

int      OS_Strnicmp(const char* pszBuf1, const char* pszBuf2, uint32_t ulLen);

int      OS_Lock(void);
void     OS_Unlock(int iLock);

uint32_t OS_GetTickCount(void);


#ifdef __cplusplus
}
#endif


#endif /* __OS_DEPENDENT__H */
