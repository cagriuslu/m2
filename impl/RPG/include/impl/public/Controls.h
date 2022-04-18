#ifndef IMPL_CONTROLS_H
#define IMPL_CONTROLS_H

#include <m2/Controls.h>
#include <array>

namespace impl {
	m2::Key scancode_to_key(SDL_Scancode scancode);

	extern const std::array<SDL_Scancode, static_cast<unsigned>(m2::Key::end)> key_to_scancode;
}

#endif //IMPL_CONTROLS_H
