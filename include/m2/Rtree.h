#pragma once
#include "Aabb.h"
#include "Pool.hh"

// Binary tree where each node is an AABB, as you go up the levels, the AABBs get merged. In the highest level, you have the AABB that covers the whole world.
// Instead of binary, we should use k=4 because it has potential to be more efficient
// We must make sure that the objects that are closer to each other form an AABB
// Is it perfectly possible to have intersections among AABBs, the important thing is the centers of the objects are grouped correctly.
// It is actually not a must, but we should have a good enough algorithm for grouping.
// If the grouping accumulates errors, we would start searching all objects

namespace m2 {
	using RtreeID = Id;

	template <typename T>
	struct RtreeItem {
		Aabb aabb;
		T obj;
	};

	template <typename T, uint64_t Capacity>
	class Rtree : private Pool<RtreeItem<T>,Capacity> {
	};
}
