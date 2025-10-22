#ifndef SSD1306_H
#define SSD1306_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define SSD1306_ADDR_8B (0x3C << 1)

typedef enum { black = 0x00, white = 0x01 } ssd1306_color;

typedef struct {

    I2C_HandleTypeDef *hi2c;
    uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
    uint16_t cursor_x;
    uint16_t cursor_y;
    bool initialized;
} ssd1306_t;

extern ssd1306_t ssd1306;

HAL_StatusTypeDef ssd1306_Init(I2C_HandleTypeDef *hi2c);
void ssd1306_Fill(ssd1306_color color);
void ssd1306_UpdateScreen(void);
void ssd1306_SetCursor(uint16_t x, uint16_t y);
void ssd1306_DrawPixel(uint16_t x, uint16_t y, ssd1306_color color);
void ssd1306_WriteChar(char ch);
void ssd1306_WriteString(const char *str);

#endif