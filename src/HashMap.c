#include "HashMap.h"
#include "Array.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define HM_KEY_SIZE (8)
#define HM_BUCKET_COUNT (256)

typedef struct _HashMapItem {
	uint8_t key[HM_KEY_SIZE];
	char data[0];
} HashMapItem;

int HashMapInit(HashMap* hm, size_t itemSize) {
	memset(hm, 0, sizeof(HashMap));
	hm->arrays = calloc(HM_BUCKET_COUNT, sizeof(Array));
	assert(hm->arrays);
	for (unsigned i = 0; i < HM_BUCKET_COUNT; i++) {
		PROPAGATE_ERROR(Array_Init(hm->arrays + i, sizeof(HashMapItem) + itemSize, 16, (size_t)-1));
	}
	hm->itemSize = itemSize;
	return 0;
}

void HashMapDeinit(HashMap* hm) {
	for (unsigned i = 0; i < HM_BUCKET_COUNT; i++) {
		Array_Term(hm->arrays + i);
	}
	free(hm->arrays);
	memset(hm, 0, sizeof(HashMap));
}

size_t HashMapSize(HashMap* hm) {
	size_t count = 0;
	for (unsigned i = 0; i < HM_BUCKET_COUNT; i++) {
		count += hm->arrays[i].length;
	}
	return count;
}

void HashMapClear(HashMap* hm) {
	for (unsigned i = 0; i < HM_BUCKET_COUNT; i++) {
		Array_Clear(hm->arrays + i);
	}
}

void* HashMapSetIntKey(HashMap* hm, int64_t key, void* copy) {
	return _HashMapSet(hm, &key, copy);
}

void* HashMapTrySetIntKey(HashMap* hm, int64_t key, void* copy) {
	return _HashMapTrySet(hm, &key, copy);
}

void* HashMapGetIntKey(HashMap* hm, int64_t key) {
	return _HashMapGet(hm, &key);
}

void HashMapUnsetIntKey(HashMap* hm, int64_t key) {
	_HashMapUnset(hm, &key);
}

void* _HashMapSet(HashMap* hm, void* key, void* copy) {
	void* collision = _HashMapGet(hm, key);
	if (collision) {
		if (copy) {
			memcpy(collision, copy, hm->itemSize);
		}
		return collision;
	} else {
		Array* array = hm->arrays + HashMapHash(key);

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

void* _HashMapTrySet(HashMap* hm, void* key, void* copy) {
	void* collision = _HashMapGet(hm, key);
	if (collision) {
		return NULL;
	} else {
		Array* array = hm->arrays + HashMapHash(key);

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

void* _HashMapGet(HashMap* hm, void* key) {
	Array* array = hm->arrays + HashMapHash(key);
	
	HashMapItem* mapItem = NULL;
	for (size_t i = 0; i < array->length; i++) {
		HashMapItem* iter = Array_Get(array, i);
		if (iter && memcmp(iter->key, key, HM_KEY_SIZE) == 0) {
			mapItem = iter;
		}
	}
	
	return mapItem ? mapItem->data : NULL;
}

void _HashMapUnset(HashMap* hm, void* key) {
	Array* array = hm->arrays + HashMapHash(key);
	
	for (size_t i = 0; i < array->length; i++) {
		HashMapItem* iter = Array_Get(array, i);
		if (iter && memcmp(iter->key, key, HM_KEY_SIZE) == 0) {
			Array_Remove(array, i);
			return;
		}
	}
}

uint8_t HashMapHash(void* key) {
	uint8_t hash = 0xFF;
	for (int i = 0; i < 8; i++) {
		hash ^= ((uint8_t*)key)[i];
	}
	return hash;
}
