# Entry point component: app_main.cpp, plus the shared src/ logic
# (hardware-agnostic and ESP32-specific) it wires together.

idf_component_register(
    SRC_DIRS "." "../../src" "../../src/state_machine" "../../src/interfaces" "../../src/sensor" "../../src/platform/esp32" "../../src/logging" "../../src/storage"
    INCLUDE_DIRS "." "../../src" "../../src/state_machine" "../../src/interfaces" "../../src/sensor" "../../src/platform/esp32" "../../src/logging" "../../src/storage"
    REQUIRES driver mqtt esp_wifi nvs_flash
)
