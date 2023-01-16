#include <m2/Level.h>
#include <m2/Value.h>
#include <m2/object/Ghost.h>
#include <m2/object/Placeholder.h>
#include <m2/protobuf/Utils.h>
#include <m2/Game.h>
#include <string>
#include <filesystem>
#include <algorithm>
#include <iterator>

m2::Level::Level(Type type) : _type(type) {}
m2::Level::Level(Type type, std::string lb_path) : _type(type), _lb_path(std::move(lb_path)) {}
m2::Level::Type m2::Level::type() const {
	return _type;
}
void m2::Level::activate_mode(EditorMode mode) {
	editor_mode = mode;
	switch (mode) {
		case EditorMode::PAINT:
			editor_paint_mode_select_sprite_type(GAME.editor_background_sprites[0]);
			break;
		case EditorMode::PLACE:
			editor_place_mode_select_object_type(GAME.editor_object_sprites.begin()->first);
			editor_place_mode_select_group_type(m2g::pb::GroupType::NO_GROUP);
			editor_place_mode_select_group_instance(0);
			break;
		default:
			editor_paint_mode_select_sprite_type({});
			editor_place_mode_select_object_type({});
			break;
	}
}
void m2::Level::editor_paint_mode_select_sprite_type(m2g::pb::SpriteType sprite_type) {
	if (editor_paint_or_place_mode_selected_sprite_ghost_id) {
		GAME.add_deferred_action(m2::create_object_deleter(editor_paint_or_place_mode_selected_sprite_ghost_id));
	}
	if (sprite_type) {
		editor_paint_mode_selected_sprite_type = sprite_type;
		editor_paint_or_place_mode_selected_sprite_ghost_id = obj::create_ghost(GAME.sprites[sprite_type]);
	}
}
void m2::Level::editor_paint_mode_paint_sprite(const Vec2i& position) {
	if (!position.is_negative()) {
		auto sprite_type = GAME.level->editor_paint_mode_selected_sprite_type;
		// Allocate item if necessary
		while (_lb.background_rows_size() < position.y + 1) {
			_lb.add_background_rows();
		}
		while (_lb.background_rows(position.y).items_size() < position.x + 1) {
			_lb.mutable_background_rows(position.y)->add_items({});
		}
		// Paint lut_index
		_lb.mutable_background_rows(position.y)->set_items(position.x, sprite_type);
		// Create/Replace placeholder
		auto placeholders_it = editor_bg_placeholders.find(position);
		if (placeholders_it != editor_bg_placeholders.end()) {
			deferred_actions.push_back(create_object_deleter(placeholders_it->second));
		}
		editor_bg_placeholders[position] = obj::create_placeholder(Vec2f{position}, GAME.sprites[sprite_type], false);
	}
}
void m2::Level::editor_erase_mode_erase_position(const Vec2i &position) {
	// Erase lut_index
	_lb.mutable_background_rows(position.y)->set_items(position.x, {});
	// Delete placeholder
	auto placeholders_it = editor_bg_placeholders.find(position);
	if (placeholders_it != editor_bg_placeholders.end()) {
		deferred_actions.push_back(create_object_deleter(placeholders_it->second));
	}
}
void m2::Level::editor_place_mode_select_object_type(m2g::pb::ObjectType object_type) {
	if (editor_paint_or_place_mode_selected_sprite_ghost_id) {
		GAME.add_deferred_action(create_object_deleter(editor_paint_or_place_mode_selected_sprite_ghost_id));
	}
	if (object_type) {
		editor_place_mode_selected_object_type = object_type;
		editor_paint_or_place_mode_selected_sprite_ghost_id = obj::create_ghost(GAME.sprites[GAME.editor_object_sprites[object_type]]);
	}
}
void m2::Level::editor_place_mode_select_group_type(m2g::pb::GroupType group_type) { editor_place_mode_selected_group_type = group_type; }
void m2::Level::editor_place_mode_select_group_instance(unsigned group_instance) { editor_place_mode_selected_group_instance = group_instance; }
void m2::Level::editor_place_mode_place_object(const Vec2i& position) {
	if (!position.is_negative()) {
		// Check if object is in fg objects, remove if found
		for (int i = 0; i < _lb.objects_size(); ++i) {
			if (position == Vec2i{_lb.objects(i).position()}) {
				auto* mutable_objects = _lb.mutable_objects();
				mutable_objects->erase(mutable_objects->begin() + i);
			}
		}
		// Add object to fg objects
		auto object_type = editor_place_mode_selected_object_type;
		auto* new_fg_object = _lb.add_objects();
		new_fg_object->mutable_position()->set_x(position.x);
		new_fg_object->mutable_position()->set_y(position.y);
		new_fg_object->set_type(object_type);
		auto* group = new_fg_object->mutable_group();
		group->set_type(editor_place_mode_selected_group_type);
		group->set_instance(editor_place_mode_selected_group_instance);
		// Create/Replace placeholder
		auto placeholders_it = editor_fg_placeholders.find(position);
		if (placeholders_it != editor_fg_placeholders.end()) {
			deferred_actions.push_back(create_object_deleter(placeholders_it->second));
		}
		editor_fg_placeholders[position] = obj::create_placeholder(Vec2f{position}, GAME.sprites[GAME.editor_object_sprites[object_type]], true);
	}
}
void m2::Level::editor_remove_mode_remove_object(const Vec2i &position) {
	// Check if object is in fg objects, remove if found
	for (int i = 0; i < _lb.objects_size(); ++i) {
		if (position == Vec2i{_lb.objects(i).position()}) {
			auto* mutable_objects = _lb.mutable_objects();
			mutable_objects->erase(mutable_objects->begin() + i);
		}
	}
	// Create/Replace placeholder
	auto placeholders_it = editor_fg_placeholders.find(position);
	if (placeholders_it != editor_fg_placeholders.end()) {
		deferred_actions.push_back(create_object_deleter(placeholders_it->second));
	}
}
void m2::Level::editor_save() {
	*proto::message_to_json_file(_lb, _lb_path);
}
m2::Value<m2::Level> m2::Level::create_single_player_level() {
	return Level{Type::SINGLE_PLAYER};
}
m2::Value<m2::Level> m2::Level::create_single_player_level(const std::string& lb_path) {
	return Level{Type::SINGLE_PLAYER, lb_path};
}
m2::Value<m2::Level> m2::Level::create_editor_level(const std::string& lb_path) {
	Level editor_level{Type::EDITOR, lb_path};

	if (std::filesystem::exists(lb_path)) {
		Value<pb::Level> lb = proto::json_file_to_message<pb::Level>(lb_path);
		m2_reflect_failure(lb);
		editor_level._lb = *lb;
		// Create background tiles
		for (int y = 0; y < lb->background_rows_size(); ++y) {
			for (int x = 0; x < lb->background_rows(y).items_size(); ++x) {
				auto sprite_type = lb->background_rows(y).items(x);
				if (sprite_type) {
					auto position = Vec2f{x, y};
					editor_level.editor_bg_placeholders[position.iround()] = obj::create_placeholder(position, GAME.sprites[sprite_type], false);
				}
			}
		}
		// Create foreground objects
		for (const auto& fg_object : lb->objects()) {
			auto position = m2::Vec2f{fg_object.position()};
			editor_level.editor_fg_placeholders[position.iround()] = obj::create_placeholder(position, GAME.sprites[GAME.editor_object_sprites[fg_object.type()]], true);
		}
	}

	return editor_level;
}
