#ifndef M2_DRAWLIST_H
#define M2_DRAWLIST_H

#include "Object.h"
#include "Vec2f.h"
#include <map>
#include <unordered_map>
#include "SpinLock.h"

namespace m2 {
	class DrawList {
	public:
		struct DrawItem {
			ObjectId obj_id;
			GraphicId gfx_id;
		};

	private:
		struct Vec2fComparator {
			bool operator()(const Vec2f& lhs, const Vec2f& rhs) const;
		};
		std::multimap<Vec2f, DrawItem, Vec2fComparator> draw_map;
		std::unordered_map<ObjectId, decltype(draw_map)::iterator> id_lookup;

		std::vector<std::pair<ObjectId, Vec2f>> update_queue;
		SpinLock update_queue_lock;

	public:
		struct ConstIterator {
			decltype(draw_map)::const_iterator map_it;

			ConstIterator& operator++();
			bool operator==(const ConstIterator& other) const;
			GraphicId operator*() const;
		};

		void insert(ObjectId id);
		void queue_update(ObjectId id, const Vec2f& pos);
		void update();
		void remove(ObjectId id);

		[[nodiscard]] ConstIterator begin() const;
		[[nodiscard]] ConstIterator end() const;
	};
}

#endif //M2_DRAWLIST_H
