/*
 * heartbeat.c
 *
 *  Created on: 2026. 6. 30.
 *      Author: rlaek
 */

#include "heartbeat.h"
#include "can.h"
#include "can_comm.h"
#include "ota.h"

extern CAN_TxHeaderTypeDef TxHeader;
extern uint8_t TxData[8];
extern uint32_t TxMailbox;

/**
  * @brief 라즈베리파이(서버) 관제 감지용 1000ms 주기 생존 데이터 송출 (0x703)
  */
void Send_Heartbeat_Signal(void)
{
    if (ota_mode_active != 0U) {
        return;
    }

    TxHeader.StdId = CAN_ID_ECU3_HB;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = 1;
    TxHeader.TransmitGlobalTime = DISABLE;

    TxData[0] = 0xAA; // 무사 정상 가동 바이트 시그널

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0)
    {
        HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
    }
}
