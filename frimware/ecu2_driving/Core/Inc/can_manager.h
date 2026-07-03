/*
 * can_manager.h
 *
 *  Created on: 2026. 7. 2.
 *      Author: itbank405_12
 */

#ifndef INC_CAN_MANAGER_H_
#define INC_CAN_MANAGER_H_

void CAN_RX_Task_Run(void);
void CAN_TX_Task_Run(void);
void Heartbeat_Task_Run(void);

#endif /* INC_CAN_MANAGER_H_ */
