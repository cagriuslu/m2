#include "Tile.h"
#include "Main.h"

#define STREQ(str1, str2) (strcmp(str1, str2) == 0)

int TileInit(Tile* tile, Vec2I position, Vec2I txIndex, Vec2F colliderSize) {
	tile->pos = position;
	tile->txSrc = (SDL_Rect) {
		txIndex.x * CurrentTileWidth(),
		txIndex.y * CurrentTileWidth(),
		CurrentTileWidth(),
		CurrentTileWidth()
	};

	if (0.0 < colliderSize.x && 0.0 < colliderSize.y) {
		Box2DBodyDef* bodyDef = Box2DBodyDefCreate();
		Box2DBodyDefSetPosition(bodyDef, (Vec2F) { position.x, position.y });
		Box2DBodyDefSetAllowSleep(bodyDef, true);
		Box2DBody* body = Box2DWorldCreateBody(CurrentWorld(), bodyDef);
		Box2DBodyDefDestroy(bodyDef);

		Box2DPolygonShape* boxShape = Box2DPolygonShapeCreate();
		Box2DPolygonShapeSetAsBox(boxShape, (Vec2F) { colliderSize.x / 2, colliderSize.y / 2 });
		Box2DFixture* fixture = Box2DBodyCreateFixtureFromShape(body, boxShape, 0.0);
		Box2DPolygonShapeDestroy(boxShape);
		tile->body = body;
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
