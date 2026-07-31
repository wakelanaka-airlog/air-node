#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "logging/FilteringLogger.h"

namespace
{
    class RecordingLogger : public Logger
    {
    public:
        void info(const char* message) override
        {
            infoMessages.emplace_back(message ? message : "");
        }

        void error(const char* message) override
        {
            errorMessages.emplace_back(message ? message : "");
        }

        std::vector<std::string> infoMessages;
        std::vector<std::string> errorMessages;
    };
}

TEST(FilteringLoggerTest, DevelopmentModeForwardsInfo)
{
    RecordingLogger target;
    FilteringLogger logger(target, true);

    logger.info("entering Idle");

    ASSERT_EQ(target.infoMessages.size(), 1u);
    EXPECT_EQ(target.infoMessages[0], "entering Idle");
}

TEST(FilteringLoggerTest, DevelopmentModeForwardsError)
{
    RecordingLogger target;
    FilteringLogger logger(target, true);

    logger.error("something broke");

    ASSERT_EQ(target.errorMessages.size(), 1u);
    EXPECT_EQ(target.errorMessages[0], "something broke");
}

TEST(FilteringLoggerTest, ProductionModeSuppressesInfo)
{
    RecordingLogger target;
    FilteringLogger logger(target, false);

    logger.info("entering Idle");

    EXPECT_TRUE(target.infoMessages.empty());
}

TEST(FilteringLoggerTest, ProductionModeStillForwardsError)
{
    RecordingLogger target;
    FilteringLogger logger(target, false);

    logger.error("something broke");

    ASSERT_EQ(target.errorMessages.size(), 1u);
    EXPECT_EQ(target.errorMessages[0], "something broke");
}

TEST(FilteringLoggerTest, ProductionModeSuppressesMultipleInfoCallsButKeepsErrors)
{
    RecordingLogger target;
    FilteringLogger logger(target, false);

    logger.info("entering Idle");
    logger.error("read failed");
    logger.info("leaving Idle");

    EXPECT_TRUE(target.infoMessages.empty());
    ASSERT_EQ(target.errorMessages.size(), 1u);
    EXPECT_EQ(target.errorMessages[0], "read failed");
}
