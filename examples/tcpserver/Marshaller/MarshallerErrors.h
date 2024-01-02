/**************************************************************************************
 
   Copyright (c) Hilscher GmbH. All Rights Reserved.
 
 **************************************************************************************
 
   Filename:
    $Workfile: OS_Dependent.h $
   Last Modification:
    $Author: MichaelT $
    $Modtime: 4.05.07 15:17 $
    $Revision: 1034 $
   
   Targets:
     Win32/ANSI   : yes
     Win32/Unicode: yes (define _UNICODE)
     WinCE        : yes
 
   Description:
    Marshaller Error code definitions
       
   Changes:
 
     Version   Date        Author   Description
     ----------------------------------------------------------------------------------
     1         07.07.2006  MT       intitial version
 
**************************************************************************************/

#ifndef __MARSHALLERERRORS__H
#define __MARSHALLERERRORS__H

/*****************************************************************************/
/*! \addtogroup NETX_MARSHALLER_MAIN
*   \{                                                                       */
/*****************************************************************************/

#define MARSHALLER_NO_ERROR     0x00000000

#define MARSHALLER_E_NOTINITIALIZED     0x800D0000
#define MARSHALLER_E_INVALIDHANDLE      0x800D0001
#define MARSHALLER_E_INVALIDOBJECTTYPE  0x800D0002
#define MARSHALLER_E_INVALIDMETHODID    0x800D0003
#define MARSHALLER_E_INVALIDPARAMETER   0x800D0004

#define MARSHALLER_E_BUSY               0x800D0010

#define MARSHALLER_E_UNSUPPORTED_TYPE   0x800D0020

/*****************************************************************************/
/*  \}                                                                       */
/*****************************************************************************/

#endif /* __MARSHALLERERRORS__H */
