#include "main.h"

// --- Variáveis Globais ---
// Controlam o estado de diferentes partes do sistema.

// Indica se o pino de monitoramento (botão) está logicamente "ligado" ou "desligado".
bool monitor_pin_on = false;
// Flag para controle de um possível alarme (não utilizado atualmente).
bool alarme = false;
// Flag para a posição de um joystick (não utilizado atualmente).
bool posicao_js = false;
// Flag para o estado do LED verde (não utilizado atualmente).
bool ledverdestatus = false;

/**
 * @brief Aloca e inicializa a estrutura de estado do cliente MQTT.
 *
 * @return Ponteiro para a estrutura MQTT_CLIENT_T alocada, ou NULL se a alocação falhar.
 */
static MQTT_CLIENT_T* mqtt_client_init(void) {
    // Aloca memória para a estrutura que guardará o estado do cliente MQTT.
    MQTT_CLIENT_T *state = calloc(1, sizeof(MQTT_CLIENT_T));
    if (!state) {
        // Imprime um erro se a alocação de memória falhar.
        DEBUG_printf("Falha ao alocar estado MQTT\n");
        return NULL;
    }
    return state;
}

/**
 * @brief Ponto de entrada principal do programa.
 */
int main() {
    // Inicializa a E/S padrão (para `printf` via USB).
    stdio_init_all();
    // Inicializa os pinos da placa (LEDs, ADC, Display).
    pinos_start();

    // Habilita o sensor de temperatura interno do microcontrolador.
    adc_set_temp_sensor_enabled(true);

    // Inicializa a interface de rede (CYW43).
    if (cyw43_arch_init()) {
        DEBUG_printf("Falha ao inicializar Wi-Fi\n");
        return 1;
    }
    // Habilita o modo "Station" (cliente Wi-Fi).
    cyw43_arch_enable_sta_mode();

    // Conecta à rede Wi-Fi com um timeout de 30 segundos.
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        DEBUG_printf("Falha ao conectar ao Wi-Fi\n");
        return 1;
    }

    // Prepara o cliente MQTT.
    MQTT_CLIENT_T *state = mqtt_client_init();
    // Resolve o endereço IP do broker MQTT.
    run_dns_lookup(state);
    // Inicia o loop principal do MQTT.
    mqtt_run_test(state);

    // Desinicializa a interface de rede ao final.
    cyw43_arch_deinit();
    return 0;
}