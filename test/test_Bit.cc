#include <gtest/gtest.h>
#include <m2/detail/Bit.h>

TEST(Bit, RotateLeft64) {
	EXPECT_EQ(m2::RotateLeft64(0xAA, 8), 0xAA00);
	EXPECT_EQ(m2::RotateLeft64(0xAA00000000000000, 8), 0xAA);
}

TEST(Bit, RotateRight64) {
	EXPECT_EQ(m2::RotateRight64(0xAA, 8), 0xAA00000000000000);
	EXPECT_EQ(m2::RotateRight64(0xAA00, 8), 0xAA);
}
