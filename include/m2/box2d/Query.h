#pragma once
#include <box2d/b2_world.h>
#include <box2d/b2_world_callbacks.h>
#include <box2d/b2_fixture.h>
#include <m2/Aabb2f.h>
#include <m2/component/Physique.h>
#include <m2/Vec2f.h>
#include <m2/Controls.h>
#include <functional>

namespace m2::box2d {
    /// QueryCallback should return false to terminate the query
    using PhysiqueQueryCallback = std::function<bool(Physique&)>;
    using FixtureQueryCallback = std::function<bool(b2Fixture&)>;
	/// Query the given world for physics objects
    void query(b2World& world, const Aabb2f& aabb, PhysiqueQueryCallback&& query_callback);
    void query(b2World& world, const Aabb2f& aabb, FixtureQueryCallback&& query_callback);

	/// Query the default world for physics objects near a position under the mouse
	void find_objects_near_position_under_mouse(Vec2f position, float max_distance, PhysiqueQueryCallback&& query_callback);
}
