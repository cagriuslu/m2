#include "SDLUtils.h"

Vec2I SDLUtils_CenterOfRect(SDL_Rect rect) {
	return (Vec2I) { rect.x + rect.w / 2, rect.y + rect.h / 2 };
}
