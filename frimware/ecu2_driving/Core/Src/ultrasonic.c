/*
 * ultrasonic.c
 *
 *  Created on: 2026. 7. 2.
 *      Author: itbank405_12
 */


#include "ultrasonic.h"
#include "driving_data.h"
#include "main.h"

void Distance_Task_Run(void) {
	// 1. 초음파 센서 Trigger 핀에 High 펄스 인가
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);

	// 2. 10us 이상 유지해야 하므로 FreeRTOS 최소 지연시간인 1ms 대기
	osDelay(1);

	// 3. Trigger 핀을 다시 Low로 변경
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);

	osDelay(50);

	// 4. 거리 구조체 업데이트
	osMutexWait(drivingMutexHandle, osWaitForever);
	drivingData.distance = ultrasonic_distance;
	osMutexRelease(drivingMutexHandle);

	osDelay(1949);
}
