#include <gtest/gtest.h>

#include <string>

#include "IdleState.h"
#include "Node.h"
#include "ReadingState.h"
#include "support/NullDoubles.h"

namespace
{
    class FakeSensor : public Sensor
    {
    public:
        bool startPeriodicMeasurement() override
        {
            return true;
        }

        bool stopPeriodicMeasurement() override
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

        bool setTemperatureOffset(float) override
        {
            return true;
        }

        int isDataReadyCount = 0;
        bool isDataReadyResult = false;
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

TEST(IdleStateTest, ProcessChecksDataReady)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());

    node.process();

    EXPECT_EQ(sensor.isDataReadyCount, 1);
}

TEST(IdleStateTest, ProcessTransitionsToReadingWhenDataIsReadyOnFirstEverCheck)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    clock.currentTime = 2000000000000ULL;
    NullLogger logger;
    NullMeasurementStore store;
    sensor.isDataReadyResult = true;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &ReadingState::getInstance());
}

TEST(IdleStateTest, ProcessStaysInIdleWhileSensorIsStillSettling)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    clock.currentTime = 2000000000000ULL;
    NullLogger logger;
    NullMeasurementStore store;
    sensor.isDataReadyResult = true;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());
    node.setLastRestartTime(clock.currentTime - 300000ULL); // 5 minutes ago

    node.process();

    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}

TEST(IdleStateTest, ProcessTransitionsToReadingOnceSensorHasSettled)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    clock.currentTime = 2000000000000ULL;
    NullLogger logger;
    NullMeasurementStore store;
    sensor.isDataReadyResult = true;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());
    node.setLastRestartTime(clock.currentTime - 660000ULL); // 11 minutes ago

    node.process();

    EXPECT_EQ(node.getState(), &ReadingState::getInstance());
}

TEST(IdleStateTest, ProcessStaysInIdleWhenClockStepsBackward)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    sensor.isDataReadyResult = true;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());
    node.setLastRestartTime(5000000ULL);
    clock.currentTime = 4000000ULL; // clock stepped backward, e.g. an SNTP resync correction

    node.process();

    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}

TEST(IdleStateTest, ProcessResetsLastRestartTimeToNowWhenClockStepsBackward)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    sensor.isDataReadyResult = true;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());
    node.setLastRestartTime(5000000ULL);
    clock.currentTime = 4000000ULL;

    node.process();

    EXPECT_EQ(node.getLastRestartTime(), 4000000ULL);
}

TEST(IdleStateTest, ProcessTransitionsToReadingAfterFullSettlingDelayFollowingClockStepBackward)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    sensor.isDataReadyResult = true;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());
    node.setLastRestartTime(5000000ULL);
    clock.currentTime = 4000000ULL; // clock steps backward, absorbed without transitioning
    node.process();

    clock.currentTime = 4000000ULL + 10 * 60 * 1000; // full settling delay elapses from the new baseline
    node.process();

    EXPECT_EQ(node.getState(), &ReadingState::getInstance());
}

TEST(IdleStateTest, ProcessStaysInIdleWhenDataIsNotReady)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    sensor.isDataReadyResult = false;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}

TEST(IdleStateTest, EnterLogsInfo)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    FakeLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());

    IdleState::getInstance().enter(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}

TEST(IdleStateTest, ExitLogsInfo)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    FakeLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());

    IdleState::getInstance().exit(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}
