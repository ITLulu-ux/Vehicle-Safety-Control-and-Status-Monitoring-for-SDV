#ifndef INC_HEARTBEAT_H_
#define INC_HEARTBEAT_H_

#include <stdint.h>

#define HEARTBEAT_TIMEOUT_SEC  3U

void Heartbeat_OnEcuReceived(uint8_t ecuNum);
void Heartbeat_CheckTimeout(void);

#endif /* INC_HEARTBEAT_H_ */
