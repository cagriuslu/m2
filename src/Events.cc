#include <m2/Events.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/Proxy.h>

using namespace m2;

void Events::Clear() { *this = Events(); }

bool Events::Gather() {
	// Set the mouse position first because below calculations may call MousePositionWorldM(), which reads _mouse_position.
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
					default: break;
				}
				break;
			case SDL_KEYDOWN:
				if (e.key.repeat == 0) {
					++key_press_count;
					if (const auto it = M2_GAME.scancodeToKeyMap.find(e.key.keysym.scancode);
							it != M2_GAME.scancodeToKeyMap.end()) {
						const auto key = it->second;
						const auto keyIndex = pb::enum_index(key);
						++keys_pressed[keyIndex];
					}
					goto postponeFutureEvents; // Read the note above the while loop
				}
				break;
			case SDL_KEYUP:
				if (e.key.repeat == 0) {
					++key_release_count;
					if (const auto it = M2_GAME.scancodeToKeyMap.find(e.key.keysym.scancode);
												it != M2_GAME.scancodeToKeyMap.end()) {
						const auto key = it->second;
						const auto keyIndex = pb::enum_index(key);
						++keys_released[keyIndex];
					}
					goto postponeFutureEvents; // Read the note above the while loop
				}
				break;
			case SDL_MOUSEMOTION:
				mouse_moved = true;
				break;
			case SDL_MOUSEBUTTONDOWN:
				++mouse_button_press_count;
				++mouse_buttons_pressed[u(button_to_mouse_button(e.button.button))];
				if (auto* primarySelection = M2_LEVEL.PrimarySelection();
						primarySelection && PeekMouseButtonPress(MouseButton::PRIMARY, primarySelection->ScreenBoundaryPx())) {
					primarySelection->SetFirstAndClearSecondPositionM(M2_GAME.MousePositionWorldM());
					if (auto* secondarySelection = M2_LEVEL.SecondarySelection()) {
						secondarySelection->Reset();
					}
				}
				if (auto* secondarySelection = M2_LEVEL.SecondarySelection();
						secondarySelection && PeekMouseButtonPress(MouseButton::SECONDARY, secondarySelection->ScreenBoundaryPx())) {
					secondarySelection->SetFirstAndClearSecondPositionM(M2_GAME.MousePositionWorldM());
					if (auto* primarySelection = M2_LEVEL.PrimarySelection()) {
						primarySelection->Reset();
					}
				}
				goto postponeFutureEvents; // Read the note above the while loop
			case SDL_MOUSEBUTTONUP:
				++mouse_button_release_count;
				++mouse_buttons_released[u(button_to_mouse_button(e.button.button))];
				if (auto* primarySelection = M2_LEVEL.PrimarySelection();
						primarySelection && PeekMouseButtonRelease(MouseButton::PRIMARY, primarySelection->ScreenBoundaryPx())) {
					primarySelection->SetSecondPositionIfFirstSetM(M2_GAME.MousePositionWorldM());
					if (auto* secondarySelection = M2_LEVEL.SecondarySelection()) {
						secondarySelection->Reset();
					}
				}
				if (auto* secondarySelection = M2_LEVEL.SecondarySelection();
						secondarySelection && PeekMouseButtonRelease(MouseButton::SECONDARY, secondarySelection->ScreenBoundaryPx())) {
					secondarySelection->SetSecondPositionIfFirstSetM(M2_GAME.MousePositionWorldM());
					if (auto* primarySelection = M2_LEVEL.PrimarySelection()) {
						primarySelection->Reset();
					}
				}
				goto postponeFutureEvents; // Read the note above the while loop
			case SDL_MOUSEWHEEL:
				mouse_wheel_vertical_scroll_count += e.wheel.y;
				mouse_wheel_horizontal_scroll_count += e.wheel.x;
				break;
			case SDL_TEXTINPUT:
				if (SDL_IsTextInputActive()) {
					for (const char c : e.text.text) {
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
	for (int i = 0; i < pb::enum_value_count<m2g::pb::KeyType>(); i++) {
		// Reset state
		keys_down[i] = false;

		const auto key = pb::enum_value<m2g::pb::KeyType>(i);
		auto [lower, upper] = M2_GAME.keyToScancodeMap.equal_range(key);
		for (auto it = lower; it != upper; ++it) {
			keys_down[i] = keys_down[i] || (it->second != SDL_SCANCODE_UNKNOWN && raw_keyboard_state[it->second]);
		}
	}

	return quit || window_resize || key_press_count || key_release_count || mouse_moved || mouse_button_press_count
			|| mouse_button_release_count || mouse_wheel_vertical_scroll_count || mouse_wheel_horizontal_scroll_count
			|| (not text_input.str().empty()) || std::ranges::any_of(keys_down, [](auto x) { return x; }) ||
		    std::ranges::any_of(mouse_buttons_down, [](auto x) { return x; });
}

bool Events::PopQuit() {
	const auto value = quit;
	quit = false;
	return value;
}

bool Events::PopWindowResize() {
	if (window_resize) {
		window_resize = false;
		return true;
	}
	return false;
}

bool Events::PopKeyPress(const m2g::pb::KeyType key) {
	if (const auto keyIndex = pb::enum_index(key); keys_pressed[keyIndex]) {
		--keys_pressed[keyIndex];
		--key_press_count;
		return true;
	}
	return false;
}

bool Events::PopKeyRelease(const m2g::pb::KeyType key) {
	if (const auto keyIndex = pb::enum_index(key); keys_released[keyIndex]) {
		--keys_released[keyIndex];
		--key_release_count;
		return true;
	}
	return false;
}

bool Events::PeekMouseButtonPress(const MouseButton mb) const { return mouse_buttons_pressed[u(mb)]; }
bool Events::PopMouseButtonPress(const MouseButton mb) {
	if (mouse_buttons_pressed[u(mb)]) {
		--mouse_buttons_pressed[u(mb)];
		--mouse_button_press_count;
		return true;
	}
	return false;
}
bool Events::PeekMouseButtonPress(const MouseButton mb, const RectI& rect) const {
	return mouse_buttons_pressed[u(mb)] && rect.contains(MousePosition());
}
bool Events::PopMouseButtonPress(const MouseButton mb, const RectI& rect) {
	if (mouse_buttons_pressed[u(mb)] && rect.contains(MousePosition())) {
		--mouse_buttons_pressed[u(mb)];
		--mouse_button_press_count;
		return true;
	}
	return false;
}
void Events::ClearMouseButtonPresses(const RectI& rect) {
	if (rect.contains(MousePosition())) {
		mouse_buttons_pressed = {};
		mouse_button_press_count = 0;
	}
}
bool Events::PeekMouseButtonRelease(const MouseButton mb) const { return mouse_buttons_released[u(mb)]; }
bool Events::PopMouseButtonRelease(const MouseButton mb) {
	if (mouse_buttons_released[u(mb)]) {
		--mouse_buttons_released[u(mb)];
		--mouse_button_release_count;
		return true;
	}
	return false;
}
bool Events::PeekMouseButtonRelease(const MouseButton mb, const RectI& rect) const {
	return mouse_buttons_released[u(mb)] && rect.contains(MousePosition());
}
bool Events::PopMouseButtonRelease(const MouseButton mb, const RectI& rect) {
	if (mouse_buttons_released[u(mb)] && rect.contains(MousePosition())) {
		--mouse_buttons_released[u(mb)];
		--mouse_button_release_count;
		return true;
	}
	return false;
}
void Events::ClearMouseButtonReleases(const RectI& rect) {
	if (rect.contains(MousePosition())) {
		mouse_buttons_released = {};
		mouse_button_release_count = 0;
	}
}

int32_t Events::PopMouseWheelVerticalScroll() {
	const auto value = mouse_wheel_vertical_scroll_count;
	mouse_wheel_vertical_scroll_count = 0;
	return value;
}
int32_t Events::PopMouseWheelVerticalScroll(const RectI& rect) {
	if (rect.contains(MousePosition())) {
		const auto value = mouse_wheel_vertical_scroll_count;
		mouse_wheel_vertical_scroll_count = 0;
		return value;
	}
	return 0;
}
int32_t Events::PopMouseWheelHorizontalScroll() {
	const auto value = mouse_wheel_horizontal_scroll_count;
	mouse_wheel_horizontal_scroll_count = 0;
	return value;
}
int32_t Events::PopMouseWheelHorizontalScroll(const RectI& rect) {
	if (rect.contains(MousePosition())) {
		const auto value = mouse_wheel_horizontal_scroll_count;
		mouse_wheel_horizontal_scroll_count = 0;
		return value;
	}
	return 0;
}
void Events::ClearMouseWheelScrolls(const RectI& rect) {
	if (rect.contains(MousePosition())) {
		mouse_wheel_vertical_scroll_count = 0;
		mouse_wheel_horizontal_scroll_count = 0;
	}
}

std::optional<std::string> Events::PopTextInput() {
	if (const auto str = text_input.str(); not str.empty()) {
		text_input = std::stringstream();
		return str;
	}
	return {};
}

bool Events::IsKeyDown(const m2g::pb::KeyType key) const { return keys_down[pb::enum_index(key)]; }
bool Events::IsMouseButtonDown(const MouseButton mb) const { return mouse_buttons_down[u(mb)]; }
void Events::ClearMouseButtonDown(const RectI& rect) {
	if (rect.contains(MousePosition())) {
		mouse_buttons_down = {};
	}
}
VecI Events::MousePosition() const { return _mouse_position; }
