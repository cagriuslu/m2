#include "../Object.h"
#include "../Main.h"
#include "../Box2DWrapper.h"
#include "../Box2DUtils.h"

int ObjectWallInit(Object* obj, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj, position));
	
	uint32_t phyId = 0;
	PhysicsComponent* phy = ObjectAddAndInitPhysics(obj, &phyId);
	phy->body = Box2DUtilsCreateStaticBox(phyId, position, STATIC_OBJECT_CATEGORY, ((Vec2F) { 0.875, 0.125 }));

	GraphicsComponent* gfx = ObjectAddAndInitGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect){ 8 * TILE_WIDTH, 0, TILE_WIDTH, 2 * TILE_WIDTH };
	gfx->txOffset = (Vec2F){ 0.0, -12.0 };

	return 0;
}
