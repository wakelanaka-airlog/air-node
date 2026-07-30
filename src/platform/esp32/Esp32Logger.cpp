#include "Esp32Logger.h"

#include <esp_log.h>

namespace
{
    constexpr const char* kTag = "Node";
}

void Esp32Logger::error(const char* message)
{
    ESP_LOGE(kTag, "%s", message);
}
