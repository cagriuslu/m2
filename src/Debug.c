#include "Debug.h"
#include <stdio.h>

void DebugVec3F(const char *message, Vec3F vec) {
	fprintf(stderr, "%s Vec3F{x:%f, y:%f, z:%f}\n", message, vec.x, vec.y, vec.z);
}

void DebugVec2I(const char *message, Vec2I vec) {
	fprintf(stderr, "%s Vec2I{x:%d, y:%d}\n", message, vec.x, vec.y);
}

void DebugSDLRect(const char *message, SDL_Rect rect) {
	fprintf(stderr, "%s SDL_Rect{x:%d, y:%d, w:%d, h:%d}\n", message, rect.x, rect.y, rect.w, rect.h);
}
