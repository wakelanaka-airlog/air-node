#pragma once

#include "Logger.h"

class Esp32Logger : public Logger
{
public:
    void error(const char* message) override;
};
