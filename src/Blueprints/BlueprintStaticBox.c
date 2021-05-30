#include "../Blueprint.h"
#include "../Main.h"
#include "../Box2DWrapper.h"
#include <stdio.h>

void StaticBox_deinit(Object* obj) {
	Box2DWorldDestroyBody(CurrentWorld(), obj->body);
}

int BlueprintStaticBoxInit(Object *obj, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->pos = position;
	obj->txSrc = (SDL_Rect) {16, 64, 16, 16};
	obj->txOffset = (Vec2F){ 0.0, -3.0 };

	Box2DBodyDef *bodyDef = Box2DBodyDefCreate();
	Box2DBodyDefSetPosition(bodyDef, position);
	Box2DBodyDefSetAllowSleep(bodyDef, true);
	Box2DBody *body = Box2DWorldCreateBody(CurrentWorld(), bodyDef);
	Box2DBodyDefDestroy(bodyDef);

	Box2DPolygonShape *boxShape = Box2DPolygonShapeCreate();
	Box2DPolygonShapeSetAsBox(boxShape, (Vec2F) {0.4375, 0.0625});
	Box2DFixture *fixture = Box2DBodyCreateFixtureFromShape(body, boxShape, 0.0);
	Box2DPolygonShapeDestroy(boxShape);
	obj->body = body;

	obj->deinit = StaticBox_deinit;
	return 0;
}
