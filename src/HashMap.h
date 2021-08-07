#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "Array.h"
#include <stdint.h>

#define HASHMAP_KEY_SIZE (8)
#define HASHMAP_BUCKET_COUNT (256)

typedef struct _HashMap {
	Array* arrays; // HASHMAP_BUCKET_COUNT arrays
	size_t itemSize;
	void (*itemTerm)(void*);
} HashMap;
typedef struct _HashMapItem {
	uint8_t key[HASHMAP_KEY_SIZE];
	char data[0];
} HashMapItem;
typedef HashMap HashMapOfInt32s;
typedef HashMap HashMapOfTextures;

int HashMap_Init(HashMap *hm, size_t itemSize, void (*itemTerm)(void*));
XErr HashMap_InitFromFile_StringToCharPtr(HashMap* hm, const char* fpath);
XErr HashMap_SaveToFile_StringToCharPtr(HashMap* hm, const char* fpath);
void HashMap_Term(HashMap* hm);

size_t HashMap_Size(HashMap* hm);
void HashMap_Clear(HashMap* hm);

// with Int32 keys
void* HashMap_SetInt32Keys(HashMap* hm, int32_t keyX, int32_t keyY, void* copy);
void* HashMap_TrySetInt32Keys(HashMap* hm, int32_t keyX, int32_t keyY, void* copy);
void* HashMap_GetInt32Keys(HashMap* hm, int32_t keyX, int32_t keyY);
void HashMap_UnsetInt32Keys(HashMap* hm, int32_t keyX, int32_t keyY);
// with Int64 key
void* HashMap_SetInt64Key(HashMap* hm, int64_t key, void* copy);
void* HashMap_TrySetInt64Key(HashMap* hm, int64_t key, void* copy);
void* HashMap_GetInt64Key(HashMap* hm, int64_t key);
void HashMap_UnsetInt64Key(HashMap* hm, int64_t key);
// Key must be 8 bytes
void* HashMap_Set(HashMap* hm, void* key, void* copy);
void* HashMap_TrySet(HashMap* hm, void* key, void* copy); // Sets only if key not exists
void* HashMap_Get(HashMap* hm, void* key);
void HashMap_Unset(HashMap* hm, void* key);

#define TO_HASHMAP_ITEM_TERM(funcptr) ((void (*)(void*))(funcptr))

#endif
