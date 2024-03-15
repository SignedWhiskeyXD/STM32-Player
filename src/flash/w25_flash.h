#ifndef W25_FLASH_H
#define W25_FLASH_H

#include "stm32f1xx_hal.h"

#define sFLASH_ID                   0XEF4017 // W25Q64

#define SPI_FLASH_PageSize          256
#define SPI_FLASH_PerWritePageSize  256

#define W25X_WriteEnable        0x06
#define W25X_WriteDisable       0x04
#define W25X_ReadStatusReg      0x05
#define W25X_WriteStatusReg     0x01
#define W25X_ReadData           0x03
#define W25X_FastReadData       0x0B
#define W25X_FastReadDual       0x3B
#define W25X_PageProgram        0x02
#define W25X_BlockErase         0xD8
#define W25X_SectorErase        0x20
#define W25X_ChipErase          0xC7
#define W25X_PowerDown          0xB9
#define W25X_ReleasePowerDown   0xAB
#define W25X_DeviceID           0xAB
#define W25X_ManufactDeviceID   0x90
#define W25X_JedecDeviceID      0x9F

#define WIP_Flag                0x01 /* Write In Progress (WIP) flag */
#define Dummy_Byte              0xFF

#define SPIx SPI1
#define SPIx_CLK_ENABLE()               __HAL_RCC_SPI1_CLK_ENABLE()
#define SPIx_SCK_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MISO_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MOSI_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_CS_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOC_CLK_ENABLE()

#define SPIx_FORCE_RESET()              __HAL_RCC_SPI1_FORCE_RESET()
#define SPIx_RELEASE_RESET()            __HAL_RCC_SPI1_RELEASE_RESET()

#define SPIx_SCK_PIN            GPIO_PIN_5
#define SPIx_SCK_GPIO_PORT      GPIOA

#define SPIx_MISO_PIN           GPIO_PIN_6
#define SPIx_MISO_GPIO_PORT     GPIOA

#define SPIx_MOSI_PIN           GPIO_PIN_7
#define SPIx_MOSI_GPIO_PORT     GPIOA

#define FLASH_CS_PIN            GPIO_PIN_0
#define FLASH_CS_GPIO_PORT      GPIOC

#define SPI_FLASH_CS_LOW()      HAL_GPIO_WritePin(FLASH_CS_GPIO_PORT, FLASH_CS_PIN, GPIO_PIN_RESET)
#define SPI_FLASH_CS_HIGH()     HAL_GPIO_WritePin(FLASH_CS_GPIO_PORT, FLASH_CS_PIN, GPIO_PIN_SET)

#define SPIT_FLAG_TIMEOUT ((uint32_t)0x1000)
#define SPIT_LONG_TIMEOUT ((uint32_t)(10 * SPIT_FLAG_TIMEOUT))

void SPI_FLASH_Init(void);
void SPI_FLASH_BufferRead(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
void flashAsyncBufferRead(uint8_t *buffer, uint32_t readAddr, uint16_t bufferSize);
uint32_t SPI_FLASH_ReadID(void);
uint32_t SPI_FLASH_ReadDeviceID(void);

SPI_HandleTypeDef* flashGetHandle();

#endif