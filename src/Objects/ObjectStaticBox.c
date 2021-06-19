#include "../Object.h"
#include "../Main.h"
#include "../Box2DWrapper.h"
#include "../Box2DUtils.h"
#include <stdio.h>

int ObjectStaticBoxInit(Object *obj, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj, position));
	
	uint64_t phyId = 0;
	PhysicsComponent* phy = ObjectAddAndInitPhysics(obj, &phyId);
	phy->body = Box2DUtilsCreateStaticBox(phyId, position, STATIC_OBJECT_CATEGORY, ((Vec2F) { 0.875, 0.125 }));

	GraphicsComponent* gfx = ObjectAddAndInitGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect) {TILE_WIDTH, 4 * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };
	gfx->txOffset = (Vec2F){ 0.0, -4.5 };
	
	return 0;
}
