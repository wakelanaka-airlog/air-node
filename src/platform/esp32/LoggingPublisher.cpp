#include "LoggingPublisher.h"
#include <esp_log.h>

namespace
{
    constexpr const char* kTag = "LoggingPublisher";
}

bool LoggingPublisher::publish(const Measurement& measurement)
{
    ESP_LOGI(kTag, "CO2: %u ppm, Temp: %.2f degC, Humidity: %.2f %%",
             measurement.co2Ppm, measurement.temperatureCelsius, measurement.humidityPercent);
    return true;
}
