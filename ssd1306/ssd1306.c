#include "ssd1306.h"
#include "ssd1306/font.h" 
#include <string.h>

static i2c_inst_t *i2c_instance;
static uint8_t i2c_address = 0x3c;
static uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

void ssd1306_command(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    i2c_write_blocking(i2c_instance, i2c_address, buf, 2, false);
}

void ssd1306_init(i2c_inst_t *i2c, uint8_t sda, uint8_t scl) {
    i2c_instance = i2c;
    // <<< CORREÇÃO: VELOCIDADE I2C REDUZIDA >>>
    // Alterado de 400 * 1000 para 100 * 1000 para maior estabilidade.
    i2c_init(i2c_instance, 400 * 1000); 
    
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);


    ssd1306_command(0xAE); // display off
    ssd1306_command(0xD5); ssd1306_command(0x80); // set display clock div
    ssd1306_command(0xA8); ssd1306_command(63);   // set multiplex
    ssd1306_command(0xD3); ssd1306_command(0x0);  // set display offset
    ssd1306_command(0x40); // set start line
    ssd1306_command(0x8D); ssd1306_command(0x14); // charge pump
    ssd1306_command(0x20); ssd1306_command(0x00); // memory mode
    ssd1306_command(0xA1); // seg remap
    ssd1306_command(0xC8); // com output scan direction
    ssd1306_command(0xDA); ssd1306_command(0x12); // com pins
    ssd1306_command(0x81); ssd1306_command(0xCF); // contrast
    ssd1306_command(0xD9); ssd1306_command(0xF1); // precharge
    ssd1306_command(0xDB); ssd1306_command(0x40); // vcom detect
    ssd1306_command(0xA4); // resume
    ssd1306_command(0xA6); // normal display
    ssd1306_command(0xAF); // display on

    ssd1306_clear();
    ssd1306_update();
}

void ssd1306_clear() {
    memset(buffer, 0, sizeof(buffer));
}

void ssd1306_update() {
    ssd1306_command(0x21); ssd1306_command(0); ssd1306_command(127);
    ssd1306_command(0x22); ssd1306_command(0); ssd1306_command(7);
    uint8_t buf[sizeof(buffer) + 1];
    buf[0] = 0x40;
    memcpy(buf + 1, buffer, sizeof(buffer));
    i2c_write_blocking(i2c_instance, i2c_address, buf, sizeof(buf), false);
}

void ssd1306_draw_pixel(int x, int y, bool on) {
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) return;
    int byte_index = (y / 8) * SSD1306_WIDTH + x;
    int bit_index = y % 8;
    if (on) {
        buffer[byte_index] |= (1 << bit_index);
    } else {
        buffer[byte_index] &= ~(1 << bit_index);
    }
}

void ssd1306_draw_char(int x, int y, float scale, char c) {
    if (c < 32 || c > 127) return;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if ((font[c-32][i] >> j) & 1) {
                for (int sx = 0; sx < scale; sx++) {
                    for (int sy = 0; sy < scale; sy++) {
                        ssd1306_draw_pixel(x + i*scale + sx, y + j*scale + sy, true);
                    }
                }
            }
        }
    }
}

void ssd1306_draw_string(int x, int y, float scale, const char *s) {
    int current_x = x;
    while (*s) {
        ssd1306_draw_char(current_x, y, scale, *s++);
        current_x += 8 * scale;
    }
}