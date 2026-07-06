/*
 * can_comm.h
 *
 *  Created on: 2026. 6. 30.
 *      Author: rlaek
 */

// can_comm.h
#ifndef INC_CAN_COMM_H_
#define INC_CAN_COMM_H_

#include "main.h"
#include "downlink.h"

// 엑셀 명세 기반 CAN ID 매핑
#define CAN_ID_ECU1_ENV      0x300
#define CAN_ID_ECU2_DRIVE    0x200
#define CAN_ID_ECU3_STATUS   0x100
#define CAN_ID_GATEWAY_CMD   CAN_ID_DOWNLINK
#define CAN_ID_CMD_DOWNLINK  CAN_ID_DOWNLINK
#define CAN_ID_ECU3_HB       0x703

// CAN 수신을 위한 큐 이송 포맷 구조체
typedef struct {
    uint32_t StdId;
    uint8_t  DLC;
    uint8_t  Data[8];
} CAN_Rx_Format_t;

void CAN_HandleDownlink(const uint8_t *rxData);
void CAN_Send_Status(void);

#endif /* INC_CAN_COMM_H_ */
