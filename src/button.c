#include "button.h"

#include "states/states.h"
#include "player.h"
#include "daemon_tasks.h"
#include "recorder.h"
#include "stm32f10x.h"

uint8_t btnHistory[BUTTON_NUM];

uint8_t btnFalling[BUTTON_NUM];

void onButtonUpClicked();

void onButtonDownClicked();

void onButtonConfirmClicked();

void onButtonCancelClicked();

void onButtonLeftClicked();

void onButtonRightClicked();

void initKeys()
{
    GPIO_InitTypeDef gpioDef;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpioDef.GPIO_Mode = GPIO_Mode_IPU;
    gpioDef.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    gpioDef.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &gpioDef);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gpioDef.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_11;
    GPIO_Init(GPIOA, &gpioDef);

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
    setKeyState(BUTTON_CONFIRM, GPIOB, GPIO_Pin_5);
    setKeyState(BUTTON_CANCEL, GPIOA, GPIO_Pin_11);
    setKeyState(BUTTON_LEFT, GPIOA, GPIO_Pin_3);
    setKeyState(BUTTON_RIGHT, GPIOA, GPIO_Pin_2);

    if(btnFalling[BUTTON_UP])
        onButtonUpClicked();
    if(btnFalling[BUTTON_DOWN])
        onButtonDownClicked();
    if(btnFalling[BUTTON_CONFIRM])
        onButtonConfirmClicked();
    if(btnFalling[BUTTON_CANCEL])
        onButtonCancelClicked();
    if(btnFalling[BUTTON_LEFT])
        onButtonLeftClicked();
    if(btnFalling[BUTTON_RIGHT])
        onButtonRightClicked();
    
    for(uint8_t i = 0; i < BUTTON_NUM; ++i) {
        if(btnFalling[i]) {
            notifyScreenRefresh();
            return;
        }
    }
}

void onButtonUpClicked()
{
    switch (getGlobalState()) {
        case BROWSING_MENU: {
            moveMenuPointer(1);
            break;
        }
        case BROWSING_DIR: {
            moveFilePointer(1);
            break;
        }
        default:
            break;
    }
}

void onButtonDownClicked()
{
    switch (getGlobalState()) {
        case BROWSING_MENU: {
            moveMenuPointer(-1);
            break;
        }
        case BROWSING_DIR: {
            moveFilePointer(-1);
            break;
        }
        default:
            break;
    }
}

void onButtonConfirmClicked()
{
    switch (getGlobalState()) {
        case BROWSING_MENU: {
            setGlobalStateFromMenu();
            break;
        }
        case BROWSING_DIR: {
            const uint8_t shouldReplay = pauseOrResumeSelectedSong();
            if (shouldReplay)
                playSelectedSong();
            break;
        }
        case RECORDING: {
            toggleRecord();
            break;
        }
        default:
            break;
    }
}

void onButtonCancelClicked()
{
    switch (getGlobalState()) {
        case BROWSING_DIR: {
            setGlobalState(BROWSING_MENU);
            break;
        }
        case RECORDING: {
            stopRecorder();
            setGlobalState(BROWSING_MENU);
            break;
        }
        default:
            break;
    }
}

void onButtonLeftClicked()
{
    switch (getGlobalState()) {
        case BROWSING_DIR: {
            setJumpFlag(-1);
            break;
        }
        default:
            break;
    }
}

void onButtonRightClicked()
{
    switch (getGlobalState()) {
        case BROWSING_DIR: {
            setJumpFlag(1);
            break;
        }
        default:
            break;
    }
}