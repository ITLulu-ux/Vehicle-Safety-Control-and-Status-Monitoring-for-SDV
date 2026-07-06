/*
 * lcd_manager.c
 *
 *  Created on: 2026. 7. 2.
 *      Author: itbank405_12
 */


#include "lcd_manager.h"
#include "driving_data.h"
#include "i2c_lcd.h"
#include "ota.h"
#include "main.h"
#include <stdio.h>

extern I2C_LCD_HandleTypeDef lcd;
extern osMutexId Mutex_I2CHandle;

void LCD_Task_Run(void) {
    char lcd_buf[32];
    uint8_t speed;
    uint16_t distance;

    if (ota_mode_active != 0U) {
        osDelay(100);
        return;
    }

    osMutexWait(drivingMutexHandle, osWaitForever);
    speed = drivingData.speed;
    distance = drivingData.distance;
    osMutexRelease(drivingMutexHandle);

	if (osMutexWait(Mutex_I2CHandle, osWaitForever) == osOK) {
		lcd_gotoxy(&lcd, 0, 0);
		sprintf(lcd_buf, "Speed: %3d km/h", speed);
		lcd_puts(&lcd, lcd_buf);

		lcd_gotoxy(&lcd, 0, 1);
		sprintf(lcd_buf, "Dist : %3d cm  ", distance);
		lcd_puts(&lcd, lcd_buf);

		osMutexRelease(Mutex_I2CHandle);
	}
	osDelay(500);
}
