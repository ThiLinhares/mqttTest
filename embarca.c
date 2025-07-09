#include "main.h"

void pinos_start()
{
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

    // Inicializa o Display OLED
    display_init();
}

// Função para ligar/desligar os LEDs
void led_control(uint gpio_pin, bool on) {
    gpio_put(gpio_pin, on);
}