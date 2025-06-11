#include <gtest/gtest.h>
#include <m2/math/VecF.h>
#include <m2/math/VecI.h>
#include <m2/M2.h>
#include <m2/Math.h>

using namespace m2;

TEST(VecF, basic) {
	VecF v1;
	EXPECT_FLOAT_EQ(v1.x, 0.0f);
	EXPECT_FLOAT_EQ(v1.y, 0.0f);

	VecF v2{1.0f, 2.0f};
	EXPECT_FLOAT_EQ(v2.x, 1.0f);
	EXPECT_FLOAT_EQ(v2.y, 2.0f);

	VecF v3{4, 5};
	EXPECT_FLOAT_EQ(v3.x, 4.0f);
	EXPECT_FLOAT_EQ(v3.y, 5.0f);

	VecF v4{7u, 8u};
	EXPECT_FLOAT_EQ(v4.x, 7.0f);
	EXPECT_FLOAT_EQ(v4.y, 8.0f);

	VecF v5{VecI{-2, -5}};
	EXPECT_FLOAT_EQ(v5.x, -2.0f);
	EXPECT_FLOAT_EQ(v5.y, -5.0f);

	VecF v6{b2Vec2{1.5f, -3.5f}};
	EXPECT_FLOAT_EQ(v6.x, 1.5f);
	EXPECT_FLOAT_EQ(v6.y, -3.5f);

	VecF v7{2.0f, 4.0f};
	VecF v8{3.0f, 5.0f};

	VecF v9{5.0f, 9.0f};
	EXPECT_EQ(v7 + v8, v9);

	EXPECT_EQ(v7 + 1.0f, v8);

	VecF v10{-1.0f, -1.0f};
	EXPECT_EQ(v7 - v8, v10);

	VecF v11{4.0f, 8.0f};
	EXPECT_EQ(v7 * 2.0f, v11);

	VecF v12{0.5f, 1.0f};
	EXPECT_EQ(v7 / 4.0f, v12);

	bool b1 = (bool) v12;
	EXPECT_TRUE(b1);
	bool b2 = (bool) v1;
	EXPECT_FALSE(b2);

	auto b2v = (b2Vec2) v12;
	EXPECT_FLOAT_EQ(b2v.x, 0.5f);
	EXPECT_FLOAT_EQ(b2v.y, 1.0f);

	VecF v13{NAN, 0.5f};
	EXPECT_TRUE(v13.IsNan());
	EXPECT_FALSE(v12.IsNan());

	VecF v14{0.75f, 0.5f};
	EXPECT_TRUE(v14.IsSmall(0.8f));
	EXPECT_FALSE(v14.IsSmall(0.1f));

	VecF v15{4.1f, 6.7f};
	EXPECT_TRUE(v15.IsNear({4.0f, 7.0f}, 0.5f));
	EXPECT_FALSE(v15.IsNear({5.0f, 6.0f}, 0.5f));

	VecF v16{1.0f, 1.0f};
	EXPECT_FLOAT_EQ(v16.GetLength(), sqrtf(2.0f));

	VecF v17{3.0f, 4.0f};
	EXPECT_FLOAT_EQ(v17.GetLengthSquared(), 25.0f);

	VecF v18{-1.0f, -2.0f};
	EXPECT_FLOAT_EQ(v18.GetDistanceTo({-4.0f, 2.0f}), 5.0f);

	EXPECT_FLOAT_EQ(v18.GetDistanceToSquared(VecF{-4.0f, 2.0f}), 25.0f);

	VecF v19{0.0f, 1.0f};
	EXPECT_FLOAT_EQ(v19.GetAngle(), PI / 2.0f);
	VecF v20{1.0f, 1.0f};
	EXPECT_FLOAT_EQ(v20.GetAngle(), PI / 4.0f);

	VecF v21{1.0f, 1.0f};
	VecF v22{1.0f / sqrtf(2.0f), 1.0f / sqrtf(2.0f)};
	EXPECT_EQ(v21.Normalize(), v22);

	VecF v23{1.0f, 1.0f};
	EXPECT_FLOAT_EQ(v23.FloorLength(1.0f).GetLength(), sqrtf(2.0f));
	EXPECT_FLOAT_EQ(v23.FloorLength(2.0f).GetLength(), 2.0f);

	VecF v24{sqrtf(3), 1.0f};
	VecF v25 = v24.WithLength(4.0f);
	EXPECT_FLOAT_EQ(v25.x, 2.0f * sqrtf(3.0f));
	EXPECT_FLOAT_EQ(v25.y, 2);

	VecF v26{1.0f, 1.0f};
	EXPECT_FLOAT_EQ(v26.CeilLength(1.0f).x, 1.0f / sqrtf(2.0f));
	EXPECT_FLOAT_EQ(v26.CeilLength(2.0f).x, 1.0f);

	VecF v27;
	VecF v28{1,1};
	EXPECT_FLOAT_EQ(v27.Lerp(v28, 1.0f / 3.0f).x, 1.0f / 3.0f);

	auto v29 = VecF::Nan();
	EXPECT_TRUE(isnan(v29.x));
	EXPECT_TRUE(isnan(v29.y));

	auto v30 = VecF::CreateUnitVectorWithAngle(PI / 4.0f);
	EXPECT_FLOAT_EQ(v30.GetLength(), 1.0f);
	EXPECT_FLOAT_EQ(v30.x, 1.0f / sqrtf(2.0f));

	VecF v31{1.23f, 4.56f};
	EXPECT_STREQ(ToString(v31).c_str(), "{x:1.23,y:4.56}");
}

TEST(VecF, hround) {
	VecF v1;
	VecF v2 = v1.RoundHalfI();
	EXPECT_FLOAT_EQ(v2.x, 0.0f);
	EXPECT_FLOAT_EQ(v2.y, 0.0f);

	VecF v3{0.4f, -0.4f};
	VecF v4 = v3.RoundHalfI();
	EXPECT_FLOAT_EQ(v4.x, 0.5f);
	EXPECT_FLOAT_EQ(v4.y, -0.5f);

	VecF v5{0.8f, -0.8f};
	VecF v6 = v5.RoundHalfI();
	EXPECT_FLOAT_EQ(v6.x, 1.0f);
	EXPECT_FLOAT_EQ(v6.y, -1.0f);
}
