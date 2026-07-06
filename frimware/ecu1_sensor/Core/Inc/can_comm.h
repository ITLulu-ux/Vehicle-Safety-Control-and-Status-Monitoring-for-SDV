#ifndef CAN_COMM_H
#define CAN_COMM_H

#include "main.h"
#include "sensor_data.h"
#include "downlink.h"

#define CAN_ID_CMD_DOWNLINK  CAN_ID_DOWNLINK
#define CAN_ID_HB_ECU1       0x701U
#define ECU1_TARGET_ID       MY_ECU_ID

void CAN_Init(void);
void CAN_RX_Task_Run(void);
void Heartbeat_Task_Run(void);
HAL_StatusTypeDef CAN_SendSensorData(SensorData_t *data);

#endif
