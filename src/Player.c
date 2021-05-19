#include "Player.h"

static void Player_ovrdGraphics(Object *obj, SDL_Renderer *renderer) {
	
}

int PlayerInit(Object *obj) {
	int res = ObjectInit(obj);
	if (res != X_OK) {
		return res;
	}
	obj->txSrc.w = 20;
	obj->txSrc.h = 40;
	obj->txOff.y = -20;
	obj->ovrdGraphics = Player_ovrdGraphics;
	return X_OK;
}

void PlayerDeinit(Object *obj) {

}
