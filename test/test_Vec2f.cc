#include <gtest/gtest.h>
#include <m2/Vec2f.h>
#include <m2/Vec2i.h>
#include <m2/M2.h>

TEST(Vec2f, basic) {
	using namespace m2;

	Vec2f v1;
	EXPECT_FLOAT_EQ(v1.x, 0.0f);
	EXPECT_FLOAT_EQ(v1.y, 0.0f);

	Vec2f v2{1.0f, 2.0f};
	EXPECT_FLOAT_EQ(v2.x, 1.0f);
	EXPECT_FLOAT_EQ(v2.y, 2.0f);

	Vec2f v3{4, 5};
	EXPECT_FLOAT_EQ(v3.x, 4.0f);
	EXPECT_FLOAT_EQ(v3.y, 5.0f);

	Vec2f v4{7u, 8u};
	EXPECT_FLOAT_EQ(v4.x, 7.0f);
	EXPECT_FLOAT_EQ(v4.y, 8.0f);

	Vec2f v5{Vec2i{-2, -5}};
	EXPECT_FLOAT_EQ(v5.x, -2.0f);
	EXPECT_FLOAT_EQ(v5.y, -5.0f);

	Vec2f v6{b2Vec2{1.5f, -3.5f}};
	EXPECT_FLOAT_EQ(v6.x, 1.5f);
	EXPECT_FLOAT_EQ(v6.y, -3.5f);

	Vec2f v7{2.0f, 4.0f};
	Vec2f v8{3.0f, 5.0f};

	Vec2f v9{5.0f, 9.0f};
	EXPECT_EQ(v7 + v8, v9);

	EXPECT_EQ(v7 + 1.0f, v8);

	Vec2f v10{-1.0f, -1.0f};
	EXPECT_EQ(v7 - v8, v10);

	Vec2f v11{4.0f, 8.0f};
	EXPECT_EQ(v7 * 2.0f, v11);

	Vec2f v12{0.5f, 1.0f};
	EXPECT_EQ(v7 / 4.0f, v12);

	bool b1 = (bool) v12;
	EXPECT_TRUE(b1);
	bool b2 = (bool) v1;
	EXPECT_FALSE(b2);

	b2Vec2 b2v = (b2Vec2) v12;
	EXPECT_FLOAT_EQ(b2v.x, 0.5f);
	EXPECT_FLOAT_EQ(b2v.y, 1.0f);

	Vec2f v13{NAN, 0.5f};
	EXPECT_TRUE(v13.is_nan());
	EXPECT_FALSE(v12.is_nan());

	Vec2f v14{0.75f, 0.5f};
	EXPECT_TRUE(v14.is_small(0.8f));
	EXPECT_FALSE(v14.is_small(0.1f));

	Vec2f v15{4.1f, 6.7f};
	EXPECT_TRUE(v15.is_near({4.0f, 7.0f}, 0.5f));
	EXPECT_FALSE(v15.is_near({5.0f, 6.0f}, 0.5f));

	Vec2f v16{1.0f, 1.0f};
	EXPECT_FLOAT_EQ(v16.length(), sqrtf(2.0f));

	Vec2f v17{3.0f, 4.0f};
	EXPECT_FLOAT_EQ(v17.length_sq(), 25.0f);

	Vec2f v18{-1.0f, -2.0f};
	EXPECT_FLOAT_EQ(v18.distance({-4.0f, 2.0f}), 5.0f);

	EXPECT_FLOAT_EQ(v18.distance_sq({-4.0f, 2.0f}), 25.0f);

	Vec2f v19{0.0f, 1.0f};
	EXPECT_FLOAT_EQ(v19.angle_rads(), PI / 2.0f);
	Vec2f v20{1.0f, 1.0f};
	EXPECT_FLOAT_EQ(v20.angle_rads(), PI / 4.0f);

	Vec2f v21{1.0f, 1.0f};
	Vec2f v22{1.0f / sqrtf(2.0f), 1.0f / sqrtf(2.0f)};
	EXPECT_EQ(v21.normalize(), v22);

	Vec2f v23{1.0f, 1.0f};
	EXPECT_FLOAT_EQ(v23.floor_length(1.0f).length(), sqrtf(2.0f));
	EXPECT_FLOAT_EQ(v23.floor_length(2.0f).length(), 2.0f);

	// TODO
}
