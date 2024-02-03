#include "button.h"

#include "states/states.h"
#include "vs1053/VS1053.h"
#include "display.h"
#include "player.h"
#include "stm32f10x.h"

uint8_t btnHistory[BUTTON_NUM];

uint8_t btnFalling[BUTTON_NUM];

void onButtonUpClicked();

void onButtonDownClicked();

void onButtonPlayClicked();

void initKeys()
{
    GPIO_InitTypeDef gpioDef;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpioDef.GPIO_Mode = GPIO_Mode_IPU;
    gpioDef.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    gpioDef.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &gpioDef);

    for(uint8_t i = 0; i < BUTTON_NUM; ++i)
        btnHistory[i] = 1;
}

void setKeyState(Button btn, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    const uint8_t flag = GPIO_ReadInputDataBit(GPIOx, GPIO_Pin);
    btnFalling[btn] = (flag < btnHistory[btn]);
    btnHistory[btn] = flag;
}

void scanKeys()
{
    setKeyState(BUTTON_UP, GPIOB, GPIO_Pin_6);
    setKeyState(BUTTON_DOWN, GPIOB, GPIO_Pin_7);
    setKeyState(BUTTON_PLAY, GPIOB, GPIO_Pin_5);

    if(btnFalling[BUTTON_UP])
        onButtonUpClicked();
    if(btnFalling[BUTTON_DOWN])
        onButtonDownClicked();
    if(btnFalling[BUTTON_PLAY])
        onButtonPlayClicked();
}

void onButtonUpClicked()
{
    switch (getGlobalState()) {
        case BROWSING_DIR:
            moveFilePointer(1);
            break;
        default:
            break;
    }
}

void onButtonDownClicked()
{
    switch (getGlobalState()) {
        case BROWSING_DIR:
            moveFilePointer(-1);
            break;
        default:
            break;
    }
}

void onButtonPlayClicked()
{
    switch (getGlobalState()) {
        case BROWSING_DIR:
            const uint8_t shouldReplay = pauseOrResumeSelectedSong();
            if(shouldReplay)
                playSelectedSong();
            break;
        default:
            break;
    }
}