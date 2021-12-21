#include "../Object.h"
#include "../Log.h"

int ObjectPlayer_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position);
int ObjectEnemy_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position);

int ObjectCharacter_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position) {
	switch (cfg->objType) {
		case CFG_OBJTYP_PLAYER:
			return ObjectPlayer_InitFromCfg(obj, cfg, position);
		case CFG_OBJTYP_ENEMY:
			return ObjectEnemy_InitFromCfg(obj, cfg, position);
		default:
			return LOGXV_ERR(XERR_OBJTYP_NOT_FOUND, Int32, cfg->objType);
	}
}
