#include <gtest/gtest.h>
#include <m2/VecI.h>
#include <m2/VecF.h>

TEST(VecI, basic) {
	using namespace m2;

	VecI v1;
	EXPECT_EQ(v1.x, 0);
	EXPECT_EQ(v1.y, 0);

	VecI v2{5, 15};
	EXPECT_EQ(v2.x, 5);
	EXPECT_EQ(v2.y, 15);

	VecI v3{25.0f, 35.0f};
	EXPECT_EQ(v3.x, 25);
	EXPECT_EQ(v3.y, 35);

	VecI v4{VecF{45.0f, 55.0f}};
	EXPECT_EQ(v4.x, 45);
	EXPECT_EQ(v4.y, 55);
}

TEST(VecI, operators) {
	using namespace m2;

	VecI v1{1, 2};
	VecI v2{3, 4};
	VecI v3 = v1 + v2;
	EXPECT_EQ(v3.x, 4);
	EXPECT_EQ(v3.y, 6);

	VecI v4{7, 8};
	VecI v5{2, 1};
	VecI v6 = v4 - v5;
	EXPECT_EQ(v6.x, 5);
	EXPECT_EQ(v6.y, 7);

	VecI v7{6, 7};
	VecI v8{8, 9};
	EXPECT_NE(v7, v8);

	VecI v9{12, 13};
	VecI v10{12, 13};
	EXPECT_EQ(v9, v10);

	auto b1 = (bool) VecI{1, 0};
	EXPECT_TRUE(b1);

	auto b2 = (bool) VecI{0, 0};
	EXPECT_FALSE(b2);
}

TEST(VecI, utils) {
	using namespace m2;

	VecI v1{10, 10};
	EXPECT_TRUE(v1.is_near(VecI{10, 10}, 1));
	EXPECT_TRUE(v1.is_near(VecI{11, 11}, 1));
	EXPECT_FALSE(v1.is_near(VecI{15, 15}, 2));
	EXPECT_TRUE(v1.is_near(VecI{15, 15}, 5));
	EXPECT_FALSE(v1.is_near(VecI{11, 11}, 0));

	VecI v2{0, 0};
	EXPECT_FALSE(v2.is_negative());
	VecI v3{-1, 0};
	EXPECT_TRUE(v3.is_negative());
	VecI v4{-1, -1};
	EXPECT_TRUE(v4.is_negative());

	VecI v5{1, 2};
	EXPECT_EQ(v5.length_sq(), 5.0f);

	VecI v6{3, 4};
	EXPECT_EQ(v6.length(), 5.0f);

	VecI v7, v8{3, 4};
	EXPECT_EQ(v7.distance(v8), 5.0f);

	VecI v9{12, 13}, v10{3, 4};
	EXPECT_EQ(v9.manhattan_distance(v10), 18);
}

TEST(VecI, to_string) {
	using namespace m2;

	EXPECT_STREQ(to_string(VecI{10,20}).c_str(), "{\"y\":\"20\",\"x\":\"10\",}");
}
