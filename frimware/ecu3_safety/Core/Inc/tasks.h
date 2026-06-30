/*
 * tasks.h
 *
 *  Created on: 2026. 6. 30.
 *      Author: rlaek
 */

#ifndef INC_TASKS_H_
#define INC_TASKS_H_

// freertos.c가 감지할 수 있도록 스레드 작업 루틴 예고 명세
void StartTask_CAN_RX(void *argument);
void StartTask_Control(void *argument);
void StartTask_DTC_Log(void *argument);
void StartTask_LED(void *argument);
void StartTask_CAN_TX(void *argument);
void StartTask_Heartbeat(void *argument);

#endif /* INC_TASKS_H_ */
