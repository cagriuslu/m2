#ifndef CGAME_MARKUP_H
#define CGAME_MARKUP_H

#include "Event.h"
#include "Cfg.h"
#include "Def.h"
#include <SDL.h>

M2Err MarkupState_Init(MarkupState *state, const CfgMarkup* cfg);
M2Err MarkupState_UpdatePositions(MarkupState* state, SDL_Rect rootRect);
M2Err MarkupState_UpdateElements(MarkupState* state);

bool MarkupState_HandleEvents(MarkupState *state, Events *evs, CfgMarkupButtonType *outPressedButton);
M2Err MarkupState_Draw(MarkupState *state);

void MarkupState_Term(MarkupState *state);

// High-level functions
M2Err Markup_ExecuteBlocking(const CfgMarkup *markup, CfgMarkupButtonType* outPressedButton);

// Helper functions
SDL_Texture* Markup_GenerateFontTexture(const char* text);

#endif //CGAME_MARKUP_H
