#include "VS1053.h"

#include <string.h>

// VS1053默认设置参数
VS_Settings vs1053_settings = {
    220, // 音量:220
    6,   // 低音上线 60Hz
    15,  // 低音提升 15dB
    10,  // 高音下限 10Khz
    15,  // 高音提升 10.5dB
    0,   // 空间效果
};

// FOR WAV HEAD0 :0X7761 HEAD1:0X7665
// FOR MIDI HEAD0 :other info HEAD1:0X4D54
// FOR WMA HEAD0 :data speed HEAD1:0X574D
// FOR MP3 HEAD0 :data speed HEAD1:ID
// 比特率预定值,阶层III
static const uint16_t bitrate[2][16] = {
    {0,  8, 16, 24, 32, 40, 48, 56,  64,  80,  96, 112, 128, 144, 160, 0},
    {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0}
};

// VS1053的WAV录音有bug,这个plugin可以修正这个问题
static const uint16_t wav_plugin[40] =                                  /* Compressed plugin */
    {
        0x0007, 0x0001, 0x8010, 0x0006, 0x001c, 0x3e12, 0xb817, 0x3e14, /* 0 */
        0xf812, 0x3e01, 0xb811, 0x0007, 0x9717, 0x0020, 0xffd2, 0x0030, /* 8 */
        0x11d1, 0x3111, 0x8024, 0x3704, 0xc024, 0x3b81, 0x8024, 0x3101, /* 10 */
        0x8024, 0x3b81, 0x8024, 0x3f04, 0xc024, 0x2808, 0x4800, 0x36f1, /* 18 */
        0x9811, 0x0007, 0x0001, 0x8028, 0x0006, 0x0002, 0x2a00, 0x040e,
};

static SPI_HandleTypeDef vsSpiHandler;

#define Delay_ms(x)    HAL_Delay(x)

#define VS_XDCS_HIGH() HAL_GPIO_WritePin(VS_GPIO_XDCS_PORT, VS_XDCS, GPIO_PIN_SET)
#define VS_XDCS_LOW()  HAL_GPIO_WritePin(VS_GPIO_XDCS_PORT, VS_XDCS, GPIO_PIN_RESET)

#define VS_XCS_HIGH()  HAL_GPIO_WritePin(VS_SPIGPIO_PORT, VS_XCS, GPIO_PIN_SET)
#define VS_XCS_LOW()   HAL_GPIO_WritePin(VS_SPIGPIO_PORT, VS_XCS, GPIO_PIN_RESET)

#define VS_RST_HIGH()  HAL_GPIO_WritePin(VS_GPIO_RST_PORT, VS_RST, GPIO_PIN_SET)
#define VS_RST_LOW()   HAL_GPIO_WritePin(VS_GPIO_RST_PORT, VS_RST, GPIO_PIN_RESET)

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

inline static void VS_DREQ_Wait()
{
    while (HAL_GPIO_ReadPin(VS_GPIO_DREQ_PORT, VS_DREQ) == 0)
        ;
}

static uint16_t VS_WRAM_Read(uint16_t addr)
{
    VS_WR_Cmd(SPI_WRAMADDR, addr);
    return VS_RD_Reg(SPI_WRAM);
}

static uint16_t VS_Get_EndFillByte()
{
    return VS_WRAM_Read(0X1E06); // 填充字节
}

void VS_SPI_SpeedLow()
{
    VS_SPI_SetSpeed(SPI_BAUDRATEPRESCALER_32);
}

void VS_SPI_SpeedHigh()
{
    VS_SPI_SetSpeed(SPI_BAUDRATEPRESCALER_8);
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

    gpioInit.Pin  = VS_XCS | VS_XDCS;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &gpioInit);

    SPI_InitTypeDef vsSpiInit = {
        .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32,
        .Direction         = SPI_DIRECTION_2LINES,
        .CLKPhase          = SPI_PHASE_2EDGE,
        .CLKPolarity       = SPI_POLARITY_HIGH,
        .CRCCalculation    = SPI_CRCCALCULATION_DISABLE,
        .CRCPolynomial     = 7,
        .DataSize          = SPI_DATASIZE_8BIT,
        .FirstBit          = SPI_FIRSTBIT_MSB,
        .NSS               = SPI_NSS_SOFT,
        .TIMode            = SPI_TIMODE_DISABLE,
        .Mode              = SPI_MODE_MASTER,
    };
    vsSpiHandler.Instance = VS_SPI;
    vsSpiHandler.Init     = vsSpiInit;

    HAL_SPI_Init(&vsSpiHandler);
}

void VS_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef gpioInit = {
        .Pin   = VS_DREQ,
        .Mode  = GPIO_MODE_INPUT,
        .Pull  = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_HIGH,
    };
    HAL_GPIO_Init(VS_GPIO_DREQ_PORT, &gpioInit);

    gpioInit.Pin  = VS_RST;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(VS_GPIO_DREQ_PORT, &gpioInit);

    VS_SPI_Init();
    VS_SPI_SendByte(0xFF);
}

void VS_Soft_Reset()
{
    VS_DREQ_Wait();

    uint8_t retry = 0;
    while (VS_RD_Reg(SPI_MODE) != 0x0800) // 软件复位,新模式
    {
        VS_WR_Cmd(SPI_MODE, 0x0804);      // 软件复位,新模式
        Delay_ms(2);                      // 等待至少1.35ms
        if (retry++ > 100) break;
    }

    VS_DREQ_Wait();

    retry = 0;
    while (VS_RD_Reg(SPI_CLOCKF) != 0X9800) // 设置VS1053的时钟,3倍频 ,1.5xADD
    {
        VS_WR_Cmd(SPI_CLOCKF, 0X9800);      // 设置VS1053的时钟,3倍频 ,1.5xADD
        if (retry++ > 100) break;
    }
}

void VS_HD_Reset()
{
    VS_RST_LOW();
    VS_XDCS_HIGH();
    VS_XCS_HIGH();
    VS_RST_HIGH();
}

void VS_Sine_Test(uint16_t duration_ms)
{
    static const uint8_t sineTestEnterCommands[4] = {0x53, 0xEF, 0x6E, 0x24};
    static const uint8_t sineTestExitCommands[4]  = {0x45, 0x78, 0x69, 0x74};

    VS_HD_Reset();
    VS_WR_Cmd(0x0b, 0X2020);     // 设置音量
    VS_WR_Cmd(SPI_MODE, 0x0820); // 进入VS10XX的测试模式
    VS_DREQ_Wait();

    VS_SPI_SpeedLow();
    VS_XDCS_LOW();
    for (uint8_t i = 0; i < 8; ++i) {
        VS_SPI_SendByte(i < 4 ? sineTestEnterCommands[i] : 0x00);
    }

    Delay_ms(duration_ms);

    for (uint8_t i = 0; i < 8; ++i) {
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
    for (uint8_t i = 0; i < 8; ++i) {
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
    VS_DREQ_Wait();
    VS_SPI_SpeedLow();
    VS_XDCS_HIGH();
    VS_XCS_LOW();

    VS_SPI_SendByte(VS_READ_COMMAND);      // 发送VS10XX的读命令
    VS_SPI_SendByte(address);              // 地址
    uint16_t regValue = VS_SPI_ReadByte(); // 读取高字节

    regValue = regValue << 8;
    regValue += VS_SPI_ReadByte(); // 读取低字节

    VS_XCS_HIGH();
    VS_SPI_SpeedHigh();
    return regValue;
}

// 返回码率的大小
// 返回值：音频流的比特速度 bps
uint16_t VS_Get_HeadInfo()
{
    uint16_t head0 = VS_RD_Reg(SPI_HDAT0);
    uint16_t head1 = VS_RD_Reg(SPI_HDAT1);
    switch (head1) {
        case 0x7665: // WAV格式
        case 0X4D54: // MIDI格式
        case 0X4154: // AAC_ADTS
        case 0X4144: // AAC_ADIF
        case 0X4D34: // AAC_MP4/M4A
        case 0X4F67: // OGG
        case 0X574D: // WMA格式
        case 0X664C: // FLAC格式
        {
            return head0 * 8;
        }
        default: // MP3格式,仅做了阶层III的表
        {
            head1 >>= 3;
            head1 = head1 & 0x03;
            if (head1 == 3)
                head1 = 1;
            else
                head1 = 0;
            return bitrate[head1][head0 >> 12];
        }
    }
}

uint16_t VS_Get_ByteRate()
{
    return VS_WRAM_Read(0X1E05); // 平均位速
}

// 发送一次音频数据
// 固定为32字节
// 返回值:0,发送成功
//		 1,VS10xx不缺数据,本次数据未成功发送
uint8_t VS_Send_MusicData(uint8_t* buf)
{
    if (HAL_GPIO_ReadPin(VS_GPIO_DREQ_PORT, VS_DREQ) == 0) return 1;

    VS_XDCS_LOW();
    HAL_SPI_Transmit(&vsSpiHandler, buf, 32, 2000);
    VS_XDCS_HIGH();

    return 0;
}

void VS_Restart_Play()
{
    uint16_t temp;
    uint16_t i;
    uint8_t  vsbuf[32];
    memset(vsbuf, 0, 32);

    temp = VS_RD_Reg(SPI_MODE); // 读取SPI_MODE的内容
    temp |= 1 << 3;             // 设置SM_CANCEL位
    temp |= 1 << 2;             // 设置SM_LAYER12位,允许播放MP1,MP2
    VS_WR_Cmd(SPI_MODE, temp);  // 设置取消当前解码指令
    for (i = 0; i < 2048;) // 发送2048个0,期间读取SM_CANCEL位.如果为0,则表示已经取消了当前解码
    {
        if (VS_Send_MusicData(vsbuf) != 0) continue; // 每发送32个字节后检测一次

        i += 32;                                     // 发送了32个字节
        temp = VS_RD_Reg(SPI_MODE);                  // 读取SPI_MODE的内容
        if ((temp & (1 << 3)) == 0) break;           // 成功取消了
    }
    if (i < 2048)                                    // SM_CANCEL正常
    {
        memset(vsbuf, (uint8_t) VS_Get_EndFillByte(), 32);
        for (i = 0; i < 2052;) {
            if (VS_Send_MusicData(vsbuf) == 0) {
                i += 32;
            }
        }
    } else {
        VS_Soft_Reset(); // SM_CANCEL不成功,坏情况,需要软复位
    }
    temp = VS_RD_Reg(SPI_HDAT0);
    temp += VS_RD_Reg(SPI_HDAT1);
    if (temp)            // 软复位,还是没有成功取消,硬复位
    {
        VS_HD_Reset();   // 硬复位
        VS_Soft_Reset(); // 软复位
    }
}

uint8_t VS_MusicJump()
{
    static uint8_t endFillBuffer[32];

    const uint16_t sciStatus = VS_RD_Reg(SPI_STATUS);

    // 若SCI_NO_JUMP位被设置，则不能快进快倒
    if (sciStatus >= 0x8000) return 1;

    memset(endFillBuffer, (uint8_t) VS_Get_EndFillByte(), 32);

    for (uint16_t i = 0; i < 2048;) {
        if (VS_Send_MusicData(endFillBuffer) == 0) {
            i += 32;
        }
    }

    return 0;
}

void VS_Reset_DecodeTime()
{
    VS_WR_Cmd(SPI_DECODE_TIME, 0x0000);
}

uint16_t VS_Get_DecodeTime()
{
    return VS_RD_Reg(SPI_DECODE_TIME);
}

// 设定VS10XX播放的音量和高低音
// vol:音量大小(0~254)
void VS_Set_Vol(uint8_t vol)
{
    uint16_t volt = 254 - vol; // 取反一下,得到最大值,表示最大的表示
    volt <<= 8;
    volt += 254 - vol;         // 得到音量设置后大小
    VS_WR_Cmd(SPI_VOL, volt);  // 设音量
}

// 设定高低音控制
// bfreq:低频上限频率	2~15(单位:10Hz)
// bass:低频增益			0~15(单位:1dB)
// tfreq:高频下限频率 	1~15(单位:Khz)
// treble:高频增益  	 	0~15(单位:1.5dB,小于9的时候为负数)
void VS_Set_Bass(uint8_t bfreq, uint8_t bass, uint8_t tfreq, uint8_t treble)
{
    uint16_t    bass_set = 0; // 暂存音调寄存器值
    signed char temp     = 0;
    if (treble == 0)
        temp = 0; // 变换
    else if (treble > 8)
        temp = treble - 8;
    else
        temp = treble - 9;
    bass_set = temp & 0X0F;        // 高音设定
    bass_set <<= 4;
    bass_set += tfreq & 0xf;       // 高音下限频率
    bass_set <<= 4;
    bass_set += bass & 0xf;        // 低音设定
    bass_set <<= 4;
    bass_set += bfreq & 0xf;       // 低音上限
    VS_WR_Cmd(SPI_BASS, bass_set); // BASS
}

// 设定音效
// eft:0,关闭;1,最小;2,中等;3,最大.
void VS_Set_Effect(uint8_t eft)
{
    uint16_t temp = VS_RD_Reg(SPI_MODE); // 读取SPI_MODE的内容
    if (eft & 0X01)
        temp |= 1 << 4;                  // 设定LO
    else
        temp &= ~(1 << 5);               // 取消LO
    if (eft & 0X02)
        temp |= 1 << 7;                  // 设定HO
    else
        temp &= ~(1 << 7);               // 取消HO
    VS_WR_Cmd(SPI_MODE, temp);           // 设定模式
}

// 设置音量,音效等.
void VS_Set_All()
{
    VS_Set_Vol(vs1053_settings.mvol);      // 设置音量
    VS_Set_Bass(vs1053_settings.bflimit, vs1053_settings.bass, vs1053_settings.tflimit, vs1053_settings.treble);
    VS_Set_Effect(vs1053_settings.effect); // 设置空间效果
}

void VS_Load_Patch(uint16_t* patch, uint16_t len)
{
    uint16_t i;
    uint16_t addr, n, val;
    for (i = 0; i < len;) {
        addr = patch[i++];
        n    = patch[i++];
        if (n & 0x8000U) // RLE run, replicate n samples
        {
            n &= 0x7FFF;
            val = patch[i++];
            while (n--) VS_WR_Cmd(addr, val);
        } else // copy run, copy n sample
        {
            while (n--) {
                val = patch[i++];
                VS_WR_Cmd(addr, val);
            }
        }
    }
}

// 激活PCM 录音模式
// agc:0,自动增益.1024相当于1倍,512相当于0.5倍,最大值65535=64倍
void VS_StartRecord(RecordSetting* recset)
{
    // 如果是IMA ADPCM,采样率计算公式如下:
    // 采样率=CLKI/256*d;
    // 假设d=0,并2倍频,外部晶振为12.288M.那么Fc=(2*12288000)/256*6=16Khz
    // 如果是线性PCM,采样率直接就写采样值
    VS_WR_Cmd(SPI_BASS, 0x0000);
    VS_WR_Cmd(SPI_AICTRL0, recset->sampleRate * 8000); // 设置采样率
    VS_WR_Cmd(SPI_AICTRL1, recset->agc * 1024 / 2); // 设置增益,0,自动增益.1024相当于1倍,512相当于0.5倍,最大值65535=64倍
    VS_WR_Cmd(SPI_AICTRL2, 0);                      // 设置增益最大值,0,代表最大值65536=64X
    VS_WR_Cmd(SPI_AICTRL3, 6 + recset->channel); // 4：线性PCM模式 + 2： 左通道 3： 右通道
    VS_WR_Cmd(SPI_CLOCKF, 0X2000);               // 设置VS10XX的时钟,MULT:2倍频;ADD:不允许;CLK:12.288Mhz
    VS_WR_Cmd(SPI_MODE, 0x1804 | (recset->input << 14)); // MIC,录音激活
    Delay_ms(5);                                         // 等待至少1.35ms
    VS_Load_Patch((uint16_t*) wav_plugin, 40);           // VS1053的WAV录音需要patch
}
