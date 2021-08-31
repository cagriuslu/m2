#define _CRT_SECURE_NO_WARNINGS
#include "TerrainLoader.h"
#include "Object.h"
#include "Txt.h"
#include "Array.h"
#include "Vec2I.h"
#include "Log.h"
#include "Vec2F.h"
#include "Error.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static TileDef TileLookup(const char* tileName) {
#define STREQ(str1, str2) (strcmp(str1, str2) == 0)
	if (STREQ(tileName, "Ground")) {
		return (TileDef) { {0, 0}, { 0.0, 0.0 }, { 0.0, 0.0 } };
	}
	if (STREQ(tileName, "WallHorizontal")) {
		return (TileDef) { {1, 0}, { 1.0, 1.0 }, { 0.0, 0.0 } };
	}
	if (STREQ(tileName, "WallVertical")) {
		return (TileDef) { {0, 1}, { 1.0, 1.0 }, { 0.0, 0.0 } };
	}
	if (STREQ(tileName, "CliffTop")) {
		return (TileDef) { {0, 2}, { 1.0, 1.0 }, { 0.0, 0.0 } };
	}
	if (STREQ(tileName, "CliffRight")) {
		return (TileDef) { {1, 2}, { 1.0, 1.0 }, { 0.0, 0.0 } };
	}
	if (STREQ(tileName, "CliffBottom")) {
		return (TileDef) { {1, 3}, { 1.0, 1.0 }, { 0.0, 0.0 } };
	}
	if (STREQ(tileName, "CliffLeft")) {
		return (TileDef) { {0, 3}, { 1.0, 1.0 }, { 0.0, 0.0 } };
	}
	if (STREQ(tileName, "CliffTopRight")) {
		return (TileDef) { {2, 2}, { 1.0, 1.0 }, { 0.0, 0.0 } };
	}
	if (STREQ(tileName, "CliffBottomRight")) {
		return (TileDef) { {2, 3}, { 1.0, 1.0 }, { 0.0, 0.0 } };
	}
	if (STREQ(tileName, "CliffBottomLeft")) {
		return (TileDef) { {3, 2}, { 1.0, 1.0 }, { 0.0, 0.0 } };
	}
	if (STREQ(tileName, "CliffTopLeft")) {
		return (TileDef) { {3, 3}, { 1.0, 1.0 }, { 0.0, 0.0 } };
	}
	return (TileDef) { {0, 0}, { 1.0, 1.0 }, { 0.0, 0.0 } };
#undef STREQ
}

int TerrainLoader_LoadTiles(Level* level, const char *tname) {
	Txt txt;
	PROPAGATE_ERROR(Txt_InitFromFile(&txt, tname));
	HashMap tileDefs;
	PROPAGATE_ERROR(HashMap_Init(&tileDefs, sizeof(TileDef), NULL));

	for (uint32_t txtKVIndex = 0; txtKVIndex < Array_Length(&txt.txtKVPairs); txtKVIndex++) {
		TxtKV* txtKV = Array_Get(&txt.txtKVPairs, txtKVIndex);
		char* value = TxtKV_DuplicateUrlEncodedValue(txtKV, "Tile=");
		if (!value) {
			continue;
		}
		TileDef tileDef = TileLookup(value);
		HashMap_SetInt64Key(&tileDefs, txtKVIndex, &tileDef);
		free(value);
	}

	for (uint32_t rowIndex = 0, *rowPtr = HashMap_GetInt32Keys(&txt.txtKVIndexes, 0, rowIndex); rowPtr; ++rowIndex, rowPtr = HashMap_GetInt32Keys(&txt.txtKVIndexes, 0, rowIndex)) {
		for (uint32_t colIndex = 0, *txtKVIndexPtr = HashMap_GetInt32Keys(&txt.txtKVIndexes, colIndex, rowIndex); txtKVIndexPtr; ++colIndex, txtKVIndexPtr = HashMap_GetInt32Keys(&txt.txtKVIndexes, colIndex, rowIndex)) {
			TileDef* tileDefPtr = HashMap_GetInt64Key(&tileDefs, *txtKVIndexPtr);
			Object* tile = Pool_Mark(&level->objects, NULL, NULL);
			ObjectTile_Init(tile, *tileDefPtr, (Vec2F) { (float)colIndex, (float)rowIndex });
		}
	}

	HashMap_Term(&tileDefs);
	Txt_Term(&txt);
	return 0;
}

void TerrainLoader_LoadEnemies_ItemTerm(void* opaqueItemPtr) {
	char** itemPtr = (char**)opaqueItemPtr;
	char* item = *itemPtr;
	free(item);
}

int TerrainLoader_LoadEnemies(Level* level, const char* tname) {
	Txt txt;
	PROPAGATE_ERROR(Txt_InitFromFile(&txt, tname));
	HashMap enemyDescriptors;
	HashMap_Init(&enemyDescriptors, sizeof(char*), TerrainLoader_LoadEnemies_ItemTerm);

	for (uint32_t txtKVIndex = 0; txtKVIndex < Array_Length(&txt.txtKVPairs); txtKVIndex++) {
		TxtKV* txtKV = Array_Get(&txt.txtKVPairs, txtKVIndex);
		char* value = TxtKV_DuplicateUrlEncodedValue(txtKV, "Enemy=");
		if (value) {
			HashMap_SetInt64Key(&enemyDescriptors, txtKVIndex, &value);
		}
	}

	for (uint32_t rowIndex = 0, *rowPtr = HashMap_GetInt32Keys(&txt.txtKVIndexes, 0, rowIndex); rowPtr; ++rowIndex, rowPtr = HashMap_GetInt32Keys(&txt.txtKVIndexes, 0, rowIndex)) {
		for (uint32_t colIndex = 0, *txtKVIndexPtr = HashMap_GetInt32Keys(&txt.txtKVIndexes, colIndex, rowIndex); txtKVIndexPtr; ++colIndex, txtKVIndexPtr = HashMap_GetInt32Keys(&txt.txtKVIndexes, colIndex, rowIndex)) {
			char** enemyDescriptor = HashMap_GetInt64Key(&enemyDescriptors, *txtKVIndexPtr);
			if (enemyDescriptor) {
				Object* enemy = Pool_Mark(&level->objects, NULL, NULL);
				ObjectEnemy_Init(enemy, (Vec2F) { (float)colIndex, (float)rowIndex }, *enemyDescriptor);
			}
		}
	}

	HashMap_Term(&enemyDescriptors);
	Txt_Term(&txt);
	return 0;
}
