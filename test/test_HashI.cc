#include <gtest/gtest.h>
#include <m2/common/math/Hash.h>
#include <m2/common/math/Exact.h>
#include <cstdint>
#include <string>

using namespace m2;

// The implementation seeds with 0x1ED980DD when initialValue is 0 and XOR-folds the little-endian bytes of the
// input into the running hash. The goldens below pin that seed and folding behavior.

TEST(HashI, Deterministic) {
	EXPECT_EQ(HashI(int32_t{12345}, 0), HashI(int32_t{12345}, 0));
	EXPECT_EQ(HashI(std::string{"hello"}, 0), HashI(std::string{"hello"}, 0));
	EXPECT_EQ(HashI(Exact{1}, 0), HashI(Exact{1}, 0));
}

TEST(HashI, ScalarGoldens) {
	// Zero value with the default seed yields the raw seed.
	EXPECT_EQ(HashI(int32_t{0}, 0), 0x1ED980DD);
	// XOR of the seed with the low byte / second byte of the value.
	EXPECT_EQ(HashI(int32_t{1}, 0), 0x1ED980DC);
	EXPECT_EQ(HashI(int32_t{0xFF}, 0), 0x1ED98022);
	EXPECT_EQ(HashI(int32_t{0x100}, 0), 0x1ED981DD);
	// A non-zero seed with a zero value passes the seed straight through.
	EXPECT_EQ(HashI(int32_t{0}, 0x0BADF00D), 0x0BADF00D);
}

TEST(HashI, DifferentValuesDifferentHashes) {
	EXPECT_NE(HashI(int32_t{3}, 0), HashI(int32_t{4}, 0));
	EXPECT_NE(HashI(int32_t{0}, 0), HashI(int32_t{1}, 0));
}

TEST(HashI, DifferentSeedsDifferentHashes) {
	EXPECT_NE(HashI(int32_t{42}, 1), HashI(int32_t{42}, 2));
}

TEST(HashI, StringGoldenAndOrderSensitivity) {
	EXPECT_EQ(HashI(std::string{"A"}, 0), 0x1ED9809C);
	// Swapping two adjacent bytes (different shift positions) changes the hash.
	EXPECT_NE(HashI(std::string{"ab"}, 0), HashI(std::string{"ba"}, 0));
	// Different content generally hashes differently.
	EXPECT_NE(HashI(std::string{"hello"}, 0), HashI(std::string{"world"}, 0));
}

TEST(HashI, StringMatchesRawByteOverload) {
	const std::string text{"abc"};
	const auto* bytes = reinterpret_cast<const uint8_t*>(text.data());
	EXPECT_EQ(HashI(text, 0), HashI(bytes, text.size(), 0));
}

TEST(HashI, ByteOverloadDeterministicAndOrderSensitive) {
	const uint8_t forward[3] = {1, 2, 3};
	const uint8_t reverse[3] = {3, 2, 1};
	EXPECT_EQ(HashI(forward, 3, 0), HashI(forward, 3, 0));
	EXPECT_NE(HashI(forward, 3, 0), HashI(reverse, 3, 0));
}

TEST(HashI, ExactOverloadDelegatesToRawValue) {
	// The Exact overload hashes the underlying raw value.
	EXPECT_EQ(HashI(Exact{1}, 0), HashI(Exact{1}.ToRawValue(), 0));
	EXPECT_NE(HashI(Exact{1}, 0), HashI(Exact{2}, 0));
}

TEST(HashI, FloatAndDouble) {
	EXPECT_EQ(HashI(1.0f, 0), HashI(1.0f, 0));
	EXPECT_NE(HashI(1.0f, 0), HashI(2.0f, 0));
	EXPECT_EQ(HashI(1.0, 0), HashI(1.0, 0));
	EXPECT_NE(HashI(1.0, 0), HashI(2.0, 0));
}

TEST(HashI, IncrementalChainingIsDeterministic) {
	const int32_t step1 = HashI(int32_t{11}, 0);
	const int32_t chained = HashI(int32_t{22}, step1);
	EXPECT_EQ(chained, HashI(int32_t{22}, HashI(int32_t{11}, 0)));
	// Feeding a second value changes the accumulated hash.
	EXPECT_NE(chained, step1);
}
