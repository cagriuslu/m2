#define _CRT_SECURE_NO_WARNINGS
#include "TerrainLoader.h"
#include "Terrain.h"
#include "Array.h"
#include "Error.h"
#include <stdio.h>
#include <assert.h>

typedef struct _TileKV {
	char key[4];
	char value[28];
} TileKV;

Array MyGetline(FILE *file);
Array MySplit(char *input, char delimiter);

int LoadTerrain(Object *terrain, const char *tname) {
	// Open file
	FILE *file = fopen(tname, "r");
	assert(file);

	// Gather tile KV pairs until % character on a line
	Array tileKVs;
	ArrayInit(&tileKVs, sizeof(TileKV));
	while (true) {
		Array lineBuffer = MyGetline(file);
		if (ArrayLength(&lineBuffer) == 0) {
			ArrayDeinit(&lineBuffer);
			continue;
		}
		char* line = ArrayGet(&lineBuffer, 0);

		if (line[0] == '%') {
			ArrayDeinit(&lineBuffer);
			break;
		}

		Array splits = MySplit(line, ' ');
		assert(2 <= ArrayLength(&splits));

		char **keyPtr = ArrayGet(&splits, 0);
		char **valuePtr = ArrayGet(&splits, 1);
		assert(strlen(*keyPtr));
		assert(strlen(*valuePtr));

		TileKV tileKV;
		memset(&tileKV, 0, sizeof(TileKV));
		strncpy(tileKV.key, *keyPtr, 3);
		strncpy(tileKV.value, *valuePtr, 27);
		ArrayAppend(&tileKVs, &tileKV);

		ArrayDeinit(&splits);
		ArrayDeinit(&lineBuffer);
	}

	// TODO verify all tile values are valid

	// Read matirx data
	size_t colCount = 0;
	while (true) {
		Array lineBuffer = MyGetline(file);
		if (ArrayLength(&lineBuffer) == 0) {
			ArrayDeinit(&lineBuffer);
			break;
		}
		char* line = ArrayGet(&lineBuffer, 0);

		Array splits = MySplit(line, '\t');
		assert(ArrayLength(&splits));

		if (colCount == 0) {
			colCount = ArrayLength(&splits);
		}

		// TODO
	}

	return 0;
}

Array MyGetline(FILE *file) {
	Array lineBuffer;
	ArrayInit(&lineBuffer, sizeof(char));

	int c;
	while ((c = fgetc(file)) != EOF && c != '\n') {
		char ch = c;
		ArrayAppend(&lineBuffer, &ch);
	}
	c = 0;
	ArrayAppend(&lineBuffer, &c);

	return lineBuffer;
}

Array MySplit(char *input, char delimiter) {
	Array splitBuffer;
	ArrayInit(&splitBuffer, sizeof(char*));

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
