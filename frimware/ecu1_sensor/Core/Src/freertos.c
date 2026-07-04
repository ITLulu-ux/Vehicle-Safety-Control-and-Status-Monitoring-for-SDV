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
#include <stdio.h>
#include "sensor_task.h"
#include "sensor_data.h"
#include "queue.h"
#include "can_task.h"
#include "can_comm.h"
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
osThreadId CAN_RX_TaskHandle;
osThreadId Heartbeat_TaskHandle;
osSemaphoreId CAN_RX_BinarySemaphoreHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartSensorTask(void const * argument);
void Start_CAN_TX_Task(void const * argument);
void Start_CAN_RX_Task(void const * argument);
void Start_Heartbeat_Task(void const * argument);

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
  /* definition and creation of CAN_RX_BinarySemaphore */
  osSemaphoreDef(CAN_RX_BinarySemaphore);
  CAN_RX_BinarySemaphoreHandle = osSemaphoreCreate(osSemaphore(CAN_RX_BinarySemaphore), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  sensorQueue = xQueueCreate(
          1,
          sizeof(SensorData_t));

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of Sensor_Task */
  osThreadDef(Sensor_Task, StartSensorTask, osPriorityNormal, 0, 256);
  Sensor_TaskHandle = osThreadCreate(osThread(Sensor_Task), NULL);

  /* definition and creation of CAN_TX_Task */
  osThreadDef(CAN_TX_Task, Start_CAN_TX_Task, osPriorityAboveNormal, 0, 256);
  CAN_TX_TaskHandle = osThreadCreate(osThread(CAN_TX_Task), NULL);

  /* definition and creation of CAN_RX_Task */
  osThreadDef(CAN_RX_Task, Start_CAN_RX_Task, osPriorityHigh, 0, 256);
  CAN_RX_TaskHandle = osThreadCreate(osThread(CAN_RX_Task), NULL);

  /* definition and creation of Heartbeat_Task */
  osThreadDef(Heartbeat_Task, Start_Heartbeat_Task, osPriorityLow, 0, 128);
  Heartbeat_TaskHandle = osThreadCreate(osThread(Heartbeat_Task), NULL);

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

/* USER CODE BEGIN Header_Start_CAN_TX_Task */
/**
* @brief Function implementing the CAN_TX_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_CAN_TX_Task */
void Start_CAN_TX_Task(void const * argument)
{
  /* USER CODE BEGIN Start_CAN_TX_Task */
  /* Infinite loop */
	CanTxTask(argument);
  /* USER CODE END Start_CAN_TX_Task */
}

/* USER CODE BEGIN Header_Start_CAN_RX_Task */
/**
* @brief Function implementing the CAN_RX_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_CAN_RX_Task */
void Start_CAN_RX_Task(void const * argument)
{
  /* USER CODE BEGIN Start_CAN_RX_Task */
  for (;;)
  {
    CAN_RX_Task_Run();
  }
  /* USER CODE END Start_CAN_RX_Task */
}

/* USER CODE BEGIN Header_Start_Heartbeat_Task */
/**
* @brief Function implementing the Heartbeat_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Heartbeat_Task */
void Start_Heartbeat_Task(void const * argument)
{
  /* USER CODE BEGIN Start_Heartbeat_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Start_Heartbeat_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
