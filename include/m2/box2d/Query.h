#ifndef M2_QUERY_H
#define M2_QUERY_H

#include <box2d/b2_world.h>
#include <box2d/b2_world_callbacks.h>
#include <box2d/b2_fixture.h>
#include <m2/Aabb2f.h>
#include <m2/component/Physique.h>
#include <functional>

namespace m2::box2d {
    // QueryCallback should return false to terminate the query
    using QueryCallback = std::function<bool(Physique&)>;

    void query(b2World& world, const Aabb2f& aabb, QueryCallback&& query_callback);
}

#endif //M2_QUERY_H
