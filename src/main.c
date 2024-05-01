#include "button.h"
#include "daemon_tasks.h"
#include "display.h"
#include "flash/w25_flash.h"
#include "states/states.h"
#include "stm32f1xx_hal.h"
#include "vs1053/vs1053.h"

static void SystemClock_Config()
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {.OscillatorType      = RCC_OSCILLATORTYPE_HSI,
                                            .HSIState            = RCC_HSI_ON,
                                            .HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT,
                                            .PLL.PLLState        = RCC_PLL_ON,
                                            .PLL.PLLSource       = RCC_PLLSOURCE_HSI_DIV2,
                                            .PLL.PLLMUL          = RCC_PLL_MUL16};
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitTypeDef RCC_ClkInitStruct = {.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                                         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
                                            .SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK,
                                            .AHBCLKDivider  = RCC_SYSCLK_DIV1,
                                            .APB1CLKDivider = RCC_HCLK_DIV1,
                                            .APB2CLKDivider = RCC_HCLK_DIV1};
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

void initPlayer()
{
    SystemClock_Config();
    HAL_Init();

    initScreen();
    initKeys();
    SPI_FLASH_Init();

    MyError error = initSD();
    if (error != OPERATION_SUCCESS) {
        setLastError(error);
        setGlobalState(PLAYER_ERROR);
        return;
    }
    loadFiles();

    VS_Init();

    if (VS_Ram_Test() != VS_RAM_TEST_GOOD) {
        setLastError(VS_RAM_TEST_FAILED);
        setGlobalState(PLAYER_ERROR);
        return;
    }

    VS_HD_Reset();
    VS_Soft_Reset();

    setGlobalState(BROWSING_MENU);
}

int main()
{
    initPlayer();

    launchDaemonTasks();

    while (1) {}
}
