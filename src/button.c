#include "button.h"

#include "daemon_tasks.h"
#include "player.h"
#include "recorder.h"
#include "states/states.h"
#include "stm32f1xx_hal.h"

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

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    gpioDef.Mode  = GPIO_MODE_INPUT;
    gpioDef.Pull  = GPIO_PULLUP;
    gpioDef.Pin   = GPIO_PIN_4 | GPIO_PIN_8;
    gpioDef.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &gpioDef);

    gpioDef.Pin = GPIO_PIN_1 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOD, &gpioDef);

    gpioDef.Pin = GPIO_PIN_10 | GPIO_PIN_14;
    HAL_GPIO_Init(GPIOA, &gpioDef);

    for (uint8_t i = 0; i < (uint8_t) BUTTON_NUM; ++i) btnHistory[i] = 1;
}

void setKeyState(Button btn, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    const uint8_t flag = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
    btnFalling[btn]    = (flag < btnHistory[btn]);
    btnHistory[btn]    = flag;
}

void scanKeys()
{
    setKeyState(BUTTON_UP, GPIOB, GPIO_PIN_8);
    setKeyState(BUTTON_DOWN, GPIOB, GPIO_PIN_4);
    setKeyState(BUTTON_CONFIRM, GPIOD, GPIO_PIN_1);
    setKeyState(BUTTON_CANCEL, GPIOD, GPIO_PIN_5);
    setKeyState(BUTTON_LEFT, GPIOA, GPIO_PIN_10);
    setKeyState(BUTTON_RIGHT, GPIOA, GPIO_PIN_14);

    if (btnFalling[BUTTON_UP]) onButtonUpClicked();
    if (btnFalling[BUTTON_DOWN]) onButtonDownClicked();
    if (btnFalling[BUTTON_CONFIRM]) onButtonConfirmClicked();
    if (btnFalling[BUTTON_CANCEL]) onButtonCancelClicked();
    if (btnFalling[BUTTON_LEFT]) onButtonLeftClicked();
    if (btnFalling[BUTTON_RIGHT]) onButtonRightClicked();

    for (uint8_t i = 0; i < (uint8_t) BUTTON_NUM; ++i) {
        if (btnFalling[i]) {
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
            if (shouldReplay) playSelectedSong();
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