#ifndef BOX2D_H
#define BOX2D_H

#include <m2/vec2f.hh>
#include "m2/AABB.hh"
#include <cstdint>

typedef void Box2DWorld;
typedef void Box2DBodyDef;
typedef void Box2DBody;
typedef void Box2DFixtureDef;
typedef void Box2DFixture;
typedef void Box2DShape;
typedef void Box2DPolygonShape;
typedef void Box2DCircleShape;
typedef void Box2DContact;
typedef void Box2DContactListener;
typedef void Box2DRayCastListener;
typedef void Box2DQueryListener;

Box2DWorld*  Box2DWorldCreate(m2::vec2f gravity);
Box2DBody*   Box2DWorldCreateBody(Box2DWorld *world, Box2DBodyDef *bodyDef);
void         Box2DWorldSetContactListener(Box2DWorld* world, Box2DContactListener* contactListener);
void         Box2DWorldStep(Box2DWorld *world, float timeStep, int velocityIterations, int positionIterations);
void         Box2DWorldRayCast(Box2DWorld* world, Box2DRayCastListener* rayCastListener, m2::vec2f point1, m2::vec2f point2);
void         Box2DWorldQuery(Box2DWorld* world, Box2DQueryListener* queryListener, AABB aabb);
void         Box2DWorldDestroyBody(Box2DWorld *world, Box2DBody *body);
void         Box2DWorldDestroy(Box2DWorld *world);

Box2DBodyDef* Box2DBodyDefCreate();
void          Box2DBodyDefSetTypeDynamic(Box2DBodyDef *bodyDef);
void          Box2DBodyDefSetPosition(Box2DBodyDef *bodyDef, m2::vec2f position);
void          Box2DBodyDefSetAllowSleep(Box2DBodyDef* bodyDef, bool flag);
void          Box2DBodyDefSetBullet(Box2DBodyDef* bodyDef, bool flag);
void          Box2DBodyDefSetUserData(Box2DBodyDef* bodyDef, void* ptr);
void          Box2DBodyDefDestroy(Box2DBodyDef *bodyDef);

Box2DFixture* Box2DBodyCreateFixtureFromFixtureDef(Box2DBody *body, Box2DFixtureDef *fixtureDef);
Box2DFixture* Box2DBodyCreateFixtureFromShape(Box2DBody *body, Box2DShape *shape, float density);
void          Box2DBodySetLinearDamping(Box2DBody *body, float linearDamping);
void          Box2DBodySetAngularDamping(Box2DBody *body, float angularDamping);
void          Box2DBodySetFixedRotation(Box2DBody *body, bool flag);
void          Box2DBodySetUserData(Box2DBody *body, void *userData);
void          Box2DBodySetTransform(Box2DBody* body, m2::vec2f position, float angle);
void          Box2DBodyApplyForceToCenter(Box2DBody *body, m2::vec2f force, bool wake);
void          Box2DBodySetLinearVelocity(Box2DBody* body, m2::vec2f velocity);
void          Box2DBodySetLinearSpeed(Box2DBody* body, float speed); // utility
void          Box2DBodySetAngularVelocity(Box2DBody* body, float omega);
m2::vec2f     Box2DBodyGetLinearVelocity(Box2DBody* body);
m2::vec2f     Box2DBodyGetPosition(Box2DBody *body);
float         Box2DBodyGetAngle(Box2DBody *body);
void*         Box2DBodyGetUserData(Box2DBody *body);
bool          Box2DBodyIsAwake(Box2DBody* body);
void          Box2DBodySetMassData(Box2DBody* body, float mass, m2::vec2f center, float inertia);
int32_t       Box2DBodyGetFixtureCount(Box2DBody* body);
Box2DFixture* Box2DBodyGetFixture(Box2DBody* body, int index);

Box2DFixtureDef* Box2DFixtureDefCreate();
void             Box2DFixtureDefSetShape(Box2DFixtureDef *fixtureDef, Box2DShape *shape);
void             Box2DFixtureDefSetDensity(Box2DFixtureDef *fixtureDef, float density);
void             Box2DFixtureDefSetFriction(Box2DFixtureDef *fixtureDef, float friction);
void             Box2DFixtureDefSetCategoryBits(Box2DFixtureDef* fixtureDef, uint16_t bits);
void             Box2DFixtureDefSetMaskBits(Box2DFixtureDef* fixtureDef, uint16_t bits);
void             Box2DFixtureDefDestroy(Box2DFixtureDef *fixtureDef);

void        Box2DFixtureSetSensor(Box2DFixture *fixture, bool flag);
Box2DBody*  Box2DFixtureGetBody(Box2DFixture* fixture);
uint16_t    Box2DFixtureGetCategory(Box2DFixture* fixture);
int32_t     Box2DFixtureGetProxyCount(Box2DFixture* fixture);
AABB        Box2DFixtureGetAABB(Box2DFixture* fixture, int32_t proxyIndex);
Box2DShape* Box2DFixtureGetShape(Box2DFixture* fixture);

Box2DPolygonShape* Box2DPolygonShapeCreate();
void               Box2DPolygonShapeSetPosition(Box2DPolygonShape *polygonShape, m2::vec2f position);
void               Box2DPolygonShapeSetAsBox(Box2DPolygonShape *polygonShape, m2::vec2f halfdims);
void               Box2DPolygonShapeSetAsBoxEx(Box2DPolygonShape* polygonShape, m2::vec2f halfDims, m2::vec2f center, float angle);
void               Box2DPolygonShapeDestroy(Box2DPolygonShape *polygonShape);

Box2DCircleShape* Box2DCircleShapeCreate();
void              Box2DCircleShapeSetRadius(Box2DCircleShape *circleShape, float radius);
void              Box2DCircleShapeDestroy(Box2DCircleShape *circleShape);

Box2DFixture* Box2DContactGetFixtureA(Box2DContact* contact);
Box2DFixture* Box2DContactGetFixtureB(Box2DContact* contact);

Box2DContactListener* Box2DContactListenerRegister(void (*cb)(Box2DContact*));
void                  Box2DContactListenerDestroy(Box2DContactListener* contactListener);

Box2DRayCastListener* Box2DRayCastListenerCreate(float (*cb)(Box2DFixture*, m2::vec2f point, m2::vec2f normal, float fraction, void* userData), uint16_t categoryMask, void* userData);
void                  Box2DRayCastListenerDestroy(Box2DRayCastListener* rayCastListener);

Box2DQueryListener* Box2DQueryListenerCreate(bool (*cb)(Box2DFixture*, void* userData), void* userData);
void                Box2DQueryListenerDestroy(Box2DQueryListener* queryListener);

#endif
