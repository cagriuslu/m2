#include <m2/box2d/Query.h>

class QueryCallback : public b2QueryCallback {
    std::function<bool(b2Fixture&)> m_cb;
public:
    QueryCallback(std::function<bool(b2Fixture&)>&& cb) : m_cb(cb) {}

    bool ReportFixture(b2Fixture* fixture) override {
        return (m_cb)(*fixture);
    }
};

void m2::box2d::query(b2World& world, const b2AABB& aabb, std::function<bool(b2Fixture&)>&& query_callback) {
    QueryCallback qcb(std::move(query_callback));
    world.QueryAABB(&qcb, aabb);
}
