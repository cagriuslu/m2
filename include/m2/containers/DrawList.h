#pragma once
#include <map>
#include <unordered_map>

#include <m2/Object.h>
#include <m2/SpinLock.h>
#include <m2/math/VecF.h>

namespace m2 {
	/// A list that orders the items with their Y-positions
	class DrawList {
	   public:
		struct DrawItem {
			ObjectId objId;
			GraphicId gfxId;
		};

	   private:
		/// Special comparator that sorts based on Y-axis because that's how we draw items
		struct VecFComparator {
			bool operator()(const VecF& lhs, const VecF& rhs) const;
		};

		/// A map is used to order the items
		std::multimap<VecF, DrawItem, VecFComparator> drawMap;

		/// A map that maps an ObjectId to an item in the draw_map
		std::unordered_map<ObjectId, decltype(drawMap)::iterator> idLookup;

		/// A queue of updates
		std::vector<std::pair<ObjectId, VecF>> updateQueue;

	   public:
		struct ConstIterator {
			decltype(drawMap)::const_iterator mapIt;

			ConstIterator& operator++();
			bool operator==(const ConstIterator& other) const;
			GraphicId operator*() const;
		};

		// Accessors

		[[nodiscard]] ConstIterator begin() const;
		[[nodiscard]] ConstIterator end() const;
		[[nodiscard]] bool ContainsObject(const ObjectId id) const { return idLookup.contains(id); }

		// Modifiers

		/// Add new object to the DrawList
		void Insert(ObjectId objId, GraphicId gfxId, const VecF& position);
		/// Queue a position update for an object for later
		void QueueUpdate(ObjectId id, const VecF& pos);
		/// Apply the queued updates
		void Update();
		/// Remove an object from the DrawList
		void Remove(ObjectId id);
	};
}  // namespace m2
