#ifndef CAN_COMM_H
#define CAN_COMM_H

#include "main.h"
#include "sensor_data.h"

/* CAN 초기화 */
void CAN_Init(void);

/* 환경 데이터 송신 */
void CAN_SendSensorData(const SensorData_t *data);

/* 제어 명령 수신 처리 (OTA, Reset, 설정 변경 등) */
void CAN_ProcessRxMessage(void);

/* Heartbeat 송신 */
void CAN_SendHeartbeat(void);

#endif /* CAN_COMM_H */
