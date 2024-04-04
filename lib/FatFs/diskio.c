/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/
#include "diskio.h"
#include "stm32f1xx_hal.h"

#define ATA          0

static HAL_SD_CardInfoTypeDef   sdInfo;
static SD_HandleTypeDef         sdHandle;

void SD_GPIO_init()
{
    GPIO_InitTypeDef gpioinitstruct = {0};

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* Common GPIO configuration */
    gpioinitstruct.Mode      = GPIO_MODE_AF_PP;
    gpioinitstruct.Pull      = GPIO_PULLUP;
    gpioinitstruct.Speed     = GPIO_SPEED_FREQ_HIGH;

    /* GPIOC configuration */
    gpioinitstruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;

    HAL_GPIO_Init(GPIOC, &gpioinitstruct);

    /* GPIOD configuration */
    gpioinitstruct.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOD, &gpioinitstruct);
}

HAL_StatusTypeDef sdInit()
{
    SD_GPIO_init();
    __HAL_RCC_SDIO_CLK_ENABLE();

    sdHandle.Instance = SDIO;
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
        HAL_Delay(2);
    }
}

DSTATUS disk_status(
    BYTE pdrv
)
{
    DSTATUS status = STA_NOINIT;

    switch (pdrv) {
        case ATA: /* SD CARD */
            status = (HAL_SD_GetCardState(&sdHandle) == HAL_SD_CARD_ERROR) ? STA_NOINIT : 0;
            break;

        default:
            status = STA_NODISK;
            break;
    }
    return status;
}

DSTATUS disk_initialize(
    BYTE pdrv
)
{
    DSTATUS status = STA_NOINIT;

    switch (pdrv) {
        case ATA: /* SD CARD */ {
            HAL_StatusTypeDef initStatus = sdInit();
            initStatus |= HAL_SD_GetCardInfo(&sdHandle, &sdInfo);

            status = initStatus == HAL_OK ? 0 : STA_NOINIT;
            break;
        }
        default:
            status = STA_NODISK;
            break;
    }
    return status;
}

DRESULT disk_read(
    BYTE pdrv,
    BYTE *buff,
    DWORD sector,
    UINT count
)
{
    DRESULT status    = RES_PARERR;

    switch (pdrv) {
        case ATA: /* SD CARD */{
            const HAL_StatusTypeDef readStatus = HAL_SD_ReadBlocks(&sdHandle, buff, sector, count, 2000);
            status = (readStatus == HAL_OK) ? RES_OK : RES_ERROR;
            break;
        }
        default:
            break;
    }
    return status;
}

#if _USE_WRITE
DRESULT disk_write(
    BYTE pdrv,        /* 设备物理编号(0..) */
    const BYTE *buff, /* 欲写入数据的缓存区 */
    DWORD sector,     /* 扇区首地址 */
    UINT count        /* 扇区个数(1..128) */
)
{
    DRESULT status    = RES_PARERR;

    switch (pdrv) {
        case ATA: {
            const HAL_StatusTypeDef writeStatus = HAL_SD_WriteBlocks(&sdHandle, (uint8_t*) buff, sector, count, 2000);
            /* 确保上一次的写入操作已完成 */
            cardStatePolling(HAL_SD_CARD_PROGRAMMING);
            status = (writeStatus == HAL_OK) ? RES_OK : RES_ERROR;
            break;
        }
        default:
            break;
    }

    return status;
}
#endif


#if _USE_IOCTL
DRESULT disk_ioctl(
    BYTE pdrv,
    BYTE cmd,  /* 控制指令 */
    void *buff /* 写入或者读取数据地址指针 */
)
{
    DRESULT status = RES_OK;
    switch (pdrv) {
        case ATA: /* SD CARD */
            switch (cmd) {
                // Get R/W sector size (WORD)
                case GET_SECTOR_SIZE:
                    *(WORD *)buff = sdInfo.BlockSize;
                    break;
                // Get erase block size in unit of sector (DWORD)
                case GET_BLOCK_SIZE:
                    *(DWORD *)buff = 1;
                    break;

                case GET_SECTOR_COUNT:
                    *(DWORD *)buff = sdInfo.BlockNbr;
                    break;

                default:
                    status = RES_ERROR;
                    break;
            }
            status = RES_OK;
            break;

        default:
            status = RES_PARERR;
            break;
    }
    return status;
}
#endif

DWORD get_fattime()
{
    return ((DWORD)(2015 - 1980) << 25) /* Year 2015 */
           | ((DWORD)1 << 21)           /* Month 1 */
           | ((DWORD)1 << 16)           /* Mday 1 */
           | ((DWORD)0 << 11)           /* Hour 0 */
           | ((DWORD)0 << 5)            /* Min 0 */
           | ((DWORD)0 >> 1);           /* Sec 0 */
}
