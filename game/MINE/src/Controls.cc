#include <m2g/Controls.h>

m2::Key m2g::scancode_to_key(SDL_Scancode scancode) {
	using namespace m2;
	switch (scancode) {
		case SDL_SCANCODE_ESCAPE:
			return Key::MENU;
		case SDL_SCANCODE_W:
			return Key::UP;
		case SDL_SCANCODE_S:
			return Key::DOWN;
		case SDL_SCANCODE_A:
			return Key::LEFT;
		case SDL_SCANCODE_D:
			return Key::RIGHT;
		case SDL_SCANCODE_SPACE:
			return Key::DASH;
		case SDL_SCANCODE_GRAVE:
			return Key::CONSOLE;
		case SDL_SCANCODE_RETURN:
			return Key::ENTER;
		case SDL_SCANCODE_BACKSPACE:
			return Key::BACKSPACE;
		default:
			return Key::UNKNOWN;
	}
}

const std::array<SDL_Scancode, static_cast<unsigned>(m2::Key::end)> m2g::key_to_scancode = {
	SDL_SCANCODE_UNKNOWN,
	SDL_SCANCODE_ESCAPE,
	SDL_SCANCODE_W,
	SDL_SCANCODE_S,
	SDL_SCANCODE_A,
	SDL_SCANCODE_D,
	SDL_SCANCODE_SPACE,
	SDL_SCANCODE_GRAVE,
	SDL_SCANCODE_RETURN,
	SDL_SCANCODE_BACKSPACE
};
