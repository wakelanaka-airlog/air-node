#pragma once

// Shared no-op/fixed-value test doubles for the ports in src/interfaces/.
// Used by tests that need a working Node collaborator but don't care about
// its behavior; tests that assert on a collaborator's behavior (e.g. a
// logger recording messages, a sensor with configurable results) define
// their own tailored fake locally instead of using these.

#include "Clock.h"
#include "Logger.h"
#include "Measurement.h"
#include "MeasurementStore.h"
#include "Publisher.h"
#include "Sensor.h"

namespace
{
    inline bool measurementsEqual(const Measurement& a, const Measurement& b)
    {
        return a.timestampUnixMillis == b.timestampUnixMillis && a.co2Ppm == b.co2Ppm &&
            a.temperatureCelsius == b.temperatureCelsius && a.humidityPercent == b.humidityPercent;
    }

    class NullLogger : public Logger
    {
    public:
        void info(const char*) override
        {
        }

        void error(const char*) override
        {
        }
    };

    class NullPublisher : public Publisher
    {
    public:
        bool publish(const Measurement&) override
        {
            return true;
        }
    };

    class NullSensor : public Sensor
    {
    public:
        bool startPeriodicMeasurement() override
        {
            return true;
        }

        bool stopPeriodicMeasurement() override
        {
            return true;
        }

        bool isDataReady() override
        {
            return false;
        }

        bool readMeasurement(Measurement&) override
        {
            return false;
        }

        bool setTemperatureOffset(float) override
        {
            return true;
        }
    };

    class NullMeasurementStore : public MeasurementStore
    {
    public:
        void enqueue(const Measurement&) override
        {
        }

        bool isEmpty() const override
        {
            return true;
        }

        bool isFull() const override
        {
            return false;
        }

        Measurement peekOldest() const override
        {
            return Measurement{};
        }

        void popOldest() override
        {
        }
    };

    // Defaults to always returning 0 (replaces the old fixed-at-zero
    // StubClock); set currentTime for tests that need to control "now"
    // (replaces the old settable FakeClock) - both were the same shape.
    class FixedClock : public Clock
    {
    public:
        uint64_t nowUnixMillis() override
        {
            return currentTime;
        }

        uint64_t currentTime = 0;
    };
}
