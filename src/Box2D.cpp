#include "Box2D.h"
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

extern "C" {
#include "Log.h"
}

#define Vec2FToB2Vec2(vec2f) (b2Vec2{vec2f.x, vec2f.y})
#define B2Vec2ToVec2F(b2vec2) (Vec2F{b2vec2.x, b2vec2.y})

#define AsWorld(world) ((b2World*) (world))
#define AsBodyDef(bodyDef) ((b2BodyDef*) (bodyDef))
#define AsBody(body) ((b2Body*) (body))
#define AsFixtureDef(fixtureDef) ((b2FixtureDef*) (fixtureDef))
#define AsFixture(fixture) ((b2Fixture*) (fixture))
#define AsShape(shape) ((b2Shape*) (shape))
#define AsPolygonShape(polygonShape) ((b2PolygonShape*) (polygonShape))
#define AsCircleShape(circleShape) ((b2CircleShape*) (circleShape))
#define AsContact(contact) ((b2Contact*) (contact))
#define AsAABB(aabb) ((b2AABB*) (aabb))
#define AsContactListener(contactListener) ((ContactListener*) (contactListener))
#define AsRayCastCallback(contactListener) ((RayCastCallback*) (contactListener))
#define AsQueryCallback(queryListener) ((QueryCallback*) (queryListener))

class ContactListener : public b2ContactListener {
	void (*m_cb)(Box2DContact*);
public:
	ContactListener(void (*cb)(Box2DContact*)) : m_cb(cb) {}

	void BeginContact(b2Contact* contact) override {
		m_cb(contact);
	}
};

class RayCastCallback : public b2RayCastCallback {
	float (*m_cb)(Box2DFixture*, Vec2F point, Vec2F normal, float fraction, void* userData);
	uint16_t m_categoryMask;
	void* m_userData;
public:
	RayCastCallback(float (*cb)(Box2DFixture*, Vec2F point, Vec2F normal, float fraction, void* userData), uint16_t categoryMask, void* userData) : m_cb(cb), m_categoryMask(categoryMask), m_userData(userData) {}

	float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override {
		if (Box2DFixtureGetCategory(fixture) & m_categoryMask) {
			return (*m_cb)(fixture, B2Vec2ToVec2F(point), B2Vec2ToVec2F(normal), fraction, m_userData);
		} else {
			return 1.0f;
		}
		
	}
};

class QueryCallback : public b2QueryCallback {
	bool (*m_cb)(Box2DFixture*, void* userData);
	void* m_userData;
public:
	QueryCallback(bool (*cb)(Box2DFixture*, void* userData), void* userData) : m_cb(cb), m_userData(userData) {}

	bool ReportFixture(b2Fixture* fixture) override {
		return (*m_cb)(fixture, m_userData);
	}
};

Box2DWorld* Box2DWorldCreate(Vec2F gravity) {
	if (b2_version.major != 2 || b2_version.minor != 4 || b2_version.revision != 0) {
		LOG_FTL("Box2D version mismatch");
		abort();
	}
	return new b2World(Vec2FToB2Vec2(gravity));
}

Box2DBody* Box2DWorldCreateBody(Box2DWorld *world, Box2DBodyDef *bodyDef) {
	return AsWorld(world)->CreateBody(AsBodyDef(bodyDef));
}

void Box2DWorldSetContactListener(Box2DWorld* world, Box2DContactListener* contactListener) {
	AsWorld(world)->SetContactListener(AsContactListener(contactListener));
}

void Box2DWorldStep(Box2DWorld *world, float timeStep, int velocityIterations, int positionIterations) {
	AsWorld(world)->Step(timeStep, velocityIterations, positionIterations);
}

void Box2DWorldRayCast(Box2DWorld* world, Box2DRayCastListener* rayCastListener, Vec2F point1F, Vec2F point2F) {
	assert(!Vec2F_Equals(point1F, point2F));
	const b2Vec2 point1 = Vec2FToB2Vec2(point1F);
	const b2Vec2 point2 = Vec2FToB2Vec2(point2F);
	AsWorld(world)->RayCast(AsRayCastCallback(rayCastListener), point1, point2);
}

void Box2DWorldQuery(Box2DWorld* world, Box2DQueryListener* queryListener, AABB aabb) {
	b2AABB b2dAabb;
	b2dAabb.lowerBound.Set(aabb.lowerBound.x, aabb.lowerBound.y);
	b2dAabb.upperBound.Set(aabb.upperBound.x, aabb.upperBound.y);
	AsWorld(world)->QueryAABB(AsQueryCallback(queryListener), b2dAabb);
}

void Box2DWorldDestroyBody(Box2DWorld *world, Box2DBody *body) {
	AsWorld(world)->DestroyBody(AsBody(body));
}

void Box2DWorldDestroy(Box2DWorld *world) {
	delete AsWorld(world);
}

Box2DBodyDef* Box2DBodyDefCreate() {
	return new b2BodyDef();
}

void Box2DBodyDefSetTypeDynamic(Box2DBodyDef *bodyDef) {
	AsBodyDef(bodyDef)->type = b2_dynamicBody;
}

void Box2DBodyDefSetPosition(Box2DBodyDef *bodyDef, Vec2F position) {
	AsBodyDef(bodyDef)->position.Set(position.x, position.y);
}

void Box2DBodyDefSetAllowSleep(Box2DBodyDef* bodyDef, bool flag) {
	AsBodyDef(bodyDef)->allowSleep = flag;
}

void Box2DBodyDefSetBullet(Box2DBodyDef* bodyDef, bool flag) {
	AsBodyDef(bodyDef)->bullet = flag;
}

void Box2DBodyDefSetUserData(Box2DBodyDef* bodyDef, void* ptr) {
	AsBodyDef(bodyDef)->userData.pointer = reinterpret_cast<uintptr_t>(ptr);
}

void Box2DBodyDefDestroy(Box2DBodyDef *bodyDef) {
	delete AsBodyDef(bodyDef);
}

Box2DFixture* Box2DBodyCreateFixtureFromFixtureDef(Box2DBody *body, Box2DFixtureDef *fixtureDef) {
	return AsBody(body)->CreateFixture(AsFixtureDef(fixtureDef));
}

Box2DFixture* Box2DBodyCreateFixtureFromShape(Box2DBody *body, Box2DShape *shape, float density) {
	return AsBody(body)->CreateFixture(AsShape(shape), density);
}

void Box2DBodySetLinearDamping(Box2DBody *body, float linearDamping) {
	AsBody(body)->SetLinearDamping(linearDamping);
}

void Box2DBodySetAngularDamping(Box2DBody *body, float angularDamping) {
	AsBody(body)->SetAngularDamping(angularDamping);
}

void Box2DBodySetFixedRotation(Box2DBody *body, bool flag) {
	AsBody(body)->SetFixedRotation(flag);
}

void Box2DBodySetUserData(Box2DBody *body, void *userData) {
	AsBody(body)->GetUserData().pointer = (uintptr_t) userData;
}

void Box2DBodySetTransform(Box2DBody *body, Vec2F position, float angle) {
	AsBody(body)->SetTransform(Vec2FToB2Vec2(position), angle);
}

void Box2DBodyApplyForceToCenter(Box2DBody *body, Vec2F force, bool wake) {
	AsBody(body)->ApplyForceToCenter(Vec2FToB2Vec2(force), wake);
}

void Box2DBodySetLinearVelocity(Box2DBody* body, Vec2F velocity) {
	AsBody(body)->SetLinearVelocity(Vec2FToB2Vec2(velocity));
}

void Box2DBodySetAngularVelocity(Box2DBody* body, float omega) {
	AsBody(body)->SetAngularVelocity(omega);
}

Vec2F Box2DBodyGetLinearVelocity(Box2DBody* body) {
	b2Vec2 v = AsBody(body)->GetLinearVelocity();
	Vec2F vec2f = { v.x, v.y };
	return vec2f;
}

Vec2F Box2DBodyGetPosition(Box2DBody *body) {
	b2Vec2 v = AsBody(body)->GetPosition();
	Vec2F vec2f = {v.x, v.y};
	return vec2f;
}

float Box2DBodyGetAngle(Box2DBody *body) {
	return AsBody(body)->GetAngle();
}

void* Box2DBodyGetUserData(Box2DBody *body) {
	return (void*) AsBody(body)->GetUserData().pointer;
}

bool Box2DBodyIsAwake(Box2DBody* body) {
	return AsBody(body)->IsAwake();
}

void Box2DBodySetMassData(Box2DBody* body, float mass, Vec2F center, float inertia) {
	b2MassData massData = { mass, {center.x, center.y}, inertia };
	AsBody(body)->SetMassData(&massData);
}

int32_t Box2DBodyGetFixtureCount(Box2DBody* body) {
	// This is a hack to extract m_fixtureCount
	class DummyBody
	{
	public:
		b2BodyType m_type;
		uint16 m_flags;
		int32 m_islandIndex;
		b2Transform m_xf;
		b2Sweep m_sweep;
		b2Vec2 m_linearVelocity;
		float m_angularVelocity;
		b2Vec2 m_force;
		float m_torque;
		b2World* m_world;
		b2Body* m_prev;
		b2Body* m_next;
		b2Fixture* m_fixtureList;
		int32 m_fixtureCount;
		b2JointEdge* m_jointList;
		b2ContactEdge* m_contactList;
		float m_mass, m_invMass;
		float m_I, m_invI;
		float m_linearDamping;
		float m_angularDamping;
		float m_gravityScale;
		float m_sleepTime;
		b2BodyUserData m_userData;
	};
	return reinterpret_cast<DummyBody*>(AsBody(body))->m_fixtureCount;
}

Box2DFixture* Box2DBodyGetFixture(Box2DBody* body, int index) {
	return AsBody(body)->GetFixtureList() + index;
}

Box2DFixtureDef* Box2DFixtureDefCreate() {
	return new b2FixtureDef();
}

void Box2DFixtureDefSetShape(Box2DFixtureDef *fixtureDef, Box2DShape *shape) {
	AsFixtureDef(fixtureDef)->shape = AsShape(shape);
}

void Box2DFixtureDefSetDensity(Box2DFixtureDef *fixtureDef, float density) {
	AsFixtureDef(fixtureDef)->density = density;
}

void Box2DFixtureDefSetFriction(Box2DFixtureDef *fixtureDef, float friction) {
	AsFixtureDef(fixtureDef)->friction = friction;
}

void Box2DFixtureDefSetCategoryBits(Box2DFixtureDef* fixtureDef, uint16_t bits) {
	AsFixtureDef(fixtureDef)->filter.categoryBits = bits;
}

void Box2DFixtureDefSetMaskBits(Box2DFixtureDef* fixtureDef, uint16_t bits) {
	AsFixtureDef(fixtureDef)->filter.maskBits = bits;
}

void Box2DFixtureDefDestroy(Box2DFixtureDef *fixtureDef) {
	delete AsFixtureDef(fixtureDef);
}

void Box2DFixtureSetSensor(Box2DFixture *fixture, bool flag) {
	AsFixture(fixture)->SetSensor(flag);
}

Box2DBody* Box2DFixtureGetBody(Box2DFixture* fixture) {
	return AsFixture(fixture)->GetBody();
}

uint16_t Box2DFixtureGetCategory(Box2DFixture* fixture) {
	return AsFixture(fixture)->GetFilterData().categoryBits;
}

int32_t Box2DFixtureGetProxyCount(Box2DFixture* fixture) {
	class DummyFixture
	{
	public:
		float m_density;
		b2Fixture* m_next;
		b2Body* m_body;
		b2Shape* m_shape;
		float m_friction;
		float m_restitution;
		float m_restitutionThreshold;
		b2FixtureProxy* m_proxies;
		int32 m_proxyCount;
		b2Filter m_filter;
		bool m_isSensor;
		b2FixtureUserData m_userData;
	};
	return reinterpret_cast<DummyFixture*>(AsFixture(fixture))->m_proxyCount;
}

AABB Box2DFixtureGetAABB(Box2DFixture* fixture, int32_t proxyIndex) {
	const auto& aabb = AsFixture(fixture)->GetAABB(proxyIndex);
	return AABB{ Vec2F{aabb.lowerBound.x, aabb.lowerBound.y}, Vec2F{aabb.upperBound.x, aabb.upperBound.y}};
}

Box2DShape* Box2DFixtureGetShape(Box2DFixture* fixture) {
	return AsFixture(fixture)->GetShape();
}

Box2DPolygonShape* Box2DPolygonShapeCreate() {
	return new b2PolygonShape();
}

void Box2DPolygonShapeSetAsBox(Box2DPolygonShape *polygonShape, Vec2F halfdims) {
	AsPolygonShape(polygonShape)->SetAsBox(halfdims.x, halfdims.y);
}

void Box2DPolygonShapeSetAsBoxEx(Box2DPolygonShape* polygonShape, Vec2F halfDims, Vec2F center, float angle) {
	AsPolygonShape(polygonShape)->SetAsBox(halfDims.x, halfDims.y, b2Vec2{center.x, center.y}, angle);
}

void Box2DPolygonShapeDestroy(Box2DPolygonShape *polygonShape) {
	delete AsPolygonShape(polygonShape);
}

Box2DCircleShape* Box2DCircleShapeCreate() {
	return new b2CircleShape();
}

void Box2DCircleShapeSetRadius(Box2DCircleShape *circleShape, float radius) {
	AsCircleShape(circleShape)->m_radius = radius;
}

void Box2DCircleShapeDestroy(Box2DCircleShape *circleShape) {
	delete AsCircleShape(circleShape);
}

Box2DFixture* Box2DContactGetFixtureA(Box2DContact* contact) {
	return AsContact(contact)->GetFixtureA();
}

Box2DFixture* Box2DContactGetFixtureB(Box2DContact* contact) {
	return AsContact(contact)->GetFixtureB();
}

Box2DContactListener* Box2DContactListenerRegister(void (*cb)(Box2DContact*)) {
	return new ContactListener(cb);
}

void Box2DContactListenerDestroy(Box2DContactListener* contactListener) {
	delete AsContactListener(contactListener);
}

Box2DRayCastListener* Box2DRayCastListenerCreate(float (*cb)(Box2DFixture*, Vec2F point, Vec2F normal, float fraction, void* userData), uint16_t categoryMask, void* userData) {
	return new RayCastCallback(cb, categoryMask, userData);
}

void Box2DRayCastListenerDestroy(Box2DRayCastListener* rayCastListener) {
	delete AsRayCastCallback(rayCastListener);
}

Box2DQueryListener* Box2DQueryListenerCreate(bool (*cb)(Box2DFixture*, void* userData), void* userData) {
	return new QueryCallback(cb, userData);
}

void Box2DQueryListenerDestroy(Box2DQueryListener* queryListener) {
	delete AsQueryCallback(queryListener);
}
