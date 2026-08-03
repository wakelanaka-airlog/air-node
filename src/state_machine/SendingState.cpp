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
        node.getMeasurementStore().enqueue(node.getMeasurement());
        node.transitionTo(ErrorState::getInstance());
        return;
    }

    while (!node.getMeasurementStore().isEmpty())
    {
        if (!node.getPublisher().publish(node.getMeasurementStore().peekOldest()))
        {
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
