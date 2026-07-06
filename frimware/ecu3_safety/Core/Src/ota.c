#include "ota.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"

static uint32_t ota_file_size = 0U;
static uint32_t ota_received_bytes = 0U;
static uint32_t ota_write_offset = 0U;
static uint8_t ota_flash_unlocked = 0U;

static void OTA_UnlockFlash(void)
{
    if (ota_flash_unlocked == 0U) {
        if (HAL_FLASH_Unlock() == HAL_OK) {
            ota_flash_unlocked = 1U;
        }
    }
}

static HAL_StatusTypeDef OTA_EraseSlot(void)
{
    FLASH_EraseInitTypeDef erase = {0};
    uint32_t sectorError = 0U;

    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector = FLASH_SECTOR_7;
    erase.NbSectors = 1U;

    return HAL_FLASHEx_Erase(&erase, &sectorError);
}

void OTA_Start(const uint8_t *rx)
{
    uint32_t size;

    if (rx == NULL) {
        return;
    }

    size = (uint32_t)rx[2]
         | ((uint32_t)rx[3] << 8)
         | ((uint32_t)rx[4] << 16)
         | ((uint32_t)rx[5] << 24);

    if ((size == 0U) || (size > OTA_SLOT_SIZE)) {
        return;
    }

    ota_file_size = size;
    ota_received_bytes = 0U;
    ota_write_offset = 0U;

    OTA_UnlockFlash();
    if (ota_flash_unlocked == 0U) {
        return;
    }

    if (OTA_EraseSlot() != HAL_OK) {
        HAL_FLASH_Lock();
        ota_flash_unlocked = 0U;
        return;
    }

    ota_mode_active = 1U;
}

void OTA_WriteChunk(const uint8_t *rx)
{
    uint32_t bytesToWrite;
    uint32_t addr;
    uint32_t i;

    if ((rx == NULL) || (ota_mode_active == 0U) || (ota_write_offset >= ota_file_size)) {
        return;
    }

    if (ota_flash_unlocked == 0U) {
        return;
    }

    bytesToWrite = 6U;
    if ((ota_write_offset + bytesToWrite) > ota_file_size) {
        bytesToWrite = ota_file_size - ota_write_offset;
    }

    addr = OTA_SLOT_ADDR + ota_write_offset;
    for (i = 0U; i < bytesToWrite; i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr + i, rx[2U + i]) != HAL_OK) {
            return;
        }
    }

    ota_write_offset += bytesToWrite;
    ota_received_bytes += bytesToWrite;
}

void OTA_End(const uint8_t *rx)
{
    (void)rx;

    if (ota_flash_unlocked != 0U) {
        HAL_FLASH_Lock();
        ota_flash_unlocked = 0U;
    }

    if (ota_received_bytes != ota_file_size) {
        /* 1차: 바이트 수 불일치 — ST-Link로 Sector 7 확인 */
    }

    ota_mode_active = 0U;
    ota_file_size = 0U;
    ota_received_bytes = 0U;
    ota_write_offset = 0U;
}
