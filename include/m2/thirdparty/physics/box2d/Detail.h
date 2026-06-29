#pragma once
#include <m2/common/math/VecF.h>
#include <box2d/b2_math.h>

namespace m2::thirdparty::physics::box2d {
	inline VecF ToVecF(const b2Vec2& b) { return {b.x, b.y}; }
	inline b2Vec2 ToBox2dVec2(const VecF& v) { return {v.GetX(), v.GetY()}; }
}
