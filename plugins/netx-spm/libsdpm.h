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

/******************************************************************************/
/*! Initializes a cifX device for SDPM (usage of SPI).
 *   \param pszSPIDevice  Name of spidev device file (optional)
 *   \param bMode         SPI mode
 *   \param bBits         Bits per SPI word
 *   \param ulFrequency   SPI frequency
 *   \param pszIRQFile    Name of IRQ trigger file (optional)
 *   \return  Pointer to initialized cifX device on success
 *            NULL on error                                                   */
/******************************************************************************/
struct CIFX_DEVICE_T* SDPMInit(uint8_t *pszSPIDevice, uint8_t bMode, uint8_t bBits, uint32_t ulFrequency, uint8_t *pszIRQFile, uint32_t ulChunkSize, uint8_t bCSChange);
void                  SDPMDeInit( struct   CIFX_DEVICE_T* ptDevice);
