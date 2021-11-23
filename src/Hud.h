#ifndef HUD_H
#define HUD_H

#include "Game.h"
#include "HudWidget.h"
#include "Pool.h"
#include "Error.h"

typedef struct _Hud {
	void* levelBackPtr;
	Pool complications;
} Hud;

XErr Hud_Init(Hud *hud, void* level);

void Hud_Draw(Hud* hud, Game *game);

void Hud_Term(Hud *hud);

#endif
