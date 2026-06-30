#include "uart_comm.h"
#include "gateway_data.h"
#include "usart.h" 
#include <stdio.h>
#include <string.h>

UartCommand_t parsedCommand;
uint8_t uartRxBuffer[ 32 ]; // 전역 변수 메모리 할당 (32칸 명시)

void UART_SendGatewayData(void) {
    char txBuffer[ 64 ]; // 송신 버퍼 크기 명시 (오버플로우 방지)
    
    sprintf(txBuffer, "100,%d,%d\n200,%d,%d,%d\n300,%d,%d,%d,%d\n",
            gatewayData.drivingData.speed, gatewayData.drivingData.distance,
            (int)gatewayData.sensorData.temp, (int)gatewayData.sensorData.humi, gatewayData.sensorData.lux,
            gatewayData.controlData.brakeLevel, gatewayData.controlData.wiperState,
            gatewayData.controlData.riskLevel, gatewayData.controlData.ledState);

    HAL_UART_Transmit_DMA(&huart2, (uint8_t*)txBuffer, strlen(txBuffer));
}

void UART_ProcessRxMessage(void) {
    int cmd = 0, target = 0, param1 = 0, param2 = 0;
    
    if (sscanf((char*)uartRxBuffer, "%d,%d,%d,%d", &cmd, &target, &param1, &param2) >= 2) {
        parsedCommand.commandId = (uint8_t)cmd;      
        parsedCommand.targetEcu = (uint8_t)target;   
        
        memset(parsedCommand.parameters, 0, 6);
        
        // 에러 원인 해결: 배열의 첫 번째와 두 번째 인덱스를 명확히 지정
        parsedCommand.parameters[ 0 ] = (uint8_t)param1; 
        parsedCommand.parameters[ 1 ] = (uint8_t)param2;
    }
    
    memset(uartRxBuffer, 0, sizeof(uartRxBuffer));
}