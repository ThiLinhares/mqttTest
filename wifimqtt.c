#include "main.h"
#define PUB_DELAY_MS 1000
static uint32_t last_pub_time = 0;
err_t mqtt_test_connect(MQTT_CLIENT_T *state);

// Função para ler o sensor de temperatura interno e converter para Celsius
float read_onboard_temperature() {
    adc_select_input(4);
    uint16_t raw = adc_read();
    const float conversion_factor = 3.3f / (1 << 12);
    float voltage = raw * conversion_factor;
    float temp_celsius = 27.0f - (voltage - 0.706f) / 0.001721f;
    return temp_celsius;
}

void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    MQTT_CLIENT_T *state = (MQTT_CLIENT_T*)callback_arg;
    if (ipaddr) {
        state->remote_addr = *ipaddr;
        DEBUG_printf("DNS resolved: %s\n", ip4addr_ntoa(ipaddr));
    } else {
        DEBUG_printf("DNS resolution failed.\n");
    }
}

void run_dns_lookup(MQTT_CLIENT_T *state) {
    DEBUG_printf("Running DNS lookup for %s...\n", MQTT_SERVER_HOST);
    if (dns_gethostbyname(MQTT_SERVER_HOST, &(state->remote_addr), dns_found, state) == ERR_INPROGRESS) {
        while (state->remote_addr.addr == 0) {
            cyw43_arch_poll();
            sleep_ms(10);
        }
    }
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        pwm_led(LED_PIN_G, 2000);
        DEBUG_printf("MQTT connected.\n");
    } else {
        pwm_led(LED_PIN_G, 0);
        pwm_led(LED_PIN_R, 2000);
        DEBUG_printf("MQTT connection failed: %d\n", status);
    }
}

void mqtt_pub_request_cb(void *arg, err_t err) {
    // Callback de publicação
}

err_t mqtt_test_publish(MQTT_CLIENT_T *state) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_pub_time >= PUB_DELAY_MS)
    {
        last_pub_time = now;
        char buffer[BUFFER_SIZE];

        // 1. Publicar o estado ON/OFF do pino
        snprintf(buffer, BUFFER_SIZE, "%s", monitor_pin_on ? "ON" : "OFF");
        mqtt_publish(state->mqtt_client, "pico_w/pin_status", buffer, strlen(buffer), 0, 0, mqtt_pub_request_cb, state);

        // 2. Publicar a temperatura
        float temperature = read_onboard_temperature();
        snprintf(buffer, BUFFER_SIZE, "%.2f", temperature);
        mqtt_publish(state->mqtt_client, "pico_w/temperature", buffer, strlen(buffer), 0, 0, mqtt_pub_request_cb, state);
        
        DEBUG_printf("Pin State: %s, Temp: %.2f C\n", monitor_pin_on ? "ON" : "OFF", temperature);

        return ERR_OK;
    }
    return ERR_OK;
}

err_t mqtt_test_connect(MQTT_CLIENT_T *state) {
    struct mqtt_connect_client_info_t ci = {0};
    ci.client_id = "PicoW-Thiago";
    return mqtt_client_connect(state->mqtt_client, &(state->remote_addr), MQTT_SERVER_PORT, mqtt_connection_cb, state, &ci);
}

// Função para verificar o botão e atualizar o estado
void check_button_press() {
    static uint32_t last_press_time = 0;
    static bool last_pin_state = true; // Inicia como HIGH (não pressionado)

    bool current_pin_state = gpio_get(MONITOR_PIN);

    // Verifica a borda de descida (de HIGH para LOW)
    if (current_pin_state == false && last_pin_state == true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        // Aplica o debounce
        if (now - last_press_time > DEBOUNCE_DELAY_MS) {
            last_press_time = now;
            // Inverte o estado
            monitor_pin_on = !monitor_pin_on;
            DEBUG_printf("Button pressed! New state: %s\n", monitor_pin_on ? "ON" : "OFF");
        }
    }
    last_pin_state = current_pin_state;
}


void mqtt_run_test(MQTT_CLIENT_T *state) {
    state->mqtt_client = mqtt_client_new();
    if (!state->mqtt_client) {
        DEBUG_printf("Failed to create MQTT client\n");
        return;
    }

    if (mqtt_test_connect(state) == ERR_OK) {
        while (1) {
            cyw43_arch_poll();

            if (mqtt_client_is_connected(state->mqtt_client)) {
                pwm_led(LED_PIN_R, 0);
                
                // Verifica o estado do botão
                check_button_press();

                // Publica os dados
                mqtt_test_publish(state);
                
            } else {
                DEBUG_printf("Reconnecting...\n");
                sleep_ms(1000);
                mqtt_test_connect(state);
            }
            sleep_ms(10);
        }
    }
}