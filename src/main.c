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

#include "myError.h"
#include "stm32f10x.h"
#include "states/states.h"
#include "button.h"
#include "display.h"
#include "vs1053/VS1053.h"
#include "SysTick/bsp_SysTick.h"

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
    SysTick_Init();

    MYERROR error = initSD();
    if(error != OPERATION_SUCCESS){
        setLastError(error);
        setGlobalState(PLAYER_ERROR);
        return;
    }
    loadFiles();
    
    VS_Init();
    VS_HD_Reset();
	VS_Soft_Reset();

    delay(5000);
    
    setGlobalState(BORWSING_DIR);
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