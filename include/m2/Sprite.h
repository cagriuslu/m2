#ifndef M2_SPRITE_H
#define M2_SPRITE_H

#include <SpriteSheets.pb.h>
#include <string>

namespace m2::sprite {
	std::unordered_map<std::string, pb::SpriteSheet> load_sprite_sheets(const std::string& sprite_sheets_path);
	std::unordered_map<std::string, pb::Sprite> load_sprites(const std::string& sprite_sheets_path);
}

#endif //M2_SPRITE_H
