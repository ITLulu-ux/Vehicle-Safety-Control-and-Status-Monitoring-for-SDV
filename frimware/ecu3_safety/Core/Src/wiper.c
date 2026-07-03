/*
 * wiper.c
 *
 *  Created on: 2026. 6. 30.
 *      Author: rlaek
 */

#include "wiper.h"
#include "tim.h"
#include "control_data.h"

/**
  * @brief ECU1의 습도 정보 및 현재 위험 상태에 따라 와이퍼 구동 속도 자동 제어
  */
void Control_Wiper(void)
{
	uint8_t temperature = 0;
    uint8_t humidity = 0;
    uint8_t current_risk = RISK_SAFE;

    if (osMutexAcquire(Mutex_RiskDataHandle, osWaitForever) == osOK)
    {
    	temperature = sensorData.temperature;
        humidity = sensorData.humidity;
        current_risk = controlData.riskLevel;
        osMutexRelease(Mutex_RiskDataHandle);
    }

    // PWM Pulse 제어 (TIM4 Channel 2)
    if (temperature > 28 || humidity > 80 || current_risk == RISK_DANGER) {
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 1800); // 고속 와이핑
        controlData.wiperState = 1;
    } else {
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 1500); // 정지
        controlData.wiperState = 0;
    }
}
