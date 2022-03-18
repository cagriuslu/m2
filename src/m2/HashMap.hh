#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "m2/Array.hh"
#include <stdint.h>

#define HASHMAP_KEY_SIZE (8)
#define HASHMAP_BUCKET_COUNT (256)

typedef struct _HashMap {
	Array buckets[HASHMAP_BUCKET_COUNT];
	size_t itemSize;
	void (*itemTerm)(void*);
} HashMap;
typedef struct _HashMapItem {
	uint8_t key[HASHMAP_KEY_SIZE];
	char data[];
} HashMapItem;
typedef HashMap HashMapOfInt32s;
typedef HashMap HashMapOfTextures;

int HashMap_Init(HashMap *hm, size_t itemSize, void (*itemTerm)(void*));
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
// with Vec2I key
void* HashMap_SetVec2IKey(HashMap* hm, m2::vec2i key, void* copy);
void* HashMap_TrySetVec2IKey(HashMap* hm, m2::vec2i key, void* copy);
void* HashMap_GetVec2IKey(HashMap* hm, m2::vec2i key);
void HashMap_UnsetVec2IKey(HashMap* hm, m2::vec2i key);
// with String key
void* HashMap_SetStringKey(HashMap* hm, const char* key, void* copy);
void* HashMap_TrySetStringKey(HashMap* hm, const char* key, void* copy);
void* HashMap_GetStringKey(HashMap* hm, const char* key);
void HashMap_UnsetStringKey(HashMap* hm, const char* key);
// Key must be 8 bytes
void* HashMap_Set(HashMap* hm, void* key, void* copy);
void* HashMap_TrySet(HashMap* hm, void* key, void* copy); // Sets only if key not exists
void* HashMap_Get(HashMap* hm, void* key);
void HashMap_Unset(HashMap* hm, void* key);

#define TO_HASHMAP_ITEM_TERM(funcptr) ((void (*)(void*))(funcptr))

#endif
