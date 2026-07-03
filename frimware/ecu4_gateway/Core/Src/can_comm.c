#include "can_comm.h"
#include "gateway_data.h"
#include "uart_comm.h"
#include "can.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>

void CAN_Filter_Config(void) {

    CAN_FilterTypeDef canFilterConfig;
    canFilterConfig.FilterBank = 0;
    canFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    canFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

    canFilterConfig.FilterIdHigh = 0x0000;
    canFilterConfig.FilterIdLow = 0x0000;
    canFilterConfig.FilterMaskIdHigh = 0x0000;
    canFilterConfig.FilterMaskIdLow = 0x0000;

    canFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    canFilterConfig.FilterActivation = ENABLE;
    canFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan1, &canFilterConfig) != HAL_OK) {
            Error_Handler();
        }
}

void CAN_ProcessRxMessage(void) {
    CAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[8];

    while (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0) {
        if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {

            printf("[CAN RX DEBUG] 수신된 메시지 ID: 0x%03X\r\n", (unsigned int)rxHeader.StdId);

            if (xSemaphoreTake(gatewayDataMutex, portMAX_DELAY) == pdTRUE) {
                switch (rxHeader.StdId) {
                    case 0x100: // ControlData (ECU3)
                        gatewayData.controlData.riskLevel = rxData[0];
                        gatewayData.controlData.brakeLevel = rxData[1];
                        gatewayData.controlData.wiperState = rxData[2];
                        gatewayData.controlData.ledState = rxData[3];
                        printf("[CAN RX] ECU3(제어) 수신 완료\r\n");
                        break;

                    case 0x200: // DrivingData (ECU2)
                        gatewayData.drivingData.speed = rxData[0];
                        // ⭐️ 엔디안(상/하위 바이트) 조립 순서 버그 수정 완료
                        gatewayData.drivingData.distance = (rxData[2] << 8) | rxData[1];
                        printf("[CAN RX] ECU2(주행) 정상 수신! 속도:%d, 거리:%d\r\n",
                                gatewayData.drivingData.speed, gatewayData.drivingData.distance);
                        break;

                    case 0x300: // SensorData (ECU1)
                        gatewayData.sensorData.temp = (float)rxData[0];
                        gatewayData.sensorData.humi = (float)rxData[1];
                        gatewayData.sensorData.lux = (rxData[2] << 8) | rxData[3];
                        printf("[CAN RX] ECU1(환경) 수신 완료\r\n");
                        break;

                    case 0x701: gatewayData.heartbeat1 = 1; break;
                    case 0x702: gatewayData.heartbeat2 = 1; break;
                    case 0x703: gatewayData.heartbeat3 = 1; break;
                }
                xSemaphoreGive(gatewayDataMutex);
            }
        }
        HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

    }

    // ⭐️ [추가] 하드웨어 버퍼를 모두 깨끗하게 비웠으므로 다시 초인종(인터럽트)을 켭니다!
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void CAN_SendCommand(void) {
    CAN_TxHeaderTypeDef txHeader;
    uint8_t txData[8] = {0};
    uint32_t txMailbox;

    txHeader.StdId = 0x400;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.IDE = CAN_ID_STD;
    txHeader.DLC = 8;

    txData[0] = parsedCommand.commandId;
    txData[1] = parsedCommand.targetEcu;
    for (int i = 0; i < 6; i++) {
        txData[2 + i] = parsedCommand.parameters[i];
    }

    printf("[CAN TX] 제어 명령 하달 - CMD:0x%02X, TARGET:ECU%d\r\n", txData[0], txData[1]);
    HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox);
}

void CAN_SendHeartbeat(void) {
    CAN_TxHeaderTypeDef txHeader;
    uint8_t txData[1] = {1};
    uint32_t txMailbox;

    txHeader.StdId = 0x704;
    txHeader.DLC = 1;
    txHeader.TransmitGlobalTime = DISABLE;

    HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox);
}
