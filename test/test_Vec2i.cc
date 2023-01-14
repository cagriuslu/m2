#include <gtest/gtest.h>
#include <m2/Vec2i.h>
#include <m2/Vec2f.h>

TEST(Vec2i, basic) {
	using namespace m2;

	Vec2i v1;
	EXPECT_EQ(v1.x, 0);
	EXPECT_EQ(v1.y, 0);

	Vec2i v2{5, 15};
	EXPECT_EQ(v2.x, 5);
	EXPECT_EQ(v2.y, 15);

	Vec2i v3{25.0f, 35.0f};
	EXPECT_EQ(v3.x, 25);
	EXPECT_EQ(v3.y, 35);

	Vec2i v4{Vec2f{45.0f, 55.0f}};
	EXPECT_EQ(v4.x, 45);
	EXPECT_EQ(v4.y, 55);
}

TEST(Vec2i, operators) {
	using namespace m2;

	Vec2i v1{1, 2};
	Vec2i v2{3, 4};
	Vec2i v3 = v1 + v2;
	EXPECT_EQ(v3.x, 4);
	EXPECT_EQ(v3.y, 6);

	Vec2i v4{7, 8};
	Vec2i v5{2, 1};
	Vec2i v6 = v4 - v5;
	EXPECT_EQ(v6.x, 5);
	EXPECT_EQ(v6.y, 7);

	Vec2i v7{6, 7};
	Vec2i v8{8, 9};
	EXPECT_NE(v7, v8);

	Vec2i v9{12, 13};
	Vec2i v10{12, 13};
	EXPECT_EQ(v9, v10);

	auto b1 = (bool) Vec2i{1, 0};
	EXPECT_TRUE(b1);

	auto b2 = (bool) Vec2i{0, 0};
	EXPECT_FALSE(b2);
}

TEST(Vec2i, utils) {
	using namespace m2;

	Vec2i v1{10, 10};
	EXPECT_TRUE(v1.is_near(Vec2i{10, 10}, 1));
	EXPECT_TRUE(v1.is_near(Vec2i{11, 11}, 1));
	EXPECT_FALSE(v1.is_near(Vec2i{15, 15}, 2));
	EXPECT_TRUE(v1.is_near(Vec2i{15, 15}, 5));
	EXPECT_FALSE(v1.is_near(Vec2i{11, 11}, 0));

	Vec2i v2{0, 0};
	EXPECT_FALSE(v2.is_negative());
	Vec2i v3{-1, 0};
	EXPECT_TRUE(v3.is_negative());
	Vec2i v4{-1, -1};
	EXPECT_TRUE(v4.is_negative());

	Vec2i v5{1, 2};
	EXPECT_EQ(v5.length_sq(), 5.0f);

	Vec2i v6{3, 4};
	EXPECT_EQ(v6.length(), 5.0f);

	Vec2i v7, v8{3, 4};
	EXPECT_EQ(v7.distance(v8), 5.0f);

	Vec2i v9{12, 13}, v10{3, 4};
	EXPECT_EQ(v9.manhattan_distance(v10), 18);
}

TEST(Vec2i, to_string) {
	using namespace m2;

	EXPECT_STREQ(to_string(Vec2i{10,20}).c_str(), "{x:10,y:20}");
}
