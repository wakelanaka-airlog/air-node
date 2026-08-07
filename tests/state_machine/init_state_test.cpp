#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "ErrorState.h"
#include "IdleState.h"
#include "InitState.h"
#include "Node.h"
#include "support/NullDoubles.h"

namespace
{
    class FakeSensor : public Sensor
    {
    public:
        bool startPeriodicMeasurement() override
        {
            ++startPeriodicMeasurementCount;
            callOrder.push_back("start");
            return startPeriodicMeasurementResult;
        }

        bool stopPeriodicMeasurement() override
        {
            ++stopPeriodicMeasurementCount;
            callOrder.push_back("stop");
            return stopPeriodicMeasurementResult;
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
        int stopPeriodicMeasurementCount = 0;
        bool stopPeriodicMeasurementResult = true;
        std::vector<std::string> callOrder;
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
}

TEST(InitStateTest, ProcessStartsPeriodicMeasurement)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    node.process();

    EXPECT_EQ(sensor.startPeriodicMeasurementCount, 1);
}

TEST(InitStateTest, ProcessStopsPeriodicMeasurement)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    node.process();

    EXPECT_EQ(sensor.stopPeriodicMeasurementCount, 1);
}

TEST(InitStateTest, ProcessStopsBeforeStarting)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    node.process();

    ASSERT_EQ(sensor.callOrder.size(), 2u);
    EXPECT_EQ(sensor.callOrder[0], "stop");
    EXPECT_EQ(sensor.callOrder[1], "start");
}

TEST(InitStateTest, ProcessTransitionsToIdleOnSuccess)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    sensor.startPeriodicMeasurementResult = true;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}

TEST(InitStateTest, ProcessSeedsLastReadingTimeOnSuccess)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    clock.currentTime = 1738156800000ULL;
    NullLogger logger;
    NullMeasurementStore store;
    sensor.startPeriodicMeasurementResult = true;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    node.process();

    EXPECT_EQ(node.getLastReadingTime(), 1738156800000ULL);
}

TEST(InitStateTest, ProcessDoesNotSeedLastReadingTimeOnFailure)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    clock.currentTime = 1738156800000ULL;
    NullLogger logger;
    NullMeasurementStore store;
    sensor.startPeriodicMeasurementResult = false;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());
    node.setLastReadingTime(123);

    node.process();

    EXPECT_EQ(node.getLastReadingTime(), 123ULL);
}

TEST(InitStateTest, ProcessTransitionsToErrorOnFailure)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    sensor.startPeriodicMeasurementResult = false;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &ErrorState::getInstance());
}

TEST(InitStateTest, EnterLogsInfo)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    FakeLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    InitState::getInstance().enter(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}

TEST(InitStateTest, ExitLogsInfo)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    FakeLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    InitState::getInstance().exit(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}
