#include <gtest/gtest.h>

#include <cstring>
#include <vector>

#include "mqtt/MeasurementPayload.h"

namespace
{
    // Mirrors the JSON payload MqttPublisher currently builds inline via
    // snprintf, so extracting formatMeasurementPayload doesn't change the
    // wire format the collector already parses. Room isn't included - the
    // collector reads it from the MQTT topic instead.
    constexpr const char* kExpectedPayload =
        "{\"timestamp\":1738156800000,\"co2Ppm\":404,\"temperatureCelsius\":25.00,\"humidityPercent\":50.00}";
}

TEST(MeasurementPayloadTest, FormatsTypicalMeasurementAsExpectedJson)
{
    const Measurement measurement{1738156800000ULL, 404, 25.0f, 50.0f};
    char buffer[192];
    std::size_t outLength = 0;

    EXPECT_TRUE(formatMeasurementPayload(measurement, buffer, sizeof(buffer), outLength));

    EXPECT_EQ(outLength, std::strlen(kExpectedPayload));
    EXPECT_STREQ(buffer, kExpectedPayload);
}

TEST(MeasurementPayloadTest, FormatsNegativeTemperatureCorrectly)
{
    const Measurement measurement{1000ULL, 900, -10.5f, 65.25f};
    char buffer[192];
    std::size_t outLength = 0;

    ASSERT_TRUE(formatMeasurementPayload(measurement, buffer, sizeof(buffer), outLength));

    EXPECT_STREQ(buffer, "{\"timestamp\":1000,\"co2Ppm\":900,\"temperatureCelsius\":-10.50,\"humidityPercent\":65.25}");
}

TEST(MeasurementPayloadTest, ReturnsFalseWhenBufferIsTooSmallForFixedFields)
{
    const Measurement measurement{1738156800000ULL, 404, 25.0f, 50.0f};
    char buffer[8];
    std::size_t outLength = 0;

    EXPECT_FALSE(formatMeasurementPayload(measurement, buffer, sizeof(buffer), outLength));
}

TEST(MeasurementPayloadTest, SucceedsWhenPayloadExactlyFitsBuffer)
{
    const Measurement measurement{1738156800000ULL, 404, 25.0f, 50.0f};
    const std::size_t exactSize = std::strlen(kExpectedPayload) + 1; // + null terminator
    std::vector<char> buffer(exactSize);
    std::size_t outLength = 0;

    EXPECT_TRUE(formatMeasurementPayload(measurement, buffer.data(), buffer.size(), outLength));
    EXPECT_EQ(outLength, std::strlen(kExpectedPayload));
    EXPECT_STREQ(buffer.data(), kExpectedPayload);
}

TEST(MeasurementPayloadTest, FailsWhenBufferIsOneByteShortOfFitting)
{
    const Measurement measurement{1738156800000ULL, 404, 25.0f, 50.0f};
    const std::size_t oneByteShort = std::strlen(kExpectedPayload); // no room for the null terminator
    std::vector<char> buffer(oneByteShort);
    std::size_t outLength = 0;

    EXPECT_FALSE(formatMeasurementPayload(measurement, buffer.data(), buffer.size(), outLength));
}
