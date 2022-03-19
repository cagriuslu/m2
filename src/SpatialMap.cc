#include "m2/SpatialMap.hh"
#include "m2/Def.hh"

static Box2DBody* CreateBody(Box2DWorld* world, m2::vec2f position, float boundaryRadius, ID iterator) {
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
	char data[];
} SpatialMapItem;

int SpatialMap_Init(SpatialMap* sm, size_t dataSize) {
	memset(sm, 0, sizeof(SpatialMap));
	sm->dataSize = dataSize;
	M2ERR_REFLECT(Pool_Init(&sm->bucket, 16, sizeof(SpatialMapItem) + dataSize));
	sm->world = Box2DWorldCreate(m2::vec2f{});
	// TODO proper error check
	return 0;
}

void SpatialMap_Term(SpatialMap* sm) {
	Box2DWorldDestroy(sm->world);
	Pool_Term(&sm->bucket);
	memset(sm, 0, sizeof(SpatialMap));
}

size_t SpatialMap_Size(SpatialMap* sm) {
	return sm->bucket.size;
}

void SpatialMap_Clear(SpatialMap* sm) {
	Pool_UnmarkAll(&sm->bucket);
	Box2DWorldDestroy(sm->world);
	sm->world = Box2DWorldCreate(m2::vec2f{});
	// TODO proper error check
}

ID SpatialMap_Add(SpatialMap* sm, m2::vec2f position, float boundaryRadius, void* copy) {
	ID iterator = 0;
	SpatialMapItem* item = static_cast<SpatialMapItem *>(Pool_Mark(&sm->bucket, NULL, &iterator));
	item->body = CreateBody(sm->world, position, boundaryRadius, iterator);
	if (copy) {
		memcpy(item->data, copy, sm->dataSize);
	} else {
		memset(item->data, 0, sm->dataSize);
	}
	return iterator;
}

void SpatialMap_Remove(SpatialMap* sm, ID iterator) {
	SpatialMapItem* item = static_cast<SpatialMapItem *>(Pool_GetById(&sm->bucket, iterator));
	if (item) {
		Box2DWorldDestroyBody(sm->world, item->body);
		memset(item->data, 0, sm->dataSize);
		Pool_UnmarkById(&sm->bucket, iterator);
	}
}

void SpatialMap_SetPosition(SpatialMap* sm, ID iterator, m2::vec2f position) {
	SpatialMapItem* item = static_cast<SpatialMapItem *>(Pool_GetById(&sm->bucket, iterator));
	if (item) {
		Box2DBodySetTransform(item->body, position, 0.0f);
	}
}

static bool StaticMapQueryCB(Box2DFixture* fixture, void* userDataArrayOfIterators) {
	Box2DBody* body = Box2DFixtureGetBody(fixture);
	ID iterator = (ID)((uintptr_t)Box2DBodyGetUserData(body));
	if (userDataArrayOfIterators) {
		static_cast<std::vector<ID>*>(userDataArrayOfIterators)->push_back(iterator);
	}
	return true;
}

void SpatialMap_Get(SpatialMap* sm, AABB bounds, std::vector<ID>& outIterators) {
	outIterators.clear();
	Box2DQueryListener* queryListener = Box2DQueryListenerCreate(StaticMapQueryCB, &outIterators);
	Box2DWorldQuery(sm->world, queryListener, bounds);
	Box2DQueryListenerDestroy(queryListener);
}

void* SpatialMap_GetData(SpatialMap* sm, ID iterator) {
	SpatialMapItem* item = static_cast<SpatialMapItem *>(Pool_GetById(&sm->bucket, iterator));
	return item ? item->data : NULL;
}
