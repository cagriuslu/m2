#ifndef SPATIAL_MAP_H
#define SPATIAL_MAP_H

#include "m2/Pool.hh"
#include "m2/Box2D.hh"
#include "m2/AABB.hh"
#include <vector>

typedef struct _SpatialMap {
	Pool bucket;
	Box2DWorld* world;
	size_t dataSize;
} SpatialMap;

int SpatialMap_Init(SpatialMap* sm, size_t dataSize);
void SpatialMap_Term(SpatialMap* sm);

size_t SpatialMap_Size(SpatialMap* sm);
void SpatialMap_Clear(SpatialMap* sm);
ID SpatialMap_Add(SpatialMap* sm, m2::vec2f position, float boundaryRadius, void* copy); // Returns iterator
void SpatialMap_Remove(SpatialMap* sm, ID iterator);
void SpatialMap_SetPosition(SpatialMap* sm, ID iterator, m2::vec2f position);
void SpatialMap_Get(SpatialMap* sm, AABB bounds, std::vector<ID>& outIterators);

void* SpatialMap_GetData(SpatialMap* sm, ID iterator);

#endif
