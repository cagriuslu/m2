#define _CRT_SECURE_NO_WARNINGS
#include "HashMap.h"
#include "Array.h"
#include "Def.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

uint8_t HashMap_Hash(void* key);

int HashMap_Init(HashMap* hm, size_t itemSize, void (*itemTerm)(void*)) {
	memset(hm, 0, sizeof(HashMap));
	for (unsigned i = 0; i < HASHMAP_BUCKET_COUNT; i++) {
		XERR_REFLECT(Array_Init(hm->buckets + i, sizeof(HashMapItem) + itemSize, 16, (size_t)-1, NULL));
	}
	hm->itemSize = itemSize;
	hm->itemTerm = itemTerm;
	return 0;
}

void HashMap_Term(HashMap* hm) {
	for (unsigned i = 0; i < HASHMAP_BUCKET_COUNT; i++) {
		Array* array = hm->buckets + i;
		if (hm->itemTerm) {
			for (size_t j = 0; j < Array_Length(array); j++) {
				HashMapItem* item = Array_Get(array, j);
				hm->itemTerm(item->data);
			}
		}
		Array_Term(array);
	}
	memset(hm, 0, sizeof(HashMap));
}

size_t HashMap_Size(HashMap* hm) {
	size_t count = 0;
	for (unsigned i = 0; i < HASHMAP_BUCKET_COUNT; i++) {
		count += hm->buckets[i].length;
	}
	return count;
}

void HashMap_Clear(HashMap* hm) {
	for (unsigned i = 0; i < HASHMAP_BUCKET_COUNT; i++) {
		Array_Clear(hm->buckets + i);
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

void* HashMap_SetStringKey(HashMap* hm, const char* key, void* copy) {
	char actualKey[HASHMAP_KEY_SIZE] = { 0 };
	strncpy(actualKey, key, HASHMAP_KEY_SIZE);
	return HashMap_Set(hm, actualKey, copy);
}

void* HashMap_TrySetStringKey(HashMap* hm, const char* key, void* copy) {
	char actualKey[HASHMAP_KEY_SIZE] = { 0 };
	strncpy(actualKey, key, HASHMAP_KEY_SIZE);
	return HashMap_TrySet(hm, actualKey, copy);
}

void* HashMap_GetStringKey(HashMap* hm, const char* key) {
	char actualKey[HASHMAP_KEY_SIZE] = { 0 };
	strncpy(actualKey, key, HASHMAP_KEY_SIZE);
	return HashMap_Get(hm, actualKey);
}

void HashMap_UnsetStringKey(HashMap* hm, const char* key) {
	char actualKey[HASHMAP_KEY_SIZE] = { 0 };
	strncpy(actualKey, key, HASHMAP_KEY_SIZE);
	HashMap_Unset(hm, actualKey);
}

void* HashMap_Set(HashMap* hm, void* key, void* copy) {
	void* collision = HashMap_Get(hm, key);
	if (collision) {
		if (copy) {
			memcpy(collision, copy, hm->itemSize);
		}
		return collision;
	} else {
		Array* array = hm->buckets + HashMap_Hash(key);

		HashMapItem* newItem = Array_Append(array, NULL);
		if (newItem) {
			memcpy(newItem->key, key, HASHMAP_KEY_SIZE);
			if (copy) {
				memcpy(newItem->data, copy, hm->itemSize);
			}
		}
		return newItem ? newItem->data : NULL;
	}
}

void* HashMap_TrySet(HashMap* hm, void* key, void* copy) {
	void* collision = HashMap_Get(hm, key);
	if (collision) {
		return NULL;
	} else {
		Array* array = hm->buckets + HashMap_Hash(key);

		HashMapItem* newItem = Array_Append(array, NULL);
		if (newItem) {
			memcpy(newItem->key, key, HASHMAP_KEY_SIZE);
			if (copy) {
				memcpy(newItem->data, copy, hm->itemSize);
			}
		}

		return newItem ? newItem->data : NULL;
	}
}

void* HashMap_Get(HashMap* hm, void* key) {
	Array* array = hm->buckets + HashMap_Hash(key);
	
	HashMapItem* mapItem = NULL;
	for (size_t i = 0; i < array->length; i++) {
		HashMapItem* iter = Array_Get(array, i);
		if (iter && memcmp(iter->key, key, HASHMAP_KEY_SIZE) == 0) {
			mapItem = iter;
			break;
		}
	}
	
	return mapItem ? mapItem->data : NULL;
}

void HashMap_Unset(HashMap* hm, void* key) {
	Array* array = hm->buckets + HashMap_Hash(key);
	
	for (size_t i = 0; i < array->length; i++) {
		HashMapItem* iter = Array_Get(array, i);
		if (iter && memcmp(iter->key, key, HASHMAP_KEY_SIZE) == 0) {
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
