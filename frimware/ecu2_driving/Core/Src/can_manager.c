/*
 * can_manager.c
 *
 *  Created on: 2026. 7. 2.
 *      Author: itbank405_12
 */


#include "can_manager.h"
#include "driving_data.h"
#include "main.h"
#include <stdio.h>

extern CAN_HandleTypeDef hcan1;

void CAN_RX_Task_Run(void) {
	CAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[8];

	// 1. 수신 버퍼(FIFO0)에 메시지가 들어왔는지 확인
	if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0) {
		// 2. 메시지 꺼내오기
		if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, RxData)
				== HAL_OK) {
			// 3. 수신된 CAN ID에 따라 동작 분류 (명세서 기준)
			if (RxHeader.StdId == 0x010) // 예: 게이트웨이 긴급 명령 (필요 시)
					{
				// 긴급 명령 처리 로직
			} else if (RxHeader.StdId == 0x400) // 예: OTA 펌웨어 데이터 (가정)
					{
				// OTA 처리 로직
			}
		}
	}
	osDelay(10);
}

void CAN_TX_Task_Run(void) {
    CAN_TxHeaderTypeDef TxHeader = {0};
    uint8_t TxData[3]; // DLC=3 (속도 1바이트 + 거리 2바이트)
    uint32_t TxMailbox;

    // printf 출력을 위한 안전한 지역 변수
    uint8_t current_speed = 0;
    uint16_t current_distance = 0;

    TxHeader.StdId = 0x200;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 3;

    // 1. Mutex를 통해 데이터 안전하게 복사
    osMutexWait(drivingMutexHandle, osWaitForever);
    current_speed = drivingData.speed;
    current_distance = drivingData.distance;
    osMutexRelease(drivingMutexHandle);

    // 2. CAN 송신용 바이트 배열 구성
    TxData[0] = (uint8_t) current_speed;                    // Byte 0: 속도
    TxData[1] = (uint8_t) (current_distance & 0xFF);        // Byte 1: 거리 하위 8비트
    TxData[2] = (uint8_t) ((current_distance >> 8) & 0xFF); // Byte 2: 거리 상위 8비트

    // 3. CAN 송신 (메일박스 확인)
    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0) {
        if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
            // 송신 실패 처리 (생략)
        }
    }

    // 4. 안전하게 복사된 지역 변수로 디버그 출력
    printf("[CAN TX] 송신 데이터 -> Speed: %d, Distance: %d\r\n", current_speed, current_distance);

    osDelay(500); // 100ms 주기 송신
}

void Heartbeat_Task_Run(void) {
    CAN_TxHeaderTypeDef HeartbeatHeader = {0};
    uint8_t HeartbeatData[1] = {0x01}; // 0x01: 정상 작동 중
    uint32_t TxMailbox;

    HeartbeatHeader.StdId = 0x702;
    HeartbeatHeader.IDE = CAN_ID_STD;
    HeartbeatHeader.RTR = CAN_RTR_DATA;
    HeartbeatHeader.DLC = 1;

	// 메일박스에 여유가 있을 때만 전송
	if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0) {
		HAL_CAN_AddTxMessage(&hcan1, &HeartbeatHeader, HeartbeatData,
				&TxMailbox);
	}

	osDelay(500);
}
