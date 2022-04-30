#include <m2g/SpriteBlueprint.h>
#include <rpg/object/Enemy.h>
#include <rpg/object/Player.h>

const m2::SpriteBlueprint m2g::sprites[]{
	{
		.index = IMPL_SPRITE_NONE
	},
	{
		.index = IMPL_SPRITE_DEFAULT,
		.texture_rect = {96, 384, tile_width, tile_height},
	},
	{
		.index = IMPL_SPRITE_GRASS,
		.texture_rect = {336, 0, tile_width, tile_height},
	},
	{
		.index = IMPL_SPRITE_CLIFF000T,
		.texture_rect = {96, 0, tile_width, tile_height},
		.collider = {
			.variant = m2::ColliderBlueprint::Rectangle{
				.dims_px = {tile_width_f, tile_height_f},
				.dims_m = {1.0f, 1.0f}
			}
		}
	},
	{
		.index = IMPL_SPRITE_CLIFF000R,
		.texture_rect = {96, 48, tile_width, tile_height},
		.collider = {
			.variant = m2::ColliderBlueprint::Rectangle{
				.dims_px = {tile_width_f, tile_height_f}, .dims_m = {1.0f, 1.0f}
			}
		}
	},
	{
		.index = IMPL_SPRITE_CLIFF000B,
		.texture_rect = {96, 96, tile_width, tile_height},
		.collider = {
			.variant = m2::ColliderBlueprint::Rectangle{
				.dims_px = {tile_width_f, tile_height_f}, .dims_m= {1.0f, 1.0f}
			}
		}
	},
	{
		.index = IMPL_SPRITE_CLIFF000L,
		.texture_rect = {96, 144, tile_width, tile_height},
		.collider = {
			.variant = m2::ColliderBlueprint::Rectangle{
				.dims_px = {tile_width_f, tile_height_f}, .dims_m= {1.0f, 1.0f}
			}
		}
	},
	{
		.index = IMPL_SPRITE_CLIFF000TR,
		.texture_rect = {96, 192, tile_width, tile_height},
		.collider = {
			.variant = m2::ColliderBlueprint::Rectangle{
				.dims_px = {tile_width_f, tile_height_f}, .dims_m= {1.0f, 1.0f}
			}
		}
	},
	{
		.index = IMPL_SPRITE_CLIFF000TL,
		.texture_rect = {96, 240, tile_width, tile_height},
		.collider = {
			.variant = m2::ColliderBlueprint::Rectangle{
				.dims_px = {tile_width_f, tile_height_f}, .dims_m={1.0f, 1.0f}
			}
		}
	},
	{
		.index = IMPL_SPRITE_CLIFF000BR,
		.texture_rect = {96, 288, tile_width, tile_height},
		.collider = {
			.variant = m2::ColliderBlueprint::Rectangle{
				.dims_px = {tile_width_f, tile_height_f}, .dims_m= {1.0f, 1.0f}
			}
		}
	},
	{
		.index = IMPL_SPRITE_CLIFF000BL,
		.texture_rect = {96, 336, tile_width, tile_height},
		.collider = {
			.variant = m2::ColliderBlueprint::Rectangle{
				.dims_px = {tile_width_f, tile_height_f}, .dims_m= {1.0f, 1.0f}
			}
		}
	},

	{
		.index = IMPL_SPRITE_PLAYER_LOOKDOWN_00,
		.texture_rect = {0, 0, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m= {-0.5f / tile_width_f, 12.0f / tile_height_f},
		.collider = {
			.center_px = {0.0f, 0.0f}, .center_m= {0.0f, 0.0f},
			.variant = m2::ColliderBlueprint::Circle{
				.radius_px = 12.0f, .radius_m = 12.0f / tile_width_f
			}
		}
	},
	{
		.index = IMPL_SPRITE_PLAYER_LOOKDOWN_01,
		.texture_rect = {0, 48, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_PLAYER_LOOKDOWN_02,
		.texture_rect = {0, 96, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_PLAYER_LOOKLEFT_00,
		.texture_rect = {0, 144, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_PLAYER_LOOKLEFT_01,
		.texture_rect = {0, 192, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_PLAYER_LOOKLEFT_02,
		.texture_rect = {0, 240, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_PLAYER_LOOKRIGHT_00,
		.texture_rect = {0, 288, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_PLAYER_LOOKRIGHT_01,
		.texture_rect = {0, 336, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_PLAYER_LOOKRIGHT_02,
		.texture_rect = {0, 384, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_PLAYER_LOOKUP_00,
		.texture_rect = {0, 432, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_PLAYER_LOOKUP_01,
		.texture_rect = {0, 480, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_PLAYER_LOOKUP_02,
		.texture_rect = {0, 528, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_ENEMY_LOOKDOWN_00,
		.texture_rect = {48, 0, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f},
		.collider = {
			.center_px = {0.0f, 0.0f}, .center_m = {0.0f, 0.0f},
			.variant = m2::ColliderBlueprint::Circle{
				.radius_px = 12.0f, .radius_m = 12.0f / tile_width_f
			}
		}
	},
	{
		.index = IMPL_SPRITE_ENEMY_LOOKDOWN_01,
		.texture_rect = {48, 48, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_ENEMY_LOOKDOWN_02,
		.texture_rect = {48, 96, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_ENEMY_LOOKLEFT_00,
		.texture_rect = {48, 144, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_ENEMY_LOOKLEFT_01,
		.texture_rect = {48, 192, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_ENEMY_LOOKLEFT_02,
		.texture_rect = {48, 240, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_ENEMY_LOOKRIGHT_00,
		.texture_rect = {48, 288, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_ENEMY_LOOKRIGHT_01,
		.texture_rect = {48, 336, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_ENEMY_LOOKRIGHT_02,
		.texture_rect = {48, 384, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_ENEMY_LOOKUP_00,
		.texture_rect = {48, 432, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_ENEMY_LOOKUP_01,
		.texture_rect = {48, 480, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_ENEMY_LOOKUP_02,
		.texture_rect = {48, 528, 48, 48},
		.obj_center_px = {-0.5f, 12.0f}, .obj_center_m = {-0.5f / tile_width_f, 12.0f / tile_height_f}
	},
	{
		.index = IMPL_SPRITE_BULLET_00,
		.texture_rect = {144, 0, 48, 48},
		.obj_center_px = {0.0f, 0.0f}, .obj_center_m = {0.0f / tile_width_f, 0.0f / tile_height_f},
		.collider = {
			.center_px = {0.0f, 0.0f}, .center_m = {0.0f, 0.0f},
			.variant = m2::ColliderBlueprint::Circle{
				.radius_px = 3.0f, .radius_m = 3.0f / tile_width_f
			}
		}
	},
	{
		.index = IMPL_SPRITE_BULLET_01,
		.texture_rect = {144, 48, 48, 48},
		.obj_center_px = {0.0f, 0.0f}, .obj_center_m = {0.0f / tile_width_f, 0.0f / tile_height_f},
		.collider = {
			.center_px = {0.0f, 0.0f}, .center_m = {0.0f, 0.0f},
			.variant = m2::ColliderBlueprint::Circle{
				.radius_px = 3.0f, .radius_m = 3.0f / tile_width_f
			}
		}
	},
	{
		.index = IMPL_SPRITE_SWORD_00,
		.texture_rect = {192, 0, 96, 48},
		.obj_center_px = {-24.0f, 0.0f}, .obj_center_m = {-24.0f / tile_width_f, 0.0f / tile_height_f},
		.collider = {
			.center_px = {24.0f, 0.0f}, .center_m = {24.0f, 0.0f},
			.variant = m2::ColliderBlueprint::Rectangle{
				.dims_px = {54.0f, 8.0f}, .dims_m = {54.0f / tile_width_f, 8.0f / tile_height_f}
			}
		}
	},
	{
		.index = IMPL_SPRITE_BOMB_00,
		.texture_rect = {288, 48, 48, 48},
		.obj_center_px = {0.0f, 0.0f}, .obj_center_m = {0.0f / tile_width_f, 0.0f / tile_height_f},
		.collider = {
			.center_px = {0.0f, 0.0f}, .center_m = {0.0f, 0.0f},
			.variant = m2::ColliderBlueprint::Circle{
				.radius_px = 6.0f, .radius_m = 6.0f / tile_width_f
			}
		}
	},
	{
		.index = IMPL_SPRITE_RED_POTION_00,
		.texture_rect = {144, 96, 48, 48},
		.collider = {
			.variant = m2::ColliderBlueprint::Circle{
				.radius_px = 6.0f, .radius_m = 6.0f / tile_width_f
			}
		}
	},
	{
		.index = IMPL_SPRITE_FLOPPY_DISK,
		.texture_rect = {144, 144, 48, 48}
	}
};

const unsigned m2g::sprite_count = IMPL_SPRITE_N;

M2Err m2g::fg_sprite_loader(m2::Object &obj, m2::SpriteIndex index, m2::GroupID group_id, m2::Vec2f position) {
	using namespace obj;
	switch (index) {
		case IMPL_SPRITE_PLAYER_LOOKDOWN_00:
			return Player::init(obj, &chr::character_player, position);
		case IMPL_SPRITE_ENEMY_LOOKDOWN_00:
			return Enemy::init(obj, &chr::character_skeleton_000_chase, group_id, position);
		default:
			return LOG_ERROR_M2V(M2ERR_INVALID_CFG_OBJTYP, Int32, index);
	}
}
