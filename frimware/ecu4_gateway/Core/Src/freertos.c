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
/* USER CODE END Variables */
osThreadId Task_CAN_ProcesHandle;
osThreadId Task_UART_ProceHandle;
osThreadId Task_HeartbeatHandle;
osMessageQId Queue_CAN_RxHandle;
osSemaphoreId Sem_UART_RxHandle;

extern CAN_HandleTypeDef hcan1;
extern UART_HandleTypeDef huart2;
extern uint8_t uartRxBuffer[32];

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
    // ⭐️ [추가] OS 셋업이 끝나고 태스크가 깨어난 '지금' 통신을 시작합니다!
    if (HAL_CAN_Start(&hcan1) == HAL_OK) {
        printf("[CAN] Start OK\r\n");
    }
    // ⭐️ [추가] CAN 수신 인터럽트(초인종) 켜기
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

    osEvent event;
    for(;;)
    {
        // CAN RX 인터럽트 발생 시까지 무한 대기
        event = osMessageGet(Queue_CAN_RxHandle, osWaitForever);
        if(event.status == osEventMessage) {
            printf("[Task] 큐 수신 완료! 파싱 진입...\r\n"); // 생사 확인용 로그
            CAN_ProcessRxMessage();
        }
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
    HAL_UART_Receive_IT(&huart2, uartRxBuffer, sizeof(uartRxBuffer));

    for(;;)
    {
        // 100ms마다 라즈베리파이로 데이터 전송, 그 사이에 명령이 오면 수신 파싱
        if(osSemaphoreWait(Sem_UART_RxHandle, 100) == osOK) {
            UART_ProcessRxMessage();
        } else {
            UART_SendGatewayData();
        }
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
	for(;;)
	  {
	    CAN_SendHeartbeat();
	    osDelay(1000);
	  }
  /* USER CODE END Start_Task_Heartbeat */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
