#ifndef M2_PROXY_H
#define M2_PROXY_H

#include "Controls.h"
#include "Group.h"
#include "Object.h"
#include "Ui.h"
#include "Value.h"
#include <GroupType.pb.h>
#include <string_view>
#include <array>

namespace m2g {
	/// Name of the game, which is used to discover the resources
	extern const std::string_view game_name;

	/// Should the b2World have gravity towards downwards direction
	extern const bool gravity;

	/// Is the camera also a sound listener
	extern const bool camera_is_listener;

	/// UI
	namespace ui {
		extern const m2::ui::Blueprint entry;
		extern const m2::ui::Blueprint pause;
		extern const m2::ui::Blueprint left_hud;
		extern const m2::ui::Blueprint right_hud;
	}

	/// Controls
	m2::Key scancode_to_key(SDL_Scancode scancode);
	extern const std::array<SDL_Scancode, static_cast<unsigned>(m2::Key::end)> key_to_scancode;

	/// Called after a tile is created
	void post_tile_create(m2::Object& obj, pb::SpriteType sprite_type);

	/// Foreground object loaded
	m2::VoidValue fg_object_loader(m2::Object& obj, pb::ObjectType object_type);

	/// Create Group for the given type
	m2::Group* create_group(pb::GroupType group_type);
}

#endif //M2_PROXY_H