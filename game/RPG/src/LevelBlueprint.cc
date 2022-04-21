#include <impl/private/LevelBlueprint.h>
#include <impl/public/SpriteBlueprint.h>

#define _       {.bg_sprite_index = impl::IMPL_SPRITE_GRASS,     .fg_sprite_index = impl::IMPL_SPRITE_NONE}
#define clfTop  {.bg_sprite_index = impl::IMPL_SPRITE_CLIFF000T, .fg_sprite_index = impl::IMPL_SPRITE_NONE}
#define clfRig  {.bg_sprite_index = impl::IMPL_SPRITE_CLIFF000R, .fg_sprite_index = impl::IMPL_SPRITE_NONE}
#define clfBot  {.bg_sprite_index = impl::IMPL_SPRITE_CLIFF000B, .fg_sprite_index = impl::IMPL_SPRITE_NONE}
#define clfLef  {.bg_sprite_index = impl::IMPL_SPRITE_CLIFF000L, .fg_sprite_index = impl::IMPL_SPRITE_NONE}
#define _plyr   {.bg_sprite_index = impl::IMPL_SPRITE_GRASS,     .fg_sprite_index = impl::IMPL_SPRITE_PLAYER_LOOKDOWN_00}
#define _skl    {.bg_sprite_index = impl::IMPL_SPRITE_GRASS,     .fg_sprite_index = impl::IMPL_SPRITE_ENEMY_LOOKDOWN_00}

const m2::TileBlueprint sp_000_tiles[] = {
        _,		clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	_,
        clfLef,	_plyr,	_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        clfLef,	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_,		_skl,	_skl,	_skl,	_,		_,		_,		_,		_,		_,		clfRig,
        _,		clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	_,
};
const m2::LevelBlueprint impl::level::sp_000 = {
        .w = 32,
        .h = 32,
        .tiles = sp_000_tiles
};
