#include "m2/Box2D.hh"
#include <b2_world.h>
#include <b2_body.h>
#include <b2_math.h>
#include <b2_polygon_shape.h>
#include <b2_circle_shape.h>
#include <b2_fixture.h>
#include <b2_contact.h>
#include <b2_common.h>
#include <cstdio>
#include <cstdlib>

#include <m2/Def.hh>

ContactListener::ContactListener(void (*cb)(b2Contact*)) : m_cb(cb) {}

void ContactListener::BeginContact(b2Contact* contact) {
	m_cb(contact);
}

RayCastCallback::RayCastCallback(std::function<float (b2Fixture*,m2::Vec2f,m2::Vec2f,float)>&& cb, uint16_t categoryMask) : m_cb(cb), m_categoryMask(categoryMask) {}

float RayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) {
	if (fixture->GetFilterData().categoryBits & m_categoryMask) {
		return (m_cb)(fixture, m2::Vec2f{point }, m2::Vec2f{normal }, fraction);
	}
	return 1.0f;
}

QueryCallback::QueryCallback(bool (*cb)(b2Fixture*, void* userData), void* userData) : m_cb(cb), m_userData(userData) {}

bool QueryCallback::ReportFixture(b2Fixture* fixture) {
	return (*m_cb)(fixture, m_userData);
}

b2AABB m2::box2d::aabb::expand(const b2AABB& in, float amount) {
    return b2AABB{
        .lowerBound = b2Vec2{in.lowerBound.x - amount, in.lowerBound.y - amount},
        .upperBound = b2Vec2{in.upperBound.x + amount, in.upperBound.y + amount}
    };
}

//void Box2DWorldQuery(Box2DWorld* world, Box2DQueryListener* queryListener, AABB aabb) {
//	b2AABB b2dAabb;
//	b2dAabb.lowerBound.Set(aabb.lowerBound.x, aabb.lowerBound.y);
//	b2dAabb.upperBound.Set(aabb.upperBound.x, aabb.upperBound.y);
//	AsWorld(world)->QueryAABB(AsQueryCallback(queryListener), b2dAabb);
//}
