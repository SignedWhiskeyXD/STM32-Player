/*
 * ************************************************
 * 
 *              STM32 blink gcc demo
 * 
 *  CPU: STM32F103C8
 *  PIN: PA1
 * 
 * ************************************************
*/

#include "stm32f10x.h"
#include "oled/OLED.h"
#include "states/states.h"
#include "button.h"
#include "display.h"

#define LED_PERIPH RCC_APB2Periph_GPIOA
#define LED_PORT GPIOA
#define LED_PIN GPIO_Pin_1

void delay(int x)
{
    for (int i = 0; i < x; i++)
    {
        for (int j = 0; j < 1000; j++)
            __NOP();
    }
}

void initPlayer()
{
    initScreen();

    initKeys();
    initSD();
    loadFiles();

    delay(5000);
    
    setGlobalState(BORWSING_DIR);
    refreshScreen();
}

int main()
{
    initPlayer();

    while (1)
    {
        scanKeys();
        delay(100);
    }
}