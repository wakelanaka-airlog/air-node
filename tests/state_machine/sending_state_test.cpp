#include <gtest/gtest.h>

#include <deque>
#include <string>
#include <vector>

#include "ErrorState.h"
#include "IdleState.h"
#include "Node.h"
#include "SendingState.h"

namespace
{
    bool measurementsEqual(const Measurement& a, const Measurement& b)
    {
        return a.timestampUnixMillis == b.timestampUnixMillis && a.co2Ppm == b.co2Ppm &&
            a.temperatureCelsius == b.temperatureCelsius && a.humidityPercent == b.humidityPercent;
    }

    class StubSensor : public Sensor
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
            publishedMeasurements.push_back(measurement);

            bool result = publishResult;
            if (!publishResultsQueue.empty())
            {
                result = publishResultsQueue.front();
                publishResultsQueue.pop_front();
            }
            return result;
        }

        int publishCount = 0;
        bool publishResult = true;
        Measurement lastMeasurement{};
        std::vector<Measurement> publishedMeasurements;

        std::deque<bool> publishResultsQueue;
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

    class FakeMeasurementStore : public MeasurementStore
    {
    public:
        void enqueue(const Measurement& measurement) override
        {
            ++enqueueCount;
            lastEnqueuedMeasurement = measurement;
            items.push_back(measurement);
        }

        bool isEmpty() const override
        {
            return items.empty();
        }

        Measurement peekOldest() const override
        {
            return items.front();
        }

        void popOldest() override
        {
            ++popCount;
            items.pop_front();
        }

        int enqueueCount = 0;
        Measurement lastEnqueuedMeasurement{};
        int popCount = 0;
        std::deque<Measurement> items;
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
    FakeMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, SendingState::getInstance());
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
    FakeMeasurementStore store;
    publisher.publishResult = true;
    Node node(sensor, publisher, clock, logger, store, SendingState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}

TEST(SendingStateTest, ProcessTransitionsToErrorOnPublishFailure)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    StubLogger logger;
    FakeMeasurementStore store;
    publisher.publishResult = false;
    Node node(sensor, publisher, clock, logger, store, SendingState::getInstance());

    node.process();

    EXPECT_EQ(node.getState(), &ErrorState::getInstance());
}

TEST(SendingStateTest, ProcessEnqueuesMeasurementInStoreOnPublishFailure)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    StubLogger logger;
    FakeMeasurementStore store;
    publisher.publishResult = false;
    Node node(sensor, publisher, clock, logger, store, SendingState::getInstance());
    node.setMeasurement(Measurement{1738156800000ULL, 404, 25.0f, 50.0f});

    node.process();

    EXPECT_EQ(store.enqueueCount, 1);
    EXPECT_TRUE(measurementsEqual(store.lastEnqueuedMeasurement, Measurement{1738156800000ULL, 404, 25.0f, 50.0f}));
}

TEST(SendingStateTest, ProcessDoesNotEnqueueWhenPublishSucceeds)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    StubLogger logger;
    FakeMeasurementStore store;
    publisher.publishResult = true;
    Node node(sensor, publisher, clock, logger, store, SendingState::getInstance());
    node.setMeasurement(Measurement{1738156800000ULL, 404, 25.0f, 50.0f});

    node.process();

    EXPECT_EQ(store.enqueueCount, 0);
}

TEST(SendingStateTest, ProcessDrainsBacklogOnPublishSuccess)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    StubLogger logger;
    FakeMeasurementStore store;
    publisher.publishResult = true;

    const Measurement backlog1{1000, 100, 20.0f, 40.0f};
    const Measurement backlog2{2000, 200, 21.0f, 41.0f};
    store.items.push_back(backlog1);
    store.items.push_back(backlog2);

    const Measurement live{3000, 300, 22.0f, 42.0f};
    Node node(sensor, publisher, clock, logger, store, SendingState::getInstance());
    node.setMeasurement(live);

    node.process();

    ASSERT_EQ(publisher.publishedMeasurements.size(), 3u);
    EXPECT_TRUE(measurementsEqual(publisher.publishedMeasurements[0], live));
    EXPECT_TRUE(measurementsEqual(publisher.publishedMeasurements[1], backlog1));
    EXPECT_TRUE(measurementsEqual(publisher.publishedMeasurements[2], backlog2));
    EXPECT_TRUE(store.isEmpty());
    EXPECT_EQ(store.popCount, 2);
    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}

TEST(SendingStateTest, ProcessStopsDrainingBacklogOnFirstDrainFailure)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    StubLogger logger;
    FakeMeasurementStore store;

    const Measurement backlog1{1000, 100, 20.0f, 40.0f};
    const Measurement backlog2{2000, 200, 21.0f, 41.0f};
    store.items.push_back(backlog1);
    store.items.push_back(backlog2);

    // Live publish succeeds, but the first backlog publish fails.
    publisher.publishResultsQueue = {true, false};

    const Measurement live{3000, 300, 22.0f, 42.0f};
    Node node(sensor, publisher, clock, logger, store, SendingState::getInstance());
    node.setMeasurement(live);

    node.process();

    ASSERT_EQ(publisher.publishedMeasurements.size(), 2u);
    EXPECT_TRUE(measurementsEqual(publisher.publishedMeasurements[0], live));
    EXPECT_TRUE(measurementsEqual(publisher.publishedMeasurements[1], backlog1));

    ASSERT_EQ(store.items.size(), 2u);
    EXPECT_TRUE(measurementsEqual(store.items[0], backlog1));
    EXPECT_TRUE(measurementsEqual(store.items[1], backlog2));
    EXPECT_EQ(store.popCount, 0);
    EXPECT_EQ(node.getState(), &IdleState::getInstance());
}

TEST(SendingStateTest, ProcessDoesNotDrainBacklogWhenLivePublishFails)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    StubLogger logger;
    FakeMeasurementStore store;

    const Measurement backlog1{1000, 100, 20.0f, 40.0f};
    store.items.push_back(backlog1);

    publisher.publishResult = false;

    const Measurement live{3000, 300, 22.0f, 42.0f};
    Node node(sensor, publisher, clock, logger, store, SendingState::getInstance());
    node.setMeasurement(live);

    node.process();

    EXPECT_EQ(publisher.publishCount, 1);

    ASSERT_EQ(store.items.size(), 2u);
    EXPECT_TRUE(measurementsEqual(store.items[0], backlog1));
    EXPECT_TRUE(measurementsEqual(store.items[1], live));
}

TEST(SendingStateTest, EnterLogsInfo)
{
    StubSensor sensor;
    FakePublisher publisher;
    StubClock clock;
    FakeLogger logger;
    FakeMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, SendingState::getInstance());

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
    FakeMeasurementStore store;
    Node node(sensor, publisher, clock, logger, store, SendingState::getInstance());

    SendingState::getInstance().exit(node);

    EXPECT_EQ(logger.infoCount, 1);
    EXPECT_FALSE(logger.lastInfoMessage.empty());
}
