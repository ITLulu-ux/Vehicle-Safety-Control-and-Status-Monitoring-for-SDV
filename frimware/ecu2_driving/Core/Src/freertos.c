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
#include "i2c_lcd.h"
#include "driving_data.h"
#include "can.h"
#include <stdio.h>
#include <string.h>

extern I2C_HandleTypeDef hi2c1;
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
extern I2C_LCD_HandleTypeDef lcd; // main.c에 있는 LCD 변수 가져오기
extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart2;

DrivingData_t drivingData = {0, 0, 0};

uint16_t ultrasonic_distance = 0;   // 초음파 거리 저장용

// 3. Mutex 핸들 (데이터 동시 접근 방지용)
osMutexId drivingMutexHandle;
/* USER CODE END Variables */

osThreadId LCD_TaskHandle;
osThreadId CAN_RX_TaskHandle;
osThreadId Distance_TaskHandle;
osThreadId Speed_TaskHandle;
osThreadId CAN_TX_TaskHandle;
osThreadId Heartbeat_TaskHandle;
osMutexId Mutex_I2CHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTask02(void const * argument);
void StartTask03(void const * argument);
void StartTask04(void const * argument);
void StartTask05(void const * argument);
void StartTask06(void const * argument);

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
  /* Create the mutex(es) */
  /* definition and creation of Mutex_I2C */
  osMutexDef(Mutex_I2C);
  Mutex_I2CHandle = osMutexCreate(osMutex(Mutex_I2C));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  osMutexDef(drivingMutex);
  drivingMutexHandle = osMutexCreate(osMutex(drivingMutex));
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of LCD_Task */
  osThreadDef(LCD_Task, StartDefaultTask, osPriorityLow, 0, 256);
  LCD_TaskHandle = osThreadCreate(osThread(LCD_Task), NULL);

  /* definition and creation of CAN_RX_Task */
  osThreadDef(CAN_RX_Task, StartTask02, osPriorityHigh, 0, 256);
  CAN_RX_TaskHandle = osThreadCreate(osThread(CAN_RX_Task), NULL);

  /* definition and creation of Distance_Task */
  osThreadDef(Distance_Task, StartTask03, osPriorityNormal, 0, 128);
  Distance_TaskHandle = osThreadCreate(osThread(Distance_Task), NULL);

  /* definition and creation of Speed_Task */
  osThreadDef(Speed_Task, StartTask04, osPriorityNormal, 0, 128);
  Speed_TaskHandle = osThreadCreate(osThread(Speed_Task), NULL);

  /* definition and creation of CAN_TX_Task */
  osThreadDef(CAN_TX_Task, StartTask05, osPriorityNormal, 0, 256);
  CAN_TX_TaskHandle = osThreadCreate(osThread(CAN_TX_Task), NULL);

  /* definition and creation of Heartbeat_Task */
  osThreadDef(Heartbeat_Task, StartTask06, osPriorityBelowNormal, 0, 128);
  Heartbeat_TaskHandle = osThreadCreate(osThread(Heartbeat_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the LCD_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  char lcd_buf[32];       // 문자열 포맷팅을 위한 버퍼 (16x2 LCD 기준)
  char uart_buf[64];  // 데이터 출력을 위한 버퍼

  /* Infinite loop */
  for(;;)
  {
	// 1. 센서 데이터를 출력
	sprintf(uart_buf, "Speed: %3d km/h | Dist: %3d cm\r\n", drivingData.speed, drivingData.distance);
	// 2. UART로 전송
	HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, strlen(uart_buf), 100);

	// 1. I2C 버스 독점권을 얻기 위해 Mutex 대기
	if (osMutexWait(Mutex_I2CHandle, osWaitForever) == osOK)
	{
		// 가변저항(속도) 데이터 출력
		lcd_gotoxy(&lcd, 0, 0);
		sprintf(lcd_buf, "Speed: %3d km/h", drivingData.speed);
		lcd_puts(&lcd, lcd_buf);

		// 초음파 센서(거리) 데이터 출력
		lcd_gotoxy(&lcd, 0, 1);
		sprintf(lcd_buf, "Dist : %3d cm  ", drivingData.distance);
		lcd_puts(&lcd, lcd_buf);

		// 2. 제어가 끝나면 반드시 Mutex 해제
		osMutexRelease(Mutex_I2CHandle);
	}
	// 화면 갱신 주기
	  osDelay(500);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the CAN_RX_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void const * argument)
{
  /* USER CODE BEGIN StartTask02 */
  CAN_RxHeaderTypeDef RxHeader;
  uint8_t RxData[8];
  /* Infinite loop */
  for(;;)
  {
	// 1. 수신 버퍼(FIFO0)에 메시지가 들어왔는지 확인
	if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0)
	{
		// 2. 메시지 꺼내오기
		if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
		{
			// 3. 수신된 CAN ID에 따라 동작 분류 (명세서 기준)
			if (RxHeader.StdId == 0x010) // 예: 게이트웨이 긴급 명령 (필요 시)
			{
				// 긴급 명령 처리 로직
			}
			else if (RxHeader.StdId == 0x400) // 예: OTA 펌웨어 데이터 (가정)
			{
				// OTA 처리 로직
			}
		}
	}
    osDelay(10);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the Distance_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void const * argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */

  for(;;)
  {
	// 1. 초음파 센서 Trigger 핀에 High 펄스 인가
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);

	// 2. 10us 이상 유지해야 하므로 FreeRTOS 최소 지연시간인 1ms 대기
	osDelay(1);

	// 3. Trigger 핀을 다시 Low로 변경
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);

	osDelay(50);

	// 4. 거리 구조체 업데이트
	osMutexWait(drivingMutexHandle, osWaitForever);
	drivingData.distance = ultrasonic_distance;
	osMutexRelease(drivingMutexHandle);

    osDelay(50);
  }
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
* @brief Function implementing the Speed_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask04 */
void StartTask04(void const * argument)
{
  /* USER CODE BEGIN StartTask04 */
  /* Infinite loop */
  for(;;)
  {
	// 1. ADC 변환 시작
	HAL_ADC_Start(&hadc1);

	// 2. 변환이 완료될 때까지 대기 (최대 10ms)
	if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
		// 3. 0~4095의 아날로그 값을 읽어와 0~150km/h로 스케일링(변환)
		uint32_t adc_val = HAL_ADC_GetValue(&hadc1);
		uint8_t current_speed = (uint8_t)((adc_val * 150) / 4095);

		// 4. Mutex로 잠그고 공유 구조체 업데이트
		osMutexWait(drivingMutexHandle, osWaitForever);
		drivingData.speed = current_speed;
		osMutexRelease(drivingMutexHandle);
	}

    osDelay(100);
  }
  /* USER CODE END StartTask04 */
}

/* USER CODE BEGIN Header_StartTask05 */
/**
* @brief Function implementing the CAN_TX_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask05 */
void StartTask05(void const * argument)
{
  /* USER CODE BEGIN StartTask05 */
  CAN_TxHeaderTypeDef TxHeader;
  uint8_t TxData[3]; // DLC=3 (속도 1바이트 + 거리 2바이트)
  uint32_t TxMailbox;

  TxHeader.StdId = 0x200;
  TxHeader.ExtId = 0;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.DLC = 3;
  TxHeader.TransmitGlobalTime = DISABLE;

  /* Infinite loop */
  for(;;)
  {
	// 1. Mutex를 통해 데이터 안전하게 복사
	osMutexWait(drivingMutexHandle, osWaitForever);

	TxData[0] = (uint8_t)drivingData.speed;	// Byte 0: 속도
	TxData[1] = (uint8_t)(drivingData.distance & 0xFF);         // Byte 1: 거리 하위 8비트
	TxData[2] = (uint8_t)((drivingData.distance >> 8) & 0xFF);  // Byte 2: 거리 상위 8비트

	osMutexRelease(drivingMutexHandle);

	// 2. CAN 송신 (인터럽트 확인)
	if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0)
	{
		if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
			// 필요시 에러 처리 (예: Error_Handler())
		}
	}

	osDelay(100); // 100ms 주기 송신
  }
  /* USER CODE END StartTask05 */
}

/* USER CODE BEGIN Header_StartTask06 */
/**
* @brief Function implementing the Heartbeat_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask06 */
void StartTask06(void const * argument)
{
  /* USER CODE BEGIN StartTask06 */
  CAN_TxHeaderTypeDef HeartbeatHeader;
  uint8_t HeartbeatData[1] = {0x01}; // 0x01: 정상 작동 중
  uint32_t TxMailbox;

  // 헤더 설정 (Heartbeat 메시지는 ID 0x702 사용)
  HeartbeatHeader.StdId = 0x702;
  HeartbeatHeader.ExtId = 0;
  HeartbeatHeader.IDE = CAN_ID_STD;
  HeartbeatHeader.RTR = CAN_RTR_DATA;
  HeartbeatHeader.DLC = 1; // 1바이트만 전송
  HeartbeatHeader.TransmitGlobalTime = DISABLE;

  /* Infinite loop */
  for(;;)
  {
	// 메일박스에 여유가 있을 때만 전송
	  if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0) {
		  HAL_CAN_AddTxMessage(&hcan1, &HeartbeatHeader, HeartbeatData, &TxMailbox);
	  }

	  osDelay(500);
  }
  /* USER CODE END StartTask06 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
