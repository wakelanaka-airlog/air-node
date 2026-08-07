#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "ErrorState.h"
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
}

TEST(ReadingStateTest, ProcessReadsMeasurement)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    node.process();

    EXPECT_EQ(sensor.readMeasurementCount, 1);
}

TEST(ReadingStateTest, ProcessStoresMeasurementOnSuccess)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
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
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    clock.currentTime = 1738156800000ULL;

    sensor.measurementToReturn = Measurement{999, 404, 25.0f, 50.0f};
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    node.process();

    EXPECT_EQ(node.getMeasurement().timestampUnixMillis, 1738156800000ULL);
}

TEST(ReadingStateTest, ProcessUpdatesLastReadingTimeOnSuccess)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    clock.currentTime = 1738156800000ULL;

    sensor.measurementToReturn = Measurement{999, 404, 25.0f, 50.0f};
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    node.process();

    EXPECT_EQ(node.getLastReadingTime(), 1738156800000ULL);
}

TEST(ReadingStateTest, ProcessDoesNotUpdateLastReadingTimeOnFailure)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
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
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    sensor.readMeasurementResult = false;
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &ErrorState::getInstance());
}

TEST(ReadingStateTest, EnterLogsInfo)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    FakeLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    ReadingState::getInstance().enter(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}

TEST(ReadingStateTest, ExitLogsInfo)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    FakeLogger logger;
    NullMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    ReadingState::getInstance().exit(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}

TEST(ReadingStateTest, ProcessLogsMeasurementInfo)
{
    FakeSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    FakeLogger logger;
    NullMeasurementStore store;
    sensor.measurementToReturn = Measurement{1738156800000ULL, 404, 25.0f, 50.0f};
    Node node(sensor, publisher, clock, logger, store, ReadingState::getInstance());

    ReadingState::getInstance().process(node);

    const bool loggedMeasurement = std::any_of(
        logger.infoMessages.begin(), logger.infoMessages.end(),
        [](const std::string& message) { return message.find("404") != std::string::npos; });
    EXPECT_TRUE(loggedMeasurement);
}
