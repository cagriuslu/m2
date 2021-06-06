#include "ObjectStore.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int ObjectStoreInit(ObjectStore* os) {
	os->items = calloc(UINT16_MAX + 1, sizeof(ObjectStoreItem));
	assert(os->items);
	os->capacity = UINT16_MAX + 1;
	os->size = 0;
	os->nextKey = 1;
	os->highestAllocatedIndex = 0;
	os->nextFreeIndex = 0;

	for (int i = 0; i < UINT16_MAX + 1; i++) {
		ObjectStoreItem* item = os->items + i;
		item->id = (i + 1) & 0xFFFF; // Each item points to next item as free
	}

	return 0;
}

Object* ObjectStoreCreateObject(ObjectStore* os, Object* copy) {
	if (os->size < os->capacity) {
		int indexToAllocate = os->nextFreeIndex;
		ObjectStoreItem* itemToAllocate = &os->items[indexToAllocate];
		os->nextFreeIndex = (itemToAllocate->id) & 0xFFFF; // Extract next free index
		itemToAllocate->id = (os->nextKey << 16) | (indexToAllocate & 0xFFFF); // Store new id of the item

		os->size++;
		os->nextKey++;
		if (os->nextKey > (UINT16_MAX + 1)) {
			os->nextKey = 1; // Rewind key to beginning
		}
		if (os->highestAllocatedIndex < indexToAllocate) {
			os->highestAllocatedIndex = indexToAllocate;
		}

		if (copy) {
			memcpy(&itemToAllocate->obj, copy, sizeof(Object));
		}
		return &itemToAllocate->obj;
	} else {
		return NULL;
	}
}

Object* ObjectStoreGetObjectById(ObjectStore* os, int id) {
	if (0 < os->size) {
		int candidateIdx = id & 0xFFFF;
		if (candidateIdx <= os->highestAllocatedIndex) {
			ObjectStoreItem* candidateItem = &os->items[candidateIdx];
			if (candidateItem->id == id) {
				return &candidateItem->obj;
			}
		}
	}
	return NULL;
}

void ObjectStoreDestroyObject(ObjectStore* os, Object* ref) {
	if (0 < os->size) {
		ObjectStoreItem* itemToDelete = (ObjectStoreItem*)ref;
		if (itemToDelete->id & 0xFFFF0000) { // Check if there is a key assigned
			int indexToDelete = itemToDelete->id & 0xFFFF;

			itemToDelete->id = os->nextFreeIndex & 0xFFFF; // Set item as free
			os->nextFreeIndex = indexToDelete; // Store next free index

			os->size--;
			if (os->highestAllocatedIndex == indexToDelete) {
				// Search backwards until highest allocated index is found
				for (int i = indexToDelete; i-- > 0; ) {
					os->highestAllocatedIndex = i;
					if (os->items[i].id & 0xFFFF0000) {
						break;
					}
				}
			}
		}
	}
}
