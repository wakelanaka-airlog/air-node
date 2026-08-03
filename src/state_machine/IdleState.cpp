#include "IdleState.h"
#include "Node.h"
#include "ReadingState.h"

namespace
{
    constexpr uint64_t kReadingIntervalMs = 15 * 60 * 1000;
}

IdleState& IdleState::getInstance()
{
    static IdleState instance{};
    return instance;
}

void IdleState::enter(Node& node)
{
    node.getLogger().info("Entering IdleState");
}

void IdleState::process(Node& node)
{
    if (!node.getSensor().isDataReady())
    {
        return;
    }

    const uint64_t now = node.getClock().nowUnixMillis();
    if (now - node.getLastReadingTime() < kReadingIntervalMs)
    {
        return;
    }

    node.transitionTo(ReadingState::getInstance());
}

void IdleState::exit(Node& node)
{
    node.getLogger().info("Exit IdleState");
}
