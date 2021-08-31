#ifndef HUD_H
#define HUD_H

#include "Pool.h"
#include "Error.h"

typedef struct _Hud Hud;

typedef struct _HudComplication {
	Hud* hud;
} HudComplication;

typedef struct _Hud {
	ID playerId;
} Hud;

XErr Hud_Init(Hud *hud, ID playerId);
void Hud_Term(Hud *hud);

void Hud_Draw(Hud* hud);

#endif
