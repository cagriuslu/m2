#include "../Cfg.h"

const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKDOWN_00 = {
	.textureRect = {0, 0, 48, 48},
	.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE},
	.collider = {
		.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
		.type = CFG_COLLIDER_TYPE_CIRCLE,
		.colliderUnion.circ = {
				.radius_px = 12.0f, 12.0f / CFG_TILE_SIZE
		}
	}
};
const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKDOWN_01 = {
		.textureRect = {0, 48, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKDOWN_02 = {
		.textureRect = {0, 96, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKLEFT_00 = {
		.textureRect = {0, 144, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKLEFT_01 = {
		.textureRect = {0, 192, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKLEFT_02 = {
		.textureRect = {0, 240, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKRIGHT_00 = {
		.textureRect = {0, 288, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKRIGHT_01 = {
		.textureRect = {0, 336, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKRIGHT_02 = {
		.textureRect = {0, 384, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKUP_00 = {
		.textureRect = {0, 432, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKUP_01 = {
		.textureRect = {0, 480, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_PLAYER_LOOKUP_02 = {
		.textureRect = {0, 528, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};

const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKDOWN_00 = {
		.textureRect = {48, 0, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE},
		.collider = {
				.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
				.type = CFG_COLLIDER_TYPE_CIRCLE,
				.colliderUnion.circ = {
						.radius_px = 12.0f, 12.0f / CFG_TILE_SIZE
				}
		}
};
const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKDOWN_01 = {
		.textureRect = {48, 48, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKDOWN_02 = {
		.textureRect = {48, 96, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKLEFT_00 = {
		.textureRect = {48, 144, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKLEFT_01 = {
		.textureRect = {48, 192, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKLEFT_02 = {
		.textureRect = {48, 240, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKRIGHT_00 = {
		.textureRect = {48, 288, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKRIGHT_01 = {
		.textureRect = {48, 336, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKRIGHT_02 = {
		.textureRect = {48, 384, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKUP_00 = {
		.textureRect = {48, 432, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKUP_01 = {
		.textureRect = {48, 480, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};
const CfgObjectTexture CFG_OBJTXTR_ENEMY_LOOKUP_02 = {
		.textureRect = {48, 528, 48, 48},
		.objCenter_px = {-0.5f, 12.0f}, {-0.5f / CFG_TILE_SIZE, 12.0f / CFG_TILE_SIZE}
};

const CfgObjectTexture CFG_OBJTXTR_BULLET_00 = {
		.textureRect = {144, 0, 48, 48},
		.objCenter_px = {0.0f, 0.0f}, {0.0f / CFG_TILE_SIZE, 0.0f / CFG_TILE_SIZE},
		.collider = {
				.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
				.type = CFG_COLLIDER_TYPE_CIRCLE,
				.colliderUnion.circ = {
						.radius_px = 3.0f, 3.0f / CFG_TILE_SIZE
				}
		}
};
const CfgObjectTexture CFG_OBJTXTR_BULLET_01 = {
		.textureRect = {144, 48, 48, 48},
		.objCenter_px = {0.0f, 0.0f}, {0.0f / CFG_TILE_SIZE, 0.0f / CFG_TILE_SIZE},
		.collider = {
				.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
				.type = CFG_COLLIDER_TYPE_CIRCLE,
				.colliderUnion.circ = {
						.radius_px = 3.0f, 3.0f / CFG_TILE_SIZE
				}
		}
};
const CfgObjectTexture CFG_OBJTXTR_SWORD_00 = {
		.textureRect = {192, 0, 96, 48},
		.objCenter_px = {-24.0f, 0.0f}, {-24.0f / CFG_TILE_SIZE, 0.0f / CFG_TILE_SIZE},
		.collider = {
				.center_px = {24.0f, 0.0f}, {24.0f, 0.0f},
				.type = CFG_COLLIDER_TYPE_RECTANGLE,
				.colliderUnion.rect = {
						.dims_px = {54.0f, 8.0f}, {54.0f / CFG_TILE_SIZE, 8.0f / CFG_TILE_SIZE}
				}
		}
};
const CfgObjectTexture CFG_OBJTXTR_BOMB_00 = {
		.textureRect = {288, 48, 48, 48},
		.objCenter_px = {0.0f, 0.0f}, {0.0f / CFG_TILE_SIZE, 0.0f / CFG_TILE_SIZE},
		.collider = {
				.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
				.type = CFG_COLLIDER_TYPE_CIRCLE,
				.colliderUnion.circ = {
						.radius_px = 6.0f, 6.0f / CFG_TILE_SIZE
				}
		}
};
