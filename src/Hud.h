#ifndef HUD_H
#define HUD_H

typedef struct _Hud {
	void (*deinit)(struct _Hud*);
} Hud;

int HudInit(Hud *hud);
void HudDeinit(Hud *hud);

#endif
