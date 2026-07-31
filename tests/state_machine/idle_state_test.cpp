#include <gtest/gtest.h>

#include <string>

#include "IdleState.h"
#include "Node.h"
#include "ReadingState.h"

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

    class StubLogger : public Logger
    {
    public:
        void info(const char*) override
        {
        }

        void error(const char*) override
        {
        }
    };

    class FakeLogger : public Logger
    {
    public:
        void info(const char* message) override
        {
            ++infoCount;
            lastInfoMessage = message ? message : "";
        }

        void error(const char*) override
        {
        }

        int infoCount = 0;
        std::string lastInfoMessage;
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
    StubLogger logger;
    Node node(sensor, publisher, clock, logger, IdleState::getInstance());

    node.process();

    EXPECT_EQ(sensor.isDataReadyCount, 1);
}

TEST(IdleStateTest, ProcessTransitionsToReadingWhenDataIsReady)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubLogger logger;
    sensor.isDataReadyResult = true;
    Node node(sensor, publisher, clock, logger, IdleState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &ReadingState::getInstance());
}

TEST(IdleStateTest, ProcessStaysInIdleWhenDataIsNotReady)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubLogger logger;
    sensor.isDataReadyResult = false;
    Node node(sensor, publisher, clock, logger, IdleState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}

TEST(IdleStateTest, EnterLogsInfo)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    FakeLogger logger;
    Node node(sensor, publisher, clock, logger, IdleState::getInstance());

    IdleState::getInstance().enter(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}

TEST(IdleStateTest, ExitLogsInfo)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    FakeLogger logger;
    Node node(sensor, publisher, clock, logger, IdleState::getInstance());

    IdleState::getInstance().exit(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}
