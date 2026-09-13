#include "Scd41Sensor.h"
#include "I2cBus.h"

Scd41Sensor::Scd41Sensor(I2cBus& bus, Delay& delay) : bus_{bus}, delay_{delay}
{
}

bool Scd41Sensor::isDataReady()
{
    constexpr uint8_t command[] = {0xe4, 0xb8};
    if (bus_.write(i2cAddress, command, sizeof(command)))
    {
        delay_.delayMs(1);
        uint8_t response[3];
        if (bus_.read(i2cAddress, response, sizeof(response)))
        {
            const uint8_t data[] = {response[0], response[1]};
            if (computeCrc8(data, sizeof(data)) != response[2])
            {
                return false;
            }
            return (response[0] << 8 | response[1]) & 0x7FF;
        }
    }
    return false;
}

bool Scd41Sensor::startPeriodicMeasurement()
{
    constexpr uint8_t command[] = {0x21, 0xac};
    return bus_.write(i2cAddress, command, sizeof(command));
}

bool Scd41Sensor::stopPeriodicMeasurement()
{
    constexpr uint8_t command[] = {0x3f, 0x86};
    const bool result = bus_.write(i2cAddress, command, sizeof(command));

    delay_.delayMs(500);
    return result;
}

bool Scd41Sensor::readMeasurement(Measurement& measurement)
{
    constexpr uint8_t command[] = {0xec, 0x05};
    if (bus_.write(i2cAddress, command, sizeof(command)))
    {
        delay_.delayMs(1);
        uint8_t response[9];
        if (bus_.read(i2cAddress, response, sizeof(response)))
        {
            measurement.co2Ppm = response[0] << 8 | response[1];
            measurement.temperatureCelsius = -45.f + 175.f * static_cast<float>(response[3] << 8 | response[4]) /
                65536.f;
            measurement.humidityPercent = 100.f * static_cast<float>(response[6] << 8 | response[7]) /
                65536.f;

            const uint8_t co2Data[] = {response[0], response[1]};
            const uint8_t temperatureData[] = {response[3], response[4]};
            const uint8_t humidityData[] = {response[6], response[7]};
            return computeCrc8(co2Data, sizeof(co2Data)) == response[2] && computeCrc8(
                    temperatureData, sizeof(temperatureData)) == response[5] &&
                computeCrc8(humidityData, sizeof(humidityData)) == response[8];
        }
    }
    return false;
}

bool Scd41Sensor::setTemperatureOffset(float offsetCelsius)
{
    const uint16_t word = static_cast<uint16_t>(offsetCelsius * 65536.f / 175.f);
    const uint8_t data[] = {static_cast<uint8_t>(word >> 8), static_cast<uint8_t>(word & 0xFF)};
    const uint8_t command[] = {0x24, 0x1d, data[0], data[1], computeCrc8(data, sizeof(data))};
    const bool result = bus_.write(i2cAddress, command, sizeof(command));

    delay_.delayMs(1);
    return result;
}

// Copied from SCD41 datasheet
uint8_t Scd41Sensor::computeCrc8(const uint8_t* data, uint16_t count)
{
    constexpr uint8_t crc8Polynomial = 0x31;
    constexpr uint8_t crc8Init = 0xFF;
    uint16_t current_byte;
    uint8_t crc = crc8Init;
    uint8_t crc_bit;
    for (current_byte = 0; current_byte < count; ++current_byte)
    {
        crc ^= (data[current_byte]);
        for (crc_bit = 8; crc_bit > 0; --crc_bit)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ crc8Polynomial;
            else
                crc = (crc << 1);
        }
    }
    return crc;
}
