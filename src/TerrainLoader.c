#define _CRT_SECURE_NO_WARNINGS
#include "TerrainLoader.h"
#include "TileLookup.h"
#include "Object.h"
#include "Txt.h"
#include "Main.h"
#include "Array.h"
#include "Error.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

typedef struct _TileKV {
	uint32_t txtKVIndex;
	TileDef tileDef;
} TileKV;

int TerrainLoad(const char *tname) {
	Txt txt;
	PROPAGATE_ERROR(Txt_InitFromFile(&txt, tname));

	HashMap tileDefs;
	HashMap_Init(&tileDefs, sizeof(TileDef));

	for (uint32_t txtKVIndex = 0; txtKVIndex < Array_Length(&txt.txtKVPairs); txtKVIndex++) {
		TxtKV* txtKV = Array_Get(&txt.txtKVPairs, txtKVIndex);
		TileDef tileDef = TileLookup(txtKV->value);
		HashMap_SetInt64Key(&tileDefs, txtKVIndex, &tileDef);
	}

	for (uint32_t rowIndex = 0, *rowPtr = HashMap_GetInt32Keys(&txt.txtKVIndexes, 0, rowIndex); rowPtr; ++rowIndex, rowPtr = HashMap_GetInt32Keys(&txt.txtKVIndexes, 0, rowIndex)) {
		for (uint32_t colIndex = 0, *txtKVIndexPtr = HashMap_GetInt32Keys(&txt.txtKVIndexes, colIndex, rowIndex); txtKVIndexPtr; ++colIndex, txtKVIndexPtr = HashMap_GetInt32Keys(&txt.txtKVIndexes, colIndex, rowIndex)) {
			// TODO
			TileDef* tileDefPtr = HashMap_GetInt64Key(&tileDefs, *txtKVIndexPtr);

			Object* tile = Bucket_Mark(&CurrentLevel()->objects, NULL, NULL);
			ObjectTileInit(tile, *tileDefPtr, (Vec2F) { (float)colIndex, (float)rowIndex });
		}
	}

	HashMap_Term(&tileDefs);
	Txt_Term(&txt);
	return 0;
}
