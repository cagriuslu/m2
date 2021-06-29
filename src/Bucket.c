#include "Bucket.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#define BucketItemData(bucket, index) ((BucketItem*) (((int8_t*) ((bucket)->items)) + ((bucket)->itemSize * (index))))
#define BucketItemFromData(itemData) ((BucketItem*) (((int8_t*) (itemData)) - offsetof(BucketItem, data)))
#define APPEND_BUCKET_ID(bucketId, id) (((uint64_t)(bucketId) << 48) |  (uint64_t)(id))

uint16_t gBucketId = 1;

XErr Bucket_Init(Bucket* bucket, size_t dataSize) {
	memset(bucket, 0, sizeof(Bucket));
	bucket->capacity = UINT16_MAX + 1;
	bucket->dataSize = dataSize;
	bucket->itemSize = dataSize + sizeof(BucketItem);
	bucket->bucketId = ((uint64_t)gBucketId++) << 48;
	bucket->items = malloc(bucket->capacity * bucket->itemSize);
	assert(bucket->items);
	Bucket_UnmarkAll(bucket);
	return 0;
}

void Bucket_Term(Bucket* bucket) {
	if (bucket->items) {
		memset(bucket->items, 0, bucket->capacity * bucket->itemSize);
		free(bucket->items);
	}
	memset(bucket, 0, sizeof(Bucket));
}

void* Bucket_Mark(Bucket* bucket, void* copy, ID* outId) {
	if (bucket->size < bucket->capacity) {
		size_t indexToAllocate = bucket->nextFreeIndex;
		BucketItem* itemToAllocate = BucketItemData(bucket, indexToAllocate);
		bucket->nextFreeIndex = (itemToAllocate->id) & 0xFFFF; // Extract next free index
		itemToAllocate->id = ((uint16_t)bucket->nextKey << 16) | ((uint16_t)indexToAllocate & 0xFFFF); // Store new id of the item
		if (outId) {
			*outId = bucket->bucketId | (uint64_t)itemToAllocate->id; // Return id
		}

		bucket->size++;
		bucket->nextKey++;
		if (bucket->nextKey > UINT16_MAX) {
			bucket->nextKey = 1; // Rewind key to beginning
		}
		if (bucket->highestAllocatedIndex < indexToAllocate) {
			bucket->highestAllocatedIndex = indexToAllocate;
		}
		if (bucket->lowestAllocatedIndex > indexToAllocate) {
			bucket->lowestAllocatedIndex = indexToAllocate;
		}

		if (copy) {
			memcpy(&(itemToAllocate->data), copy, bucket->dataSize);
		}
		return &(itemToAllocate->data);
	} else {
		return NULL;
	}
}

void Bucket_Unmark(Bucket* bucket, void* data) {
	if (!Bucket_IsMarked(bucket, data)) {
		return;
	}

	BucketItem* itemToDelete = BucketItemFromData(data);
	size_t indexToDelete = itemToDelete->id & 0xFFFF;
	itemToDelete->id = bucket->nextFreeIndex & 0xFFFF; // Set item as free
	bucket->nextFreeIndex = indexToDelete; // Store next free index

	bucket->size--;
	if (bucket->highestAllocatedIndex == indexToDelete) {
		// Search backwards until highest allocated index is found
		for (size_t i = indexToDelete; i-- > 0; ) {
			bucket->highestAllocatedIndex = i;
			if (BucketItemData(bucket, i)->id & 0xFFFF0000) {
				break;
			}
		}
	}
	if (bucket->lowestAllocatedIndex == indexToDelete) {
		// Search forward until lowest allocated index is found
		for (size_t i = indexToDelete + 1; i < bucket->capacity; i++) {
			bucket->lowestAllocatedIndex = i;
			if (BucketItemData(bucket, i)->id & 0xFFFF0000) {
				break;
			}
		}
	}
}

void Bucket_UnmarkByIndex(Bucket* bucket, size_t idx) {
	void* item = Bucket_GetByIndex(bucket, idx);
	if (item) {
		Bucket_Unmark(bucket, item);
	}
}

void Bucket_UnmarkById(Bucket* bucket, ID id) {
	void* item = Bucket_GetById(bucket, id);
	if (item) {
		Bucket_Unmark(bucket, item);
	}
}

void Bucket_UnmarkAll(Bucket* bucket) {
	bucket->size = 0;
	bucket->nextKey = 1;
	bucket->highestAllocatedIndex = 0;
	bucket->lowestAllocatedIndex = 0;
	bucket->nextFreeIndex = 0;
	if (bucket->items) {
		for (size_t i = 0; i < bucket->capacity; i++) {
			// Each item points to next item as free
			BucketItemData(bucket, i)->id = 0xFFFF & (i + 1);
		}
	}
}

bool Bucket_IsMarked(Bucket* bucket, void* data) {
	return Bucket_GetId(bucket, data) != 0;
}

bool Bucket_IsMarkedByIndex(Bucket* bucket, size_t idx) {
	void* data = Bucket_GetByIndex(bucket, idx);
	return data != NULL;
}

bool Bucket_IsMarkedById(Bucket* bucket, ID id) {
	void* data = Bucket_GetById(bucket, id);
	return data != NULL;
}

void* Bucket_GetFirst(Bucket* bucket) {
	if (bucket->size) {
		return Bucket_GetByIndex(bucket, bucket->lowestAllocatedIndex);
	}
	return NULL;
}

void* Bucket_GetLast(Bucket* bucket) {
	if (bucket->size) {
		return Bucket_GetByIndex(bucket, bucket->highestAllocatedIndex);
	}
	return NULL;
}

void* Bucket_GetNext(Bucket* bucket, void* currData) {
	uint32_t currId = (uint32_t)Bucket_GetId(bucket, currData);
	if (currId == 0) {
		return NULL;
	}
	size_t currIdx = currId & 0xFFFF;
	for (size_t i = currIdx + 1; i <= bucket->highestAllocatedIndex; i++) {
		void* data = Bucket_GetByIndex(bucket, i);
		if (data) {
			return data;
		}
	}
	return NULL;
}

void* Bucket_GetPrev(Bucket* bucket, void* currData) {
	uint32_t currId = (uint32_t)Bucket_GetId(bucket, currData);
	if (currId == 0) {
		return NULL;
	}
	size_t currIdx = currId & 0xFFFF;
	for (size_t i = currIdx; i-- > bucket->lowestAllocatedIndex; ) {
		void* data = Bucket_GetByIndex(bucket, i);
		if (data) {
			return data;
		}
	}
	return NULL;
}

void* Bucket_GetByIndex(Bucket* bucket, size_t idx) {
	BucketItem* candidateItem = BucketItemData(bucket, idx);
	if (candidateItem->id & 0xFFFF0000) {
		return &(candidateItem->data);
	}
	return NULL;
}

void* Bucket_GetById(Bucket* bucket, ID id) {
	if (bucket->bucketId != (id & 0xFFFF000000000000ull)) {
		return NULL;
	}
	size_t candidateIdx = (uint32_t)id & 0xFFFFu;
	BucketItem* candidateItem = BucketItemData(bucket, candidateIdx);
	if (candidateItem->id == ((uint32_t)id & 0xFFFFFFFFu)) {
		return &(candidateItem->data);
	}
	return NULL;
}

ID Bucket_GetId(Bucket* bucket, void* data) {
	BucketItem* itemToCheck = BucketItemFromData(data);
	// Check if object resides in range
	if ((itemToCheck < BucketItemData(bucket, bucket->lowestAllocatedIndex)) || (BucketItemData(bucket, bucket->highestAllocatedIndex + 1) < itemToCheck)) {
		return 0;
	}
	// Check if object is allocated
	if (itemToCheck->id & 0xFFFF0000) {
		return bucket->bucketId | (uint64_t)itemToCheck->id;
	} else {
		return 0;
	}
}
