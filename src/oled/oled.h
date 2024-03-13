#ifndef __OLED_H
#define __OLED_H

#include "stm32f1xx_hal.h"

void OLED_Init();
void OLED_Clear();

void OLED_ShowChar(uint8_t row, uint8_t col, signed char ch);
void OLED_ShowString(uint8_t row, uint8_t col, char* str);

void OLED_ShowGBK(uint8_t row, uint8_t col, uint8_t* font);
void OLED_ShowGBKString(uint8_t row, uint8_t col, uint8_t padding, char* str, uint8_t* font);
void OLED_ShowPaddingString(uint8_t row, uint8_t col, char* str, uint8_t padding);

void OLED_flushScreen();

#endif
