#include "main.h"

static uint32_t last_debounce_time[3] = {0, 0, 0};
uint adc_x_raw;
uint adc_y_raw;
uint brightness = 0;
bool increasing = true; 


static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};

void pinos_start()
{
    gpio_init(LED_PIN_R);
    gpio_init(LED_PIN_B);
    gpio_init(LED_PIN_G);
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    gpio_set_function(LED_PIN_R, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN_G, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN_B, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(LED_PIN_R);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f);
    pwm_init(slice_num, &config, true);
    slice_num = pwm_gpio_to_slice_num(LED_PIN_B);
    pwm_init(slice_num, &config, true);
    slice_num = pwm_gpio_to_slice_num(LED_PIN_G);
    pwm_init(slice_num, &config, true);
    
    // Inicializa o pino de monitoramento
    gpio_init(MONITOR_PIN);
    gpio_set_dir(MONITOR_PIN, GPIO_IN);
    gpio_pull_up(MONITOR_PIN);

    // Removemos as interrupções que não serão utilizadas
}

// As outras funções permanecem, mas não serão chamadas no fluxo principal
void gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}

void gpio5_callback(uint gpio, uint32_t events) {
    // A lógica de callback pode ser mantida ou removida se não for necessária
}

void js()
{
    adc_select_input(0);
    adc_x_raw = adc_read();
    printf("Posicao eixo X: %u\n",adc_x_raw);
    adc_select_input(1);
    adc_y_raw = adc_read();
    printf("Posicao eixo Y: %u\n",adc_y_raw);
    sleep_ms(50);
}

void setup_pwm(uint gpio_pin) {
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio_pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f);
    pwm_init(slice_num, &config, true);
}

void update_pwm(uint gpio_pin) {
    pwm_set_gpio_level(LED_PIN_R, brightness);
    printf("brilho_led_vermelho: %u\n",brightness);
    if (increasing) 
    {
        brightness = brightness+400;
        if (brightness >= PWM_STEPS) 
        {
            increasing = false;
        }
    }
    else 
    {
        brightness = brightness-400;
        if (brightness == 0) {
            increasing = true;
        }
    }
}

void pwm_led(uint gpio_pin, uint brilho)
{
    if(gpio_pin == LED_PIN_B)
    {
        pwm_set_gpio_level(LED_PIN_B, brilho);
    }
    else if (gpio_pin == LED_PIN_G)
    {
        pwm_set_gpio_level(LED_PIN_G, brilho);
    }
}