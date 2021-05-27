#ifndef BOX2D_WRAPPER_H
#define BOX2D_WRAPPER_H

#include "Vec2F.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void Box2DWorld;
typedef void Box2DBodyDef;
typedef void Box2DBody;
typedef void Box2DFixtureDef;
typedef void Box2DFixture;
typedef void Box2DShape;
typedef void Box2DPolygonShape;
typedef void Box2DCircleShape;

Box2DWorld*  Box2DWorldCreate(Vec2F gravity);
Box2DBody*   Box2DWorldCreateBody(Box2DWorld *world, Box2DBodyDef *bodyDef);
void         Box2DWorldStep(Box2DWorld *world, float timeStep, int velocityIterations, int positionIterations);
void         Box2DWorldDestroyBody(Box2DWorld *world, Box2DBody *body);
void         Box2DWorldDestroy(Box2DWorld *world);

Box2DBodyDef* Box2DBodyDefCreate();
void          Box2DBodyDefSetTypeDynamic(Box2DBodyDef *bodyDef);
void          Box2DBodyDefSetPosition(Box2DBodyDef *bodyDef, Vec2F position);
void          Box2DBodyDefDestroy(Box2DBodyDef *bodyDef);

Box2DFixture* Box2DBodyCreateFixtureFromFixtureDef(Box2DBody *body, Box2DFixtureDef *fixtureDef);
Box2DFixture* Box2DBodyCreateFixtureFromShape(Box2DBody *body, Box2DShape *shape, float density);
void          Box2DBodySetLinearDamping(Box2DBody *body, float linearDamping);
void          Box2DBodySetAngularDamping(Box2DBody *body, float angularDamping);
void          Box2DBodySetFixedRotation(Box2DBody *body, bool flag);
void          Box2DBodySetUserData(Box2DBody *body, void *userData);
void          Box2DBodyApplyForceToCenter(Box2DBody *body, Vec2F force, bool wake);
Vec2F         Box2DBodyGetPosition(Box2DBody *body);
float         Box2DBodyGetAngle(Box2DBody *body);
void*         Box2DBodyGetUserData(Box2DBody *body);

Box2DFixtureDef* Box2DFixtureDefCreate();
void             Box2DFixtureDefSetShape(Box2DFixtureDef *fixtureDef, Box2DShape *shape);
void             Box2DFixtureDefSetDensity(Box2DFixtureDef *fixtureDef, float density);
void             Box2DFixtureDefSetFriction(Box2DFixtureDef *fixtureDef, float friction);
void             Box2DFixtureDefDestroy(Box2DFixtureDef *fixtureDef);

void Box2DFixtureSetSensor(Box2DFixture *fixture, bool flag);

Box2DPolygonShape* Box2DPolygonShapeCreate();
void               Box2DPolygonShapeSetPosition(Box2DPolygonShape *polygonShape, Vec2F position);
void               Box2DPolygonShapeSetAsBox(Box2DPolygonShape *polygonShape, Vec2F halfdims);
void               Box2DPolygonShapeDestroy(Box2DPolygonShape *polygonShape);

Box2DCircleShape* Box2DCircleShapeCreate();
void              Box2DCircleShapeSetRadius(Box2DCircleShape *circleShape, float radius);
void              Box2DCircleShapeDestroy(Box2DCircleShape *circleShape);

#ifdef __cplusplus
}
#endif

#endif
