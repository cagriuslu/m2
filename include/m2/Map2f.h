#ifndef M2_MAP2F_H
#define M2_MAP2F_H

#include "Pool.hh"
#include "Vec2f.h"

namespace m2 {
	using Map2fID = ID;

	template <typename T, uint64_t Capacity = 65536>
	class Map2f : private Pool<T,Capacity> {

	public:
		Map2f() : Pool<T,Capacity>() {}

		std::pair<T&, Map2fID> alloc(const Vec2f& pos);
		using Pool<T,Capacity>::free;
		using Pool<T,Capacity>::clear;

		using Pool<T,Capacity>::size;
		using Pool<T,Capacity>::contains;

		using Pool<T,Capacity>::operator[];
		using Pool<T,Capacity>::get;
		using Pool<T,Capacity>::get_id;

		std::vector<T*> find(const Vec2f& pos, float radius);
		std::vector<T*> find(Map2fID id, float radius);
	};
}

#endif //M2_MAP2F_H
