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

    class StubMeasurementStore : public MeasurementStore
    {
    public:
        void enqueue(const Measurement&) override
        {
        }

        bool isEmpty() const override
        {
            return true;
        }

        Measurement peekOldest() const override
        {
            return Measurement{};
        }

        void popOldest() override
        {
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

    class FakeClock : public Clock
    {
    public:
        uint64_t nowUnixMillis() override
        {
            return currentTime;
        }

        uint64_t currentTime = 0;
    };
}

TEST(IdleStateTest, ProcessChecksDataReady)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubLogger logger;
    StubMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());

    node.process();

    EXPECT_EQ(sensor.isDataReadyCount, 1);
}

TEST(IdleStateTest, ProcessTransitionsToReadingWhenDataIsReadyOnFirstEverCheck)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    clock.currentTime = 2000000000000ULL;
    StubLogger logger;
    StubMeasurementStore store;
    sensor.isDataReadyResult = true;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &ReadingState::getInstance());
}

TEST(IdleStateTest, ProcessStaysInIdleWhenReadingIntervalHasNotElapsed)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    clock.currentTime = 2000000000000ULL;
    StubLogger logger;
    StubMeasurementStore store;
    sensor.isDataReadyResult = true;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());
    node.setLastReadingTime(clock.currentTime - 600000ULL); // 10 minutes ago

    node.process();

    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}

TEST(IdleStateTest, ProcessTransitionsToReadingWhenReadingIntervalHasElapsed)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    clock.currentTime = 2000000000000ULL;
    StubLogger logger;
    StubMeasurementStore store;
    sensor.isDataReadyResult = true;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());
    node.setLastReadingTime(clock.currentTime - 1000000ULL); // ~16.7 minutes ago

    node.process();

    EXPECT_EQ(node.getState(), &ReadingState::getInstance());
}

TEST(IdleStateTest, ProcessStaysInIdleWhenDataIsNotReady)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubLogger logger;
    StubMeasurementStore store;
    sensor.isDataReadyResult = false;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}

TEST(IdleStateTest, EnterLogsInfo)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    FakeLogger logger;
    StubMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());

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
    StubMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, IdleState::getInstance());

    IdleState::getInstance().exit(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}
