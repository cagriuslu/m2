#include <gtest/gtest.h>
#include <m2/common/containers/Pool.h>

using namespace m2;

TEST(Pool, EmplaceAndSize) {
	Pool<int, 64> pool;
	EXPECT_TRUE(pool.Empty());
	EXPECT_EQ(pool.Size(), 0u);

	auto firstItem = pool.Emplace(10);
	auto secondItem = pool.Emplace(20);
	EXPECT_FALSE(pool.Empty());
	EXPECT_EQ(pool.Size(), 2u);
	EXPECT_EQ(*firstItem, 10);
	EXPECT_EQ(*secondItem, 20);
	// Each item gets a distinct Id
	EXPECT_NE(firstItem.GetId(), secondItem.GetId());
}

TEST(Pool, Access) {
	Pool<int, 64> pool;
	const auto id = pool.Emplace(42).GetId();

	EXPECT_TRUE(pool.Contains(id));
	ASSERT_NE(pool.Get(id), nullptr);
	EXPECT_EQ(*pool.Get(id), 42);
	EXPECT_EQ(pool[id], 42);

	// A null Id refers to nothing
	EXPECT_EQ(pool.Get(0), nullptr);
	EXPECT_FALSE(pool.Contains(Id{0}));
}

TEST(Pool, IdFromData) {
	Pool<int, 64> pool;
	auto item = pool.Emplace(7);
	// The pool can recover an item's Id from a pointer into it
	EXPECT_EQ(pool.GetId(item.Data()), item.GetId());
}

TEST(Pool, Free) {
	Pool<int, 64> pool;
	const auto firstId = pool.Emplace(1).GetId();
	const auto secondId = pool.Emplace(2).GetId();

	pool.Free(firstId);
	EXPECT_EQ(pool.Size(), 1u);
	EXPECT_FALSE(pool.Contains(firstId));
	EXPECT_TRUE(pool.Contains(secondId));
	EXPECT_EQ(pool[secondId], 2);
}

TEST(Pool, ReusedSlotGetsNewId) {
	Pool<int, 64> pool;
	const auto firstId = pool.Emplace(1).GetId();
	pool.Free(firstId);
	// The freed slot is reused, but the new item must not answer to the old Id
	const auto secondId = pool.Emplace(2).GetId();
	EXPECT_NE(firstId, secondId);
	EXPECT_FALSE(pool.Contains(firstId));
	EXPECT_TRUE(pool.Contains(secondId));
}

TEST(Pool, Iteration) {
	Pool<int, 64> pool;
	pool.Emplace(10);
	pool.Emplace(20);
	pool.Emplace(30);

	int visitedCount = 0;
	int sum = 0;
	for (auto& value : pool) {
		++visitedCount;
		sum += value;
	}
	EXPECT_EQ(visitedCount, 3);
	EXPECT_EQ(sum, 60);
}

TEST(Pool, Clear) {
	Pool<int, 64> pool;
	pool.Emplace(1);
	pool.Emplace(2);
	pool.Clear();
	EXPECT_TRUE(pool.Empty());
	EXPECT_EQ(pool.Size(), 0u);
}
