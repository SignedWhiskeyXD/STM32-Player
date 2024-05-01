//
// Created by wsmrxd on 2024/4/5.
//

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"

static TIM_HandleTypeDef htim1;

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    RCC_ClkInitTypeDef clkconfig;
    uint32_t           pFLatency;

    /* Enable TIM1 clock */
    __HAL_RCC_TIM1_CLK_ENABLE();

    /* Get clock configuration */
    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

    /* Compute TIM1 clock */
    uint32_t uwTimclock = HAL_RCC_GetPCLK2Freq();

    /* Compute the prescaler value to have TIM1 counter clock equal to 1MHz */
    uint32_t uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000U) - 1U);

    /* Initialize TIM1 */
    htim1.Instance = TIM1;

    /* Initialize TIMx peripheral as follows:

    + Period = [(TIM1CLK/1000) - 1]. to have a (1/1000) s time base.
    + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
    + ClockDivision = 0
    + Counter direction = Up
    */
    htim1.Init.Period            = (1000000U / 1000U) - 1U;
    htim1.Init.Prescaler         = uwPrescalerValue;
    htim1.Init.ClockDivision     = 0;
    htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_StatusTypeDef status = HAL_TIM_Base_Init(&htim1);
    if (status != HAL_OK) return status;

    /* Start the TIM time Base generation in interrupt mode */
    status = HAL_TIM_Base_Start_IT(&htim1);
    if (status != HAL_OK) return status;

    /* Enable the TIM1 global Interrupt */
    HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);

    /* Configure the SysTick IRQ priority */
    if (TickPriority < (1UL << __NVIC_PRIO_BITS)) {
        /* Configure the TIM IRQ priority */
        HAL_NVIC_SetPriority(TIM1_UP_IRQn, TickPriority, 0U);
        uwTickPrio = TickPriority;
    } else {
        status = HAL_ERROR;
    }

    /* Return function status */
    return status;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1) {
        HAL_IncTick();
    }
}

void TIM1_UP_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
}