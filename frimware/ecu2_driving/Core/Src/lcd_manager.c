/*
 * lcd_manager.c
 *
 *  Created on: 2026. 7. 2.
 *      Author: itbank405_12
 */


#include "lcd_manager.h"
#include "driving_data.h"
#include "i2c_lcd.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

extern I2C_LCD_HandleTypeDef lcd;
extern UART_HandleTypeDef huart2;
extern osMutexId Mutex_I2CHandle;

void LCD_Task_Run(void) {
    char lcd_buf[32];       // 문자열 포맷팅을 위한 버퍼 (16x2 LCD 기준)
    char uart_buf[64];      // 데이터 출력을 위한 버퍼

	// 1. 센서 데이터를 출력
	sprintf(uart_buf, "Speed: %3d km/h | Dist: %3d cm\r\n", drivingData.speed, drivingData.distance);
	// 2. UART로 전송
	HAL_UART_Transmit(&huart2, (uint8_t*) uart_buf, strlen(uart_buf), 100);

	// 1. I2C 버스 독점권을 얻기 위해 Mutex 대기
	if (osMutexWait(Mutex_I2CHandle, osWaitForever) == osOK) {
		// 가변저항(속도) 데이터 출력
		lcd_gotoxy(&lcd, 0, 0);
		sprintf(lcd_buf, "Speed: %3d km/h", drivingData.speed);
		lcd_puts(&lcd, lcd_buf);

		// 초음파 센서(거리) 데이터 출력
		lcd_gotoxy(&lcd, 0, 1);
		sprintf(lcd_buf, "Dist : %3d cm  ", drivingData.distance);
		lcd_puts(&lcd, lcd_buf);

		// 2. 제어가 끝나면 반드시 Mutex 해제
		osMutexRelease(Mutex_I2CHandle);
	}
	// 화면 갱신 주기
	osDelay(500);
}
