#ifndef OTA_H
#define OTA_H

#include <stdint.h>

#define OTA_SLOT_ADDR  0x08060000U
#define OTA_SLOT_SIZE  (128U * 1024U)

void OTA_Start(const uint8_t *rx);
void OTA_WriteChunk(const uint8_t *rx);
void OTA_End(const uint8_t *rx);

extern volatile uint8_t ota_mode_active;

#endif /* OTA_H */
