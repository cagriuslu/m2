#include "SpatialMap.h"
#include "Error.h"
#include <string.h>
#include <assert.h>

static Box2DBody* CreateBody(Box2DWorld* world, Vec2F position, float boundaryRadius, ID iterator) {
	Box2DBodyDef* bodyDef = Box2DBodyDefCreate();
	Box2DBodyDefSetPosition(bodyDef, position);
	Box2DBodyDefSetUserData(bodyDef, (void*)((uintptr_t)iterator));
	Box2DBody* body = Box2DWorldCreateBody(world, bodyDef);
	Box2DBodyDefDestroy(bodyDef);

	Box2DFixtureDef* fixtureDef = Box2DFixtureDefCreate();
	Box2DCircleShape* circleShape = Box2DCircleShapeCreate();
	Box2DCircleShapeSetRadius(circleShape, boundaryRadius); // Disk of circumference 1cm
	Box2DFixtureDefSetShape(fixtureDef, circleShape);
	Box2DBodyCreateFixtureFromFixtureDef(body, fixtureDef);
	Box2DFixtureDefDestroy(fixtureDef);
	Box2DCircleShapeDestroy(circleShape);

	return body;
}

typedef struct _SpatialMapItem {
	Box2DBody* body;
	char data[0];
} SpatialMapItem;

int SpatialMapInit(SpatialMap* sm, size_t dataSize) {
	memset(sm, 0, sizeof(SpatialMap));
	sm->dataSize = dataSize;
	PROPAGATE_ERROR(Bucket_Init(&sm->bucket, sizeof(SpatialMapItem) + dataSize));
	sm->world = Box2DWorldCreate((Vec2F) { 0.0f, 0.0f });
	assert(sm->world);
	return 0;
}

void SpatialMapDeinit(SpatialMap* sm) {
	Box2DWorldDestroy(sm->world);
	Bucket_Term(&sm->bucket);
	memset(sm, 0, sizeof(SpatialMap));
}

size_t SpatialMapSize(SpatialMap* sm) {
	return sm->bucket.size;
}

void SpatialMapClear(SpatialMap* sm) {
	Bucket_UnmarkAll(&sm->bucket);
	Box2DWorldDestroy(sm->world);
	sm->world = Box2DWorldCreate((Vec2F) { 0.0f, 0.0f });
	assert(sm->world);
}

ID SpatialMapAdd(SpatialMap* sm, Vec2F position, float boundaryRadius, void* copy) {
	ID iterator = 0;
	SpatialMapItem* item = Bucket_Mark(&sm->bucket, NULL, &iterator);
	item->body = CreateBody(sm->world, position, boundaryRadius, iterator);
	if (copy) {
		memcpy(item->data, copy, sm->dataSize);
	} else {
		memset(item->data, 0, sm->dataSize);
	}
	return iterator;
}

void SpatialMapRemove(SpatialMap* sm, ID iterator) {
	SpatialMapItem* item = Bucket_GetById(&sm->bucket, iterator);
	if (item) {
		Box2DWorldDestroyBody(sm->world, item->body);
		memset(item->data, 0, sm->dataSize);
		Bucket_UnmarkById(&sm->bucket, iterator);
	}
}

void SpatialMapSetPosition(SpatialMap* sm, ID iterator, Vec2F position) {
	SpatialMapItem* item = Bucket_GetById(&sm->bucket, iterator);
	if (item) {
		Box2DBodySetTransform(item->body, position, 0.0f);
	}
}

static bool StaticMapQueryCB(Box2DFixture* fixture, void* userDataArrayOfIterators) {
	Box2DBody* body = Box2DFixtureGetBody(fixture);
	ID iterator = (ID)((uintptr_t)Box2DBodyGetUserData(body));
	if (userDataArrayOfIterators) {
		Array_Append((Array*)userDataArrayOfIterators, &iterator);
	}
	return true;
}

void SpatialMapGet(SpatialMap* sm, AABB bounds, Array* outIterators) {
	Array_Clear(outIterators);
	Box2DQueryListener* queryListener = Box2DQueryListenerCreate(StaticMapQueryCB, outIterators);
	Box2DWorldQuery(sm->world, queryListener, bounds);
	Box2DQueryListenerDestroy(queryListener);
}

void* SpatialMapGetData(SpatialMap* sm, ID iterator) {
	SpatialMapItem* item = Bucket_GetById(&sm->bucket, iterator);
	return item ? item->data : NULL;
}
