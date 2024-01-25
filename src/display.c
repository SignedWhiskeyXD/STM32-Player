#include "display.h"

#include "stm32f10x.h"
#include "states/states.h"
#include "oled/OLED.h"

extern char filenames[MAX_FILE_LIST_LENGTH][12];

extern uint8_t filenameBase;

extern uint8_t counter;

uint8_t shouldRefresh = 0;

void refreshScreen()
{
    shouldRefresh = 1;
}

void TIM2_IRQHandler()
{
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != SET) return;

    if(shouldRefresh == 0) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        return;
    }

    OLED_Clear();
    for(uint8_t i = 0; i < 4; ++i){
        OLED_ShowChar(i + 1, 1, i == counter ? '>' : ' ');

        OLED_ShowString(i + 1, 2, filenames[filenameBase + i]);
    }
    
    shouldRefresh = 0;
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}