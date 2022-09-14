#ifndef IMPL_SPRITE_H
#define IMPL_SPRITE_H

#include <initializer_list>
#include <string>
#include <utility>

namespace m2g {
	enum class SpriteId {
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

	using SpriteIdToSpriteKeyLut = std::initializer_list<std::pair<SpriteId, const char*>>;
	constexpr SpriteIdToSpriteKeyLut sprite_id_to_sprite_key_lut{
		{SpriteId::Grass,     "main.grass"},
		{SpriteId::Player,    "main.player_lookdown_00"},
		{SpriteId::Skeleton,  "main.enemy_lookdown_00"},
		{SpriteId::Bullet0,   "main.bullet_00"},
		{SpriteId::Bullet1,   "main.bullet_01"},
		{SpriteId::Bomb,      "main.bomb_00"},
		{SpriteId::Sword,     "main.sword_00"},
		{SpriteId::RedPotion, "main.red_potion_00"}
	};
}

#endif //IMPL_SPRITE_H
