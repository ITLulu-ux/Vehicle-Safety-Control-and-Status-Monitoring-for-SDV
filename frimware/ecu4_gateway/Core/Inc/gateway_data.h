#ifndef GATEWAY_DATA_H
#define GATEWAY_DATA_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"

// 1. ECU1(환경 ECU) 데이터 구조체
typedef struct {
    float temp;       // 온도
    float humi;       // 습도
    uint16_t lux;     // 조도
} SensorData_t;

// 2. ECU2(주행 ECU) 데이터 구조체
typedef struct {
    uint8_t speed;    // 차량 속도 (가변저항)
    uint16_t distance; // 전방 장애물 거리 (초음파)
} DrivingData_t;

// 3. ECU3(안전제어 ECU) 제어 상태 구조체
typedef struct {
    uint8_t riskLevel;   // 위험도 단계 (SAFE, CAUTION, WARNING, DANGER)
    uint8_t brakeLevel;  // 브레이크 작동 상태
    uint8_t wiperState;  // 와이퍼 작동 상태
    uint8_t ledState;    // 경고 LED 상태
} ControlData_t;

// 4. ECU4(Gateway ECU) 최종 통합 데이터 구조체
typedef struct {
    SensorData_t sensorData;
    DrivingData_t drivingData;
    ControlData_t controlData;
    
    // 타 ECU 생존 상태(Heartbeat) 확인용 플래그
    uint8_t heartbeat1;
    uint8_t heartbeat2;
    uint8_t heartbeat3;
} GatewayData_t;

// 전역 변수 외부 참조 선언 (메모리 할당은 freertos.c에서 수행)
extern GatewayData_t gatewayData;
extern SemaphoreHandle_t gatewayDataMutex;

#endif // GATEWAY_DATA_H