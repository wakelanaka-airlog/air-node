#include "InitState.h"
#include "Node.h"
#include "Scd41Sensor.h"
#include "platform/esp32/Esp32Delay.h"
#include "platform/esp32/Esp32I2cBus.h"
#include "platform/esp32/Esp32Logger.h"
#include "platform/esp32/Esp32SntpClock.h"
#include "platform/esp32/MqttPublisher.h"

#include <string>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "Esp32WifiStation.h"

namespace
{
    constexpr const char* kTag = "app_main";
    constexpr uint32_t kWifiConnectTimeoutMs = 30000;
}

extern "C" void app_main()
{
    esp_err_t nvsResult = nvs_flash_init();
    if (nvsResult == ESP_ERR_NVS_NO_FREE_PAGES || nvsResult == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }

    esp_netif_init();
    esp_event_loop_create_default();

    static Esp32WifiStation wifiStation(CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);
    if (!wifiStation.waitUntilConnected(kWifiConnectTimeoutMs))
    {
        ESP_LOGE(kTag, "WiFi did not connect within %lu ms - halting, not starting the node",
                 static_cast<unsigned long>(kWifiConnectTimeoutMs));
        while (true)
        {
            vTaskDelay(portMAX_DELAY);
        }
    }

    static Esp32SntpClock clock(CONFIG_SNTP_SERVER);

    static Esp32I2cBus i2cBus{I2C_NUM_0, GPIO_NUM_8, GPIO_NUM_9};
    static Esp32Delay delay{};
    static Scd41Sensor scd41Sensor{i2cBus, delay};
    static Esp32Logger logger{};

    const std::string topic = std::string("wakelanaka-airlog/air-node/") + CONFIG_NODE_ROOM + "/measurement";
    static MqttPublisher publisher{
        CONFIG_MQTT_BROKER_URI, CONFIG_MQTT_USERNAME, CONFIG_MQTT_PASSWORD, topic, CONFIG_NODE_ROOM
    };

    static Node node{scd41Sensor, publisher, clock, logger, InitState::getInstance()};

    while (true)
    {
        node.process();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
