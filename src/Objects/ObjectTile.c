#include "../Object.h"
#include "../Main.h"
#include "../Box2DUtils.h"
#include <string.h>
#include <stdio.h>

int ObjectTile_Init(Object* obj, TileDef tileDef, Vec2F position) {
	PROPAGATE_ERROR(Object_Init(obj, position, false));

	if (tileDef.colliderSize.x && tileDef.colliderSize.y) {
		ID phyId = 0;
		ComponentPhysics* phy = Object_AddPhysics(obj, &phyId);
		phy->body = Box2DUtils_CreateStaticBox(phyId, position, CATEGORY_STATIC_CLIFF, tileDef.colliderSize);
	}
	
	ComponentGraphics* gfx = Object_AddTerrainGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect){
		tileDef.txIndex.x * CurrentTileWidth(),
		tileDef.txIndex.y * CurrentTileWidth(),
		CurrentTileWidth(),
		CurrentTileWidth()
	};
	
	return 0;
}
