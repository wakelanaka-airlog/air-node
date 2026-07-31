#include "FilteringLogger.h"

FilteringLogger::FilteringLogger(Logger& target, const bool isDevelopmentMode) : target_{target},
    isDevelopmentMode_{isDevelopmentMode}
{
}

void FilteringLogger::info(const char* message)
{
    if (isDevelopmentMode_)
    {
        target_.info(message);
    }
}

void FilteringLogger::error(const char* message)
{
    target_.error(message);
}
