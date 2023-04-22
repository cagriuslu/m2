#include <m2/protobuf/Level.h>

void m2::proto::level::shift_background_right(pb::Level& lb, const Vec2i& pos) {
	// Iterate over background rows
	for (int y = 0; y < lb.background_rows_size(); ++y) {
		auto* sprite_type_array = lb.mutable_background_rows(y);
		if (pos.x < sprite_type_array->items_size()) {
			// Append element
			*(sprite_type_array->mutable_items()->Add()) = m2g::pb::SpriteType::NO_SPRITE;
			// Swap elements
			for (int x = sprite_type_array->items_size() - 1; pos.x < x--; ) {
				sprite_type_array->mutable_items()->SwapElements(x, x + 1);
			}
		}
	}
}

void m2::proto::level::shift_foreground_right(pb::Level& lb, const Vec2i& pos) {
	// Iterate over foreground objects
	for (int o = 0; o < lb.objects_size(); ++o) {
		auto* level_object = lb.mutable_objects(o);
		auto* level_object_pos = level_object->mutable_position();
		if (pos.x <= level_object_pos->x()) {
			level_object_pos->set_x(level_object_pos->x() + 1);
		}
	}
}

void m2::proto::level::shift_background_down(pb::Level& lb, const Vec2i& pos) {

}

void m2::proto::level::shift_foreground_down(pb::Level& lb, const Vec2i& pos) {

}

void m2::proto::level::shift_background_right_down(pb::Level& lb, const Vec2i& pos) {

}

void m2::proto::level::shift_foreground_right_down(pb::Level& lb, const Vec2i& pos) {

}
