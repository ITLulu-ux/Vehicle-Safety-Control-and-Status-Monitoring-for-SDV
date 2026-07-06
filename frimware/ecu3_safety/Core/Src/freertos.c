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
#include "main.h"
#include "cmsis_os.h"
#include "control_data.h"
#include "risk.h"
#include "brake.h"
#include "wiper.h"
#include "led.h"
#include "heartbeat.h"
#include "can_comm.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can.h"
#include "tim.h"
// #include "lcd_i2c.h" // 팀원들이 만든 LCD 헤더 연동 시 주석 해제
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// main.c에 정의된 RiskLevel 열거형 공유용
/*typedef enum {
    RISK_SAFE = 0,
    RISK_CAUTION,
    RISK_WARNING,
    RISK_DANGER
} RiskLevel_t;*/

// 큐 데이터 포맷 정의 (CAN RX 메시지 백업용 구조체)
/*typedef struct {
    uint32_t StdId;
    uint8_t  DLC;
    uint8_t  Data[8];
} CAN_Rx_Format_t;*/
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
  .stack_size = 256 * 4,
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
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_Wiper */
osThreadId_t Task_WiperHandle;
const osThreadAttr_t Task_Wiper_attributes = {
  .name = "Task_Wiper",
  .stack_size = 256 * 4,
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
  Queue_CAN_RXHandle = osMessageQueueNew (16, sizeof(CAN_Rx_Format_t), &Queue_CAN_RX_attributes);

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
  CAN_Rx_Format_t rxMsg;

  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

  for(;;)
  {
	  if (osMessageQueueGet(Queue_CAN_RXHandle, &rxMsg, NULL, osWaitForever) == osOK)
	   {
		  switch (rxMsg.StdId)
		              {
		                  case CAN_ID_ECU1_ENV:
		                      if (osMutexAcquire(Mutex_RiskDataHandle, osWaitForever) == osOK) {
		                    	  sensorData.temperature=rxMsg.Data[0];
		                          sensorData.humidity = rxMsg.Data[1];
		                          sensorData.lux = (rxMsg.Data[2] << 8) | rxMsg.Data[3];
		                          osMutexRelease(Mutex_RiskDataHandle);
		                      }
		                      break;

		                  case CAN_ID_ECU2_DRIVE:
		                      if (osMutexAcquire(Mutex_RiskDataHandle, osWaitForever) == osOK) {
		                          drivingData.speed = rxMsg.Data[0];
		                          drivingData.distance = (rxMsg.Data[2] << 8) | rxMsg.Data[1];
		                          osMutexRelease(Mutex_RiskDataHandle);
		                      }
		                      // 수신 즉시 위험도 계산 노드 구동
		                      Process_Risk_Analysis();
		                      break;

		                  case CAN_ID_GATEWAY_CMD:
		                      CAN_HandleDownlink(rxMsg.Data);
		                      break;
		              }
	    }
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
    if (ota_mode_active == 0U) {
      Control_LED_Alert();
    }
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
    if (ota_mode_active == 0U) {
      CAN_Send_Status();
    }
    osDelay(1000);
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
    if (ota_mode_active == 0U) {
      Send_Heartbeat_Signal();
    }
    osDelay(1000);
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
    if (ota_mode_active == 0U) {
      Process_Risk_Analysis();
    }
    osDelay(20);
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
    if (ota_mode_active == 0U) {
      Control_Brake();
    }
    osDelay(20);
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
    if (ota_mode_active == 0U) {
      Control_Wiper();
    }
    osDelay(1);
  }
  /* USER CODE END StartWiperTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

