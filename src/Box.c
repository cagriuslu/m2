#include "Box.h"
#include "Main.h"

static void Box_ovrdGraphics(Object *obj) {
	// Draw a white box
	SDL_SetRenderDrawColor(CurrentRenderer(), 255, 255, 255, 255);
	SDL_Rect rect = (SDL_Rect) {
		0,
		0,
		(int32_t) round(obj->txSrc.w * obj->txScaleW),
		(int32_t) round(obj->txSrc.h * obj->txScaleH)
	};
	SDL_RenderFillRect(CurrentRenderer(), &rect);
}

int BoxInit(Object *obj) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->txSrc.w = 20;
	obj->txSrc.h = 20;
	obj->ovrdGraphics = Box_ovrdGraphics;
	return 0;
}

void BoxDeinit(Object *obj) {
	ObjectDeinit(obj);
}
