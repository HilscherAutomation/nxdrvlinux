/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: HilPCIDefs.h 13179 2019-09-02 11:35:36Z LuisContreras $:

  Description:
    cifX PCI definitions file

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2019-09-01  Updated CIFX M2 IDs
    2019-08-07  Removed packing macros (if needed, see cifXUser.h)
    2019-08-06  CIFX M2 added
    2018-10-12  CIFX4000 added
    2010-10-02  netJACK 100 added
    2009-09-04  PCI information for PLX based devices added
    2009-07-15  created

**************************************************************************************/

/* prevent multiple inclusion */
#ifndef __CIFX_PCI_DEFS_H
#define __CIFX_PCI_DEFS_H

/*****************************************************************************/
/*! CIFX PCI information                                                     */
/*****************************************************************************/
/* Default Hilscher PCI Information (netX chip only) */
#define HILSCHER_PCI_VENDOR_ID              0x15CF
#define HILSCHER_PCI_SUBSYSTEM_VENDOR_ID    0x15CF

#define NETX_CHIP_PCI_DEVICE_ID             0x0000
#define NETX_CHIP_PCI_SUBYSTEM_ID           0x0000

/* PCI Information for CIFX50-FB and CIFX50-RE cards */
#define CIFX50_PCI_DEVICE_ID                0x0000
#define CIFX50_PCI_SUBYSTEM_ID              0x0000

/* PCI Information for CIFX M2 cards */
#define CIFXM2_PCI_DEVICE_ID                0x0090
#define CIFXM2_PCI_SUBYSTEM_ID_FLASH        0x6001
#define CIFXM2_PCI_SUBYSTEM_ID_GPIO         0x1002

/* PCI Information for CIFX4000 cards */
#define CIFX4000_PCI_DEVICE_ID              0x4000
#define CIFX4000_PCI_SUBVENDOR_ID           0x0000
#define CIFX4000_PCI_SUBYSTEM_ID_FLASH      0x0000

/* PCI Information for NETPLC100C-FB and NETPLC100C-RE cards */
#define NETPLC100C_PCI_DEVICE_ID            0x0010
#define NETPLC100C_PCI_SUBYSTEM_ID_RAM      0x0000
#define NETPLC100C_PCI_SUBYSTEM_ID_FLASH    0x0001

/* PCI Information for netJACK100 cards */
#define NETJACK100_PCI_DEVICE_ID            0x0020
#define NETJACK100_PCI_SUBYSTEM_ID_RAM      0x0000
#define NETJACK100_PCI_SUBYSTEM_ID_FLASH    0x0001

/* PLX Technology PCI Information */
#define PLX_PCI_VENDOR_ID                   0x10B5
#define PCI9030_DEVICE_ID                   0x9030

/* PCI Information for NXSB-PCA / NX-PCA-PCI cards */
#define NXPCAPCI_REV2_SUBSYS_ID             0x3235
#define NXPCAPCI_REV3_SUBSYS_ID             0x3335

#endif  /* __CIFX_PCI_DEFS_H */
