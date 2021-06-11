#ifndef OBJECT_STORE_H
#define OBJECT_STORE_H

#include "GameObject.h"
#include <stdint.h>

#define CreateObject() (ObjectStoreCreateObject(CurrentObjectStore(), NULL, NULL))

typedef struct _ObjectStoreItem {
	GameObject obj;
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
GameObject* ObjectStoreCreateObject(ObjectStore* os, GameObject* copy, uint32_t*outId);
bool ObjectStoreIsOwnerOfObject(ObjectStore* os, GameObject* ref);
GameObject* ObjectStoreGetFirstObject(ObjectStore* os);
GameObject* ObjectStoreGetNextObject(ObjectStore* os, GameObject *ref);
GameObject* ObjectStoreGetObjectByIndex(ObjectStore* os, int idx);
GameObject* ObjectStoreGetObjectById(ObjectStore* os, uint32_t id);
uint32_t ObjectStoreGetIdByObject(ObjectStore* os, GameObject* ref);
void ObjectStoreDestroyObject(ObjectStore* os, GameObject* ref);
void ObjectStoreDestroyObjectById(ObjectStore* os, uint32_t id);
void ObjectStoreDestroyAllObjects(ObjectStore* os);

#endif
