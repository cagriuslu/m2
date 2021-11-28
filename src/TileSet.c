#include "TileSet.h"
#include "VSON.h"
#include "Log.h"
#include "Game.h"
#include <SDL_image.h>
#include <stdio.h>
#include <string.h>

XErr TileSet_InitFromFile(TileSet* ts, const char* fpath) {
	memset(ts, 0, sizeof(TileSet));

	VSON tileSetVson;
	REFLECT_ERROR_CLEANUP(VSON_InitParseFile(&tileSetVson, fpath),
		LOG_ERR("invalid tileset vson"));

	XErr result = TileSet_InitFromVson(ts, &tileSetVson);
	VSON_Term(&tileSetVson);
	return result;
}

static XErr TileSet_InitFromVson_InitTextureRect(SDL_Rect* textureRect, VSON* terrainOrObjectVson) {
	textureRect->x = VSON_GetLong(terrainOrObjectVson, "x", 0);
	textureRect->y = VSON_GetLong(terrainOrObjectVson, "y", 0);
	textureRect->w = VSON_GetLong(terrainOrObjectVson, "w", 0);
	textureRect->h = VSON_GetLong(terrainOrObjectVson, "h", 0);
	ASSERT_TRUE_CLEANUP(textureRect->w && textureRect->h,
		LOG_ERR("invalid texture rect"),
		XERR_CORRUPTED);
	return XOK;
}

static XErr TileSet_InitFromVson_InitCollider(ColliderDef* colliderDef, VSON* terrainOrObjectVson) {
	VSON* colliderVson = NULL;
	if ((colliderVson = VSON_GetObject(terrainOrObjectVson, "RectCollider"))) {
		colliderDef->colliderType = COLLIDER_TYPE_RECTANGLE;
		colliderDef->colliderUnion.rectangle.center.x = VSON_GetFloat(colliderVson, "x", 0.0f);
		colliderDef->colliderUnion.rectangle.center.y = VSON_GetFloat(colliderVson, "y", 0.0f);
		colliderDef->colliderUnion.rectangle.dims.x = VSON_GetFloat(colliderVson, "w", 0.0f);
		colliderDef->colliderUnion.rectangle.dims.y = VSON_GetFloat(colliderVson, "h", 0.0f);
		ASSERT_TRUE_CLEANUP(colliderDef->colliderUnion.rectangle.dims.x && colliderDef->colliderUnion.rectangle.dims.y,
			LOG_ERR("invalid rectangle collider"),
			XERR_CORRUPTED);
	} else if ((colliderVson = VSON_GetObject(terrainOrObjectVson, "CircCollider"))) {
		colliderDef->colliderType = COLLIDER_TYPE_CIRCLE;
		colliderDef->colliderUnion.circle.center.x = VSON_GetFloat(colliderVson, "x", 0.0f);
		colliderDef->colliderUnion.circle.center.y = VSON_GetFloat(colliderVson, "y", 0.0f);
		colliderDef->colliderUnion.circle.radius = VSON_GetFloat(colliderVson, "r", 0.0f);
		ASSERT_TRUE_CLEANUP(colliderDef->colliderUnion.circle.radius,
			LOG_ERR("invalid circle collider"),
			XERR_CORRUPTED);
	}
	return XOK;
}

XErr TileSet_InitFromVson(TileSet* ts, VSON* vson) {
	memset(ts, 0, sizeof(TileSet));

	ASSERT_TRUE_CLEANUP(vson->type == VSON_VALUE_TYPE_OBJECT,
		LOG_ERR("non-object tileset"),
		XERR_CORRUPTED);

	ts->tileWidth = VSON_GetLong(vson, "TileWidth", 0);
	ASSERT_TRUE_CLEANUP(ts->tileWidth,
		LOG_ERR("invalid tilewidth"),
		XERR_CORRUPTED);

	const char* textureFileStr = VSON_GetString(vson, "TextureFile");
	ASSERT_TRUE_CLEANUP(textureFileStr,
		LOG_ERR("missing texturefile"),
		XERR_CORRUPTED);
	SDL_Surface* textureSurface = IMG_Load(textureFileStr);
	ASSERT_TRUE_CLEANUP(textureSurface,
		LOG_ERR("invalid image"),
		XERR_NOT_FOUND);
	ts->texture = SDL_CreateTextureFromSurface(GAME->sdlRenderer, textureSurface);
	ASSERT_TRUE_CLEANUP(ts->texture,
		LOGOBJ_ERR(LOGOBJ_SDL_ERROR, String, SDL_GetError());
		SDL_FreeSurface(textureSurface),
		XERR_SDL);
	SDL_FreeSurface(textureSurface);

	REFLECT_ERROR_CLEANUP(HashMap_Init(&ts->terrainDefs, sizeof(TerrainDef), NULL),
		TileSet_Term(ts));
	VSON* terrainsVson = VSON_GetObject(vson, "Terrains");
	ASSERT_TRUE_CLEANUP(terrainsVson,
		LOG_ERR("invalid terrains");
		TileSet_Term(ts),
		XERR_CORRUPTED);
	VSON_OBJECT_ITERATE(terrainsVson, terrainKeyValuePtr) {
		TerrainDef* terrainDef = HashMap_SetStringKey(&ts->terrainDefs, terrainKeyValuePtr->key, NULL);

		REFLECT_ERROR_CLEANUP(TileSet_InitFromVson_InitTextureRect(&terrainDef->textureRect, &terrainKeyValuePtr->value),
			LOG_ERR("invalid terrain texture rect");
			TileSet_Term(ts));

		REFLECT_ERROR_CLEANUP(TileSet_InitFromVson_InitCollider(&terrainDef->colliderDef, &terrainKeyValuePtr->value),
			LOG_ERR("invalid terrain collider");
			TileSet_Term(ts));
	}

	REFLECT_ERROR_CLEANUP(HashMap_Init(&ts->objectDefs, sizeof(ObjectDef), NULL),
		TileSet_Term(ts));
	VSON* objectsVson = VSON_GetObject(vson, "Objects");
	ASSERT_TRUE_CLEANUP(objectsVson,
		LOG_ERR("invalid objects");
		TileSet_Term(ts),
		XERR_CORRUPTED);
	VSON_OBJECT_ITERATE(objectsVson, objectKeyValuePtr) {
		ObjectDef* objectDef = HashMap_SetStringKey(&ts->objectDefs, objectKeyValuePtr->key, NULL);

		REFLECT_ERROR_CLEANUP(TileSet_InitFromVson_InitTextureRect(&objectDef->textureRect, &objectKeyValuePtr->value),
			LOG_ERR("invalid object texture rect");
			TileSet_Term(ts));

		VSON* centerVson = VSON_GetObject(&objectKeyValuePtr->value, "Center");
		if (centerVson) {
			objectDef->center.x = VSON_GetFloat(centerVson, "x", 0.0f);
			objectDef->center.y = VSON_GetFloat(centerVson, "y", 0.0f);
		}

		REFLECT_ERROR_CLEANUP(TileSet_InitFromVson_InitCollider(&objectDef->colliderDef, &objectKeyValuePtr->value),
			LOG_ERR("invalid object collider");
			TileSet_Term(ts));
	}

	return XOK;
}

void TileSet_Term(TileSet* ts) {
	HashMap_Term(&ts->terrainDefs);
	HashMap_Term(&ts->objectDefs);
	if (ts->texture) {
		SDL_DestroyTexture(ts->texture);
	}
	memset(ts, 0, sizeof(TileSet));
}
