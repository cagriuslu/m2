#ifndef CGAME_MARKUP_H
#define CGAME_MARKUP_H

#include "Cfg.h"
#include "Error.h"
#include <SDL.h>

XErr Markup_ExecuteBlocking(const CfgMarkup *markup);
XErr Markup_Draw_NoPresent(const CfgMarkup *markup, SDL_Rect rect);

// Utilities
SDL_Rect Markup_CalculateElementRect(SDL_Rect parentRect, unsigned parentW, unsigned parentH, unsigned childX, unsigned childY, unsigned childW, unsigned childH);

#endif //CGAME_MARKUP_H
