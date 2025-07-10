#include "main.h"

/**
 * @brief Inicializa os pinos GPIO, o conversor analógico-digital (ADC) e o display.
 * Esta função deve ser chamada no início da execução do programa.
 */
void pinos_start()
{
    // --- Configuração dos LEDs RGB ---
    // Inicializa o controle dos pinos dos LEDs.
    gpio_init(LED_PIN_R);
    gpio_init(LED_PIN_G);
    gpio_init(LED_PIN_B);

    // Define os pinos dos LEDs como SAÍDA.
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);
    gpio_set_dir(LED_PIN_B, GPIO_OUT);

    // --- Configuração do ADC ---
    // Inicializa o hardware do ADC para leitura de sensores analógicos.
    adc_init();

    // --- Configuração do Pino de Monitoramento (Botão) ---
    // Inicializa o pino que será monitorado (ex: um botão).
    gpio_init(MONITOR_PIN);
    // Define o pino como ENTRADA.
    gpio_set_dir(MONITOR_PIN, GPIO_IN);
    // Habilita o resistor de pull-up interno para garantir um estado estável
    // quando o botão não está pressionado.
    gpio_pull_up(MONITOR_PIN);

    // --- Inicialização do Display ---
    // Chama a função que configura e prepara o display OLED para uso.
    display_init();
}

/**
 * @brief Controla o estado de um pino de LED (ligado ou desligado).
 *
 * @param gpio_pin O número do pino GPIO do LED a ser controlado.
 * @param on       `true` para ligar o LED, `false` para desligar.
 */
void led_control(uint gpio_pin, bool on) {
    gpio_put(gpio_pin, on);
}