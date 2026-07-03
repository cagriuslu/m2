#include <gtest/gtest.h>
#include <m2/common/video/Color.h>

using namespace m2;

static void ExpectRgb(const RGB& color, int r, int g, int b) {
	EXPECT_EQ(color.r, r);
	EXPECT_EQ(color.g, g);
	EXPECT_EQ(color.b, b);
}

static void ExpectRgba(const RGBA& color, int r, int g, int b, int a) {
	EXPECT_EQ(color.r, r);
	EXPECT_EQ(color.g, g);
	EXPECT_EQ(color.b, b);
	EXPECT_EQ(color.a, a);
}

TEST(Color, RgbArithmetic) {
	RGB gray{100, 100, 100};
	ExpectRgb(gray * 2.0f, 200, 200, 200);
	ExpectRgb(gray / 2.0f, 50, 50, 50);
	// Rounds to nearest
	ExpectRgb((RGB{100, 100, 100} * 1.5f), 150, 150, 150);
}

TEST(Color, RgbaArithmetic) {
	// Alpha is left unscaled
	RGBA color{100, 100, 100, 200};
	ExpectRgba(color * 2.0f, 200, 200, 200, 200);
	ExpectRgba(color / 2.0f, 50, 50, 50, 200);
}

TEST(Color, RgbaFloatConstructor) {
	RGBA color{0.5f, 1.0f, 0.0f, 1.0f};
	ExpectRgba(color, 128, 255, 0, 255);
}

TEST(Color, RgbaToRgb) {
	RGBA color{10, 20, 30, 40};
	ExpectRgb(static_cast<RGB>(color), 10, 20, 30);
}

TEST(Color, NamedConstants) {
	ExpectRgb(RGB::Red, 255, 0, 0);
	ExpectRgb(RGB::Green, 0, 255, 0);
	ExpectRgb(RGB::Blue, 0, 0, 255);

	ExpectRgba(RGBA::Red, 255, 0, 0, 255);
	ExpectRgba(RGBA::Black, 0, 0, 0, 255);
	ExpectRgba(RGBA::TransparentBlack, 0, 0, 0, 128);
}
