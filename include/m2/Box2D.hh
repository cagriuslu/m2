#ifndef BOX2D_H
#define BOX2D_H

#include <b2_world_callbacks.h>
#include <b2_collision.h>
#include <m2/Vec2f.h>
#include <functional>
#include <cstdint>

class ContactListener : public b2ContactListener {
	void (*m_cb)(b2Contact*);
public:
	ContactListener(void (*cb)(b2Contact*));

	void BeginContact(b2Contact* contact) override;
};

class RayCastCallback : public b2RayCastCallback {
    std::function<float (b2Fixture*,m2::Vec2f,m2::Vec2f,float)> m_cb;
	uint16_t m_categoryMask;
public:
    RayCastCallback(std::function<float (b2Fixture*,m2::Vec2f,m2::Vec2f,float)>&& cb, uint16_t categoryMask);

	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;
};

class QueryCallback : public b2QueryCallback {
	bool (*m_cb)(b2Fixture*, void* userData);
	void* m_userData;
public:
	QueryCallback(bool (*cb)(b2Fixture*, void* userData), void* userData);

	bool ReportFixture(b2Fixture* fixture) override;
};

namespace m2::box2d::aabb {
	b2AABB expand(const b2AABB& in, float amount);
}

#endif
