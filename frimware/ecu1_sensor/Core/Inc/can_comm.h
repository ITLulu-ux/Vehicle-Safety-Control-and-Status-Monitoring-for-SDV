#ifndef CAN_COMM_H
#define CAN_COMM_H

#include "main.h"
#include "sensor_data.h"

void CAN_Init(void);
HAL_StatusTypeDef CAN_SendSensorData(SensorData_t *data);

#endif
