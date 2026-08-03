#pragma once

#include <cstddef>
#include <vector>

#include "Measurement.h"
#include "MeasurementStore.h"

class InMemoryMeasurementStore : public MeasurementStore
{
public:
    explicit InMemoryMeasurementStore(std::size_t capacity);

    void enqueue(const Measurement& measurement) override;
    bool isEmpty() const override;
    Measurement peekOldest() const override;
    void popOldest() override;

private:
    std::size_t indexOf(std::size_t offsetFromStart) const;

    std::vector<Measurement> measurements_;
    std::size_t capacity_;
    std::size_t startIndex_ = 0;
    std::size_t size_ = 0;
};
