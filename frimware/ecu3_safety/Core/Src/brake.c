/*
 * brake.c
 *
 *  Created on: 2026. 6. 30.
 *      Author: rlaek
 */

#include "brake.h"
#include "tim.h"
#include "control_data.h"

/**
  * @brief 위험도 단계에 맞춤형 브레이크 제동 강도(Duty Cycle) 제어
  */
void Control_Brake(void)
{
    uint8_t current_risk = RISK_SAFE;

    if (osMutexAcquire(Mutex_RiskDataHandle, osWaitForever) == osOK)
    {
        current_risk = controlData.riskLevel;
        osMutexRelease(Mutex_RiskDataHandle);
    }

    // PWM Pulse 제어 (TIM4 Channel 1)
    if (current_risk == RISK_DANGER) {
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 2000); // 꽉 제동 (2.0ms)
        controlData.brakeLevel = 100;
    } else if (current_risk == RISK_WARNING) {
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 1750); // 감속 (1.75ms)
        controlData.brakeLevel = 50;
    } else {
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 1500); // 완전 해제 중립 (1.5ms)
        controlData.brakeLevel = 0;
    }
}
