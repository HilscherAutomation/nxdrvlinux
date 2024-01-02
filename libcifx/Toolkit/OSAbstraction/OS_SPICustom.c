/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: OS_SPICustom.c 12305 2018-08-09 12:54:55Z Robert $:

  Description:
    Implementation of the custom SPI abstration layer

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2018-07-26  Added return value to OS_SpiInit()
    2014-08-27  created

**************************************************************************************/

/*****************************************************************************/
/*! \file OS_SPICustom.c
*    Sample SPI abstraction layer. Implementation must be done
*    according to used target system                                         */
/*****************************************************************************/

#include "OS_Spi.h"

#ifdef CIFX_TOOLKIT_HWIF
  #error "Implement SPI target system abstraction in this file"
#endif

/*****************************************************************************/
/*!  \addtogroup CIFX_TK_OS_ABSTRACTION Operating System Abstraction
*    \{                                                                      */
/*****************************************************************************/


/*****************************************************************************/
/*! Initialize SPI components
*   \param pvOSDependent OS Dependent parameter
*   \return CIFX_NO_ERROR on success                                         */
/*****************************************************************************/
long OS_SpiInit(void* pvOSDependent)
{
  /* initialize SPI device */
  return 0;
}

/*****************************************************************************/
/*! Assert chip select
*   \param pvOSDependent OS Dependent parameter to identify card             */
/*****************************************************************************/
void OS_SpiAssert(void* pvOSDependent)
{
  /* assert chip select */
}

/*****************************************************************************/
/*! Deassert chip select
*   \param pvOSDependent OS Dependent parameter to identify card             */
/*****************************************************************************/
void OS_SpiDeassert(void* pvOSDependent)
{
  /* deassert chip select */
}

/*****************************************************************************/
/*! Lock the SPI bus
*   \param pvOSDependent OS Dependent parameter                              */
/*****************************************************************************/
void OS_SpiLock(void* pvOSDependent)
{
  /* lock access to SPI device */
}

/*****************************************************************************/
/*! Unlock the SPI bus
*   \param pvOSDependent OS Dependent parameter                              */
/*****************************************************************************/
void OS_SpiUnlock(void* pvOSDependent)
{
  /* unlock access to SPI device */
}

/*****************************************************************************/
/*! Transfer byte stream via SPI
*   \param pvOSDependent OS Dependent parameter to identify card
*   \param pbSend        Send buffer (NULL for polling)
*   \param pbRecv        Receive buffer (NULL if discard)
*   \param ulLen         Length of SPI transfer                              */
/*****************************************************************************/
void OS_SpiTransfer(void* pvOSDependent, uint8_t* pbSend, uint8_t* pbRecv, uint32_t ulLen)
{

}
/*****************************************************************************/
/*! \}                                                                       */
/*****************************************************************************/
