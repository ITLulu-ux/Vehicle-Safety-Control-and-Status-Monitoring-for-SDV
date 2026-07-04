#ifndef CAN_COMM_H
#define CAN_COMM_H

#include "main.h"
#include "sensor_data.h"

#define CAN_ID_CMD_DOWNLINK  0x400U
#define CAN_ID_HB_ECU1       0x701U
#define ECU1_TARGET_ID       0x01U

void CAN_Init(void);
void CAN_RX_Task_Run(void);
void Heartbeat_Task_Run(void);
HAL_StatusTypeDef CAN_SendSensorData(SensorData_t *data);

extern volatile uint8_t ota_mode_active;

#endif
