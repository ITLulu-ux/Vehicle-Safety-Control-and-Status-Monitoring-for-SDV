/**
 *  @file mk_dht11.h
 *	@brief DHT11 Library
 *  @date Created on: Oct 4, 2019
 *  @author Author: mesut.kilic
 *	@version 1.0.0
 */


#ifndef MK_DHT11_H_
#define MK_DHT11_H_

//#include "stm32l0xx.h"
#include "main.h"
#include "gpio.h"

#define OUTPUT 1
#define INPUT 0

/**
 * @brief DHT11 struct
 */
typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
    TIM_HandleTypeDef *htim;

    uint8_t temperature;
    uint8_t humidity;

} DHT11_HandleTypeDef;


void DHT11_Init(DHT11_HandleTypeDef *dht,
                GPIO_TypeDef *port,
                uint16_t pin,
                TIM_HandleTypeDef *htim); // 호출부 매개변수 순서에 맞춰서 작성

void DHT11_SetGpioMode(DHT11_HandleTypeDef *dht,
                       uint8_t pMode);

uint8_t DHT11_Read(DHT11_HandleTypeDef *dht);

#endif /* MK_DHT11_H_ */
