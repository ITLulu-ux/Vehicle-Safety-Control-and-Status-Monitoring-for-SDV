#include "heartbeat.h"
#include "gateway_data.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>

extern SemaphoreHandle_t gatewayDataMutex;

static uint8_t hbAliveTicks[3] = {0, 0, 0};

void Heartbeat_OnEcuReceived(uint8_t ecuNum)
{
    if (ecuNum >= 1U && ecuNum <= 3U) {
        hbAliveTicks[ecuNum - 1U] = HEARTBEAT_TIMEOUT_SEC;
    }
}

void Heartbeat_CheckTimeout(void)
{
    if (xSemaphoreTake(gatewayDataMutex, portMAX_DELAY) != pdTRUE) {
        return;
    }

    for (uint8_t i = 0; i < 3U; i++) {
        if (hbAliveTicks[i] > 0U) {
            hbAliveTicks[i]--;
        }

        if (hbAliveTicks[i] == 0U) {
            if (i == 0U) {
                gatewayData.heartbeat1 = 0;
            } else if (i == 1U) {
                gatewayData.heartbeat2 = 0;
            } else {
                gatewayData.heartbeat3 = 0;
            }
        } else {
            if (i == 0U) {
                gatewayData.heartbeat1 = 1;
            } else if (i == 1U) {
                gatewayData.heartbeat2 = 1;
            } else {
                gatewayData.heartbeat3 = 1;
            }
        }
    }

    xSemaphoreGive(gatewayDataMutex);
}

void OTA_HandleLocalCommand(const UartCommand_t *cmd)
{
    if (cmd == NULL) {
        return;
    }

    printf("[OTA] ECU4 로컬 명령 수신 - CMD:0x%02X, P1:%d, P2:%d\r\n",
           cmd->commandId, cmd->parameters[0], cmd->parameters[1]);
    /* Phase 2: 플래시 OTA 처리 */
}
