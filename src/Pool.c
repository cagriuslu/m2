#include "Pool.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#define PoolItemData(pool, index) ((PoolItem*) (((int8_t*) ((pool)->items)) + ((pool)->itemSize * (index))))
#define PoolItemFromData(itemData) ((PoolItem*) (((int8_t*) (itemData)) - offsetof(PoolItem, data)))

uint16_t gPoolId = 1;

M2Err Pool_Init(Pool* pool, unsigned poolCapacityInBits, size_t dataSize) {
	memset(pool, 0, sizeof(Pool));
	pool->poolCapacityInBits = poolCapacityInBits;
	pool->capacity = (size_t)1 << pool->poolCapacityInBits;
	pool->dataSize = dataSize;
	pool->itemSize = dataSize + sizeof(PoolItem);
	pool->shiftedPoolId = ((uint64_t)gPoolId++) << 48;
	pool->items = malloc(pool->capacity * pool->itemSize);
	assert(pool->items);
	Pool_UnmarkAll(pool);
	return 0;
}

void Pool_Term(Pool* pool) {
	if (pool->items) {
		memset(pool->items, 0, pool->capacity * pool->itemSize);
		free(pool->items);
	}
	memset(pool, 0, sizeof(Pool));
}

void* Pool_Mark(Pool* pool, void* copy, ID* outId) {
	if (pool->size < pool->capacity) {
		const size_t indexToAllocate = pool->nextFreeIndex;
		PoolItem* itemToAllocate = PoolItemData(pool, indexToAllocate);
		pool->nextFreeIndex = itemToAllocate->id & 0xFFFF; // Extract next nextFreeIndex
		itemToAllocate->id = ((uint16_t)pool->nextKey << 16) | ((uint16_t)indexToAllocate & 0xFFFF); // Store new id of the item
		ID id = pool->shiftedPoolId | (uint64_t)itemToAllocate->id;
		if (outId) { *outId = id; }

		pool->size++;
		pool->nextKey++;
		if (pool->nextKey > pool->capacity) {
			pool->nextKey = 1; // Rewind key to beginning
		}
		if (pool->highestAllocatedIndex < indexToAllocate) {
			pool->highestAllocatedIndex = indexToAllocate;
		}
		if (pool->lowestAllocatedIndex > indexToAllocate) {
			pool->lowestAllocatedIndex = indexToAllocate;
		}

		if (copy) {
			memcpy(&(itemToAllocate->data), copy, pool->dataSize);
		}
		return &(itemToAllocate->data);
	} else {
		return NULL;
	}
}

void Pool_Unmark(Pool* pool, void* data) {
	if (!Pool_IsMarked(pool, data)) {
		return;
	}

	PoolItem* itemToDelete = PoolItemFromData(data);
	const size_t indexToDelete = itemToDelete->id & 0xFFFF;
	itemToDelete->id = pool->nextFreeIndex & 0xFFFF; // Set item as free
	pool->nextFreeIndex = indexToDelete; // Store next free index

	pool->size--;
	if (pool->highestAllocatedIndex == indexToDelete) {
		// Search backwards until highest allocated index is found
		for (size_t i = indexToDelete; i-- > 0; ) {
			pool->highestAllocatedIndex = i;
			if (PoolItemData(pool, i)->id & 0xFFFF0000) {
				break;
			}
		}
	}
	if (pool->lowestAllocatedIndex == indexToDelete) {
		// Search forward until lowest allocated index is found
		for (size_t i = indexToDelete + 1; i < pool->capacity; i++) {
			pool->lowestAllocatedIndex = i;
			if (PoolItemData(pool, i)->id & 0xFFFF0000) {
				break;
			}
		}
	}
}

void Pool_UnmarkByIndex(Pool* pool, size_t idx) {
	void* item = Pool_GetByIndex(pool, idx);
	if (item) {
		Pool_Unmark(pool, item);
	}
}

void Pool_UnmarkById(Pool* pool, ID id) {
	void* item = Pool_GetById(pool, id);
	if (item) {
		Pool_Unmark(pool, item);
	}
}

void Pool_UnmarkAll(Pool* pool) {
	pool->size = 0;
	pool->nextKey = 1;
	pool->highestAllocatedIndex = 0;
	pool->lowestAllocatedIndex = 0;
	pool->nextFreeIndex = 0;
	if (pool->items) {
		for (size_t i = 0; i < pool->capacity; i++) {
			// Each item points to next item as free
			PoolItemData(pool, i)->id = 0xFFFF & (i + 1);
		}
	}
}

size_t Pool_Size(Pool* pool) {
	return pool->size;
}

bool Pool_IsMarked(Pool* pool, void* data) {
	return Pool_GetId(pool, data) != 0;
}

bool Pool_IsMarkedByIndex(Pool* pool, size_t idx) {
	void* data = Pool_GetByIndex(pool, idx);
	return data != NULL;
}

bool Pool_IsMarkedById(Pool* pool, ID id) {
	void* data = Pool_GetById(pool, id);
	return data != NULL;
}

void* Pool_GetFirst(Pool* pool) {
	if (pool->size) {
		return Pool_GetByIndex(pool, pool->lowestAllocatedIndex);
	}
	return NULL;
}

void* Pool_GetLast(Pool* pool) {
	if (pool->size) {
		return Pool_GetByIndex(pool, pool->highestAllocatedIndex);
	}
	return NULL;
}

void* Pool_GetNext(Pool* pool, void* currData) {
	const uint32_t currId = (uint32_t)Pool_GetId(pool, currData);
	if (currId == 0) {
		return NULL;
	}
	const size_t currIdx = currId & 0xFFFF;
	for (size_t i = currIdx + 1; i <= pool->highestAllocatedIndex; i++) {
		void* data = Pool_GetByIndex(pool, i);
		if (data) {
			return data;
		}
	}
	return NULL;
}

void* Pool_GetPrev(Pool* pool, void* currData) {
	const uint32_t currId = (uint32_t)Pool_GetId(pool, currData);
	if (currId == 0) {
		return NULL;
	}
	const size_t currIdx = currId & 0xFFFF;
	for (size_t i = currIdx; i-- > pool->lowestAllocatedIndex; ) {
		void* data = Pool_GetByIndex(pool, i);
		if (data) {
			return data;
		}
	}
	return NULL;
}

void* Pool_GetByIndex(Pool* pool, size_t idx) {
	PoolItem* candidateItem = PoolItemData(pool, idx);
	if (candidateItem->id & 0xFFFF0000) {
		return &(candidateItem->data);
	}
	return NULL;
}

void* Pool_GetById(Pool* pool, ID id) {
	if (pool->shiftedPoolId != (id & 0xFFFF000000000000ull)) {
		return NULL;
	}
	const size_t candidateIdx = (uint32_t)id & 0xFFFFu;
	PoolItem* candidateItem = PoolItemData(pool, candidateIdx);
	if (candidateItem->id == ((uint32_t)id & 0xFFFFFFFFu)) {
		return &(candidateItem->data);
	}
	return NULL;
}

ID Pool_GetId(Pool* pool, void* data) {
	PoolItem* itemToCheck = PoolItemFromData(data);
	// Check if object resides in range
	if ((itemToCheck < PoolItemData(pool, pool->lowestAllocatedIndex)) || (PoolItemData(pool, pool->highestAllocatedIndex + 1) < itemToCheck)) {
		return 0;
	}
	// Check if object is allocated
	if (itemToCheck->id & 0xFFFF0000) {
		return pool->shiftedPoolId | (uint64_t)itemToCheck->id;
	} else {
		return 0;
	}
}

uint32_t Pool_GetIndex(Pool* pool, ID id) {
	if (Pool_IsMarkedById(pool, id)) {
		return (uint32_t)id & 0xFFFF;
	} else {
		return -1;
	}
}
