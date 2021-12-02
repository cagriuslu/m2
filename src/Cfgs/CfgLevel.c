#include "../Cfg.h"

#define _ {.gndTile = &CFG_GNDTILE_GROUND000, .obj = NULL}
#define clfTop {.gndTile = &CFG_GNDTILE_CLIFF000T, .obj = NULL}
#define clfRig {.gndTile = &CFG_GNDTILE_CLIFF000R, .obj = NULL}
#define clfBot {.gndTile = &CFG_GNDTILE_CLIFF000B, .obj = NULL}
#define clfLef {.gndTile = &CFG_GNDTILE_CLIFF000L, .obj = NULL}
#define cTopRig {.gndTile = &CFG_GNDTILE_CLIFF000TR, .obj = NULL}
#define cTopLef {.gndTile = &CFG_GNDTILE_CLIFF000TL, .obj = NULL}
#define cBotRig {.gndTile = &CFG_GNDTILE_CLIFF000BR, .obj = NULL}
#define cBotLef {.gndTile = &CFG_GNDTILE_CLIFF000BL, .obj = NULL}

const LevelTile levelMap[] = {
	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,
	_,		clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	_,		_,		_,		_,
	_,		_,		_,		_,		_,		_,		clfRig,	_,		_,		_,
	_,		clfBot,	clfBot,	clfBot,	cBotLef,_,		clfRig,	_,		_,		_,
	_,		_,		_,		_,		clfLef,	_,		clfRig,	_,		_,		_,
	_,		_,		_,		_,		clfLef,	_,		clfRig,	_,		_,		_,
	_,		_,		_,		_,		clfLef,	_,		clfRig,	_,		_,		_,
	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,
	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,
	_,		_,		_,		_,		_,		_,		_,		_,		_,		_
};

const CfgLevel CFG_LVL_SP000 = {
		.tiles = levelMap,
		.w = 10,
		.h = 10
};