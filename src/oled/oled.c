#include "oled.h"

#include <string.h>
#include "fonts.h"
#include "ascii_font.h"

#define OLED_SCL_GPIOX      GPIOD
#define OLED_SCL_PIN        GPIO_PIN_4

#define OLED_SDA_GPIOX      GPIOD
#define OLED_SDA_PIN        GPIO_PIN_5

#define OLED_W_SCL(x)		HAL_GPIO_WritePin(OLED_SCL_GPIOX, OLED_SCL_PIN, (GPIO_PinState)x)
#define OLED_W_SDA(x)		HAL_GPIO_WritePin(OLED_SDA_GPIOX, OLED_SDA_PIN, (GPIO_PinState)x)

static uint8_t frameBuffer[8][128];

/*引脚初始化*/
void OLED_I2C_Init(void)
{
    __HAL_RCC_GPIOD_CLK_ENABLE();
	
	GPIO_InitTypeDef oledInit = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Speed = GPIO_SPEED_FREQ_HIGH
    };

	oledInit.Pin = OLED_SCL_PIN;
 	HAL_GPIO_Init(OLED_SCL_GPIOX, &oledInit);
	oledInit.Pin = OLED_SDA_PIN;
 	HAL_GPIO_Init(OLED_SDA_GPIOX, &oledInit);
	
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  * @retval 无
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(Byte & (0x80 >> i));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x00);		//写命令
	OLED_I2C_SendByte(Command); 
	OLED_I2C_Stop();
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x40);		//写数据
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

/**
  * @brief  OLED清屏
  * @param  无
  * @retval 无
  */
void OLED_Clear(void)
{
	memset(frameBuffer, 0, 1024);
}

/**
  * @brief  OLED显示一个字符
  * @param  row 行位置，范围：0~3
  * @param  col 列位置，范围：0~15
  * @param  ch 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowChar(uint8_t row, uint8_t col, signed char ch)
{      	
	if(ch < 0) ch = '?';

	const uint16_t fontIndex = (ch - ' ') * 16;
	
	memcpy(frameBuffer[row * 2] + col * 8, OLED_F8x16 + fontIndex, 8);
	memcpy(frameBuffer[row * 2 + 1] + col * 8, OLED_F8x16 + fontIndex + 8, 8);
}

/**
  * @brief  OLED显示字符串
  * @param  row 起始行位置，范围：1~4
  * @param  col 起始列位置，范围：1~16
  * @param  str 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowString(uint8_t row, uint8_t col, char* str)
{
	for (uint8_t i = 0; str[i] != '\0'; ++i) {
		OLED_ShowChar(row, col + i, str[i]);
	}
}

/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);
	}
	
	OLED_I2C_Init();			//端口初始化
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0x20);	//设置显存寻址模式，默认页寻址模式
	OLED_WriteCommand(0x00);	//使用水平寻址模式

	OLED_WriteCommand(0xAF);	//开启显示
		
	OLED_Clear();				//OLED清屏
}

void OLED_ShowGBK(uint8_t row, uint8_t col, uint8_t* font)
{
	memcpy(frameBuffer[row * 2] + col * 8, font, 16);
	memcpy(frameBuffer[row * 2 + 1] + col * 8, font + 16, 16);
}

void OLED_ShowGBKString(uint8_t row, uint8_t col, uint8_t padding, char* str, uint8_t* font)
{	
	uint8_t* wStr = (uint8_t*) str;
	uint8_t i = 0;
	
	for(; wStr[i] != '\0' && i < padding;) {
		// ASCII
		if(wStr[i] < 128) {
			OLED_ShowChar(row, col + i, wStr[i]);
			i += 1;
		}
		// GBK
		else if(font != NULL) {
			OLED_ShowGBK(row, col + i, font);
			font += 32;
			i += 2;
		}
		else {
			const uint16_t code = (wStr[i] << 8) | wStr[i + 1];
			OLED_ShowGBK(row, col + i, loadFont(code));
			i += 2;
		}
	}

	for(; i < padding; ++i) {
		OLED_ShowChar(row, col + i, ' ');
	}
}

void OLED_ShowPaddingString(uint8_t row, uint8_t col, char* str, uint8_t padding)
{
	uint8_t i;
	for(i = 0; str[i] != '\0' && i < padding; ++i) 
		OLED_ShowChar(row, col + i, str[i]);
	
	for(; i < padding; ++i)
		OLED_ShowChar(row, col + i, ' ');
}

void OLED_flushScreen()
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x40);		//首个控制字节，Co = 0，D/C# = 1，表明接下来所有传输内容均为数据字节，向显存写入

	for(uint8_t page = 0; page < 8; ++page) {
		for(uint8_t seg = 0; seg < 128; ++seg) {
			OLED_I2C_SendByte(frameBuffer[page][seg]);
		}
	}

	OLED_I2C_Stop();
}
