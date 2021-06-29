#include "../Object.h"
#include "../Main.h"
#include "../TileLookup.h"
#include "../Box2DUtils.h"
#include <string.h>
#include <stdio.h>

int ObjectTileInit(Object* obj, TileDef tileDef, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj, position));

	if (tileDef.colliderSize.x && tileDef.colliderSize.y) {
		ID phyId = 0;
		PhysicsComponent* phy = ObjectAddPhysics(obj, &phyId);
		phy->body = Box2DUtils_CreateStaticBox(phyId, position, CATEGORY_STATIC_CLIFF, tileDef.colliderSize);
	}
	
	GraphicsComponent* gfx = ObjectAddTerrainGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect){
		tileDef.txIndex.x * CurrentTileWidth(),
		tileDef.txIndex.y * CurrentTileWidth(),
		CurrentTileWidth(),
		CurrentTileWidth()
	};
	
	return 0;
}
