#pragma once

class Logger
{
public:
    virtual ~Logger() = default;

    virtual void error(const char* message) = 0;

    virtual void info(const char* message) = 0;
};
