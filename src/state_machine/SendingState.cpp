#include "SendingState.h"

#include "ErrorState.h"
#include "IdleState.h"
#include "Node.h"

SendingState& SendingState::getInstance()
{
    static SendingState instance;
    return instance;
}

void SendingState::enter(Node& node)
{
    node.getLogger().info("Entering SendingState");
}

void SendingState::process(Node& node)
{
    if (!node.getPublisher().publish(node.getMeasurement()))
    {
        if (node.getMeasurementStore().isFull())
        {
            node.getLogger().error("Measurement store is full - oldest backlogged reading will be overwritten");
        }
        node.getMeasurementStore().enqueue(node.getMeasurement());
        node.transitionTo(ErrorState::getInstance());
        return;
    }

    while (!node.getMeasurementStore().isEmpty())
    {
        if (!node.getPublisher().publish(node.getMeasurementStore().peekOldest()))
        {
            node.getLogger().error("Failed to drain backlog - will retry next cycle");
            break;
        }
        node.getMeasurementStore().popOldest();
    }
    node.transitionTo(IdleState::getInstance());
}

void SendingState::exit(Node& node)
{
    node.getLogger().info("Exit SendingState");
}
