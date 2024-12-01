#pragma once
#include "../math/VecF.h"
#include <box2d/b2_world.h>
#include <box2d/b2_world_callbacks.h>
#include <box2d/b2_fixture.h>
#include <functional>

namespace m2::box2d {
    class RayCastCallback : public b2RayCastCallback {
        std::function<float(b2Fixture*,m2::VecF,m2::VecF,float)> m_cb;
        uint16_t m_categoryMask;
    public:
        RayCastCallback(std::function<float (b2Fixture*,m2::VecF,m2::VecF,float)>&& cb, uint16_t categoryMask);

        float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;
    };

    bool check_eyesight(b2World& world, VecF from, VecF to, uint16_t category_bits);
	float check_distance(b2World& world, VecF from, VecF to, uint16_t category_bits);
}
