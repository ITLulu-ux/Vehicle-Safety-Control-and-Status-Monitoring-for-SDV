#include "uart_comm.h"
#include "gateway_data.h"
#include "can_comm.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>

uint8_t uartRxBuffer[UART_DOWNLINK_PACKET_LEN];

// [TX] 라즈베리파이로 통합 데이터 송신 (JSON, Pi DB 연동용)
void UART_SendGatewayData(void) {
    static char json[220];
    SensorData_t sensor;
    DrivingData_t driving;
    uint8_t hb1, hb2, hb3;

    if (xSemaphoreTake(gatewayDataMutex, portMAX_DELAY) != pdTRUE) {
        return;
    }

    sensor  = gatewayData.sensorData;
    driving = gatewayData.drivingData;
    hb1     = gatewayData.heartbeat1;
    hb2     = gatewayData.heartbeat2;
    hb3     = gatewayData.heartbeat3;

    xSemaphoreGive(gatewayDataMutex);

    snprintf(json, sizeof(json),
        "{\"type\":\"sensor\",\"can_id\":0,"
        "\"temperature\":%d,\"humidity\":%d,\"lux\":%u,"
        "\"speed\":%u,\"distance\":%u,"
        "\"hb1\":%u,\"hb2\":%u,\"hb3\":%u}\r\n",
        (int)sensor.temp,
        (int)sensor.humi,
        (unsigned int)sensor.lux,
        (unsigned int)driving.speed,
        (unsigned int)driving.distance,
        (unsigned int)hb1,
        (unsigned int)hb2,
        (unsigned int)hb3);

    HAL_UART_Transmit(&huart2, (uint8_t *)json, (uint16_t)strlen(json), 200);
}

// [RX] Pi VCP 8바이트 → CAN 0x400 중계 (해석 없음)
void UART_OnPacket(void) {
    CAN_TxDownlink(uartRxBuffer);
    HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_DOWNLINK_PACKET_LEN);
}
