#include "Esp32SntpClock.h"

#include <sys/time.h>

#include "esp_netif_sntp.h"
#include "freertos/FreeRTOS.h"

namespace
{
    constexpr uint32_t kSyncTimeoutMs = 10000;
}

Esp32SntpClock::Esp32SntpClock(const char* ntpServer, Logger& logger)
{
    const esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(ntpServer);
    esp_netif_sntp_init(&config);

    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(kSyncTimeoutMs)) != ESP_OK)
    {
        logger.error("SNTP sync did not complete within timeout - clock may be inaccurate");
    }
}

uint64_t Esp32SntpClock::nowUnixMillis()
{
    timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<uint64_t>(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}
