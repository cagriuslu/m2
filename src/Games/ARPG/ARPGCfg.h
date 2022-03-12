#ifndef M2_ARPGCFG_H
#define M2_ARPGCFG_H

#include "../../Cfg.h"

typedef enum _ARPGCfgUIButtonType {
	ARPG_CFG_UI_BUTTONTYPE_NEWGAME = CFG_UI_BUTTONTYP_N,
	ARPG_CFG_UI_BUTTONTYPE_RESUME
} ARPGCfgUIButtonType;

extern const CfgUI ARPG_CFG_UI_ENTRYUI;
extern const CfgUI ARPG_CFG_UI_PAUSEUI;

#endif //M2_ARPGCFG_H
