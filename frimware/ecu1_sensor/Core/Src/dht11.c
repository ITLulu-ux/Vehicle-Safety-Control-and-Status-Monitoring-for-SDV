/**
 *  @file dht11.c
 *	@brief DHT11 Library
 *  @date Created on: Oct 4, 2019
 *  @author Author: mesut.kilic
 *	@version 1.0.0
 */

#include "dht11.h"

/**
 * @brief configure dht11 struct with given parameter
 * @param htim TIMER for calculate delays ex:&htim2
 * @param port GPIO port ex:GPIOA
 * @param pin GPIO pin ex:GPIO_PIN_2
 * @param dht struct to configure ex:&dht
 */
void DHT11_Init(DHT11_HandleTypeDef *dht,GPIO_TypeDef* port,uint16_t pin,TIM_HandleTypeDef *htim){
	dht->htim = htim;
	dht->port = port;
	dht->pin = pin;
}

/**
 * @brief set DHT pin direction with given parameter
 * @param dht struct for dht
 * @param pMode GPIO Mode ex:INPUT or OUTPUT
 */
void DHT11_SetGpioMode(DHT11_HandleTypeDef *dht, uint8_t pMode)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(pMode == OUTPUT)
	{
	  GPIO_InitStruct.Pin = dht->pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	  HAL_GPIO_Init(dht->port, &GPIO_InitStruct);
	}else if(pMode == INPUT)
	{
	  GPIO_InitStruct.Pin = dht->pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
//	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	  HAL_GPIO_Init(dht->port, &GPIO_InitStruct);
	}
}

/**
 * @brief reads dht11 value
 * @param dht struct for dht11
 * @return 1 if read ok 0 if something wrong in read
 */
uint8_t DHT11_Read(DHT11_HandleTypeDef *dht)
{
	uint16_t mTime1 = 0, mTime2 = 0, mBit = 0;
	uint8_t humVal = 0;
	uint8_t humDec = 0;
	uint8_t tempVal = 0;
	uint8_t tempDec = 0;
	uint8_t parityVal = 0;
	uint8_t genParity = 0;
	uint8_t mData[40];

	//start comm
	DHT11_SetGpioMode(dht, OUTPUT);			//set pin direction as input
	HAL_GPIO_WritePin(dht->port, dht->pin, GPIO_PIN_RESET);
//	printf("Drive LOW\r\n");
	HAL_Delay(1);
//	printf("Pin=%d\r\n", HAL_GPIO_ReadPin(dht->port, dht->pin));
	HAL_Delay(18);					//wait 18 ms in Low state
	__HAL_TIM_SET_COUNTER(dht->htim, 0);
//	printf("CNT=%lu\r\n", __HAL_TIM_GET_COUNTER(dht->htim));
	__disable_irq();	//disable all interupts to do only read dht otherwise miss timer
	HAL_TIM_Base_Start(dht->htim);

	DHT11_SetGpioMode(dht, INPUT);
//	delay_us(30);




	//check dht answer
	__HAL_TIM_SET_COUNTER(dht->htim, 0);
	while(HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_SET)
	{
	    if(__HAL_TIM_GET_COUNTER(dht->htim) > 1000)
	    {
//	    	HAL_TIM_Base_Stop(dht->htim);
	        __enable_irq();
	        return 0;
	    }
	}
	__HAL_TIM_SET_COUNTER(dht->htim, 0);
	while(HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_RESET)
	{
	    if((uint16_t)__HAL_TIM_GET_COUNTER(dht->htim) > 1000)
	    {
//	    	HAL_TIM_Base_Stop(dht->htim);
	        __enable_irq();
	        return 0;
	    }
	}
	mTime1 = (uint16_t)__HAL_TIM_GET_COUNTER(dht->htim);
	__HAL_TIM_SET_COUNTER(dht->htim, 0);
	while(HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_SET)
	{
	    if((uint16_t)__HAL_TIM_GET_COUNTER(dht->htim) > 1000)
	    {
//	    	HAL_TIM_Base_Stop(dht->htim);
	        __enable_irq();
	        return 0;
	    }
	}
	mTime2 = (uint16_t)__HAL_TIM_GET_COUNTER(dht->htim);

	//if answer is wrong return
	if(mTime1 < 10 || mTime1 > 120 || mTime2 < 10 || mTime2 > 120)
	{
		__enable_irq();
		return 0;
	}

//	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	// 40bit 읽기
	for (int j = 0; j < 40; j++)
	{
	    __HAL_TIM_SET_COUNTER(dht->htim, 0);

	    while (HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_RESET)
	    {
	        if (__HAL_TIM_GET_COUNTER(dht->htim) > 1000)
	        {
	            __enable_irq();
	            return 0;
	        }
	    }

	    __HAL_TIM_SET_COUNTER(dht->htim, 0);

	    while (HAL_GPIO_ReadPin(dht->port, dht->pin) == GPIO_PIN_SET)
	    {
	        if (__HAL_TIM_GET_COUNTER(dht->htim) > 1000)
	        {
	            __enable_irq();
	            return 0;
	        }
	    }

	    mTime1 = __HAL_TIM_GET_COUNTER(dht->htim);

	    if (mTime1 > 15 && mTime1 < 40)
	        mBit = 0;
	    else if (mTime1 > 45 && mTime1 < 90)
	        mBit = 1;
	    else
	        mBit = 0;

	    mData[j] = mBit;
	}

	/* ===== 디버그 출력 ===== */


//	for(int i=0;i<40;i++)
//	{
//	    printf("%d", mData[i]);
//	}
//
//	printf("\r\n");

	HAL_TIM_Base_Stop(dht->htim); //stop timer
	__enable_irq(); //enable all interrupts

	//get hum value from data buffer
//	for(int i = 0; i < 8; i++)
//	{
//		humVal += mData[i];
//		humVal = humVal << 1;
//	}
	humVal = 0;
	for(int i=0;i<8;i++)
	{
	    humVal = (humVal << 1) | mData[i];
	}
	humDec = 0;
	for(int i=8;i<16;i++)
	{
	    humDec = (humDec << 1) | mData[i];
	}
	//get temp value from data buffer
//	for(int i = 16; i < 24; i++)
//	{
//		tempVal += mData[i];
//		tempVal = tempVal << 1;
//	}
	tempVal = 0;
	for(int i=16;i<24;i++)
	{
	    tempVal = (tempVal << 1) | mData[i];
	}
	tempDec = 0;
	for(int i=24;i<32;i++)
	{
	    tempDec = (tempDec << 1) | mData[i];
	}

	//get parity value from data buffer
//	for(int i = 32; i < 40; i++)
//	{
//		parityVal += mData[i];
//		parityVal = parityVal << 1;
//	}
	parityVal = 0;
	for(int i=32;i<40;i++)
	{
	    parityVal = (parityVal << 1) | mData[i];
	}

//	parityVal = parityVal >> 1;
//	humVal = humVal >> 1;
//	tempVal = tempVal >> 1;

	genParity = humVal + humDec + tempVal + tempDec;

	dht->temperature = tempVal;
	dht->humidity = humVal;


	return 1;
}
