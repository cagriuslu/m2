#ifndef CGAME_MARKUP_H
#define CGAME_MARKUP_H

#include "Event.h"
#include "Cfg.h"
#include "Error.h"
#include <SDL.h>

XErr MarkupState_Init(MarkupState *state, const CfgMarkup* cfg);
XErr MarkupState_Update(MarkupState* state, SDL_Rect rootRect);

bool MarkupState_HandleEvents(MarkupState *state, Events *evs, CfgMarkupButtonType *outPressedButton);

void MarkupState_Term(MarkupState *state);

// High-level functions
XErr Markup_ExecuteBlocking(const CfgMarkup *markup, CfgMarkupButtonType* outPressedButton);

#endif //CGAME_MARKUP_H
