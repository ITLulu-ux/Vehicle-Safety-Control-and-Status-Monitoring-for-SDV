#include "can_comm.h"
#include "gateway_data.h"
#include "uart_comm.h"
#include "heartbeat.h"
#include "can.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>

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

void CAN_ApplyDownlinkFromPayload(const uint8_t *rxData)
{
    parsedCommand.commandId = rxData[0];
    parsedCommand.targetEcu = rxData[1];
    memcpy(parsedCommand.parameters, &rxData[2], 6);

    printf("[CAN RX] 0x500 명령 수신 -> ECU%d 릴레이 (CMD:0x%02X)\r\n",
           parsedCommand.targetEcu, parsedCommand.commandId);

    CAN_RelayParsedCommand();
}

void CAN_RelayParsedCommand(void)
{
    if (parsedCommand.targetEcu == 4U) {
        OTA_HandleLocalCommand(&parsedCommand);
        return;
    }

    if (parsedCommand.targetEcu >= 1U && parsedCommand.targetEcu <= 3U) {
        CAN_SendCommand();
        return;
    }

    printf("[CAN TX ERROR] 잘못된 TARGET ECU: %d\r\n", parsedCommand.targetEcu);
}

void CAN_ProcessRxMessage(void) {
    CAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[8];

    while (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0) {
        if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {

            if (rxHeader.StdId == CAN_ID_CMD_GATEWAY) {
                CAN_ApplyDownlinkFromPayload(rxData);
                continue;
            }

            if (xSemaphoreTake(gatewayDataMutex, portMAX_DELAY) == pdTRUE) {
                switch (rxHeader.StdId) {
                    case 0x100:
                        gatewayData.controlData.riskLevel = rxData[0];
                        gatewayData.controlData.brakeLevel = rxData[1];
                        gatewayData.controlData.wiperState = rxData[2];
                        gatewayData.controlData.ledState = rxData[3];
                        break;

                    case 0x200:
                        gatewayData.drivingData.speed = rxData[0];
                        gatewayData.drivingData.distance = (rxData[2] << 8) | rxData[1];
                        break;

                    case 0x300:
                        gatewayData.sensorData.temp = (float)rxData[0];
                        gatewayData.sensorData.humi = (float)rxData[1];
                        gatewayData.sensorData.lux = (rxData[2] << 8) | rxData[3];
                        break;

                    case CAN_ID_HB_ECU1:
                        Heartbeat_OnEcuReceived(1);
                        gatewayData.heartbeat1 = 1;
                        break;

                    case CAN_ID_HB_ECU2:
                        Heartbeat_OnEcuReceived(2);
                        gatewayData.heartbeat2 = 1;
                        break;

                    case CAN_ID_HB_ECU3:
                        Heartbeat_OnEcuReceived(3);
                        gatewayData.heartbeat3 = 1;
                        break;

                    default:
                        break;
                }
                xSemaphoreGive(gatewayDataMutex);
            }

            switch (rxHeader.StdId) {
                case 0x100:
                    printf("[CAN RX] ECU3(제어) 수신 완료\r\n");
                    break;
                case 0x200:
                    printf("[CAN RX] ECU2 속도:%d, 거리:%d\r\n",
                           rxData[0], (rxData[2] << 8) | rxData[1]);
                    break;
                case 0x300:
                    printf("[CAN RX] ECU1 T:%d H:%d L:%u\r\n",
                           rxData[0], rxData[1],
                           (unsigned int)((rxData[2] << 8) | rxData[3]));
                    break;
                case CAN_ID_HB_ECU1:
                    printf("[CAN RX] HB ECU1 alive\r\n");
                    break;
                case CAN_ID_HB_ECU2:
                    printf("[CAN RX] HB ECU2 alive\r\n");
                    break;
                case CAN_ID_HB_ECU3:
                    printf("[CAN RX] HB ECU3 alive\r\n");
                    break;
                default:
                    break;
            }
        }
    }
}

void CAN_SendCommand(void) {
    CAN_TxHeaderTypeDef txHeader = {0};
    uint8_t txData[8] = {0};
    uint32_t txMailbox;

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0U) {
        printf("[CAN TX ERROR] 메일박스 꽉 참 (0x400 송신 실패)\r\n");
        return;
    }

    txHeader.StdId = CAN_ID_CMD_DOWNLINK;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.IDE = CAN_ID_STD;
    txHeader.DLC = 8;
    txHeader.TransmitGlobalTime = DISABLE;

    txData[0] = parsedCommand.commandId;
    txData[1] = parsedCommand.targetEcu;
    memcpy(&txData[2], parsedCommand.parameters, 6);

    printf("[CAN TX] 0x400 제어 명령 하달 - CMD:0x%02X, TARGET:ECU%d\r\n",
           txData[0], txData[1]);

    if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox) != HAL_OK) {
        printf("[CAN TX ERROR] 0x400 송신 실패\r\n");
    }
}

void CAN_SendHeartbeat(void) {
    CAN_TxHeaderTypeDef txHeader = {0};
    uint8_t txData[1] = {0x01};
    uint32_t txMailbox;

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0U) {
        printf("[CAN TX ERROR] 메일박스 꽉 참 (0x704 송신 실패)\r\n");
        return;
    }

    txHeader.StdId = CAN_ID_HB_GATEWAY;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.IDE = CAN_ID_STD;
    txHeader.DLC = 1;
    txHeader.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox) != HAL_OK) {
        printf("[CAN TX ERROR] 0x704 하트비트 송신 실패\r\n");
    }
}
