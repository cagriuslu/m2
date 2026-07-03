#include <gtest/gtest.h>
#include <m2/common/containers/Cache.h>

using namespace m2;

TEST(Cache, GeneratesOnFirstAccess) {
	int generatorCallCount = 0;
	auto squareGenerator = [&generatorCallCount](const int& key) {
		++generatorCallCount;
		return key * key;
	};
	Cache<int, int, decltype(squareGenerator)> cache{std::move(squareGenerator)};

	EXPECT_EQ(cache(3), 9);
	EXPECT_EQ(generatorCallCount, 1);
}

TEST(Cache, CachesRepeatedAccess) {
	int generatorCallCount = 0;
	auto squareGenerator = [&generatorCallCount](const int& key) {
		++generatorCallCount;
		return key * key;
	};
	Cache<int, int, decltype(squareGenerator)> cache{std::move(squareGenerator)};

	EXPECT_EQ(cache(4), 16);
	EXPECT_EQ(cache(4), 16);
	// The generator runs only once for the same key
	EXPECT_EQ(generatorCallCount, 1);
}

TEST(Cache, GeneratesPerDistinctKey) {
	int generatorCallCount = 0;
	auto squareGenerator = [&generatorCallCount](const int& key) {
		++generatorCallCount;
		return key * key;
	};
	Cache<int, int, decltype(squareGenerator)> cache{std::move(squareGenerator)};

	EXPECT_EQ(cache(2), 4);
	EXPECT_EQ(cache(3), 9);
	EXPECT_EQ(generatorCallCount, 2);
}

TEST(Cache, ReturnsMutableReference) {
	auto zeroGenerator = [](const int&) { return 0; };
	Cache<int, int, decltype(zeroGenerator)> cache{std::move(zeroGenerator)};

	cache(1) = 100;
	// The mutated value persists in the cache
	EXPECT_EQ(cache(1), 100);
}
