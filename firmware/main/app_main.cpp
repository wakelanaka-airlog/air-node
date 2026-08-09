#include "InitState.h"
#include "Node.h"
#include "Scd41Sensor.h"
#include "platform/esp32/Esp32Delay.h"
#include "platform/esp32/Esp32I2cBus.h"
#include "platform/esp32/Esp32Logger.h"
#include "platform/esp32/Esp32SntpClock.h"
#include "platform/esp32/MqttPublisher.h"
#include "platform/esp32/Esp32WifiStation.h"

#include <cstddef>
#include <cstdlib>
#include <string>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "logging/FilteringLogger.h"
#include "storage/InMemoryMeasurementStore.h"

#ifdef CONFIG_NODE_DEVELOPMENT_MODE
constexpr bool developmentMode = true;
#else
constexpr bool developmentMode = false;
#endif

namespace
{
    constexpr const char* kTag = "app_main";
    constexpr uint32_t kWifiConnectTimeoutMs = 30000;

    // SCD41 low-power periodic mode samples every ~30s; sized to survive a
    // 12-hour (overnight) MQTT/WiFi outage before the oldest backlogged
    // reading gets overwritten - see README.md's Measurement behavior section.
    constexpr uint32_t kScd41ReadingIntervalSeconds = 30;
    constexpr uint32_t kOvernightOutageSurvivalHours = 12;
    constexpr std::size_t kMeasurementStoreCapacity =
        kOvernightOutageSurvivalHours * 3600 / kScd41ReadingIntervalSeconds;
}

extern "C" void app_main()
{
    esp_err_t nvsResult = nvs_flash_init();
    if (nvsResult == ESP_ERR_NVS_NO_FREE_PAGES || nvsResult == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvsResult = nvs_flash_init();
        if (nvsResult != ESP_OK)
        {
            ESP_LOGE(kTag, "NVS could not be initialize");
            abort();
        }
    }

    esp_netif_init();
    esp_event_loop_create_default();

    static Esp32WifiStation wifiStation(CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);
    if (!wifiStation.waitUntilConnected(kWifiConnectTimeoutMs))
    {
        ESP_LOGE(kTag, "WiFi did not connect within %lu ms - halting, not starting the node",
                 static_cast<unsigned long>(kWifiConnectTimeoutMs));
        abort();
    }

    static Esp32Logger logger{};
    static FilteringLogger filteringLogger{logger, developmentMode};

    static Esp32SntpClock clock(CONFIG_SNTP_SERVER, filteringLogger);

    static Esp32I2cBus i2cBus{I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22, filteringLogger};
    static Esp32Delay delay{};
    static Scd41Sensor scd41Sensor{i2cBus, delay};

    const std::string topic = std::string("wakelanaka-airlog/air-node/") + CONFIG_NODE_ROOM + "/measurement";
    static MqttPublisher publisher{CONFIG_MQTT_BROKER_URI, CONFIG_MQTT_USERNAME, CONFIG_MQTT_PASSWORD, topic};

    static InMemoryMeasurementStore measurementStore{kMeasurementStoreCapacity};

    constexpr float temperatureOffsetCelsius = CONFIG_NODE_TEMPERATURE_OFFSET_CENTIDEGREES_C / 100.f;
    static Node node{
        scd41Sensor, publisher, clock, filteringLogger, measurementStore, InitState::getInstance(),
        temperatureOffsetCelsius
    };

    while (true)
    {
        node.process();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
