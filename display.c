#include "display.h"
#include "main.h"
#include "ssd1306/ssd1306.h"

// --- Definições dos Pinos I2C ---
// Define os pinos SDA e SCL para a comunicação com o display.
#define I2C_SDA 14
#define I2C_SCL 15

/**
 * @brief Inicializa o display OLED.
 * Configura a comunicação I2C e exibe uma tela de boas-vindas.
 */
void display_init() {
    // Configura o driver do display para usar a instância i2c1 do Pico.
    ssd1306_init(i2c1, I2C_SDA, I2C_SCL);
    // Limpa qualquer conteúdo residual no buffer do display.
    ssd1306_clear();
    // Escreve as mensagens iniciais no buffer.
    ssd1306_draw_string(0, 0, 1.0, "Monitoramento");
    ssd1306_draw_string(0, 24, 1, "Aguardando comando...");
    // Envia o conteúdo do buffer para a tela.
    ssd1306_update();
}

/**
 * @brief Atualiza o display com duas linhas de informação.
 * A primeira mensagem (msg1) é exibida com destaque.
 *
 * @param msg1 A primeira string a ser exibida (em fonte maior).
 * @param msg2 A segunda string a ser exibida (em fonte menor).
 */
void display_update(const char* msg1, const char* msg2) {
    // Limpa o buffer antes de desenhar o novo conteúdo.
    ssd1306_clear();

    // --- Layout da Tela ---
    // 1. Título Fixo no topo.
    ssd1306_draw_string(0, 0, 1.0, "Monitoramento");
    // 2. Linha divisória para separar o título do conteúdo.
    ssd1306_draw_string(0, 16, 1, "--------------------");
    // 3. Primeira mensagem (principal), com fonte maior para destaque.
    ssd1306_draw_string(2, 28, 1.4, msg1);
    // 4. Segunda mensagem (secundária), abaixo da primeira.
    ssd1306_draw_string(2, 48, 1.4, msg2);

    // Envia o buffer atualizado para a tela do display.
    ssd1306_update();
}