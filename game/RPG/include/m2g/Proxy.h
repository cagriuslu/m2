#ifndef IMPL_PROXY_H
#define IMPL_PROXY_H

#include <string_view>

namespace m2g {
	constexpr std::string_view texture_map_file("resource/48.png");
	constexpr std::string_view texture_mask_file("resource/48-Mask.png");
	constexpr std::string_view sprite_sheets("resource/game/RPG/SpriteSheets.json");

	void dynamic_assert();
}

#endif //IMPL_PROXY_H
