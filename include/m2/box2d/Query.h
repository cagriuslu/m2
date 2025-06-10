#pragma once
#include <box2d/b2_world.h>
#include <box2d/b2_fixture.h>
#include <m2/math/AABB.h>
#include <m2/component/Physique.h>
#include <m2/math/VecF.h>
#include <m2/Controls.h>
#include <functional>

namespace m2::box2d {
    /// QueryCallback should return false to terminate the query
    using PhysiqueQueryCallback = std::function<bool(Physique&)>;
    using FixtureQueryCallback = std::function<bool(b2Fixture&)>;
	/// Query the given world for physics objects
    void Query(b2World& world, const AABB& aabb, PhysiqueQueryCallback&& query_callback);
    void Query(b2World& world, const AABB& aabb, FixtureQueryCallback&& query_callback);

	/// Query the default world for physics objects near a position under the mouse
	void FindObjectsNearPositionUnderMouse(VecF position, float max_distance, PhysiqueQueryCallback&& query_callback);
}
