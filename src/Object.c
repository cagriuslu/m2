#include "Object.h"
#include "Main.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

void Object_ovrdGraphics(Object* obj) {
	if (obj->tx) {
		Object* camera = ArrayGet(CurrentObjectArray(), CAMERA_INDEX);

		Vec2F obj_origin_wrt_camera_obj = Vec2FSub(obj->pos, camera->pos);
		Vec2I obj_origin_wrt_screen_center = Vec2Fto2I(Vec2FMul(obj_origin_wrt_camera_obj, CurrentPixelsPerMeter()));
		Vec2I obj_gfx_origin_wrt_screen_center = Vec2IAdd(obj_origin_wrt_screen_center, (Vec2I) {
			(int) round(obj->txOffset.x * CurrentPixelsPerMeter()) / CurrentTileWidth(),
			(int) round(obj->txOffset.y * CurrentPixelsPerMeter()) / CurrentTileWidth()
		});
		Vec2I obj_gfx_origin_wrt_screen_origin = Vec2IAdd((Vec2I) { CurrentScreenWidth() / 2, CurrentScreenHeight() / 2 }, obj_gfx_origin_wrt_screen_center);
		SDL_Rect dstrect = (SDL_Rect){
			obj_gfx_origin_wrt_screen_origin.x - (int) round(obj->txSrc.w * CurrentPixelsPerMeter()) / CurrentTileWidth() / 2,
			obj_gfx_origin_wrt_screen_origin.y - (int) round(obj->txSrc.h * CurrentPixelsPerMeter()) / CurrentTileWidth() / 2,
			(int) round(obj->txSrc.w * CurrentPixelsPerMeter()) / CurrentTileWidth(),
			(int) round(obj->txSrc.h * CurrentPixelsPerMeter()) / CurrentTileWidth()
		};
		SDL_RenderCopyEx(CurrentRenderer(), obj->tx, &obj->txSrc, &dstrect, obj->angle * 180.0 / M_PI, NULL, SDL_FLIP_NONE);
	}
}

int ObjectInit(Object *obj) {
	memset(obj, 0, sizeof(Object));
	obj->tx = CurrentTextureLUT();
	obj->ovrdGraphics = Object_ovrdGraphics;
	return 0;
}

void ObjectDeinit(Object *obj) {
	if (obj->deinit) {
		obj->deinit(obj);
	}
	// Delete common members
}

void ObjectContactCB(Box2DContact* contact) {
	fprintf(stderr, "Contact beginned\n");
	
	ObjectType* aType = Box2DBodyGetUserData(Box2DFixtureGetBody(Box2DContactGetFixtureA(contact)));
	ObjectType* bType = Box2DBodyGetUserData(Box2DFixtureGetBody(Box2DContactGetFixtureB(contact)));

	if (IS_OBJECT(*aType)) {
		Object* a = (Object*) aType;
		if (a->onCollision) {
			a->onCollision(a, bType);
		}
	}

	if (IS_OBJECT(*bType)) {
		Object* b = (Object*) bType;
		if (b->onCollision) {
			b->onCollision(b, aType);
		}
	}
}
