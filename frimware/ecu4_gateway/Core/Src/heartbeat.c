#include "heartbeat.h"
#include "gateway_data.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

extern SemaphoreHandle_t gatewayDataMutex;

static uint8_t hbAliveTicks[4] = {0, 0, 0, 0};

void Heartbeat_OnEcuReceived(uint8_t ecuNum)
{
    if (ecuNum >= 1U && ecuNum <= 4U) {
        hbAliveTicks[ecuNum - 1U] = HEARTBEAT_TIMEOUT_SEC;
    }
}

void Heartbeat_CheckTimeout(void)
{
    if (xSemaphoreTake(gatewayDataMutex, portMAX_DELAY) != pdTRUE) {
        return;
    }

    for (uint8_t i = 0; i < 4U; i++) {
        if (hbAliveTicks[i] > 0U) {
            hbAliveTicks[i]--;
        }
        // ===== 여기부터 추가 =====
           if (i == 3U)
           {
               char dbg[64];

//               snprintf(dbg,
//                        sizeof(dbg),
//                        "HB4 Tick=%u\r\n",
//                        hbAliveTicks[3]);
//
//               HAL_UART_Transmit(&huart2,
//                                 (uint8_t *)dbg,
//                                 (uint16_t)strlen(dbg),
//                                 100);
           }
           // ===== 여기까지 추가 =====

        if (hbAliveTicks[i] == 0U) {
            if (i == 0U) {
                gatewayData.heartbeat1 = 0;
            } else if (i == 1U) {
                gatewayData.heartbeat2 = 0;
            } else if (i == 2U) {
                gatewayData.heartbeat3 = 0;
            } else {
                gatewayData.heartbeat4 = 0;
            }
        } else {
            if (i == 0U) {
                gatewayData.heartbeat1 = 1;
            } else if (i == 1U) {
                gatewayData.heartbeat2 = 1;
            } else if (i == 2U) {
                gatewayData.heartbeat3 = 1;
            } else {
                gatewayData.heartbeat4 = 1;
            }
        }
    }

    xSemaphoreGive(gatewayDataMutex);
}
