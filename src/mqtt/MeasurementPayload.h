#pragma once

#include <cstddef>

#include "Measurement.h"

bool formatMeasurementPayload(const char* room, const Measurement& measurement, char* buffer,
                               std::size_t bufferSize, std::size_t& outLength);
