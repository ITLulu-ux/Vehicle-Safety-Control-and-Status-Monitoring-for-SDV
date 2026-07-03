

#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"


typedef struct
{
	uint8_t temperature;
	uint8_t humidity;
    uint16_t lux;

} SensorData_t;

extern QueueHandle_t sensorQueue;
#endif
