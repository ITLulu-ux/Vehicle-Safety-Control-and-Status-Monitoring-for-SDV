#ifndef CAN_COMM_H
#define CAN_COMM_H

#include "main.h"
#include "stm32f4xx_hal.h"
#include "can.h"

#define CAN_ID_DOWNLINK       0x400U
#define CAN_ID_HB_ECU1        0x701U
#define CAN_ID_HB_ECU2        0x702U
#define CAN_ID_HB_ECU3        0x703U
#define CAN_ID_HB_GATEWAY     0x704U

#define CMD_RESET             0x01U
#define CMD_OTA_START         0x02U
#define CMD_OTA_DATA          0x03U
#define CMD_OTA_END           0x04U

void CAN_Filter_Config(void);
void CAN_ProcessRxMessage(void);
void CAN_TxDownlink(const uint8_t *data);
void CAN_SendHeartbeat(void);

#endif /* CAN_COMM_H */
