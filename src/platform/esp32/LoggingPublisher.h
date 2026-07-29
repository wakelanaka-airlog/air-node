#pragma once

#include "Publisher.h"

class LoggingPublisher : public Publisher
{
public:
    bool publish(const Measurement& measurement) override;
};
