#include "sensor_task.h"

#include "main.h"
#include "i2c.h"
#include "tim.h"

#include "sensor_data.h"
#include "dht11.h"
#include "bh1750.h"


void SensorTask(void const *argument)
{
    DHT11_HandleTypeDef hdht11;
    SensorData_t sensorData;

    float lux = 0.0f;

    DHT11_Init(&hdht11, DHT11_GPIO_Port, DHT11_Pin, &htim2);
    BH1750_Init(&hi2c1);

    for (;;)
    {
        if (DHT11_Read(&hdht11))
        {
            sensorData.temperature = hdht11.temperature;
            sensorData.humidity = hdht11.humidity;
        }

        if (BH1750_ReadLight(&lux) == BH1750_OK)
        {
            sensorData.lux = (uint16_t)lux;
        }

        xQueueOverwrite(sensorQueue, &sensorData); //freertos.c에서 sensorQueue 정의

        osDelay(100);
    }
}
