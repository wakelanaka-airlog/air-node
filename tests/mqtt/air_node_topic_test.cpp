#include <gtest/gtest.h>

#include "mqtt/AirNodeTopic.h"

TEST(AirNodeTopicTest, BuildsMeasurementTopicForRoom)
{
    EXPECT_EQ(AirNodeTopic::measurementTopic("living_room"), "wakelanaka-airlog/air-node/living_room/measurement");
}

TEST(AirNodeTopicTest, BuildsStatusTopicForRoom)
{
    EXPECT_EQ(AirNodeTopic::statusTopic("living_room"), "wakelanaka-airlog/air-node/living_room/status");
}

TEST(AirNodeTopicTest, DifferentRoomsProduceDifferentMeasurementTopics)
{
    EXPECT_NE(AirNodeTopic::measurementTopic("bedroom"), AirNodeTopic::measurementTopic("living_room"));
}

TEST(AirNodeTopicTest, DifferentRoomsProduceDifferentStatusTopics)
{
    EXPECT_NE(AirNodeTopic::statusTopic("bedroom"), AirNodeTopic::statusTopic("living_room"));
}

TEST(AirNodeTopicTest, MeasurementAndStatusTopicsForSameRoomDiffer)
{
    EXPECT_NE(AirNodeTopic::measurementTopic("living_room"), AirNodeTopic::statusTopic("living_room"));
}
