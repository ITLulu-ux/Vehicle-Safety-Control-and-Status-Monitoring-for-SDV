#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include "cmsis_os.h"
#include <stdint.h>

typedef struct
{
    float temperature;
    float humidity;
    uint16_t lux;

} SensorData_t;

extern QueueHandle_t sensorQueue;
// CAN RX Binary Semaphore
extern SemaphoreHandle_t canRxSemaphore;

#endif
