/*
 * tasks.c
 *
 *  Created on: 2026. 6. 30.
 *      Author: rlaek
 */

#include "tasks.h"
#include "cmsis_os.h"
#include "control_data.h"
#include "can_comm.h"
#include "risk.h"
#include "brake.h"
#include "wiper.h"
#include "led.h"
#include "heartbeat.h"

extern osMessageQueueId_t Queue_CAN_RXHandle;
extern osMessageQueueId_t Queue_DTCHandle;
extern volatile uint8_t ota_mode_active;

/**
  * @brief 최상위 수신 라우팅 타겟 (osPriorityRealtime)
  */
void StartTask_CAN_RX(void *argument)
{
    CAN_Rx_Format_t rxMsg;
    for(;;)
    {
        if (osMessageQueueGet(Queue_CAN_RXHandle, &rxMsg, NULL, osWaitForever) == osOK)
        {
            switch (rxMsg.StdId)
            {
                case CAN_ID_ECU1_ENV:
                    if (osMutexAcquire(Mutex_RiskDataHandle, osWaitForever) == osOK) {
                        sensorData.humidity = rxMsg.Data[1];
                        sensorData.lux = (rxMsg.Data[2] << 8) | rxMsg.Data[3];
                        osMutexRelease(Mutex_RiskDataHandle);
                    }
                    break;

                case CAN_ID_ECU2_DRIVE:
                    if (osMutexAcquire(Mutex_RiskDataHandle, osWaitForever) == osOK) {
                        drivingData.speed = rxMsg.Data[0];
                        drivingData.distance = (rxMsg.Data[1] << 8) | rxMsg.Data[2];
                        osMutexRelease(Mutex_RiskDataHandle);
                    }
                    // 수신 즉시 위험도 계산 노드 구동
                    Process_Risk_Analysis();
                    break;

                case CAN_ID_GATEWAY_CMD:
                    if (rxMsg.Data[0] == 0x01) HAL_NVIC_SystemReset(); // UDS 강제 리셋 명령
                    else if (rxMsg.Data[0] == 0x02 && rxMsg.Data[1] == 0x03) ota_mode_active = 1;
                    else if (rxMsg.Data[0] == 0x04) ota_mode_active = 0;
                    break;
            }
        }
    }
}

void StartTask_Control(void *argument)
{
    for(;;)
    {
        Control_Brake();
        Control_Wiper();
        CAN_Send_Status(); // 주기 0x300 송신 연동
        osDelay(20);
    }
}

void StartTask_DTC_Log(void *argument)
{
    uint32_t dtc_code = 0;
    for(;;)
    {
        if (osMessageQueueGet(Queue_DTCHandle, &dtc_code, NULL, osWaitForever) == osOK)
        {
            // 결함 코드 저장 핸들러 확장 영역
        }
    }
}

void StartTask_LED(void *argument)
{
    for(;;)
    {
        Control_LED_Alert();
        // 위험 단계별 차등 틱 반영을 루프 최적화 위임
        uint8_t level = controlData.riskLevel;
        if (level == RISK_DANGER)       osDelay(100);
        else if (level == RISK_WARNING) osDelay(300);
        else                            osDelay(500);
    }
}

void StartTask_CAN_TX(void *argument)
{
    for(;;)
    {
        if (osMutexAcquire(Mutex_I2CHandle, osWaitForever) == osOK)
        {
            // LCD 200ms 주사율 드로잉 구현 영역
            osMutexRelease(Mutex_I2CHandle);
        }
        osDelay(200);
    }
}

void StartTask_Heartbeat(void *argument)
{
    for(;;)
    {
        Send_Heartbeat_Signal();
        osDelay(1000);
    }
}
