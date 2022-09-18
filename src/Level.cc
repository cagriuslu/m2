#include <m2/Level.h>
#include <m2/Value.h>
#include <m2/object/Ghost.h>
#include <m2/object/Placeholder.h>
#include <m2/Proto.h>
#include <m2/Game.hh>
#include <string>
#include <filesystem>
#include <algorithm>

namespace {
	m2::VoidValue level_blueprint_iterate_tiles(
		const m2::pb::Level& lb,
		const std::function<void(const m2::Vec2f& position, const m2::Sprite& sprite)>& bg_tile_loader,
		const std::function<void(const m2::Vec2f& position, const std::string& sprite_key, const m2::pb::GroupBlueprint& gb)>& fg_object_loader) {
		if (bg_tile_loader) {
			// Look up background sprites
			std::vector<const m2::Sprite*> bg_sprites;
			for (const auto& bg_sprite_key : lb.bg_tile_lut()) {
				auto sprite_it = GAME.sprite_key_to_sprite_map.find(bg_sprite_key);
				m2_fail_unless(sprite_it != GAME.sprite_key_to_sprite_map.end(), "Unknown sprite key " + bg_sprite_key);
				bg_sprites.push_back(&sprite_it->second);
			}
			// Create background tiles
			for (int y = 0; y < lb.bg_rows_size(); ++y) {
				for (int x = 0; x < lb.bg_rows(y).items_size(); ++x) {
					auto lut_index = lb.bg_rows(y).items(x);
					if (lut_index < 0) {
						continue; // Skip negative tiles
					}
					if (lb.bg_tile_lut_size() <= lut_index) {
						return m2::failure("Background tile LUT index out of bounds");
					}
					bg_tile_loader(m2::Vec2f{x, y}, *bg_sprites[lut_index]);
				}
			}
		}
		if (fg_object_loader) {
			// Create foreground objects
			for (const auto& fg_object : lb.fg_objects()) {
				fg_object_loader(m2::Vec2f{fg_object.position()}, fg_object.key(), fg_object.group());
			}
		}
		return {};
	}
}

m2::Level::Level(Type type, std::string lb_path) : _type(type), _lb_path(std::move(lb_path)) {}

m2::Level::Type m2::Level::type() const {
	return _type;
}

void m2::Level::activate_mode(EditorMode mode) {
	editor_mode = mode;
	switch (mode) {
		case EditorMode::PAINT:
		case EditorMode::PLACE:
			editor_paint_or_place_mode_select_sprite(0);
			break;
		default:
			editor_paint_or_place_mode_select_sprite(-1);
			break;
	}
}
void m2::Level::editor_paint_or_place_mode_select_sprite(int index) {
	auto ghost_deleter = [](Id ghost_id) {
		if (ghost_id) {
			GAME.add_deferred_action(m2::create_object_deleter(ghost_id));
		}
	};

	if (index < 0) {
		ghost_deleter(editor_paint_or_place_mode_selected_sprite_ghost_id);
		editor_paint_or_place_mode_selected_sprite = -1;
	} else if (editor_mode == EditorMode::PAINT && index < (long)GAME.editor_bg_sprites.size() && index != editor_paint_or_place_mode_selected_sprite) {
		ghost_deleter(editor_paint_or_place_mode_selected_sprite_ghost_id);
		editor_paint_or_place_mode_selected_sprite = index;
		editor_paint_or_place_mode_selected_sprite_ghost_id = obj::create_ghost(GAME.sprite_key_to_sprite_map.at(GAME.editor_bg_sprites[index]));
	} else if (editor_mode == EditorMode::PLACE && index < (long)GAME.editor_fg_sprites.size() && index != editor_paint_or_place_mode_selected_sprite) {
		ghost_deleter(editor_paint_or_place_mode_selected_sprite_ghost_id);
		editor_paint_or_place_mode_selected_sprite = index;
		editor_paint_or_place_mode_selected_sprite_ghost_id = obj::create_ghost(GAME.sprite_key_to_sprite_map.at(GAME.editor_fg_sprites[index]));
	}
}

void m2::Level::editor_paint_mode_paint_sprite(const Vec2i& position) {
	if (0 <= editor_paint_or_place_mode_selected_sprite && position.in_nonnegative()) {
		auto sprite_key = GAME.editor_bg_sprites[editor_paint_or_place_mode_selected_sprite];
		// Check if sprite key is in LUT
		auto lut_it = std::find(_lb.bg_tile_lut().begin(), _lb.bg_tile_lut().end(), sprite_key);
		if (lut_it == _lb.bg_tile_lut().end()) {
			// Add to LUT
			_lb.add_bg_tile_lut(sprite_key);
			lut_it = _lb.bg_tile_lut().end() - 1;
		}
		auto lut_index = lut_it - _lb.bg_tile_lut().begin();
		// Allocate item if necessary
		while (_lb.bg_rows_size() < position.y + 1) {
			_lb.add_bg_rows();
		}
		while (_lb.bg_rows(position.y).items_size() < position.x + 1) {
			_lb.mutable_bg_rows(position.y)->add_items(-1);
		}
		// Paint lut_index
		_lb.mutable_bg_rows(position.y)->set_items(position.x, (int32_t)lut_index);
		// Create/Replace placeholder
		auto placeholders_it = editor_bg_placeholders.find(position);
		if (placeholders_it != editor_bg_placeholders.end()) {
			deferred_actions.push_back(create_object_deleter(placeholders_it->second));
		}
		editor_bg_placeholders[position] = obj::create_placeholder(Vec2f{position}, GAME.sprite_key_to_sprite_map.at(sprite_key), false);
	}
}

void m2::Level::editor_erase_mode_erase_position(const Vec2i &position) {
	// Erase lut_index
	_lb.mutable_bg_rows(position.y)->set_items(position.x, -1);
	// Delete placeholder
	auto placeholders_it = editor_bg_placeholders.find(position);
	if (placeholders_it != editor_bg_placeholders.end()) {
		deferred_actions.push_back(create_object_deleter(placeholders_it->second));
	}
}

void m2::Level::editor_place_mode_place_object(const Vec2i& position) {
	if (0 <= editor_paint_or_place_mode_selected_sprite && position.in_nonnegative()) {
		// Check if object is in fg objects, remove if found
		for (int i = 0; i < _lb.fg_objects_size(); ++i) {
			if (position == Vec2i{_lb.fg_objects(i).position()}) {
				auto* mutable_fg_objects = _lb.mutable_fg_objects();
				mutable_fg_objects->erase(mutable_fg_objects->begin() + i);
			}
		}
		// Add object to fg objects
		auto sprite_key = GAME.editor_fg_sprites[editor_paint_or_place_mode_selected_sprite];
		auto* new_fg_object = _lb.add_fg_objects();
		new_fg_object->mutable_position()->set_x(position.x);
		new_fg_object->mutable_position()->set_y(position.y);
		new_fg_object->set_key(sprite_key);
		// Create/Replace placeholder
		auto placeholders_it = editor_fg_placeholders.find(position);
		if (placeholders_it != editor_fg_placeholders.end()) {
			deferred_actions.push_back(create_object_deleter(placeholders_it->second));
		}
		editor_fg_placeholders[position] = obj::create_placeholder(Vec2f{position}, GAME.sprite_key_to_sprite_map.at(sprite_key), true);
	}
}

void m2::Level::editor_remove_mode_remove_object(const Vec2i &position) {
	// Check if object is in fg objects, remove if found
	for (int i = 0; i < _lb.fg_objects_size(); ++i) {
		if (position == Vec2i{_lb.fg_objects(i).position()}) {
			auto* mutable_fg_objects = _lb.mutable_fg_objects();
			mutable_fg_objects->erase(mutable_fg_objects->begin() + i);
		}
	}
	// Create/Replace placeholder
	auto placeholders_it = editor_fg_placeholders.find(position);
	if (placeholders_it != editor_fg_placeholders.end()) {
		deferred_actions.push_back(create_object_deleter(placeholders_it->second));
	}
}

m2::Value<m2::Level> m2::Level::create_single_player_level(const std::string& lb_path) {
	Level level{Type::SINGLE_PLAYER, lb_path};
	return level;
}

m2::Value<m2::Level> m2::Level::create_editor_level(const std::string& lb_path) {
	Level editor_level{Type::EDITOR, lb_path};

	if (std::filesystem::exists(lb_path)) {
		Value<pb::Level> lb = proto::json_file_to_message<pb::Level>(lb_path);
		m2_reflect_failure(lb);

		editor_level._lb = *lb;

		auto iterate_results = level_blueprint_iterate_tiles(editor_level._lb,
			[&](const m2::Vec2f& position, const m2::Sprite& sprite) {
				editor_level.editor_bg_placeholders[position.iround()] = obj::create_placeholder(position, sprite, false);
			},
			[&](const m2::Vec2f& position, const std::string& sprite_key, MAYBE const m2::pb::GroupBlueprint& gb) {
				editor_level.editor_fg_placeholders[position.iround()] = obj::create_placeholder(position, GAME.sprite_key_to_sprite_map.at(sprite_key), true);
			}
		);
	}

	return editor_level;
}
