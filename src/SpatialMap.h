#ifndef SPATIAL_MAP_H
#define SPATIAL_MAP_H

#include "Pool.h"
#include "Box2D.h"
#include "AABB.h"
#include "Array.h"

typedef struct _SpatialMap {
	Pool bucket;
	Box2DWorld* world;
	size_t dataSize;
} SpatialMap;

int SpatialMap_Init(SpatialMap* sm, size_t dataSize);
void SpatialMap_Term(SpatialMap* sm);

size_t SpatialMap_Size(SpatialMap* sm);
void SpatialMap_Clear(SpatialMap* sm);
ID SpatialMap_Add(SpatialMap* sm, Vec2F position, float boundaryRadius, void* copy); // Returns iterator
void SpatialMap_Remove(SpatialMap* sm, ID iterator);
void SpatialMap_SetPosition(SpatialMap* sm, ID iterator, Vec2F position);
void SpatialMap_Get(SpatialMap* sm, AABB bounds, Array* outIterators);

void* SpatialMap_GetData(SpatialMap* sm, ID iterator);

#endif
