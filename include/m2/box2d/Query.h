#ifndef M2_QUERY_H
#define M2_QUERY_H

#include <b2_world.h>
#include <b2_world_callbacks.h>
#include <b2_fixture.h>
#include <functional>

namespace m2::box2d {
    void query(b2World& world, const b2AABB& aabb, std::function<bool(b2Fixture&)>&& query_callback);
}

#endif //M2_QUERY_H
