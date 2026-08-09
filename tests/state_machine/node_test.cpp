#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "Node.h"
#include "NodeState.h"
#include "support/NullDoubles.h"

namespace
{
    class StubState : public NodeState
    {
    public:
        void enter(Node&) override
        {
        }

        void process(Node&) override
        {
        }

        void exit(Node&) override
        {
        }
    };

    class TrackingState : public NodeState
    {
    public:
        explicit TrackingState(std::vector<std::string>* log = nullptr) : log_(log)
        {
        }

        void enter(Node& node) override
        {
            ++enterCount;
            lastEnterNode = &node;
            if (log_)
            {
                log_->push_back("enter");
            }
        }

        void process(Node& node) override
        {
            ++processCount;
            lastProcessNode = &node;
            if (log_)
            {
                log_->push_back("process");
            }
        }

        void exit(Node& node) override
        {
            ++exitCount;
            lastExitNode = &node;
            if (log_)
            {
                log_->push_back("exit");
            }
        }

        int enterCount = 0;
        int processCount = 0;
        int exitCount = 0;
        Node* lastEnterNode = nullptr;
        Node* lastProcessNode = nullptr;
        Node* lastExitNode = nullptr;

    private:
        std::vector<std::string>* log_;
    };
}

TEST(NodeTest, CanBeInstantiated)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    StubState state;
    Node node(sensor, publisher, clock, logger, store, state);
    SUCCEED();
}

TEST(NodeTest, ConstructorSetsInitialState)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    StubState state;

    Node node(sensor, publisher, clock, logger, store, state);

    EXPECT_EQ(node.getState(), &state);
}

TEST(NodeTest, ConstructorSetsSensor)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    StubState state;

    Node node(sensor, publisher, clock, logger, store, state);

    EXPECT_EQ(&node.getSensor(), &sensor);
}

TEST(NodeTest, ConstructorSetsPublisher)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    StubState state;

    Node node(sensor, publisher, clock, logger, store, state);

    EXPECT_EQ(&node.getPublisher(), &publisher);
}

TEST(NodeTest, ConstructorSetsClock)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    StubState state;

    Node node(sensor, publisher, clock, logger, store, state);

    EXPECT_EQ(&node.getClock(), &clock);
}

TEST(NodeTest, ConstructorSetsMeasurementStore)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    StubState state;

    Node node(sensor, publisher, clock, logger, store, state);

    EXPECT_EQ(&node.getMeasurementStore(), &store);
}

TEST(NodeTest, ConstructorDefaultsTemperatureOffsetToScd41FactoryDefault)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    StubState state;

    Node node(sensor, publisher, clock, logger, store, state);

    EXPECT_FLOAT_EQ(node.getTemperatureOffsetCelsius(), 4.f);
}

TEST(NodeTest, ConstructorSetsTemperatureOffsetWhenProvided)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    StubState state;

    Node node(sensor, publisher, clock, logger, store, state, 0.f);

    EXPECT_FLOAT_EQ(node.getTemperatureOffsetCelsius(), 0.f);
}

TEST(NodeTest, ProcessDelegatesToCurrentState)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    TrackingState state;

    Node node(sensor, publisher, clock, logger, store, state);
    node.process();

    EXPECT_EQ(state.processCount, 1);
    EXPECT_EQ(state.lastProcessNode, &node);
}

TEST(NodeTest, TransitionToChangesCurrentState)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    StubState initialState;
    Node node(sensor, publisher, clock, logger, store, initialState);

    StubState newState;
    node.transitionTo(newState);

    EXPECT_EQ(node.getState(), &newState);
}

TEST(NodeTest, TransitionToCallsExitThenEnter)
{
    NullSensor sensor;
    NullPublisher publisher;
    FixedClock clock;
    NullLogger logger;
    NullMeasurementStore store;
    std::vector<std::string> log;

    TrackingState oldState(&log);
    TrackingState newState(&log);

    Node node(sensor, publisher, clock, logger, store, oldState);
    node.transitionTo(newState);

    EXPECT_EQ(oldState.exitCount, 1);
    EXPECT_EQ(oldState.lastExitNode, &node);
    EXPECT_EQ(newState.enterCount, 1);
    EXPECT_EQ(newState.lastEnterNode, &node);

    ASSERT_EQ(log.size(), 2u);
    EXPECT_EQ(log[0], "exit");
    EXPECT_EQ(log[1], "enter");
}
