#ifndef M2_DRAWLIST_H
#define M2_DRAWLIST_H

#include "Object.h"
#include "Vec2f.h"
#include "Def.h"
#include <map>
#include <unordered_map>

namespace m2 {
	class DrawList {
	public:
		struct DrawItem {
			ObjectID obj_id;
			GraphicID gfx_id;
		};

	private:
		struct Vec2fComparator {
			bool operator()(const Vec2f& lhs, const Vec2f& rhs) const;
		};
		std::multimap<Vec2f, DrawItem, Vec2fComparator> draw_map;
		std::unordered_map<ObjectID, decltype(draw_map)::iterator> id_lookup;

	public:
		struct ConstIterator {
			decltype(draw_map)::const_iterator map_it;

			ConstIterator& operator++();
			bool operator==(const ConstIterator& other) const;
			GraphicID operator*() const;
		};

		void insert(ObjectID id);
		void update(ObjectID id);
		void remove(ObjectID id);

		[[nodiscard]] ConstIterator begin() const;
		[[nodiscard]] ConstIterator end() const;
	};
}

#endif //M2_DRAWLIST_H
