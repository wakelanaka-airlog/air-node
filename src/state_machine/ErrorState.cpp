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
    node.getLogger().info("Entering ErrorState");
}

void ErrorState::process(Node& node)
{
    node.getLogger().error("Node entered Error state - retrying from Init");
    node.transitionTo(InitState::getInstance());
}

void ErrorState::exit(Node& node)
{
    node.getLogger().info("Exit ErrorState");
}
