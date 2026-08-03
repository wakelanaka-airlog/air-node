#pragma once

#include "Clock.h"
#include "Logger.h"
#include "Measurement.h"
#include "NodeState.h"
#include "Publisher.h"
#include "Sensor.h"
#include "MeasurementStore.h"

class Node
{
public:
    Node(Sensor& sensor, Publisher& publisher, Clock& clock, Logger& logger, MeasurementStore& store,
         NodeState& initialState);

    void process();
    void transitionTo(NodeState& state);
    [[nodiscard]] NodeState* getState() const;
    [[nodiscard]] Sensor& getSensor() const;
    [[nodiscard]] Publisher& getPublisher() const;
    [[nodiscard]] Clock& getClock() const;
    [[nodiscard]] Logger& getLogger() const;
    void setMeasurement(const Measurement& measurement);
    [[nodiscard]] const Measurement& getMeasurement() const;
    [[nodiscard]] MeasurementStore& getMeasurementStore() const;
    void setLastReadingTime(uint64_t timestampUnixMillis);
    [[nodiscard]] uint64_t getLastReadingTime() const;

private:
    Sensor& sensor_;
    Publisher& publisher_;
    Clock& clock_;
    Logger& logger_;
    NodeState* state_;
    MeasurementStore& store_;
    Measurement measurement_;
    uint64_t lastReadingTimeUnixMillis_ = 0;
};
