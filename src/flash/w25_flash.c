#include "w25_flash.h"

static SPI_HandleTypeDef spiHandle;

uint8_t SPI_FLASH_ReadByte()
{
    static uint8_t readByte;

    HAL_SPI_Receive(&spiHandle, &readByte, 1, 5000);

    return readByte;
}

void SPI_FLASH_SendByte(uint8_t byte)
{
    HAL_SPI_Transmit(&spiHandle, &byte, 1, 5000);
}

void FLASH_GPIO_Init()
{
    SPIx_SCK_GPIO_CLK_ENABLE();
    SPIx_MISO_GPIO_CLK_ENABLE();
    SPIx_MOSI_GPIO_CLK_ENABLE();
    SPIx_CS_GPIO_CLK_ENABLE();

    SPIx_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {
        .Pin = SPIx_SCK_PIN | SPIx_MISO_PIN | SPIx_MOSI_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_HIGH,
    };
    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = FLASH_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(FLASH_CS_GPIO_PORT, &GPIO_InitStruct);
}

void SPI_FLASH_Init(void)
{
    FLASH_GPIO_Init();

    const SPI_InitTypeDef spiInit = {
        .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4,
        .Direction = SPI_DIRECTION_2LINES,
        .CLKPhase = SPI_PHASE_2EDGE,
        .CLKPolarity = SPI_POLARITY_HIGH,
        .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
        .CRCPolynomial = 7,
        .DataSize = SPI_DATASIZE_8BIT,
        .FirstBit = SPI_FIRSTBIT_MSB,
        .NSS = SPI_NSS_SOFT,
        .TIMode = SPI_TIMODE_DISABLE,
        .Mode = SPI_MODE_MASTER,
    };

    spiHandle.Instance = SPIx;
    spiHandle.Init = spiInit;

    HAL_SPI_Init(&spiHandle);
    __HAL_SPI_ENABLE(&spiHandle);
}

void SPI_FLASH_BufferRead(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    SPI_FLASH_CS_LOW();

    SPI_FLASH_SendByte(W25X_ReadData);

    SPI_FLASH_SendByte(ReadAddr >> 16);
    SPI_FLASH_SendByte(ReadAddr >> 8);
    SPI_FLASH_SendByte(ReadAddr);

    HAL_SPI_Receive(&spiHandle, pBuffer, NumByteToRead, 5000);

    SPI_FLASH_CS_HIGH();
}

uint32_t SPI_FLASH_ReadID(void)
{
    SPI_FLASH_CS_LOW();

    SPI_FLASH_SendByte(W25X_JedecDeviceID);

    uint8_t idBuffer[3];

    HAL_SPI_Receive(&spiHandle, idBuffer, sizeof(idBuffer), 5000);

    SPI_FLASH_CS_HIGH();

    return (idBuffer[0] << 16) | (idBuffer[1] << 8) | idBuffer[2];
}

uint32_t SPI_FLASH_ReadDeviceID(void)
{
    SPI_FLASH_CS_LOW();

    SPI_FLASH_SendByte(W25X_DeviceID);
    SPI_FLASH_SendByte(Dummy_Byte);
    SPI_FLASH_SendByte(Dummy_Byte);
    SPI_FLASH_SendByte(Dummy_Byte);

    const uint8_t devId = SPI_FLASH_ReadByte();

    SPI_FLASH_CS_HIGH();

    return devId;
}
