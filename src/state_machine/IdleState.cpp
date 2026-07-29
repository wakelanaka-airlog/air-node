#include "IdleState.h"
#include "Node.h"

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
    node.getSensor().isDataReady();
}

void IdleState::exit(Node& node)
{
}
