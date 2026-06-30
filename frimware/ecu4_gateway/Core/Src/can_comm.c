#include "can_comm.h"
#include "gateway_data.h"
#include "uart_comm.h" 
#include "FreeRTOS.h"  
#include "semphr.h"    

// 1. CAN 필터 설정 (수신 허용 ID 설정)
void CAN_Filter_Config(void) {
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

    HAL_CAN_ConfigFilter(&hcan1, &canFilterConfig);
}

// 2. 수신된 CAN 메시지 파싱 및 통합 구조체 저장
void CAN_ProcessRxMessage(void) {
    CAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[ 8 ]; // 에러 원인 해결: CAN 페이로드 8바이트 배열 명시

    while (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0) {
        if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {
            
            // 공유 데이터 접근을 위한 Mutex 획득
            if (xSemaphoreTake(gatewayDataMutex, portMAX_DELAY) == pdTRUE) {
                
                switch (rxHeader.StdId) {
                    case 0x100: // ControlData (ECU3)
                        gatewayData.controlData.riskLevel  = rxData[ 0 ];
                        gatewayData.controlData.brakeLevel = rxData[ 1 ];
                        gatewayData.controlData.wiperState = rxData[ 2 ];
                        gatewayData.controlData.ledState   = rxData[ 3 ];
                        break;
                        
                    case 0x200: // DrivingData (ECU2)
                        gatewayData.drivingData.speed    = rxData[ 0 ];
                        // 16비트 데이터를 2개의 8비트 배열에서 합치기
                        gatewayData.drivingData.distance = (rxData[ 1 ] << 8) | rxData[ 2 ];
                        break;
                        
                    case 0x300: // SensorData (ECU1)
                        gatewayData.sensorData.temp = (float)rxData[ 0 ];
                        gatewayData.sensorData.humi = (float)rxData[ 1 ];
                        // 16비트 데이터 조립
                        gatewayData.sensorData.lux  = (rxData[ 2 ] << 8) | rxData[ 3 ];
                        break;
                        
                    // 하트비트 수신 처리
                    case 0x701: gatewayData.heartbeat1 = 1; break;
                    case 0x702: gatewayData.heartbeat2 = 1; break;
                    case 0x703: gatewayData.heartbeat3 = 1; break;
                }
                
                xSemaphoreGive(gatewayDataMutex);
            }
        }
    }
}

// 3. ESP32에서 들어온 제어 명령을 CAN 버스(0x400)로 브로드캐스팅
void CAN_SendCommand(void) {
    CAN_TxHeaderTypeDef txHeader;
    uint8_t txData[ 8 ]; // 에러 원인 해결: 8바이트 배열 명시
    uint32_t txMailbox;

    txHeader.StdId = 0x400; // 제어 명령 0x400
    txHeader.ExtId = 0x00;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.IDE = CAN_ID_STD;
    txHeader.DLC = 8; 
    txHeader.TransmitGlobalTime = DISABLE;

    // 파싱된 구조체 데이터를 CAN 8바이트 페이로드에 매핑
    txData[ 0 ] = parsedCommand.commandId; // Byte 0
    txData[ 1 ] = parsedCommand.targetEcu; // Byte 1
    
    // Byte 2~7: Parameters
    for (int i = 0; i < 6; i++) {
        txData[ 2 + i ] = parsedCommand.parameters[ i ];
    }

    HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox);
}

// 4. ECU4 Gateway 자신의 생존 상태 송신
void CAN_SendHeartbeat(void) {
    CAN_TxHeaderTypeDef txHeader;
    uint8_t txData[ 1 ] = { 1 }; // 에러 원인 해결: 1바이트 배열 명시 및 초기화
    uint32_t txMailbox;

    txHeader.StdId = 0x704; 
    txHeader.ExtId = 0x00;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.IDE = CAN_ID_STD;
    txHeader.DLC = 1;
    txHeader.TransmitGlobalTime = DISABLE;

    HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox);
}