#include "Debug.h"
#include "Controls.h"
#include <stdio.h>

void DebugVec2F(const char *message, Vec2F vec) {
	fprintf(stderr, "%s Vec2F{x:%f, y:%f}\n", message, vec.x, vec.y);
}

void DebugVec2I(const char *message, Vec2I vec) {
	fprintf(stderr, "%s Vec2I{x:%d, y:%d}\n", message, vec.x, vec.y);
}

void DebugSDLRect(const char *message, SDL_Rect rect) {
	fprintf(stderr, "%s SDL_Rect{x:%d, y:%d, w:%d, h:%d}\n", message, rect.x, rect.y, rect.w, rect.h);
}

void DebugKeys(const char *message, uint8_t *keysPressed, uint8_t *keysReleased, uint8_t *keyState) {
	fprintf(stderr, "%s ", message);
	if (keysPressed) {
		fprintf(stderr, "KeysPressed{");
		if (keysPressed[KEY_UP]) {
			fprintf(stderr, "UP ");
		}
		if (keysPressed[KEY_DOWN]) {
			fprintf(stderr, "DOWN ");
		}
		if (keysPressed[KEY_LEFT]) {
			fprintf(stderr, "LEFT ");
		}
		if (keysPressed[KEY_RIGHT]) {
			fprintf(stderr, "RIGHT ");
		}
		fprintf(stderr, "} ");
	}
	if (keysReleased) {
		fprintf(stderr, "KeysReleased{");
		if (keysReleased[KEY_UP]) {
			fprintf(stderr, "UP ");
		}
		if (keysReleased[KEY_DOWN]) {
			fprintf(stderr, "DOWN ");
		}
		if (keysReleased[KEY_LEFT]) {
			fprintf(stderr, "LEFT ");
		}
		if (keysReleased[KEY_RIGHT]) {
			fprintf(stderr, "RIGHT ");
		}
		fprintf(stderr, "} ");
	}
	if (keyState) {
		fprintf(stderr, "KeyState{");
		if (keyState[KEY_UP]) {
			fprintf(stderr, "UP ");
		}
		if (keyState[KEY_DOWN]) {
			fprintf(stderr, "DOWN ");
		}
		if (keyState[KEY_LEFT]) {
			fprintf(stderr, "LEFT ");
		}
		if (keyState[KEY_RIGHT]) {
			fprintf(stderr, "RIGHT ");
		}
		fprintf(stderr, "} ");
	}
	fprintf(stderr, "\n");
}

void DebugObjectDrawList(const char *message, ObjectDrawList *list) {
	fprintf(stderr, "%s ObjectDrawList{", message);
	for (size_t i = 0; i < ObjectDrawListLength(list); i++) {
		fprintf(stderr, "%p ", ObjectDrawListGet(list, i));
	}
	fprintf(stderr, "}\n");
}
