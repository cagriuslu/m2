#ifndef POLL_H
#define POLL_H

#include "Def.h"

#pragma warning(disable : 4200)
typedef struct _PoolItem {
	uint32_t id; // If allocated: key|index, else: 0|nextFreeIndex
	char data[];
} PoolItem;

typedef struct _Pool {
	PoolItem* items;
	size_t dataSize;
	size_t itemSize;
	uint64_t shiftedPoolId;
	size_t capacity; // max 65536
	size_t size; // [0, 65536]
	// What is the purpose of having a key?
	// Key is monotonically increasing, and it is a part of the ID
	// This means if an object is deallocated, and some other object is allocated at the same location,
	// they will have different IDs.
	size_t nextKey; // [1, 65536]
	size_t highestAllocatedIndex;
	size_t lowestAllocatedIndex;
	size_t nextFreeIndex;
	unsigned poolCapacityInBits; // max:16 -> 65536
} Pool;

M2Err Pool_Init(Pool* pool, unsigned poolCapacityInBits, size_t dataSize);
void Pool_Term(Pool* pool);

void* Pool_Mark(Pool* pool, void* copy, ID* outId);
void Pool_Unmark(Pool* pool, void* data);
void Pool_UnmarkByIndex(Pool* pool, size_t idx);
void Pool_UnmarkById(Pool* pool, ID id);
void Pool_UnmarkAll(Pool* pool);

size_t Pool_Size(Pool* pool);
bool Pool_IsMarked(Pool* pool, void* data);
bool Pool_IsMarkedByIndex(Pool* pool, size_t idx);
bool Pool_IsMarkedById(Pool* pool, ID id);
void* Pool_GetFirst(Pool* pool);
void* Pool_GetLast(Pool* pool);
void* Pool_GetNext(Pool* pool, void* currData);
void* Pool_GetPrev(Pool* pool, void* data);
void* Pool_GetByIndex(Pool* pool, size_t idx);
void* Pool_GetById(Pool* pool, ID id);
ID Pool_GetId(Pool* pool, void* data);
uint32_t Pool_GetIndex(Pool* pool, ID id);

#endif
