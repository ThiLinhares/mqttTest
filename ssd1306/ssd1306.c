#include "ssd1306.h"
#include "ssd1306/font.h"
#include <string.h>

// --- Variáveis Estáticas do Módulo ---
static i2c_inst_t *i2c_instance;      // Instância I2C utilizada (i2c0 ou i2c1).
static uint8_t i2c_address = 0x3c;    // Endereço padrão do display SSD1306.
// Buffer de memória que representa o conteúdo da tela (128x64 pixels).
static uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

/**
 * @brief Envia um byte de comando para o display via I2C.
 */
void ssd1306_command(uint8_t cmd) {
    // O primeiro byte (0x00) indica que o byte seguinte é um comando.
    uint8_t buf[2] = {0x00, cmd};
    i2c_write_blocking(i2c_instance, i2c_address, buf, 2, false);
}

/**
 * @brief Inicializa o hardware I2C e envia a sequência de comandos de configuração para o display.
 */
void ssd1306_init(i2c_inst_t *i2c, uint8_t sda, uint8_t scl) {
    i2c_instance = i2c;
    // Inicializa a interface I2C com uma velocidade de 400 kHz.
    // Esta velocidade é um bom equilíbrio entre performance e estabilidade.
    i2c_init(i2c_instance, 400 * 1000);

    // Configura os pinos SDA e SCL para a função I2C.
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    // Habilita os resistores de pull-up internos para a estabilidade do barramento.
    gpio_pull_up(sda);
    gpio_pull_up(scl);

    // --- Sequência de Comandos de Inicialização do SSD1306 ---
    // Esta sequência configura o display para o modo de operação desejado.
    ssd1306_command(0xAE); // Desliga o display
    ssd1306_command(0xD5); ssd1306_command(0x80); // Define o clock do display
    ssd1306_command(0xA8); ssd1306_command(63);   // Define a multiplexação (altura do display - 1)
    ssd1306_command(0xD3); ssd1306_command(0x0);  // Sem offset no display
    ssd1306_command(0x40); // Define a linha inicial do display (0)
    ssd1306_command(0x8D); ssd1306_command(0x14); // Habilita a bomba de carga
    ssd1306_command(0x20); ssd1306_command(0x00); // Modo de endereçamento de memória horizontal
    ssd1306_command(0xA1); // Remapeamento de segmento (colunas 127-0)
    ssd1306_command(0xC8); // Direção de varredura COM (de cima para baixo)
    ssd1306_command(0xDA); ssd1306_command(0x12); // Configuração dos pinos COM
    ssd1306_command(0x81); ssd1306_command(0xCF); // Define o contraste
    ssd1306_command(0xD9); ssd1306_command(0xF1); // Define o período de pré-carga
    ssd1306_command(0xDB); ssd1306_command(0x40); // Define o nível de VCOMH
    ssd1306_command(0xA4); // O conteúdo da RAM segue o que está no buffer
    ssd1306_command(0xA6); // Display normal (não invertido)
    ssd1306_command(0xAF); // Liga o display

    // Limpa a tela e atualiza após a inicialização.
    ssd1306_clear();
    ssd1306_update();
}

/**
 * @brief Limpa o buffer da tela, preenchendo-o com zeros (pixels apagados).
 */
void ssd1306_clear() {
    memset(buffer, 0, sizeof(buffer));
}

/**
 * @brief Envia o conteúdo completo do buffer de memória para o display via I2C.
 */
void ssd1306_update() {
    // Define a janela de escrita para cobrir a tela inteira.
    ssd1306_command(0x21); ssd1306_command(0); ssd1306_command(127); // Colunas
    ssd1306_command(0x22); ssd1306_command(0); ssd1306_command(7);   // Páginas

    // Prepara um buffer para a transferência I2C. O primeiro byte (0x40)
    // indica que os bytes seguintes são dados para a RAM do display.
    uint8_t buf[sizeof(buffer) + 1];
    buf[0] = 0x40;
    memcpy(buf + 1, buffer, sizeof(buffer));

    // Escreve o buffer de uma só vez para o display.
    i2c_write_blocking(i2c_instance, i2c_address, buf, sizeof(buf), false);
}

/**
 * @brief Desenha um único pixel no buffer nas coordenadas (x, y).
 */
void ssd1306_draw_pixel(int x, int y, bool on) {
    // Ignora coordenadas fora dos limites da tela.
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) return;

    // Calcula a posição do byte e do bit no buffer que corresponde ao pixel.
    int byte_index = (y / 8) * SSD1306_WIDTH + x;
    int bit_index = y % 8;

    if (on) {
        buffer[byte_index] |= (1 << bit_index);  // Liga o bit.
    } else {
        buffer[byte_index] &= ~(1 << bit_index); // Desliga o bit.
    }
}

/**
 * @brief Desenha um caractere no buffer usando uma fonte bitmap.
 */
void ssd1306_draw_char(int x, int y, float scale, char c) {
    // Ignora caracteres não imprimíveis.
    if (c < 32 || c > 127) return;

    // Itera sobre a matriz de 8x8 pixels do caractere na fonte.
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            // Verifica se o pixel do caractere na fonte está aceso.
            if ((font[c-32][i] >> j) & 1) {
                // Desenha um bloco de pixels escalonado, se necessário.
                for (int sx = 0; sx < scale; sx++) {
                    for (int sy = 0; sy < scale; sy++) {
                        ssd1306_draw_pixel(x + i*scale + sx, y + j*scale + sy, true);
                    }
                }
            }
        }
    }
}

/**
 * @brief Desenha uma string de caracteres no buffer.
 */
void ssd1306_draw_string(int x, int y, float scale, const char *s) {
    int current_x = x;
    // Itera sobre a string, desenhando um caractere de cada vez.
    while (*s) {
        ssd1306_draw_char(current_x, y, scale, *s++);
        // Avança a posição X para o próximo caractere.
        current_x += 8 * scale;
    }
}