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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

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
/* Definitions for Task_Risk */
osThreadId_t Task_RiskHandle;
const osThreadAttr_t Task_Risk_attributes = {
  .name = "Task_Risk",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
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
/* Definitions for Task_LED */
osThreadId_t Task_LEDHandle;
const osThreadAttr_t Task_LED_attributes = {
  .name = "Task_LED",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
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
/* Definitions for Queue_CAN_Rx */
osMessageQueueId_t Queue_CAN_RxHandle;
const osMessageQueueAttr_t Queue_CAN_Rx_attributes = {
  .name = "Queue_CAN_Rx"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartCANRX(void *argument);
void StartRiskAnalysis(void *argument);
void StartBrakeControl(void *argument);
void StartWiperControl(void *argument);
void StartLEDControl(void *argument);
void StartCANTX(void *argument);
void StartHeartbeat(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of Queue_CAN_Rx */
  Queue_CAN_RxHandle = osMessageQueueNew (16, 12, &Queue_CAN_Rx_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of Task_CAN_RX */
  Task_CAN_RXHandle = osThreadNew(StartCANRX, NULL, &Task_CAN_RX_attributes);

  /* creation of Task_Risk */
  Task_RiskHandle = osThreadNew(StartRiskAnalysis, NULL, &Task_Risk_attributes);

  /* creation of Task_Brake */
  Task_BrakeHandle = osThreadNew(StartBrakeControl, NULL, &Task_Brake_attributes);

  /* creation of Task_Wiper */
  Task_WiperHandle = osThreadNew(StartWiperControl, NULL, &Task_Wiper_attributes);

  /* creation of Task_LED */
  Task_LEDHandle = osThreadNew(StartLEDControl, NULL, &Task_LED_attributes);

  /* creation of Task_CAN_TX */
  Task_CAN_TXHandle = osThreadNew(StartCANTX, NULL, &Task_CAN_TX_attributes);

  /* creation of Task_Heartbeat */
  Task_HeartbeatHandle = osThreadNew(StartHeartbeat, NULL, &Task_Heartbeat_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
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

/* USER CODE BEGIN Header_StartCANRX */
/**
* @brief Function implementing the Task_CAN_RX thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCANRX */
void StartCANRX(void *argument)
{
  /* USER CODE BEGIN StartCANRX */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartCANRX */
}

/* USER CODE BEGIN Header_StartRiskAnalysis */
/**
* @brief Function implementing the Task_Risk thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartRiskAnalysis */
void StartRiskAnalysis(void *argument)
{
  /* USER CODE BEGIN StartRiskAnalysis */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartRiskAnalysis */
}

/* USER CODE BEGIN Header_StartBrakeControl */
/**
* @brief Function implementing the Task_Brake thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartBrakeControl */
void StartBrakeControl(void *argument)
{
  /* USER CODE BEGIN StartBrakeControl */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartBrakeControl */
}

/* USER CODE BEGIN Header_StartWiperControl */
/**
* @brief Function implementing the Task_Wiper thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartWiperControl */
void StartWiperControl(void *argument)
{
  /* USER CODE BEGIN StartWiperControl */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartWiperControl */
}

/* USER CODE BEGIN Header_StartLEDControl */
/**
* @brief Function implementing the Task_LED thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLEDControl */
void StartLEDControl(void *argument)
{
  /* USER CODE BEGIN StartLEDControl */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartLEDControl */
}

/* USER CODE BEGIN Header_StartCANTX */
/**
* @brief Function implementing the Task_CAN_TX thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCANTX */
void StartCANTX(void *argument)
{
  /* USER CODE BEGIN StartCANTX */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartCANTX */
}

/* USER CODE BEGIN Header_StartHeartbeat */
/**
* @brief Function implementing the Task_Heartbeat thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartHeartbeat */
void StartHeartbeat(void *argument)
{
  /* USER CODE BEGIN StartHeartbeat */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartHeartbeat */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

