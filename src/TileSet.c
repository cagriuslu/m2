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
	REFLECT_ERROR(VSON_InitParseFile(&tileSetVson, fpath));
	ASSERT_TRUE_CLEANUP(tileSetVson.type == VSON_VALUE_TYPE_OBJECT, XERR_CORRUPTED, VSON_Term(&tileSetVson));

	const char* tileWidthStr = VSON_GetString(&tileSetVson, "TileWidth");
	ASSERT_TRUE_CLEANUP(tileWidthStr, XERR_CORRUPTED, VSON_Term(&tileSetVson));
	ts->tileWidth = strtol(tileWidthStr, NULL, 0);

	const char* textureFileStr = VSON_GetString(&tileSetVson, "TextureFile");
	ASSERT_TRUE_CLEANUP(textureFileStr, XERR_CORRUPTED, VSON_Term(&tileSetVson));
	SDL_Surface* textureSurface = IMG_Load(textureFileStr);
	ASSERT_TRUE_CLEANUP(textureSurface, XERR_NOT_FOUND, VSON_Term(&tileSetVson));
	ts->texture = SDL_CreateTextureFromSurface(GAME->sdlRenderer, textureSurface);
	SDL_FreeSurface(textureSurface);
	ASSERT_TRUE_CLEANUP(ts->texture, XERR_SDL, VSON_Term(&tileSetVson));
	
	
	
	





	
	
	HashMap tileSetHashMap;
	REFLECT_ERROR_CLEANUP(VSON_Object_CreateVsonPtrHashMap(&tileSetVson, &tileSetHashMap), VSON_Term(&tileSetVson));

	REFLECT_ERROR_CLEANUP(HashMap_Init(&ts->terrainDefs, sizeof(TerrainDef), NULL),
		VSON_Term(&tileSetVson); HashMap_Term(&tileSetHashMap));
	REFLECT_ERROR_CLEANUP(HashMap_Init(&ts->objectDefs, sizeof(ObjectDef), NULL),
		VSON_Term(&tileSetVson); HashMap_Term(&tileSetHashMap); HashMap_Term(&ts->terrainDefs));
	

	return 0;
}
