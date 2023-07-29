#include <m2/Proxy.h>
#include <rpg/group/ItemGroup.h>
#include <rpg/object/Enemy.h>
#include <rpg/object/Player.h>
#include <rpg/object/FinishPoint.h>
#include <rpg/Context.h>
#include <rpg/object/DroppedItem.h>
#include <rpg/object/Decoration.h>
#include <m2/Game.h>
#include <rpg/Defs.h>

using namespace rpg;

const std::string_view m2g::game_name = "RPG";
const m2::Rational m2g::default_game_height_m = {16, 1};
const bool m2g::gravity = false;
const bool m2g::world_is_static = true;
const bool m2g::lightning = false;
float m2g::camera_height = 0.0f;
float m2g::camera_distance = 8.0f;
float m2g::horizontal_field_of_view = 90.0f;
const float m2g::xy_plane_z_component = 0.75f;
const bool m2g::camera_is_listener = true;

void* m2g::create_context() {
	auto* context = new rpg::Context();
	return context;
}
void m2g::destroy_context(MAYBE void* context) {
	delete reinterpret_cast<rpg::Context*>(context);
}

m2::Key m2g::scancode_to_key(SDL_Scancode scancode) {
	using namespace m2;
	switch (scancode) {
		case SDL_SCANCODE_ESCAPE:
			return Key::MENU;
		case SDL_SCANCODE_W:
			return Key::UP;
		case SDL_SCANCODE_S:
			return Key::DOWN;
		case SDL_SCANCODE_A:
			return Key::LEFT;
		case SDL_SCANCODE_D:
			return Key::RIGHT;
		case SDL_SCANCODE_SPACE:
			return Key::DASH;
		case SDL_SCANCODE_GRAVE:
			return Key::CONSOLE;
		case SDL_SCANCODE_RETURN:
			return Key::ENTER;
		case SDL_SCANCODE_BACKSPACE:
			return Key::BACKSPACE;
		default:
			return Key::UNKNOWN;
	}
}

const std::array<SDL_Scancode, static_cast<unsigned>(m2::Key::end)> m2g::key_to_scancode = {
		SDL_SCANCODE_UNKNOWN,
		SDL_SCANCODE_ESCAPE,
		SDL_SCANCODE_W,
		SDL_SCANCODE_S,
		SDL_SCANCODE_A,
		SDL_SCANCODE_D,
		SDL_SCANCODE_SPACE,
		SDL_SCANCODE_GRAVE,
		SDL_SCANCODE_RETURN,
		SDL_SCANCODE_BACKSPACE
};

void m2g::pre_single_player_level_init(const std::string& name) {
	LOG_INFO("Loading level", name);

	auto level_number = strtol(name.c_str(), nullptr, 10);
	if (level_number < 5) {
		m2g::camera_height = 0.0f;
	} else {
		m2g::camera_height = 5.5f;
	}
}

void m2g::post_single_player_level_init(const std::string& name) {
	if (LEVEL.name() == "1") {
		LEVEL.display_message("Use W,A,S,D to walk, SPACE to dash.", MESSAGE_TIMEOUT);
	} else if (LEVEL.name() == "2") {
		LEVEL.display_message("Use left mouse button to shoot.", MESSAGE_TIMEOUT);
	} else if (LEVEL.name() == "3") {
		LEVEL.display_message("Use right mouse button to melee.", MESSAGE_TIMEOUT);
	}
}

void m2g::post_tile_create(MAYBE m2::Object& obj, MAYBE pb::SpriteType sprite_type) {}

m2::VoidValue m2g::fg_object_loader(m2::Object& obj, pb::ObjectType object_type) {
	using namespace rpg;
	switch (object_type) {
		case pb::ObjectType::PLAYER:
			return Player::init(obj);
		case pb::ObjectType::SKELETON:
			return Enemy::init(obj, object_type);
		case pb::ObjectType::CASTLE_FINISH_POINT:
			return rpg::init_finish_point(obj, object_type);
		case pb::ObjectType::CUTEOPUS:
			return Enemy::init(obj, object_type);
		case pb::ObjectType::MACHINE_GUN:
			return rpg::create_dropped_item(obj, m2g::pb::ITEM_REUSABLE_MACHINE_GUN);
		case pb::ObjectType::EXPLOSIVE:
			return rpg::create_dropped_item(obj, m2g::pb::ITEM_REUSABLE_EXPLOSIVE);
		case pb::BUSH_01:
			return rpg::create_decoration(obj, GAME.level_editor_object_sprites[object_type]);
		default:
			return m2::failure("Unhandled object type");
	}
}

m2::Group* m2g::create_group(pb::GroupType group_type) {
	switch (group_type) {
		case pb::GROUP_LOW_HP_POTION_DROPPER:
			return new rpg::ItemGroup({{pb::ITEM_CONSUMABLE_HP_POTION_20, 4}, {pb::ITEM_CONSUMABLE_HP_POTION_80, 1}});
		case pb::GROUP_MACHINE_GUN_DROPPER:
			return new rpg::ItemGroup({{pb::ITEM_REUSABLE_MACHINE_GUN, 1}});
		default:
			return nullptr;
	}
}
