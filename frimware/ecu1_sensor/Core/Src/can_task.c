#include "can_task.h"
#include "sensor_data.h"
#include "can_comm.h"

#include "FreeRTOS.h"
#include "queue.h"

#include <stdio.h>

void CanTxTask(void const *argument)
{
    SensorData_t sensorData;

    for(;;)
    {
        if(xQueueReceive(sensorQueue,
                         &sensorData,
                         portMAX_DELAY) == pdPASS)
        {
        	 printf("CAN Tx T=%d H=%d L=%u\r\n",
        	                   sensorData.temperature,
        	                   sensorData.humidity,
        	                   sensorData.lux);

            CAN_SendSensorData(&sensorData);
        }
    }
}
