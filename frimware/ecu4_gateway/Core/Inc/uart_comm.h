#ifndef UART_COMM_H
#define UART_COMM_H

#include <stdint.h>

#define UART_DOWNLINK_PACKET_LEN  8U

extern uint8_t uartRxBuffer[UART_DOWNLINK_PACKET_LEN];

void UART_SendGatewayData(void);
void UART_OnPacket(void);

#endif /* UART_COMM_H */
