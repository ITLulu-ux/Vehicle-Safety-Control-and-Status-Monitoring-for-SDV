/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "can.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// 위험도 정의 전역 열거형 (freertos.c에서도 extern으로 사용 가능)
typedef enum {
    RISK_SAFE = 0,
    RISK_CAUTION,
    RISK_WARNING,
    RISK_DANGER
} RiskLevel_t;

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

/* USER CODE BEGIN PV */
// [추가] 다른 파일(freertos.c)에 있는 큐 핸들러를 main.c로 땡겨옵니다!
extern osMessageQueueId_t Queue_CAN_RXHandle;

// 전역 변수 선언 (인터럽트 수신 데이터 저장용)
CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];

// 위험도 자체 판단용 상태 변수
volatile uint8_t current_risk_level = RISK_SAFE;

// CAN 송신용 변수 (ECU3 상태 및 Heartbeat 송신용)
CAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];
uint32_t TxMailbox;

// [OTA 업데이트용 상태 변수]
volatile uint8_t ota_mode_active = 0;
uint32_t ota_file_size = 0;
uint32_t ota_received_bytes = 0;

volatile uint8_t current_humidity = 0; // ECU1에게 받을 습도 데이터 저장용
volatile uint16_t current_lux = 0;     // 조도 데이터 저장용
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void CAN_Filter_Config(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * @brief  CAN 메시지 수신 완료 콜백 함수 (인터럽트 방식)
  * 기능 명세서의 [CAN 수신] 및 Gateway를 통한 [제어 명령 수신]을 처리합니다.
  */
// main.c 내부 인터럽트 함수 강추 수정본
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    if (hcan->Instance == CAN1)
    {
        CAN_Rx_Format_t rxMsg;
        CAN_RxHeaderTypeDef header;

        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &header, rxMsg.Data) == HAL_OK)
        {
            rxMsg.StdId = header.StdId;
            rxMsg.DLC = header.DLC;

            // 인터럽트에서는 즉시 FreeRTOS 최상위 수신큐로 휙 던지고 빠져나옵니다.
            // 대기 시간(Timeout)은 무조건 0이어야 ISR 안에서 안전합니다.
            osMessageQueuePut(Queue_CAN_RXHandle, &rxMsg, 0, 0);
        }
    }
}
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
  MX_CAN1_Init();
  MX_CAN2_Init();
  MX_TIM4_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  // 1. CAN 필터 설정 및 가동 활성화
  CAN_Filter_Config();
  HAL_CAN_Start(&hcan1);

  // 2. CAN FIFO0 수신 인터럽트(Notification) 활성화
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

  // 3. 서보모터 제어용 TIM4 PWM CH1(브레이크), CH2(와이퍼) 가동 시작
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

  // 초기 서보모터 위치 설정 (정지 혹은 기본 위치 예: 1.5ms = 1500)
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 1500);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 1500);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/**
  * @brief  CAN1 통신 필터 기본 세팅 함수
  * 모든 ID의 CAN 데이터를 우선 수신하도록 설정합니다. (테스트 및 개발용)
  */
void CAN_Filter_Config(void)
{
    CAN_FilterTypeDef  sFilterConfig;

    sFilterConfig.FilterBank = 0;                     // 필터 뱅크 0번 사용
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK; // 마스크 모드 사용
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;          // 0으로 설정 시 모든 ID 통과 수신
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;// 수신 데이터를 FIFO0로 할당
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
