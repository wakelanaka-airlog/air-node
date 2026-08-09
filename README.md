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
sample rate) rather than normal mode (~5s) - low-power mode draws less
current, and the node has no reason to poll faster than the sensor itself
produces new samples.

The node reads/stores/publishes **every reading the SCD41 produces** - there
is no reporting-interval throttle in the firmware, and the collector saves
every one of them into TimescaleDB's raw `air_measurements` hypertable.
Anything coarser (e.g. a 15-minute-average view) is a downstream concern,
handled by a TimescaleDB continuous aggregate
(`hub/timescaledb/init/003-air-measurements-continuous-aggregate.sql`), not
the node.

The one thing the node does still gate on is the sensor's post-restart
**settling time**: every time `InitState` runs (at boot, or after any
`Error`-triggered restart), it stops and restarts periodic measurement, which
resets the SCD41's internal signal compensation - readings taken immediately
after aren't yet accurate (per Sensirion's datasheet, response time to 63% of
a step change is ~60s for CO2, ~90s for humidity, ~120s for temperature; full
settling takes a few time constants longer). `InitState` seeds a restart
timestamp (`Node::getLastRestartTime()`/`setLastRestartTime()`) on a
successful restart, and `IdleState` withholds the first post-restart reading
until `kSensorSettlingDelayMs` (10 minutes, `IdleState.cpp`) has passed -
after that, every ready reading is taken and published immediately.

If a publish fails (MQTT/WiFi outage), `SendingState` queues the reading in
`InMemoryMeasurementStore` - a RAM-only ring buffer - and drains it on a
later successful publish, so a failed send doesn't stop the node from
continuing to take new readings. `kMeasurementStoreCapacity` (`app_main.cpp`)
is derived from the SCD41's ~30s reading interval and a target overnight
(12h) outage-survival window, not a fixed number - it's sized to whatever
that math currently works out to, so a future cadence change won't silently
shrink how long an outage the node can absorb (as happened here: this
constant was originally sized for once-every-15-minutes readings, before the
node moved to publishing every reading it takes). Being RAM-only, the backlog
doesn't survive a power cycle - acceptable for this project's lossy-telemetry
use case, not a guarantee.

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

### Flashing multiple nodes

`idf.py menuconfig` writes to a single `firmware/sdkconfig` file, which is
gitignored (it holds `WIFI_PASSWORD`/`MQTT_PASSWORD` in plaintext) but does
persist as-is across ordinary `idf.py build`/`flash` calls - config is only
lost if you delete that file yourself. The one thing it can't do is hold two
nodes' settings at once: each physical node needs its own `NODE_ROOM` and
`NODE_TEMPERATURE_OFFSET_CENTIDEGREES_C` (see Measurement behavior above),
so reusing the same `sdkconfig` file to flash a second node overwrites the
first node's settings.

To keep one persisted, gitignored config per node, override ESP-IDF's
`SDKCONFIG` (and build directory, since generated headers are
config-dependent) per node:

```sh
idf.py -D SDKCONFIG=sdkconfig.living_room -B build.living_room menuconfig
idf.py -D SDKCONFIG=sdkconfig.living_room -B build.living_room build
idf.py -D SDKCONFIG=sdkconfig.living_room -B build.living_room -p <PORT> flash monitor
```

`export SDKCONFIG=sdkconfig.living_room` in your shell avoids repeating the
first flag for a session spent flashing just that one node.
