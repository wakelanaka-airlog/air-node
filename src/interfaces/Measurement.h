#pragma once

#include <cstdint>

struct Measurement
{
    uint64_t timestampUnixMillis;
    uint16_t co2Ppm;
    float temperatureCelsius;
    float humidityPercent;
};
