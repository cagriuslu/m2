#include "TerrainLoader.h"
#include "Terrain.h"
#include "Array.h"
#include "Error.h"
#include <stdio.h>

typedef struct _TileDef {
	char key[4];
	char value[28];
} TileDef;

Array MyGetline(FILE *file);
Array MySplit(char *input, char delimiter);

int LoadTerrain(Object *terrain, const char *tname) {
	// Open file
	FILE *file = fopen(tname, "r");
	assert(file);

	Array tileDefs;
	ArrayInit(&tileDefs, sizeof(TileDef));
	while (true) {
		Array line = MyGetline(file);
		Array splits = MySplit(ArrayGet(&line, &1), ' ');
		assert(2 <= ArrayLength(&splits));

		char **keyPtr = ArrayGet(&splits, 0);
		char **valuePtr = ArrayGet(&splits, 1);

		TileDef tileDef;
		memset(&tileDef, 0, sizeof(TileDef));
		strncpy(tileDef.key, *keyPtr, 3);
		strncpy(tileDef.value, *valuePtr, 27);
		ArrayAppend(&tileDefs, &tileDef);

		ArrayDeinit(&splits);
		ArrayDeinit(&line);
	}

	// TODO test above code



	
}

Array MyGetline(FILE *file) {
	Array lineBuffer;
	ArrayInit(&lineBuffer, sizeof(char));
	int c;
	while ((c = fgetc(file)) != EOF && c != '\n') {
		char ch = c;
		ArrayAppend(&lineBuffer, &ch);
	}
	return lineBuffer;
}

Array MySplit(char *input, char delimiter) {
	Array splitBuffer;
	ArrayInit(&splitBuffer, sizeof(char*));
	for (size_t i = 0; i < strlen(input); i++) {
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
