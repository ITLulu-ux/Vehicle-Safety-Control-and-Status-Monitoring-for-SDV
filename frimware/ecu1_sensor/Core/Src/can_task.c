#include "can_task.h"

#include "cmsis_os.h"

#include "sensor_data.h"
#include "can_comm.h"

void CanTxTask(void const *argument)
{
	SensorData_t sensorData;

	for(;;)
	{
		if (xQueueReceive(sensorQueue,
							&sensorData,
							portMAX_DELAY)==pdPASS)
		{
	CAN_SendSensorData(&sensorData);
		}
	}
}

void CanRxTask(void const *argument)
{
	for(;;)
	{
		if (xSemaphoreTake(canRxSemaphore,portMAX_DELAY)==pdTRUE)
		{
			CAN_ProcessRxMessage();
		}
	}
}
