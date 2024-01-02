/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: Hilcrc32.h 14199 2021-09-03 11:11:45Z RMayer $:

  Description:
    CRC32 function definition

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2021-09-01  - Created a separate function module

**************************************************************************************/
#ifndef HILCRC32_INCLUDED
#define HILCRC32_INCLUDED

#include <stdint.h> /*lint !e537 !e451 */

#ifdef __cplusplus
extern "C"
{
#endif

/* Create a continued CRC32 value. Start with ulCRC = 0. */
uint32_t CreateCRC32(uint32_t ulCRC, uint8_t* pabBuffer, uint32_t ulLength);

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif /* HILCRC32_INCLUDED */
