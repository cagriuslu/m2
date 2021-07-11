#define _CRT_SECURE_NO_WARNINGS
#include "Txt.h"
#include "Log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

static Array MyGetline(FILE* file);
static Array MySplit(char* input, char delimiter);

XErr TxtKV_Init(TxtKV* kv) {
	memset(kv, 0, sizeof(TxtKV));
	return 0;
}

void TxtKV_SetKey(TxtKV* kv, const char* key) {
	strncpy(kv->key, key, 7);
	kv->key[7] = 0;
}

void TxtKV_SetValue(TxtKV* kv, const char* value, bool isDynamic) {
	kv->value = value;
	kv->isValueDynamic = isDynamic;
}

void TxtKV_Term(TxtKV* kv) {
	if (kv->isValueDynamic && kv->value) {
		free(kv->value);
	}
	memset(kv, 0, sizeof(TxtKV));
}

XErr Txt_Init(Txt* txt) {
	memset(txt, 0, sizeof(Txt));
	PROPAGATE_ERROR(Array_Init(&txt->txtKVPairs, sizeof(TxtKV), 16, UINT32_MAX));
	PROPAGATE_ERROR(HashMap_Init(&txt->txtKVIndexes, sizeof(uint32_t)));
	return 0;
}

XErr Txt_InitFromFile(Txt* txt, const char* fpath) {
	PROPAGATE_ERROR(Txt_Init(txt));
	
	FILE* file = fopen(fpath, "r");
	assert(file);

	// Load KV pairs until %
	while (true) {
		Array lineBuffer = MyGetline(file);
		if (Array_Length(&lineBuffer) == 0) {
			LOGTYP_ERR("CorruptedTxtFile", String, fpath);
			Array_Term(&lineBuffer);
			fclose(file);
			return XERR_CORRUPTED_FILE;
		}
		if (Array_Length(&lineBuffer) == 1) {
			// Empty line
			Array_Term(&lineBuffer);
			continue;
		}
		char* line = Array_Get(&lineBuffer, 0);
		if (line[0] == '%') {
			Array_Term(&lineBuffer);
			break;
		}

		// Split key and value
		Array split = MySplit(line, '\t');
		if (Array_Length(&split) != 2) {
			LOGTYP_ERR("CorruptedTxtFile", String, fpath);
			Array_Term(&split);
			Array_Term(&lineBuffer);
			fclose(file);
			return XERR_CORRUPTED_FILE;
		}
		char** keyPtr = Array_Get(&split, 0);
		char** valuePtr = Array_Get(&split, 1);
		char* valueDup = _strdup(*valuePtr);
		assert(valueDup);
		
		TxtKV* txtKV = Array_Append(&txt->txtKVPairs, NULL);;
		TxtKV_Init(txtKV);
		TxtKV_SetKey(txtKV, *keyPtr);
		TxtKV_SetValue(txtKV, valueDup, true);

		Array_Term(&split);
		Array_Term(&lineBuffer);
	}

	uint32_t rowIndex = 0;
	while (true) {
		Array lineBuffer = MyGetline(file);
		if (Array_Length(&lineBuffer) == 0) {
			Array_Term(&lineBuffer);
			break;
		}
		if (Array_Length(&lineBuffer) == 1) {
			// Empty line
			Array_Term(&lineBuffer);
			continue;
		}
		char* line = Array_Get(&lineBuffer, 0);

		// Split values
		Array split = MySplit(line, '\t');
		for (uint32_t colIndex = 0; colIndex < Array_Length(&split); colIndex++) {
			char** colDataPtr = Array_Get(&split, colIndex);
			char* colData = *colDataPtr;
			// Search in the array of TxtKVs
			for (uint32_t i = 0; i < Array_Length(&txt->txtKVPairs); i++) {
				TxtKV* txtKV = Array_Get(&txt->txtKVPairs, i);
				if (strcmp(colData, txtKV->key) == 0) {
					HashMap_SetInt32Keys(&txt->txtKVIndexes, colIndex, rowIndex, &i);
					break;
				}
			}
		}

		rowIndex++;
	}
	
	fclose(file);
	return 0;
}

XErr Txt_SaveToFile(Txt* txt, const char* fpath) {
	FILE* file = fopen(fpath, "w");
	assert(file);

	for (uint32_t i = 0; i < Array_Length(&txt->txtKVPairs); i++) {
		TxtKV* txtKV = Array_Get(&txt->txtKVPairs, i);
		fprintf(file, "%s\t%s\n", txtKV->key, txtKV->value);
	}
	fprintf(file, "%%\n");
	for (uint32_t rowIndex = 0, *rowPtr = HashMap_GetInt32Keys(&txt->txtKVIndexes, 0, rowIndex); rowPtr; ++rowIndex, rowPtr = HashMap_GetInt32Keys(&txt->txtKVIndexes, 0, rowIndex)) {
		for (uint32_t colIndex = 0, *txtKVIndexPtr = HashMap_GetInt32Keys(&txt->txtKVIndexes, colIndex, rowIndex); txtKVIndexPtr; ++colIndex, txtKVIndexPtr = HashMap_GetInt32Keys(&txt->txtKVIndexes, colIndex, rowIndex)) {
			TxtKV* txtKV = Array_Get(&txt->txtKVPairs, *txtKVIndexPtr);
			fprintf(file, "%s\t", txtKV->key);
		}
		fprintf(file, "\n");
	}
	
	fclose(file);
	return 0;
}

void Txt_Term(Txt* txt) {
	HashMap_Term(&txt->txtKVIndexes);
	Array_Term(&txt->txtKVPairs);
	memset(txt, 0, sizeof(Txt));
}

static Array MyGetline(FILE* file) {
	Array lineBuffer;
	Array_Init(&lineBuffer, sizeof(char), 256, SIZE_MAX);

	int c;
	char ch;
	while ((c = fgetc(file)) != EOF && c != '\n') {
		ch = (char)c;
		Array_Append(&lineBuffer, &ch);
	}
	if (Array_Length(&lineBuffer) || c != EOF) {
		ch = 0;
		Array_Append(&lineBuffer, &ch);
	}

	return lineBuffer;
}

static Array MySplit(char* input, char delimiter) {
	Array splitBuffer;
	Array_Init(&splitBuffer, sizeof(char*), 256, SIZE_MAX);

	size_t totalSize = strlen(input);
	for (size_t i = 0; i < totalSize; i++) {
		if (input[i] != delimiter) {
			char* startOfPhrase = input + i;
			Array_Append(&splitBuffer, &startOfPhrase);
			while (input[i] != delimiter && input[i] != 0) {
				i++;
			}
			input[i] = 0;
		}
	}
	return splitBuffer;
}