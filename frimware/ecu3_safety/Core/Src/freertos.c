/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can.h"
#include "tim.h"
// #include "lcd_i2c.h" // 팀원들이 만든 LCD 헤더 연동 시 주석 해제
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// main.c에 정의된 RiskLevel 열거형 공유용
typedef enum {
    RISK_SAFE = 0,
    RISK_CAUTION,
    RISK_WARNING,
    RISK_DANGER
} RiskLevel_t;

// 큐 데이터 포맷 정의 (CAN RX 메시지 백업용 구조체)
typedef struct {
    uint32_t StdId;
    uint8_t  DLC;
    uint8_t  Data[8];
} CAN_Rx_Format_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
// main.c에 선언된 전역 변수들 extern 참조
extern volatile uint8_t current_risk_level;
extern volatile uint8_t ota_mode_active;
extern volatile uint8_t current_humidity;
extern volatile uint16_t current_lux;

extern CAN_TxHeaderTypeDef TxHeader;
extern uint8_t TxData[8];
extern uint32_t TxMailbox;

// CubeMX가 자동 생성해 주는 핸들러들 명칭 (OS 오브젝트)
extern osThreadId_t Task_CAN_RXHandle;
extern osThreadId_t Task_ControlHandle;
extern osThreadId_t Task_DTC_LogHandle;
extern osThreadId_t Task_LEDHandle;
extern osThreadId_t Task_CAN_TXHandle;
extern osThreadId_t Task_HeartbeatHandle;

extern osMessageQueueId_t Queue_CAN_RXHandle;
extern osMessageQueueId_t Queue_DTCHandle;

extern osMutexId_t Mutex_RiskDataHandle; // 위험도 데이터 보호용 뮤텍스
extern osMutexId_t Mutex_I2CHandle;        // LCD용 I2C 보호용 뮤텍스
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_CAN_RX */
osThreadId_t Task_CAN_RXHandle;
const osThreadAttr_t Task_CAN_RX_attributes = {
  .name = "Task_CAN_RX",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Task_LED */
osThreadId_t Task_LEDHandle;
const osThreadAttr_t Task_LED_attributes = {
  .name = "Task_LED",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_CAN_TX */
osThreadId_t Task_CAN_TXHandle;
const osThreadAttr_t Task_CAN_TX_attributes = {
  .name = "Task_CAN_TX",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_Heartbeat */
osThreadId_t Task_HeartbeatHandle;
const osThreadAttr_t Task_Heartbeat_attributes = {
  .name = "Task_Heartbeat",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Task_Risk */
osThreadId_t Task_RiskHandle;
const osThreadAttr_t Task_Risk_attributes = {
  .name = "Task_Risk",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Task_Brake */
osThreadId_t Task_BrakeHandle;
const osThreadAttr_t Task_Brake_attributes = {
  .name = "Task_Brake",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_Wiper */
osThreadId_t Task_WiperHandle;
const osThreadAttr_t Task_Wiper_attributes = {
  .name = "Task_Wiper",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Queue_CAN_RX */
osMessageQueueId_t Queue_CAN_RXHandle;
const osMessageQueueAttr_t Queue_CAN_RX_attributes = {
  .name = "Queue_CAN_RX"
};
/* Definitions for Queue_DTC */
osMessageQueueId_t Queue_DTCHandle;
const osMessageQueueAttr_t Queue_DTC_attributes = {
  .name = "Queue_DTC"
};
/* Definitions for vehicleMutex */
osMutexId_t vehicleMutexHandle;
const osMutexAttr_t vehicleMutex_attributes = {
  .name = "vehicleMutex"
};
/* Definitions for controlMutex */
osMutexId_t controlMutexHandle;
const osMutexAttr_t controlMutex_attributes = {
  .name = "controlMutex"
};
/* Definitions for Mutex_RiskData */
osMutexId_t Mutex_RiskDataHandle;
const osMutexAttr_t Mutex_RiskData_attributes = {
  .name = "Mutex_RiskData"
};
/* Definitions for Mutex_I2C */
osMutexId_t Mutex_I2CHandle;
const osMutexAttr_t Mutex_I2C_attributes = {
  .name = "Mutex_I2C"
};
/* Definitions for CANRXSemaphore */
osSemaphoreId_t CANRXSemaphoreHandle;
const osSemaphoreAttr_t CANRXSemaphore_attributes = {
  .name = "CANRXSemaphore"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartCANRXTask(void *argument);
void StartLEDTask(void *argument);
void StartCANTXTask(void *argument);
void StartHeartbeatTask(void *argument);
void StartRiskTask(void *argument);
void StartBrakeTask(void *argument);
void StartWiperTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of vehicleMutex */
  vehicleMutexHandle = osMutexNew(&vehicleMutex_attributes);

  /* creation of controlMutex */
  controlMutexHandle = osMutexNew(&controlMutex_attributes);

  /* creation of Mutex_RiskData */
  Mutex_RiskDataHandle = osMutexNew(&Mutex_RiskData_attributes);

  /* creation of Mutex_I2C */
  Mutex_I2CHandle = osMutexNew(&Mutex_I2C_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
    // 만약 CubeMX에서 생성 안 되어 있다면 여기서 직접 생성 가능
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of CANRXSemaphore */
  CANRXSemaphoreHandle = osSemaphoreNew(1, 0, &CANRXSemaphore_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of Queue_CAN_RX */
  Queue_CAN_RXHandle = osMessageQueueNew (16, 12, &Queue_CAN_RX_attributes);

  /* creation of Queue_DTC */
  Queue_DTCHandle = osMessageQueueNew (16, 4, &Queue_DTC_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of Task_CAN_RX */
  Task_CAN_RXHandle = osThreadNew(StartCANRXTask, NULL, &Task_CAN_RX_attributes);

  /* creation of Task_LED */
  Task_LEDHandle = osThreadNew(StartLEDTask, NULL, &Task_LED_attributes);

  /* creation of Task_CAN_TX */
  Task_CAN_TXHandle = osThreadNew(StartCANTXTask, NULL, &Task_CAN_TX_attributes);

  /* creation of Task_Heartbeat */
  Task_HeartbeatHandle = osThreadNew(StartHeartbeatTask, NULL, &Task_Heartbeat_attributes);

  /* creation of Task_Risk */
  Task_RiskHandle = osThreadNew(StartRiskTask, NULL, &Task_Risk_attributes);

  /* creation of Task_Brake */
  Task_BrakeHandle = osThreadNew(StartBrakeTask, NULL, &Task_Brake_attributes);

  /* creation of Task_Wiper */
  Task_WiperHandle = osThreadNew(StartWiperTask, NULL, &Task_Wiper_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    // 각 태스크들은 CubeMX 툴에 의해 자동으로 osThreadNew로 등록되어 올라옵니다.
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartCANRXTask */
/**
* @brief Function implementing the Task_CAN_RX thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCANRXTask */
void StartCANRXTask(void *argument)
{
  /* USER CODE BEGIN StartCANRXTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartCANRXTask */
}

/* USER CODE BEGIN Header_StartLEDTask */
/**
* @brief Function implementing the Task_LED thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLEDTask */
void StartLEDTask(void *argument)
{
  /* USER CODE BEGIN StartLEDTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartLEDTask */
}

/* USER CODE BEGIN Header_StartCANTXTask */
/**
* @brief Function implementing the Task_CAN_TX thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCANTXTask */
void StartCANTXTask(void *argument)
{
  /* USER CODE BEGIN StartCANTXTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartCANTXTask */
}

/* USER CODE BEGIN Header_StartHeartbeatTask */
/**
* @brief Function implementing the Task_Heartbeat thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartHeartbeatTask */
void StartHeartbeatTask(void *argument)
{
  /* USER CODE BEGIN StartHeartbeatTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartHeartbeatTask */
}

/* USER CODE BEGIN Header_StartRiskTask */
/**
* @brief Function implementing the Task_Risk thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartRiskTask */
void StartRiskTask(void *argument)
{
  /* USER CODE BEGIN StartRiskTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartRiskTask */
}

/* USER CODE BEGIN Header_StartBrakeTask */
/**
* @brief Function implementing the Task_Brake thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartBrakeTask */
void StartBrakeTask(void *argument)
{
  /* USER CODE BEGIN StartBrakeTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartBrakeTask */
}

/* USER CODE BEGIN Header_StartWiperTask */
/**
* @brief Function implementing the Task_Wiper thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartWiperTask */
void StartWiperTask(void *argument)
{
  /* USER CODE BEGIN StartWiperTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartWiperTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
  * @brief  1등: CAN 수신 처리 태스크 (osPriorityRealtime)
  * 인터럽트가 던져준 수신 큐에서 데이터를 빼서 UDS 명령 및 데이터 파싱을 수행합니다.
  */
/*void StartTask_CAN_RX(void *argument)
{
    CAN_Rx_Format_t rxMsg;
    for(;;)
    {
        // 큐에 데이터가 들어올 때까지 무한 대기 (Block 상태로 전력 소모 감소)
        if (osMessageQueueGet(Queue_CAN_RXHandle, &rxMsg, NULL, osWaitForever) == osOK)
        {
            switch (rxMsg.StdId)
            {
                case 0x100: // ECU1 환경 데이터 수신
                    current_humidity = rxMsg.Data[1];
                    current_lux = (rxMsg.Data[2] << 8) | rxMsg.Data[3];
                    break;

                case 0x200: // ECU2 주행 데이터 수신 ➡️ 위험도 실시간 판단
                    // 다른 태스크가 읽어가는 동안 값 안 꼬이게 뮤텍스 처리
                    if (osMutexAcquire(Mutex_RiskDataHandle, osWaitForever) == osOK)
                    {
                        uint16_t distance = (rxMsg.Data[1] << 8) | rxMsg.Data[2]; // 초음파 거리 계산 예시
                        if (distance < 20)       current_risk_level = RISK_DANGER;
                        else if (distance < 50)  current_risk_level = RISK_WARNING;
                        else                     current_risk_level = RISK_SAFE;

                        osMutexRelease(Mutex_RiskDataHandle);
                    }
                    break;

                case 0x400: // ECU4(Gateway) 제어 명령 수신 (UDS 진단 및 제어 권역)
                    switch (rxMsg.Data[0])
                    {
                        case 0x01: // UDS: ECU Reset Command
                            HAL_NVIC_SystemReset();
                            break;

                        case 0x02: // UDS: OTA Start
                            if (rxMsg.Data[1] == 0x03) ota_mode_active = 1;
                            break;

                        case 0x04: // UDS: OTA End
                            ota_mode_active = 0;
                            break;

                        default:
                            break;
                    }
                    break;
            }
        }
    }
}

/**
  * @brief  2등: 실시간 차량 안전 제어 태스크 (osPriorityHigh1)
  * 위험도 상태에 맞춰 브레이크 및 와이퍼 서보모터(PWM)를 즉각 제어합니다.
  */
/*void StartTask_Control(void *argument)
{
    uint8_t local_risk = RISK_SAFE;
    for(;;)
    {
        // 위험도 데이터 안전하게 카피해오기
        if (osMutexAcquire(Mutex_RiskDataHandle, osWaitForever) == osOK)
        {
            local_risk = current_risk_level;
            osMutexRelease(Mutex_RiskDataHandle);
        }

        // 1. 브레이크 서보모터 제어 (TIM4 CH1)
        if (local_risk == RISK_DANGER) {
            __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 2000); // 꽉 제동 (2.0ms)
        } else if (local_risk == RISK_WARNING) {
            __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 1700); // 감속 제동 (1.7ms)
        } else {
            __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 1500); // 해제 중립 (1.5ms)
        }

        // 2. 와이퍼 서보모터 제어 (TIM4 CH2 - 습도가 높거나 비올 때)
        if (current_humidity > 80 || local_risk == RISK_DANGER) {
            __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 1800); // 고속 와이핑
        } else {
            __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 1500); // 와이퍼 정지 위치
        }

        osDelay(20); // 20ms 제어 주기 반영
    }
}

/**
  * @brief  3등: UDS 진단 로그 기록 태스크 (osPriorityHigh)
  * DANGER 상황이나 시스템 에러 발생 시 DTC 결함 코드를 생성하여 큐에 보관/기록합니다.
  */
/*void StartTask_DTC_Log(void *argument)
{
    uint32_t dtc_code = 0;
    for(;;)
    {
        // 🚨 시나리오: 제어 태스크에서 심각한 에러코드를 Queue_DTC에 던지면 감지하여 플래시/로그 기록
        if (osMessageQueueGet(Queue_DTCHandle, &dtc_code, NULL, osWaitForever) == osOK)
        {
            // 수신된 DTC 고장 진단 코드를 내부 보관하거나 기록 로직 연동
            // 예: dtc_code == 0xB001 (Brake System Error)
        }
    }
}

/**
  * @brief  4등: 시각 경고 LED 태스크 (osPriorityNormal)
  * 위험도 단계를 읽어와 운전자에게 직관적인 LED 알림을 쏩니다.
  */
/*void StartTask_LED(void *argument)
{
    uint8_t local_risk = RISK_SAFE;
    for(;;)
    {
        if (osMutexAcquire(Mutex_RiskDataHandle, osWaitForever) == osOK)
        {
            local_risk = current_risk_level;
            osMutexRelease(Mutex_RiskDataHandle);
        }

        if (local_risk == RISK_DANGER) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // 보드 내장 LED 초고속 점멸 경고
            osDelay(100);
        } else if (local_risk == RISK_WARNING) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // 일반 점멸
            osDelay(300);
        } else {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); // 정상 상태 소등
            osDelay(500);
        }
    }
}

/**
  * @brief  4등(공동): 일반 데이터 수신 보고 및 LCD 출력 (osPriorityNormal)
  * I2C LCD 리소스를 선점당하지 않도록 Mutex로 묶어 화면을 200ms 주기로 갱신합니다.
  */
/*void StartTask_CAN_TX(void *argument)
{
    for(;;)
    {
        // 🔒 I2C 통신 중 다른 태스크가 침범해서 텍스트 깨지는 현상 방지
        if (osMutexAcquire(Mutex_I2CHandle, osWaitForever) == osOK)
        {
            // LCD_ShowStatus(current_risk_level); // 실제 LCD 출력 모듈 연동 구역
            osMutexRelease(Mutex_I2CHandle);
        }
        osDelay(200); // LCD 스펙 가이드라인 200ms 유지
    }
}

/**
  * @brief  5등: 시스템 생존용 하트비트 송신 태스크 (osPriorityLow)
  * 모든 메인 중요 제어가 끝나고 시스템이 안전할 때, 1초 주기로 생존 신호(0x703)를 날립니다.
  */
/*void StartTask_Heartbeat(void *argument)
{
    // CAN 하트비트 메시지 헤더 뼈대 구성
    TxHeader.StdId = 0x703;      // 엑셀 명세 기반 ECU3 Heartbeat ID
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = 1;            // 1바이트 데이터
    TxHeader.TransmitGlobalTime = DISABLE;

    for(;;)
    {
        TxData[0] = 0xAA; // 0xAA: "나 무사히 정상 작동 중이야" 상태 바이트

        // CAN 버스가 가득 차서 대기하지 않도록 우아하게 전송 요청
        if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0)
        {
            HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
        }

        osDelay(1000); // 1초(1000ms) 주기 엄수
    }
}*/

/* USER CODE END Application */

