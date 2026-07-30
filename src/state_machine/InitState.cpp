#include "InitState.h"

#include "ErrorState.h"
#include "IdleState.h"
#include "Node.h"

InitState& InitState::getInstance()
{
    static InitState instance{};
    return instance;
}

void InitState::enter(Node& node)
{
}

void InitState::exit(Node& node)
{
}

void InitState::process(Node& node)
{
    if (node.getSensor().startPeriodicMeasurement())
    {
        node.transitionTo(IdleState::getInstance());
    }
    else
    {
        node.transitionTo(ErrorState::getInstance());
    }
}
