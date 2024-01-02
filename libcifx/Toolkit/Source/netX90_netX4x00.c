/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: netX90_netX4x00.c 14189 2021-08-31 10:49:31Z RMayer $:

  Description:
    cifX Toolkit implementation of the netX90 and netX4000 detection functions

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2021-08-31  - Added separate functions for FLASH and ROM chip detection
                - Fixed typo in function headers
    2018-11-30  created

**************************************************************************************/

#include "cifXToolkit.h"
#include "cifXEndianess.h"
#include "netx90_4x00_romloader_dpm.h"


/*****************************************************************************/
/*! Detect a running netX4000 firmware via DPM mapped config registers
*   In case of successful detection, device instance member eChipType is set.
*
*   \param ptDevInstance        Current device instance
*   \return !=0 if netX4x00 has been detected                                */
/*****************************************************************************/
int IsNetX4x00FLASH(PDEVICEINSTANCE ptDevInstance)
{
  int iRet = 0;
  uint32_t ulDpmNetxVersion = 0;

  /* Use the netX global register block to detect the netX chip */
  /* Note: the pointer to the global register block is set in cifXStartDevice() */

  /* ulDpm_netx_version in register block (end of DPM memory) */
  ulDpmNetxVersion = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptDevInstance->ptGlobalRegisters->reserved6));

  /* Check for known version/cookie */
  if( HBOOT_DPM_NETX4000_COOKIE == ulDpmNetxVersion)
  {
    /* This is a netX4000 */
    ptDevInstance->eChipType = eCHIP_TYPE_NETX4000;
    iRet = 1;
  } else if( HBOOT_DPM_NETX4100_COOKIE == ulDpmNetxVersion)
  {
    /* This is a netX4100 */
    ptDevInstance->eChipType = eCHIP_TYPE_NETX4100;
    iRet = 1;
  }

  return iRet;
}

/*****************************************************************************/
/*! Detect a running netX4000 ROM code via DPM mapped config registers
*   In case of successful detection, device instance member eChipType is set.
*
*   \param ptDevInstance        Current device instance
*   \return !=0 if netX4x00 has been detected                                */
/*****************************************************************************/
int IsNetX4x00ROM(PDEVICEINSTANCE ptDevInstance)
{
  int iRet = 0;
  uint32_t ulDpmNetxVersion = 0;

  /* Use the netX global register block to detect the netX chip */
  HBOOT_V2_DPM_CFG_AREA_T* ptDpmCfg = (HBOOT_V2_DPM_CFG_AREA_T*)ptDevInstance->pbDPM;
  ulDpmNetxVersion = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptDpmCfg->ulDpm_netx_version));

  if( HBOOT_DPM_NETX4000_COOKIE == ulDpmNetxVersion )
  {
    /* This is a netX4000 */
    ptDevInstance->eChipType = eCHIP_TYPE_NETX4000;
    iRet = 1;
  } else if( HBOOT_DPM_NETX4100_COOKIE == ulDpmNetxVersion )
  {
    /* This is a netX4100 */
    ptDevInstance->eChipType = eCHIP_TYPE_NETX4100;
    iRet = 1;
  }
  return iRet;
}


/*****************************************************************************/
/*! Detect a running netX90 firmware via DPM mapped config registers
*   In case of successful detection, device instance member eChipType is set.
*
*   \param ptDevInstance        Current device instance
*   \return !=0 if netX90 has been detected                                  */
/*****************************************************************************/
int IsNetX90FLASH(PDEVICEINSTANCE ptDevInstance)
{
  int iRet = 0;
  uint32_t ulDpmNetxVersion = 0;

  /* Mask out netX90 specific differentiation */
  uint32_t ulMsk = ~((uint32_t)MSK_HBOOT_DPM_NETX90_TYPE | MSK_HBOOT_DPM_NETX90_ROMSTEP);

  /* Use the netX global register block to detect the netX chip */
  /* Note: the pointer to the global register block is set in cifXStartDevice() */

  /* ulDpm_netx_version in register block (end of DPM memory) */
  ulDpmNetxVersion = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptDevInstance->ptGlobalRegisters->reserved6));

  /* Check for known version/cookie */
  if( HBOOT_DPM_NETX90_COOKIE == (ulMsk & ulDpmNetxVersion))
  {
    ptDevInstance->eChipType = eCHIP_TYPE_NETX90;
    iRet = 1;
  }

  return iRet;
}

/*****************************************************************************/
/*! Detect a running netX90 ROM code  via DPM mapped config registers
*   In case of successful detection, device instance member eChipType is set.
*
*   \param ptDevInstance        Current device instance
*   \return !=0 if netX90 has been detected                                  */
/*****************************************************************************/
int IsNetX90ROM(PDEVICEINSTANCE ptDevInstance)
{
  int iRet = 0;
  uint32_t ulDpmNetxVersion = 0;

  /* Mask out netX90 specific differentiation */
  uint32_t ulMsk = ~((uint32_t)MSK_HBOOT_DPM_NETX90_TYPE | MSK_HBOOT_DPM_NETX90_ROMSTEP);

  /* When checking at DPM start, also look for ROMcode cookie 'NXBL' at Offset 0x100 */
  uint32_t ulCookie = 0;
  HBOOT_V2_DPM_CFG_AREA_T* ptDpmCfg = (HBOOT_V2_DPM_CFG_AREA_T*)ptDevInstance->pbDPM;

  ulDpmNetxVersion = LE32_TO_HOST(HWIF_READ32(ptDevInstance, ptDpmCfg->ulDpm_netx_version));

  HWIF_READN(ptDevInstance, &ulCookie, (ptDevInstance->pbDPM + HBOOT_V2_DPM_ID_ADR), sizeof(ulCookie));

  if((HBOOT_DPM_NETX90_COOKIE       == (ulMsk & ulDpmNetxVersion)) &&
     (HOST_TO_LE32(HBOOT_V2_DPM_ID) == ulCookie) )
  {
    ptDevInstance->eChipType = eCHIP_TYPE_NETX90;
    iRet = 1;
  }

  return iRet;
}