#ifndef IMPL_PROXY_H
#define IMPL_PROXY_H

#include <string_view>

namespace m2g {
	constexpr bool gravity = false;
	constexpr std::string_view objects("resource/game/RPG/Objects.json");
	constexpr std::string_view sprite_sheets("resource/game/RPG/SpriteSheets.json");
	constexpr std::string_view items("resource/game/RPG/Items.json");
	constexpr std::string_view animations("resource/game/RPG/Animations.json");
}

#endif //IMPL_PROXY_H
