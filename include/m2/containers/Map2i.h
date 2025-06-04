#pragma once
#include <m2/containers/Pool.h>
#include <m2/math/VecI.h>
#include <map>
#include <algorithm>
#include <iterator>

namespace m2 {
	using Map2iID = Id;

	template <typename T>
	struct Map2iItem {
		VecI pos;
		T obj;
	};

	template <typename T, uint64_t Capacity = 65536>
	class Map2i : private Pool<Map2iItem<T>,Capacity> {
		using x_pos = int32_t;
		using y_pos = int32_t;
		std::multimap<x_pos, std::pair<y_pos, Map2iID>> _map;

	public:
		Map2i() : Pool<Map2iItem<T>, Capacity>() {}

		std::pair<T&,Map2iID> Allocate(const VecI& pos) {
			auto it = Pool<Map2iItem<T>,Capacity>::Emplace();
			it->pos = pos;
			_map.insert({pos.x, {pos.y, it.GetId()}});
			return {it->obj, it.GetId()};
		}
		using Pool<Map2iItem<T>,Capacity>::Free;
		using Pool<Map2iItem<T>,Capacity>::Clear;

		using Pool<Map2iItem<T>,Capacity>::Size;
		using Pool<Map2iItem<T>,Capacity>::Contains;

		using Pool<Map2iItem<T>,Capacity>::operator[];
		using Pool<Map2iItem<T>,Capacity>::Get;
		using Pool<Map2iItem<T>,Capacity>::GetId;

		std::vector<Map2iID> FindIds(const VecI& pos, int32_t radius) {
			std::vector<Map2iID> items;
			for (auto it = _map.lower_bound(pos.x - radius); it != _map.end() && it->first <= pos.x + radius; it++) {
				y_pos y = it->second.first;
				if ((pos.y - radius) <= y && y <= (pos.y + radius)) {
					items.emplace_back(it->second.second);
				}
			}
			return items;
		}
		std::vector<Map2iID> FindIds(Map2iID id, int32_t radius) {
			auto* item = Get(id);
			if (item) {
				return FindIds(item->pos, radius);
			}
			return {};
		}
		std::vector<T*> FindObjects(const VecI& pos, int32_t radius) {
			auto ids = FindIds(pos, radius);
			std::vector<T*> objects;
			std::transform(ids.cbegin(), ids.cend(), std::back_inserter(objects), [=](Map2iID id) {
				return &Get(id)->obj;
			});
			return objects;
		}
		std::vector<T*> FindObjects(Map2iID id, int32_t radius) {
			auto* item = Get(id);
			if (item) {
				return FindObjects(item->pos, radius);
			}
			return {};
		}
	};
}
