#pragma once

class Logger
{
public:
    virtual ~Logger() = default;

    virtual void error(const char* message) = 0;
};
