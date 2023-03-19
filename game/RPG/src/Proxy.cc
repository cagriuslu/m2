#include <m2/Proxy.h>
#include <rpg/group/ItemGroup.h>
#include <rpg/object/Enemy.h>
#include <rpg/object/Player.h>

const std::string_view m2g::game_name = "RPG";
const bool m2g::gravity = false;
const bool m2g::camera_is_listener = true;

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

void m2g::post_tile_create(MAYBE m2::Object& obj, MAYBE pb::SpriteType sprite_type) {}

m2::VoidValue m2g::fg_object_loader(m2::Object& obj, pb::ObjectType object_type) {
	using namespace obj;
	switch (object_type) {
		case pb::ObjectType::PLAYER:
			return Player::init(obj, &chr::character_player);
		case pb::ObjectType::SKELETON:
			return Enemy::init(obj, &chr::character_skeleton_000_chase);
		default:
			return m2::failure("Invalid sprite index");
	}
}

m2::Group* m2g::create_group(pb::GroupType group_type) {
	switch (group_type) {
		case pb::GroupType::CONSUMABLE_ITEM_HP20: {
			auto* g = new rpg::ItemGroup();
			g->add_item(pb::ITEM_CONSUMABLE_POTION_20HP);
			return g;
		}
		default:
			return nullptr;
	}
}
