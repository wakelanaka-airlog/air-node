#include <gtest/gtest.h>

#include <string>
#include <vector>

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

TEST(InitStateTest, ProcessStartsPeriodicMeasurement)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubLogger logger;
    StubMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    node.process();

    EXPECT_EQ(sensor.startPeriodicMeasurementCount, 1);
}

TEST(InitStateTest, ProcessStopsPeriodicMeasurement)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubLogger logger;
    StubMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    node.process();

    EXPECT_EQ(sensor.stopPeriodicMeasurementCount, 1);
}

TEST(InitStateTest, ProcessStopsBeforeStarting)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubLogger logger;
    StubMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    node.process();

    ASSERT_EQ(sensor.callOrder.size(), 2u);
    EXPECT_EQ(sensor.callOrder[0], "stop");
    EXPECT_EQ(sensor.callOrder[1], "start");
}

TEST(InitStateTest, ProcessTransitionsToIdleOnSuccess)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubLogger logger;
    StubMeasurementStore store;
    sensor.startPeriodicMeasurementResult = true;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}

TEST(InitStateTest, ProcessSeedsLastReadingTimeOnSuccess)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    clock.currentTime = 1738156800000ULL;
    StubLogger logger;
    StubMeasurementStore store;
    sensor.startPeriodicMeasurementResult = true;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    node.process();

    EXPECT_EQ(node.getLastReadingTime(), 1738156800000ULL);
}

TEST(InitStateTest, ProcessDoesNotSeedLastReadingTimeOnFailure)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    clock.currentTime = 1738156800000ULL;
    StubLogger logger;
    StubMeasurementStore store;
    sensor.startPeriodicMeasurementResult = false;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());
    node.setLastReadingTime(123);

    node.process();

    EXPECT_EQ(node.getLastReadingTime(), 123ULL);
}

TEST(InitStateTest, ProcessTransitionsToErrorOnFailure)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubLogger logger;
    StubMeasurementStore store;
    sensor.startPeriodicMeasurementResult = false;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &ErrorState::getInstance());
}

TEST(InitStateTest, EnterLogsInfo)
{
    FakeSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    FakeLogger logger;
    StubMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

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
    StubMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, InitState::getInstance());

    InitState::getInstance().exit(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}
