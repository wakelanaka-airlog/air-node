#pragma once

#include <cstdint>

class Clock
{
public:
    virtual ~Clock() = default;

    virtual uint64_t nowUnixMillis() = 0;
};
