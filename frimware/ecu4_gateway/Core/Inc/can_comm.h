#ifndef CAN_COMM_H
#define CAN_COMM_H

#include "main.h"
#include "stm32f4xx_hal.h"
#include "can.h"

#define CAN_ID_CMD_DOWNLINK   0x400U
#define CAN_ID_CMD_GATEWAY    0x500U
#define CAN_ID_HB_ECU1        0x701U
#define CAN_ID_HB_ECU2        0x702U
#define CAN_ID_HB_ECU3        0x703U
#define CAN_ID_HB_GATEWAY     0x704U

void CAN_Filter_Config(void);
void CAN_ProcessRxMessage(void);
void CAN_ApplyDownlinkFromPayload(const uint8_t *rxData);
void CAN_RelayParsedCommand(void);
void CAN_SendCommand(void);
void CAN_SendHeartbeat(void);

#endif /* CAN_COMM_H */
