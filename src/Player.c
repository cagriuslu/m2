#include "Player.h"
#include "Main.h"

static void Player_prePhysics(Object *obj) {
	if (IsKeyDown(KEY_UP)) {
		obj->pos.y -= 0.1;
	}
	if (IsKeyDown(KEY_DOWN)) {
		obj->pos.y += 0.1;
	}
	if (IsKeyDown(KEY_LEFT)) {
		obj->pos.x -= 0.1;
	}
	if (IsKeyDown(KEY_RIGHT)) {
		obj->pos.x += 0.1;
	}
}

static void Player_ovrdGraphics(Object *obj, SDL_Renderer *renderer) {
	// Draw a blue box
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	SDL_Rect rect = (SDL_Rect) {
		0, 
		0, 
		(int32_t) round(obj->txSrc.w * obj->txScaleW),
		(int32_t) round(obj->txSrc.h * obj->txScaleH)
	};
	SDL_RenderFillRect(renderer, &rect);
}

int PlayerInit(Object *obj) {
	int res = ObjectInit(obj);
	if (res != X_OK) {
		return res;
	}
	obj->prePhysics = Player_prePhysics;
	obj->txSrc.w = 20;
	obj->txSrc.h = 40;
	obj->txOff.y = -20;
	obj->ovrdGraphics = Player_ovrdGraphics;
	return X_OK;
}

void PlayerDeinit(Object *obj) {

}
