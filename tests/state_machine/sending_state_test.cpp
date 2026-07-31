#include <gtest/gtest.h>

#include <string>

#include "ErrorState.h"
#include "IdleState.h"
#include "Node.h"
#include "SendingState.h"

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

    class FakePublisher : public Publisher
    {
    public:
        bool publish(const Measurement& measurement) override
        {
            ++publishCount;
            lastMeasurement = measurement;
            return publishResult;
        }

        int publishCount = 0;
        bool publishResult = true;
        Measurement lastMeasurement{};
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

    class StubClock : public Clock
    {
    public:
        uint64_t nowUnixMillis() override
        {
            return 0;
        }
    };
}

TEST(SendingStateTest, ProcessPublishesMeasurement)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    StubLogger logger;
    Node node(sensor, publisher, clock, logger, SendingState::getInstance());
    node.setMeasurement(Measurement{1738156800000ULL, 404, 25.0f, 50.0f});

    node.process();

    EXPECT_EQ(publisher.publishCount, 1);
    EXPECT_EQ(publisher.lastMeasurement.timestampUnixMillis, 1738156800000ULL);
    EXPECT_EQ(publisher.lastMeasurement.co2Ppm, 404);
    EXPECT_EQ(publisher.lastMeasurement.temperatureCelsius, 25.0f);
    EXPECT_EQ(publisher.lastMeasurement.humidityPercent, 50.0f);
}

TEST(SendingStateTest, ProcessTransitionsToIdleOnPublishSuccess)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    StubLogger logger;
    publisher.publishResult = true;
    Node node(sensor, publisher, clock, logger, SendingState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}

TEST(SendingStateTest, ProcessTransitionsToErrorOnPublishFailure)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    StubLogger logger;
    publisher.publishResult = false;
    Node node(sensor, publisher, clock, logger, SendingState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &ErrorState::getInstance());
}

TEST(SendingStateTest, EnterLogsInfo)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    FakeLogger logger;
    Node node(sensor, publisher, clock, logger, SendingState::getInstance());

    SendingState::getInstance().enter(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}

TEST(SendingStateTest, ExitLogsInfo)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    FakeLogger logger;
    Node node(sensor, publisher, clock, logger, SendingState::getInstance());

    SendingState::getInstance().exit(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}
