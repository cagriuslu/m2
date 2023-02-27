#include <m2/Level.h>
#include <m2g/Object.h>
#include <m2g/Ui.h>
#include <m2g/Group.h>
#include <m2/Value.h>
#include <m2/object/Tile.h>
#include <m2/object/Ghost.h>
#include <m2/object/God.h>
#include <m2/Editor.h>
#include <m2/object/Origin.h>
#include <m2/object/Camera.h>
#include <m2/object/Pointer.h>
#include <m2/object/Placeholder.h>
#include <m2/protobuf/Utils.h>
#include <m2/Game.h>
#include <filesystem>
#include <iterator>

m2::Level::~Level() {
	// Custom destructor is provided because the order is important
	characters.clear();
	lights.clear();
	terrainGraphics.clear();
	graphics.clear();
	physics.clear();
	objects.clear();
	groups.clear();

	delete contactListener;
	contactListener = nullptr;
	delete world;
	world = nullptr;
}

m2::VoidValue m2::Level::init_single_player(const std::variant<FilePath,pb::Level>& level_path_or_blueprint) {
	_type = Type::SINGLE_PLAYER;
	if (std::holds_alternative<FilePath>(level_path_or_blueprint)) {
		_lb_path = std::get<FilePath>(level_path_or_blueprint);
		auto lb = proto::json_file_to_message<pb::Level>(*_lb_path);
		m2_reflect_failure(lb);
		_lb = *lb;
	} else {
		_lb_path = {};
		_lb = std::get<pb::Level>(level_path_or_blueprint);
	}

	world = new b2World(m2g::gravity ? b2Vec2{0.0f, 10.0f} : b2Vec2{});
	contactListener = new m2::box2d::ContactListener(m2::Physique::default_begin_contact_cb, m2::Physique::default_end_contact_cb);
	world->SetContactListener(contactListener);

	// Create background tiles
	for (int y = 0; y < _lb->background_rows_size(); ++y) {
		for (int x = 0; x < _lb->background_rows(y).items_size(); ++x) {
			auto sprite_type = _lb->background_rows(y).items(x);
			if (sprite_type) {
                LOGF_TRACE("Creating tile from %d sprite at (%d,%d)...", sprite_type, x, y);
				auto [tile_obj, tile_id] = obj::create_tile(Vec2f{x, y} + Vec2f{0.5f, 0.5f}, GAME.sprites[sprite_type]);
				m2g::post_tile_create(tile_obj, sprite_type);
                LOG_TRACE("Created tile", tile_id);
			}
		}
	}
	// Create foreground objects
	for (const auto& fg_object : _lb->objects()) {
        LOGF_TRACE("Creating %d type object at (%d,%d)...", fg_object.type(), fg_object.position().x(), fg_object.position().y());
		auto [obj, id] = m2::create_object(m2::Vec2f{fg_object.position()} + Vec2f{0.5f, 0.5f});

		// Assign to group
		if (fg_object.has_group() && fg_object.group().type() != m2g::pb::GroupType::NO_GROUP) {
			GroupId group_id{fg_object.group()};

			Group *group;
			auto group_it = groups.find(group_id);
			if (group_it != groups.end()) {
				group = group_it->second.get();
			} else {
				group = m2g::create_group(group_id.type);
				groups[group_id] = std::unique_ptr<Group>(group);
			}
			obj.set_group(group_id, group->add_member(id));
		}

		auto load_result = m2g::fg_object_loader(obj, fg_object.type());
		m2_reflect_failure(load_result);
        LOG_TRACE("Created object", id);
	}
	// Init pathfinder map
	pathfinder = Pathfinder{*_lb};

	// Create default objects
	m2::obj::create_camera();
	m2::obj::create_pointer();

	// Init HUD
	leftHudUIState = m2::ui::State(&m2g::ui::left_hud);
	leftHudUIState->update_positions(GAME.leftHudRect);
	leftHudUIState->update_contents();
	rightHudUIState = m2::ui::State(&m2g::ui::right_hud);
	rightHudUIState->update_positions(GAME.rightHudRect);
	rightHudUIState->update_contents();

	return {};
}
m2::VoidValue m2::Level::init_editor(const FilePath& lb_path) {
	_type = Type::EDITOR;
	if (std::filesystem::exists(lb_path)) {
		_lb_path = lb_path;
		auto lb = proto::json_file_to_message<pb::Level>(*_lb_path);
		m2_reflect_failure(lb);
		_lb = *lb;

		// Create background tiles
		for (int y = 0; y < lb->background_rows_size(); ++y) {
			for (int x = 0; x < lb->background_rows(y).items_size(); ++x) {
				auto sprite_type = lb->background_rows(y).items(x);
				if (sprite_type) {
					auto position = Vec2f{x, y};
					editor_bg_placeholders[position.iround()] = obj::create_placeholder(position, GAME.sprites[sprite_type], false);
				}
			}
		}
		// Create foreground objects
		for (const auto& fg_object : lb->objects()) {
			auto position = m2::Vec2f{fg_object.position()};
			editor_fg_placeholders[position.iround()] = obj::create_placeholder(position, GAME.sprites[GAME.editor_object_sprites[fg_object.type()]], true);
		}
	} else {
		_lb_path = lb_path;
		_lb = pb::Level{};
	}

	// Create default objects
	playerId = m2::obj::create_god();
	m2::obj::create_camera();
	m2::obj::create_origin();

	// UI Hud
	leftHudUIState = m2::ui::State(&ui::editor_left_hud);
	leftHudUIState->update_positions(GAME.leftHudRect);
	leftHudUIState->update_contents();
	rightHudUIState = m2::ui::State(&ui::editor_right_hud);
	rightHudUIState->update_positions(GAME.rightHudRect);
	rightHudUIState->update_contents();

	return {};
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
		auto sprite_type = LEVEL.editor_paint_mode_selected_sprite_type;
		// Allocate item if necessary
		while (_lb->background_rows_size() < position.y + 1) {
			_lb->add_background_rows();
		}
		while (_lb->background_rows(position.y).items_size() < position.x + 1) {
			_lb->mutable_background_rows(position.y)->add_items({});
		}
		// Paint lut_index
		_lb->mutable_background_rows(position.y)->set_items(position.x, sprite_type);
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
	_lb->mutable_background_rows(position.y)->set_items(position.x, {});
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
		for (int i = 0; i < _lb->objects_size(); ++i) {
			if (position == Vec2i{_lb->objects(i).position()}) {
				auto* mutable_objects = _lb->mutable_objects();
				mutable_objects->erase(mutable_objects->begin() + i);
			}
		}
		// Add object to fg objects
		auto object_type = editor_place_mode_selected_object_type;
		auto* new_fg_object = _lb->add_objects();
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
	for (int i = 0; i < _lb->objects_size(); ++i) {
		if (position == Vec2i{_lb->objects(i).position()}) {
			auto* mutable_objects = _lb->mutable_objects();
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
	*proto::message_to_json_file(*_lb, *_lb_path);
}
