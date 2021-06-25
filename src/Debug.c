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

void DebugKeys(const char *message, uint16_t *keysPressed, uint16_t *keysReleased, uint8_t *keysState) {
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
	if (keysState) {
		fprintf(stderr, "KeysState{");
		if (keysState[KEY_UP]) {
			fprintf(stderr, "UP ");
		}
		if (keysState[KEY_DOWN]) {
			fprintf(stderr, "DOWN ");
		}
		if (keysState[KEY_LEFT]) {
			fprintf(stderr, "LEFT ");
		}
		if (keysState[KEY_RIGHT]) {
			fprintf(stderr, "RIGHT ");
		}
		fprintf(stderr, "} ");
	}
	fprintf(stderr, "\n");
}

void DebugButtons(const char *message, uint16_t *buttonsPressed, uint16_t *buttonsReleased, uint8_t *buttonsState) {
	fprintf(stderr, "%s ", message);
	if (buttonsPressed) {
		fprintf(stderr, "ButtonsPressed{");
		if (buttonsPressed[BUTTON_PRIMARY]) {
			fprintf(stderr, "PRIMARY ");
		}
		if (buttonsPressed[BUTTON_SECONDARY]) {
			fprintf(stderr, "SECONDARY ");
		}
		fprintf(stderr, "} ");
	}
	if (buttonsReleased) {
		fprintf(stderr, "ButtonsReleased{");
		if (buttonsReleased[BUTTON_PRIMARY]) {
			fprintf(stderr, "PRIMARY ");
		}
		if (buttonsReleased[BUTTON_SECONDARY]) {
			fprintf(stderr, "SECONDARY ");
		}
		fprintf(stderr, "} ");
	}
	if (buttonsState) {
		fprintf(stderr, "ButtonsState{");
		if (buttonsState[BUTTON_PRIMARY]) {
			fprintf(stderr, "PRIMARY ");
		}
		if (buttonsState[BUTTON_SECONDARY]) {
			fprintf(stderr, "SECONDARY ");
		}
		fprintf(stderr, "} ");
	}
	fprintf(stderr, "\n");
}

void DebugBox2DAABB(const char* message, Box2DAABB aabb) {
	fprintf(stderr, "%s Box2DAABB{lowerBound:{x:%f, y:%f}, upperBound:{x:%f, y:%f}}\n", message, aabb.lowerBound.x, aabb.lowerBound.y, aabb.upperBound.x, aabb.upperBound.y);
}

void DebugIntArray(const char* message, Array* array) {
	fprintf(stderr, "%s ", message);
	fprintf(stderr, "Array{length:%zu, data:[", array->length);
	for (size_t i = 0; i < array->length; i++) {
		int* ptr = ArrayGet(array, i);
		fprintf(stderr, "%d, ", *ptr);
	}
	fprintf(stderr, "]}\n");
}

void DebugVec2IList(const char* message, List* list) {
	fprintf(stderr, "%s ", message);
	fprintf(stderr, "List{length:%zu, data:[", list->bucket.size);
	for (uint64_t iterator = ListGetFirst(list); iterator; iterator = ListGetNext(list, iterator)) {
		Vec2I* ptr = ListGetData(list, iterator);
		fprintf(stderr, "{%d,%d}, ", ptr->x, ptr->y);
	}
	fprintf(stderr, "]}\n");
}
