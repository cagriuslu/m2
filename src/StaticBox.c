#include "StaticBox.h"
#include "Main.h"
#include "Box2DWrapper.h"

static void StaticBox_ovrdGraphics(Object *obj) {
	// Draw a white box
	SDL_SetRenderDrawColor(CurrentRenderer(), 255, 255, 255, 255);
	SDL_Rect rect = (SDL_Rect) {
		0,
		0,
		(int32_t) round(obj->txSrc.w * obj->txScaleW),
		(int32_t) round(obj->txSrc.h * obj->txScaleH)
	};
	SDL_RenderFillRect(CurrentRenderer(), &rect);
}

int StaticBoxInit(Object *obj, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->pos = position;
	obj->txSrc.w = 20;
	obj->txSrc.h = 20;
	obj->ovrdGraphics = StaticBox_ovrdGraphics;

	Box2DBodyDef *bodyDef = Box2DBodyDefCreate();
	Box2DBodyDefSetPosition(bodyDef, position);
	Box2DBody *body = Box2DWorldCreateBody(CurrentWorld(), bodyDef);
	Box2DBodyDefDestroy(bodyDef);

	Box2DPolygonShape *boxShape = Box2DPolygonShapeCreate();
	Box2DPolygonShapeSetAsBox(boxShape, (Vec2F) {0.5, 0.5});
	Box2DFixture *fixture = Box2DBodyCreateFixtureFromShape(body, boxShape, 0.0);
	Box2DPolygonShapeDestroy(boxShape);

	obj->privData = body;
	return 0;
}

void StaticBoxDeinit(Object *obj) {
	Box2DWorldDestroyBody(CurrentWorld(), obj->privData);
	ObjectDeinit(obj);
}
