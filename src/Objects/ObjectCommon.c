#include "../Object.h"

int Object_InitFromCfg_Player(Object *obj, const CfgObject *cfg, Vec2F position) {
	// TODO
	return 0;
}

static int (*Object_InitFromCfg_FunctionTable[])(Object *obj, const CfgObject *cfg, Vec2F position) = {
	NULL,
	Object_InitFromCfg_Player,
	ObjectEnemy_InitFromCfg
};

int Object_InitFromCfg(Object *obj, const CfgObject *cfg, Vec2F position) {
	return cfg->id ? Object_InitFromCfg_FunctionTable[cfg->id](obj, cfg, position) : 0;
}
