#pragma once
#include <m2/containers/Pool.h>
#include <m2/math/RectF.h>
#include <deque>
#include <memory>

namespace m2 {
	// Forward declaration
	template <typename T, uint64_t Capacity = 65536, float ComparisonTolerance = 0.001f> class MapF;
}

namespace m2::detail {
	struct QuadrantItem {
		RectF area;
		Id id;
	};

	template <typename T, uint64_t Capacity, float ComparisonTolerance>
	class Quadrant {
		/// Most common cache line size
		static constexpr unsigned ITEM_COUNT_THRESHOLD = 64 / sizeof(Id);
		/// Back-pointer to the map
		MapF<T,Capacity>& _map;
		/// Area managed by this Quadrant
		const RectF _area;
		/// For items that lay on multiple quadrants, or all items until the child quadrants are created.
		std::deque<QuadrantItem> _items{};
		/// Order: TL, TR, BL, BR
		std::array<std::pair<RectF, std::unique_ptr<Quadrant>>, 4> _children{};

	public:
		// Constructors

		Quadrant(MapF<T,Capacity,ComparisonTolerance>& map, const RectF& area) : _map(map), _area(area) {
			const auto half_w = area.w / 2.0f;
			const auto half_h = area.h / 2.0f;
			_children[0].first = RectF{area.x, area.y, half_w, half_h};
			_children[1].first = RectF{area.x + half_w, area.y, half_w, half_h};
			_children[2].first = RectF{area.x, area.y + half_h, half_w, half_h};
			_children[3].first = RectF{area.x + half_w, area.y + half_h, half_w, half_h};
		}

		// Accessors

		[[nodiscard]] const RectF& GetArea() const { return _area; }

		template <typename Op>
		int ForEachEncapsulated(const RectF& area, Op op) const {
			int count = 0;
			// First, search in items
			for (auto& item : _items) {
				if (area.DoesContain(item.area, ComparisonTolerance)) {
					if (op(item.area, item.id, *_map.TryGetObject(item.id))) {
						++count;
					} else {
						return count;
					}
				}
			}
			// Then, search in quadrants
			for (auto& [childArea, child] : _children) {
				if (child && area.GetIntersection(childArea, ComparisonTolerance)) {
					count += child->ForEachEncapsulated(area, op);
				}
			}
			return count;
		}

		template <typename Op>
		int ForEachIntersecting(const RectF& area, Op op) const {
			int count = 0;
			// First, search in items
			for (auto& item : _items) {
				if (area.GetIntersection(item.area, ComparisonTolerance)) {
					if (op(item.area, item.id, *_map.TryGetObject(item.id))) {
						++count;
					} else {
						return count;
					}
				}
			}
			// Then, search in quadrants
			for (auto& [childArea, child] : _children) {
				if (child && area.GetIntersection(childArea, ComparisonTolerance)) {
					count += child->ForEachIntersecting(area, op);
				}
			}
			return count;
		}

		// Modifiers

		std::deque<QuadrantItem>* Insert(QuadrantItem&& item) {
			// Check if the item can be wholly contained in a children, and the children is created
			for (size_t i = 0; i < _children.size(); ++i) {
				if (auto& [childArea, child] = _children[i]; childArea.DoesContain(item.area, ComparisonTolerance)) {
					// Item can be wholly contained in the child
					if (child) {
						// Child is already created
						return child->Insert(std::move(item));
					} else if (_items.size() < ITEM_COUNT_THRESHOLD) {
						// Child is not created, but there isn't enough items at this level to create it yet
						_items.emplace_back(item);
						return &_items;
					} else {
						// Child is not created, and this level has too many items already
						// Distribute the items on this level to the children
						Distribute();
						// Check the child. It may or may not have been created during the distribution process
						if (not _children[i].second) {
							_children[i].second = std::make_unique<Quadrant>(_map, childArea);
						}
						return _children[i].second->Insert(std::move(item));
					}
				}
			}
			// Item does not wholly fit into any children, store it at this level
			_items.emplace_back(item);
			return &_items;
		}

		void Clear() {
			_items.clear();
			for (auto& child : _children) {
				child.second.reset();
			}
		}

	private:
		void Distribute() {
			// Distribute the items at this level to the children
			for (auto itemIt = _items.begin(); itemIt != _items.end(); ) {
				bool childCanContain = false;
				for (size_t i = 0; i < _children.size(); ++i) {
					if (auto& [childArea, child] = _children[i]; childArea.DoesContain(itemIt->area, ComparisonTolerance)) {
						// Item can be wholly contained in the child
						childCanContain = true;
						if (child) {
							// Child is already created, adjust container to new position
							_map.AdjustContainer(itemIt->id, child->Insert(std::move(*itemIt)));
						} else {
							// Child needs to be created
							_children[i].second = std::make_unique<Quadrant>(_map, childArea);
							// Adjust container to new position
							_map.AdjustContainer(itemIt->id, _children[i].second->Insert(std::move(*itemIt)));
						}
						itemIt = _items.erase(itemIt);
						break;
					}
				}
				if (not childCanContain) {
					++itemIt; // Item does not wholly fit into any children
				}
			}
		}

		friend class MapF<T,Capacity,ComparisonTolerance>;
	};

	/// The stored object is wrapped inside MapPoolItem before being placed in the Pool
	template <typename T>
	struct MapPoolItem {
		/// Back-pointer to either MapF::_foreign_items, or Quadrant::_items. If the object lays outside the area
		/// managed by the Map, this points to MapF::_foreign_items. Otherwise, it points to some Quadrant::_items
		/// in some Quadrant.
		std::deque<QuadrantItem>* container{};
		RectF area; /// Area taken up by the object
		T t; /// The object itself

		/// Constructor only constructs the object
		template <typename... Args>
		explicit MapPoolItem(Args&&... args) : t(std::forward<Args>(args)...) {}
	};
}
