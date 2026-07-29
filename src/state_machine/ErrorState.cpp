#include "ErrorState.h"

#include "InitState.h"
#include "Node.h"

ErrorState& ErrorState::getInstance()
{
    static ErrorState instance;
    return instance;
}

void ErrorState::enter(Node& node)
{
}

void ErrorState::process(Node& node)
{
    node.transitionTo(InitState::getInstance());
}

void ErrorState::exit(Node& node)
{
}
