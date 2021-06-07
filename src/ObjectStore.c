#include "ObjectStore.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int ObjectStoreInit(ObjectStore* os) {
	os->capacity = UINT16_MAX + 1;
	os->items = calloc(os->capacity, sizeof(ObjectStoreItem));
	assert(os->items);
	ObjectStoreDestroyAllObjects(os);
	return 0;
}

Object* ObjectStoreCreateObject(ObjectStore* os, Object* copy, uint32_t* outId) {
	if (os->size < os->capacity) {
		int indexToAllocate = os->nextFreeIndex;
		ObjectStoreItem* itemToAllocate = &os->items[indexToAllocate];
		os->nextFreeIndex = (itemToAllocate->id) & 0xFFFF; // Extract next free index
		itemToAllocate->id = (os->nextKey << 16) | (indexToAllocate & 0xFFFF); // Store new id of the item
		if (outId) {
			*outId = itemToAllocate->id;
		}

		os->size++;
		os->nextKey++;
		if (os->nextKey > (UINT16_MAX + 1)) {
			os->nextKey = 1; // Rewind key to beginning
		}
		if (os->highestAllocatedIndex < indexToAllocate) {
			os->highestAllocatedIndex = indexToAllocate;
		}
		if (indexToAllocate < os->lowestAllocatedIndex) {
			os->lowestAllocatedIndex = indexToAllocate;
		}

		if (copy) {
			memcpy(&itemToAllocate->obj, copy, sizeof(Object));
		}
		return &itemToAllocate->obj;
	} else {
		return NULL;
	}
}

bool ObjectStoreIsOwnerOfObject(ObjectStore* os, Object* ref) {
	ObjectStoreItem* itemRef = (ObjectStoreItem*)ref;
	// Check if object resides in range
	if ((itemRef < os->items + os->lowestAllocatedIndex) || (os->items + os->highestAllocatedIndex < itemRef)) {
		return false;
	}
	// Check if object is allocated
	return itemRef->id & 0xFFFF0000;
}

Object* ObjectStoreGetFirstObject(ObjectStore* os) {
	if (0 < os->size) {
		return ObjectStoreGetObjectByIndex(os, os->lowestAllocatedIndex);
	}
	return NULL;
}

Object* ObjectStoreGetNextObject(ObjectStore* os, Object* ref) {
	uint32_t currId = ObjectStoreGetIdByObject(os, ref);
	if (currId == 0) {
		return NULL;
	}
	int currIdx = currId & 0xFFFF;
	for (int i = currIdx + 1; i <= os->highestAllocatedIndex; i++) {
		Object* candidate = ObjectStoreGetObjectByIndex(os, i);
		if (candidate) {
			return candidate;
		}
	}
	return NULL;
}

Object* ObjectStoreGetObjectByIndex(ObjectStore* os, int idx) {
	ObjectStoreItem* candidateItem = &os->items[idx];
	if (candidateItem->id & 0xFFFF0000) {
		return &candidateItem->obj;
	}
	return NULL;
}

Object* ObjectStoreGetObjectById(ObjectStore* os, uint32_t id) {
	if (0 < os->size) {
		int candidateIdx = id & 0xFFFF;
		ObjectStoreItem* candidateItem = &os->items[candidateIdx];
		if (candidateItem->id == id) {
			return &candidateItem->obj;
		}
	}
	return NULL;
}

uint32_t ObjectStoreGetIdByObject(ObjectStore* os, Object* ref) {
	return ObjectStoreIsOwnerOfObject(os, ref) ? ((ObjectStoreItem*)ref)->id : 0;
}

void ObjectStoreDestroyObject(ObjectStore* os, Object* ref) {
	if (!ObjectStoreIsOwnerOfObject(os, ref)) {
		return;
	}

	ObjectStoreItem* itemToDelete = (ObjectStoreItem*)ref;
	if ((itemToDelete->id & 0xFFFF0000) == 0) { // Check if item is allocated
		return;
	}
	
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
	if (os->lowestAllocatedIndex == indexToDelete) {
		// Search forward until lowest allocated index is found
		for (int i = indexToDelete + 1; i < os->capacity; i++) {
			os->lowestAllocatedIndex = i;
			if (os->items[i].id & 0xFFFF0000) {
				break;
			}
		}
	}
}

void ObjectStoreDestroyObjectById(ObjectStore* os, uint32_t id) {
	Object* obj = ObjectStoreGetObjectById(os, id);
	if (obj) {
		ObjectStoreDestroyObject(os, obj);
	}
}

void ObjectStoreDestroyAllObjects(ObjectStore* os) {
	os->size = 0;
	os->nextKey = 1;
	os->highestAllocatedIndex = 0;
	os->lowestAllocatedIndex = 0;
	os->nextFreeIndex = 0;
	for (int i = 0; i < os->capacity; i++) {
		os->items[i].id = (i + 1) & 0xFFFF; // Each item points to next item as free
	}
}
