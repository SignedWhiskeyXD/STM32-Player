//
// Created by wsmrxd on 2024/3/18.
//

#include "lcd.h"

#define LCD_CS_HIGH() HAL_GPIO_WritePin(GPIOD, LCD_CS_Pin, GPIO_PIN_SET)
#define LCD_CS_LOW()  HAL_GPIO_WritePin(GPIOD, LCD_CS_Pin, GPIO_PIN_RESET)

#define LCD_SELECT_DATA()   HAL_GPIO_WritePin(GPIOD, LCD_DCX_Pin, GPIO_PIN_SET)
#define LCD_SELECT_COMMAND()  HAL_GPIO_WritePin(GPIOD, LCD_DCX_Pin, GPIO_PIN_RESET)

lv_display_t* lcd_disp;

static SPI_HandleTypeDef lcdHandle;
static DMA_HandleTypeDef lcdRxHandle;
static DMA_HandleTypeDef lcdTxHandle;

static volatile uint8_t lcdBusy = 0;

HAL_StatusTypeDef lcdDisplayOn()
{
    LCD_CS_LOW();
    LCD_SELECT_COMMAND();

    uint8_t command = 0x29;
    const HAL_StatusTypeDef ret = HAL_SPI_Transmit(&lcdHandle, &command, 1, 5000);

    LCD_CS_HIGH();
    return ret;
}

void lcdDMAInit()
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    lcdRxHandle.Instance = DMA1_Channel2;
    lcdRxHandle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    lcdRxHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    lcdRxHandle.Init.MemInc = DMA_MINC_ENABLE;
    lcdRxHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    lcdRxHandle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    lcdRxHandle.Init.Mode = DMA_NORMAL;
    lcdRxHandle.Init.Priority = DMA_PRIORITY_MEDIUM;
    HAL_DMA_Init(&lcdRxHandle);

    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, LCD_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);

    __HAL_LINKDMA(&lcdHandle, hdmarx, lcdRxHandle);

    lcdTxHandle.Instance = DMA1_Channel3;
    lcdTxHandle.Init.Direction = DMA_MEMORY_TO_PERIPH;
    lcdTxHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    lcdTxHandle.Init.MemInc = DMA_MINC_ENABLE;
    lcdTxHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    lcdTxHandle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    lcdTxHandle.Init.Mode = DMA_NORMAL;
    lcdTxHandle.Init.Priority = DMA_PRIORITY_MEDIUM;
    HAL_DMA_Init(&lcdTxHandle);

    HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, LCD_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

    __HAL_LINKDMA(&lcdHandle, hdmatx, lcdTxHandle);
}

HAL_StatusTypeDef lcdInit()
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();

    GPIO_InitTypeDef gpioInit = {
        .Pin   = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,
        .Speed = GPIO_SPEED_FREQ_HIGH,
        .Mode  = GPIO_MODE_AF_PP,
    };
    HAL_GPIO_Init(GPIOA, &gpioInit);

    gpioInit.Pin  = LCD_RESET_Pin | LCD_DCX_Pin | LCD_CS_Pin;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOD, &gpioInit);

    SPI_InitTypeDef vsSpiInit = {
        .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2,
        .Direction         = SPI_DIRECTION_2LINES,
        .CLKPhase          = SPI_PHASE_1EDGE,
        .CLKPolarity       = SPI_POLARITY_LOW,
        .CRCCalculation    = SPI_CRCCALCULATION_DISABLE,
        .CRCPolynomial     = 7,
        .DataSize          = SPI_DATASIZE_8BIT,
        .FirstBit          = SPI_FIRSTBIT_MSB,
        .NSS               = SPI_NSS_SOFT,
        .TIMode            = SPI_TIMODE_DISABLE,
        .Mode              = SPI_MODE_MASTER,
    };
    lcdHandle.Instance = SPI1;
    lcdHandle.Init     = vsSpiInit;

    HAL_GPIO_WritePin(GPIOD, LCD_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOD, LCD_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(100);

    HAL_GPIO_WritePin(GPIOD, LCD_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, LCD_DCX_Pin, GPIO_PIN_SET);

    HAL_NVIC_SetPriority(SPI1_IRQn, LCD_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);

    HAL_SPI_Init(&lcdHandle);
    lcdDMAInit();
    return lcdDisplayOn();
}

void lcd_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size)
{
    while (lcdBusy);

    lcdBusy = 1;

    lcdHandle.Init.DataSize = SPI_DATASIZE_8BIT;
    HAL_SPI_Init(&lcdHandle);

    LCD_SELECT_COMMAND();
    LCD_CS_LOW();

    if(HAL_SPI_Transmit(&lcdHandle, cmd, cmd_size, 5000) == HAL_OK) {
        LCD_SELECT_DATA();
        HAL_SPI_Transmit(&lcdHandle, param, param_size, 5000);
        LCD_CS_HIGH();
    }
    lcdBusy = 0;
}

void lcd_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size)
{
    while (lcdBusy);

    lcdBusy = 1;

    lcdHandle.Init.DataSize = SPI_DATASIZE_8BIT;
    HAL_SPI_Init(&lcdHandle);

    LCD_SELECT_COMMAND();
    LCD_CS_LOW();

    if(HAL_SPI_Transmit(&lcdHandle, cmd, cmd_size, 5000) == HAL_OK) {
        LCD_SELECT_DATA();

        lcdHandle.Init.DataSize = SPI_DATASIZE_16BIT;
        HAL_SPI_Init(&lcdHandle);

        HAL_SPI_Transmit_DMA(&lcdHandle, param, param_size / 2);
    }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    LCD_CS_HIGH();
    lcdBusy = 0;
    lv_display_flush_ready(lcd_disp);
}

void DMA1_Channel2_IRQHandler()
{
    HAL_DMA_IRQHandler(&lcdRxHandle);
}

void DMA1_Channel3_IRQHandler()
{
    HAL_DMA_IRQHandler(&lcdTxHandle);
}

void SPI1_IRQHandler()
{
    HAL_SPI_IRQHandler(&lcdHandle);
}
