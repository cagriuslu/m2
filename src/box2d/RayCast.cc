#include <m2/box2d/RayCast.h>
#include <m2/M2.h>

m2::box2d::RayCastCallback::RayCastCallback(std::function<float (b2Fixture*,m2::VecF,m2::VecF,float)>&& cb, uint16_t categoryMask) : m_cb(cb), m_categoryMask(categoryMask) {}

float m2::box2d::RayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) {
    if (fixture->GetFilterData().categoryBits & m_categoryMask) {
        return (m_cb)(fixture, m2::VecF{point}, m2::VecF{normal}, fraction);
    }
    return 1.0f;
}

bool m2::box2d::CheckEyesight(b2World& world, m2::VecF from, m2::VecF to, uint16_t category_bits) {
    if (from == to) {
        return true;
    } else {
        bool result = true;
        RayCastCallback rccb([&result](MAYBE b2Fixture* fixture, MAYBE VecF point, MAYBE VecF normal, MAYBE float fraction) -> float {
			result = false;
			return 0.0f;
		}, category_bits);
        world.RayCast(&rccb, static_cast<b2Vec2>(from), static_cast<b2Vec2>(to));
        return result;
    }
}

float m2::box2d::CheckDistance(b2World& world, VecF from, VecF to, uint16_t category_bits) {
	VecF poi{to};
	RayCastCallback rccb([&poi](MAYBE b2Fixture* fixture, VecF point, MAYBE VecF normal, MAYBE float fraction) -> float {
		poi = point;
		return fraction;
	}, category_bits);
	world.RayCast(&rccb, static_cast<b2Vec2>(from), static_cast<b2Vec2>(to));
	return (poi - from).GetLength();
}
