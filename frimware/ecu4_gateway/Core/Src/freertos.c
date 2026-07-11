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
#include "can_comm.h"
#include "uart_comm.h"
#include "heartbeat.h"
#include "semphr.h"
#include <stdio.h>
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
SemaphoreHandle_t gatewayDataMutex = NULL;

extern CAN_HandleTypeDef hcan1;
extern UART_HandleTypeDef huart2;
extern uint8_t uartRxBuffer[UART_DOWNLINK_PACKET_LEN];

/* USER CODE END Variables */
osThreadId Task_CAN_ProcesHandle;
osThreadId Task_UART_ProceHandle;
osThreadId Task_HeartbeatHandle;
osMessageQId Queue_CAN_RxHandle;
osSemaphoreId Sem_UART_RxHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void Start_Task_Can(void const * argument);
void Start_Task_UART(void const * argument);
void Start_Task_Heartbeat(void const * argument);

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
	gatewayDataMutex = xSemaphoreCreateMutex();
	if (gatewayDataMutex == NULL)
	{
	    Error_Handler();
	}
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of Sem_UART_Rx */
  osSemaphoreDef(Sem_UART_Rx);
  Sem_UART_RxHandle = osSemaphoreCreate(osSemaphore(Sem_UART_Rx), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of Queue_CAN_Rx */
  osMessageQDef(Queue_CAN_Rx, 16, uint16_t);
  Queue_CAN_RxHandle = osMessageCreate(osMessageQ(Queue_CAN_Rx), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of Task_CAN_Proces */
  osThreadDef(Task_CAN_Proces, Start_Task_Can, osPriorityHigh, 0, 512);
  Task_CAN_ProcesHandle = osThreadCreate(osThread(Task_CAN_Proces), NULL);

  /* definition and creation of Task_UART_Proce */
  osThreadDef(Task_UART_Proce, Start_Task_UART, osPriorityNormal, 0, 128);
  Task_UART_ProceHandle = osThreadCreate(osThread(Task_UART_Proce), NULL);

  /* definition and creation of Task_Heartbeat */
  osThreadDef(Task_Heartbeat, Start_Task_Heartbeat, osPriorityLow, 0, 128);
  Task_HeartbeatHandle = osThreadCreate(osThread(Task_Heartbeat), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_Start_Task_Can */
/**
  * @brief  Function implementing the Task_CAN_Proces thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Start_Task_Can */
void Start_Task_Can(void const * argument)
{
  /* USER CODE BEGIN Start_Task_Can */
    if (HAL_CAN_Start(&hcan1) == HAL_OK) {
        printf("[CAN] Start OK\r\n");
    }

    for (;;)
    {
        if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0) {
            CAN_ProcessRxMessage();
        }
        osDelay(10);
    }
  /* USER CODE END Start_Task_Can */
}

/* USER CODE BEGIN Header_Start_Task_UART */
/**
* @brief Function implementing the Task_UART_Proce thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Task_UART */
void Start_Task_UART(void const * argument)
{
  /* USER CODE BEGIN Start_Task_UART */
    // ⭐️ [추가] 라즈베리파이 수신 초인종도 이 타이밍에 켭니다.
    HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_DOWNLINK_PACKET_LEN);

//    for(;;)
//    {
//        // 2s마다 업링크 JSON, 그 사이 Pi OTA/제어 8바이트 수신 시 CAN 0x400 중계
//        if(osSemaphoreWait(Sem_UART_RxHandle, 2000) == osOK) {
//            UART_OnPacket();
//        } else {
//            UART_SendGatewayData();
//        }

        for(;;)
        {
            // 2s마다 업링크 JSON, 그 사이 Pi OTA/제어 8바이트 수신 시 CAN 0x400 중계
        	UART_SendGatewayData();
        	osDelay(1000);
        }
  /* USER CODE END Start_Task_UART */
}

/* USER CODE BEGIN Header_Start_Task_Heartbeat */
/**
* @brief Function implementing the Task_Heartbeat thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Task_Heartbeat */
void Start_Task_Heartbeat(void const * argument)
{
  /* USER CODE BEGIN Start_Task_Heartbeat */

  /* Infinite loop */
//	for(;;)
//	  {
////	    Heartbeat_CheckTimeout();
////	    CAN_SendHeartbeat();
////	    osDelay(1000);
//		char dbg[] = "Heartbeat Task\r\n";
//
//		    HAL_UART_Transmit(&huart2,
//		                      (uint8_t *)dbg,
//		                      sizeof(dbg) - 1,
//		                      100);
//
//		    Heartbeat_CheckTimeout();
//
//		    CAN_SendHeartbeat();
//
//		    osDelay(1000);
//	  }
	for (;;)
	{
		printf("Heartbeat Loop\r\n");
	    CAN_SendHeartbeat();
	    Heartbeat_CheckTimeout();

//		HAL_UART_Transmit(
//		            &huart2,
//		            (uint8_t*)"HB_TASK\r\n",
//		            9,
//		            100);
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	    osDelay(1000);
	}
  /* USER CODE END Start_Task_Heartbeat */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
