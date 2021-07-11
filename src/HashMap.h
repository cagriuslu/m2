#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "Array.h"
#include <stdint.h>

typedef struct _HashMap {
	Array* arrays; // 256
	size_t itemSize;
} HashMap;
typedef HashMap HashMapOfInt32s;

int HashMap_Init(HashMap *hm, size_t itemSize);
XErr HashMap_InitFromFile_StringToCharPtr(HashMap* hm, const char* fpath);
XErr HashMap_SaveToFile_StringToCharPtr(HashMap* hm, const char* fpath); // Char pointers must be freed
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

#endif
