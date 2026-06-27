#pragma once

#include "detail/MapF.h"

namespace m2 {
	/// MapF is a container that can store objects that occupy rectangular areas on a map. The map can store objects
	/// anywhere, but only a predetermined area is "managed", while objects laying outside the managed area are
	/// accumulated in foreign items list. For the managed area, a quad-tree is utilized to provide quick access to the
	/// objects. The map stores the objects in a Pool, whereas the quad-trees contain references to the objects.
	template <typename T, uint64_t Capacity, float ComparisonTolerance>
	class MapF : Pool<detail::MapPoolItem<T>,Capacity> {
		detail::Quadrant<T,Capacity,ComparisonTolerance> _rootQuadrant; /// The main plot of land managed by the Map
		std::deque<detail::QuadrantItem> _foreignItems; /// Items that at least partially lay outside the main plot

	public:
		explicit MapF(const RectF& area) : Pool<detail::MapPoolItem<T>,Capacity>(), _rootQuadrant(*this, area) {}
		MapF(const float x, const float y, const float w, const float h) : MapF(RectF{x, y, w, h}) {}

		// Accessors

		using Pool<detail::MapPoolItem<T>,Capacity>::Size;
		using Pool<detail::MapPoolItem<T>,Capacity>::Contains;
		const T* TryGetObject(Id id) const;
		const T& UnsafeGetObject(Id id) const;
		[[nodiscard]] const RectF& GetArea() const { return _rootQuadrant.GetArea(); }
		/// Iterates over all objects in the container
		template <typename Op>
		int ForEach(Op op) const;
		/// Iterates over all objects in the container that is completely encapsulated inside the given area
		template <typename Op>
		int ForEachEncapsulated(const RectF& area, Op op) const;
		/// Iterates over all objects in the container that intersects with the given area
		template <typename Op>
		int ForEachIntersecting(const RectF& area, Op op) const;

		// Modifiers

		T* TryGetObject(Id id);
		T& UnsafeGetObject(Id id);
		template <typename... Args>
		std::pair<T&, Id> Emplace(const RectF& area, Args&&... args);
		void Erase(Id id);
		void Move(Id id, const RectF& new_area);
		void Clear() override;
		/// Iterates over all objects in the container
		template <typename Op>
		int ForEach(Op op);
		/// Iterates over all objects in the container that is completely encapsulated inside the given area
		template <typename Op>
		int ForEachEncapsulated(const RectF& area, Op op);
		/// Iterates over all objects in the container that intersects with the given area
		template <typename Op>
		int ForEachIntersecting(const RectF& area, Op op);

	private:
		void AdjustContainer(Id id, std::deque<detail::QuadrantItem>* container);

		friend class detail::Quadrant<T,Capacity,ComparisonTolerance>;
	};
}

// Implementations

template <typename T, uint64_t Capacity, float ComparisonTolerance>
const T* m2::MapF<T,Capacity,ComparisonTolerance>::TryGetObject(const Id id) const {
	if (const auto* item = Pool<detail::MapPoolItem<T>,Capacity>::Get(id)) { return &item->t; }
	return nullptr;
}
template <typename T, uint64_t Capacity, float ComparisonTolerance>
const T& m2::MapF<T,Capacity,ComparisonTolerance>::UnsafeGetObject(const Id id) const {
	if (const auto* t = TryGetObject(id)) { return *t; }
	throw M2_ERROR("Out of bounds");
}
template <typename T, uint64_t Capacity, float ComparisonTolerance>
template <typename Op>
int m2::MapF<T,Capacity,ComparisonTolerance>::ForEach(Op op) const {
	int count = 0;
	for (auto it = Pool<detail::MapPoolItem<T>,Capacity>::begin(); it != Pool<detail::MapPoolItem<T>,Capacity>::end(); ++it) {
		if (op(it->area, it.GetId(), it->t)) { ++count; } else { return count; }
	}
	return count;
}
template <typename T, uint64_t Capacity, float ComparisonTolerance>
template <typename Op>
int m2::MapF<T,Capacity,ComparisonTolerance>::ForEachEncapsulated(const RectF& area, Op op) const {
	int count = 0;
	// Iterate over foreign items
	for (const auto& item : _foreignItems) {
		if (area.DoesContain(item.area, ComparisonTolerance)) {
			if (op(item.area, item.id, *TryGetObject(item.id))) { ++count; } else { return count; }
		}
	}
	return count + _rootQuadrant.ForEachEncapsulated(area, op); // Search in quadrants
}
template <typename T, uint64_t Capacity, float ComparisonTolerance>
template <typename Op>
int m2::MapF<T,Capacity,ComparisonTolerance>::ForEachIntersecting(const RectF& area, Op op) const {
	int count = 0;
	// Iterate over foreign items
	for (const auto& item : _foreignItems) {
		if (area.GetIntersection(item.area, ComparisonTolerance)) {
			if (op(item.area, item.id, *TryGetObject(item.id))) { ++count; } else { return count; }
		}
	}
	return count + _rootQuadrant.ForEachIntersecting(area, op); // Search in quadrants
}

template <typename T, uint64_t Capacity, float ComparisonTolerance>
T* m2::MapF<T,Capacity,ComparisonTolerance>::TryGetObject(const Id id) {
	if (auto* item = Pool<detail::MapPoolItem<T>,Capacity>::Get(id)) { return &item->t; }
	return nullptr;
}
template <typename T, uint64_t Capacity, float ComparisonTolerance>
T& m2::MapF<T,Capacity,ComparisonTolerance>::UnsafeGetObject(const Id id) {
	if (auto* t = TryGetObject(id)) { return *t; }
	throw M2_ERROR("Out of bounds");
}
template <typename T, uint64_t Capacity, float ComparisonTolerance>
template <typename... Args>
std::pair<T&, m2::Id> m2::MapF<T,Capacity,ComparisonTolerance>::Emplace(const RectF& area, Args&&... args) {
	// Insert item into Pool
	auto pool_it = Pool<detail::MapPoolItem<T>,Capacity>::Emplace(std::forward<Args>(args)...);
	// Store area
	pool_it->area = area;
	// Point back-pointer to the correct container
	if (_rootQuadrant.GetArea().DoesContain(area, ComparisonTolerance)) {
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
void m2::MapF<T,Capacity,ComparisonTolerance>::Erase(const Id id) {
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
void m2::MapF<T,Capacity,ComparisonTolerance>::Move(const Id id, const RectF& new_area) {
	auto* pool_item = Pool<detail::MapPoolItem<T>,Capacity>::Get(id);
	pool_item->area = new_area;
	// Erase from container
	auto* container = pool_item->container;
	std::remove_if(container->begin(), container->end(), [=](const auto& it) { return it.id == id; });
	// Reinsert
	if (_rootQuadrant.GetArea().DoesContain(new_area, ComparisonTolerance)) {
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
void m2::MapF<T,Capacity,ComparisonTolerance>::AdjustContainer(const Id id, std::deque<detail::QuadrantItem>* container) {
	Pool<detail::MapPoolItem<T>,Capacity>::Get(id)->container = container;
}
template <typename T, uint64_t Capacity, float ComparisonTolerance>
template <typename Op>
int m2::MapF<T,Capacity,ComparisonTolerance>::ForEach(Op op) {
	int count = 0;
	for (auto it = Pool<detail::MapPoolItem<T>,Capacity>::begin(); it != Pool<detail::MapPoolItem<T>,Capacity>::end(); ++it) {
		if (op(it->area, it.GetId(), it->t)) { ++count; } else { return count; }
	}
	return count;
}
template <typename T, uint64_t Capacity, float ComparisonTolerance>
template <typename Op>
int m2::MapF<T,Capacity,ComparisonTolerance>::ForEachEncapsulated(const RectF& area, Op op) {
	int count = 0;
	// Iterate over foreign items
	for (auto& item : _foreignItems) {
		if (area.DoesContain(item.area, ComparisonTolerance)) {
			if (op(item.area, item.id, *TryGetObject(item.id))) { ++count; } else { return count; }
		}
	}
	return count + _rootQuadrant.ForEachEncapsulated(area, op); // Search in quadrants
}
template <typename T, uint64_t Capacity, float ComparisonTolerance>
template <typename Op>
int m2::MapF<T,Capacity,ComparisonTolerance>::ForEachIntersecting(const RectF& area, Op op) {
	int count = 0;
	// Iterate over foreign items
	for (auto& item : _foreignItems) {
		if (area.GetIntersection(item.area, ComparisonTolerance)) {
			if (op(item.area, item.id, *TryGetObject(item.id))) { ++count; } else { return count; }
		}
	}
	return count + _rootQuadrant.ForEachIntersecting(area, op); // Search in quadrants
}
