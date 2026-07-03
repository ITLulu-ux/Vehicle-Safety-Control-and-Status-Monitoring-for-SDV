/*
 * control_data.h
 *
 *  Created on: 2026. 6. 30.
 *      Author: rlaek
 */

#ifndef INC_CONTROL_DATA_H_
#define INC_CONTROL_DATA_H_

#include "cmsis_os2.h"
#include <stdint.h>

// 위험도 레벨 열거형 정의
typedef enum {
    RISK_SAFE = 0,
    RISK_CAUTION,
    RISK_WARNING,
    RISK_DANGER
} RiskLevel_t;

// ECU3 제어 상태 구조체
typedef struct
{
    uint8_t riskLevel;
    uint8_t brakeLevel;
    uint8_t wiperState;
    uint8_t ledState;
} ControlData_t;

// ECU1 환경 데이터 구조체
typedef struct {
	uint8_t temperature;
    uint8_t humidity;
    uint16_t lux;
} SensorData_t;

// ECU2 주행 데이터 구조체
typedef struct {
    uint8_t speed;
    uint16_t distance;
} DrivingData_t;

// 전역 변수 extern 선언
extern ControlData_t controlData;
extern SensorData_t sensorData;
extern DrivingData_t drivingData;

// OS 뮤텍스 핸들러 extern 선언
extern osMutexId_t Mutex_RiskDataHandle;
extern osMutexId_t Mutex_I2CHandle;

#endif /* INC_CONTROL_DATA_H_ */
