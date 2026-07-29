#pragma once

#include "Delay.h"

class Esp32Delay : public Delay
{
public:
    void delayMs(uint32_t milliseconds) override;
};
