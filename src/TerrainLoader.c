#define _CRT_SECURE_NO_WARNINGS
#include "TerrainLoader.h"
#include "TileLookup.h"
#include "Object.h"
#include "Main.h"
#include "Array.h"
#include "Error.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

typedef struct _TileKV {
	char key[4];
	TileDef tileDef;
} TileKV;

static Array MyGetline(FILE *file);
static Array MySplit(char *input, char delimiter);

int TerrainLoad(const char *tname) {
	// Open file
	FILE *file = fopen(tname, "r");
	assert(file);

	// Gather tile KV pairs until % character on a line
	Array tileKVs;
	Array_Init(&tileKVs, sizeof(TileKV), 16, SIZE_MAX);
	while (true) {
		Array lineBuffer = MyGetline(file);
		if (lineBuffer.length == 0) {
			Array_Term(&lineBuffer);
			continue;
		}
		// Break if % is encountered
		char* line = Array_Get(&lineBuffer, 0);
		if (line[0] == '%') {
			Array_Term(&lineBuffer);
			break;
		}

		// Split key and value
		Array splits = MySplit(line, '\t');
		assert(2 == splits.length);

		char **keyPtr = Array_Get(&splits, 0);
		char **valuePtr = Array_Get(&splits, 1);
		assert(strlen(*keyPtr));
		assert(strlen(*valuePtr));

		TileKV tileKV;
		memset(&tileKV, 0, sizeof(TileKV));
		strncpy(tileKV.key, *keyPtr, 3);
		tileKV.tileDef = TileLookup(*valuePtr);
		Array_Append(&tileKVs, &tileKV);

		Array_Term(&splits);
		Array_Term(&lineBuffer);
	}

	// Read matirx data
	size_t rowIndex = 0, colCount = 0;
	while (true) {
		Array lineBuffer = MyGetline(file);
		if (lineBuffer.length == 0) {
			Array_Term(&lineBuffer);
			break;
		}
		char* line = Array_Get(&lineBuffer, 0);
		if (strlen(line) == 0) {
			Array_Term(&lineBuffer);
			break;
		}

		Array splits = MySplit(line, '\t');
		assert(splits.length);

		if (colCount == 0) {
			colCount = splits.length;
		}
		assert(splits.length == colCount);

		for (size_t colIndex = 0; colIndex < colCount; colIndex++) {
			char** colDataPtr = Array_Get(&splits, colIndex);
			char* colData = *colDataPtr;
			// Lookup Tile from TileKV Array
			TileDef tileDef = { 0 };
			for (size_t j = 0; j < tileKVs.length; j++) {
				TileKV* tileKV = Array_Get(&tileKVs, j);
				if (strcmp(colData, tileKV->key) == 0) {
					tileDef = tileKV->tileDef;
					break;
				}
			}
			Object* tile = Bucket_Mark(&CurrentLevel()->objects, NULL, NULL);
			ObjectTileInit(tile, tileDef, (Vec2F) { (float)colIndex, (float)rowIndex });
		}

		rowIndex++;
	}
	
	Array_Term(&tileKVs);
	fclose(file);
	return 0;
}

static Array MyGetline(FILE *file) {
	Array lineBuffer;
	Array_Init(&lineBuffer, sizeof(char), 1024, SIZE_MAX);

	int c;
	while ((c = fgetc(file)) != EOF && c != '\n') {
		char ch = (char) c;
		Array_Append(&lineBuffer, &ch);
	}
	c = 0;
	Array_Append(&lineBuffer, &c);

	return lineBuffer;
}

static Array MySplit(char *input, char delimiter) {
	Array splitBuffer;
	Array_Init(&splitBuffer, sizeof(char*), 256, SIZE_MAX);

	size_t totalSize = strlen(input);
	for (size_t i = 0; i < totalSize; i++) {
		if (input[i] != delimiter) {
			char *startOfPhrase = input + i;
			Array_Append(&splitBuffer, &startOfPhrase);
			while (input[i] != delimiter && input[i] != 0) {
				i++;
			}
			input[i] = 0;
		}
	}
	return splitBuffer;
}
