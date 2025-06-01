#include <m2/box2d/Query.h>
#include <m2/Game.h>

class QueryCallbackType : public b2QueryCallback {
    m2::box2d::PhysiqueQueryCallback _callback;

public:
    explicit QueryCallbackType(m2::box2d::PhysiqueQueryCallback&& cb) : _callback(cb) {}
    bool ReportFixture(b2Fixture* fixture) override {
        m2::Id physique_id = fixture->GetBody()->GetUserData().pointer;
        return (_callback)(M2_LEVEL.physics[physique_id]);
    }
};
void m2::box2d::Query(b2World& world, const Aabb& aabb, PhysiqueQueryCallback&& query_callback) {
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
void m2::box2d::Query(b2World& world, const Aabb& aabb, FixtureQueryCallback&& query_callback) {
	FixtureQueryCallbackType q{std::move(query_callback)};
	world.QueryAABB(&q, static_cast<b2AABB>(aabb));
}

void m2::box2d::FindObjectsNearPositionUnderMouse(VecF position, float max_distance, PhysiqueQueryCallback&& query_callback) {
	if (M2_GAME.MousePositionWorldM().is_near(position, max_distance)) {
		m2::box2d::Query(*M2_LEVEL.world[I(PhysicsLayer::P0)], m2::Aabb{M2_GAME.MousePositionWorldM(), 0.0001f}, std::move(query_callback));
	}
}
