#include "../Cfg.h"

const CfgGroundTexture CFG_GNDTXTR_DEFAULT = {
	.textureRect = {96, 384, CFG_TILE_SIZE, CFG_TILE_SIZE},
	.collider = {
		.type = CFG_COLLIDER_TYPE_INVALID
	}
};
const CfgGroundTexture CFG_GNDTXTR_GRASS = {
	.textureRect = {336, 0, CFG_TILE_SIZE, CFG_TILE_SIZE},
	.collider = {
		.type = CFG_COLLIDER_TYPE_INVALID
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000T = {
	.textureRect = {96, 0, CFG_TILE_SIZE, CFG_TILE_SIZE},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.dims_px = {CFG_TILE_SIZE_F, CFG_TILE_SIZE_F}, {1.0f, 1.0f}
		}
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000R = {
	.textureRect = {96, 48, CFG_TILE_SIZE, CFG_TILE_SIZE},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.dims_px = {CFG_TILE_SIZE_F, CFG_TILE_SIZE_F}, {1.0f, 1.0f}
		}
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000B = {
	.textureRect = {96, 96, CFG_TILE_SIZE, CFG_TILE_SIZE},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.dims_px = {CFG_TILE_SIZE_F, CFG_TILE_SIZE_F}, {1.0f, 1.0f}
		}
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000L = {
	.textureRect = {96, 144, CFG_TILE_SIZE, CFG_TILE_SIZE},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.dims_px = {CFG_TILE_SIZE_F, CFG_TILE_SIZE_F}, {1.0f, 1.0f}
		}
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000TR = {
	.textureRect = {96, 192, CFG_TILE_SIZE, CFG_TILE_SIZE},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.dims_px = {CFG_TILE_SIZE_F, CFG_TILE_SIZE_F}, {1.0f, 1.0f}
		}
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000BL = {
		.textureRect = {96, 240, CFG_TILE_SIZE, CFG_TILE_SIZE},
		.collider = {
				.type = CFG_COLLIDER_TYPE_RECTANGLE,
				.colliderUnion.rect = {
						.dims_px = {CFG_TILE_SIZE_F, CFG_TILE_SIZE_F}, {1.0f, 1.0f}
				}
		}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000TL = {
	.textureRect = {96, 288, CFG_TILE_SIZE, CFG_TILE_SIZE},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.dims_px = {CFG_TILE_SIZE_F, CFG_TILE_SIZE_F}, {1.0f, 1.0f}
		}
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000BR = {
	.textureRect = {96, 336, CFG_TILE_SIZE, CFG_TILE_SIZE},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.dims_px = {CFG_TILE_SIZE_F, CFG_TILE_SIZE_F}, {1.0f, 1.0f}
		}
	}
};
