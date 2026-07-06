#include "can_task.h"
#include "sensor_data.h"
#include "can_comm.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "cmsis_os.h"

void CanTxTask(void const *argument)
{
    SensorData_t sensorData;

    for (;;)
    {
        if (ota_mode_active != 0U) {
            osDelay(100);
            continue;
        }

        if (xQueueReceive(sensorQueue,
                         &sensorData,
                         portMAX_DELAY) == pdPASS)
        {
            CAN_SendSensorData(&sensorData);
        }
    }
}
