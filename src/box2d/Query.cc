#include <m2/box2d/Query.h>
#include <m2/Game.hh>

class QueryCallbackType : public b2QueryCallback {


    m2::box2d::QueryCallback _callback;

public:
    explicit QueryCallbackType(m2::box2d::QueryCallback&& cb) : _callback(cb) {}

    bool ReportFixture(b2Fixture* fixture) override {
        m2::Id physique_id = fixture->GetBody()->GetUserData().pointer;
        return (_callback)(GAME.physics[physique_id]);
    }
};

void m2::box2d::query(b2World& world, const Aabb2f& aabb, m2::box2d::QueryCallback&& query_callback) {
    QueryCallbackType q{std::move(query_callback)};
    world.QueryAABB(&q, static_cast<b2AABB>(aabb));
}
