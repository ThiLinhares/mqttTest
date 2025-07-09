#pragma once

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Screen resolution
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

void ssd1306_init(i2c_inst_t *i2c, uint8_t sda, uint8_t scl);
void ssd1306_clear();
void ssd1306_draw_string(int x, int y, float scale, const char *s);
void ssd1306_update();