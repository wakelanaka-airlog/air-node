#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "ErrorState.h"
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
            return false;
        }

        bool readMeasurement(Measurement& measurement) override
        {
            ++readMeasurementCount;
            measurement = measurementToReturn;
            return readMeasurementResult;
        }

        int readMeasurementCount = 0;
        bool readMeasurementResult = true;
        Measurement measurementToReturn{};
    };

    class StubPublisher : public Publisher
    {
    public:
        bool publish(const Measurement&) override
        {
            return true;
        }
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
            infoMessages.push_back(lastInfoMessage);
        }

        void error(const char*) override
        {
        }

        int infoCount = 0;
        std::string lastInfoMessage;
        std::vector<std::string> infoMessages;
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

TEST(ReadingStateTest, ProcessReadsMeasurement)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    StubLogger logger;
    StubMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    node.process();

    EXPECT_EQ(sensor.readMeasurementCount, 1);
}

TEST(ReadingStateTest, ProcessStoresMeasurementOnSuccess)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    StubLogger logger;
    StubMeasurementStore store;
    sensor.measurementToReturn = Measurement{0, 404, 25.0f, 50.0f};
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    node.process();

    EXPECT_EQ(node.getMeasurement().co2Ppm, 404);
    EXPECT_EQ(node.getMeasurement().temperatureCelsius, 25.0f);
    EXPECT_EQ(node.getMeasurement().humidityPercent, 50.0f);
}

TEST(ReadingStateTest, ProcessStampsMeasurementWithCurrentTime)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    StubLogger logger;
    StubMeasurementStore store;
    clock.currentTime = 1738156800000ULL;

    sensor.measurementToReturn = Measurement{999, 404, 25.0f, 50.0f};
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    node.process();

    EXPECT_EQ(node.getMeasurement().timestampUnixMillis, 1738156800000ULL);
}

TEST(ReadingStateTest, ProcessUpdatesLastReadingTimeOnSuccess)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    StubLogger logger;
    StubMeasurementStore store;
    clock.currentTime = 1738156800000ULL;

    sensor.measurementToReturn = Measurement{999, 404, 25.0f, 50.0f};
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    node.process();

    EXPECT_EQ(node.getLastReadingTime(), 1738156800000ULL);
}

TEST(ReadingStateTest, ProcessDoesNotUpdateLastReadingTimeOnFailure)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    StubLogger logger;
    StubMeasurementStore store;
    clock.currentTime = 1738156800000ULL;
    sensor.readMeasurementResult = false;
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());
    node.setLastReadingTime(123);

    node.process();

    EXPECT_EQ(node.getLastReadingTime(), 123ULL);
}

TEST(ReadingStateTest, ProcessTransitionsToErrorOnReadFailure)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    StubLogger logger;
    StubMeasurementStore store;
    sensor.readMeasurementResult = false;
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &ErrorState::getInstance());
}

TEST(ReadingStateTest, EnterLogsInfo)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    FakeLogger logger;
    StubMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    ReadingState::getInstance().enter(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}

TEST(ReadingStateTest, ExitLogsInfo)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    FakeLogger logger;
    StubMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    ReadingState::getInstance().exit(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}

TEST(ReadingStateTest, ProcessLogsMeasurementInfo)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    FakeLogger logger;
    StubMeasurementStore store;
    sensor.measurementToReturn = Measurement{1738156800000ULL, 404, 25.0f, 50.0f};
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    ReadingState::getInstance().process(node);

    const bool loggedMeasurement = std::any_of(
        logger.infoMessages.begin(), logger.infoMessages.end(),
        [](const std::string& message) { return message.find("404") != std::string::npos; });
    EXPECT_TRUE(loggedMeasurement);
}
