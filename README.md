# air-node

Sensor node firmware for wakelanaka-airlog: an ESP32 (ESP-WROOM-32E) reading
an SCD41 CO2/temperature/humidity sensor and publishing readings over MQTT.

## Layout

- `src/` — hardware-agnostic logic, platform-independent C++, grouped by
  role: `state_machine/` (the GoF-State-pattern node logic), `interfaces/`
  (hardware-agnostic ports: `Sensor`, `Publisher`, `I2cBus`, `Delay`,
  `Measurement`), `sensor/` (the SCD41 driver). Code under
  `src/platform/esp32/` may use ESP-IDF headers and is excluded from the
  host build below.
- `tests/` — GoogleTest unit tests for `src/`.
- `firmware/` — the ESP-IDF project that builds the actual firmware image for
  the esp-wroom-32e. Its `main/` component is the entry point (`app_main`)
  and wires the shared `src/` logic to real ESP32/SCD41/MQTT hardware.

Two independent CMake projects, since ESP-IDF's cross-compiling toolchain
can't link against a host GoogleTest build:

## Building & testing the logic (host)

```sh
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug
ctest --test-dir cmake-build-debug
```

## Building the firmware

```sh
cd firmware
. $IDF_PATH/export.sh   # source, don't pipe
idf.py set-target esp32 # first time only
idf.py build
idf.py -p <PORT> flash monitor
```
