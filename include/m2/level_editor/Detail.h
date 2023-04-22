#ifndef M2_LEVELEDITOR_DETAIL_H
#define M2_LEVELEDITOR_DETAIL_H

#include <m2/Object.h>
#include <m2/Pool.hh>
#include <m2/Vec2i.h>

namespace m2::level_editor::detail {
	using PlaceholderMap = std::unordered_map<Vec2i, Id, Vec2iHash>;

	void shift_placeholders_right(PlaceholderMap& placeholders, Pool<Object>& objects, int x);
	void shift_placeholders_down(PlaceholderMap& placeholders, Pool<Object>& objects, int y);
	void shift_placeholders_right_down(PlaceholderMap& placeholders, Pool<Object>& objects, int x, int y);
}

#endif //M2_LEVELEDITOR_DETAIL_H
