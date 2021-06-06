#ifndef OBJECT_STORE_H
#define OBJECT_STORE_H

#include "Object.h"
#include <stdint.h>

typedef struct _ObjectStoreItem {
	Object obj;
	uint32_t id; // If allocated, key|index. If free, 0|nextFreeIndex
} ObjectStoreItem;

typedef struct _ObjectStore {
	ObjectStoreItem* items;
	int capacity; // 65536
	int size; // [0, 65536]
	int nextKey; // [1, 65536]
	int highestAllocatedIndex;
	int nextFreeIndex;
} ObjectStore;

int ObjectStoreInit(ObjectStore* os);
Object* ObjectStoreCreateObject(ObjectStore* os, Object* copy);
Object* ObjectStoreGetObjectById(ObjectStore* os, int id);
void ObjectStoreDestroyObject(ObjectStore* os, Object* ref);

#endif
