#include "uart_comm.h"
#include "gateway_data.h"
#include "usart.h" 
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>

UartCommand_t parsedCommand;
uint8_t uartRxBuffer[32]; // 라즈베리파이 수신 버퍼

// 동기화 헤더 및 테일
const uint8_t SYNC_HEADER[2] = {0xAA, 0x55};
const uint8_t SYNC_TAIL[2]   = {0x0D, 0x0A}; // \r\n

// [TX] 라즈베리파이로 통합 데이터 송신 (Binary + DMA)
void UART_SendGatewayData(void) {
    // DMA 전송을 위한 static 버퍼 (헤더 2 + 구조체 크기 + 테일 2)
    static uint8_t txBuffer[sizeof(GatewayData_t) + 4];
    
    if (xSemaphoreTake(gatewayDataMutex, portMAX_DELAY) == pdTRUE) {
        txBuffer[0] = SYNC_HEADER[0];
        txBuffer[1] = SYNC_HEADER[1];

        // 구조체 통째로 복사
        memcpy(&txBuffer[2], &gatewayData, sizeof(GatewayData_t));

        txBuffer[2 + sizeof(GatewayData_t)] = SYNC_TAIL[0];
        txBuffer[3 + sizeof(GatewayData_t)] = SYNC_TAIL[1];

        xSemaphoreGive(gatewayDataMutex);
    }

    // DMA 전송 시작
    HAL_UART_Transmit_DMA(&huart2, txBuffer, sizeof(txBuffer));
}

// [RX] 라즈베리파이에서 들어온 제어 명령 파싱 (문자열)
void UART_ProcessRxMessage(void) {
    int cmd = 0, target = 0, param1 = 0, param2 = 0;
    
    printf("[UART RX from Pi] 수신 명령: %s\r\n", (char*)uartRxBuffer);

    if (sscanf((char*)uartRxBuffer, "CMD,%d,%d,%d,%d", &cmd, &target, &param1, &param2) >= 2) {
        parsedCommand.commandId = (uint8_t)cmd;      
        parsedCommand.targetEcu = (uint8_t)target;   
        
        memset(parsedCommand.parameters, 0, 6);
        parsedCommand.parameters[0] = (uint8_t)param1;
        parsedCommand.parameters[1] = (uint8_t)param2;
        
        printf(" -> 파싱 성공! CMD:0x%02X, TARGET:ECU%d\r\n", parsedCommand.commandId, parsedCommand.targetEcu);
    } else {
        printf(" -> 파싱 실패! 형식 불일치\r\n");
    }
    
    // 수신 버퍼 초기화 후 다시 인터럽트 대기
    memset(uartRxBuffer, 0, sizeof(uartRxBuffer));
    HAL_UART_Receive_IT(&huart2, uartRxBuffer, sizeof(uartRxBuffer));
}
