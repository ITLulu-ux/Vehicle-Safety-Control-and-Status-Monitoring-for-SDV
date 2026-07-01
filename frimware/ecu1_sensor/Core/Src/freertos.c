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
#include "sensor_task.h"
#include "can_task.h"
#include "heartbeat_task.h"
#include "sensor_data.h"
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
osThreadId Sensor_TaskHandle;
osThreadId CAN_TX_TaskHandle;
osThreadId Hearbeat_TaskHandle;
osThreadId CAN_RX_TaskHandle;
osMessageQId SensorQueueHandle;
osSemaphoreId CAN_RX_BinarySemphoreHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartSensorTask(void const * argument);
void StartCanTxTask(void const * argument);
void StartHeartbeatTask(void const * argument);
void StartCanRxTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

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

  /* Create the semaphores(s) */
  /* definition and creation of CAN_RX_BinarySemphore */
  osSemaphoreDef(CAN_RX_BinarySemphore);
  CAN_RX_BinarySemphoreHandle = osSemaphoreCreate(osSemaphore(CAN_RX_BinarySemphore), 0);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  canRxSemaphore = (SemaphoreHandle_t)CAN_RX_BinarySemphoreHandle;
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of SensorQueue */
  osMessageQDef(SensorQueue, 1, SensorData_t);
  SensorQueueHandle = osMessageCreate(osMessageQ(SensorQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  sensorQueue = (QueueHandle_t)SensorQueueHandle;
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of Sensor_Task */
  osThreadDef(Sensor_Task, StartSensorTask, osPriorityNormal, 0, 256);
  Sensor_TaskHandle = osThreadCreate(osThread(Sensor_Task), NULL);

  /* definition and creation of CAN_TX_Task */
  osThreadDef(CAN_TX_Task, StartCanTxTask, osPriorityAboveNormal, 0, 256);
  CAN_TX_TaskHandle = osThreadCreate(osThread(CAN_TX_Task), NULL);

  /* definition and creation of Hearbeat_Task */
  osThreadDef(Hearbeat_Task, StartHeartbeatTask, osPriorityLow, 0, 128);
  Hearbeat_TaskHandle = osThreadCreate(osThread(Hearbeat_Task), NULL);

  /* definition and creation of CAN_RX_Task */
  osThreadDef(CAN_RX_Task, StartCanRxTask, osPriorityHigh, 0, 256);
  CAN_RX_TaskHandle = osThreadCreate(osThread(CAN_RX_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartSensorTask */
/**
  * @brief  Function implementing the Sensor_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartSensorTask */
void StartSensorTask(void const * argument)
{
  /* USER CODE BEGIN StartSensorTask */
  /* Infinite loop */
  SensorTask(argument);
  /* USER CODE END StartSensorTask */
}

/* USER CODE BEGIN Header_StartCanTxTask */
/**
* @brief Function implementing the CAN_TX_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCanTxTask */
void StartCanTxTask(void const * argument)
{
  /* USER CODE BEGIN StartCanTxTask */
  /* Infinite loop */
  CanTxTask(argument);
  /* USER CODE END StartCanTxTask */
}

/* USER CODE BEGIN Header_StartHeartbeatTask */
/**
* @brief Function implementing the Hearbeat_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartHeartbeatTask */
void StartHeartbeatTask(void const * argument)
{
  /* USER CODE BEGIN StartHeartbeatTask */
  /* Infinite loop */
  HeartbeatTask(argument);
  /* USER CODE END StartHeartbeatTask */
}

/* USER CODE BEGIN Header_StartCanRxTask */
/**
* @brief Function implementing the CAN_RX_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCanRxTask */
void StartCanRxTask(void const * argument)
{
  /* USER CODE BEGIN StartCanRxTask */
  /* Infinite loop */
  CanRxTask(argument);
  /* USER CODE END StartCanRxTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
