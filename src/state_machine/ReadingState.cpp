#include "ReadingState.h"

#include "ErrorState.h"
#include "Node.h"
#include "SendingState.h"

#include <cstdio>

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
    node.setLastReadingTime(measurement.timestampUnixMillis);
    char loggingInfo[128];
    std::snprintf(loggingInfo, sizeof(loggingInfo),
                  "Measurement: { timestampUnixMillis: %llu, co2Ppm: %u, temperatureCelsius: %.2f, humidityPercent: %.2f}",
                  static_cast<unsigned long long>(measurement.timestampUnixMillis), measurement.co2Ppm,
                  measurement.temperatureCelsius, measurement.humidityPercent);
    node.getLogger().info(loggingInfo);
    node.transitionTo(SendingState::getInstance());
}

void ReadingState::exit(Node& node)
{
    node.getLogger().info("Exit ReadingState");
}
