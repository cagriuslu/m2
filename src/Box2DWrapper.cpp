#include "Box2DWrapper.h"
#include <b2_world.h>
#include <b2_body.h>
#include <b2_math.h>
#include <b2_polygon_shape.h>

#define ToVec2(vec2f) (b2Vec2{vec2f.x, vec2f.y})

#define AsWorld(world) ((b2World*) (world))
#define AsBodyDef(bodyDef) ((b2BodyDef*) (bodyDef))
#define AsBody(body) ((b2Body*) (body))
#define AsFixture(fixture) ((b2Fixture*) (fixture))
#define AsShape(shape) ((b2Shape*) (shape))
#define AsPolygonShape(polygonShape) ((b2PolygonShape*) (polygonShape))

Box2DWorld* Box2DWorldCreate(Vec2F gravity) {
	return new b2World(ToVec2(gravity));
}

Box2DBody* Box2DWorldCreateBody(Box2DWorld *world, Box2DBodyDef *bodyDef) {
	return AsWorld(world)->CreateBody(AsBodyDef(bodyDef));
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

void Box2DBodyDefDestroy(Box2DBodyDef *bodyDef) {
	delete AsBodyDef(bodyDef);
}

Box2DFixture* Box2DBodyCreateFixtureFromShape(Box2DBody *body, Box2DShape *shape, float density) {
	return AsBody(body)->CreateFixture(AsShape(shape), density);
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
