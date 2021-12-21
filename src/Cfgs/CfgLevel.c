#include "../Cfg.h"

#define _ {.gndTile = &CFG_GNDTXTR_DEFAULT, .chr = NULL}
#define clfTop {.gndTile = &CFG_GNDTXTR_CLIFF000T, .chr = NULL}
#define clfRig {.gndTile = &CFG_GNDTXTR_CLIFF000R, .chr = NULL}
#define clfBot {.gndTile = &CFG_GNDTXTR_CLIFF000B, .chr = NULL}
#define clfLef {.gndTile = &CFG_GNDTXTR_CLIFF000L, .chr = NULL}
#define cTopRig {.gndTile = &CFG_GNDTXTR_CLIFF000TR, .chr = NULL}
#define cTopLef {.gndTile = &CFG_GNDTXTR_CLIFF000TL, .chr = NULL}
#define cBotRig {.gndTile = &CFG_GNDTXTR_CLIFF000BR, .chr = NULL}
#define cBotLef {.gndTile = &CFG_GNDTXTR_CLIFF000BL, .chr = NULL}

#define _plyr {.gndTile = &CFG_GNDTXTR_DEFAULT, .chr = &CFG_CHARACTER_PLAYER }
#define _skl  {.gndTile = &CFG_GNDTXTR_DEFAULT, .chr = &CFG_CHARACTER_SKELETON000}

const CfgLevelTile sp000_tiles[] = {
	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,
	_,		clfTop,	clfTop,	clfTop,	clfTop,	clfTop,	_,		_,		_,		_,
	_plyr,	_,		_,		_,		_,		_,		clfRig,	_,		_,		_,
	_,		clfBot,	clfBot,	clfBot,	cBotLef,_,		clfRig,	_,		_,		_,
	_,		_,		_,		_,		clfLef,	_,		clfRig,	_,		_,		_,
	_,		_,		_,		_,		clfLef,	_,		clfRig,	_,		_,		_,
	_,		_,		_,		_,		clfLef,	_,		clfRig,	_,		_,		_,
	_,		_,		_,		_,		_,		_,		_,		_,		_,		_,
	_,		_,		_,		_,		_skl,	_,		_skl,	_,		_,		_,
	_,		_,		_,		_,		_,		_,		_,		_,		_,		_
};
const CfgLevel CFG_LVL_SP000 = {
		.tiles = sp000_tiles,
		.w = 10,
		.h = 10
};
#undef _
#undef clfTop
#undef clfRig
#undef clfBot
#undef clfLef
#undef cTopRig
#undef cTopLef
#undef cBotRig
#undef cBotLef
