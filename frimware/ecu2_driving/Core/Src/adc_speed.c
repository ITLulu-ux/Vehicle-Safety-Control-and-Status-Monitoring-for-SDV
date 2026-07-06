/*
 * adc_speed.c
 *
 *  Created on: 2026. 7. 2.
 *      Author: itbank405_12
 */


#include "adc_speed.h"
#include "driving_data.h"
#include "ota.h"
#include "main.h"
#include <stdio.h>

extern ADC_HandleTypeDef hadc1;

void Speed_Task_Run(void) {
	if (ota_mode_active != 0U) {
		osDelay(100);
		return;
	}

	// 1. ADC 변환 시작
	HAL_ADC_Start(&hadc1);

	// 2. 변환이 완료될 때까지 대기 (최대 10ms)
	if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
		// 3. 0~4095의 아날로그 값을 읽어와 0~150km/h로 스케일링(변환)
		uint32_t adc_val = HAL_ADC_GetValue(&hadc1);
		uint8_t current_speed = (uint8_t) ((adc_val * 150) / 4095);

		// 4. Mutex로 잠그고 공유 구조체 업데이트
		osMutexWait(drivingMutexHandle, osWaitForever);
		drivingData.speed = current_speed;
		osMutexRelease(drivingMutexHandle);
	}

	osDelay(2000);
}
