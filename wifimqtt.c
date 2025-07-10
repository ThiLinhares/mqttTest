#include "main.h"

// Define o intervalo mínimo entre publicações MQTT em milissegundos.
#define PUB_DELAY_MS 1000
// Armazena o tempo da última publicação para controlar a frequência.
static uint32_t last_pub_time = 0;

// Protótipo da função de conexão MQTT.
err_t mqtt_test_connect(MQTT_CLIENT_T *state);

/**
 * @brief Lê a temperatura do sensor interno do Pico e converte para Celsius.
 *
 * @return A temperatura em graus Celsius.
 */
float read_onboard_temperature() {
    // Seleciona o canal do ADC conectado ao sensor de temperatura.
    adc_select_input(4);
    // Lê o valor bruto do ADC.
    uint16_t raw = adc_read();
    // Fator de conversão para transformar o valor bruto em tensão.
    const float conversion_factor = 3.3f / (1 << 12);
    float voltage = raw * conversion_factor;
    // Fórmula para converter a tensão em temperatura (conforme datasheet).
    float temp_celsius = 27.0f - (voltage - 0.706f) / 0.001721f;
    return temp_celsius;
}

/**
 * @brief Callback executado quando dados são recebidos de um tópico MQTT inscrito.
 * Controla os LEDs RGB com base nos comandos recebidos.
 */
static void mqtt_pub_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    char buffer[BUFFER_SIZE];
    // Garante que a mensagem não exceda o tamanho do buffer.
    if (len < BUFFER_SIZE) {
        memcpy(buffer, data, len);
        buffer[len] = '\0'; // Adiciona terminador nulo para criar uma string válida.
        DEBUG_printf("Comando recebido: %s\n", buffer);

        // Compara a mensagem recebida e aciona o controle do LED correspondente.
        if (strcmp(buffer, "red_on") == 0) {
            led_control(LED_PIN_R, true);
        } else if (strcmp(buffer, "red_off") == 0) {
            led_control(LED_PIN_R, false);
        } else if (strcmp(buffer, "green_on") == 0) {
            led_control(LED_PIN_G, true);
        } else if (strcmp(buffer, "green_off") == 0) {
            led_control(LED_PIN_G, false);
        } else if (strcmp(buffer, "blue_on") == 0) {
            led_control(LED_PIN_B, true);
        } else if (strcmp(buffer, "blue_off") == 0) {
            led_control(LED_PIN_B, false);
        }
    }
}

/**
 * @brief Callback para a resolução de DNS. Armazena o endereço IP do broker.
 */
void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    MQTT_CLIENT_T *state = (MQTT_CLIENT_T*)callback_arg;
    if (ipaddr) {
        state->remote_addr = *ipaddr; // Armazena o IP resolvido.
        DEBUG_printf("DNS resolvido: %s\n", ip4addr_ntoa(ipaddr));
    } else {
        DEBUG_printf("Falha na resolução de DNS.\n");
    }
}

/**
 * @brief Inicia a resolução de DNS para o host do broker MQTT.
 */
void run_dns_lookup(MQTT_CLIENT_T *state) {
    DEBUG_printf("Iniciando busca DNS para %s...\n", MQTT_SERVER_HOST);
    // Tenta resolver o nome do host. A função `dns_found` será chamada quando terminar.
    if (dns_gethostbyname(MQTT_SERVER_HOST, &(state->remote_addr), dns_found, state) == ERR_INPROGRESS) {
        // Aguarda a resolução ser concluída.
        while (state->remote_addr.addr == 0) {
            cyw43_arch_poll(); // Processa eventos de rede.
            sleep_ms(10);
        }
    }
}

/**
 * @brief Callback executado no início de uma publicação MQTT recebida.
 */
static void mqtt_pub_start_cb(void *arg, const char *topic, u32_t tot_len) {
    DEBUG_printf("Mensagem recebida no tópico: %s, tamanho: %lu\n", topic, tot_len);
}

/**
 * @brief Callback para o status da conexão MQTT.
 */
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        // Acende o LED da placa para indicar conexão bem-sucedida.
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        DEBUG_printf("MQTT conectado.\n");
        // Inscreve-se no tópico para receber comandos.
        mqtt_sub_unsub(client, "pico_w/recv", 1, NULL, NULL, 1);
    } else {
        // Apaga o LED da placa em caso de falha na conexão.
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        DEBUG_printf("Falha na conexão MQTT: %d\n", status);
    }
}

/**
 * @brief Callback executado após uma tentativa de publicação.
 */
void mqtt_pub_request_cb(void *arg, err_t err) {
    // Pode ser usado para verificar se a publicação foi bem-sucedida.
}

/**
 * @brief Publica os dados do sensor (temperatura e botão) via MQTT.
 * Também atualiza o display OLED com as mesmas informações.
 */
err_t mqtt_test_publish(MQTT_CLIENT_T *state) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    // Garante que as publicações ocorram no intervalo definido por PUB_DELAY_MS.
    if (now - last_pub_time >= PUB_DELAY_MS)
    {
        last_pub_time = now;

        // --- Preparação dos dados para o Display ---
        char button_status_str[20];
        snprintf(button_status_str, sizeof(button_status_str), "Botao: %s", monitor_pin_on ? "ON" : "OFF");

        float temperature = read_onboard_temperature();
        char temp_str[20];
        snprintf(temp_str, sizeof(temp_str), "Temp: %.2fC", temperature);

        // Atualiza o display OLED com as informações formatadas.
        display_update(button_status_str, temp_str);

        // --- Publicação dos dados via MQTT ---
        char buffer[BUFFER_SIZE];

        // Publica o estado do botão no tópico "pico_w/pin_status".
        snprintf(buffer, BUFFER_SIZE, "%s", monitor_pin_on ? "ON" : "OFF");
        mqtt_publish(state->mqtt_client, "pico_w/pin_status", buffer, strlen(buffer), 0, 0, mqtt_pub_request_cb, state);

        // Publica a temperatura no tópico "pico_w/temperature".
        snprintf(buffer, BUFFER_SIZE, "%.2f", temperature);
        mqtt_publish(state->mqtt_client, "pico_w/temperature", buffer, strlen(buffer), 0, 0, mqtt_pub_request_cb, state);

        return ERR_OK;
    }
    return ERR_OK;
}

/**
 * @brief Conecta-se ao broker MQTT.
 */
err_t mqtt_test_connect(MQTT_CLIENT_T *state) {
    struct mqtt_connect_client_info_t ci = {0};
    ci.client_id = "PicoW-Thiago"; // ID único para o cliente.

    // Define os callbacks para recebimento de dados.
    mqtt_set_inpub_callback(state->mqtt_client, mqtt_pub_start_cb, mqtt_pub_data_cb, state);

    // Tenta conectar ao broker. `mqtt_connection_cb` será chamada com o resultado.
    return mqtt_client_connect(state->mqtt_client, &(state->remote_addr), MQTT_SERVER_PORT, mqtt_connection_cb, state, &ci);
}

/**
 * @brief Verifica o estado do botão, aplicando um debounce para evitar leituras múltiplas.
 */
void check_button_press() {
    static uint32_t last_press_time = 0;
    static bool last_pin_state = true; // Assume que o pino começa em ALTO (pull-up).

    bool current_pin_state = gpio_get(MONITOR_PIN);

    // Verifica se ocorreu uma borda de descida (botão pressionado).
    if (current_pin_state == false && last_pin_state == true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        // Aplica o debounce para filtrar ruídos.
        if (now - last_press_time > DEBOUNCE_DELAY_MS) {
            last_press_time = now;
            monitor_pin_on = !monitor_pin_on; // Inverte o estado lógico do botão.
            DEBUG_printf("Botao pressionado! Novo estado: %s\n", monitor_pin_on ? "ON" : "OFF");
        }
    }
    last_pin_state = current_pin_state; // Atualiza o último estado conhecido do pino.
}


/**
 * @brief Loop principal que gerencia a conexão e a comunicação MQTT.
 */
void mqtt_run_test(MQTT_CLIENT_T *state) {
    // Aloca um novo cliente MQTT.
    state->mqtt_client = mqtt_client_new();
    if (!state->mqtt_client) {
        DEBUG_printf("Falha ao criar cliente MQTT\n");
        return;
    }

    if (mqtt_test_connect(state) == ERR_OK) {
        // Loop infinito de operação.
        while (1) {
            // Processa eventos de rede (essencial para LwIP).
            cyw43_arch_poll();

            if (mqtt_client_is_connected(state->mqtt_client)) {
                // Se conectado, verifica o botão e publica os dados.
                check_button_press();
                mqtt_test_publish(state);
            } else {
                // Se desconectado, tenta reconectar.
                DEBUG_printf("Reconectando...\n");
                sleep_ms(1000); // Aguarda antes de tentar novamente.
                mqtt_test_connect(state);
            }
            sleep_ms(10); // Pequena pausa para não sobrecarregar a CPU.
        }
    }
}