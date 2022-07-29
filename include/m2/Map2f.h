#ifndef M2_MAP2F_H
#define M2_MAP2F_H

#include "Pool.hh"
#include "Vec2f.h"
#include <array>
#include <algorithm>
#include <numeric>
#include <iterator>
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
			ArrayOrQuads quad[4]; /// Order of quads: Bottom-right, Bottom-left, Top-left, Top-right
			Quads(const Vec2f& origin, const Array& array) : origin(origin) {
				for (auto& q : quad) {
					q = new Array();
				}
				for (const auto& item : array) {
					std::get<Array*>(quad[find_quad_index(item.pos)])->emplace_back(item);
				}
			}
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
			void append_intersecting_quads(const Vec2f& pos, float aabb_radius, std::queue<const ArrayOrQuads*>& out) const {
				auto aabb_corners = pos.aabb_corners(aabb_radius);
				for (unsigned i = 0; i < 4; i++) {
					if (find_quad_index(aabb_corners[i]) == i) {
						out.push(&quad[i]);
					}
				}
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
		Map2f() : Pool<Map2fItem<T>,Capacity>(), _root(new Array()) {}

		std::pair<T&, Map2fID> alloc(const Vec2f& pos) {
			auto [item, id] = Pool<Map2fItem<T>,Capacity>::alloc();
			item.pos = pos;

			Array* array;
			{
				ArrayOrQuads* node = find_node(pos);
				Array* prov_array = std::get<Array*>(*node);
				if (LinearN <= prov_array->size()) {
					// Check if all items are in the same position. If so, don't attempt to divide
					auto is_crowded_predicate = [=](const ArrayItem &array_item) -> bool {
						return array_item.pos.is_near(prov_array->front().pos, 0.001f);
					};
					auto is_crowded = std::all_of(prov_array->begin(), prov_array->end(), is_crowded_predicate);
					if (is_crowded) {
						array = prov_array;
					} else {
						// Divide into quads, find middle point
						Vec2f accumulation;
						std::for_each(prov_array->begin(), prov_array->end(), [&accumulation](const ArrayItem& array_item) {
							accumulation += array_item.pos;
						});
						auto mid_point = accumulation / (float) prov_array->size();
						auto* quads = new Quads(mid_point, *prov_array);
						*node = quads;
						array = std::get<Array*>(quads->quad[quads->find_quad_index(pos)]);
					}
				} else {
					array = prov_array;
				}
			}
			array->emplace_back(ArrayItem{pos, id});

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
			std::vector<Map2fID> items;

			std::queue<const ArrayOrQuads*> nodes_to_check = {&_root};
			while (!nodes_to_check.empty()) {
				const ArrayOrQuads* node = nodes_to_check.front();
				nodes_to_check.pop();
				if (std::holds_alternative<Array*>(*node)) {
					const Array* array = std::get<Array*>(*node);
					std::copy_if(array->begin(), array->end(), std::back_inserter(items), [=](const ArrayItem& item) {
						return pos.is_near(item.pos, radius);
					});
				} else {
					const Quads* quads = std::get<Quads*>(*node);
					quads->append_intersecting_quads(pos, radius, nodes_to_check);
				}
			}

			return items;
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

		// TODO add move function, which can move if they are in the same array, or erase->insert
	};
}

#endif //M2_MAP2F_H
