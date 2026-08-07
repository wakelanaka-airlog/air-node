#include <gtest/gtest.h>

#include <string>

#include "ErrorState.h"
#include "InitState.h"
#include "Node.h"
#include "support/NullDoubles.h"

namespace
{
    class FakeLogger : public Logger
    {
    public:
        void info(const char* message) override
        {
            ++infoCount;
            lastInfoMessage = message ? message : "";
        }

        void error(const char* message) override
        {
            ++errorCount;
            lastMessage = message ? message : "";
        }

        int infoCount = 0;
        std::string lastInfoMessage;
        int errorCount = 0;
        std::string lastMessage;
    };
}

TEST(ErrorStateTest, ProcessTransitionsToInit)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    FakeLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, ErrorState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &InitState::getInstance());
}

TEST(ErrorStateTest, ProcessLogsAnError)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    FakeLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, ErrorState::getInstance());

    node.process();

    EXPECT_EQ(logger.errorCount, 1);
    EXPECT_FALSE(logger.lastMessage.empty());
}

TEST(ErrorStateTest, EnterLogsInfo)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    FakeLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, ErrorState::getInstance());

    ErrorState::getInstance().enter(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}

TEST(ErrorStateTest, ExitLogsInfo)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    FakeLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, ErrorState::getInstance());

    ErrorState::getInstance().exit(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}
