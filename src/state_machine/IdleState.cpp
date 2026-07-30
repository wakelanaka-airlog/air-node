#include "IdleState.h"
#include "Node.h"
#include "ReadingState.h"

IdleState& IdleState::getInstance()
{
    static IdleState instance{};
    return instance;
}

void IdleState::enter(Node& node)
{
}

void IdleState::process(Node& node)
{
    if (node.getSensor().isDataReady())
    {
        node.transitionTo(ReadingState::getInstance());
    }
}

void IdleState::exit(Node& node)
{
}
