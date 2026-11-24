#ifndef __OLED_H
#define __OLED_H

#include "main.h"

/* 0.96 寸 SSD1306 OLED I2C 8位写地址 */
#define OLED_ADDRESS  0x78

void OLED_Clear(void);
void OLED_Init(void);
void OLED_ShowChar(uint8_t x, uint8_t y, char chr, uint8_t size);
void OLED_ShowString(uint8_t x, uint8_t y, const char *chr, uint8_t size);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);

#endif /* __OLED_H */
