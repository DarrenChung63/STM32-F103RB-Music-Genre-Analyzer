#ifndef SH1106_H
#define SH1106_H

#include "stm32f1xx_hal.h"
#include "fonts.h"

void I2C1_Init(void);
void SH1106_Init(void);
void SH1106_UpdateScreen(void);
void SH1106_UpdateScreenPartial(uint8_t start_page, uint8_t end_page);
void SH1106_Clear(void);
void SH1106_Puts(uint8_t x, uint8_t y, char* str, FontDef_t* Font, uint8_t color);
void SH1106_DrawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);
#endif
