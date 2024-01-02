/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: SerialDPMInterface.h 12249 2018-08-07 07:43:24Z Robert $:

  Description:
    Serial DPM Interface

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2014-08-01  initial version

**************************************************************************************/

/*****************************************************************************/
/*! \file SerialDPMInterface.h
*   Serial DPM interface function definition                                 */
/*****************************************************************************/

#ifndef SERIALDPMINTERFACE__H
#define SERIALDPMINTERFACE__H

#ifdef __cplusplus
extern "C"
{
#endif

#define SERDPM_UNKNOWN  0x00
#define SERDPM_NETX10   0x01
#define SERDPM_NETX50   0x02
#define SERDPM_NETX51   0x03
#define SERDPM_NETX100  0x04

int SerialDPM_Init ( DEVICEINSTANCE* ptDevice);

#ifdef __cplusplus
}
#endif

#endif /* SERIALDPMINTERFACE__H */
