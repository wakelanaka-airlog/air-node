#include "mqtt/AirNodeTopic.h"

namespace
{
    constexpr const char* kTopicPrefix = "wakelanaka-airlog/air-node/";
}

std::string AirNodeTopic::measurementTopic(const std::string& room)
{
    return kTopicPrefix + room + "/measurement";
}

std::string AirNodeTopic::statusTopic(const std::string& room)
{
    return kTopicPrefix + room + "/status";
}
