#ifndef CAN_DATA_H
#define CAN_DATA_H

#include "main.h"

typedef struct
{
    CAN_RxHeaderTypeDef header;
    uint8_t data[8];

} CAN_Message_t;S

#endif
