
#include "can_comm.h"
#include "ota.h"
#include "can.h"
#include "cmsis_os.h"

CAN_TxHeaderTypeDef TxHeader;
uint32_t TxMailbox;

extern CAN_HandleTypeDef hcan1;

static void CAN_HandleDownlink(const uint8_t *rxData)
{
    if (rxData[1] != MY_ECU_ID) {
        return;
    }

    switch (rxData[0]) {
        case CMD_RESET:
            HAL_NVIC_SystemReset();
            break;
        case CMD_OTA_START:
            OTA_Start(rxData);
            break;
        case CMD_OTA_DATA:
            OTA_WriteChunk(rxData);
            break;
        case CMD_OTA_END:
            OTA_End(rxData);
            break;
        default:
            break;
    }
}

void CAN_Init(void)
{
    CAN_FilterTypeDef canFilterConfig = {0};

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
        Error_Handler();
    }

    HAL_CAN_Start(&hcan1);

    TxHeader.StdId = 0x300;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 4;
}

void CAN_RX_Task_Run(void)
{
    CAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[8];

    while (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0) {
        if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rxHeader, rxData) != HAL_OK) {
            break;
        }
        if (rxHeader.StdId == CAN_ID_CMD_DOWNLINK) {
            CAN_HandleDownlink(rxData);
        }
    }

    osDelay(10);
}

void Heartbeat_Task_Run(void)
{
    CAN_TxHeaderTypeDef hbHeader = {0};
    uint8_t hbData[1] = {0x01};
    uint32_t txMailbox;

    if (ota_mode_active != 0U) {
        osDelay(2000);
        return;
    }

    hbHeader.StdId = CAN_ID_HB_ECU1;
    hbHeader.IDE = CAN_ID_STD;
    hbHeader.RTR = CAN_RTR_DATA;
    hbHeader.DLC = 1;

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0) {
        HAL_CAN_AddTxMessage(&hcan1, &hbHeader, hbData, &txMailbox);
    }

    osDelay(2000);
}

HAL_StatusTypeDef CAN_SendSensorData(SensorData_t *data)
{
    uint8_t txData[8];

    txData[0] = data->temperature;
    txData[1] = data->humidity;
    txData[2] = (uint8_t)(data->lux >> 8);
    txData[3] = (uint8_t)(data->lux);
    txData[4] = 0;
    txData[5] = 0;

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0U) {
        return HAL_BUSY;
    }

    return HAL_CAN_AddTxMessage(&hcan1,
                                &TxHeader,
                                txData,
                                &TxMailbox);
}
