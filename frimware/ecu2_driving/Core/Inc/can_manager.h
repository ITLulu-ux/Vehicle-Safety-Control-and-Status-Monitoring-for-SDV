/*
 * can_manager.h
 *
 *  Created on: 2026. 7. 2.
 *      Author: itbank405_12
 */

#ifndef INC_CAN_MANAGER_H_
#define INC_CAN_MANAGER_H_

#define CAN_ID_CMD_DOWNLINK  0x400U
#define ECU2_TARGET_ID       0x02U

void CAN_RX_Task_Run(void);
void CAN_TX_Task_Run(void);
void Heartbeat_Task_Run(void);

extern volatile uint8_t ota_mode_active;

#endif /* INC_CAN_MANAGER_H_ */
