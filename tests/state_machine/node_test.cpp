#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "Node.h"
#include "NodeState.h"

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

    class StubPublisher : public Publisher
    {
    public:
        bool publish(const Measurement&) override
        {
            return true;
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
    StubSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubState state;
    Node node(sensor, publisher, clock, state);
    SUCCEED();
}

TEST(NodeTest, ConstructorSetsInitialState)
{
    StubSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubState state;

    Node node(sensor, publisher, clock, state);

    EXPECT_EQ(node.getState(), &state);
}

TEST(NodeTest, ConstructorSetsSensor)
{
    StubSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubState state;

    Node node(sensor, publisher, clock, state);

    EXPECT_EQ(&node.getSensor(), &sensor);
}

TEST(NodeTest, ConstructorSetsPublisher)
{
    StubSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubState state;

    Node node(sensor, publisher, clock, state);

    EXPECT_EQ(&node.getPublisher(), &publisher);
}

TEST(NodeTest, ConstructorSetsClock)
{
    StubSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubState state;

    Node node(sensor, publisher, clock, state);

    EXPECT_EQ(&node.getClock(), &clock);
}

TEST(NodeTest, ProcessDelegatesToCurrentState)
{
    StubSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    TrackingState state;

    Node node(sensor, publisher, clock, state);
    node.process();

    EXPECT_EQ(state.processCount, 1);
    EXPECT_EQ(state.lastProcessNode, &node);
}

TEST(NodeTest, TransitionToChangesCurrentState)
{
    StubSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    StubState initialState;
    Node node(sensor, publisher, clock, initialState);

    StubState newState;
    node.transitionTo(newState);

    EXPECT_EQ(node.getState(), &newState);
}

TEST(NodeTest, TransitionToCallsExitThenEnter)
{
    StubSensor sensor;
    StubPublisher publisher;
    StubClock clock;
    std::vector<std::string> log;

    TrackingState oldState(&log);
    TrackingState newState(&log);

    Node node(sensor, publisher, clock, oldState);
    node.transitionTo(newState);

    EXPECT_EQ(oldState.exitCount, 1);
    EXPECT_EQ(oldState.lastExitNode, &node);
    EXPECT_EQ(newState.enterCount, 1);
    EXPECT_EQ(newState.lastEnterNode, &node);

    ASSERT_EQ(log.size(), 2u);
    EXPECT_EQ(log[0], "exit");
    EXPECT_EQ(log[1], "enter");
}
