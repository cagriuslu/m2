#include <gtest/gtest.h>
#include <m2/Vec2i.h>
#include <m2/Vec2f.h>

TEST(Vec2i, basic) {
	m2::Vec2i v1;
	EXPECT_EQ(v1.x, 0);
	EXPECT_EQ(v1.y, 0);

	m2::Vec2i v2{5, 15};
	EXPECT_EQ(v2.x, 5);
	EXPECT_EQ(v2.y, 15);

	m2::Vec2i v3{25.0f, 35.0f};
	EXPECT_EQ(v3.x, 25);
	EXPECT_EQ(v3.y, 35);

	m2::Vec2i v4{m2::Vec2f{45.0f, 55.0f}};
	EXPECT_EQ(v4.x, 45);
	EXPECT_EQ(v4.y, 55);

	m2::Vec2i v5{1,2};
	m2::Vec2i v6{3,4};
	m2::Vec2i v7 = v5 + v6;
	EXPECT_EQ(v7.x, 4);
	EXPECT_EQ(v7.y, 6);

	m2::Vec2i v8{7,8};
	m2::Vec2i v9{2,1};
	m2::Vec2i v10 = v8 - v9;
	EXPECT_EQ(v10.x, 5);
	EXPECT_EQ(v10.y, 7);

	m2::Vec2i v11{6,7};
	m2::Vec2i v12{8,9};
	EXPECT_NE(v11, v12);

	m2::Vec2i v13{12,13};
	m2::Vec2i v14{12,13};
	EXPECT_EQ(v13, v14);

	auto b1 = (bool)m2::Vec2i{1,0};
	EXPECT_TRUE(b1);

	auto b2 = (bool)m2::Vec2i{0,0};
	EXPECT_FALSE(b2);

	EXPECT_STREQ(m2::to_string(m2::Vec2i{10,20}).c_str(), "{ x: 10, y: 20 }");
}
