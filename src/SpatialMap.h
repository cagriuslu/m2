#ifndef SPATIAL_MAP_H
#define SPATIAL_MAP_H

#include "Bucket.h"
#include "Box2D.h"
#include "AABB.h"
#include "Array.h"

typedef struct _SpatialMap {
	Bucket bucket;
	Box2DWorld* world;
	size_t dataSize;
} SpatialMap;

int SpatialMapInit(SpatialMap* sm, size_t dataSize);
void SpatialMapDeinit(SpatialMap* sm);

size_t SpatialMapSize(SpatialMap* sm);
void SpatialMapClear(SpatialMap* sm);
ID SpatialMapAdd(SpatialMap* sm, Vec2F position, float boundaryRadius, void* copy); // Returns iterator
void SpatialMapRemove(SpatialMap* sm, ID iterator);
void SpatialMapSetPosition(SpatialMap* sm, ID iterator, Vec2F position);
void SpatialMapGet(SpatialMap* sm, AABB bounds, Array* outIterators);

void* SpatialMapGetData(SpatialMap* sm, ID iterator);

#endif
