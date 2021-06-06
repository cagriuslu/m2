#include "Tile.h"
#include "Main.h"
#include "Box2DUtils.h"
#include <string.h>
#include <stdio.h>

#define STREQ(str1, str2) (strcmp(str1, str2) == 0)

int TileInit(Tile* tile, Vec2I position, Vec2I txIndex, Vec2F colliderSize) {
	memset(tile, 0, sizeof(Tile));
	tile->type = OBJTYP_TILE;
	tile->pos = position;
	tile->txSrc = (SDL_Rect) {
		txIndex.x * CurrentTileWidth(),
		txIndex.y * CurrentTileWidth(),
		CurrentTileWidth(),
		CurrentTileWidth()
	};

	if (0.0 < colliderSize.x && 0.0 < colliderSize.y) {
		tile->body = Box2DUtilsCreateStaticBox(tile, ((Vec2F) { (float) position.x, (float) position.y }), STATIC_CATEGORY, colliderSize);
	} else {
		tile->body = NULL;
	}

	return 0;
}

void TileDeinit(Tile* tile) {
	if (tile->body) {
		Box2DWorldDestroyBody(CurrentWorld(), tile->body);
	}
}

TileDef TileLookup(const char* tileName) {
	if (STREQ(tileName, "Ground")) {
		return (TileDef) { {0, 0}, { 0.0, 0.0 } };
	} else if (STREQ(tileName, "WallHorizontal")) {
		return (TileDef) { {1, 0}, { 1.0, 1.0 } };
	} else if (STREQ(tileName, "WallVertical")) {
		return (TileDef) { {0, 1}, { 1.0, 1.0 } };
	} else {
		return (TileDef) { {0, 0}, { 1.0, 1.0 } };
	}
}
