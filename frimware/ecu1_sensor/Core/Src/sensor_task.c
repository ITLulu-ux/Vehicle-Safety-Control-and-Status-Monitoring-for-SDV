#include "sensor_task.h"
#include "sensor_data.h"
#include "dht11.h"
#include "bh1750.h"
#include "tim.h"
#include "i2c.h"

void SensorTask(void const *argument)
{
    DHT11_HandleTypeDef hdht11;
    SensorData_t sensorData;

    float lux = 0.0f;

    DHT11_Init(&hdht11,
               DHT11_DATA_GPIO_Port,
               DHT11_DATA_Pin,
               &htim2);

    osDelay(2000);

    for(;;)
    {
//    	if(DHT11_Read(&hdht11))
//    	{
//    	    sensorData.temperature = hdht11.temperature;
//    	    sensorData.humidity = hdht11.humidity;
//
//    	    printf("DHT11 OK\r\n");
//    	}
//    	else
//    	{
//    	    printf("DHT11 Fail\r\n");
//    	}
//    	taskENTER_CRITICAL();

    	uint8_t ok = DHT11_Read(&hdht11);

//    	taskEXIT_CRITICAL();
    	if(ok)
    	{
    	    sensorData.temperature = hdht11.temperature;
    	    sensorData.humidity = hdht11.humidity;
    	}
    	else
    	{
    	    osDelay(2000);
    	    continue;
    	}


        if(BH1750_ReadLight(&lux) == BH1750_OK)
        {
            sensorData.lux = (uint16_t)lux;
        }

        xQueueOverwrite(sensorQueue, &sensorData);

        osDelay(2000);
    }
}
