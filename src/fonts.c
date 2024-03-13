#include "fonts.h"

#include "flash/w25_flash.h"

static uint32_t getFontAddr(uint16_t code)
{
    // 高8位为区号，有效范围为 1 ~ 94
    const uint8_t section = (code >> 8) - 0xA0;

    // 低8位为区内偏移量，有效范围为 1 ~ 94
    const uint8_t offset = (code & 0x00FF) - 0xA1;

    return ((section - 1) * 94 + offset) * 32;
}

uint8_t* loadFont(uint16_t code)
{
    static uint8_t fontBuffer[32];

    const uint32_t addr = getFontAddr(code);
    SPI_FLASH_BufferRead(fontBuffer, addr, 32);
    return fontBuffer;
}