#include "../Object.h"
#include "../Def.h"

int ObjectPlayer_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position);
int ObjectEnemy_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position);

int ObjectCharacter_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position) {
	switch (cfg->objType) {
		case CFG_OBJTYP_PLAYER:
			return ObjectPlayer_InitFromCfg(obj, cfg, position);
		case CFG_OBJTYP_ENEMY:
			return ObjectEnemy_InitFromCfg(obj, cfg, position);
		default:
			return LOGXV_ERR(XERR_INVALID_CFG_OBJTYP, Int32, cfg->objType);
	}
}
