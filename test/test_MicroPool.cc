#include <gtest/gtest.h>
#include <m2/containers/MicroPool.h>

TEST(MicroPool, basic) {
	m2::MicroPool<int, 10> myPool;
	EXPECT_EQ(myPool.empty(), true);
	EXPECT_EQ(myPool.size(), 0);
	EXPECT_EQ(myPool.cbegin(), myPool.cend());
	EXPECT_EQ(myPool.begin(), myPool.end());

	EXPECT_EQ(myPool.emplace(12).second, true);
	EXPECT_EQ(myPool.empty(), false);
	EXPECT_EQ(myPool.size(), 1);
	EXPECT_EQ(*myPool.cbegin(), 12);
	EXPECT_EQ(*myPool.begin(), 12);

	EXPECT_EQ(myPool.emplace(24).second, true);
	EXPECT_EQ(myPool.empty(), false);
	EXPECT_EQ(myPool.size(), 2);
	EXPECT_EQ(*myPool.cbegin(), 12);
	EXPECT_EQ(*myPool.begin(), 12);
	EXPECT_EQ(*++myPool.cbegin(), 24);
	EXPECT_EQ(*++myPool.begin(), 24);

	myPool.erase(myPool.begin());
	EXPECT_EQ(myPool.empty(), false);
	EXPECT_EQ(myPool.size(), 1);
	EXPECT_EQ(*myPool.cbegin(), 24);
	EXPECT_EQ(*myPool.begin(), 24);
	EXPECT_EQ(++myPool.cbegin(), myPool.cend());
	EXPECT_EQ(++myPool.begin(), myPool.end());

	myPool.clear();
	EXPECT_EQ(myPool.empty(), true);
	EXPECT_EQ(myPool.size(), 0);
	EXPECT_EQ(myPool.cbegin(), myPool.cend());
	EXPECT_EQ(myPool.begin(), myPool.end());
}
