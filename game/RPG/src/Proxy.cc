#include <m2/Proxy.h>
#include <rpg/group/ItemGroup.h>
#include <rpg/object/Enemy.h>
#include <rpg/object/Player.h>
#include <rpg/Objects.h>
#include <rpg/Context.h>
#include <m2/Game.h>
#include <rpg/Defs.h>

using namespace rpg;

const std::string_view m2g::game_name = "RPG";
const int m2g::default_game_height_m = 16;
const bool m2g::gravity = false;
const bool m2g::world_is_static = true;
const bool m2g::lightning = false;
float m2g::focus_point_height = 2.0f;
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
		case SDL_SCANCODE_MINUS:
			return Key::MINUS;
		case SDL_SCANCODE_EQUALS:
			return Key::PLUS;
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
		SDL_SCANCODE_BACKSPACE,
		SDL_SCANCODE_MINUS,
		SDL_SCANCODE_EQUALS
};

void m2g::pre_single_player_level_init(const std::string& name, MAYBE const m2::pb::Level& level) {
	LOG_INFO("Loading level", name);
}

void m2g::post_single_player_level_init(MAYBE const std::string& name, const m2::pb::Level& level) {
	const auto& id = level.identifier();
	if (id == "WalkingTutorialClosed") {
		LEVEL.display_message("Use W,A,S,D to walk.", MESSAGE_TIMEOUT);
	} else if (id == "WalkingTutorialOpen") {
		LEVEL.display_message("Some levels will be outdoors.", MESSAGE_TIMEOUT);
	} else if (id == "FlagTutorialClosed") {
		LEVEL.display_message("Find the blue flag to exit the level.", MESSAGE_TIMEOUT);
	} else if (id == "FlagTutorialOpen") {
		LEVEL.display_message("Find the blue flag to exit the level.", MESSAGE_TIMEOUT);
	} else if (id == "DashTutorialClosed") {
		LEVEL.display_message("Use SPACE button while walking to dash.", MESSAGE_TIMEOUT);
	} else if (id == "RangedWeaponTutorialClosed") {
		LEVEL.display_message("Use left mouse button to shoot bullets.", MESSAGE_TIMEOUT);
	} else if (id == "MeleeTutorialClosed") {
		LEVEL.display_message("Use right mouse button to melee.", MESSAGE_TIMEOUT);
	} else if (id == "AllMustBeKilledTutorialOpen") {
		LEVEL.display_message("All enemies must be killed to complete the level successfully.", MESSAGE_TIMEOUT);
	}
}

void m2g::pre_multi_player_level_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}
void m2g::post_multi_player_level_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}

void m2g::post_tile_create(MAYBE m2::Object& obj, MAYBE pb::SpriteType sprite_type) {}

m2::void_expected m2g::init_fg_object(m2::Object& obj, pb::ObjectType object_type) {
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
		case pb::ObjectType::MACHINE_GUN_ITEM_DROP:
			return rpg::create_dropped_item(obj, m2g::pb::ITEM_REUSABLE_MACHINE_GUN);
		case pb::ObjectType::EXPLOSIVE_ITEM_DROP:
			return rpg::create_dropped_item(obj, m2g::pb::ITEM_REUSABLE_EXPLOSIVE);
		case pb::ObjectType::LONG_SWORD_ITEM_DROP:
			return rpg::create_dropped_item(obj, m2g::pb::ITEM_REUSABLE_LONG_SWORD);
		case pb::ObjectType::AXE_ITEM_DROP:
			return rpg::create_dropped_item(obj, m2g::pb::ITEM_REUSABLE_AXE);
		case pb::BUSH_01:
		case pb::FLOWER_ORANGE_02:
		case pb::FLOWER_PINK_03:
		case pb::FLOWER_WHITE_05:
		case pb::PINE_01:
		case pb::PINE_DEAD_02:
		case pb::ROCK_03:
		case pb::TREE_03:
		case pb::TREE_DEAD_03:
		case pb::FENCE_HORIZONTAL:
		case pb::FENCE_VERTICAL:
			return rpg::create_decoration(obj, object_type, GAME.object_main_sprites[object_type]);
		case pb::SPIKES:
			return rpg::create_spikes(obj);
		default:
			return m2::make_unexpected("Unhandled object type");
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
