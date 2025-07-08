#include "main.h"

// Funções não utilizadas foram removidas para clareza,
// mas a lógica principal está na inicialização dos pinos e no controle do LED.

void pinos_start()
{
    // <<< CORREÇÃO: INICIALIZAÇÃO COMO SAÍDA DIGITAL >>>
    // Inicializa os pinos dos LEDs
    gpio_init(LED_PIN_R);
    gpio_init(LED_PIN_G);
    gpio_init(LED_PIN_B);

    // Define a direção deles como SAÍDA (GPIO_OUT)
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);
    gpio_set_dir(LED_PIN_B, GPIO_OUT);

    // Inicializa o ADC (para o sensor de temperatura)
    adc_init();
    
    // Inicializa o pino de monitoramento como entrada
    gpio_init(MONITOR_PIN);
    gpio_set_dir(MONITOR_PIN, GPIO_IN);
    gpio_pull_up(MONITOR_PIN);
}

// <<< CORREÇÃO: NOVA FUNÇÃO DE CONTROLE DIGITAL >>>
// Substituímos a função pwm_led por esta, muito mais simples.
// Ela recebe o pino e um booleano (true para ligar, false para desligar)
void led_control(uint gpio_pin, bool on) {
    gpio_put(gpio_pin, on);
}