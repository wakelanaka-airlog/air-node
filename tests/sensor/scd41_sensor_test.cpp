#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "Scd41Sensor.h"

namespace
{
    uint8_t sensirionCrc8(const uint8_t* data, size_t length)
    {
        uint8_t crc = 0xFF;
        for (size_t i = 0; i < length; ++i)
        {
            crc ^= data[i];
            for (int bit = 0; bit < 8; ++bit)
            {
                crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x31) : static_cast<uint8_t>(crc << 1);
            }
        }
        return crc;
    }

    std::vector<uint8_t> makeFrame(uint8_t high, uint8_t low)
    {
        const uint8_t data[] = {high, low};
        return {high, low, sensirionCrc8(data, 2)};
    }

    class FakeI2cBus : public I2cBus
    {
    public:
        bool write(uint8_t address, const uint8_t* data, size_t length) override
        {
            ++writeCount;
            lastWriteAddress = address;
            lastWriteData.assign(data, data + length);
            return writeResult;
        }

        bool read(uint8_t address, uint8_t* buffer, size_t length) override
        {
            ++readCount;
            lastReadAddress = address;
            lastReadLength = length;
            for (size_t i = 0; i < length && i < readData.size(); ++i)
            {
                buffer[i] = readData[i];
            }
            return readResult;
        }

        int writeCount = 0;
        bool writeResult = true;
        uint8_t lastWriteAddress = 0;
        std::vector<uint8_t> lastWriteData;

        int readCount = 0;
        bool readResult = true;
        uint8_t lastReadAddress = 0;
        size_t lastReadLength = 0;
        std::vector<uint8_t> readData;
    };

    class FakeDelay : public Delay
    {
    public:
        void delayMs(uint32_t milliseconds) override
        {
            ++callCount;
            lastMilliseconds = milliseconds;
        }

        int callCount = 0;
        uint32_t lastMilliseconds = 0;
    };
}

TEST(Scd41SensorTest, StartPeriodicMeasurementSendsCorrectCommand)
{
    FakeI2cBus bus;
    FakeDelay delay;
    Scd41Sensor sensor(bus, delay);

    sensor.startPeriodicMeasurement();

    EXPECT_EQ(bus.writeCount, 1);
    EXPECT_EQ(bus.lastWriteAddress, Scd41Sensor::i2cAddress);
    ASSERT_EQ(bus.lastWriteData.size(), 2u);
    EXPECT_EQ(bus.lastWriteData[0], 0x21);
    EXPECT_EQ(bus.lastWriteData[1], 0xB1);
}

TEST(Scd41SensorTest, IsDataReadySendsCorrectCommand)
{
    FakeI2cBus bus;
    FakeDelay delay;
    bus.readData = makeFrame(0x00, 0x00);
    Scd41Sensor sensor(bus, delay);

    sensor.isDataReady();

    EXPECT_EQ(bus.writeCount, 1);
    EXPECT_EQ(bus.lastWriteAddress, Scd41Sensor::i2cAddress);
    ASSERT_EQ(bus.lastWriteData.size(), 2u);
    EXPECT_EQ(bus.lastWriteData[0], 0xE4);
    EXPECT_EQ(bus.lastWriteData[1], 0xB8);

    EXPECT_EQ(bus.readCount, 1);
    EXPECT_EQ(bus.lastReadAddress, Scd41Sensor::i2cAddress);
    EXPECT_EQ(bus.lastReadLength, 3u);
}

TEST(Scd41SensorTest, IsDataReadyDelaysBeforeReading)
{
    FakeI2cBus bus;
    FakeDelay delay;
    bus.readData = makeFrame(0x00, 0x00);
    Scd41Sensor sensor(bus, delay);

    sensor.isDataReady();

    EXPECT_EQ(delay.callCount, 1);
    EXPECT_EQ(delay.lastMilliseconds, 1u);
}

TEST(Scd41SensorTest, IsDataReadyReturnsTrueWhenReady)
{
    FakeI2cBus bus;
    FakeDelay delay;
    // Lower 11 bits non-zero => ready.
    bus.readData = makeFrame(0x00, 0x01);
    Scd41Sensor sensor(bus, delay);

    EXPECT_TRUE(sensor.isDataReady());
}

TEST(Scd41SensorTest, IsDataReadyChecksFullElevenBitMask)
{
    FakeI2cBus bus;
    FakeDelay delay;

    bus.readData = makeFrame(0x00, 0x80);
    Scd41Sensor sensor(bus, delay);

    EXPECT_TRUE(sensor.isDataReady());
}

TEST(Scd41SensorTest, IsDataReadyReturnsFalseWhenNotReady)
{
    FakeI2cBus bus;
    FakeDelay delay;
    // Lower 11 bits all zero => not ready.
    bus.readData = makeFrame(0x00, 0x00);
    Scd41Sensor sensor(bus, delay);

    EXPECT_FALSE(sensor.isDataReady());
}

TEST(Scd41SensorTest, IsDataReadyReturnsFalseOnCrcMismatch)
{
    FakeI2cBus bus;
    FakeDelay delay;
    bus.readData = makeFrame(0x00, 0x01); // would read as "ready" if trusted
    bus.readData[2] ^= 0xFF;              // corrupt the CRC byte
    Scd41Sensor sensor(bus, delay);

    EXPECT_FALSE(sensor.isDataReady());
}

TEST(Scd41SensorTest, ReadMeasurementSendsCorrectCommand)
{
    FakeI2cBus bus;
    FakeDelay delay;
    const auto co2Frame = makeFrame(0x01, 0x94);
    const auto tempFrame = makeFrame(0x66, 0x67);
    const auto humidityFrame = makeFrame(0x80, 0x00);
    bus.readData = co2Frame;
    bus.readData.insert(bus.readData.end(), tempFrame.begin(), tempFrame.end());
    bus.readData.insert(bus.readData.end(), humidityFrame.begin(), humidityFrame.end());

    Scd41Sensor sensor(bus, delay);
    Measurement measurement{};

    sensor.readMeasurement(measurement);

    EXPECT_EQ(bus.writeCount, 1);
    EXPECT_EQ(bus.lastWriteAddress, Scd41Sensor::i2cAddress);
    ASSERT_EQ(bus.lastWriteData.size(), 2u);
    EXPECT_EQ(bus.lastWriteData[0], 0xEC);
    EXPECT_EQ(bus.lastWriteData[1], 0x05);

    EXPECT_EQ(bus.readCount, 1);
    EXPECT_EQ(bus.lastReadAddress, Scd41Sensor::i2cAddress);
    EXPECT_EQ(bus.lastReadLength, 9u);
}

TEST(Scd41SensorTest, ReadMeasurementReturnsTrueAndDecodesValuesCorrectly)
{
    FakeI2cBus bus;
    FakeDelay delay;
    // Raw values with known expected results:
    //  - CO2 is a direct ppm value, no formula: 0x0194 = 404.
    //  - T = -45 + 175 * raw / 65536: 0x6667 = 26215 -> ~25.00 degC.
    //  - RH = 100 * raw / 65536: 0x8000 = 32768 -> exactly 50%.
    const auto co2Frame = makeFrame(0x01, 0x94);
    const auto tempFrame = makeFrame(0x66, 0x67);
    const auto humidityFrame = makeFrame(0x80, 0x00);
    bus.readData = co2Frame;
    bus.readData.insert(bus.readData.end(), tempFrame.begin(), tempFrame.end());
    bus.readData.insert(bus.readData.end(), humidityFrame.begin(), humidityFrame.end());

    Scd41Sensor sensor(bus, delay);
    Measurement measurement{};

    EXPECT_TRUE(sensor.readMeasurement(measurement));
    EXPECT_EQ(measurement.co2Ppm, 404);
    EXPECT_NEAR(measurement.temperatureCelsius, 25.0f, 0.01f);
    EXPECT_NEAR(measurement.humidityPercent, 50.0f, 0.01f);
}

TEST(Scd41SensorTest, ReadMeasurementDelaysBeforeReading)
{
    FakeI2cBus bus;
    FakeDelay delay;
    bus.readData = makeFrame(0x00, 0x00);
    bus.readData.resize(9);
    Scd41Sensor sensor(bus, delay);
    Measurement measurement{};

    sensor.readMeasurement(measurement);

    EXPECT_EQ(delay.callCount, 1);
    EXPECT_EQ(delay.lastMilliseconds, 1u);
}
