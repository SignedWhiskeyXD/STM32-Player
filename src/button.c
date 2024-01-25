#include "button.h"

#include "states/states.h"
#include "display.h"
#include "stm32f10x.h"

uint8_t btnHistory[BUTTON_NUM];

uint8_t btnFalling[BUTTON_NUM];

uint8_t counter = 0;

extern char filenames[MAX_FILE_LIST_LENGTH][12];

extern uint8_t filenameBase;

void initKeys()
{
    GPIO_InitTypeDef gpioDef;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpioDef.GPIO_Mode = GPIO_Mode_IPU;
    gpioDef.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6;
    gpioDef.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &gpioDef);

    for(uint8_t i = 0; i < BUTTON_NUM; ++i)
        btnHistory[i] = 1;
}

void scanKeys()
{
    uint8_t flag_up = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4);
    btnFalling[BUTTON_UP] = (flag_up < btnHistory[BUTTON_UP]);
    btnHistory[BUTTON_UP] = flag_up;

    uint8_t flag_down = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6);
    btnFalling[BUTTON_DOWN] = (flag_down < btnHistory[BUTTON_DOWN]);
    btnHistory[BUTTON_DOWN] = flag_down;

    if(btnFalling[BUTTON_UP]){
        refreshScreen();
        if(counter < 3) counter++;
        else if(filenameBase + 4 < MAX_FILE_LIST_LENGTH && filenames[filenameBase + 4][0] != '\0'){
            filenameBase++;
        }
    }
    if(btnFalling[BUTTON_DOWN]){
        refreshScreen();
        if(counter > 0) counter--;
        else if(filenameBase > 0){
            filenameBase--;
        }
    }
}