#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "Array.h"
#include <stdint.h>

typedef struct _HashMap {
	Array* arrays; // 256
	size_t itemSize;
} HashMap;

int HashMapInit(HashMap *hm, size_t itemSize);
void HashMapDeinit(HashMap* hm);

size_t HashMapSize(HashMap* hm);
void HashMapClear(HashMap* hm);
void* HashMapSetIntKey(HashMap* hm, int64_t key, void* copy);
void* HashMapTrySetIntKey(HashMap* hm, int64_t key, void* copy);
void* HashMapGetIntKey(HashMap* hm, int64_t key);
void HashMapUnsetIntKey(HashMap* hm, int64_t key);

// Key must be AT LEAST 8 bytes
void* _HashMapSet(HashMap* hm, void* key, void* copy);
void* _HashMapTrySet(HashMap* hm, void* key, void* copy);
void* _HashMapGet(HashMap* hm, void* key);
void _HashMapUnset(HashMap* hm, void* key);

uint8_t HashMapHash(void* key);

#endif
