#include "Node.h"

Node::Node(Sensor& sensor, Publisher& publisher, Clock& clock, Logger& logger, MeasurementStore& store,
           NodeState& initialState, float temperatureOffsetCelsius) : sensor_{sensor},
                                      publisher_{publisher},
                                      clock_{clock}, logger_{logger},
                                      state_{&initialState},
                                      store_{store},
                                      temperatureOffsetCelsius_{temperatureOffsetCelsius}
{
}

NodeState* Node::getState() const
{
    return state_;
}

void Node::process()
{
    state_->process(*this);
}

void Node::transitionTo(NodeState& state)
{
    state_->exit(*this);
    state_ = &state;
    state_->enter(*this);
}

Sensor& Node::getSensor() const
{
    return sensor_;
}

Publisher& Node::getPublisher() const
{
    return publisher_;
}

Clock& Node::getClock() const
{
    return clock_;
}

Logger& Node::getLogger() const
{
    return logger_;
}

const Measurement& Node::getMeasurement() const
{
    return measurement_;
}

void Node::setMeasurement(const Measurement& measurement)
{
    measurement_ = measurement;
}

MeasurementStore& Node::getMeasurementStore() const
{
    return store_;
}

void Node::setLastRestartTime(uint64_t timestampUnixMillis)
{
    lastRestartTimeUnixMillis_ = timestampUnixMillis;
}

uint64_t Node::getLastRestartTime() const
{
    return lastRestartTimeUnixMillis_;
}

float Node::getTemperatureOffsetCelsius() const
{
    return temperatureOffsetCelsius_;
}
