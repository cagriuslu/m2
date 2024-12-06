#pragma once

#include <m2/math/RectF.h>
#include <m2/containers/Pool.h>
#include <deque>
#include <memory>
#include <cstdint>

namespace m2 {
	// Forward declaration
	template <typename T, uint64_t Capacity = 65536> class MapF;

	namespace detail {
		// The struct that's held inside Quadrants and _foreign_items
		struct MapContainerItemF {
			// Copy area here for quick access
			RectF area;
			// ID of the object inside the Pool (MapF)
			Id id{};

			MapContainerItemF(RectF&& area, Id id) : area(area), id(id) {}
			MapContainerItemF(const RectF& area, Id id) : area(area), id(id) {}
		};

		template <typename T, uint64_t Capacity>
		class Quadrant {
			// Most common cache line size is 64 bytes.
			// Thus, each quadrant can contain multiples of 8 IDs.
			// The quadrant shouldn't be divided unless there are ITEM_COUNT_THRESHOLD items.
			static constexpr unsigned ITEM_COUNT_THRESHOLD = 64 / sizeof(m2::Id);

			MapF<T,Capacity>& _map; // Back-pointer to the map, mostly used to look up the object from the Pool
			RectF _area; // The plot of land managed by this Quadrant
			float _tolerance;
			std::deque<MapContainerItemF> _items{}; // Items that lay on multiple quadrants, or all items until the child quadrants are created
			bool _dirty{}; // Signifies that _items contain an item that could wholly fit into a child, but the children are not created yet.
			std::array<std::pair<RectF, std::unique_ptr<Quadrant>>, 4> _children{}; // Children Quadrants. Order: TL, TR, BL, BR

		public:
			Quadrant(MapF<T,Capacity>& map, const RectF& area, float tolerance) : _map(map), _area(area), _tolerance(tolerance) {
				auto half_w = area.w / 2.0f;
				auto half_h = area.h / 2.0f;
				_children[0].first = RectF{area.x, area.y, half_w, half_h};
				_children[1].first = RectF{area.x + half_w, area.y, half_w, half_h};
				_children[2].first = RectF{area.x, area.y + half_h, half_w, half_h};
				_children[3].first = RectF{area.x + half_w, area.y + half_h, half_w, half_h};
			}
			// Accessors
			[[nodiscard]] const RectF& area() const { return _area; }
			// Iterators
			uint64_t for_each_containing(const RectF& area, const std::function<bool(const RectF&,Id,T&)>& op, float tolerance) {
				uint64_t count = 0;
				// Search in items
				for (auto& item : _items) {
					if (area.contains(item.area, tolerance)) {
						if (op(item.area, item.id, *_map.get(item.id))) {
							++count;
						} else {
							return count;
						}
					}
				}
				// Search in quadrants
				for (auto& [child_area, child] : _children) {
					if (child && area.intersect(child_area, tolerance)) {
						count += child->for_each_containing(area, op, tolerance);
					}
				}
				return count;
			}
			uint64_t for_each_intersecting(const RectF& area, const std::function<bool(const RectF&,Id,T&)>& op, float tolerance) {
				uint64_t count = 0;
				// Search in items
				for (auto& item : _items) {
					if (area.intersect(item.area, tolerance)) {
						if (op(item.area, item.id, *_map.get(item.id))) {
							++count;
						} else {
							return count;
						}
					}
				}
				// Search in quadrants
				for (auto& [child_area, child] : _children) {
					if (child && area.intersect(child_area, tolerance)) {
						count += child->for_each_containing(area, op, tolerance);
					}
				}
				return count;
			}
			// Modifiers
			std::deque<MapContainerItemF>* insert(MapContainerItemF&& item) {
				// Check if the item can be wholly contained in a children, and the children is created
				for (size_t i = 0; i < _children.size(); ++i) {
					if (auto& [child_area, child] = _children[i]; child_area.contains(item.area, _tolerance)) {
						// Item can be wholly contained in the child
						if (child) {
							// Child is already created
							return child->insert(std::move(item));
						} else if (_items.size() < ITEM_COUNT_THRESHOLD) {
							// Child is not created, but there isn't enough items at this level to create it yet
							_dirty = true;
							_items.emplace_back(std::move(item));
							return &_items;
						} else {
							// Child is not created, and this level has too many items already
							// Distribute the items on this level to the children
							distribute();
							// Check the child. It may or may not have been created during the distribution process
							if (not _children[i].second) {
								_children[i].second = std::make_unique<Quadrant>(_map, child_area, _tolerance);
							}
							return _children[i].second->insert(std::move(item));
						}
					}
				}
				// Item does not wholly fit into any children, store it at this level
				_items.emplace_back(std::move(item));
				return &_items;
			}
			void clear() {
				_items.clear();
				for (auto& child : _children) {
					child.second.reset();
				}
			}
		private:
			// Distribute the items at this level to children
			void distribute() {
				if (not _dirty) {
					return; // Nothing to distribute
				}
				for (auto item_it = _items.begin(); item_it != _items.end(); ) {
					bool child_can_contain = false;
					for (size_t i = 0; i < _children.size(); ++i) {
						if (auto& [child_area, child] = _children[i]; child_area.contains(item_it->area, _tolerance)) {
							// Item can be wholly contained in the child
							child_can_contain = true;
							auto id = item_it->id;
							if (child) {
								// Child is already created, adjust container to new position
								_map.adjust_container(id, child->insert(std::move(*item_it)));
							} else {
								// Child needs to be created
								_children[i].second = std::make_unique<Quadrant>(_map, child_area, _tolerance);
								// Adjust container to new position
								_map.adjust_container(id, _children[i].second->insert(std::move(*item_it)));
							}
							item_it = _items.erase(item_it);
							break;
						}
					}
					if (not child_can_contain) {
						++item_it; // Item does not wholly fit into any children
					}
				}
				_dirty = false; // No longer dirty
			}

			friend class MapF<T,Capacity>;
		};

		// The stored object is wrapped inside MapPoolItemF before being placed in the Pool
		template <typename T>
		struct MapPoolItemF {
			// Back-pointer to either MapF::_foreign_items, or Quadrant::_items. If the object lays outside the area
			// managed by the Map, this points to MapF::_foreign_items. Otherwise, it points to some Quadrant::_items in
			// some Quadrant.
			std::deque<MapContainerItemF>* container{};
			// Area taken up by the object
			RectF area;
			// Object itself
			T t;

			// Constructor only constructs the object itself.
			template <typename... Args>
			explicit MapPoolItemF(Args&&... args) : t(std::forward<Args>(args)...) {}
		};
	}

	// MapF extends Pool privately. The items are stored in the inherited Pool.
	template <typename T, uint64_t Capacity>
	class MapF : private Pool<detail::MapPoolItemF<T>,Capacity> {
		float _tolerance;
		detail::Quadrant<T,Capacity> _root_quadrant; /// The main plot of land managed by the Map
		std::deque<detail::MapContainerItemF> _foreign_items; /// Items that at least partially lay outside the main plot

	public:
		explicit MapF(const RectF& area, float tolerance = 0.001f) : Pool<detail::MapPoolItemF<T>,Capacity>(), _tolerance(tolerance), _root_quadrant(*this, area, _tolerance) {}
		MapF(float x, float y, float w, float h) : MapF(RectF{x, y, w, h}) {}

		//<editor-fold desc="Accessors">
		using Pool<detail::MapPoolItemF<T>,Capacity>::size;
		using Pool<detail::MapPoolItemF<T>,Capacity>::contains;
		T* get(Id id) {
			if (auto* item = Pool<detail::MapPoolItemF<T>,Capacity>::get(id)) {
				return &item->t;
			}
			return nullptr;
		}
		const T* get(Id id) const {
			if (const auto* item = Pool<detail::MapPoolItemF<T>,Capacity>::get(id)) {
				return &item->t;
			}
			return nullptr;
		}
		T& operator[](Id id) {
			if (auto* t = get(id)) {
				return *t;
			}
			throw M2_ERROR("Out of bounds");
		}
		const T& operator[](Id id) const {
			if (const auto* t = get(id)) {
				return *t;
			}
			throw M2_ERROR("Out of bounds");
		}
		[[nodiscard]] const RectF& area() const { return _root_quadrant.area(); }
		//</editor-fold>

		//<editor-fold desc="Iterators">
		// Iterate over all objects in the container. The iteration continues as long as `op` returns true.
		// Returns the number items iterated for which op returned true.
		uint64_t for_each(const std::function<bool(const RectF&,Id,T&)>& op) {
			uint64_t count = 0;
			for (auto it = Pool<detail::MapPoolItemF<T>,Capacity>::begin(); it != Pool<detail::MapPoolItemF<T>,Capacity>::end(); ++it) {
				if (op(it->area, it.id(), it->t)) {
					++count;
				} else {
					return count;
				}
			}
			return count;
		}
		uint64_t for_each_containing(const RectF& area, const std::function<bool(const RectF&,Id,T&)>& op, float tolerance = 0.001f) {
			uint64_t count = 0;
			// Iterate over foreign items
			for (auto& item : _foreign_items) {
				if (area.contains(item.area, tolerance)) {
					if (op(item.area, item.id, *get(item.id))) {
						++count;
					} else {
						return count;
					}
				}
			}
			return count + _root_quadrant.for_each_containing(area, op, tolerance); // Search in quadrants
		}
		uint64_t for_each_intersecting(const RectF& area, const std::function<bool(const RectF&,Id,T&)>& op, float tolerance = 0.001f) {
			uint64_t count = 0;
			// Iterate over foreign items
			for (auto& item : _foreign_items) {
				if (area.intersect(item.area, tolerance)) {
					if (op(item.area, item.id, *get(item.id))) {
						++count;
					} else {
						return count;
					}
				}
			}
			return count + _root_quadrant.for_each_intersecting(area, op, tolerance); // Search in quadrants
		}
		//</editor-fold>

		//<editor-fold desc="Modifiers">
		template <typename... Args>
		std::pair<T&, Id> emplace(const RectF& area, Args&&... args) {
			// Insert item into Pool
			auto pool_it = Pool<detail::MapPoolItemF<T>,Capacity>::emplace(std::forward<Args>(args)...);
			// Store area
			pool_it->area = area;
			// Point back-pointer to the correct container
			if (_root_quadrant.area().contains(area, _tolerance)) {
				// Object fits inside the managed area, insert to one of the quadrants
				pool_it->container = _root_quadrant.insert({area, pool_it.id()});
			} else {
				// Object doesn't fit inside the managed area, insert to _foreign_items
				_foreign_items.emplace_back(area, pool_it.id());
				pool_it->container = &_foreign_items;
			}
			return {pool_it->t, pool_it.id()};
		}
		void erase(Id id) {
			// Erase from container
			auto* container = Pool<detail::MapPoolItemF<T>,Capacity>::get(id)->container;
			for (auto it = container->begin(); it != container->end(); ++it) {
				if (it->id == id) {
					container->erase(it);
					Pool<detail::MapPoolItemF<T>,Capacity>::free(id);
					return;
				}
			}
			throw M2_ERROR("Attempt to erase item from wrong quadrant");
		}
		void move(Id id, const RectF& new_area) {
			auto* pool_item = Pool<detail::MapPoolItemF<T>,Capacity>::get(id);
			pool_item->area = new_area;
			// Erase from container
			auto* container = pool_item->container;
			for (auto it = container->begin(); it != container->end(); ++it) {
				if (it->id == id) {
					container->erase(it); // TODO erase_if
					break;
				}
			}
			// Reinsert
			if (_root_quadrant.area().contains(new_area, _tolerance)) {
				pool_item->container = _root_quadrant.insert({new_area, id});
			} else {
				_foreign_items.emplace_back(new_area, id);
				pool_item->container = &_foreign_items;
			}
		}
		void clear() {
			Pool<detail::MapPoolItemF<T>,Capacity>::clear();
			_root_quadrant.clear();
			_foreign_items.clear();
		}
		//</editor-fold>

	private:
		void adjust_container(Id id, std::deque<detail::MapContainerItemF>* container) {
			Pool<detail::MapPoolItemF<T>,Capacity>::get(id)->container = container;
		}

		friend class detail::Quadrant<T,Capacity>;
	};
}
