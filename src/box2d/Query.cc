#include <m2/box2d/Query.h>
#include <m2/Game.h>

class QueryCallbackType : public b2QueryCallback {


    m2::box2d::QueryCallback _callback;

public:
    explicit QueryCallbackType(m2::box2d::QueryCallback&& cb) : _callback(cb) {}

    bool ReportFixture(b2Fixture* fixture) override {
        m2::Id physique_id = fixture->GetBody()->GetUserData().pointer;
        return (_callback)(LEVEL.physics[physique_id]);
    }
};

void m2::box2d::query(b2World& world, const Aabb2f& aabb, m2::box2d::QueryCallback&& query_callback) {
    QueryCallbackType q{std::move(query_callback)};
    world.QueryAABB(&q, static_cast<b2AABB>(aabb));
}

void m2::box2d::find_objects_near_position_under_mouse(Vec2f position, float max_distance, QueryCallback&& query_callback) {
	if (GAME.mousePositionWRTGameWorld_m.is_near(position, max_distance)) {
		m2::box2d::query(*LEVEL.world, m2::Aabb2f{GAME.mousePositionWRTGameWorld_m, 0.0001f}, std::move(query_callback));
	}
}
