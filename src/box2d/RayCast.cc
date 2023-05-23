#include <m2/box2d/RayCast.h>
#include <m2/M2.h>

m2::box2d::RayCastCallback::RayCastCallback(std::function<float (b2Fixture*,m2::Vec2f,m2::Vec2f,float)>&& cb, uint16_t categoryMask) : m_cb(cb), m_categoryMask(categoryMask) {}

float m2::box2d::RayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) {
    if (fixture->GetFilterData().categoryBits & m_categoryMask) {
        return (m_cb)(fixture, m2::Vec2f{point}, m2::Vec2f{normal}, fraction);
    }
    return 1.0f;
}

bool m2::box2d::check_eyesight(b2World& world, m2::Vec2f from, m2::Vec2f to, uint16_t category_bits) {
    if (from == to) {
        return true;
    } else {
        bool result = true;
        RayCastCallback rccb([&result](MAYBE b2Fixture* fixture, MAYBE Vec2f point, MAYBE Vec2f normal, MAYBE float fraction) -> float {
			result = false;
			return 0.0f;
		}, category_bits);
        world.RayCast(&rccb, static_cast<b2Vec2>(from), static_cast<b2Vec2>(to));
        return result;
    }
}

float m2::box2d::check_distance(b2World& world, Vec2f from, Vec2f to, uint16_t category_bits) {
	Vec2f poi{to};
	RayCastCallback rccb([&poi](MAYBE b2Fixture* fixture, Vec2f point, MAYBE Vec2f normal, MAYBE float fraction) -> float {
		poi = point;
		return fraction;
	}, category_bits);
	world.RayCast(&rccb, static_cast<b2Vec2>(from), static_cast<b2Vec2>(to));
	return (poi - from).length();
}
