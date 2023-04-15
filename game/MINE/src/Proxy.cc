#include <m2/Proxy.h>
#include <mine/object/Dwarf.h>
#include <mine/object/Blacksmith.h>

const std::string_view m2g::game_name = "MINE";
const bool m2g::gravity = true;
const bool m2g::camera_is_listener = false;

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

void m2g::post_tile_create(m2::Object& obj, pb::SpriteType sprite_type) {
	switch (sprite_type) {
		case pb::SpriteType::DUNGEON_COAL_1:
		case pb::SpriteType::DUNGEON_COAL_2: {
			// Add HP to destroyable tiles
			auto& chr = obj.add_tiny_character();
			chr.add_resource(m2g::pb::RESOURCE_HP, 2.0f);
			chr.set_max_resource(m2g::pb::RESOURCE_HP, 2.0f);
			break;
		}
		case pb::SpriteType::GRASSLAND_DIRT_1:
		case pb::SpriteType::GRASSLAND_DIRT_2: {
			// Add HP to destroyable tiles
			auto& chr = obj.add_tiny_character();
			chr.add_resource(m2g::pb::RESOURCE_HP, 1.0f);
			chr.set_max_resource(m2g::pb::RESOURCE_HP, 1.0f);
			break;
		}
		default:
			break;
	}
}

m2::VoidValue m2g::fg_object_loader(m2::Object& obj, pb::ObjectType object_type) {
	switch (object_type) {
		case pb::ObjectType::DWARF:
			return create_dwarf(obj);
		case pb::ObjectType::BLACKSMITH:
			return create_blacksmith(obj);
		default:
			return m2::failure("Invalid object type");
	}
}

m2::Group* m2g::create_group(MAYBE pb::GroupType group_type) {
	return nullptr;
}

std::vector<m2::Action> m2g::actions;
