#pragma once

#include "Clock.h"
#include "Measurement.h"
#include "NodeState.h"
#include "Publisher.h"
#include "Sensor.h"

class Node
{
public:
    Node(Sensor& sensor, Publisher& publisher, Clock& clock, NodeState& initialState);

    void process();
    void transitionTo(NodeState& state);
    [[nodiscard]] NodeState* getState() const;
    [[nodiscard]] Sensor& getSensor() const;
    [[nodiscard]] Publisher& getPublisher() const;
    [[nodiscard]] Clock& getClock() const;
    void setMeasurement(const Measurement& measurement);
    [[nodiscard]] const Measurement& getMeasurement() const;

private:
    Sensor& sensor_;
    Publisher& publisher_;
    Clock& clock_;
    NodeState* state_{};
    Measurement measurement_{};
};
