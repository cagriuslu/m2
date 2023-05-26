#include <m2/box2d/Query.h>
#include <m2/Game.h>

class QueryCallbackType : public b2QueryCallback {
    m2::box2d::PhysiqueQueryCallback _callback;

public:
    explicit QueryCallbackType(m2::box2d::PhysiqueQueryCallback&& cb) : _callback(cb) {}
    bool ReportFixture(b2Fixture* fixture) override {
        m2::Id physique_id = fixture->GetBody()->GetUserData().pointer;
        return (_callback)(LEVEL.physics[physique_id]);
    }
};
void m2::box2d::query(b2World& world, const AABB& aabb, PhysiqueQueryCallback&& query_callback) {
    QueryCallbackType q{std::move(query_callback)};
    world.QueryAABB(&q, static_cast<b2AABB>(aabb));
}

class FixtureQueryCallbackType : public b2QueryCallback {
	m2::box2d::FixtureQueryCallback _callback;

public:
	explicit FixtureQueryCallbackType(m2::box2d::FixtureQueryCallback&& cb) : _callback(cb) {}
	bool ReportFixture(b2Fixture* fixture) override {
		return (_callback)(*fixture);
	}
};
void m2::box2d::query(b2World& world, const AABB& aabb, FixtureQueryCallback&& query_callback) {
	FixtureQueryCallbackType q{std::move(query_callback)};
	world.QueryAABB(&q, static_cast<b2AABB>(aabb));
}

void m2::box2d::find_objects_near_position_under_mouse(VecF position, float max_distance, PhysiqueQueryCallback&& query_callback) {
	if (GAME.mouse_position_world_m().is_near(position, max_distance)) {
		m2::box2d::query(*LEVEL.world, m2::AABB{GAME.mouse_position_world_m(), 0.0001f}, std::move(query_callback));
	}
}
