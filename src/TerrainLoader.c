#define _CRT_SECURE_NO_WARNINGS
#include "TerrainLoader.h"
#include "Main.h"
#include "TileObject.h"
#include "Array.h"
#include "Error.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

typedef struct _TileKV {
	char key[4];
	TileDef tileDef;
} TileKV;

Array MyGetline(FILE *file);
Array MySplit(char *input, char delimiter);

int TerrainLoad(const char *tname) {
	// Open file
	FILE *file = fopen(tname, "r");
	assert(file);

	// Gather tile KV pairs until % character on a line
	Array tileKVs;
	ArrayInit(&tileKVs, sizeof(TileKV), 16, SIZE_MAX);
	while (true) {
		Array lineBuffer = MyGetline(file);
		if (lineBuffer.length == 0) {
			ArrayDeinit(&lineBuffer);
			continue;
		}
		// Break if % is encountered
		char* line = ArrayGet(&lineBuffer, 0);
		if (line[0] == '%') {
			ArrayDeinit(&lineBuffer);
			break;
		}

		// Split key and value
		Array splits = MySplit(line, '\t');
		assert(2 == splits.length);

		char **keyPtr = ArrayGet(&splits, 0);
		char **valuePtr = ArrayGet(&splits, 1);
		assert(strlen(*keyPtr));
		assert(strlen(*valuePtr));

		TileKV tileKV;
		memset(&tileKV, 0, sizeof(TileKV));
		strncpy(tileKV.key, *keyPtr, 3);
		tileKV.tileDef = TileLookup(*valuePtr);
		ArrayAppend(&tileKVs, &tileKV);

		ArrayDeinit(&splits);
		ArrayDeinit(&lineBuffer);
	}

	// Read matirx data
	//Array tiles;
	//ArrayInit(&tiles, sizeof(TileObject), 16, SIZE_MAX);
	size_t rowIndex = 0, colCount = 0;
	while (true) {
		Array lineBuffer = MyGetline(file);
		if (lineBuffer.length == 0) {
			ArrayDeinit(&lineBuffer);
			break;
		}
		char* line = ArrayGet(&lineBuffer, 0);
		if (strlen(line) == 0) {
			ArrayDeinit(&lineBuffer);
			break;
		}

		Array splits = MySplit(line, '\t');
		assert(splits.length);

		if (colCount == 0) {
			colCount = splits.length;
		}
		assert(splits.length == colCount);

		for (size_t colIndex = 0; colIndex < colCount; colIndex++) {
			char** colDataPtr = ArrayGet(&splits, colIndex);
			char* colData = *colDataPtr;
			// Lookup Tile from TileKV Array
			TileDef tileDef = { 0 };
			for (size_t j = 0; j < tileKVs.length; j++) {
				TileKV* tileKV = ArrayGet(&tileKVs, j);
				if (strcmp(colData, tileKV->key) == 0) {
					tileDef = tileKV->tileDef;
					break;
				}
			}
			// Save Tile
			//TileObject* tile = ArrayAppend(&tiles, NULL);
			//TileInit(tile, (Vec2I) { (int) colIndex, (int) rowIndex }, tileDef.txIndex, tileDef.colliderSize);

			Object* tile = BucketMark(&CurrentLevel()->objects, NULL, NULL);
			NewTileInit(tile, tileDef, (Vec2F) { (float)colIndex, (float)rowIndex });
		}

		rowIndex++;
	}
	
	ArrayDeinit(&tileKVs);
	fclose(file);
	return 0;
}

Array MyGetline(FILE *file) {
	Array lineBuffer;
	ArrayInit(&lineBuffer, sizeof(char), 1024, SIZE_MAX);

	int c;
	while ((c = fgetc(file)) != EOF && c != '\n') {
		char ch = (char) c;
		ArrayAppend(&lineBuffer, &ch);
	}
	c = 0;
	ArrayAppend(&lineBuffer, &c);

	return lineBuffer;
}

Array MySplit(char *input, char delimiter) {
	Array splitBuffer;
	ArrayInit(&splitBuffer, sizeof(char*), 256, SIZE_MAX);

	size_t totalSize = strlen(input);
	for (size_t i = 0; i < totalSize; i++) {
		if (input[i] != delimiter) {
			char *startOfPhrase = input + i;
			ArrayAppend(&splitBuffer, &startOfPhrase);
			while (input[i] != delimiter && input[i] != 0) {
				i++;
			}
			input[i] = 0;
		}
	}
	return splitBuffer;
}
