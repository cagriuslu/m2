#include <m2/box2d/RayCast.h>

m2::box2d::RayCastCallback::RayCastCallback(std::function<float (b2Fixture*,m2::Vec2f,m2::Vec2f,float)>&& cb, uint16_t categoryMask) : m_cb(cb), m_categoryMask(categoryMask) {}

float m2::box2d::RayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) {
    if (fixture->GetFilterData().categoryBits & m_categoryMask) {
        return (m_cb)(fixture, m2::Vec2f{point }, m2::Vec2f{normal }, fraction);
    }
    return 1.0f;
}

bool m2::box2d::check_eye_sight(b2World& world, m2::Vec2f from, m2::Vec2f to, uint16_t category_bits) {
    if (from == to) {
        return true;
    } else {
        bool result = true;
        RayCastCallback rccb([&](b2Fixture* fixture, m2::Vec2f point, m2::Vec2f normal, float fraction) -> float {
            result = false;
            return 0.0f;
        }, category_bits);
        world.RayCast(&rccb, static_cast<b2Vec2>(from), static_cast<b2Vec2>(to));
        return result;
    }
}
