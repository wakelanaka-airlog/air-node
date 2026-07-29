#pragma once

#include "esp_event.h"
#include "freertos/event_groups.h"

class Esp32WifiStation
{
public:
    Esp32WifiStation(const char* ssid, const char* password);

    Esp32WifiStation(const Esp32WifiStation&) = delete;
    Esp32WifiStation& operator=(const Esp32WifiStation&) = delete;

    void waitUntilConnected();

private:
    static void handleEvent(void* handlerArgs, esp_event_base_t base, int32_t eventId, void* eventData);

    EventGroupHandle_t eventGroup_;
};
