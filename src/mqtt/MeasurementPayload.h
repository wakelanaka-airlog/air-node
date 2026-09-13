#pragma once

#include "Measurement.h"

bool formatMeasurementPayload(const Measurement& measurement, char* buffer, std::size_t bufferSize,
                               std::size_t& outLength);
