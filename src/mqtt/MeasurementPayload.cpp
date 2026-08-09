#include "mqtt/MeasurementPayload.h"

#include <cstdio>

// Room isn't included here - it's already the MQTT topic's wildcard segment
// (wakelanaka-airlog/air-node/<room>/measurement), and the collector reads
// it from there instead, so it's not asserted twice on the wire.
bool formatMeasurementPayload(const Measurement& measurement, char* buffer, const std::size_t bufferSize,
                               std::size_t& outLength)
{
    const int written = std::snprintf(buffer, bufferSize,
                                       "{\"timestamp\":%llu,\"co2Ppm\":%u,\"temperatureCelsius\":%.2f,"
                                       "\"humidityPercent\":%.2f}",
                                       static_cast<unsigned long long>(measurement.timestampUnixMillis),
                                       measurement.co2Ppm, measurement.temperatureCelsius,
                                       measurement.humidityPercent);

    if (written < 0 || static_cast<std::size_t>(written) >= bufferSize)
    {
        return false;
    }

    outLength = static_cast<std::size_t>(written);
    return true;
}
