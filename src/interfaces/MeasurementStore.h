#pragma once

#include "Measurement.h"

struct MeasurementStore
{
    virtual ~MeasurementStore() = default;

    virtual void enqueue(const Measurement& measurement) = 0;
    virtual bool isEmpty() const = 0;
    virtual bool isFull() const = 0;
    virtual Measurement peekOldest() const = 0;
    virtual void popOldest() = 0;
};
