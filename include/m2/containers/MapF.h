#pragma once

#include <m2/math/RectF.h>
#include <m2/containers/Pool.h>
#include <deque>
#include <memory>
#include <cstdint>

namespace m2 {
	// Forward declaration
	template <typename T, uint64_t Capacity = 65536, float ComparisonTolerance = 0.001f> class MapF;

	namespace detail {
		using ObjectAreaAndId = std::pair<RectF,Id>;

		template <typename T, uint64_t Capacity, float ComparisonTolerance>
		class Quadrant {
			// Most common cache line size is 64 bytes.
			// Thus, each quadrant can contain multiples of 8 IDs.
			// The quadrant shouldn't be divided unless there are ITEM_COUNT_THRESHOLD items.
			static constexpr unsigned ITEM_COUNT_THRESHOLD = 64 / sizeof(Id);

			MapF<T,Capacity>& _map; // Back-pointer to the map, mostly used to look up the objects from the Pool
			RectF _area; // The plot of land managed by this Quadrant
			std::deque<ObjectAreaAndId> _items{}; // Items that lay on multiple quadrants, or all items until the child quadrants are created
			bool _dirty{}; // Signifies that _items contain an item that could wholly fit into a child, but the children are not created yet.
			std::array<std::pair<RectF, std::unique_ptr<Quadrant>>, 4> _children{}; // Children Quadrants. Order: TL, TR, BL, BR

		public:
			// Constructors

			Quadrant(MapF<T,Capacity,ComparisonTolerance>& map, const RectF& area);

			// Accessors

			[[nodiscard]] const RectF& Area() const { return _area; }
			uint64_t ForEachContaining(const RectF& area, const std::function<bool(const RectF&,Id,T&)>& op) const;
			uint64_t ForEachIntersecting(const RectF& area, const std::function<bool(const RectF&,Id,T&)>& op) const;

			// Modifiers

			std::deque<ObjectAreaAndId>* Insert(ObjectAreaAndId&& item);
			void Clear();

		private:
			// Distribute the items at this level to children
			void Distribute();

			friend class MapF<T,Capacity,ComparisonTolerance>;
		};

		// The stored object is wrapped inside MapPoolItem before being placed in the Pool
		template <typename T>
		struct MapPoolItem {
			// Back-pointer to either MapF::_foreign_items, or Quadrant::_items. If the object lays outside the area
			// managed by the Map, this points to MapF::_foreign_items. Otherwise, it points to some Quadrant::_items in
			// some Quadrant.
			std::deque<ObjectAreaAndId>* container{};
			RectF area; // Area taken up by the object
			T t; // The object itself

			// Constructor only constructs the object itself.
			template <typename... Args>
			explicit MapPoolItem(Args&&... args) : t(std::forward<Args>(args)...) {}
		};
	}

	/// MapF is a container that can store objects that occupy rectangular areas on a map. The map can store objects
	/// anywhere, but only a predetermined area is "managed", while objects laying outside the managed area are
	/// accumulated in foreign items list. For the managed area, a quad-tree is utilized to provide quick access to the
	/// objects. The map stores the objects in a Pool, whereas the quad-trees contain references to the objects.
	template <typename T, uint64_t Capacity, float ComparisonTolerance>
	class MapF : Pool<detail::MapPoolItem<T>,Capacity> {
		detail::Quadrant<T,Capacity,ComparisonTolerance> _rootQuadrant; /// The main plot of land managed by the Map
		std::deque<detail::ObjectAreaAndId> _foreignItems; /// Items that at least partially lay outside the main plot

	public:
		explicit MapF(const RectF& area) : Pool<detail::MapPoolItem<T>,Capacity>(), _rootQuadrant(*this, area) {}
		MapF(const float x, const float y, const float w, const float h) : MapF(RectF{x, y, w, h}) {}

		using Pool<detail::MapPoolItem<T>,Capacity>::Size;
		using Pool<detail::MapPoolItem<T>,Capacity>::Contains;
		T* Get(Id id);
		const T* Get(Id id) const;
		T& operator[](Id id);
		const T& operator[](Id id) const;
		[[nodiscard]] const RectF& Area() const { return _rootQuadrant.Area(); }

		// Iterate over all objects in the container. The iteration continues as long as `op` returns true.
		// Returns the number items iterated for which op returned true.
		uint64_t ForEach(const std::function<bool(const RectF&,Id,T&)>& op);
		uint64_t ForEachContaining(const RectF& area, const std::function<bool(const RectF&,Id,T&)>& op);
		uint64_t ForEachIntersecting(const RectF& area, const std::function<bool(const RectF&,Id,T&)>& op);

		template <typename... Args>
		std::pair<T&, Id> Emplace(const RectF& area, Args&&... args);
		void Erase(Id id);
		void Move(Id id, const RectF& new_area);
		void Clear();

	private:
		void AdjustContainer(Id id, std::deque<detail::ObjectAreaAndId>* container);

		friend class detail::Quadrant<T,Capacity,ComparisonTolerance>;
	};
}

// Implementations

template <typename T, uint64_t Capacity, float ComparisonTolerance>
m2::detail::Quadrant<T,Capacity,ComparisonTolerance>::Quadrant(MapF<T,Capacity,ComparisonTolerance>& map, const RectF& area) : _map(map), _area(area) {
	const auto half_w = area.w / 2.0f;
	const auto half_h = area.h / 2.0f;
	_children[0].first = RectF{area.x, area.y, half_w, half_h};
	_children[1].first = RectF{area.x + half_w, area.y, half_w, half_h};
	_children[2].first = RectF{area.x, area.y + half_h, half_w, half_h};
	_children[3].first = RectF{area.x + half_w, area.y + half_h, half_w, half_h};
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
uint64_t m2::detail::Quadrant<T,Capacity,ComparisonTolerance>::ForEachContaining(const RectF& area, const std::function<bool(const RectF&,Id,T&)>& op) const {
	uint64_t count = 0;
	// Search in items
	for (auto& item : _items) {
		if (area.DoesContain(std::get<RectF>(item), ComparisonTolerance)) {
			if (op(std::get<RectF>(item), std::get<Id>(item), *_map.Get(std::get<Id>(item)))) {
				++count;
			} else {
				return count;
			}
		}
	}
	// Search in quadrants
	for (auto& [child_area, child] : _children) {
		if (child && area.GetIntersection(child_area, ComparisonTolerance)) {
			count += child->ForEachContaining(area, op, ComparisonTolerance);
		}
	}
	return count;
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
uint64_t m2::detail::Quadrant<T,Capacity,ComparisonTolerance>::ForEachIntersecting(const RectF& area, const std::function<bool(const RectF&,Id,T&)>& op) const {
	uint64_t count = 0;
	// Search in items
	for (auto& item : _items) {
		if (area.GetIntersection(std::get<RectF>(item), ComparisonTolerance)) {
			if (op(std::get<RectF>(item), std::get<Id>(item), *_map.Get(std::get<Id>(item)))) {
				++count;
			} else {
				return count;
			}
		}
	}
	// Search in quadrants
	for (auto& [child_area, child] : _children) {
		if (child && area.GetIntersection(child_area, ComparisonTolerance)) {
			count += child->ForEachContaining(area, op, ComparisonTolerance);
		}
	}
	return count;
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
std::deque<m2::detail::ObjectAreaAndId>* m2::detail::Quadrant<T,Capacity,ComparisonTolerance>::Insert(ObjectAreaAndId&& item) {
	// Check if the item can be wholly contained in a children, and the children is created
	for (size_t i = 0; i < _children.size(); ++i) {
		if (auto& [child_area, child] = _children[i]; child_area.DoesContain(std::get<RectF>(item), ComparisonTolerance)) {
			// Item can be wholly contained in the child
			if (child) {
				// Child is already created
				return child->Insert(std::move(item));
			} else if (_items.size() < ITEM_COUNT_THRESHOLD) {
				// Child is not created, but there isn't enough items at this level to create it yet
				_dirty = true;
				_items.emplace_back(item);
				return &_items;
			} else {
				// Child is not created, and this level has too many items already
				// Distribute the items on this level to the children
				Distribute();
				// Check the child. It may or may not have been created during the distribution process
				if (not _children[i].second) {
					_children[i].second = std::make_unique<Quadrant>(_map, child_area);
				}
				return _children[i].second->Insert(std::move(item));
			}
		}
	}
	// Item does not wholly fit into any children, store it at this level
	_items.emplace_back(item);
	return &_items;
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
void m2::detail::Quadrant<T,Capacity,ComparisonTolerance>::Clear() {
	_items.clear();
	for (auto& child : _children) {
		child.second.reset();
	}
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
void m2::detail::Quadrant<T,Capacity,ComparisonTolerance>::Distribute() {
	if (not _dirty) {
		return; // Nothing to distribute
	}
	for (auto item_it = _items.begin(); item_it != _items.end(); ) {
		bool child_can_contain = false;
		for (size_t i = 0; i < _children.size(); ++i) {
			if (auto& [child_area, child] = _children[i]; child_area.DoesContain(std::get<RectF>(*item_it), ComparisonTolerance)) {
				// Item can be wholly contained in the child
				child_can_contain = true;
				if (child) {
					// Child is already created, adjust container to new position
					_map.AdjustContainer(std::get<Id>(*item_it), child->Insert(std::move(*item_it)));
				} else {
					// Child needs to be created
					_children[i].second = std::make_unique<Quadrant>(_map, child_area);
					// Adjust container to new position
					_map.AdjustContainer(std::get<Id>(*item_it), _children[i].second->Insert(std::move(*item_it)));
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

template <typename T, uint64_t Capacity, float ComparisonTolerance>
T* m2::MapF<T,Capacity,ComparisonTolerance>::Get(Id id) {
	if (auto* item = Pool<detail::MapPoolItem<T>,Capacity>::Get(id)) {
		return &item->t;
	}
	return nullptr;
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
const T* m2::MapF<T,Capacity,ComparisonTolerance>::Get(Id id) const {
	if (const auto* item = Pool<detail::MapPoolItem<T>,Capacity>::Get(id)) {
		return &item->t;
	}
	return nullptr;
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
T& m2::MapF<T,Capacity,ComparisonTolerance>::operator[](Id id) {
	if (auto* t = Get(id)) {
		return *t;
	}
	throw M2_ERROR("Out of bounds");
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
const T& m2::MapF<T,Capacity,ComparisonTolerance>::operator[](Id id) const {
	if (const auto* t = Get(id)) {
		return *t;
	}
	throw M2_ERROR("Out of bounds");
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
uint64_t m2::MapF<T,Capacity,ComparisonTolerance>::ForEach(const std::function<bool(const RectF&,Id,T&)>& op) {
	uint64_t count = 0;
	for (auto it = Pool<detail::MapPoolItem<T>,Capacity>::begin(); it != Pool<detail::MapPoolItem<T>,Capacity>::end(); ++it) {
		if (op(it->area, it.GetId(), it->t)) {
			++count;
		} else {
			return count;
		}
	}
	return count;
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
uint64_t m2::MapF<T,Capacity,ComparisonTolerance>::ForEachContaining(const RectF& area, const std::function<bool(const RectF&,Id,T&)>& op) {
	uint64_t count = 0;
	// Iterate over foreign items
	for (auto& item : _foreignItems) {
		if (area.DoesContain(std::get<RectF>(item), ComparisonTolerance)) {
			if (op(std::get<RectF>(item), std::get<Id>(item), *Get(std::get<Id>(item)))) {
				++count;
			} else {
				return count;
			}
		}
	}
	return count + _rootQuadrant.ForEachContaining(area, op, ComparisonTolerance); // Search in quadrants
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
uint64_t m2::MapF<T,Capacity,ComparisonTolerance>::ForEachIntersecting(const RectF& area, const std::function<bool(const RectF&,Id,T&)>& op) {
	uint64_t count = 0;
	// Iterate over foreign items
	for (auto& item : _foreignItems) {
		if (area.GetIntersection(std::get<RectF>(item), ComparisonTolerance)) {
			if (op(std::get<RectF>(item), std::get<Id>(item), *Get(std::get<Id>(item)))) {
				++count;
			} else {
				return count;
			}
		}
	}
	return count + _rootQuadrant.ForEachIntersecting(area, op, ComparisonTolerance); // Search in quadrants
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
template <typename... Args>
std::pair<T&, m2::Id> m2::MapF<T,Capacity,ComparisonTolerance>::Emplace(const RectF& area, Args&&... args) {
	// Insert item into Pool
	auto pool_it = Pool<detail::MapPoolItem<T>,Capacity>::Emplace(std::forward<Args>(args)...);
	// Store area
	pool_it->area = area;
	// Point back-pointer to the correct container
	if (_rootQuadrant.Area().DoesContain(area, ComparisonTolerance)) {
		// Object fits inside the managed area, insert to one of the quadrants
		pool_it->container = _rootQuadrant.Insert({area, pool_it.GetId()});
	} else {
		// Object doesn't fit inside the managed area, insert to _foreign_items
		_foreignItems.emplace_back(area, pool_it.GetId());
		pool_it->container = &_foreignItems;
	}
	return {pool_it->t, pool_it.GetId()};
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
void m2::MapF<T,Capacity,ComparisonTolerance>::Erase(Id id) {
	// Erase from container
	auto* container = Pool<detail::MapPoolItem<T>,Capacity>::Get(id)->container;
	for (auto it = container->begin(); it != container->end(); ++it) {
		if (it->id == id) {
			container->erase(it);
			Pool<detail::MapPoolItem<T>,Capacity>::Free(id);
			return;
		}
	}
	throw M2_ERROR("Attempt to erase item from wrong quadrant");
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
void m2::MapF<T,Capacity,ComparisonTolerance>::Move(Id id, const RectF& new_area) {
	auto* pool_item = Pool<detail::MapPoolItem<T>,Capacity>::Get(id);
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
	if (_rootQuadrant.Area().DoesContain(new_area, ComparisonTolerance)) {
		pool_item->container = _rootQuadrant.Insert({new_area, id});
	} else {
		_foreignItems.emplace_back(new_area, id);
		pool_item->container = &_foreignItems;
	}
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
void m2::MapF<T,Capacity,ComparisonTolerance>::Clear() {
	Pool<detail::MapPoolItem<T>,Capacity>::Clear();
	_rootQuadrant.Clear();
	_foreignItems.clear();
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
void m2::MapF<T,Capacity,ComparisonTolerance>::AdjustContainer(Id id, std::deque<detail::ObjectAreaAndId>* container) {
	Pool<detail::MapPoolItem<T>,Capacity>::Get(id)->container = container;
}
