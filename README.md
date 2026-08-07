# air-node

Sensor node firmware for wakelanaka-airlog: an ESP32 (ESP-WROOM-32E) reading
an SCD41 CO2/temperature/humidity sensor and publishing readings over MQTT.

## Layout

- `src/` — hardware-agnostic logic, platform-independent C++, grouped by
  role: `state_machine/` (the GoF-State-pattern node logic), `interfaces/`
  (hardware-agnostic ports: `Sensor`, `Publisher`, `Clock`, `Logger`,
  `I2cBus`, `Delay`, `Measurement`), `sensor/` (the SCD41 driver), `logging/`
  (the `FilteringLogger`, see Development mode below). Code under
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

## Wiring

The SCD41 connects to the ESP32 over I2C:

| SCD41 pin | ESP32 GPIO |
|-----------|------------|
| SDA       | GPIO21     |
| SCL       | GPIO22     |
| VDD       | 3V3        |
| GND       | GND        |

GPIO21/22 are the conventional default I2C pins on ESP32 dev boards (e.g.
most boards silkscreen them as `SDA`/`SCL` directly). Note that GPIO6-11 are
*not* usable for anything — they're wired internally to the module's SPI
flash and aren't broken out.

## Measurement behavior

The SCD41 runs in **low-power periodic measurement mode** (~30s internal
sample rate) rather than normal mode (~5s) — the node only ever acts on one
reading every 15 minutes anyway, so there's no benefit to the faster internal
rate, and low-power mode draws less current.

The node only reads/stores/publishes a measurement **once every 15
minutes** (`kReadingIntervalMs` in `IdleState.cpp`), tracked via
`Node::getLastReadingTime()`/`setLastReadingTime()`. This same 15-minute gate
also covers the sensor's post-restart settling time: every time `InitState`
runs (at boot, or after any `Error`-triggered restart), it stops and restarts
periodic measurement, which resets the SCD41's internal signal compensation —
readings taken immediately after aren't yet accurate (per Sensirion's
datasheet, response time to 63% of a step change is ~60s for CO2, ~90s for
humidity, ~120s for temperature; full settling takes a few time constants
longer). `InitState` seeds the last-reading-time to "now" on a successful
restart, so the very next reading is naturally delayed by the full 15-minute
interval - comfortably past the settling window - without needing a separate
discard-first-reading mechanism.

## Building the firmware

```sh
cd firmware
. $IDF_PATH/export.sh   # source, don't pipe
idf.py set-target esp32 # first time only
idf.py menuconfig       # set WiFi/MQTT/room config - see Configuration below
idf.py build
idf.py -p <PORT> flash monitor
```

### Configuration

Before the first flash, set the node's config via `idf.py menuconfig` under
**"wakelanaka-airlog Node Configuration"** (`firmware/main/Kconfig.projbuild`).
`WIFI_SSID`, `WIFI_PASSWORD`, `NODE_ROOM`, and `MQTT_PASSWORD` all default to
empty strings, so the node won't connect to anything until these are set.

`NODE_DEVELOPMENT_MODE` (off by default) switches logging from
production mode (errors only) to development mode (verbose - includes the
state machine's entering/leaving-state logs). It's a general-purpose
dev-mode flag, meant to gate any future development-only feature, not just
logging.
