//
// Created by wsmrxd on 2024/3/18.
//

#ifndef MYPLAYER_LCD_H
#define MYPLAYER_LCD_H

#include "stm32f1xx_hal.h"
#include "lvgl/lvgl.h"
#include "FreeRTOSConfig.h"

#define LCD_H_RES 240
#define LCD_V_RES 320

#define LCD_RESET_Pin GPIO_PIN_8
#define LCD_CS_Pin GPIO_PIN_10
#define LCD_DCX_Pin GPIO_PIN_9

#define LCD_IRQ_PRIORITY 13

#if ((LCD_IRQ_PRIORITY << 4) | 0xF) <= configMAX_SYSCALL_INTERRUPT_PRIORITY
#error "LCD should not interrupt within FreeRTOS critical zone, please set `LCD_IRQ_PRIORITY` higher than 11"
#endif

extern lv_display_t* lcd_disp;

HAL_StatusTypeDef lcdInit();

void lcd_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size);

void lcd_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size);

#endif // MYPLAYER_LCD_H
