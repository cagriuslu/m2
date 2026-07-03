#include <gtest/gtest.h>
#include <m2/thirdparty/compression/Deflate.h>
#include <m2/thirdparty/compression/Inflate.h>
#include <cstdint>
#include <vector>

using namespace m2::thirdparty::compression;

TEST(Compression, RoundTripSmall) {
	const std::vector<uint8_t> original{1, 2, 3, 4, 5, 250, 128, 0, 255};

	const auto compressed = Deflate(original);
	ASSERT_TRUE(compressed.has_value());

	const auto restored = Inflate(*compressed, static_cast<int>(original.size()));
	ASSERT_TRUE(restored.has_value());
	EXPECT_EQ(*restored, original);
}

TEST(Compression, RoundTripLargeRepetitive) {
	std::vector<uint8_t> original(10000, 0);
	for (size_t i = 0; i < original.size(); ++i) {
		original[i] = static_cast<uint8_t>(i % 7);
	}

	const auto compressed = Deflate(original);
	ASSERT_TRUE(compressed.has_value());
	// Repetitive data must actually compress to fewer bytes than the input.
	EXPECT_LT(compressed->size(), original.size());

	const auto restored = Inflate(*compressed, static_cast<int>(original.size()));
	ASSERT_TRUE(restored.has_value());
	EXPECT_EQ(*restored, original);
}

TEST(Compression, InflateOfGarbageFails) {
	// Bytes that don't form a valid zlib stream must surface an error, not a value.
	const std::vector<uint8_t> garbage{0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x11, 0x22};

	const auto restored = Inflate(garbage, 16);
	EXPECT_FALSE(restored.has_value());
}

TEST(Compression, InflateWithTooSmallSizeFails) {
	const std::vector<uint8_t> original(500, 42);
	const auto compressed = Deflate(original);
	ASSERT_TRUE(compressed.has_value());

	// The decompressed data (500 bytes) can't fit into a 10-byte buffer, so
	// zlib reports Z_BUF_ERROR and Inflate returns an error.
	const auto restored = Inflate(*compressed, 10);
	EXPECT_FALSE(restored.has_value());
}
