#ifndef M2_MAP2I_H
#define M2_MAP2I_H

#include "Pool.hh"
#include "Vec2i.h"
#include <map>
#include <algorithm>
#include <iterator>

namespace m2 {
	using Map2iID = Id;

	template <typename T>
	struct Map2iItem {
		Vec2i pos;
		T obj;
	};

	template <typename T, uint64_t Capacity = 65536>
	class Map2i : private Pool<Map2iItem<T>,Capacity> {
		using x_pos = int32_t;
		using y_pos = int32_t;
		std::multimap<x_pos, std::pair<y_pos, Map2iID>> _map;

	public:
		Map2i() : Pool<Map2iItem<T>, Capacity>() {}

		std::pair<T&,Map2iID> alloc(const Vec2i& pos) {
			auto [item, id] = Pool<Map2iItem<T>,Capacity>::alloc();
			item.pos = pos;
			_map.insert({pos.x, {pos.y, id}});
			return {item.obj, id};
		}
		using Pool<Map2iItem<T>,Capacity>::free;
		using Pool<Map2iItem<T>,Capacity>::clear;

		using Pool<Map2iItem<T>,Capacity>::size;
		using Pool<Map2iItem<T>,Capacity>::contains;

		using Pool<Map2iItem<T>,Capacity>::operator[];
		using Pool<Map2iItem<T>,Capacity>::get;
		using Pool<Map2iItem<T>,Capacity>::get_id;

		std::vector<Map2iID> find_ids(const Vec2i& pos, int32_t radius) {
			std::vector<Map2iID> items;
			for (auto it = _map.lower_bound(pos.x - radius); it != _map.end() && it->first <= pos.x + radius; it++) {
				y_pos y = it->second.first;
				if ((pos.y - radius) <= y && y <= (pos.y + radius)) {
					items.emplace_back(it->second.second);
				}
			}
			return items;
		}
		std::vector<Map2iID> find_ids(Map2iID id, int32_t radius) {
			auto* item = get(id);
			if (item) {
				return find_ids(item->pos, radius);
			}
			return {};
		}
		std::vector<T*> find_objects(const Vec2i& pos, int32_t radius) {
			auto ids = find_ids(pos, radius);
			std::vector<T*> objects;
			std::transform(ids.cbegin(), ids.cend(), std::back_inserter(objects), [=](Map2iID id) {
				return &get(id)->obj;
			});
			return objects;
		}
		std::vector<T*> find_objects(Map2iID id, int32_t radius) {
			auto* item = get(id);
			if (item) {
				return find_objects(item->pos, radius);
			}
			return {};
		}
	};
}

#endif //M2_MAP2I_H
