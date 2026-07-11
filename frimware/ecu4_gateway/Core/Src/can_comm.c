#include "can_comm.h"
#include "gateway_data.h"
#include "heartbeat.h"
#include "can.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"

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

void CAN_TxDownlink(const uint8_t *data) {
    CAN_TxHeaderTypeDef txHeader = {0};
    uint8_t txData[8];
    uint32_t txMailbox;

    if (data == NULL) {
        return;
    }

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0U) {
        printf("[CAN TX ERROR] mailbox full (0x400)\r\n");
        return;
    }

    memcpy(txData, data, 8);

    txHeader.StdId = CAN_ID_DOWNLINK;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.IDE = CAN_ID_STD;
    txHeader.DLC = 8;
    txHeader.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox) != HAL_OK) {
        printf("[CAN TX ERROR] 0x400 send failed\r\n");
    }
}

void CAN_ProcessRxMessage(void) {
    CAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[8];

    while (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0) {
        if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {

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
        }
    }
}

void CAN_SendHeartbeat(void)
{
    CAN_TxHeaderTypeDef txHeader = {0};
    uint8_t txData[1] = {0x01};
    uint32_t txMailbox;
    HAL_StatusTypeDef ret;

    char dbg[64];

//    snprintf(dbg,
//             sizeof(dbg),
//             "Heartbeat Task Running Mailbox=%lu\r\n",
//             HAL_CAN_GetTxMailboxesFreeLevel(&hcan1));
//
//    HAL_UART_Transmit(&huart2,
//                      (uint8_t *)dbg,
//                      strlen(dbg),
//                      100);

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0U)
    {
        HAL_UART_Transmit(&huart2,
                          (uint8_t *)"[HB] Mailbox Full\r\n",
                          19,
                          100);
        return;
    }

    txHeader.StdId = CAN_ID_HB_GATEWAY;
    txHeader.RTR   = CAN_RTR_DATA;
    txHeader.IDE   = CAN_ID_STD;
    txHeader.DLC   = 1;
    txHeader.TransmitGlobalTime = DISABLE;

    ret = HAL_CAN_AddTxMessage(&hcan1,
                               &txHeader,
                               txData,
                               &txMailbox);

    if (ret != HAL_OK)
    {
        snprintf(dbg,
                 sizeof(dbg),
                 "HB4 TX FAIL ret=%d\r\n",
                 ret);

        HAL_UART_Transmit(&huart2,
                          (uint8_t *)dbg,
                          strlen(dbg),
                          100);
        return;
    }

//    HAL_UART_Transmit(&huart2,
//                      (uint8_t *)"HB4 TX OK\r\n",
//                      11,
//                      100);

    Heartbeat_OnEcuReceived(4);

    if (xSemaphoreTake(gatewayDataMutex, portMAX_DELAY) == pdTRUE)
    {
        gatewayData.heartbeat4 = 1;
        xSemaphoreGive(gatewayDataMutex);
    }

//    HAL_UART_Transmit(&huart2,
//                      (uint8_t *)"HB4 SET=1\r\n",
//                      11,
//                      100);
}
//void CAN_SendHeartbeat(void) {
//    CAN_TxHeaderTypeDef txHeader = {0};
//    uint8_t txData[1] = {0x01};
//    uint32_t txMailbox;
//
//    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0U) {
//        printf("[CAN TX ERROR] mailbox full (0x704)\r\n");
//        return;
//    }
//
//    txHeader.StdId = CAN_ID_HB_GATEWAY;
//    txHeader.RTR = CAN_RTR_DATA;
//    txHeader.IDE = CAN_ID_STD;
//    txHeader.DLC = 1;
//    txHeader.TransmitGlobalTime = DISABLE;
//
//    if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox) != HAL_OK) {
//        printf("[CAN TX ERROR] 0x704 heartbeat failed\r\n");
//    } else {
//        Heartbeat_OnEcuReceived(4);
//    }
//
//    Heartbeat_OnEcuReceived(4);
//
//    if (xSemaphoreTake(gatewayDataMutex, portMAX_DELAY) == pdTRUE)
//    {
//        gatewayData.heartbeat4 = 1;
//        xSemaphoreGive(gatewayDataMutex);
//    }
//
//}


//void CAN_SendHeartbeat(void)
//{
//
////	printf("Heartbeat Task Running\r\n");
////	printf("Mailbox=%lu\r\n",
////	       HAL_CAN_GetTxMailboxesFreeLevel(&hcan1));
//	char dbg[64];
//
//	snprintf(dbg,
//	         sizeof(dbg),
//	         "Heartbeat Task Running Mailbox=%lu\r\n",
//	         HAL_CAN_GetTxMailboxesFreeLevel(&hcan1));
//
//	HAL_UART_Transmit(&huart2,
//	                  (uint8_t*)dbg,
//	                  strlen(dbg),
//	                  100);
//    CAN_TxHeaderTypeDef txHeader = {0};
//    uint8_t txData[1] = {0x01};
//    uint32_t txMailbox;
//    HAL_StatusTypeDef ret;
//
//    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0U)
//    {
//        printf("[CAN TX ERROR] mailbox full (0x704)\r\n");
//        return;
//    }
//
//    txHeader.StdId = CAN_ID_HB_GATEWAY;
//    txHeader.RTR = CAN_RTR_DATA;
//    txHeader.IDE = CAN_ID_STD;
//    txHeader.DLC = 1;
//    txHeader.TransmitGlobalTime = DISABLE;
//
//    ret = HAL_CAN_AddTxMessage(&hcan1,
//                               &txHeader,
//                               txData,
//                               &txMailbox);
//
//    if (ret != HAL_OK)
//    {
//        char dbg[64];
//
//        snprintf(dbg,
//                 sizeof(dbg),
//                 "HB4 TX FAIL ret=%d\r\n",
//                 ret);
//
//        HAL_UART_Transmit(&huart2,
//                          (uint8_t *)dbg,
//                          (uint16_t)strlen(dbg),
//                          100);
//    }
//    else
//    {
////        HAL_UART_Transmit(&huart2,
////                          (uint8_t *)"HB4 TX OK\r\n",
////                          (uint16_t)strlen("HB4 TX OK\r\n"),
////                          100);
//
//    	char dbg[64];
//
//    	snprintf(dbg,
//    	         sizeof(dbg),
//    	         "HB4 TX OK, hb4=%d\r\n",
//    	         gatewayData.heartbeat4);
//
//    	HAL_UART_Transmit(&huart2,
//    	                  (uint8_t *)dbg,
//    	                  (uint16_t)strlen(dbg),
//    	                  100);
//
//        Heartbeat_OnEcuReceived(4);
//
//        // 테스트용 : hb4를 바로 1로 설정
////        if (xSemaphoreTake(gatewayDataMutex, portMAX_DELAY) == pdTRUE)
////        {
////            gatewayData.heartbeat4 = 1;
////            xSemaphoreGive(gatewayDataMutex);
////        }
//
//
//        if (xSemaphoreTake(gatewayDataMutex, portMAX_DELAY) == pdTRUE)
//        {
//            gatewayData.heartbeat4 = 1;
//
//            snprintf(dbg,
//                     sizeof(dbg),
//                     "HB4 SET=1\r\n");
//
//            HAL_UART_Transmit(&huart2,
//                              (uint8_t *)dbg,
//                              (uint16_t)strlen(dbg),
//                              100);
//
//            xSemaphoreGive(gatewayDataMutex);
//        }
//    }
//}
