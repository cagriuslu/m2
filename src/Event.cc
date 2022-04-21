#include <m2/Event.hh>
#include <m2g/Controls.h>
#include <m2/Def.h>

m2::Events::Events() : quit(), window_resized(), key_pressed(), keys_pressed(), key_released(), keys_released(),
	mouse_moved(), mouse_button_pressed(), mouse_wheel_scrolled(), mouse_buttons_pressed(), mouse_button_released(),
	mouse_buttons_released(), text_input(), text(), key_down(), mouse_button_down(), raw_key_down() {}

bool m2::Events::gather() {
	*this = Events();

	SDL_Event e;
	while (!quit && SDL_PollEvent(&e) != 0) {
		switch (e.type) {
		case SDL_QUIT:
			quit = true;
			break;
		case SDL_WINDOWEVENT:
			switch (e.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				window_resized = true;
				window_dimensions = m2::Vec2i{e.window.data1 , e.window.data2 };
				break;
			}
			break;
		case SDL_KEYDOWN:
			if (e.key.repeat == 0) {
				key_pressed = true;
				keys_pressed[u(impl::scancode_to_key(e.key.keysym.scancode))] += 1;
			}
			break;
		case SDL_KEYUP:
			if (e.key.repeat == 0) {
				key_released = true;
				keys_released[u(impl::scancode_to_key(e.key.keysym.scancode))] += 1;
			}
			break;
		case SDL_MOUSEMOTION:
			mouse_moved = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			mouse_button_pressed = true;
			mouse_buttons_pressed[u(m2::button_to_mouse_button(e.button.button))] += 1;
			break;
		case SDL_MOUSEBUTTONUP:
			mouse_button_released = true;
			mouse_buttons_released[u(m2::button_to_mouse_button(e.button.button))] += 1;
			break;
		case SDL_MOUSEWHEEL:
			mouse_wheel_scrolled = true;
			if (0 < e.wheel.y) {
				mouse_buttons_pressed[u(m2::MouseButton::SCROLL_UP)] += (int16_t)e.wheel.y;
			} else {
				mouse_buttons_pressed[u(m2::MouseButton::SCROLL_DOWN)] += -(int16_t)e.wheel.y;
			}
			break;
		case SDL_TEXTINPUT:
			if (SDL_IsTextInputActive()) {
				text_input = true;
                unsigned i = 0;
                for (auto& t : text) {
                    t = e.text.text[i++];
                }
			}
			break;
		default:
			break;
		}
	}

	int keyCount = 0;
	const uint8_t* raw_keyboard_state = SDL_GetKeyboardState(&keyCount);
	for (unsigned i = 1; i < u(m2::Key::end); i++) {
		auto scancode = impl::key_to_scancode[i];
		if (scancode != SDL_SCANCODE_UNKNOWN) {
			key_down[i] = raw_keyboard_state[scancode];
		}
	}
	for (int i = 0; i < keyCount; i++) {
		raw_key_down[i] = raw_keyboard_state[i];
	}

	const uint32_t mouseStateBitmask = SDL_GetMouseState(&mouse_position.x, &mouse_position.y);
	if (mouseStateBitmask & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		mouse_button_down[u(m2::MouseButton::PRIMARY)] = true;
	}
	if (mouseStateBitmask & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
		mouse_button_down[u(m2::MouseButton::SECONDARY)] = true;
	}
	if (mouseStateBitmask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
		mouse_button_down[u(m2::MouseButton::MIDDLE)] = true;
	}

	return quit || window_resized || key_pressed || key_released || mouse_moved || mouse_button_pressed || mouse_wheel_scrolled || mouse_button_released || text_input;
}
