/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_BOOTLOADER Functions
 * @brief HAL code to interface to the OpenPilot AHRS module
 * @{
 *
 * @file       pios_bl_helper.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Bootloader Helper Functions
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <pios.h>

#ifdef PIOS_INCLUDE_BL_HELPER

#include <pios_board_info.h>
#include <stm32f0xx_flash.h>
#include <stdbool.h>

uint8_t *PIOS_BL_HELPER_FLASH_If_Read(uint32_t SectorAddress)
{
    return (uint8_t *)(SectorAddress);
}

#if defined(PIOS_INCLUDE_BL_HELPER_WRITE_SUPPORT)

static bool erase_flash(uint32_t startAddress, uint32_t endAddress);

uint8_t PIOS_BL_HELPER_FLASH_Ini()
{
    FLASH_Unlock();
    return 1;
}

uint8_t PIOS_BL_HELPER_FLASH_Start()
{
    const struct pios_board_info *bdinfo = &pios_board_info_blob;
    uint32_t startAddress = bdinfo->fw_base;
    uint32_t endAddress   = bdinfo->fw_base + bdinfo->fw_size + bdinfo->desc_size;

    bool success = erase_flash(startAddress, endAddress);

    return (success) ? 1 : 0;
}

uint8_t PIOS_BL_HELPER_FLASH_Erase_Bootloader()
{
/// Bootloader memory space erase
    uint32_t startAddress = BL_BANK_BASE;
    uint32_t endAddress   = BL_BANK_BASE + BL_BANK_SIZE;

    bool success = erase_flash(startAddress, endAddress);

    return (success) ? 1 : 0;
}

static bool erase_flash(uint32_t startAddress, uint32_t endAddress)
{
    uint32_t pageAddress = startAddress;
    uint8_t fail = false;

    while ((pageAddress < endAddress) && (fail == false)) {
        for (int retry = 0; retry < MAX_DEL_RETRYS; ++retry) {
            if (FLASH_ErasePage(pageAddress) == FLASH_COMPLETE) {
                fail = false;
                break;
            } else {
                fail = true;
            }
        }
        pageAddress += 1024;
    }
    return !fail;
}

#endif /* if defined(PIOS_INCLUDE_BL_HELPER_WRITE_SUPPORT) */

uint32_t PIOS_BL_HELPER_CRC_Memory_Calc()
{
    const struct pios_board_info *bdinfo = &pios_board_info_blob;

    PIOS_BL_HELPER_CRC_Ini();
    CRC_ResetDR();
    CRC_CalcBlockCRC((uint32_t *)bdinfo->fw_base, (bdinfo->fw_size) >> 2);
    return CRC_GetCRC();
}

void PIOS_BL_HELPER_FLASH_Read_Description(uint8_t *array, uint8_t size)
{
    const struct pios_board_info *bdinfo = &pios_board_info_blob;
    uint8_t x = 0;

    if (size > bdinfo->desc_size) {
        size = bdinfo->desc_size;
    }
    for (uint32_t i = bdinfo->fw_base + bdinfo->fw_size; i < bdinfo->fw_base + bdinfo->fw_size + size; ++i) {
        array[x] = *PIOS_BL_HELPER_FLASH_If_Read(i);
        ++x;
    }
}

void PIOS_BL_HELPER_CRC_Ini()
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
}

#endif /* PIOS_INCLUDE_BL_HELPER */
