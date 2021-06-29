#include "../Object.h"
#include "../Main.h"
#include "../Box2D.h"
#include "../Box2DUtils.h"
#include <stdio.h>

int ObjectStaticBoxInit(Object *obj, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj, position));
	
	ID phyId = 0;
	PhysicsComponent* phy = ObjectAddPhysics(obj, &phyId);
	phy->body = Box2DUtils_CreateStaticBox(phyId, position, CATEGORY_STATIC_OBJECT, ((Vec2F) { 0.875, 0.125 }));

	GraphicsComponent* gfx = ObjectAddGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect) {TILE_WIDTH, 4 * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };
	gfx->txCenter = (Vec2F){ 0.0, 4.5 };
	
	return 0;
}
