#include "main.h"
#define PUB_DELAY_MS 1000
static uint32_t last_pub_time = 0;
err_t mqtt_test_connect(MQTT_CLIENT_T *state);

// Buffer para as duas últimas mensagens
char last_msg[BUFFER_SIZE] = " ";
char prev_msg[BUFFER_SIZE] = " ";


// Função para ler o sensor de temperatura interno e converter para Celsius
float read_onboard_temperature() {
    adc_select_input(4);
    uint16_t raw = adc_read();
    const float conversion_factor = 3.3f / (1 << 12);
    float voltage = raw * conversion_factor;
    float temp_celsius = 27.0f - (voltage - 0.706f) / 0.001721f;
    return temp_celsius;
}

static void mqtt_pub_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    char buffer[BUFFER_SIZE];
    if (len < BUFFER_SIZE) {
        memcpy(buffer, data, len);
        buffer[len] = '\0';
        DEBUG_printf("Command received: %s\n", buffer);

        char display_message[32]; // Buffer para a mensagem em português

        // Traduz o comando recebido para uma string amigável
        if (strcmp(buffer, "red_on") == 0) {
            strcpy(display_message, "Vermelho Ligado");
            led_control(LED_PIN_R, true);
        } else if (strcmp(buffer, "red_off") == 0) {
            strcpy(display_message, "Vermelho Desligado");
            led_control(LED_PIN_R, false);
        } else if (strcmp(buffer, "green_on") == 0) {
            strcpy(display_message, "Verde Ligado");
            led_control(LED_PIN_G, true);
        } else if (strcmp(buffer, "green_off") == 0) {
            strcpy(display_message, "Verde Desligado");
            led_control(LED_PIN_G, false);
        } else if (strcmp(buffer, "blue_on") == 0) {
            strcpy(display_message, "Azul Ligado");
            led_control(LED_PIN_B, true);
        } else if (strcmp(buffer, "blue_off") == 0) {
            strcpy(display_message, "Azul Desligado");
            led_control(LED_PIN_B, false);
        } else {
            strcpy(display_message, "Comando invalido"); // Mensagem para comandos não reconhecidos
        }
        
        // <<< LÓGICA DO DISPLAY >>>
        // Move a última mensagem para a posição da anterior
        strcpy(prev_msg, last_msg);
        // Armazena a nova mensagem
        strcpy(last_msg, buffer);
        // Atualiza a tela com as duas mensagens
        display_update(last_msg, prev_msg);
        // <<< FIM DA LÓGICA DO DISPLAY >>>


        // <<< CORREÇÃO: CHAMANDO A NOVA FUNÇÃO led_control >>>
        // Comandos para o LED Vermelho (GPIO 13)
        if (strcmp(buffer, "red_on") == 0) {
            led_control(LED_PIN_R, true); // true para ligar
        } else if (strcmp(buffer, "red_off") == 0) {
            led_control(LED_PIN_R, false); // false para desligar
        }
        // Comandos para o LED Verde (GPIO 11)
        else if (strcmp(buffer, "green_on") == 0) {
            led_control(LED_PIN_G, true);
        } else if (strcmp(buffer, "green_off") == 0) {
            led_control(LED_PIN_G, false);
        }
        // Comandos para o LED Azul (GPIO 12)
        else if (strcmp(buffer, "blue_on") == 0) {
            led_control(LED_PIN_B, true);
        } else if (strcmp(buffer, "blue_off") == 0) {
            led_control(LED_PIN_B, false);
        }
    }
}

// --- O restante do arquivo (funções de conexão, publicação, etc.) permanece o mesmo ---
// ... (copie o resto do seu wifimqtt.c aqui se necessário, mas as funções abaixo não precisam de mudança)

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

static void mqtt_pub_start_cb(void *arg, const char *topic, u32_t tot_len) {
    DEBUG_printf("Incoming message on topic: %s, size: %lu\n", topic, tot_len);
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        // Liga o LED verde da placa para indicar conexão MQTT bem-sucedida
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        DEBUG_printf("MQTT connected.\n");
        mqtt_sub_unsub(client, "pico_w/recv", 1, NULL, NULL, 1);
    } else {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        DEBUG_printf("MQTT connection failed: %d\n", status);
    }
}

void mqtt_pub_request_cb(void *arg, err_t err) {}

err_t mqtt_test_publish(MQTT_CLIENT_T *state) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_pub_time >= PUB_DELAY_MS)
    {
        last_pub_time = now;
        char buffer[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "%s", monitor_pin_on ? "ON" : "OFF");
        mqtt_publish(state->mqtt_client, "pico_w/pin_status", buffer, strlen(buffer), 0, 0, mqtt_pub_request_cb, state);
        float temperature = read_onboard_temperature();
        snprintf(buffer, BUFFER_SIZE, "%.2f", temperature);
        mqtt_publish(state->mqtt_client, "pico_w/temperature", buffer, strlen(buffer), 0, 0, mqtt_pub_request_cb, state);
        return ERR_OK;
    }
    return ERR_OK;
}

err_t mqtt_test_connect(MQTT_CLIENT_T *state) {
    struct mqtt_connect_client_info_t ci = {0};
    ci.client_id = "PicoW-Thiago";
    mqtt_set_inpub_callback(state->mqtt_client, mqtt_pub_start_cb, mqtt_pub_data_cb, state);
    return mqtt_client_connect(state->mqtt_client, &(state->remote_addr), MQTT_SERVER_PORT, mqtt_connection_cb, state, &ci);
}

void check_button_press() {
    static uint32_t last_press_time = 0;
    static bool last_pin_state = true;
    bool current_pin_state = gpio_get(MONITOR_PIN);
    if (current_pin_state == false && last_pin_state == true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_press_time > DEBOUNCE_DELAY_MS) {
            last_press_time = now;
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
                check_button_press();
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