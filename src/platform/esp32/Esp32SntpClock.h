#pragma once

#include "Clock.h"
#include "Logger.h"

class Esp32SntpClock : public Clock
{
public:
    Esp32SntpClock(const char* ntpServer, Logger& logger);

    Esp32SntpClock(const Esp32SntpClock&) = delete;
    Esp32SntpClock& operator=(const Esp32SntpClock&) = delete;

    uint64_t nowUnixMillis() override;
};
