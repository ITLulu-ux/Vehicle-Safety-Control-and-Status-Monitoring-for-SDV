/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "can.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c_lcd.h"
#include "driving_data.h"
#include "can.h"
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

/* USER CODE BEGIN PV */
I2C_LCD_HandleTypeDef lcd;  // LCD 핸들 구조체 전역 변수 선언

volatile uint32_t IC_Val1 = 0;
volatile uint32_t IC_Val2 = 0;
volatile uint32_t Difference = 0;
volatile uint8_t Is_First_Captured = 0;  // 첫 번째 엣지(Rising) 캡처 여부 플래그

extern uint16_t ultrasonic_distance; // freertos.c에 있는 변수 가져오기
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_CAN1_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	// 1. I2C LCD 하드웨어 핸들 매핑
	lcd.hi2c = &hi2c1;
	uint8_t start_msg[] = "System Start!\r\n";
	HAL_UART_Transmit(&huart2, start_msg, sizeof(start_msg) - 1, 100);

	// 2. I2C LCD 주소 자동 스캔 및 설정
	HAL_Delay(1000);
	if (HAL_I2C_IsDeviceReady(&hi2c1, 0x27 << 1, 5, 100) == HAL_OK) {
		lcd.address = 0x27 << 1; // 0x27 주소 매핑
	} else if (HAL_I2C_IsDeviceReady(&hi2c1, 0x3F << 1, 5, 100) == HAL_OK) {
		lcd.address = 0x3F << 1; // 0x3F 주소 매핑
	} else {
		lcd.address = 0x27 << 1;
	}

	// 3. 올바른 주소로 LCD 초기화 진행
	lcd_init(&lcd);

	// 4. CAN 필터 및 인터럽트 설정
	CAN_FilterTypeDef canFilterConfig;
	canFilterConfig.FilterBank = 0;
	canFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	canFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	canFilterConfig.FilterIdHigh = 0x0000;
	canFilterConfig.FilterIdLow = 0x0000;
	canFilterConfig.FilterMaskIdHigh = 0x0000;
	canFilterConfig.FilterMaskIdLow = 0x0000;
	canFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	canFilterConfig.FilterActivation = ENABLE;
	canFilterConfig.SlaveStartFilterBank = 14;

	if (HAL_CAN_ConfigFilter(&hcan1, &canFilterConfig) != HAL_OK) {
		uint8_t err_msg[] = "CAN Filter Error!\r\n";
		HAL_UART_Transmit(&huart2, err_msg, sizeof(err_msg) - 1, 100);
	}

	// CAN 통신 시작
	HAL_CAN_Start(&hcan1);

	// 수신 인터럽트 활성화
	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING)
			!= HAL_OK) {
		uint8_t err_msg[] = "CAN Interrupt Error!\r\n";
		HAL_UART_Transmit(&huart2, err_msg, sizeof(err_msg) - 1, 100);
	}

	// 5. 초음파 Echo 핀 캡처 인터럽트 시작
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// 1. 변수 선언 (volatile 키워드 추가: 인터럽트와 태스크 간 데이터 동기화 보장)
// UART 출력용
int __io_putchar(int ch) {
	HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, 10);
	return ch;
}

// 2. 타이머 인터럽트 콜백 함수
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	// PA15에 연결된 TIM2_CH1 인터럽트 확인
	if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
		if (Is_First_Captured == 0) // 상승 엣지(Rising Edge) 감지 -> 측정 시작
				{
			IC_Val1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			Is_First_Captured = 1;

			// 다음 인터럽트는 하강 엣지(Falling Edge)에서 발생하도록 설정 변경
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1,
					TIM_INPUTCHANNELPOLARITY_FALLING);
		} else if (Is_First_Captured == 1) // 하강 엣지(Falling Edge) 감지
				{
			IC_Val2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

			// 머무른 시간(펄스 폭) 계산
			if (IC_Val2 > IC_Val1) {
				Difference = IC_Val2 - IC_Val1;
			} else if (IC_Val1 > IC_Val2) {
				Difference = (0xFFFFFFFF - IC_Val1) + IC_Val2; // 오버플로우 처리
			}

			// 거리 계산: (시간차(us) * 0.034) / 2
			ultrasonic_distance = (uint16_t) (Difference * 0.034f / 2.0f);

			// ⭐️ [디버그 추가] 초음파 하드웨어 인터럽트 계산 확인
			printf("[TIM2 IC] 초음파 원시 거리: %d cm\r\n", ultrasonic_distance);

			Is_First_Captured = 0;

			// 다음 측정을 위해 다시 상승 엣지 대기
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1,
					TIM_INPUTCHANNELPOLARITY_RISING);
		}
	}
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	  /* User can add his own implementation to report the file name and line number,
		 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
