#pragma once
#include <map>
#include <unordered_map>

#include "Object.h"
#include "SpinLock.h"
#include "math/VecF.h"

namespace m2 {
	/// A list that orders the items with their Y-positions
	class DrawList {
	   public:
		struct DrawItem {
			ObjectId obj_id;
			GraphicId gfx_id;
		};

	   private:
		/// Special Vec2f comparator that sorts based on Y-axis because that's how we draw items
		struct Vec2fComparator {
			bool operator()(const VecF& lhs, const VecF& rhs) const;
		};

		/// A map is used to order the items
		std::multimap<VecF, DrawItem, Vec2fComparator> draw_map;

		/// A map that maps an ObjectId to an item in the draw_map
		std::unordered_map<ObjectId, decltype(draw_map)::iterator> id_lookup;

		/// A queue of updates
		std::vector<std::pair<ObjectId, VecF>> update_queue;

	   public:
		struct ConstIterator {
			decltype(draw_map)::const_iterator map_it;

			ConstIterator& operator++();
			bool operator==(const ConstIterator& other) const;
			GraphicId operator*() const;
		};

		/// Add new object to the DrawList
		void insert(ObjectId id);

		/// Queue a position update for an object for later
		void queue_update(ObjectId id, const VecF& pos);

		/// Apply the queued updates
		void update();

		/// Remove an object from the DrawList
		void remove(ObjectId id);

		[[nodiscard]] ConstIterator begin() const;
		[[nodiscard]] ConstIterator end() const;
	};
}  // namespace m2
