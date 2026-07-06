/*
 * can_comm.c
 *
 *  Created on: 2026. 6. 30.
 *      Author: rlaek
 */

#include "can_comm.h"
#include "can.h"
#include "control_data.h"
#include "ota.h"

extern CAN_TxHeaderTypeDef TxHeader;
extern uint8_t TxData[8];
extern uint32_t TxMailbox;

void CAN_HandleDownlink(const uint8_t *rxData)
{
    if (rxData[1] != MY_ECU_ID) {
        return;
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

/**
  * @brief ECU3 현재 제어 상태(위험도, 브레이크, 와이퍼 등)를 외부로 송신 (0x100)
  */
void CAN_Send_Status(void)
{
    if (ota_mode_active != 0U) {
        return;
    }

    TxHeader.StdId = CAN_ID_ECU3_STATUS;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = 4; // 4바이트 전송
    TxHeader.TransmitGlobalTime = DISABLE;

    if (osMutexAcquire(Mutex_RiskDataHandle, osWaitForever) == osOK)
    {
        TxData[0] = controlData.riskLevel;
        TxData[1] = controlData.brakeLevel;
        TxData[2] = controlData.wiperState;
        TxData[3] = controlData.ledState;
        osMutexRelease(Mutex_RiskDataHandle);
    }

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0)
    {
        HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
    }
}
