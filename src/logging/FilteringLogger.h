#pragma once
#include "Logger.h"

class FilteringLogger : public Logger
{
public:
    FilteringLogger(Logger& target, bool isDevelopmentMode);

    void info(const char* message) override;
    void error(const char* message) override;

private:
    Logger& target_;
    bool isDevelopmentMode_;
};
