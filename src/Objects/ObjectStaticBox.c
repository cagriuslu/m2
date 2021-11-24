#include "../Object.h"
#include "../Main.h"
#include "../Box2D.h"
#include "../Box2DUtils.h"
#include <stdio.h>

int ObjectStaticBox_Init(Object *obj, Vec2F position) {
	PROPAGATE_ERROR(Object_Init(obj, position, false));
	
	ID phyId = 0;
	ComponentPhysics* phy = Object_AddPhysics(obj, &phyId);
	phy->body = Box2DUtils_CreateStaticBox(phyId, position, CATEGORY_STATIC_OBJECT, ((Vec2F) { 0.875, 0.125 }));

	ComponentGraphics* gfx = Object_AddGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect) {CurrentGame()->tileWidth, 4 * CurrentGame()->tileWidth, CurrentGame()->tileWidth, CurrentGame()->tileWidth };
	gfx->txCenter = (Vec2F){ 0.0, 4.5 };
	
	return 0;
}
