#ifndef OBJECT_STORE_H
#define OBJECT_STORE_H

#include "Object.h"
#include <stdint.h>

#define CreateObject() (ObjectStoreCreateObject(CurrentObjectStore(), NULL, NULL))

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
	int lowestAllocatedIndex;
	int nextFreeIndex;
} ObjectStore;

int ObjectStoreInit(ObjectStore* os);
Object* ObjectStoreCreateObject(ObjectStore* os, Object* copy, uint32_t*outId);
bool ObjectStoreIsOwnerOfObject(ObjectStore* os, Object* ref);
Object* ObjectStoreGetFirstObject(ObjectStore* os);
Object* ObjectStoreGetNextObject(ObjectStore* os, Object *ref);
Object* ObjectStoreGetObjectByIndex(ObjectStore* os, int idx);
Object* ObjectStoreGetObjectById(ObjectStore* os, uint32_t id);
uint32_t ObjectStoreGetIdByObject(ObjectStore* os, Object* ref);
void ObjectStoreDestroyObject(ObjectStore* os, Object* ref);
void ObjectStoreDestroyObjectById(ObjectStore* os, uint32_t id);
void ObjectStoreDestroyAllObjects(ObjectStore* os);

#endif
