#include "heartbeat_task.h"

#include "cmsis_os.h"

#include "can_comm.h"

void HeartbeatTask(void const *argument)
{
    for (;;)
    {
        CAN_SendHeartbeat();

        osDelay(1000);
    }
}
