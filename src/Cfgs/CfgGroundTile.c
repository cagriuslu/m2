#include "../Cfg.h"

const CfgGroundTile CFG_GNDTILE_DEFAULT = {
	.textureRect = {0, 0, 0, 0},
	.collider = {
		.type = COLLIDER_TYPE_NONE
	}
};
const CfgGroundTile CFG_GNDTILE_GROUND000 = {
	.textureRect = {0, 0, 24, 24},
	.collider = {
		.type = COLLIDER_TYPE_NONE
	}
};
const CfgGroundTile CFG_GNDTILE_CLIFF000T = {
	.textureRect = {0, 48, 24, 24},
	.collider = {
		.type = COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgGroundTile CFG_GNDTILE_CLIFF000R = {
	.textureRect = {24, 48, 24, 24},
	.collider = {
		.type = COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgGroundTile CFG_GNDTILE_CLIFF000B = {
	.textureRect = {24, 72, 24, 24},
	.collider = {
		.type = COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgGroundTile CFG_GNDTILE_CLIFF000L = {
	.textureRect = {0, 72, 24, 24},
	.collider = {
		.type = COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgGroundTile CFG_GNDTILE_CLIFF000TR = {
	.textureRect = {48, 48, 24, 24},
	.collider = {
		.type = COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgGroundTile CFG_GNDTILE_CLIFF000TL = {
	.textureRect = {72, 72, 24, 24},
	.collider = {
		.type = COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgGroundTile CFG_GNDTILE_CLIFF000BR = {
	.textureRect = {48, 72, 24, 24},
	.collider = {
		.type = COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
const CfgGroundTile CFG_GNDTILE_CLIFF000BL = {
	.textureRect = {72, 48, 24, 24},
	.collider = {
		.type = COLLIDER_TYPE_RECTANGLE,
		.colliderUnion.rect = {
			.center_px = {0.0f, 0.0f}, {0.0f, 0.0f},
			.dims_px = {24.0f, 24.0f}, {24.0f / CFG_TILE_SIZE, 24.0f / CFG_TILE_SIZE}
		}
	}
};
