#include "ReadingState.h"

#include "ErrorState.h"
#include "Node.h"
#include "SendingState.h"

ReadingState& ReadingState::getInstance()
{
    static ReadingState instance;
    return instance;
}


void ReadingState::enter(Node& node)
{
    node.getLogger().info("Entering ReadingState");
}

void ReadingState::process(Node& node)
{
    Measurement measurement{};
    if (!node.getSensor().readMeasurement(measurement))
    {
        node.transitionTo(ErrorState::getInstance());
        return;
    }

    measurement.timestampUnixMillis = node.getClock().nowUnixMillis();
    node.setMeasurement(measurement);
    node.transitionTo(SendingState::getInstance());
}

void ReadingState::exit(Node& node)
{
    node.getLogger().info("Exit ReadingState");
}
