#ifndef M2_RTREE_H
#define M2_RTREE_H

#include "AABB2f.h"
#include "Pool.hh"

namespace m2 {
	using RtreeID = ID;

	template <typename T>
	struct RtreeItem {
		AABB2f aabb;
		T obj;
	};

	template <typename T, uint64_t Capacity>
	class Rtree : private Pool<RtreeItem<T>,Capacity> {
	};
}

#endif //M2_RTREE_H
