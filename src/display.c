#include "display.h"

#include "stm32f10x.h"
#include "states/states.h"
#include "oled/OLED.h"

uint8_t shouldRefresh = 1;

void initScreen()
{
    OLED_Init();

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

void refreshScreen()
{
    shouldRefresh = 1;
}

void showStartUp()
{
    OLED_Clear();
    OLED_ShowString(1, 1, "Starting");
}

void showDirectoryBrowsing()
{
    File_State* fileState = useFileState();

    OLED_Clear();
    for(uint8_t i = 0; i < 4; ++i){
        OLED_ShowChar(i + 1, 1, i == fileState->offset ? '>' : ' ');

        OLED_ShowString(i + 1, 2, fileState->filenames[fileState->filenameBase + i]);
    }
}

void TIM2_IRQHandler()
{
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != SET) return;

    if(shouldRefresh == 0) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        return;
    }

    switch (getGlobalState()) {
        case PLAYER_START_UP:
            showStartUp();
            break;

        case BORWSING_DIR:
            showDirectoryBrowsing();
            break;
    
        default:
            break;
    }

    shouldRefresh = 0;
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}