
#include "sensor_data.h"

/* 공유 환경 데이터 */
SensorData_t sensorData;


/* Sensor Queue */
QueueHandle_t sensorQueue;


/* CAN RX Binary Semaphore*/
SemaphoreHandle_t canRxSemaphore;
