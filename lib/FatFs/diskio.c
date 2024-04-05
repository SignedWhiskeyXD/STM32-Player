/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/
#include "diskio.h"
#include "stm32f1xx_hal.h"

#define ATA 0

static HAL_SD_CardInfoTypeDef sdInfo;
static SD_HandleTypeDef       sdHandle;

void SD_GPIO_init()
{
    GPIO_InitTypeDef gpioinitstruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    gpioinitstruct.Mode  = GPIO_MODE_AF_PP;
    gpioinitstruct.Pull  = GPIO_PULLUP;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;

    gpioinitstruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;

    HAL_GPIO_Init(GPIOC, &gpioinitstruct);

    gpioinitstruct.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOD, &gpioinitstruct);
}

HAL_StatusTypeDef sdInit()
{
    SD_GPIO_init();
    __HAL_RCC_SDIO_CLK_ENABLE();

    sdHandle.Instance                 = SDIO;
    sdHandle.Init.ClockEdge           = SDIO_CLOCK_EDGE_RISING;
    sdHandle.Init.ClockBypass         = SDIO_CLOCK_BYPASS_DISABLE;
    sdHandle.Init.ClockPowerSave      = SDIO_CLOCK_POWER_SAVE_DISABLE;
    sdHandle.Init.BusWide             = SDIO_BUS_WIDE_1B;
    sdHandle.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    sdHandle.Init.ClockDiv            = SDIO_TRANSFER_CLK_DIV;

    return HAL_SD_Init(&sdHandle);
}

static void cardStatePolling(HAL_SD_CardStateTypeDef cardState)
{
    while (HAL_SD_GetCardState(&sdHandle) == cardState) {
        for(uint16_t i = 0; i < 1000; ++i);
    }
}

DSTATUS disk_status(BYTE pdrv)
{
    if (pdrv != ATA) return STA_NODISK;

    return HAL_SD_GetCardState(&sdHandle) == HAL_SD_CARD_ERROR ? STA_NOINIT : 0;
}

DSTATUS disk_initialize(BYTE pdrv)
{
    if (pdrv != ATA) return STA_NODISK;

    HAL_StatusTypeDef initStatus = sdInit();
    initStatus |= HAL_SD_GetCardInfo(&sdHandle, &sdInfo);

    return initStatus == HAL_OK ? 0 : STA_NOINIT;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
    if (pdrv != ATA) return RES_PARERR;

    const HAL_StatusTypeDef readStatus = HAL_SD_ReadBlocks(&sdHandle, buff, sector, count, 2000);
    return readStatus == HAL_OK ? RES_OK : RES_ERROR;
}

#if _USE_WRITE
DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
    if (pdrv != ATA) return RES_PARERR;

    const HAL_StatusTypeDef writeStatus = HAL_SD_WriteBlocks(&sdHandle, (uint8_t*) buff, sector, count, 2000);

    /* 确保上一次的写入操作已完成 */
    cardStatePolling(HAL_SD_CARD_PROGRAMMING);
    return writeStatus == HAL_OK ? RES_OK : RES_ERROR;
}
#endif

#ifdef _USE_IOCTL
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
    if (pdrv != ATA) return RES_PARERR;

    switch (cmd) {
        case GET_SECTOR_SIZE:
            *(WORD*) buff = sdInfo.BlockSize;
            break;
        case GET_BLOCK_SIZE:
            *(DWORD*) buff = 1;
            break;
        case GET_SECTOR_COUNT:
            *(DWORD*) buff = sdInfo.BlockNbr;
            break;
        default:
            return RES_ERROR;
    }
    return RES_OK;
}
#endif

DWORD get_fattime()
{
    return ((DWORD) (2024 - 1980) << 25) /* Year 2024 */
           | ((DWORD) 5 << 21)           /* Month 5 */
           | ((DWORD) 24 << 16)          /* Mday 24 */
           | ((DWORD) 11 << 11)          /* Hour 11 */
           | ((DWORD) 45 << 5)           /* Min 45 */
           | ((DWORD) 14 >> 1);          /* Sec 14 */
}
