#pragma once

#include <string>

namespace AirNodeTopic
{
    std::string measurementTopic(const std::string& room);

    std::string statusTopic(const std::string& room);
}
