#pragma once // Garante que o arquivo seja incluído apenas uma vez.

#include <time.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "lwip/dns.h"
#include "lwip/apps/mqtt.h"
#include "display.h" // Inclusão do cabeçalho do display.

/* --- Macros da Placa Pico --- */
#define LED_PIN_G 11
#define LED_PIN_B 12
#define LED_PIN_R 13
#define MONITOR_PIN 5          // Pino GPIO a ser monitorado (ex: botão).
#define DEBOUNCE_DELAY_MS 200  // Tempo (ms) para filtrar ruído do botão.

/* --- Macros de Configuração MQTT --- */
#define DEBUG_printf printf
#define MQTT_SERVER_HOST "broker.hivemq.com"
#define MQTT_SERVER_PORT 1883
#define BUFFER_SIZE 256

/* --- Variáveis Globais Externas --- */
// Declaradas como 'extern' para serem acessíveis em outros arquivos
// que incluam este cabeçalho.
extern bool monitor_pin_on;
extern bool alarme;
extern bool posicao_js;
extern bool ledverdestatus;

/* --- Estrutura de Estado MQTT --- */
typedef struct MQTT_CLIENT_T_ {
    ip_addr_t remote_addr;      // Endereço IP do broker MQTT.
    mqtt_client_t *mqtt_client; // Instância do cliente MQTT da biblioteca LwIP.
    u32_t received;             // Contador de pacotes recebidos (se necessário).
    u32_t counter;              // Contador genérico (se necessário).
    u32_t reconnect;            // Contador de tentativas de reconexão (se necessário).
} MQTT_CLIENT_T;

/* --- Protótipos de Funções Públicas --- */
void pinos_start();
void led_control(uint gpio_pin, bool on);
void display_init();
void display_update(const char* msg1, const char* msg2);
void run_dns_lookup(MQTT_CLIENT_T *state);
void mqtt_run_test(MQTT_CLIENT_T *state);