#ifndef CGAME_UI_H
#define CGAME_UI_H

#include "m2/Event.hh"
#include "m2/Cfg.hh"
#include "m2/Def.hh"
#include <SDL.h>

M2Err UIState_Init(UIState *state, const CfgUI* cfg);
M2Err UIState_UpdatePositions(UIState* state, SDL_Rect rootRect);
M2Err UIState_UpdateElements(UIState* state);

bool UIState_HandleEvents(UIState *state, Events *evs, CfgUIButtonType *outPressedButton);
M2Err UIState_Draw(UIState *state);

void UIState_Term(UIState *state);

// High-level functions
M2Err UI_ExecuteBlocking(const CfgUI *ui, CfgUIButtonType* outPressedButton);

// Helper functions
SDL_Texture* UI_GenerateFontTexture(const char* text);

#endif //CGAME_UI_H
