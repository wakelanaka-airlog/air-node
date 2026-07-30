#include "Node.h"

Node::Node(Sensor& sensor, Publisher& publisher, Clock& clock, Logger& logger, NodeState& initialState) : sensor_{sensor},
    publisher_{publisher},
    clock_{clock}, logger_{logger}, state_{&initialState}
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
