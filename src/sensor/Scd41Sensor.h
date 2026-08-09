#pragma once

#include <cstdint>

#include "Delay.h"
#include "I2cBus.h"
#include "Sensor.h"

class Scd41Sensor : public Sensor
{
public:
    static constexpr uint8_t i2cAddress = 0x62;

    Scd41Sensor(I2cBus& bus, Delay& delay);

    bool startPeriodicMeasurement() override;
    bool stopPeriodicMeasurement() override;
    bool isDataReady() override;
    bool readMeasurement(Measurement& measurement) override;
    bool setTemperatureOffset(float offsetCelsius) override;

private:
    static uint8_t computeCrc8(const uint8_t* data, uint16_t count);
    I2cBus& bus_;
    Delay& delay_;
};
