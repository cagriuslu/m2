#include <m2/Events.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/Proxy.h>

using namespace m2;

namespace {
	// Global state
	std::optional<RectI> primary_selection_screen_rect_px, secondary_selection_screen_rect_px;
	std::optional<VecF> primary_selection_position_1_m, primary_selection_position_2_m;
	std::optional<VecF> secondary_selection_position_1_m, secondary_selection_position_2_m;
}  // namespace

void Events::clear() { *this = Events(); }

bool Events::gather() {
	const uint32_t mouseStateBitmask = SDL_GetMouseState(&_mouse_position.x, &_mouse_position.y);
	mouse_buttons_down[u(MouseButton::PRIMARY)] = mouseStateBitmask & SDL_BUTTON(SDL_BUTTON_LEFT);
	mouse_buttons_down[u(MouseButton::SECONDARY)] = mouseStateBitmask & SDL_BUTTON(SDL_BUTTON_RIGHT);
	mouse_buttons_down[u(MouseButton::MIDDLE)] = mouseStateBitmask & SDL_BUTTON(SDL_BUTTON_MIDDLE);

	// Current Events architecture does not store the relative ordering of the events that happens back to back during
	// one frame. This is not a problem for most events except key up/down and mouse button up/down events. Since we
	// don't store the relative ordering of the events, if a key is pressed and released before the next frame, we can't
	// tell which happened before the other. There are multiple strategies of dealing with this. Caching the event for
	// later, or storing the order of the events are valid strategies, but caching is complex to implement here, and
	// storing the ordering of the events would complicate the game code. The easiest to implement solution is to just
	// postponing the future events to the future frames if a key/button up/down event is encountered.

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
						window_resize = true;
						break;
				}
				break;
			case SDL_KEYDOWN:
				if (e.key.repeat == 0) {
					// Game keys
					key_press_count++;
					keys_pressed[u(M2G_PROXY.scancode_to_key(e.key.keysym.scancode))]++;
					// UI keys
					if (ui_keys_pressed.size() < ui_key_press_count_limit) {
						ui_keys_pressed.push_back(e.key.keysym.scancode);
					} else {
						LOG_WARN("UI key press count limit exceeded");
					}
					goto postponeFutureEvents; // Read the note above the while loop
				}
				break;
			case SDL_KEYUP:
				if (e.key.repeat == 0) {
					key_release_count++;
					keys_released[u(M2G_PROXY.scancode_to_key(e.key.keysym.scancode))]++;
					goto postponeFutureEvents; // Read the note above the while loop
				}
				break;
			case SDL_MOUSEMOTION:
				mouse_moved = true;
				break;
			case SDL_MOUSEBUTTONDOWN:
				mouse_button_press_count++;
				mouse_buttons_pressed[u(button_to_mouse_button(e.button.button))]++;
				if (primary_selection_screen_rect_px &&
				    peek_mouse_button_press(MouseButton::PRIMARY, *primary_selection_screen_rect_px)) {
					primary_selection_position_1_m = M2_GAME.MousePositionWorldM();
					primary_selection_position_2_m = std::nullopt;
					secondary_selection_position_1_m = std::nullopt;
					secondary_selection_position_2_m = std::nullopt;
				}
				if (secondary_selection_screen_rect_px &&
				    peek_mouse_button_press(MouseButton::SECONDARY, *secondary_selection_screen_rect_px)) {
					primary_selection_position_1_m = std::nullopt;
					primary_selection_position_2_m = std::nullopt;
					secondary_selection_position_1_m = M2_GAME.MousePositionWorldM();
					secondary_selection_position_2_m = std::nullopt;
				}
				goto postponeFutureEvents; // Read the note above the while loop
			case SDL_MOUSEBUTTONUP:
				mouse_button_release_count++;
				mouse_buttons_released[u(button_to_mouse_button(e.button.button))]++;
				if (primary_selection_screen_rect_px &&
				    peek_mouse_button_release(MouseButton::PRIMARY, *primary_selection_screen_rect_px)) {
					primary_selection_position_2_m = M2_GAME.MousePositionWorldM();
					secondary_selection_position_1_m = std::nullopt;
					secondary_selection_position_2_m = std::nullopt;
				}
				if (secondary_selection_screen_rect_px &&
				    peek_mouse_button_release(MouseButton::SECONDARY, *secondary_selection_screen_rect_px)) {
					primary_selection_position_1_m = std::nullopt;
					primary_selection_position_2_m = std::nullopt;
					secondary_selection_position_2_m = M2_GAME.MousePositionWorldM();
				}
				goto postponeFutureEvents; // Read the note above the while loop
			case SDL_MOUSEWHEEL:
				mouse_wheel_vertical_scroll_count += e.wheel.y;
				mouse_wheel_horizontal_scroll_count += e.wheel.x;
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
	postponeFutureEvents:

	int keyCount = 0;
	const uint8_t* raw_keyboard_state = SDL_GetKeyboardState(&keyCount);
	for (int i = 0; i < keyCount; i++) {
		sdl_keys_down[i] = raw_keyboard_state[i];
	}
	for (unsigned i = 1; i < u(Key::end); i++) {
		auto scancode = M2G_PROXY.key_to_scancode[i];
		keys_down[i] = (scancode != SDL_SCANCODE_UNKNOWN) && raw_keyboard_state[scancode];
	}

	return quit || window_resize || key_press_count || !ui_keys_pressed.empty() || key_release_count || mouse_moved ||
	    mouse_button_press_count || mouse_button_release_count || mouse_wheel_vertical_scroll_count ||
	    mouse_wheel_horizontal_scroll_count || (not text_input.str().empty()) ||
	    (std::find(sdl_keys_down.begin(), sdl_keys_down.end(), true) != sdl_keys_down.end()) ||
	    (std::find(keys_down.begin(), keys_down.end(), true) != keys_down.end()) ||
	    (std::find(mouse_buttons_down.begin(), mouse_buttons_down.end(), true) != mouse_buttons_down.end());
}

bool Events::pop_quit() {
	auto value = quit;
	quit = false;
	return value;
}

bool Events::pop_window_resize() {
	if (window_resize) {
		window_resize = false;
		return true;
	}
	return false;
}

bool Events::pop_key_press(Key k) {
	if (keys_pressed[u(k)]) {
		keys_pressed[u(k)]--;
		key_press_count--;
		return true;
	} else {
		return false;
	}
}

bool Events::pop_ui_key_press(SDL_Scancode scode) {
	for (auto it = ui_keys_pressed.begin(); it != ui_keys_pressed.end(); it++) {
		if (*it == scode) {
			ui_keys_pressed.erase(it);
			return true;
		}
	}
	return false;
}

bool Events::pop_key_release(Key k) {
	if (keys_released[u(k)]) {
		keys_released[u(k)]--;
		key_release_count--;
		return true;
	} else {
		return false;
	}
}

bool Events::peek_mouse_button_press(MouseButton mb) { return mouse_buttons_pressed[u(mb)]; }
bool Events::pop_mouse_button_press(MouseButton mb) {
	if (mouse_buttons_pressed[u(mb)]) {
		mouse_buttons_pressed[u(mb)]--;
		mouse_button_press_count--;
		return true;
	} else {
		return false;
	}
}
bool Events::peek_mouse_button_press(MouseButton mb, const RectI& rect) {
	return mouse_buttons_pressed[u(mb)] && rect.contains(mouse_position());
}
bool Events::pop_mouse_button_press(MouseButton mb, const RectI& rect) {
	if (mouse_buttons_pressed[u(mb)] && rect.contains(mouse_position())) {
		mouse_buttons_pressed[u(mb)]--;
		mouse_button_press_count--;
		return true;
	} else {
		return false;
	}
}
void Events::clear_mouse_button_presses(const RectI& rect) {
	if (rect.contains(mouse_position())) {
		mouse_buttons_pressed = {};
		mouse_button_press_count = 0;
	}
}
bool Events::peek_mouse_button_release(MouseButton mb) { return mouse_buttons_released[u(mb)]; }
bool Events::pop_mouse_button_release(MouseButton mb) {
	if (mouse_buttons_released[u(mb)]) {
		mouse_buttons_released[u(mb)]--;
		mouse_button_release_count--;
		return true;
	} else {
		return false;
	}
}
bool Events::peek_mouse_button_release(MouseButton mb, const RectI& rect) {
	return mouse_buttons_released[u(mb)] && rect.contains(mouse_position());
}
bool Events::pop_mouse_button_release(MouseButton mb, const RectI& rect) {
	if (mouse_buttons_released[u(mb)] && rect.contains(mouse_position())) {
		mouse_buttons_released[u(mb)]--;
		mouse_button_release_count--;
		return true;
	} else {
		return false;
	}
}
void Events::clear_mouse_button_releases(const RectI& rect) {
	if (rect.contains(mouse_position())) {
		mouse_buttons_released = {};
		mouse_button_release_count = 0;
	}
}

int32_t Events::pop_mouse_wheel_vertical_scroll() {
	auto value = mouse_wheel_vertical_scroll_count;
	mouse_wheel_vertical_scroll_count = 0;
	return value;
}
int32_t Events::pop_mouse_wheel_vertical_scroll(const RectI& rect) {
	if (rect.contains(mouse_position())) {
		auto value = mouse_wheel_vertical_scroll_count;
		mouse_wheel_vertical_scroll_count = 0;
		return value;
	}
	return 0;
}
int32_t Events::pop_mouse_wheel_horizontal_scroll() {
	auto value = mouse_wheel_horizontal_scroll_count;
	mouse_wheel_horizontal_scroll_count = 0;
	return value;
}
int32_t Events::pop_mouse_wheel_horizontal_scroll(const RectI& rect) {
	if (rect.contains(mouse_position())) {
		auto value = mouse_wheel_horizontal_scroll_count;
		mouse_wheel_horizontal_scroll_count = 0;
		return value;
	}
	return 0;
}
void Events::clear_mouse_wheel_scrolls(const RectI& rect) {
	if (rect.contains(mouse_position())) {
		mouse_wheel_vertical_scroll_count = 0;
		mouse_wheel_horizontal_scroll_count = 0;
	}
}

std::optional<std::string> Events::pop_text_input() {
	auto str = text_input.str();
	if (not str.empty()) {
		text_input = std::stringstream();
		return str;
	} else {
		return {};
	}
}

bool Events::is_sdl_key_down(SDL_Scancode sc) const { return sdl_keys_down[sc]; }
bool Events::is_key_down(Key k) const { return keys_down[u(k)]; }
bool Events::is_mouse_button_down(MouseButton mb) const { return mouse_buttons_down[u(mb)]; }
void Events::clear_mouse_button_down(const RectI& rect) {
	if (rect.contains(mouse_position())) {
		mouse_buttons_down = {};
	}
}
VecI Events::mouse_position() const { return _mouse_position; }

std::pair<std::optional<VecF>, std::optional<VecF>> Events::primary_selection_position_m() const {
	return std::make_pair(primary_selection_position_1_m, primary_selection_position_2_m);
}
std::pair<std::optional<VecF>, std::optional<VecF>> Events::secondary_selection_position_m() const {
	return std::make_pair(secondary_selection_position_1_m, secondary_selection_position_2_m);
}

void Events::enable_primary_selection(const RectI& screen_rect) {
	LOG_DEBUG("Enabling primary selection");
	primary_selection_screen_rect_px = screen_rect;
	reset_primary_selection();
}
void Events::enable_secondary_selection(const RectI& screen_rect) {
	LOG_DEBUG("Enabling secondary selection");
	secondary_selection_screen_rect_px = screen_rect;
	reset_secondary_selection();
}
void Events::reset_primary_selection() {
	primary_selection_position_1_m = std::nullopt;
	primary_selection_position_2_m = std::nullopt;
}
void Events::reset_secondary_selection() {
	secondary_selection_position_1_m = std::nullopt;
	secondary_selection_position_2_m = std::nullopt;
}
void Events::disable_primary_selection() {
	LOG_DEBUG("Disabling primary selection");
	primary_selection_screen_rect_px = std::nullopt;
	reset_primary_selection();
}
void Events::disable_secondary_selection() {
	LOG_DEBUG("Disabling secondary selection");
	secondary_selection_screen_rect_px = std::nullopt;
	reset_secondary_selection();
}
