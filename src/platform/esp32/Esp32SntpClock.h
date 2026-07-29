#pragma once

#include "Clock.h"

class Esp32SntpClock : public

Clock
{
public:
    explicit Esp32SntpClock(const char* ntpServer);

    Esp32SntpClock(const Esp32SntpClock &) = delete;
    Esp32SntpClock & operator = (const Esp32SntpClock &
    )
    =
    delete;

    uint64_t nowUnixMillis() override;
};
