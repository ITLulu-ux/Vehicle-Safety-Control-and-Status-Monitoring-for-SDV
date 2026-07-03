

#include "can_comm.h"
#include "can.h"

CAN_TxHeaderTypeDef TxHeader;
uint32_t TxMailbox;

void CAN_Init(void)
{
    HAL_CAN_Start(&hcan1);

    TxHeader.StdId = 0x300;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 4;
}

HAL_StatusTypeDef CAN_SendSensorData(SensorData_t *data)
{
    uint8_t txData[8];

    int16_t temp = (int16_t)(data->temperature * 10);
    int16_t hum  = (int16_t)(data->humidity * 10);

    txData[0] = data->temperature;
    txData[1] = data->humidity;

    txData[2] = (uint8_t)(data->lux >> 8);
    txData[3] = (uint8_t)(data->lux);

    txData[4] = 0;
    txData[5] = 0;

    return HAL_CAN_AddTxMessage(&hcan1,
                                &TxHeader,
                                txData,
                                &TxMailbox);
}
