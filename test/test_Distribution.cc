#include <gtest/gtest.h>
#include <m2/common/rng/Distribution.h>
#include <m2/common/rng/XsrRng.h>

using namespace m2;

TEST(CustomOptionDistribution, SingleOptionAlwaysReturned) {
	CustomOptionDistribution distribution{std::vector<Exact>{Exact{42}}};
	XsrRng rng{1, 2, 3, 4};

	for (int i = 0; i < 100; ++i) {
		Exact result;
		distribution.GenerateNextExact(rng, result);
		EXPECT_EQ(result, Exact{42});
	}
}

TEST(CustomOptionDistribution, ResultIsAlwaysAnOption) {
	CustomOptionDistribution distribution{std::vector<Exact>{Exact{1}, Exact{2}, Exact{3}}};
	XsrRng rng{5, 6, 7, 8};

	for (int i = 0; i < 100; ++i) {
		Exact result;
		distribution.GenerateNextExact(rng, result);
		EXPECT_TRUE(result == Exact{1} || result == Exact{2} || result == Exact{3});
	}
}

TEST(CustomOptionDistribution, DeterministicForSameSeed) {
	const std::vector<Exact> options{Exact{10}, Exact{20}, Exact{30}, Exact{40}};
	CustomOptionDistribution firstDistribution{std::vector<Exact>{options}};
	CustomOptionDistribution secondDistribution{std::vector<Exact>{options}};
	XsrRng firstRng{9, 10, 11, 12};
	XsrRng secondRng{9, 10, 11, 12};

	for (int i = 0; i < 20; ++i) {
		Exact firstResult, secondResult;
		firstDistribution.GenerateNextExact(firstRng, firstResult);
		secondDistribution.GenerateNextExact(secondRng, secondResult);
		EXPECT_EQ(firstResult, secondResult);
	}
}
