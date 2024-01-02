/**************************************************************************************

Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.

***************************************************************************************

  $Id: netx90_4x00_romloader_dpm.h 14189 2021-08-31 10:49:31Z RMayer $:

  Description:
    netX90 and netX4000 ROM Loader DPM layout

  Changes:
    Date        Description
    -----------------------------------------------------------------------------------
    2021-08-31  added some comments
    2018-11-30  initial version

**************************************************************************************/

#ifndef __NETX90_4X00_ROMLOADER_DPM_H__
#define __NETX90_4X00_ROMLOADER_DPM_H__

/*-----------------------------------------------*/
/* netX90 chip identifier definitions            */
/*-----------------------------------------------*/
#define HBOOT_DPM_NETX90_COOKIE                 0x0900000D

#define MSK_HBOOT_DPM_NETX90_TYPE               0x00FF0000
#define MSK_HBOOT_DPM_NETX90_ROMSTEP            0x0000FF00

/*-----------------------------------------------*/
/* netX4000 chip identifier definitions          */
/*-----------------------------------------------*/
#define HBOOT_DPM_NETX4000_COOKIE               0x84524C0B
#define HBOOT_DPM_NETX4100_COOKIE               0x93615B0B

/*-----------------------------------------------*/
/* netX ROM code definitions                     */
/*-----------------------------------------------*/
/* The 'NXBL' DPM boot identifier ('NXBL') is shown when the ROM code is running. */
#define HBOOT_V2_DPM_ID                         0x4c42584e
#define HBOOT_V2_DPM_ID_ADR                     0x100

/* Definition of the ROM code DPM layout and functions */
#define HBOOT_V2_DPM_NETX_TO_HOST_BUFFERSIZE    0x0200
#define HBOOT_V2_DPM_HOST_TO_NETX_BUFFERSIZE    0x0400

#define HBOOT_V2_DPM_BITFLIP_BUFFERSIZE         0xf800

#define DPM_BOOT_NETX_RECEIVED_CMD              0x01
#define DPM_BOOT_NETX_SEND_CMD                  0x02

#define DPM_BOOT_HOST_SEND_CMD                  0x01
#define DPM_BOOT_HOST_RECEIVED_CMD              0x02

#define SRT_HANDSHAKE_REG_ARM_DATA              16
#define SRT_HANDSHAKE_REG_PC_DATA               24

#define HBOOT_MSK_SYS_STA_BITFLIP               0x80

#define MSK_HBOOT_dpm_sys_sta_NETX_STA_CODE_ro  0x00ff0000U
#define SRT_HBOOT_dpm_sys_sta_NETX_STA_CODE_ro  8

#define MSK_HBOOT_dpm_status_unlocked           0x00000001U
#define SRT_HBOOT_dpm_status_unlocked           0


typedef struct HBOOT_V2_DPM_CFG_AREA_STRUCT
{
  volatile uint32_t   aulReserved0[7];
  volatile uint32_t   ulDpm_status;
  volatile uint32_t   aulReserved1[46];
  volatile uint32_t   ulDpm_sys_sta;
  volatile uint32_t   ulDpm_reset_request;
  volatile uint32_t   aulReserved5[7];
  volatile uint32_t   ulDpm_netx_version;
} HBOOT_V2_DPM_CFG_AREA_T;



typedef struct HBOOT_V2_DPM_BLOCKS_STRUCTURE
{
  volatile uint32_t   ulDpmBootId;
  volatile uint32_t   ulDpmByteSize;
  volatile uint32_t   aulReserved_08[28];
  volatile uint32_t   ulNetxToHostDataSize;
  volatile uint32_t   ulHostToNetxDataSize;
  volatile uint32_t   ulHandshake;
  volatile uint32_t   aulReserved_84[31];
  volatile uint8_t    aucNetxToHostData[HBOOT_V2_DPM_NETX_TO_HOST_BUFFERSIZE];
  volatile uint8_t    aucHostToNetxData[HBOOT_V2_DPM_HOST_TO_NETX_BUFFERSIZE];
} HBOOT_V2_DPM_BLOCKS_T;


typedef struct HBOOT_V2_DPM_AREA_STRUCTURE
{
  HBOOT_V2_DPM_CFG_AREA_T tConfigurationRegisters;
  HBOOT_V2_DPM_BLOCKS_T   tDpmBlocks;
  volatile uint8_t        aucBitflipArea[HBOOT_V2_DPM_BITFLIP_BUFFERSIZE];
} HBOOT_V2_DPM_AREA_T;


#endif /* __NETX90_4X00_ROMLOADER_DPM_H__ */
