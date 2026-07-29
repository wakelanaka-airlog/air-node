#include <gtest/gtest.h>

#include "ErrorState.h"
#include "InitState.h"
#include "Node.h"

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

TEST(ErrorStateTest, ProcessTransitionsToInit)
{
    StubSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    Node node(sensor, publisher, clock, ErrorState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &InitState::getInstance());
}
