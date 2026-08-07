#include "InMemoryMeasurementStore.h"

InMemoryMeasurementStore::InMemoryMeasurementStore(std::size_t capacity)
    : measurements_(capacity), capacity_{capacity}
{
}

std::size_t InMemoryMeasurementStore::indexOf(std::size_t offsetFromStart) const
{
    return (startIndex_ + offsetFromStart) % capacity_;
}

void InMemoryMeasurementStore::enqueue(const Measurement& measurement)
{
    if (size_ == capacity_)
    {
        startIndex_ = indexOf(1);
        --size_;
    }

    measurements_[indexOf(size_)] = measurement;
    ++size_;
}

bool InMemoryMeasurementStore::isEmpty() const
{
    return size_ == 0;
}

bool InMemoryMeasurementStore::isFull() const
{
    return size_ == capacity_;
}

Measurement InMemoryMeasurementStore::peekOldest() const
{
    return measurements_[startIndex_];
}

void InMemoryMeasurementStore::popOldest()
{
    startIndex_ = indexOf(1);
    --size_;
}
