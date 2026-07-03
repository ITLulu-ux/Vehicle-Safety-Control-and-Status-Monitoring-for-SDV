#ifndef UART_COMM_H
#define UART_COMM_H

#include <stdint.h>

// CAN_ID_matrix 규격 기반 8바이트 Payload 구조체
typedef struct {
    uint8_t commandId;     // Byte 0: Command ID (0x01 ~ 0x07)
    uint8_t targetEcu;     // Byte 1: Target ECU (1 ~ 4)
    uint8_t parameters[ 6 ]; // Byte 2~7: Command Parameter (에러 원인 해결: 6칸 배열)
} UartCommand_t;

extern UartCommand_t parsedCommand;
extern uint8_t uartRxBuffer[ 32 ]; // 배열 크기 32칸 명시

void UART_SendGatewayData(void);
void UART_ProcessRxMessage(void);

#endif // UART_COMM_H