#include <gtest/gtest.h>

#include "IdleState.h"
#include "Node.h"

namespace
{
    class FakeSensor : public Sensor
    {
    public:
        bool startPeriodicMeasurement() override
        {
            return true;
        }

        bool isDataReady() override
        {
            ++isDataReadyCount;
            return isDataReadyResult;
        }

        bool readMeasurement(Measurement&) override
        {
            return false;
        }

        int isDataReadyCount = 0;
        bool isDataReadyResult = false;
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

TEST(IdleStateTest, ProcessChecksDataReady)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    Node node(sensor, publisher, clock, IdleState::getInstance());

    node.process();

    EXPECT_EQ(sensor.isDataReadyCount, 1);
}
