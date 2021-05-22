#ifndef BOX2D_WRAPPER_H
#define BOX2D_WRAPPER_H

#include "Vec2F.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void Box2DWorld;
typedef void Box2DBodyDef;
typedef void Box2DBody;
typedef void Box2DFixture;
typedef void Box2DShape;
typedef void Box2DPolygonShape;

Box2DWorld*  Box2DWorldCreate(Vec2F gravity);
Box2DBody*   Box2DWorldCreateBody(Box2DWorld *world, Box2DBodyDef *bodyDef);
void         Box2DWorldDestroyBody(Box2DWorld *world, Box2DBody *body);
void         Box2DWorldDestroy(Box2DWorld *world);

Box2DBodyDef* Box2DBodyDefCreate();
void          Box2DBodyDefSetTypeDynamic(Box2DBodyDef *bodyDef);
void          Box2DBodyDefSetPosition(Box2DBodyDef *bodyDef, Vec2F position);
void          Box2DBodyDefDestroy(Box2DBodyDef *bodyDef);

Box2DFixture* Box2DBodyCreateFixtureFromShape(Box2DBody *body, Box2DShape *shape, float density);

Box2DPolygonShape* Box2DPolygonShapeCreate();
void               Box2DPolygonShapeSetAsBox(Box2DPolygonShape *polygonShape, Vec2F halfdims);
void               Box2DPolygonShapeDestroy(Box2DPolygonShape *polygonShape);

#ifdef __cplusplus
}
#endif

#endif
