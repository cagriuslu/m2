#include <m2/third_party/video/Window.h>
#include <m2/Game.h>

uint32_t m2::thirdparty::video::GetWindowPixelFormat() {
	return SDL_GetWindowPixelFormat(M2_GAME.window);
}
