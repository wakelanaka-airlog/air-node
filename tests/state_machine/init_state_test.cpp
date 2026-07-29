#include <gtest/gtest.h>

#include "IdleState.h"
#include "InitState.h"
#include "Node.h"

namespace
{
    class FakeSensor : public Sensor
    {
    public:
        bool startPeriodicMeasurement() override
        {
            ++startPeriodicMeasurementCount;
            return startPeriodicMeasurementResult;
        }

        bool isDataReady() override
        {
            return false;
        }

        bool readMeasurement(Measurement&) override
        {
            return false;
        }

        int startPeriodicMeasurementCount = 0;
        bool startPeriodicMeasurementResult = true;
    };

    class StubPublisher : public Publisher
    {
    public:
        bool publish(const Measurement&) override
        {
            return true;
        }
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

TEST(InitStateTest, ProcessStartsPeriodicMeasurement)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    Node node(sensor, publisher, clock, InitState::getInstance());

    node.process();

    EXPECT_EQ(sensor.startPeriodicMeasurementCount, 1);
}

TEST(InitStateTest, ProcessTransitionsToIdleOnSuccess)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    sensor.startPeriodicMeasurementResult = true;
    Node node(sensor, publisher, clock, InitState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}
