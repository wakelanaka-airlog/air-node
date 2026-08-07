#include <gtest/gtest.h>

#include "storage/InMemoryMeasurementStore.h"
#include "support/NullDoubles.h"

TEST(InMemoryMeasurementStoreTest, NewStoreIsEmpty)
{
    InMemoryMeasurementStore store(4);

    EXPECT_TRUE(store.isEmpty());
}

TEST(InMemoryMeasurementStoreTest, EnqueueMakesStoreNonEmpty)
{
    InMemoryMeasurementStore store(4);

    store.enqueue(Measurement{1000, 100, 20.0f, 40.0f});

    EXPECT_FALSE(store.isEmpty());
}

TEST(InMemoryMeasurementStoreTest, PeekOldestReturnsFirstEnqueuedMeasurement)
{
    InMemoryMeasurementStore store(4);
    const Measurement first{1000, 100, 20.0f, 40.0f};
    store.enqueue(first);

    EXPECT_TRUE(measurementsEqual(store.peekOldest(), first));
}

TEST(InMemoryMeasurementStoreTest, PeekOldestDoesNotRemoveTheItem)
{
    InMemoryMeasurementStore store(4);
    store.enqueue(Measurement{1000, 100, 20.0f, 40.0f});

    static_cast<void>(store.peekOldest());
    static_cast<void>(store.peekOldest());

    EXPECT_FALSE(store.isEmpty());
}

TEST(InMemoryMeasurementStoreTest, PopOldestRemovesTheItem)
{
    InMemoryMeasurementStore store(4);
    store.enqueue(Measurement{1000, 100, 20.0f, 40.0f});

    store.popOldest();

    EXPECT_TRUE(store.isEmpty());
}

TEST(InMemoryMeasurementStoreTest, MaintainsFifoOrderAcrossMultipleItems)
{
    InMemoryMeasurementStore store(4);
    const Measurement first{1000, 100, 20.0f, 40.0f};
    const Measurement second{2000, 200, 21.0f, 41.0f};
    const Measurement third{3000, 300, 22.0f, 42.0f};

    store.enqueue(first);
    store.enqueue(second);
    store.enqueue(third);

    EXPECT_TRUE(measurementsEqual(store.peekOldest(), first));
    store.popOldest();
    EXPECT_TRUE(measurementsEqual(store.peekOldest(), second));
    store.popOldest();
    EXPECT_TRUE(measurementsEqual(store.peekOldest(), third));
    store.popOldest();
    EXPECT_TRUE(store.isEmpty());
}

TEST(InMemoryMeasurementStoreTest, NewStoreIsNotFull)
{
    InMemoryMeasurementStore store(2);

    EXPECT_FALSE(store.isFull());
}

TEST(InMemoryMeasurementStoreTest, IsFullReturnsTrueWhenAtCapacity)
{
    InMemoryMeasurementStore store(2);

    store.enqueue(Measurement{1000, 100, 20.0f, 40.0f});
    store.enqueue(Measurement{2000, 200, 21.0f, 41.0f});

    EXPECT_TRUE(store.isFull());
}

TEST(InMemoryMeasurementStoreTest, IsFullReturnsFalseAfterPoppingBelowCapacity)
{
    InMemoryMeasurementStore store(2);
    store.enqueue(Measurement{1000, 100, 20.0f, 40.0f});
    store.enqueue(Measurement{2000, 200, 21.0f, 41.0f});

    store.popOldest();

    EXPECT_FALSE(store.isFull());
}

TEST(InMemoryMeasurementStoreTest, DropsOldestEntryWhenCapacityExceeded)
{
    InMemoryMeasurementStore store(2);
    const Measurement first{1000, 100, 20.0f, 40.0f};
    const Measurement second{2000, 200, 21.0f, 41.0f};
    const Measurement third{3000, 300, 22.0f, 42.0f};

    store.enqueue(first);
    store.enqueue(second);
    store.enqueue(third); // store is full at capacity 2 - should drop "first"

    EXPECT_TRUE(measurementsEqual(store.peekOldest(), second));
    store.popOldest();
    EXPECT_TRUE(measurementsEqual(store.peekOldest(), third));
    store.popOldest();
    EXPECT_TRUE(store.isEmpty());
}
