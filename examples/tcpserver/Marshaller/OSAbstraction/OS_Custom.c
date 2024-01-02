/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: OS_Custom.c 13819 2020-11-25 09:36:58Z AMinor $:

  Description:
    Target system abstraction layer

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2011-12-13  added OS_Time() function body
    2006-08-07  initial version

**************************************************************************************/

/*****************************************************************************/
/*! \file OS_Custom.c
*    Sample Target system abstraction layer. Implementation must be done
*    according to used target system                                         */
/*****************************************************************************/

#include "OS_Dependent.h"

#error "Implement target system abstraction in this file"

/*****************************************************************************/
/*!  \addtogroup CIFX_TK_OS_ABSTRACTION Operating System Abstraction
*    \{                                                                      */
/*****************************************************************************/

/*****************************************************************************/
/*! Memory allocation function
*   NOTE: Malloc with size 0 should be prevented and returns a NULL pointer
*   \param ulSize    Length of memory to allocate
*   \return Pointer to allocated memory                                      */
/*****************************************************************************/
void* OS_Malloc(uint32_t ulSize)
{
  if( 0 == ulSize)
    return NULL;
}

/*****************************************************************************/
/*! Memory freeing function
*   \param pvMem Memory block to free                                        */
/*****************************************************************************/
void OS_Free(void* pvMem)
{
}

/*****************************************************************************/
/*! Memory setting
*   \param pvMem     Memory block
*   \param bFill     Byte to use for memory initialization
*   \param ulSize    Memory size for initialization)                         */
/*****************************************************************************/
void OS_Memset(void* pvMem, uint8_t bFill, uint32_t ulSize)
{
}

/*****************************************************************************/
/*! Copy memory from one block to another
*   \param pvDest    Destination memory block
*   \param pvSrc     Source memory block
*   \param ulSize    Copy size in bytes                                      */
/*****************************************************************************/
void OS_Memcpy(void* pvDest, void* pvSrc, uint32_t ulSize)
{
}

/*****************************************************************************/
/*! Compare characters of two strings without regard to case
*   \param pszBuf1   First buffer
*   \param pszBuf2   Second buffer
*   \param ulLen     Number of characters to compare
*   \return 0 if strings are equal                                           */
/*****************************************************************************/
int OS_Strnicmp(const char* pszBuf1, const char* pszBuf2, uint32_t ulLen)
{
}

/*****************************************************************************/
/*! Enter a critical section/spinlock
*   \return Handle to the critical section/spinlock                          */
/*****************************************************************************/
int OS_Lock(void)
{
}

/*****************************************************************************/
/*! Leave a critical section/spinlock
*   \param iLock Handle to the locked object                                 */
/*****************************************************************************/
void OS_Unlock(int iLock)
{
}

/*****************************************************************************/
/*! Get Millisecond counter value (used for timeout handling)
*   Retrieves the number of milliseconds that have elapsed since the system
*   was started.
*   Note: Depending on the timer resolution this value can wrap around
*     \return Counter value with a resolution of 1ms                         */
/*****************************************************************************/
uint32_t OS_GetTickCount(void)
{
}

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/
