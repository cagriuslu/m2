#ifndef MINE_CONTROLS_H
#define MINE_CONTROLS_H

#include "m2/Controls.h"
#include <array>

namespace m2g {
	m2::Key scancode_to_key(SDL_Scancode scancode);

	extern const std::array<SDL_Scancode, static_cast<unsigned>(m2::Key::end)> key_to_scancode;
}

#endif //MINE_CONTROLS_H
