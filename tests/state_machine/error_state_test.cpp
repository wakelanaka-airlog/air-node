#include <gtest/gtest.h>

#include <string>

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

    class FakeLogger : public Logger
    {
    public:
        void error(const char* message) override
        {
            ++errorCount;
            lastMessage = message ? message : "";
        }

        int errorCount = 0;
        std::string lastMessage;
    };
}

TEST(ErrorStateTest, ProcessTransitionsToInit)
{
    StubSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    FakeLogger logger;
    Node node(sensor, publisher, clock, logger, ErrorState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &InitState::getInstance());
}

TEST(ErrorStateTest, ProcessLogsAnError)
{
    StubSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    FakeLogger logger;
    Node node(sensor, publisher, clock, logger, ErrorState::getInstance());

    node.process();

    EXPECT_EQ(logger.errorCount, 1);
    EXPECT_FALSE(logger.lastMessage.empty());
}
