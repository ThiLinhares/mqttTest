# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# CMakeLists.txt

cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(wifimqtt C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w)
if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_executable(wifimqtt
    main.c
    wifimqtt.c
    embarca.c
    # --- NOVOS ARQUIVOS ---
    display.c
    ssd1306/ssd1306.c
)

# enable usb output, disable uart output
pico_enable_stdio_usb(wifimqtt 1)
pico_enable_stdio_uart(wifimqtt 0)


target_compile_definitions(wifimqtt PRIVATE
    WIFI_SSID="@thilinhares"
    WIFI_PASSWORD="mafredzudo18"
    NO_SYS=1
)

target_include_directories(wifimqtt PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
)
      
target_link_libraries(wifimqtt
    pico_cyw43_arch_lwip_poll
    pico_stdlib
    hardware_i2c  # --- NECESSÁRIO PARA O DISPLAY ---
    pico_lwip_mbedtls
    pico_mbedtls
    pico_lwip_mqtt
    hardware_pwm
    hardware_gpio
    hardware_adc
)

pico_add_extra_outputs(wifimqtt)