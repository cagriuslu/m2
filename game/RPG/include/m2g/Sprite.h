#ifndef IMPL_SPRITE_H
#define IMPL_SPRITE_H

#include <initializer_list>
#include <string>
#include <utility>

namespace m2g {
	enum class Sprite {
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

	constexpr std::initializer_list<std::pair<Sprite, const char*>> sprite_lut{
		{Sprite::Grass, "main.grass"},
		{Sprite::Player, "main.player_lookdown_00"},
		{Sprite::Skeleton, "main.enemy_lookdown_00"},
		{Sprite::Bullet0, "main.bullet_00"},
		{Sprite::Bullet1, "main.bullet_01"},
		{Sprite::Bomb, "main.bomb_00"},
		{Sprite::Sword, "main.sword_00"},
		{Sprite::RedPotion, "main.red_potion_00"}
	};
}

#endif //IMPL_SPRITE_H
