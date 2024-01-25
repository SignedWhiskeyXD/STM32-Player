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
#include "OLED.h"
#include "fileOps.h"
#include "status/button.h"
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

void timerInit()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    TIM_InternalClockConfig(TIM2);
    
    TIM_TimeBaseInitTypeDef timerDef;
    timerDef.TIM_RepetitionCounter = 0;
    timerDef.TIM_CounterMode = TIM_CounterMode_Up;
    timerDef.TIM_ClockDivision = TIM_CKD_DIV1;
    
    timerDef.TIM_Prescaler = 7200 - 1;
    timerDef.TIM_Period = 500 -1;
    TIM_TimeBaseInit(TIM2, &timerDef);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    NVIC_InitTypeDef nvicDef;
    nvicDef.NVIC_IRQChannel = TIM2_IRQn;
    nvicDef.NVIC_IRQChannelCmd = ENABLE;
    nvicDef.NVIC_IRQChannelPreemptionPriority = 2;
    nvicDef.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nvicDef);
    
    TIM_Cmd(TIM2, ENABLE); 
}

int main()
{
    GPIO_InitTypeDef gpioDef;
    RCC_APB2PeriphClockCmd(LED_PERIPH, ENABLE);
    gpioDef.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioDef.GPIO_Pin = LED_PIN;
    gpioDef.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(LED_PORT, &gpioDef);
    OLED_Init();
    timerInit();
    initKeys();
    
    initSD();
    loadFiles();
    refreshScreen();

    while (1)
    {
        scanKeys();
        delay(100);
    }
}