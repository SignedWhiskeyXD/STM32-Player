#ifndef VS1053_H
#define VS1053_H

#include "stm32f1xx_hal.h"

#define VS_SPI            SPI2

#define VS_XCS            GPIO_PIN_12 /*定义VS1053的片选管脚*/
#define VS_SCLK           GPIO_PIN_13 /*定义VS1053的时钟管脚*/
#define VS_MISO           GPIO_PIN_14 /*定义VS1053的MISO管脚*/
#define VS_MOSI           GPIO_PIN_15 /*定义VS1053的MOSI管脚*/
#define VS_SPIGPIO_PORT   GPIOB       /* GPIO端口 */

#define VS_XDCS           GPIO_PIN_9  /*定义VS1053的片选管脚*/
#define VS_GPIO_XDCS_PORT GPIOB       /* GPIO端口 */

#define VS_GPIO_RST_PORT  GPIOC       /* GPIO端口 */
#define VS_RST            GPIO_PIN_7  /*定义VS1053的RST管脚*/

#define VS_GPIO_DREQ_PORT GPIOC       /* GPIO端口 */
#define VS_DREQ           GPIO_PIN_6  /*定义VS1053的DREQ管脚*/

#define VS_RAM_TEST_GOOD  0x83FF

#define VS_WRITE_COMMAND  0x02
#define VS_READ_COMMAND   0x03

// VS1053寄存器定义
#define SPI_MODE        0x00
#define SPI_STATUS      0x01
#define SPI_BASS        0x02
#define SPI_CLOCKF      0x03
#define SPI_DECODE_TIME 0x04
#define SPI_AUDATA      0x05
#define SPI_WRAM        0x06
#define SPI_WRAMADDR    0x07
#define SPI_HDAT0       0x08
#define SPI_HDAT1       0x09
#define SPI_AIADDR      0x0a
#define SPI_VOL         0x0b
#define SPI_AICTRL0     0x0c
#define SPI_AICTRL1     0x0d
#define SPI_AICTRL2     0x0e
#define SPI_AICTRL3     0x0f

#define SM_DIFF         0x01
#define SM_JUMP         0x02
#define SM_RESET        0x04
#define SM_OUTOFWAV     0x08
#define SM_PDOWN        0x10
#define SM_TESTS        0x20
#define SM_STREAM       0x40
#define SM_PLUSV        0x80
#define SM_DACT         0x100
#define SM_SDIORD       0x200
#define SM_SDISHARE     0x400
#define SM_SDINEW       0x800
#define SM_ADPCM        0x1000
#define SM_ADPCM_HP     0x2000

#define I2S_CONFIG      0XC040
#define GPIO_DDR        0XC017
#define GPIO_IDATA      0XC018
#define GPIO_ODATA      0XC019

typedef struct {
    uint8_t mvol;    // 主音量,范围:0~254
    uint8_t bflimit; // 低音频率限定,范围:2~15(单位:10Hz)
    uint8_t bass;    // 低音,范围:0~15.0表示关闭.(单位:1dB)
    uint8_t tflimit; // 高音频率限定,范围:1~15(单位:Khz)
    uint8_t treble;  // 高音,范围:0~15(单位:1.5dB)(原本范围是:-8~7,通过函数修改了);
    uint8_t effect;  // 空间效果设置.0,关闭;1,最小;2,中等;3,最大.
} VS_Settings;

typedef struct {
    uint8_t input;      // 输入通道选择.0：MICP，1：LINE1
    uint8_t sampleRate; // 采样速率选择（x*8K）：1：8K，2：16K，3：24K...
    uint8_t channel;    // 声道：1：双声道，2：左声道，3：右声道
    uint8_t agc;        // 增益：1~64
} RecordSetting;

uint16_t VS_RD_Reg(uint8_t address);                // 读寄存器
void     VS_WR_Cmd(uint8_t address, uint16_t data); // 写命令
void     VS_HD_Reset();                             // 硬复位
void     VS_Soft_Reset();                           // 软复位

void     VS_Sine_Test(uint16_t duration_ms);        // 正弦测试
uint16_t VS_Ram_Test();                             // RAM测试

void VS_Init();                                     // 初始化VS10XX

void VS_SPI_SpeedLow();
void VS_SPI_SpeedHigh();

uint16_t VS_Get_HeadInfo();               // 得到比特率
uint16_t VS_Get_ByteRate();               // 得到字节速率
uint8_t  VS_Send_MusicData(uint8_t* buf); // 向VS10XX发送32字节
void     VS_Restart_Play();               // 重新开始下一首歌播放
uint8_t  VS_MusicJump();
void     VS_Reset_DecodeTime();           // 重设解码时间
uint16_t VS_Get_DecodeTime();             // 得到解码时间

void VS_Set_Vol(uint8_t vol);             // 设置主音量
void VS_Set_Bass(uint8_t bfreq, uint8_t bass, uint8_t tfreq,
                 uint8_t treble);         // 设置高低音
void VS_Set_Effect(uint8_t eft);          // 设置音效
void VS_Set_All();

void VS_Load_Patch(uint16_t *patch,uint16_t len);	//加载用户patch

void VS_StartRecord(RecordSetting* recset);

#endif