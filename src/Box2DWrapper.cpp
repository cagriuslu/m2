#include "Box2DWrapper.h"
#include <b2_world.h>
#include <b2_body.h>
#include <b2_math.h>
#include <b2_polygon_shape.h>
#include <b2_circle_shape.h>
#include <b2_fixture.h>
#include <b2_contact.h>

#define ToVec2(vec2f) (b2Vec2{vec2f.x, vec2f.y})

#define AsWorld(world) ((b2World*) (world))
#define AsBodyDef(bodyDef) ((b2BodyDef*) (bodyDef))
#define AsBody(body) ((b2Body*) (body))
#define AsFixtureDef(fixtureDef) ((b2FixtureDef*) (fixtureDef))
#define AsFixture(fixture) ((b2Fixture*) (fixture))
#define AsShape(shape) ((b2Shape*) (shape))
#define AsPolygonShape(polygonShape) ((b2PolygonShape*) (polygonShape))
#define AsCircleShape(circleShape) ((b2CircleShape*) (circleShape))
#define AsContact(contact) ((b2Contact*) (contact))
#define AsContactListener(contactListener) ((ContactListener*) (contactListener))

class ContactListener : public b2ContactListener {
	void (*m_cb)(Box2DContact*);
public:
	ContactListener(void (*cb)(Box2DContact*)) : m_cb(cb) { }

	void BeginContact(b2Contact* contact) {
		m_cb(contact);
	}
};

Box2DWorld* Box2DWorldCreate(Vec2F gravity) {
	return new b2World(ToVec2(gravity));
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

void Box2DBodyApplyForceToCenter(Box2DBody *body, Vec2F force, bool wake) {
	AsBody(body)->ApplyForceToCenter(ToVec2(force), wake);
}

void Box2DBodySetLinearVelocity(Box2DBody* body, Vec2F velocity) {
	AsBody(body)->SetLinearVelocity(ToVec2(velocity));
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

Box2DPolygonShape* Box2DPolygonShapeCreate() {
	return new b2PolygonShape();
}

void Box2DPolygonShapeSetAsBox(Box2DPolygonShape *polygonShape, Vec2F halfdims) {
	AsPolygonShape(polygonShape)->SetAsBox(halfdims.x, halfdims.y);
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
