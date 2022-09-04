#ifndef IMPL_SPRITE_H
#define IMPL_SPRITE_H

#include <initializer_list>
#include <string>
#include <utility>

namespace m2g {
	enum class SpriteID {
		Grass,
		Player,
		Skeleton,
		Bullet0,
		Bullet1,
		Bomb,
		Sword,
		RedPotion,
		_end_
	};

	using SpriteIDToSpriteKeyLUT = std::initializer_list<std::pair<SpriteID, const char*>>;
	constexpr SpriteIDToSpriteKeyLUT sprite_id_to_sprite_key_lut{
		{SpriteID::Grass,     "main.grass"},
		{SpriteID::Player,    "main.player_lookdown_00"},
		{SpriteID::Skeleton,  "main.enemy_lookdown_00"},
		{SpriteID::Bullet0,   "main.bullet_00"},
		{SpriteID::Bullet1,   "main.bullet_01"},
		{SpriteID::Bomb,      "main.bomb_00"},
		{SpriteID::Sword,     "main.sword_00"},
		{SpriteID::RedPotion, "main.red_potion_00"}
	};
}

#endif //IMPL_SPRITE_H
