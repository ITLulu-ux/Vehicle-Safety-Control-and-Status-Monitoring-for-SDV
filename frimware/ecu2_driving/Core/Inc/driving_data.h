/*
 * driving_data.h
 *
 *  Created on: 2026. 6. 30.
 *      Author: itbank405_12
 */

#ifndef INC_DRIVING_DATA_H_
#define INC_DRIVING_DATA_H_

#include "stdint.h"     // 정수형 자료형 사용을 위해
#include "cmsis_os.h"   // Mutex 사용을 위해

// 1. 공유 데이터 구조체 정의
typedef struct {
    uint8_t speed;
    uint16_t distance;
} DrivingData_t;

// 2. 다른 파일에서도 이 변수들을 쓸 수 있도록 extern 선언
extern DrivingData_t drivingData;
extern uint16_t ultrasonic_distance;
extern osMutexId drivingMutexHandle; // (이름은 기존에 설정하신 대로)

#endif /* INC_DRIVING_DATA_H_ */
