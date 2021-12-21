#include "../Cfg.h"

const CfgGroundTexture CFG_GNDTXTR_DEFAULT = {
	.textureRect = {0, 0, 0, 0},
	.collider = {
		.type = CFG_COLLIDER_TYPE_NONE
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000T = {
	.textureRect = {0, 48, 24, 24},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000R = {
	.textureRect = {24, 48, 24, 24},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000B = {
	.textureRect = {24, 72, 24, 24},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000L = {
	.textureRect = {0, 72, 24, 24},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000TR = {
	.textureRect = {48, 48, 24, 24},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000TL = {
	.textureRect = {72, 72, 24, 24},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000BR = {
	.textureRect = {48, 72, 24, 24},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgGroundTexture CFG_GNDTXTR_CLIFF000BL = {
	.textureRect = {72, 48, 24, 24},
	.collider = {
		.type = CFG_COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
