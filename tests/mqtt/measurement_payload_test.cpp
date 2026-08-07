#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <vector>

#include "mqtt/MeasurementPayload.h"

namespace
{
    // Mirrors the JSON payload MqttPublisher currently builds inline via
    // snprintf, so extracting formatMeasurementPayload doesn't change the
    // wire format the collector already parses.
    constexpr const char* kExpectedPayload =
        "{\"room\":\"living_room\", \"timestamp\":1738156800000,\"co2Ppm\":404, "
        "\"temperatureCelsius\":25.00,\"humidityPercent\":50.00}";
}

TEST(MeasurementPayloadTest, FormatsTypicalMeasurementAsExpectedJson)
{
    const Measurement measurement{1738156800000ULL, 404, 25.0f, 50.0f};
    char buffer[192];
    std::size_t outLength = 0;

    EXPECT_TRUE(formatMeasurementPayload("living_room", measurement, buffer, sizeof(buffer), outLength));

    EXPECT_EQ(outLength, std::strlen(kExpectedPayload));
    EXPECT_STREQ(buffer, kExpectedPayload);
}

TEST(MeasurementPayloadTest, FormatsNegativeTemperatureCorrectly)
{
    const Measurement measurement{1000ULL, 900, -10.5f, 65.25f};
    char buffer[192];
    std::size_t outLength = 0;

    ASSERT_TRUE(formatMeasurementPayload("cellar", measurement, buffer, sizeof(buffer), outLength));

    EXPECT_STREQ(buffer,
                 "{\"room\":\"cellar\", \"timestamp\":1000,\"co2Ppm\":900, "
                 "\"temperatureCelsius\":-10.50,\"humidityPercent\":65.25}");
}

TEST(MeasurementPayloadTest, ReturnsFalseWhenBufferIsTooSmallForFixedFields)
{
    const Measurement measurement{1738156800000ULL, 404, 25.0f, 50.0f};
    char buffer[8];
    std::size_t outLength = 0;

    EXPECT_FALSE(formatMeasurementPayload("living_room", measurement, buffer, sizeof(buffer), outLength));
}

TEST(MeasurementPayloadTest, ReturnsFalseWhenRoomNameOverflowsBuffer)
{
    // A misconfigured NODE_ROOM (no length limit is enforced by Kconfig) must
    // not cause the caller to read past its buffer - this is the exact
    // scenario that made the un-guarded snprintf in MqttPublisher unsafe.
    const std::string veryLongRoomName(300, 'a');
    const Measurement measurement{1738156800000ULL, 404, 25.0f, 50.0f};
    char buffer[192];
    std::size_t outLength = 0;

    EXPECT_FALSE(formatMeasurementPayload(veryLongRoomName.c_str(), measurement, buffer, sizeof(buffer), outLength));
}

TEST(MeasurementPayloadTest, SucceedsWhenPayloadExactlyFitsBuffer)
{
    const Measurement measurement{1738156800000ULL, 404, 25.0f, 50.0f};
    const std::size_t exactSize = std::strlen(kExpectedPayload) + 1; // + null terminator
    std::vector<char> buffer(exactSize);
    std::size_t outLength = 0;

    EXPECT_TRUE(formatMeasurementPayload("living_room", measurement, buffer.data(), buffer.size(), outLength));
    EXPECT_EQ(outLength, std::strlen(kExpectedPayload));
    EXPECT_STREQ(buffer.data(), kExpectedPayload);
}

TEST(MeasurementPayloadTest, FailsWhenBufferIsOneByteShortOfFitting)
{
    const Measurement measurement{1738156800000ULL, 404, 25.0f, 50.0f};
    const std::size_t oneByteShort = std::strlen(kExpectedPayload); // no room for the null terminator
    std::vector<char> buffer(oneByteShort);
    std::size_t outLength = 0;

    EXPECT_FALSE(formatMeasurementPayload("living_room", measurement, buffer.data(), buffer.size(), outLength));
}
