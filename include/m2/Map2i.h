#ifndef M2_MAP2I_H
#define M2_MAP2I_H

#include "Pool.hh"
#include "Vec2i.h"
#include <map>

namespace m2 {
	using Map2iID = ID;

	template <typename T, uint64_t Capacity = 65536>
	class Map2i : private Pool<T,Capacity> {
		std::multimap<int32_t, Map2iID> _xmap;
		std::multimap<int32_t, Map2iID> _ymap;

	public:
		Map2i() : Pool<T, Capacity>() {}

		std::pair<T&, Map2iID> alloc(const Vec2i& pos);
		using Pool<T,Capacity>::free;
		using Pool<T,Capacity>::clear;

		using Pool<T,Capacity>::size;
		using Pool<T,Capacity>::contains;

		using Pool<T,Capacity>::operator[];
		using Pool<T,Capacity>::get;
		using Pool<T,Capacity>::get_id;

		std::vector<T*> find(const Vec2i& pos, int32_t radius);
		std::vector<T*> find(Map2iID id, int32_t radius);
	};
}

#endif //M2_MAP2I_H
