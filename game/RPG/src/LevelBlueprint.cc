#include <rpg/LevelBlueprint.h>
#include <m2g/SpriteBlueprint.h>

// clang-format off

#define _     {.bg_sprite_index = m2g::IMPL_SPRITE_GRASS,     .fg_sprite_index = m2g::IMPL_SPRITE_NONE}
#define cTop  {.bg_sprite_index = m2g::IMPL_SPRITE_CLIFF000T, .fg_sprite_index = m2g::IMPL_SPRITE_NONE}
#define cRig  {.bg_sprite_index = m2g::IMPL_SPRITE_CLIFF000R, .fg_sprite_index = m2g::IMPL_SPRITE_NONE}
#define cBot  {.bg_sprite_index = m2g::IMPL_SPRITE_CLIFF000B, .fg_sprite_index = m2g::IMPL_SPRITE_NONE}
#define cLef  {.bg_sprite_index = m2g::IMPL_SPRITE_CLIFF000L, .fg_sprite_index = m2g::IMPL_SPRITE_NONE}
#define _plyr {.bg_sprite_index = m2g::IMPL_SPRITE_GRASS,     .fg_sprite_index = m2g::IMPL_SPRITE_PLAYER_LOOKDOWN_00}
#define _skl  {.bg_sprite_index = m2g::IMPL_SPRITE_GRASS,     .fg_sprite_index = m2g::IMPL_SPRITE_ENEMY_LOOKDOWN_00}
#define _sklg {.bg_sprite_index = m2g::IMPL_SPRITE_GRASS,     .fg_sprite_index = m2g::IMPL_SPRITE_ENEMY_LOOKDOWN_00, .fg_object_group = lvl::CONSUMABLE_GROUP_HP}

const m2::TileBlueprint sp_000_tiles[] = {
        _,		cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	cTop,	_,
        cLef,	_plyr,	_,		_,		_,		_,		_,		_sklg,	_sklg,	_sklg,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_sklg,	_sklg,	_sklg,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_sklg,	_sklg,	_sklg,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_sklg,	_sklg,	_sklg,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_sklg,	_sklg,	_sklg,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_sklg,	_sklg,	_sklg,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_sklg,	_sklg,	_sklg,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_sklg,	_sklg,	_sklg,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        cLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		cRig,
        _,		cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	cBot,	_,
};
const m2::LevelBlueprint lvl::sp_000 = {
        .w = 32,
        .h = 32,
        .tiles = sp_000_tiles
};
