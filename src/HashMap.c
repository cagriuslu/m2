#include "HashMap.h"
#include "Array.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "Txt.h"

#define HM_KEY_SIZE (8)
#define HM_BUCKET_COUNT (256)

typedef struct _HashMapItem {
	uint8_t key[HM_KEY_SIZE];
	char data[0];
} HashMapItem;

uint8_t HashMap_Hash(void* key);

int HashMap_Init(HashMap* hm, size_t itemSize) {
	memset(hm, 0, sizeof(HashMap));
	hm->arrays = calloc(HM_BUCKET_COUNT, sizeof(Array));
	assert(hm->arrays);
	for (unsigned i = 0; i < HM_BUCKET_COUNT; i++) {
		PROPAGATE_ERROR(Array_Init(hm->arrays + i, sizeof(HashMapItem) + itemSize, 16, (size_t)-1));
	}
	hm->itemSize = itemSize;
	return 0;
}

XErr HashMap_InitFromFile_StringToCharPtr(HashMap* hm, const char* fpath) {
	PROPAGATE_ERROR(HashMap_Init(hm, sizeof(char*)));
	Txt txt;
	PROPAGATE_ERROR(Txt_InitFromFile(&txt, fpath));
	// Iterate over columns of the first row
	for (uint32_t colIndex = 0, *txtKVIndexPtr = HashMap_GetInt32Keys(&txt.txtKVIndexes, colIndex, 0); txtKVIndexPtr; ++colIndex, txtKVIndexPtr = HashMap_GetInt32Keys(&txt.txtKVIndexes, colIndex, 0)) {
		// Get TxtKV from Txt array
		TxtKV* txtKV = Array_Get(&txt.txtKVPairs, *txtKVIndexPtr);
		// Duplicate string
		const char* dupValue = _strdup(txtKV->value);
		HashMap_Set(hm, txtKV->key, &dupValue);
	}
	Txt_Term(&txt);
	return 0;
}

XErr HashMap_SaveToFile_StringToCharPtr(HashMap* hm, const char* fpath) {
	Txt txt;
	PROPAGATE_ERROR(Txt_Init(&txt));
	uint32_t insertedIdx = 0;
	// Iterate over arrays
	for (unsigned arrayIdx = 0; arrayIdx < HM_BUCKET_COUNT; arrayIdx++) {
		// Iterate over items in the array
		for (unsigned itemIdx = 0; itemIdx < Array_Length(hm->arrays + arrayIdx); itemIdx++) {
			// Add new TxtKV to Txt
			HashMapItem* item = Array_Get(hm->arrays + arrayIdx, itemIdx);
			TxtKV* newKV = Array_Append(&txt.txtKVPairs, NULL);
			TxtKV_SetKey(newKV, item->key);
			TxtKV_SetValue(newKV, ((char**)item->data)[0], false);
			// Add an instance of the value into the HashMap
			HashMap_SetInt32Keys(&txt.txtKVIndexes, insertedIdx, 0, &insertedIdx);
			insertedIdx++;
		}
	}
	Txt_SaveToFile(&txt, fpath);
	Txt_Term(&txt);
	return 0;
}

void HashMap_Term(HashMap* hm) {
	for (unsigned i = 0; i < HM_BUCKET_COUNT; i++) {
		Array_Term(hm->arrays + i);
	}
	free(hm->arrays);
	memset(hm, 0, sizeof(HashMap));
}

size_t HashMap_Size(HashMap* hm) {
	size_t count = 0;
	for (unsigned i = 0; i < HM_BUCKET_COUNT; i++) {
		count += hm->arrays[i].length;
	}
	return count;
}

void HashMap_Clear(HashMap* hm) {
	for (unsigned i = 0; i < HM_BUCKET_COUNT; i++) {
		Array_Clear(hm->arrays + i);
	}
}

void* HashMap_SetInt32Keys(HashMap* hm, int32_t key1, int32_t key2, void* copy) {
	const int64_t key = (((int64_t)key2) << 32) | ((int64_t)key1 & 0x00000000FFFFFFFFll);
	return HashMap_SetInt64Key(hm, key, copy);
}

void* HashMap_TrySetInt32Keys(HashMap* hm, int32_t key1, int32_t key2, void* copy) {
	const int64_t key = (((int64_t)key2) << 32) | ((int64_t)key1 & 0x00000000FFFFFFFFll);
	return HashMap_TrySetInt64Key(hm, key, copy);
}

void* HashMap_GetInt32Keys(HashMap* hm, int32_t key1, int32_t key2) {
	const int64_t key = (((int64_t)key2) << 32) | ((int64_t)key1 & 0x00000000FFFFFFFFll);
	return HashMap_GetInt64Key(hm, key);
}

void HashMap_UnsetInt32Keys(HashMap* hm, int32_t key1, int32_t key2) {
	const int64_t key = (((int64_t)key2) << 32) | ((int64_t)key1 & 0x00000000FFFFFFFFll);
	HashMap_UnsetInt64Key(hm, key);
}

void* HashMap_SetInt64Key(HashMap* hm, int64_t key, void* copy) {
	return HashMap_Set(hm, &key, copy);
}

void* HashMap_TrySetInt64Key(HashMap* hm, int64_t key, void* copy) {
	return HashMap_TrySet(hm, &key, copy);
}

void* HashMap_GetInt64Key(HashMap* hm, int64_t key) {
	return HashMap_Get(hm, &key);
}

void HashMap_UnsetInt64Key(HashMap* hm, int64_t key) {
	HashMap_Unset(hm, &key);
}

void* HashMap_Set(HashMap* hm, void* key, void* copy) {
	void* collision = HashMap_Get(hm, key);
	if (collision) {
		if (copy) {
			memcpy(collision, copy, hm->itemSize);
		}
		return collision;
	} else {
		Array* array = hm->arrays + HashMap_Hash(key);

		HashMapItem* newItem = Array_Append(array, NULL);
		if (newItem) {
			memcpy(newItem->key, key, HM_KEY_SIZE);
			if (copy) {
				memcpy(newItem->data, copy, hm->itemSize);
			}
		}

		return newItem;
	}
}

void* HashMap_TrySet(HashMap* hm, void* key, void* copy) {
	void* collision = HashMap_Get(hm, key);
	if (collision) {
		return NULL;
	} else {
		Array* array = hm->arrays + HashMap_Hash(key);

		HashMapItem* newItem = Array_Append(array, NULL);
		if (newItem) {
			memcpy(newItem->key, key, HM_KEY_SIZE);
			if (copy) {
				memcpy(newItem->data, copy, hm->itemSize);
			}
		}

		return newItem;
	}
}

void* HashMap_Get(HashMap* hm, void* key) {
	Array* array = hm->arrays + HashMap_Hash(key);
	
	HashMapItem* mapItem = NULL;
	for (size_t i = 0; i < array->length; i++) {
		HashMapItem* iter = Array_Get(array, i);
		if (iter && memcmp(iter->key, key, HM_KEY_SIZE) == 0) {
			mapItem = iter;
		}
	}
	
	return mapItem ? mapItem->data : NULL;
}

void HashMap_Unset(HashMap* hm, void* key) {
	Array* array = hm->arrays + HashMap_Hash(key);
	
	for (size_t i = 0; i < array->length; i++) {
		HashMapItem* iter = Array_Get(array, i);
		if (iter && memcmp(iter->key, key, HM_KEY_SIZE) == 0) {
			Array_Remove(array, i);
			return;
		}
	}
}

uint8_t HashMap_RightRotateUInt8(uint8_t x, unsigned n) {
	const uint8_t rightShifted = x >> n;
	const uint8_t leftShifted = x << (8 - n);
	return leftShifted | rightShifted;
}

uint8_t HashMap_Hash(void* key) {
	uint8_t hash = 0xFF;
	for (int i = 0; i < 8; i++) {
		hash ^= HashMap_RightRotateUInt8(((uint8_t*)key)[i], i);
	}
	return hash;
}