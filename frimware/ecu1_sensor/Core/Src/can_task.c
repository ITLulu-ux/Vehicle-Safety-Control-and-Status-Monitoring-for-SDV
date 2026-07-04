#include "can_task.h"
#include "sensor_data.h"
#include "can_comm.h"

#include "FreeRTOS.h"
#include "queue.h"

void CanTxTask(void const *argument)
{
    SensorData_t sensorData;

    for(;;)
    {
        if(xQueueReceive(sensorQueue,
                         &sensorData,
                         portMAX_DELAY) == pdPASS)
        {
            CAN_SendSensorData(&sensorData);
        }
    }
}
