#include "stm32f1xx_hal.h"
#include "oled/oled.h"
#include "flash/w25_flash.h"
#include "vs1053/vs1053.h"
#include "display.h"
#include "button.h"
#include "daemon_tasks.h"
#include "states/states.h"

static void SystemClock_Config()
{
    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitTypeDef RCC_OscInitStruct = {
        .OscillatorType = RCC_OSCILLATORTYPE_HSI,
        .HSIState = RCC_HSI_ON,
        .HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT,
        .PLL.PLLState = RCC_PLL_ON,
        .PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2,
        .PLL.PLLMUL = RCC_PLL_MUL16
    };
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {
        .ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
        .SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
        .AHBCLKDivider = RCC_SYSCLK_DIV1,
        .APB1CLKDivider = RCC_HCLK_DIV1,
        .APB2CLKDivider = RCC_HCLK_DIV1
    };
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

void initPlayer()
{
    HAL_Init();
    SystemClock_Config();

    initScreen();
    initKeys();
    SPI_FLASH_Init();

    MyError error = initSD();
    if(error != OPERATION_SUCCESS){
        setLastError(error);
        setGlobalState(PLAYER_ERROR);
        return;
    }
    loadFiles();
    
    VS_Init();

    if(VS_Ram_Test() != VS_RAM_TEST_GOOD) {
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
    
    while (1)
    {

    }
}
