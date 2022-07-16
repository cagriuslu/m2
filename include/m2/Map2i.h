#ifndef M2_MAP2I_H
#define M2_MAP2I_H

#include "Pool.hh"
#include "Vec2i.h"
#include <map>

namespace m2 {
	using Map2iID = ID;

	template <typename T>
	struct Map2iItem {
		Vec2i pos;
		T obj;
	};

	template <typename T, uint64_t Capacity = 65536>
	class Map2i : private Pool<Map2iItem<T>,Capacity> {
		std::multimap<int32_t, Map2iID> _xmap;
		std::multimap<int32_t, Map2iID> _ymap;

	public:
		Map2i() : Pool<Map2iItem<T>, Capacity>() {}

		std::pair<T&,Map2iID> alloc(const Vec2i& pos) {
			auto alloced_pair = Pool<Map2iItem<T>,Capacity>::alloc();
			return {alloced_pair.first.obj, alloced_pair.second};
		}
		using Pool<Map2iItem<T>,Capacity>::free;
		using Pool<Map2iItem<T>,Capacity>::clear;

		using Pool<Map2iItem<T>,Capacity>::size;
		using Pool<Map2iItem<T>,Capacity>::contains;

		using Pool<Map2iItem<T>,Capacity>::operator[];
		using Pool<Map2iItem<T>,Capacity>::get;
		using Pool<Map2iItem<T>,Capacity>::get_id;

		std::vector<T*> find(const Vec2i& pos, int32_t radius) {
			std::vector<const Map2iItem<T>*> x_items;
			std::vector<const Map2iItem<T>*> y_items;
			// TODO
			return {};
		}
		std::vector<T*> find(Map2iID id, int32_t radius) {
			auto* item = get(id);
			if (item) {
				return find(item->pos, radius);
			}
			return {};
		}
	};
}

#endif //M2_MAP2I_H
