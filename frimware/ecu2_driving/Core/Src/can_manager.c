/*
 * can_manager.c
 *
 *  Created on: 2026. 7. 2.
 *      Author: itbank405_12
 */


#include "can_manager.h"
#include "driving_data.h"
#include "ota.h"
#include "main.h"

extern CAN_HandleTypeDef hcan1;

static void CAN_HandleDownlink(const uint8_t *rxData)
{
	if (rxData[1] != MY_ECU_ID) {
		return;
	}

	if (ota_mode_active != 0U) {
		switch (rxData[0]) {
			case CMD_OTA_START:
			case CMD_OTA_DATA:
			case CMD_OTA_END:
				break;
			default:
				return;
		}
	}

	switch (rxData[0]) {
		case CMD_RESET:
			HAL_NVIC_SystemReset();
			break;
		case CMD_OTA_START:
			OTA_Start(rxData);
			break;
		case CMD_OTA_DATA:
			OTA_WriteChunk(rxData);
			break;
		case CMD_OTA_END:
			OTA_End(rxData);
			break;
		default:
			break;
	}
}

void CAN_RX_Task_Run(void) {
	CAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[8];

	while (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0) {
		if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, RxData)
				!= HAL_OK) {
			break;
		}
		if (RxHeader.StdId == CAN_ID_CMD_DOWNLINK) {
			CAN_HandleDownlink(RxData);
		}
	}
	osDelay(10);
}

void CAN_TX_Task_Run(void) {
    CAN_TxHeaderTypeDef TxHeader = {0};
    uint8_t TxData[3]; // DLC=3 (속도 1바이트 + 거리 2바이트)
    uint32_t TxMailbox;

    uint8_t current_speed = 0;
    uint16_t current_distance = 0;

    if (ota_mode_active != 0U) {
        osDelay(100);
        return;
    }

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

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0) {
        HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
    }

    osDelay(2000);
}

void Heartbeat_Task_Run(void) {
    CAN_TxHeaderTypeDef HeartbeatHeader = {0};
    uint8_t HeartbeatData[1] = {0x01}; // 0x01: 정상 작동 중
    uint32_t TxMailbox;

    if (ota_mode_active != 0U) {
        osDelay(2000);
        return;
    }

    HeartbeatHeader.StdId = 0x702;
    HeartbeatHeader.IDE = CAN_ID_STD;
    HeartbeatHeader.RTR = CAN_RTR_DATA;
    HeartbeatHeader.DLC = 1;

	// 메일박스에 여유가 있을 때만 전송
	if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0) {
		HAL_CAN_AddTxMessage(&hcan1, &HeartbeatHeader, HeartbeatData,
				&TxMailbox);
	}

	osDelay(2000);
}
