#include "VS1053.h"

// VS1053默认设置参数
static VS_Settings vs1053_settings = {
    220, // 音量:220
    6,   // 低音上线 60Hz
    15,  // 低音提升 15dB
    10,  // 高音下限 10Khz
    15,  // 高音提升 10.5dB
    0,   // 空间效果
};

static SPI_HandleTypeDef vsSpiHandler;

#define Delay_ms(x)         HAL_Delay(x);

#define VS_XDCS_HIGH()      HAL_GPIO_WritePin(VS_GPIO_XDCS_PORT,VS_XDCS, GPIO_PIN_SET)
#define VS_XDCS_LOW()       HAL_GPIO_WritePin(VS_GPIO_XDCS_PORT,VS_XDCS, GPIO_PIN_RESET)

#define VS_XCS_HIGH()       HAL_GPIO_WritePin(VS_SPIGPIO_PORT,VS_XCS, GPIO_PIN_SET)
#define VS_XCS_LOW()        HAL_GPIO_WritePin(VS_SPIGPIO_PORT,VS_XCS, GPIO_PIN_RESET)

#define VS_RST_HIGH()       HAL_GPIO_WritePin(VS_GPIO_RST_PORT,VS_RST, GPIO_PIN_SET)
#define VS_RST_LOW()        HAL_GPIO_WritePin(VS_GPIO_RST_PORT,VS_RST, GPIO_PIN_RESET)

inline static void VS_SPI_SendByte(uint8_t data)
{
    HAL_SPI_Transmit(&vsSpiHandler, &data, 1, 5000);
}

inline static uint8_t VS_SPI_ReadByte()
{
    static uint8_t readByte;

    HAL_SPI_Receive(&vsSpiHandler, &readByte, 1, 5000);

    return readByte;
}

inline static void VS_SPI_SetSpeed(uint32_t prescaler)
{
    vsSpiHandler.Init.BaudRatePrescaler = prescaler;
    HAL_SPI_Init(&vsSpiHandler);
}

inline static void VS_SPI_SpeedLow()
{
    VS_SPI_SetSpeed(SPI_BAUDRATEPRESCALER_32);
}

inline static void VS_SPI_SpeedHigh()
{
    VS_SPI_SetSpeed(SPI_BAUDRATEPRESCALER_8);
}

inline static void VS_DREQ_Wait()
{
    while(HAL_GPIO_ReadPin(VS_GPIO_DREQ_PORT,VS_DREQ) == 0);
}

void VS_SPI_Init()
{
    __HAL_RCC_SPI2_CLK_ENABLE();

    GPIO_InitTypeDef gpioInit = {
        .Pin   = VS_SCLK | VS_MISO | VS_MOSI,
        .Speed = GPIO_SPEED_FREQ_HIGH,
        .Mode  = GPIO_MODE_AF_PP,
    };
    HAL_GPIO_Init(VS_SPIGPIO_PORT, &gpioInit);

    gpioInit.Pin = VS_XCS | VS_XDCS;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &gpioInit);

    SPI_InitTypeDef vsSpiInit = {
        .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32,
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
    vsSpiHandler.Instance = VS_SPI;
    vsSpiHandler.Init = vsSpiInit;

    HAL_SPI_Init(&vsSpiHandler);
}

// 初始化VS1053的IO口
void VS_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef gpioInit = {
        .Pin = VS_DREQ,
        .Mode = GPIO_MODE_INPUT,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_HIGH,
    };
    HAL_GPIO_Init(VS_GPIO_DREQ_PORT, &gpioInit);

    gpioInit.Pin = VS_RST;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(VS_GPIO_DREQ_PORT, &gpioInit);

    VS_SPI_Init();
}

// 软复位VS10XX
void VS_Soft_Reset(void)
{

}

void VS_HD_Reset(void)
{
    VS_RST_LOW();
    VS_XDCS_HIGH();
    VS_XCS_HIGH();
    VS_RST_HIGH();
}

void VS_Sine_Test(uint16_t duration_ms)
{
    static const uint8_t sineTestEnterCommands[4] = {0x53, 0xEF, 0x6E, 0x24};
    static const uint8_t sineTestExitCommands[4] = {0x45, 0x78, 0x69, 0x74};

    VS_HD_Reset();
    VS_WR_Cmd(0x0b, 0X2020);     // 设置音量
    VS_WR_Cmd(SPI_MODE, 0x0820); // 进入VS10XX的测试模式
    VS_DREQ_Wait();

    VS_SPI_SpeedLow();
    VS_XDCS_LOW();
    for(uint8_t i = 0; i < 8; ++i) {
        VS_SPI_SendByte(i < 4 ? sineTestEnterCommands[i] : 0x00);
    }

    Delay_ms(duration_ms);

    for(uint8_t i = 0; i < 8; ++i) {
        VS_SPI_SendByte(i < 4 ? sineTestExitCommands[i] : 0x00);
    }
    VS_XDCS_HIGH();
}

uint16_t VS_Ram_Test()
{
    static const uint8_t ramTestCommands[4] = {0x4D, 0xEA, 0x6D, 0x54};

    VS_HD_Reset();
    VS_WR_Cmd(SPI_MODE, 0x0820); // 进入VS10XX的测试模式
    VS_DREQ_Wait();
    VS_SPI_SpeedLow();

    VS_XDCS_LOW();
    for(uint8_t i = 0; i < 8; ++i) {
        VS_SPI_SendByte(i < 4 ? ramTestCommands[i] : 0x00);
    }
    VS_XDCS_HIGH();

    Delay_ms(150);
    return VS_RD_Reg(SPI_HDAT0);
}

void VS_WR_Cmd(uint8_t address, uint16_t data)
{
    VS_DREQ_Wait();
    VS_SPI_SpeedLow();
    VS_XDCS_HIGH();
    VS_XCS_LOW();
    VS_SPI_SendByte(VS_WRITE_COMMAND); // 发送VS10XX的写命令
    VS_SPI_SendByte(address);          // 地址
    VS_SPI_SendByte(data >> 8);        // 发送高八位
    VS_SPI_SendByte(data);             // 第八位
    VS_XCS_HIGH();
    VS_SPI_SpeedHigh();
}

uint16_t VS_RD_Reg(uint8_t address)
{
    uint16_t temp = 0;
    VS_DREQ_Wait();
    VS_SPI_SpeedLow();
    VS_XDCS_HIGH();
    VS_XCS_LOW();
    VS_SPI_SendByte(VS_READ_COMMAND); // 发送VS10XX的读命令
    VS_SPI_SendByte(address);         // 地址
    temp = VS_SPI_ReadByte();     // 读取高字节
    temp = temp << 8;
    temp += VS_SPI_ReadByte(); // 读取低字节
    VS_XCS_HIGH();
    VS_SPI_SpeedHigh();
    return temp;
}
