#include "IdleState.h"
#include "Node.h"
#include "ReadingState.h"

namespace
{
    // Per SCD41 datasheet: response time to 63% of a step change is ~120s for
    // temperature (the slowest of the three signals) after InitState resets
    // the sensor's internal signal compensation; full settling takes a few
    // time constants longer. 10 minutes is a conservative margin past that.
    constexpr uint64_t kSensorSettlingDelayMs = 10 * 60 * 1000;
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
    if (now < node.getLastRestartTime())
    {
        // Clock stepped backward (e.g. a periodic SNTP resync correction) -
        // restart the settling window instead of underflowing the
        // subtraction below into treating the sensor as already settled.
        node.setLastRestartTime(now);
        return;
    }

    if (now - node.getLastRestartTime() < kSensorSettlingDelayMs)
    {
        return;
    }

    node.transitionTo(ReadingState::getInstance());
}

void IdleState::exit(Node& node)
{
    node.getLogger().info("Exit IdleState");
}
