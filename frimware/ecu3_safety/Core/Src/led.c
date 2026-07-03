/*
 * led.c
 *
 *  Created on: 2026. 6. 30.
 *      Author: rlaek
 */

#include "led.h"
#include "gpio.h"
#include "control_data.h"

/**
  * @brief 위험도 단계를 판독하여 PC13 온보드 LED의 물리 상태 제어
  */
void Control_LED_Alert(void)
{
    uint8_t current_risk = RISK_SAFE;

    if (osMutexAcquire(Mutex_RiskDataHandle, osWaitForever) == osOK)
    {
        current_risk = controlData.riskLevel;
        osMutexRelease(Mutex_RiskDataHandle);
    }

    if (current_risk == RISK_DANGER) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // 고속 깜빡임 유도용 토글
        controlData.ledState = 2;
    } else if (current_risk == RISK_WARNING) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        controlData.ledState = 1;
    } else {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // 완전 소등(안전)
        controlData.ledState = 0;
    }
}
