#include "button.h"

#include "states/fileOps.h"
#include "states/states.h"
#include "display.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

uint8_t btnHistory[BUTTON_NUM];

uint8_t btnFalling[BUTTON_NUM];

uint8_t counter = 0;

void onButtonUpClicked();

void onButtonDownClicked();

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

void setKeyState(Button btn, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    const uint8_t flag = GPIO_ReadInputDataBit(GPIOx, GPIO_Pin);
    btnFalling[btn] = (flag < btnHistory[btn]);
    btnHistory[btn] = flag;
}

void scanKeys()
{
    setKeyState(BUTTON_UP, GPIOB, GPIO_Pin_4);
    setKeyState(BUTTON_DOWN, GPIOB, GPIO_Pin_6);

    if(btnFalling[BUTTON_UP])
        onButtonUpClicked();
    if(btnFalling[BUTTON_DOWN])
        onButtonDownClicked();
}

void onButtonUpClicked()
{
    File_State* fileState = useFileState();

    refreshScreen();

    if(counter < 3) counter++;
    else if(fileState->filenameBase + 4 < MAX_FILE_LIST_LENGTH && 
            fileState->filenames[fileState->filenameBase + 4][0] != '\0')
    {
        fileState->filenameBase++;
    }
}

void onButtonDownClicked()
{
    File_State* fileState = useFileState();

    refreshScreen();

    if(counter > 0) counter--;
    else if(fileState->filenameBase > 0){
        fileState->filenameBase--;
    }
}