#include <m2/Events.h>
#include <m2g/Controls.h>
#include <m2/Def.h>

m2::Events::Events() : quit(), window_resize(), key_press_count(), keys_pressed(), key_release_count(),
					   keys_released(), mouse_button_press_count(), mouse_buttons_pressed(), mouse_button_release_count(),
					   mouse_buttons_released(), mouse_wheel_scroll_count(), sdl_keys_down(), keys_down(), mouse_buttons_down() {}

void m2::Events::clear() {
	*this = Events();
}

bool m2::Events::gather() {
	bool mouse_moved = false;
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
				window_resize = m2::Vec2i{e.window.data1 , e.window.data2};
				break;
			}
			break;
		case SDL_KEYDOWN:
			if (e.key.repeat == 0) {
				key_press_count++;
				keys_pressed[u(m2g::scancode_to_key(e.key.keysym.scancode))]++;
			}
			break;
		case SDL_KEYUP:
			if (e.key.repeat == 0) {
				key_release_count++;
				keys_released[u(m2g::scancode_to_key(e.key.keysym.scancode))]++;
			}
			break;
		case SDL_MOUSEMOTION:
			mouse_moved = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			mouse_button_press_count++;
			mouse_buttons_pressed[u(m2::button_to_mouse_button(e.button.button))]++;
			break;
		case SDL_MOUSEBUTTONUP:
			mouse_button_release_count++;
			mouse_buttons_released[u(m2::button_to_mouse_button(e.button.button))]++;
			break;
		case SDL_MOUSEWHEEL:
			mouse_wheel_scroll_count += e.wheel.y;
			break;
		case SDL_TEXTINPUT:
			if (SDL_IsTextInputActive()) {
				for (char c : e.text.text) {
					if (c != 0) {
						text_input << c;
					} else {
						break;
					}
				}
			} else {
				text_input = std::stringstream();
			}
			break;
		default:
			break;
		}
	}

	int keyCount = 0;
	const uint8_t* raw_keyboard_state = SDL_GetKeyboardState(&keyCount);
	for (int i = 0; i < keyCount; i++) {
		sdl_keys_down[i] = raw_keyboard_state[i];
	}
	for (unsigned i = 1; i < u(m2::Key::end); i++) {
		auto scancode = m2g::key_to_scancode[i];
		keys_down[i] = (scancode != SDL_SCANCODE_UNKNOWN) && raw_keyboard_state[scancode];
	}

	const uint32_t mouseStateBitmask = SDL_GetMouseState(&_mouse_position.x, &_mouse_position.y);
	mouse_buttons_down[u(m2::MouseButton::PRIMARY)] = mouseStateBitmask & SDL_BUTTON(SDL_BUTTON_LEFT);
	mouse_buttons_down[u(m2::MouseButton::SECONDARY)] = mouseStateBitmask & SDL_BUTTON(SDL_BUTTON_RIGHT);
	mouse_buttons_down[u(m2::MouseButton::MIDDLE)] = mouseStateBitmask & SDL_BUTTON(SDL_BUTTON_MIDDLE);

	return quit || window_resize || key_press_count || key_release_count || mouse_moved || mouse_button_press_count ||
		mouse_button_release_count || mouse_wheel_scroll_count || text_input.tellp();
}

bool m2::Events::pop_quit() {
	auto value = quit;
	quit = false;
	return value;
}

std::optional<m2::Vec2i> m2::Events::pop_window_resize() {
	if (window_resize) {
		auto value = *window_resize;
		window_resize.reset();
		return value;
	} else {
		return {};
	}
}

bool m2::Events::pop_key_press(m2::Key k) {
	if (keys_pressed[u(k)]) {
		keys_pressed[u(k)]--;
		key_press_count--;
		return true;
	} else {
		return false;
	}
}

bool m2::Events::pop_key_release(m2::Key k) {
	if (keys_released[u(k)]) {
		keys_released[u(k)]--;
		key_release_count--;
		return true;
	} else {
		return false;
	}
}

bool m2::Events::pop_mouse_button_press(m2::MouseButton mb) {
	if (mouse_buttons_pressed[u(mb)]) {
		mouse_buttons_pressed[u(mb)]--;
		mouse_button_press_count--;
		return true;
	} else {
		return false;
	}
}

bool m2::Events::pop_mouse_button_release(m2::MouseButton mb) {
	if (mouse_buttons_released[u(mb)]) {
		mouse_buttons_released[u(mb)]--;
		mouse_button_release_count--;
		return true;
	} else {
		return false;
	}
}

bool m2::Events::pop_mouse_wheel_scroll() {
	auto value = mouse_wheel_scroll_count;
	mouse_wheel_scroll_count = 0;
	return value;
}

std::optional<std::string> m2::Events::pop_text_input() {
	if (text_input.tellp()) {
		auto str = text_input.str();
		text_input = std::stringstream();
		return str;
	} else {
		return {};
	}
}

bool m2::Events::is_sdl_key_down(SDL_Scancode sc) const {
	return sdl_keys_down[sc];
}

bool m2::Events::is_key_down(Key k) const {
	return keys_down[u(k)];
}

bool m2::Events::is_mouse_button_down(MouseButton mb) const {
	return mouse_buttons_down[u(mb)];
}

m2::Vec2i m2::Events::mouse_position() const {
	return _mouse_position;
}
