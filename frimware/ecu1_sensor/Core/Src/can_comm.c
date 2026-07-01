

#include "can_comm.h"

#include "can.h"
#include "cmsis_os.h"

#include "sensor_data.h"
// CAN 프레임의 헤더 정보를 저장하는 구조체
static CAN_TxHeaderTypeDef txHeader;
static CAN_RxHeaderTypeDef rxHeader;

static uint32_t txMailbox;
static uint8_t txData[8];
static uint8_t rxData[8];

/* CAN 초기화 */
void CAN_Init(void)
{
    CAN_FilterTypeDef filter;

    filter.FilterActivation = ENABLE;
    filter.FilterBank = 0;
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;

    filter.FilterIdHigh = 0x0000;
    filter.FilterIdLow = 0x0000;

    filter.FilterMaskIdHigh = 0x0000;
    filter.FilterMaskIdLow = 0x0000;

    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;

    if (HAL_CAN_ConfigFilter(&hcan1, &filter) != HAL_OK)
    {
    	Error_Handler();
    }
    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_CAN_ActivateNotification(&hcan1,
            CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
    	Error_Handler();
    }
    txHeader.StdId = 0x300;
    txHeader.ExtId = 0;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.DLC = 8;
    txHeader.TransmitGlobalTime = DISABLE;
}

/* 환경 데이터 송신 */
//CAN Task가 호출하는 함수
void CAN_SendSensorData(const SensorData_t *data)
{
    int16_t temperature = (int16_t)(data->temperature * 100);
    uint16_t humidity   = (uint16_t)(data->humidity * 100);
    uint16_t lux        = data->lux;

    txData[0] = (temperature >> 8) & 0xFF;
    txData[1] = temperature & 0xFF;

    txData[2] = (humidity >> 8) & 0xFF;
    txData[3] = humidity & 0xFF;

    txData[4] = (lux >> 8) & 0xFF;
    txData[5] = lux & 0xFF;

    txData[6] = 0x00;
    txData[7] = 0x00;

    if(HAL_CAN_AddTxMessage(&hcan1,
                            &txHeader,
                            txData,
                            &txMailbox) != HAL_OK)
    {
        /* TODO : CAN 송신 오류 처리 */
    }
}

/* ECU4 제어 명령 처리 */
void CAN_ProcessRxMessage(void)
{
	if (HAL_CAN_GetRxMessage(&hcan1,
	                         CAN_RX_FIFO0,
	                         &rxHeader,
	                         rxData) != HAL_OK)
	{
	    return;
	}

    switch(rxHeader.StdId)
    {
        case 0x500:
            /* OTA, Reset, 설정 변경 처리 */
            break;

        default:
            break;
    }
}

/* Heartbeat 송신 */
void CAN_SendHeartbeat(void)
{
    txHeader.StdId = 0x701;

    txData[0] = 0x01;
    txData[1] = 0x00;
    txData[2] = 0x00;
    txData[3] = 0x00;
    txData[4] = 0x00;
    txData[5] = 0x00;
    txData[6] = 0x00;
    txData[7] = 0x00;

    if(HAL_CAN_AddTxMessage(&hcan1,
                                &txHeader,
                                txData,
                                &txMailbox) != HAL_OK)
        {
            /* TODO : CAN 송신 오류 처리 */
        }

    /* 환경 데이터 CAN ID로 복원 */
    txHeader.StdId = 0x300;
}
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
/*
 * CAN interrupt
 * 		|
 * Semaphore Give
 * 		|
 * CanRxTask
 * 를 만들어주는 코드
 * */
{
	if (hcan->Instance == CAN1) // CAN2 추가 대비
	{
	    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	    xSemaphoreGiveFromISR(
	            canRxSemaphore,
	            &xHigherPriorityTaskWoken);

	    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}
