#include <gtest/gtest.h>
#include <m2/common/Meta.h>
#include <array>
#include <iterator>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

using namespace m2;

namespace {
	// std::array utilities (constexpr).
	static_assert(AreArrayElementsUnique(std::array{1, 2, 3}));
	static_assert(not AreArrayElementsUnique(std::array{1, 2, 2}));

	static_assert(DoesArrayContainElement(std::array{1, 2, 3}, 2));
	static_assert(not DoesArrayContainElement(std::array{1, 2, 3}, 9));

	constexpr auto kConcatenated = ConcatArray(std::array{1, 2}, std::array{3, 4, 5});
	static_assert(kConcatenated.size() == 5);
	static_assert(kConcatenated[0] == 1 && kConcatenated[2] == 3 && kConcatenated[4] == 5);

	// Variant / tuple index lookups (compile-time).
	static_assert(GetIndexInVariant<double, std::variant<int, double, char>>::value == 1);
	static_assert(GetIndexInVariant<char, std::variant<int, double, char>>::value == 2);
	static_assert(GetIndexInTuple<int, std::tuple<char, int, double>>::value == 1);
}

TEST(Meta, TransformIf) {
	const std::vector<int> input{1, 2, 3, 4, 5, 6};
	std::vector<int> output;
	TransformIf(input.begin(), input.end(), std::back_inserter(output),
		[](const int x) { return x % 2 == 0; },
		[](const int x) { return x * 10; });
	EXPECT_EQ(output, (std::vector<int>{20, 40, 60}));
}

TEST(Meta, ForEachAdjacentPair) {
	const std::vector<int> input{1, 2, 4, 7};
	std::vector<int> differences;
	ForEachAdjacentPair(input.begin(), input.end(), [&](const int a, const int b) {
		differences.push_back(b - a);
	});
	EXPECT_EQ(differences, (std::vector<int>{1, 2, 3}));

	// Fewer than two elements yields no pairs.
	const std::vector<int> single{5};
	int callCount = 0;
	ForEachAdjacentPair(single.begin(), single.end(), [&](int, int) { ++callCount; });
	EXPECT_EQ(callCount, 0);
}

TEST(Meta, ForEachZipStopsAtFirstResult) {
	const std::vector<int> a{1, 2, 3, 4};
	const std::vector<int> b{1, 2, 9, 4};
	auto firstA = a.begin();
	auto firstB = b.begin();
	const std::optional<int> result = ForEachZip<int>(firstA, a.end(), firstB, b.end(),
		[](const int x, const int y) -> std::optional<int> {
			if (x != y) { return x + y; } // Stop and return the sum at the first mismatch.
			return std::nullopt;
		});
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ(*result, 12); // 3 + 9
}

TEST(Meta, ForEachZipReturnsNulloptWhenExhausted) {
	const std::vector<int> a{1, 2, 3};
	const std::vector<int> b{1, 2, 3};
	auto firstA = a.begin();
	auto firstB = b.begin();
	const std::optional<int> result = ForEachZip<int>(firstA, a.end(), firstB, b.end(),
		[](const int x, const int y) -> std::optional<int> {
			if (x != y) { return x + y; }
			return std::nullopt;
		});
	EXPECT_FALSE(result.has_value());
}

TEST(Meta, ToValues) {
	const std::map<int, std::string> map{{1, "a"}, {2, "b"}, {3, "c"}};
	const std::vector<std::string> values = ToValues<std::string>(map);
	EXPECT_EQ(values, (std::vector<std::string>{"a", "b", "c"}));
}

TEST(Meta, ToVectorAndToSet) {
	const std::vector<int> vec = ToVector(std::set<int>{3, 1, 2});
	EXPECT_EQ(vec, (std::vector<int>{1, 2, 3}));

	const std::set<int> set = ToSet(std::vector<int>{2, 2, 1, 3});
	EXPECT_EQ(set, (std::set<int>{1, 2, 3}));
}

TEST(Meta, IsFirstEqualsAndIsSecondEquals) {
	// The returned predicate captures its argument by reference, so the keys must outlive the predicate.
	const int firstKey = 2;
	const auto firstIsTwo = IsFirstEquals<int, std::string>(firstKey);
	EXPECT_TRUE(firstIsTwo(std::pair<int, std::string>{2, "x"}));
	EXPECT_FALSE(firstIsTwo(std::pair<int, std::string>{3, "x"}));

	const std::string secondKey = "x";
	const auto secondIsX = IsSecondEquals<int, std::string>(secondKey);
	EXPECT_TRUE(secondIsX(std::pair<int, std::string>{7, "x"}));
	EXPECT_FALSE(secondIsX(std::pair<int, std::string>{7, "y"}));
}
