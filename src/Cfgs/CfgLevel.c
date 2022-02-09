#include "../Cfg.h"

#define _ {.gndTile = &CFG_GNDTXTR_GRASS, .chr = NULL}
#define clfTop {.gndTile = &CFG_GNDTXTR_CLIFF000T, .chr = NULL}
#define clfRig {.gndTile = &CFG_GNDTXTR_CLIFF000R, .chr = NULL}
#define clfBot {.gndTile = &CFG_GNDTXTR_CLIFF000B, .chr = NULL}
#define clfLef {.gndTile = &CFG_GNDTXTR_CLIFF000L, .chr = NULL}
#define cTopRig {.gndTile = &CFG_GNDTXTR_CLIFF000TR, .chr = NULL}
#define cTopLef {.gndTile = &CFG_GNDTXTR_CLIFF000TL, .chr = NULL}
#define cBotRig {.gndTile = &CFG_GNDTXTR_CLIFF000BR, .chr = NULL}
#define cBotLef {.gndTile = &CFG_GNDTXTR_CLIFF000BL, .chr = NULL}

#define _plyr {.gndTile = &CFG_GNDTXTR_GRASS, .chr = &CFG_CHARACTER_PLAYER }
#define _skl  {.gndTile = &CFG_GNDTXTR_GRASS, .chr = &CFG_CHARACTER_SKELETON_000_CHASE}

const CfgLevelTile sp000_tiles[] = {
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
	_,		clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	clfBot,	_,
};
const CfgLevel CFG_LVL_SP000 = {
		.tiles = sp000_tiles,
		.w = 32,
		.h = 32
};
