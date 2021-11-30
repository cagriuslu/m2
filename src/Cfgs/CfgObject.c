#include "../Cfg.h"

const CfgObject CFG_OBJ_BOX000 = {
	.textureRect = {24, 96, 24, 24},
	.objCenter_px = {0.0f, -2.0f}, {0.0f, -2.0f / CFG_TILE_SIZE},
	.collider = {
		.type = COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {22.0f, 9.0f}, {22.0f / CFG_TILE_SIZE, 9.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgObject CFG_OBJ_SWORD000 = {
	.textureRect = {144, 96, 48, 24},
	.objCenter_px = {-14.0f, 0.0f}, {-14.0f / CFG_TILE_SIZE, 0.0f},
	.collider = {
		.type = COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {15.5f, 0.0f}, {15.5f / CFG_TILE_SIZE, 0.0f},
			.dims_px = {25.0f, 4.0f}, {25.0f / CFG_TILE_SIZE, 4.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgObject CFG_OBJ_SKELETON000 = {
	.textureRect = {48, 0, 24, 24},
	.objCenter_px = {0.0f, 5.0f}, {0.0f, 5.0f / CFG_TILE_SIZE},
	.collider = {
		.type = COLLIDER_TYPE_CIRCLE,
		.colliderUnion.circ = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.radius_px = 6.0f, 6.0f / CFG_TILE_SIZE
		}
	}
};
const CfgObject CFG_OBJ_PLAYER000 = {
	.textureRect = {72, 0, 24, 24},
	.objCenter_px = {0.0f, 6.0f}, {0.0f, 6.0f / CFG_TILE_SIZE},
	.collider = {
		.type = COLLIDER_TYPE_CIRCLE,
		.colliderUnion.circ = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.radius_px = 6.0f, 6.0f / CFG_TILE_SIZE
		}
	}
};
const CfgObject CFG_OBJ_ARROW000 = {
	.textureRect = {72, 96, 24, 24},
	.objCenter_px = {2.5f, 0.5f}, {2.5f / CFG_TILE_SIZE, 0.5f / CFG_TILE_SIZE},
	.collider = {
		.type = COLLIDER_TYPE_CIRCLE,
		.colliderUnion.circ = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.radius_px = 1.5f, 1.5f / CFG_TILE_SIZE
		}
	}
};
const CfgObject CFG_OBJ_BULLET000 = {
	.textureRect = {96, 96, 24, 24},
	.objCenter_px = {1.5f, 0.5f}, {1.5f / CFG_TILE_SIZE, 0.5f / CFG_TILE_SIZE},
	.collider = {
		.type = COLLIDER_TYPE_CIRCLE,
		.colliderUnion.circ = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.radius_px = 1.5f, 1.5f / CFG_TILE_SIZE
		}
	}
};
const CfgObject CFG_OBJ_BULLET001 = {
	.textureRect = {120, 96, 24, 24},
	.objCenter_px = {3.5f, 0.5f}, {3.5f / CFG_TILE_SIZE, 0.5f / CFG_TILE_SIZE},
	.collider = {
		.type = COLLIDER_TYPE_CIRCLE,
		.colliderUnion.circ = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.radius_px = 1.5f, 1.5f / CFG_TILE_SIZE
		}
	}
};
