/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: cifXToolkit.h 15448 2025-12-17 15:31:20Z AMinor $:

  Description:
    cifX toolkit function declaration.

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2025-05-26  - Moved USER_* functions into own header file
    2023-04-26  - Moved DEV function definitions to cifXHWFunctions.h
                - Check parameter macros from cifXFunctions.c moved here
    2021-06-14  - Added new user function USER_GetCachedIOBufferMode()
    2018-10-10  - Updated header and definitions to new Hilscher defines
                - Added chip type definitions for netX90/netX4000 (eCHIP_TYPE_NETX90 / eCHIP_TYPE_NETX4000)
                - Derived from cifX Toolkit V1.6.0.0

**************************************************************************************/

/*****************************************************************************/
/*! \file                                                                    *
 *  cifX toolkit function declaration                                        */
/*****************************************************************************/
#ifndef CIFX_TOOLKIT__H
#define CIFX_TOOLKIT__H

#include "cifXFunctionList.h"
#include "USER_Dependent.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define TOOLKIT_VERSION   "cifX Toolkit 2.8.4.0"

/*****************************************************************************/
/*! \addtogroup CIFX_TK_GLOBAL_API Toolkit global API functions              */
/*! \{                                                                       */
/*****************************************************************************/

/* Global Toolkit Functions */
int32_t cifXTKitInit              (void);
void    cifXTKitDeinit            (void);
int32_t cifXTKitAddDevice         (PDEVICEINSTANCE ptDevInstance);
int32_t cifXTKitRemoveDevice      (char* szBoard, int fForceRemove);
void    cifXTKitEnableHWInterrupt (PDEVICEINSTANCE ptDevInstance);
void    cifXTKitDisableHWInterrupt(PDEVICEINSTANCE ptDevInstance);
int     cifXTKitISRHandler        (PDEVICEINSTANCE ptDevInstance, int fPCIIgnoreGlobalIntFlag);
void    cifXTKitDSRHandler        (PDEVICEINSTANCE ptDevInstance);
void    cifXTKitCyclicTimer       (void);

/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* CIFX_TOOLKIT__H */
