#ifndef M2_MAP2I_H
#define M2_MAP2I_H

#include "Pool.hh"
#include "Vec2i.h"
#include <map>
#include <algorithm>

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
			alloced_pair.first.pos = pos;
			_xmap.insert({pos.x, alloced_pair.second});
			_ymap.insert({pos.y, alloced_pair.second});
			return {alloced_pair.first.obj, alloced_pair.second};
		}
		using Pool<Map2iItem<T>,Capacity>::free;
		using Pool<Map2iItem<T>,Capacity>::clear;

		using Pool<Map2iItem<T>,Capacity>::size;
		using Pool<Map2iItem<T>,Capacity>::contains;

		using Pool<Map2iItem<T>,Capacity>::operator[];
		using Pool<Map2iItem<T>,Capacity>::get;
		using Pool<Map2iItem<T>,Capacity>::get_id;

		std::vector<Map2iID> find_ids(const Vec2i& pos, int32_t radius) {
			// Find items in x-range
			std::vector<Map2iID> x_items;
			for (auto it = _xmap.lower_bound(pos.x - radius); it != _xmap.end() && it->first <= pos.x + radius; it++) {
				x_items.emplace_back(it->second);
			}
			fprintf(stderr, "Map2i x_items count %zu\n", x_items.size());
			if (x_items.empty()) {
				return {};
			}
			// Find items in y-range
			std::vector<Map2iID> y_items;
			for (auto it = _ymap.lower_bound(pos.y - radius); it != _ymap.end() && it->first <= pos.y + radius; it++) {
				y_items.emplace_back(it->second);
			}
			fprintf(stderr, "Map2i y_items count %zu\n", y_items.size());
			if (y_items.empty()) {
				return {};
			}
			// Sort items
			std::sort(x_items.begin(), x_items.end());
			std::sort(y_items.begin(), y_items.end());
			// Find the intersection of items
			std::vector<Map2iID> intersect;
			std::set_intersection(x_items.begin(), x_items.end(), y_items.begin(), y_items.end(), std::back_inserter(intersect));
			return intersect;
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
