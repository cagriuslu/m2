#include "../Object.h"
#include "../Main.h"
#include "../Box2D.h"
#include "../Box2DUtils.h"

int ObjectWall_Init(Object* obj, Vec2F position) {
	PROPAGATE_ERROR(Object_Init(obj, position, false));
	
	ID phyId = 0;
	ComponentPhysics* phy = Object_AddPhysics(obj, &phyId);
	phy->body = Box2DUtils_CreateStaticBox(phyId, position, CATEGORY_STATIC_OBJECT, ((Vec2F) { 0.875, 0.125 }));

	ComponentGraphics* gfx = Object_AddGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect){ 8 * TILE_WIDTH, 0, TILE_WIDTH, 2 * TILE_WIDTH };
	gfx->txCenter = (Vec2F){ 0.0, 12.0 };

	return 0;
}
