#include "display.h"
#include "main.h"
#include "ssd1306/ssd1306.h"

// Pinos I2C
#define I2C_SDA 14
#define I2C_SCL 15

void display_init() {
    // Usa a instância i2c1 do Pico (pinos 14 e 15)
    ssd1306_init(i2c1, I2C_SDA, I2C_SCL);
    ssd1306_clear();
    ssd1306_draw_string(0, 0, 1.0, "Monitoramento"); // Título alterado
    ssd1306_draw_string(0, 24, 1, "Aguardando comando...");
    ssd1306_update();
}

// <<< FUNÇÃO MODIFICADA PARA MELHOR LAYOUT >>>
void display_update(const char* msg1, const char* msg2) {
    ssd1306_clear();
    // Título Fixo
    ssd1306_draw_string(0, 0, 1.0, "Monitoramento");
    // Linha divisória
    ssd1306_draw_string(0, 16, 1, "--------------------");
    // Mensagem mais recente (maior)
    ssd1306_draw_string(2, 28, 1.4, msg1); 
    // Mensagem anterior (menor)
    ssd1306_draw_string(2, 48, 1.4, msg2); 
    ssd1306_update();
}