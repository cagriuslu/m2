#include "Bucket.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdio.h>

#define BucketItemData(bucket, index) ((BucketItem*) (((int8_t*) ((bucket)->items)) + ((bucket)->itemSize * (index))))
#define BucketItemFromData(itemData) ((BucketItem*) (((int8_t*) (itemData)) - offsetof(BucketItem, data)))

int BucketInit(Bucket* bucket, size_t dataSize) {
	bucket->capacity = UINT16_MAX + 1;
	bucket->dataSize = dataSize;
	bucket->itemSize = dataSize + sizeof(BucketItem);
	bucket->items = malloc(bucket->capacity * bucket->itemSize);
	assert(bucket->items);
	BucketUnmarkAll(bucket);
	return 0;
}

void BucketDeinit(Bucket* bucket) {
	if (bucket->items) {
		memset(bucket->items, 0, bucket->capacity * bucket->itemSize);
		free(bucket->items);
	}
	memset(bucket, 0, sizeof(Bucket));
}

void* BucketMark(Bucket* bucket, void* copy, uint32_t* outId) {
	if (bucket->size < bucket->capacity) {
		int indexToAllocate = bucket->nextFreeIndex;



		BucketItem* itemToAllocate = BucketItemData(bucket, indexToAllocate);
		fprintf(stderr, "id before allocate %08x\n", itemToAllocate->id);
		bucket->nextFreeIndex = (itemToAllocate->id) & 0xFFFF; // Extract next free index
		itemToAllocate->id = (bucket->nextKey << 16) | (indexToAllocate & 0xFFFF); // Store new id of the item
		if (outId) {
			*outId = itemToAllocate->id; // Return id
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

void BucketUnmark(Bucket* bucket, void* data) {
	if (!BucketIsMarked(bucket, data)) {
		return;
	}

	BucketItem* itemToDelete = BucketItemFromData(data);
	int indexToDelete = itemToDelete->id & 0xFFFF;
	itemToDelete->id = bucket->nextFreeIndex & 0xFFFF; // Set item as free
	bucket->nextFreeIndex = indexToDelete; // Store next free index

	bucket->size--;
	if (bucket->highestAllocatedIndex == indexToDelete) {
		// Search backwards until highest allocated index is found
		for (int i = indexToDelete; i-- > 0; ) {
			bucket->highestAllocatedIndex = i;
			if (BucketItemData(bucket, i)->id & 0xFFFF0000) {
				break;
			}
		}
	}
	if (bucket->lowestAllocatedIndex == indexToDelete) {
		// Search forward until lowest allocated index is found
		for (int i = indexToDelete + 1; i < bucket->capacity; i++) {
			bucket->lowestAllocatedIndex = i;
			if (BucketItemData(bucket, i)->id & 0xFFFF0000) {
				break;
			}
		}
	}
}

void BucketUnmarkByIndex(Bucket* bucket, int idx) {
	void* item = BucketGetByIndex(bucket, idx);
	if (item) {
		BucketUnmark(bucket, item);
	}
}

void BucketUnmarkById(Bucket* bucket, uint32_t id) {
	void* item = BucketGetById(bucket, id);
	if (item) {
		BucketUnmark(bucket, item);
	}
}

void BucketUnmarkAll(Bucket* bucket) {
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

bool BucketIsMarked(Bucket* bucket, void* data) {
	return BucketGetId(bucket, data) != 0;
}

bool BucketIsMarkedByIndex(Bucket* bucket, int idx) {
	void* data = BucketGetByIndex(bucket, idx);
	return data != NULL;
}

bool BucketIsMarkedById(Bucket* bucket, uint32_t id) {
	void* data = BucketGetById(bucket, id);
	return data != NULL;
}

void* BucketGetFirst(Bucket* bucket) {
	if (bucket->size) {
		return BucketGetByIndex(bucket, bucket->lowestAllocatedIndex);
	}
	return NULL;
}

void* BucketGetNext(Bucket* bucket, void* currData) {
	uint32_t currId = BucketGetId(bucket, currData);
	if (currId == 0) {
		return NULL;
	}
	int currIdx = currId & 0xFFFF;
	for (int i = currIdx + 1; i <= bucket->highestAllocatedIndex; i++) {
		void* data = BucketGetByIndex(bucket, i);
		if (data) {
			return data;
		}
	}
	return NULL;
}

void* BucketGetPrev(Bucket* bucket, void* currData) {
	uint32_t currId = BucketGetId(bucket, currData);
	if (currId == 0) {
		return NULL;
	}
	int currIdx = currId & 0xFFFF;
	for (int i = currIdx; i-- > bucket->lowestAllocatedIndex; ) {
		void* data = BucketGetByIndex(bucket, i);
		if (data) {
			return data;
		}
	}
	return NULL;
}

void* BucketGetByIndex(Bucket* bucket, int idx) {
	BucketItem* candidateItem = BucketItemData(bucket, idx);
	if (candidateItem->id & 0xFFFF0000) {
		return &(candidateItem->data);
	}
	return NULL;
}

void* BucketGetById(Bucket* bucket, uint32_t id) {
	int candidateIdx = id & 0xFFFF;
	BucketItem* candidateItem = BucketItemData(bucket, candidateIdx);
	if (candidateItem->id == id) {
		return &(candidateItem->data);
	}
	return NULL;
}

uint32_t BucketGetId(Bucket* bucket, void* data) {
	BucketItem* itemToCheck = BucketItemFromData(data);
	// Check if object resides in range
	if ((itemToCheck < BucketItemData(bucket, bucket->lowestAllocatedIndex)) || (BucketItemData(bucket, bucket->highestAllocatedIndex + 1) < itemToCheck)) {
		return false;
	}
	// Check if object is allocated
	if (itemToCheck->id & 0xFFFF0000) {
		return itemToCheck->id;
	} else {
		return 0;
	}
}
