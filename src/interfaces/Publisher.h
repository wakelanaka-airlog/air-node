#pragma once

#include "Measurement.h"

class Publisher
{
public:
    virtual ~Publisher() = default;

    virtual bool publish(const Measurement& measurement) = 0;
};
