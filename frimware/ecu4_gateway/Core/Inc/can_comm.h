#ifndef CAN_COMM_H
#define CAN_COMM_H

#include "main.h" 
#include "stm32f4xx_hal.h" 
#include "can.h"          

// CAN 통신 관련 함수 원형 선언
void CAN_Filter_Config(void);
void CAN_ProcessRxMessage(void);
void CAN_SendCommand(void);
void CAN_SendHeartbeat(void);

#endif // CAN_COMM_H