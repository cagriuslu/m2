#ifndef M2_MAP2F_H
#define M2_MAP2F_H

#include "Pool.hh"
#include "Vec2f.h"
#include <array>
#include <variant>

namespace m2 {
	using Map2fID = ID;

	template <typename T>
	struct Map2fItem {
		Vec2f pos;
		T obj;
	};

	template <typename T, uint64_t Capacity = 65536, uint32_t LinearN = 64>
	class Map2f : private Pool<Map2fItem<T>,Capacity> {
		struct ArrayItem {
			Vec2f pos;
			Map2fID id{0};
		};
		using Array = std::vector<ArrayItem>;

		struct Quads;
		using ArrayOrQuads = std::variant<Array*, Quads*>;
		struct Quads {
			Vec2f origin;
			ArrayOrQuads quad[4];
			unsigned find_quad_index(const Vec2f& pos) {
				if (origin.x <= pos.x && origin.y <= pos.y) {
					return 0;
				} else if (pos.x < origin.x && origin.y <= pos.y) {
					return 1;
				} else if (pos.x < origin.x && pos.y < origin.y) {
					return 2;
				}
				return 3;
			}
		};
		ArrayOrQuads _root;

		ArrayOrQuads* find_node(const Vec2f& pos) {
			ArrayOrQuads* array_or_quads = &_root;
			while (std::holds_alternative<Quads*>(*array_or_quads)) {
				auto* quads = std::get<Quads*>(*array_or_quads);
				auto index = quads->find_quad_index(pos);
				array_or_quads = &quads->quad[index];
			}
			return array_or_quads;
		}

	public:
		Map2f() : Pool<Map2fItem<T>,Capacity>() {}

		std::pair<T&, Map2fID> alloc(const Vec2f& pos) {
			auto& [item, id] = Pool<Map2fItem<T>,Capacity>::alloc();
			item.pos = pos;

			Array* array = nullptr;
			{
				Array* provisional_array = std::get<Array*>(*find_node(pos));
				if (LinearN <= provisional_array->size()) {
					// TODO allocate quad
					// TODO what if all items are on the same position exactly
				} else {
					array = provisional_array;
				}
			}

			auto array_item = ArrayItem{pos, id};

			// TODO

			return {item.obj, id};
		}
		using Pool<Map2fItem<T>,Capacity>::free;
		using Pool<Map2fItem<T>,Capacity>::clear;

		using Pool<Map2fItem<T>,Capacity>::size;
		using Pool<Map2fItem<T>,Capacity>::contains;

		using Pool<Map2fItem<T>,Capacity>::operator[];
		using Pool<Map2fItem<T>,Capacity>::get;
		using Pool<Map2fItem<T>,Capacity>::get_id;

		std::vector<Map2fID> find_ids(const Vec2f& pos, float radius) {
			// TODO
			return {};
		}
		std::vector<Map2fID> find_ids(Map2fID id, float radius) {
			auto* item = get(id);
			if (item) {
				return find_ids(item->pos, radius);
			}
			return {};
		}
		std::vector<T*> find_objects(const Vec2i& pos, float radius) {
			auto ids = find_ids(pos, radius);
			std::vector<T*> objects;
			std::transform(ids.cbegin(), ids.cend(), std::back_inserter(objects), [=](Map2fID id) {
				return &get(id)->obj;
			});
			return objects;
		}
		std::vector<T*> find_objects(Map2fID id, float radius) {
			auto* item = get(id);
			if (item) {
				return find_objects(item->pos, radius);
			}
			return {};
		}
	};
}

#endif //M2_MAP2F_H
