#include "TileObject.h"
#include "Main.h"
#include "Box2DUtils.h"
#include <string.h>
#include <stdio.h>

int NewTileInit(Object* obj, TileDef tileDef, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj, position));

	if (tileDef.colliderSize.x && tileDef.colliderSize.y) {
		uint32_t phyId = 0;
		PhysicsComponent* phy = ObjectAddAndInitPhysics(obj, &phyId);
		phy->body = Box2DUtilsCreateStaticBox(phyId, position, STATIC_CLIFF_CATEGORY, tileDef.colliderSize);
	}
	
	GraphicsComponent* gfx = ObjectAddAndInitTerrainGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect){
		tileDef.txIndex.x * CurrentTileWidth(),
		tileDef.txIndex.y * CurrentTileWidth(),
		CurrentTileWidth(),
		CurrentTileWidth()
	};
	
	return 0;
}
