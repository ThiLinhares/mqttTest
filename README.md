# Cliente MQTT com Display OLED na Pico W

Este repositório contém o código-fonte para um projeto de IoT utilizando o microcontrolador Raspberry Pi Pico W. O dispositivo atua como um cliente MQTT para monitorar sensores locais, exibir dados em um display OLED e permitir o controle remoto de um LED RGB.


---

## 📋 Funcionalidades

* **Monitoramento de Sensores:** Lê a temperatura interna do chip RP2040 e o estado de um botão (com tratamento de debounce).
* **Display Local:** Exibe os dados dos sensores em tempo real em um display OLED I2C.
* **Comunicação MQTT:** Conecta-se a um broker MQTT para publicar os dados dos sensores e se inscrever em um tópico de comando.
* **Controle Remoto:** Recebe comandos via MQTT para controlar individualmente as cores de um LED RGB (vermelho, verde e azul).

---

## 🛠️ Componentes Necessários

* Raspberry Pi Pico W
* Display OLED I2C 128x64 (SSD1306)
* 1x LED RGB (cátodo comum ou ânodo comum)
* 1x Botão de pressão (Push-button)
* 3x Resistores de 220Ω (ou valor apropriado para o LED)
* Protoboard e Jumpers
* Cabo Micro USB

---

## ⚙️ Configuração e Instalação

Siga os passos abaixo para compilar e executar o projeto.

### 1. Pré-requisitos

* **Visual Studio Code** com a extensão **Raspberry Pi Pico**.
* **Pico SDK**, **toolchain ARM GCC** e **CMake** devidamente instalados e configurados no seu sistema.

### 2. Clonar o Repositório

Clone este projeto para a sua máquina local:

```bash
git clone https://github.com/ThiLinhares/mqttTest.git

```

### 3. Configurar Credenciais Wi-Fi (Passo Essencial)

Para que o Pico W se conecte à sua rede, você **precisa** adicionar suas credenciais de Wi-Fi. Esta configuração é feita no arquivo `CMakeLists.txt` para evitar expor senhas no código-fonte.

1.  Abra o arquivo `CMakeLists.txt` na raiz do projeto.
2.  Encontre a seção `target_compile_definitions`.
3.  Substitua `"SEU_SSID_AQUI"` e `"SUA_SENHA_AQUI"` pelos dados da sua rede.

```cmake
# Localize esta seção no arquivo CMakeLists.txt
target_compile_definitions(wifimqtt PRIVATE
    # <--- COLOQUE AQUI O NOME DA SUA REDE WIFI
    WIFI_SSID="SEU_SSID_AQUI"

    # <--- COLOQUE AQUI A SENHA DA SUA REDE WIFI
    WIFI_PASSWORD="SUA_SENHA_AQUI"

    NO_SYS=1
)
```

### 4. Compilar o Projeto

Com o VS Code aberto no diretório do projeto, use a paleta de comandos (`Ctrl+Shift+P`) para compilar:

1.  Selecione **CMake: Configure**.
2.  Selecione o Kit **Pico (arm-none-eabi)**.
3.  Selecione **CMake: Build**.

O firmware compilado (`wifimqtt.uf2`) estará no diretório `build`.

---

## 🔌 Montagem do Hardware

Conecte os componentes ao seu Raspberry Pi Pico W conforme o diagrama de pinos abaixo:

| Componente             | Pino no Pico W |
| ---------------------- | -------------- |
| **LED Vermelho (R)** | `GP13`         |
| **LED Verde (G)** | `GP11`         |
| **LED Azul (B)** | `GP12`         |
| **Botão** | `GP5`          |
| **Display OLED (SDA)** | `GP14` (I2C1)  |
| **Display OLED (SCL)** | `GP15` (I2C1)  |
| **Alimentação (VCC)** | `3V3 (OUT)`    |
| **Terra (GND)** | `GND`          |

---

## 🚀 Teste e Interação via MQTT

Você pode monitorar e controlar o dispositivo usando qualquer cliente MQTT. O guia abaixo utiliza o cliente web público do **HiveMQ**.

### Tópicos MQTT do Projeto

* **Publicação (Pico → Nuvem):**
    * `pico_w/temperature`: Publica o valor da temperatura.
    * `pico_w/pin_status`: Publica o estado do botão (`ON`/`OFF`).
* **Inscrição (Nuvem → Pico):**
    * `pico_w/recv`: O Pico escuta este tópico para receber comandos para os LEDs.

### Passo a Passo para Teste

1.  **Acesse o Cliente Web:**
    Abra o [HiveMQ Websocket Client](https://www.hivemq.com/demos/websocket-client/) no seu navegador.

2.  **Conecte-se ao Broker:**
    A página já vem pré-configurada. Apenas clique no botão **Connect**.

3.  **Monitore os Dados (Subscribe):**
    * Na seção **Subscriptions**, clique em **+ Add New Topic Subscription**.
    * Para receber todas as mensagens do dispositivo, use o tópico com curinga: `pico_w/#`
    * Clique em **Subscribe**.
    * Você verá os dados de temperatura e do botão chegando na caixa **Messages**.

4.  **Envie Comandos (Publish):**
    * Vá para a seção **Publish**.
    * No campo **Topic**, digite: `pico_w/recv`
    * No campo **Message**, digite um dos comandos abaixo:
        * `red_on` / `red_off`
        * `green_on` / `green_off`
        * `blue_on` / `blue_off`
    * Clique em **Publish**. O LED no seu dispositivo responderá instantaneamente.
