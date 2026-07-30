#include <gtest/gtest.h>

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
        void error(const char*) override
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
    Node node(sensor, publisher, clock, logger, ReadingState::getInstance());

    node.process();

    EXPECT_EQ(sensor.readMeasurementCount, 1);
}

TEST(ReadingStateTest, ProcessStoresMeasurementOnSuccess)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    StubLogger logger;
    sensor.measurementToReturn = Measurement{0, 404, 25.0f, 50.0f};
    Node node(sensor, publisher, clock, logger, ReadingState::getInstance());

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
    clock.currentTime = 1738156800000ULL;

    sensor.measurementToReturn = Measurement{999, 404, 25.0f, 50.0f};
    Node node(sensor, publisher, clock, logger, ReadingState::getInstance());

    node.process();

    EXPECT_EQ(node.getMeasurement().timestampUnixMillis, 1738156800000ULL);
}

TEST(ReadingStateTest, ProcessTransitionsToErrorOnReadFailure)
{
    FakeSensor sensor;
    StubPublisher publisher;
    FakeClock clock;
    StubLogger logger;
    sensor.readMeasurementResult = false;
    Node node(sensor, publisher, clock, logger, ReadingState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &ErrorState::getInstance());
}
