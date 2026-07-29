#include <gtest/gtest.h>

#include "ErrorState.h"
#include "IdleState.h"
#include "Node.h"
#include "SendingState.h"

namespace
{
    class StubSensor : public Sensor
    {
    public:
        bool startPeriodicMeasurement() override
        {
            return true;
        }

        bool isDataReady() override
        {
            return false;
        }

        bool readMeasurement(Measurement&) override
        {
            return false;
        }
    };

    class FakePublisher : public Publisher
    {
    public:
        bool publish(const Measurement& measurement) override
        {
            ++publishCount;
            lastMeasurement = measurement;
            return publishResult;
        }

        int publishCount = 0;
        bool publishResult = true;
        Measurement lastMeasurement{};
    };

    class StubClock : public Clock
    {
    public:
        uint64_t nowUnixMillis() override
        {
            return 0;
        }
    };
}

TEST(SendingStateTest, ProcessPublishesMeasurement)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    Node node(sensor, publisher, clock, SendingState::getInstance());
    node.setMeasurement(Measurement{1738156800000ULL, 404, 25.0f, 50.0f});

    node.process();

    EXPECT_EQ(publisher.publishCount, 1);
    EXPECT_EQ(publisher.lastMeasurement.timestampUnixMillis, 1738156800000ULL);
    EXPECT_EQ(publisher.lastMeasurement.co2Ppm, 404);
    EXPECT_EQ(publisher.lastMeasurement.temperatureCelsius, 25.0f);
    EXPECT_EQ(publisher.lastMeasurement.humidityPercent, 50.0f);
}

TEST(SendingStateTest, ProcessTransitionsToIdleOnPublishSuccess)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    publisher.publishResult = true;
    Node node(sensor, publisher, clock, SendingState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}

TEST(SendingStateTest, ProcessTransitionsToErrorOnPublishFailure)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    publisher.publishResult = false;
    Node node(sensor, publisher, clock, SendingState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &ErrorState::getInstance());
}
