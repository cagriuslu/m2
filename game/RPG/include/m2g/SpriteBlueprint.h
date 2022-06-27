#ifndef IMPL_SPRITEBLUEPRINT_H
#define IMPL_SPRITEBLUEPRINT_H

#include <m2/Object.h>
#include <m2/SpriteBlueprint.h>
#include <m2/LevelBlueprint.h>
#include <m2/Value.h>

namespace m2g {
    constexpr unsigned tile_width = 48;
    constexpr unsigned tile_height = 48;
    constexpr float tile_width_f = static_cast<float>(tile_width);
    constexpr float tile_height_f = static_cast<float>(tile_height);

    enum SpriteIndex : m2::SpriteIndex {
        IMPL_SPRITE_NONE = 0,
        IMPL_SPRITE_DEFAULT,
        IMPL_SPRITE_GRASS,
        IMPL_SPRITE_CLIFF000T,
        IMPL_SPRITE_CLIFF000R,
        IMPL_SPRITE_CLIFF000B,
        IMPL_SPRITE_CLIFF000L,
        IMPL_SPRITE_CLIFF000TR,
        IMPL_SPRITE_CLIFF000TL,
        IMPL_SPRITE_CLIFF000BR,
        IMPL_SPRITE_CLIFF000BL,
	    IMPL_SPRITE_PLAYER_LOOKDOWN_00,
	    IMPL_SPRITE_ENEMY_LOOKDOWN_00,
	    IMPL_SPRITE_RED_POTION_00,

		IMPL_EDITOR_SPRITE_N, // Sprites before this point will be exposed to editor

        IMPL_SPRITE_PLAYER_LOOKDOWN_01,
        IMPL_SPRITE_PLAYER_LOOKDOWN_02,
        IMPL_SPRITE_PLAYER_LOOKLEFT_00,
        IMPL_SPRITE_PLAYER_LOOKLEFT_01,
        IMPL_SPRITE_PLAYER_LOOKLEFT_02,
        IMPL_SPRITE_PLAYER_LOOKRIGHT_00,
        IMPL_SPRITE_PLAYER_LOOKRIGHT_01,
        IMPL_SPRITE_PLAYER_LOOKRIGHT_02,
        IMPL_SPRITE_PLAYER_LOOKUP_00,
        IMPL_SPRITE_PLAYER_LOOKUP_01,
        IMPL_SPRITE_PLAYER_LOOKUP_02,

        IMPL_SPRITE_ENEMY_LOOKDOWN_01,
        IMPL_SPRITE_ENEMY_LOOKDOWN_02,
        IMPL_SPRITE_ENEMY_LOOKLEFT_00,
        IMPL_SPRITE_ENEMY_LOOKLEFT_01,
        IMPL_SPRITE_ENEMY_LOOKLEFT_02,
        IMPL_SPRITE_ENEMY_LOOKRIGHT_00,
        IMPL_SPRITE_ENEMY_LOOKRIGHT_01,
        IMPL_SPRITE_ENEMY_LOOKRIGHT_02,
        IMPL_SPRITE_ENEMY_LOOKUP_00,
        IMPL_SPRITE_ENEMY_LOOKUP_01,
        IMPL_SPRITE_ENEMY_LOOKUP_02,

        IMPL_SPRITE_BULLET_00,
        IMPL_SPRITE_BULLET_01,
        IMPL_SPRITE_SWORD_00,
        IMPL_SPRITE_BOMB_00,
		IMPL_SPRITE_FLOPPY_DISK,

        IMPL_SPRITE_N
    };

    extern const m2::SpriteBlueprint sprites[];
	extern const unsigned editor_sprite_count;
    extern const unsigned sprite_count;
	m2::VoidValue fg_sprite_loader(m2::Object& obj, m2::SpriteIndex index, m2::GroupID gid, m2::Vec2f position);
}

#endif //IMPL_SPRITEBLUEPRINT_H
