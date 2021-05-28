#include "Tile.h"
#include "Main.h"

int TileInit(Tile* tile, Vec2F position, Vec2I txIndex) {
	tile->pos = position;
	tile->txSrc = (SDL_Rect) {
		txIndex.x * CurrentTileWidth(),
		txIndex.y * CurrentTileWidth(),
		CurrentTileWidth(),
		CurrentTileWidth()
	};

	Box2DBodyDef* bodyDef = Box2DBodyDefCreate();
	Box2DBodyDefSetPosition(bodyDef, position);
	Box2DBodyDefSetAllowSleep(bodyDef, true);
	Box2DBody* body = Box2DWorldCreateBody(CurrentWorld(), bodyDef);
	Box2DBodyDefDestroy(bodyDef);

	Box2DPolygonShape* boxShape = Box2DPolygonShapeCreate();
	Box2DPolygonShapeSetAsBox(boxShape, (Vec2F) { 0.5, 0.5 });
	Box2DFixture* fixture = Box2DBodyCreateFixtureFromShape(body, boxShape, 0.0);
	Box2DPolygonShapeDestroy(boxShape);
	tile->body = body;

	return 0;
}

void TileDeinit(Tile* tile) {

}
