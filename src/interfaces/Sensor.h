#pragma once

#include "Measurement.h"

class Sensor
{
public:
    virtual ~Sensor() = default;

    virtual bool startPeriodicMeasurement() = 0;
    virtual bool isDataReady() = 0;
    virtual bool readMeasurement(Measurement& measurement) = 0;
};
