#ifndef M2_RAYCAST_H
#define M2_RAYCAST_H

#include "../Vec2f.h"
#include <box2d/b2_world.h>
#include <box2d/b2_world_callbacks.h>
#include <box2d/b2_fixture.h>
#include <functional>

namespace m2::box2d {
    class RayCastCallback : public b2RayCastCallback {
        std::function<float(b2Fixture*,m2::Vec2f,m2::Vec2f,float)> m_cb;
        uint16_t m_categoryMask;
    public:
        RayCastCallback(std::function<float (b2Fixture*,m2::Vec2f,m2::Vec2f,float)>&& cb, uint16_t categoryMask);

        float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;
    };

    bool check_eyesight(b2World& world, Vec2f from, Vec2f to, uint16_t category_bits);
	float check_distance(b2World& world, Vec2f from, Vec2f to, uint16_t category_bits);
}

#endif //M2_RAYCAST_H
