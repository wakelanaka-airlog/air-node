#pragma once

#include <cstdint>

class Delay
{
public:
    virtual ~Delay() = default;

    virtual void delayMs(uint32_t milliseconds) = 0;
};
