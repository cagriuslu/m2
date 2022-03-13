#ifndef M2_ARPG_CFG_H
#define M2_ARPG_CFG_H

#include "../../Cfg.h"

#define ARPG_CFG_TILE_SIZE (48)
#define ARPG_CFG_TILE_SIZE_F (48.0f)
#define ARPG_CFG_TEXTURE_FILE "resources/48.png"
#define ARPG_CFG_TEXTURE_MASK_FILE "resources/48-Mask.png"

typedef enum _ARPG_CfgUIButtonType {
	ARPG_CFG_UI_BUTTONTYPE_NEWGAME = CFG_UI_BUTTONTYP_N,
	ARPG_CFG_UI_BUTTONTYPE_RESUME
} ARPG_CfgUIButtonType;

extern const CfgUI ARPG_CFG_UI_ENTRYUI;
extern const CfgUI ARPG_CFG_UI_PAUSEUI;

#endif //M2_ARPG_CFG_H
