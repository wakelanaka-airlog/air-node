#include "Esp32WifiStation.h"

#include <cstring>

#include "esp_wifi.h"

namespace
{
    constexpr int kConnectedBit = BIT0;
}

Esp32WifiStation::Esp32WifiStation(const char* ssid, const char* password)
{
    eventGroup_ = xEventGroupCreate();

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t initConfig = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&initConfig);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &Esp32WifiStation::handleWifiEvent, this,
                                        nullptr);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &Esp32WifiStation::handleGotIpEvent, this,
                                        nullptr);

    wifi_config_t wifiConfig = {};
    std::strncpy(reinterpret_cast<char*>(wifiConfig.sta.ssid), ssid, sizeof(wifiConfig.sta.ssid) - 1);
    std::strncpy(reinterpret_cast<char*>(wifiConfig.sta.password), password, sizeof(wifiConfig.sta.password) - 1);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    esp_wifi_start();
}

bool Esp32WifiStation::waitUntilConnected(const uint32_t timeoutMs) const
{
    const EventBits_t bits =
        xEventGroupWaitBits(eventGroup_, kConnectedBit, pdFALSE, pdTRUE, pdMS_TO_TICKS(timeoutMs));
    return (bits & kConnectedBit) != 0;
}

void Esp32WifiStation::handleGotIpEvent(void* handlerArgs, const esp_event_base_t base, const int32_t eventId, void*)
{
    const auto* self = static_cast<Esp32WifiStation*>(handlerArgs);
    xEventGroupSetBits(self->eventGroup_, kConnectedBit);
}

void Esp32WifiStation::handleWifiEvent(void* handlerArgs, const esp_event_base_t base, const int32_t eventId, void*)
{
    if (eventId == WIFI_EVENT_STA_START || eventId == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
    }
}
