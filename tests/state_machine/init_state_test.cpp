#include <gtest/gtest.h>

#include <string>

#include "ErrorState.h"
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

TEST(InitStateTest, ProcessStartsPeriodicMeasurement)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubLogger logger;
    Node node(sensor, publisher, clock, logger, InitState::getInstance());

    node.process();

    EXPECT_EQ(sensor.startPeriodicMeasurementCount, 1);
}

TEST(InitStateTest, ProcessTransitionsToIdleOnSuccess)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubLogger logger;
    sensor.startPeriodicMeasurementResult = true;
    Node node(sensor, publisher, clock, logger, InitState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}

TEST(InitStateTest, ProcessTransitionsToErrorOnFailure)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubLogger logger;
    sensor.startPeriodicMeasurementResult = false;
    Node node(sensor, publisher, clock, logger, InitState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &ErrorState::getInstance());
}

TEST(InitStateTest, EnterLogsInfo)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    FakeLogger logger;
    Node node(sensor, publisher, clock, logger, InitState::getInstance());

    InitState::getInstance().enter(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}

TEST(InitStateTest, ExitLogsInfo)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    FakeLogger logger;
    Node node(sensor, publisher, clock, logger, InitState::getInstance());

    InitState::getInstance().exit(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}
