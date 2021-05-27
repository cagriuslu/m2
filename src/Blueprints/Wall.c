#include "../Blueprint.h"
#include "../Main.h"
#include "../Box2DWrapper.h"

int WallInit(Object *obj, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->pos = position;
	obj->txSrc = (SDL_Rect) {112, 32, 16, 16};
	obj->txSize = (Vec2F) {1.0, 1.0};

	Box2DBodyDef *bodyDef = Box2DBodyDefCreate();
	Box2DBodyDefSetPosition(bodyDef, position);
	Box2DBody *body = Box2DWorldCreateBody(CurrentWorld(), bodyDef);
	Box2DBodyDefDestroy(bodyDef);

	Box2DPolygonShape *boxShape = Box2DPolygonShapeCreate();
	Box2DPolygonShapeSetAsBox(boxShape, (Vec2F) {0.5, 0.5});
	Box2DFixture *fixture = Box2DBodyCreateFixtureFromShape(body, boxShape, 0.0);
	Box2DPolygonShapeDestroy(boxShape);

	obj->body = body;
	return 0;
}
