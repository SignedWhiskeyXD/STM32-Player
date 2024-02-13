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
#include "states/states.h"
#include "button.h"
#include "display.h"
#include "vs1053/VS1053.h"
#include "rtos/FreeRTOS.h"
#include "rtos/task.h"

TaskHandle_t taskCreationHandler;
TaskHandle_t taskKeyScanHandler;
TaskHandle_t taskScreenHandler;

void initPlayer()
{
    initScreen();
    initKeys();

    MyError error = initSD();
    if(error != OPERATION_SUCCESS){
        setLastError(error);
        setGlobalState(PLAYER_ERROR);
        return;
    }
    loadFiles();
    
    VS_Init();
    VS_HD_Reset();
	VS_Soft_Reset();
    
    setGlobalState(BROWSING_MENU);
}

void taskKeyScan()
{
    while(1)
    {
        scanKeys();
        vTaskDelay(10);
    }
}

void taskScreenRefresh()
{
    while(1)
    {
        onScreenRefresh();
        vTaskDelay(33);
    }
}

void taskCreation()
{
    taskENTER_CRITICAL();

    xTaskCreate(taskKeyScan, "TaskKeyScan", 512, NULL, 2, &taskKeyScanHandler);
    xTaskCreate(taskScreenRefresh, "TaskScreen", 512, NULL, 3, &taskScreenHandler);

    vTaskDelete(taskCreationHandler);

    taskEXIT_CRITICAL();
}

int main()
{
    initPlayer();

    xTaskCreate(taskCreation, "TaskCreation", 512, NULL, 2, &taskCreationHandler);

    vTaskStartScheduler();

    while (1);
}