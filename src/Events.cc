#include <m2/Events.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/thirdparty/video/Mouse.h>

using namespace m2;

void Events::Clear() { *this = Events(); }

bool Events::Gather() {
	// Current Events architecture does not store the relative ordering of the events that happens back to back during
	// one frame. This is not a problem for most events except key up/down and mouse button up/down events. Since we
	// don't store the relative ordering of the events, if a key is pressed and released before the next frame, we can't
	// tell which happened before the other. There are multiple strategies of dealing with this. Caching the event for
	// later, or storing the order of the events are valid strategies, but caching is complex to implement here, and
	// storing the ordering of the events would complicate the game code. The easiest to implement solution is to just
	// postponing the future events to the future frames if a key/button up/down event is encountered.

	bool keyPressed = false, keyReleased = false, mouseMoved = false, mouseButtonPressed = false, mouseButtonReleased = false;
	SDL_Event e;
	while (!_quit && SDL_PollEvent(&e) != 0) {
		switch (e.type) {
			case SDL_QUIT:
				_quit = true;
				goto postponeFutureEvents; // Ignore/postpone other events
			case SDL_WINDOWEVENT:
				switch (e.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						_windowResized = true;
						break;
					default: break;
				}
				break;
			case SDL_KEYDOWN:
				if (e.key.repeat == 0) {
					keyPressed = true;
					if (const auto it = M2_GAME.scancodeToKeyMap.find(e.key.keysym.scancode);
							it != M2_GAME.scancodeToKeyMap.end()) {
						const auto key = it->second;
						const auto keyIndex = pb::enum_index(key);
						++_keysPressed[keyIndex];
					}
					goto postponeFutureEvents; // Read the note above the while loop
				}
				break;
			case SDL_KEYUP:
				if (e.key.repeat == 0) {
					keyReleased = true;
					if (const auto it = M2_GAME.scancodeToKeyMap.find(e.key.keysym.scancode); it != M2_GAME.scancodeToKeyMap.end()) {
						const auto key = it->second;
						const auto keyIndex = pb::enum_index(key);
						++_keysReleased[keyIndex];
					}
					goto postponeFutureEvents; // Read the note above the while loop
				}
				break;
			case SDL_MOUSEMOTION:
				mouseMoved = true;
				// Store the latest mouse position again
				_mousePositionPx.x = e.motion.x;
				_mousePositionPx.y = e.motion.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (const auto mouseButton = thirdparty::video::SystemButtonToMouseButton(e.button.button)) {
					const auto mousePositionPx = VecI{e.button.x, e.button.y};
					mouseButtonPressed = true;
					_mouseActions[U(*mouseButton)] = MouseAction{
						.type = MouseActionType::PRESSED,
						.positionPx = mousePositionPx
					};
					// It is possible that the button was pressed after querying mouse state, but before polling this event.
					// If button down was pressed, also make sure mouse_buttons_down reflects that.
					_downButtons[U(*mouseButton)] = true;
					if (M2_GAME.HasLevel()) {
						if (auto* primarySelection = M2_LEVEL.GetPrimarySelection(); primarySelection && PeekMouseButtonPress(MouseButton::PRIMARY, primarySelection->ScreenBoundaryPx())) {
							primarySelection->SetFirstAndClearSecondPositionM(M2_LEVEL.GetWorldPositionOfPixel(mousePositionPx));
							if (auto* secondarySelection = M2_LEVEL.GetSecondarySelection()) {
								secondarySelection->Reset();
							}
						}
						if (auto* secondarySelection = M2_LEVEL.GetSecondarySelection(); secondarySelection && PeekMouseButtonPress(MouseButton::SECONDARY, secondarySelection->ScreenBoundaryPx())) {
							secondarySelection->SetFirstAndClearSecondPositionM(M2_LEVEL.GetWorldPositionOfPixel(mousePositionPx));
							if (auto* primarySelection = M2_LEVEL.GetPrimarySelection()) {
								primarySelection->Reset();
							}
						}
					}
					goto postponeFutureEvents; // Read the note above the while loop
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (const auto mouseButton = thirdparty::video::SystemButtonToMouseButton(e.button.button)) {
					const auto mousePositionPx = VecI{e.button.x, e.button.y};
					mouseButtonReleased = true;
					_mouseActions[U(*mouseButton)] = MouseAction{
						.type = MouseActionType::RELEASED,
						.positionPx = mousePositionPx
					};
					// It is possible that the button was released after querying mouse state, but before polling this event.
					// If button down was released, also make sure mouse_buttons_down reflects that.
					_downButtons[U(*mouseButton)] = false;
					if (M2_GAME.HasLevel()) {
						if (auto* primarySelection = M2_LEVEL.GetPrimarySelection(); primarySelection && PeekMouseButtonRelease(MouseButton::PRIMARY, primarySelection->ScreenBoundaryPx())) {
							primarySelection->SetSecondPositionIfFirstSetM(M2_LEVEL.GetWorldPositionOfPixel(mousePositionPx));
							if (auto* secondarySelection = M2_LEVEL.GetSecondarySelection()) {
								secondarySelection->Reset();
							}
						}
						if (auto* secondarySelection = M2_LEVEL.GetSecondarySelection(); secondarySelection && PeekMouseButtonRelease(MouseButton::SECONDARY, secondarySelection->ScreenBoundaryPx())) {
							secondarySelection->SetSecondPositionIfFirstSetM(M2_LEVEL.GetWorldPositionOfPixel(mousePositionPx));
							if (auto* primarySelection = M2_LEVEL.GetPrimarySelection()) {
								primarySelection->Reset();
							}
						}
					}
					goto postponeFutureEvents; // Read the note above the while loop
				}
				break;
			case SDL_MOUSEWHEEL:
				_verticalScrollCount += e.wheel.y;
				_horizontalScrollCount += e.wheel.x;
				break;
			case SDL_TEXTINPUT:
				if (SDL_IsTextInputActive()) {
					for (const char c : e.text.text) {
						if (c != 0) {
							_textInput << c;
						} else {
							break;
						}
					}
				} else {
					_textInput = std::stringstream();
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
		_downKeys[i] = false;

		const auto key = pb::enum_value<m2g::pb::KeyType>(i);
		auto [lower, upper] = M2_GAME.keyToScancodeMap.equal_range(key);
		for (auto it = lower; it != upper; ++it) {
			_downKeys[i] = _downKeys[i] || (it->second != SDL_SCANCODE_UNKNOWN && raw_keyboard_state[it->second]);
		}
	}

	const uint32_t mouseStateBitmask = SDL_GetMouseState(&_mousePositionPx.x, &_mousePositionPx.y);
	_mousePositionM = M2_GAME.HasLevel() ? M2_LEVEL.GetWorldPositionOfPixel(_mousePositionPx) : VecF{};
	_downButtons[U(MouseButton::PRIMARY)] = mouseStateBitmask & SDL_BUTTON(SDL_BUTTON_LEFT);
	_downButtons[U(MouseButton::SECONDARY)] = mouseStateBitmask & SDL_BUTTON(SDL_BUTTON_RIGHT);
	_downButtons[U(MouseButton::MIDDLE)] = mouseStateBitmask & SDL_BUTTON(SDL_BUTTON_MIDDLE);

	return _quit || _windowResized || keyPressed || keyReleased || mouseMoved || mouseButtonPressed
			|| mouseButtonReleased || _verticalScrollCount || _horizontalScrollCount
			|| not _textInput.str().empty() || std::ranges::any_of(_downKeys, [](auto x) { return x; }) ||
		    std::ranges::any_of(_downButtons, [](auto x) { return x; });
}

bool Events::PopQuit() {
	const auto value = _quit;
	_quit = false;
	return value;
}

bool Events::PopWindowResize() {
	const auto tmp = _windowResized;
	_windowResized = false;
	return tmp;
}

bool Events::PopKeyPress(const m2g::pb::KeyType key) {
	if (const auto keyIndex = pb::enum_index(key); _keysPressed[keyIndex]) {
		--_keysPressed[keyIndex];
		return true;
	}
	return false;
}

bool Events::PopKeyRelease(const m2g::pb::KeyType key) {
	if (const auto keyIndex = pb::enum_index(key); _keysReleased[keyIndex]) {
		--_keysReleased[keyIndex];
		return true;
	}
	return false;
}

bool Events::PeekMouseButtonPress(const MouseButton mb) const {
	return _mouseActions[U(mb)].type == MouseActionType::PRESSED;
}
bool Events::PopMouseButtonPress(const MouseButton mb) {
	if (PeekMouseButtonPress(mb)) {
		_mouseActions[U(mb)].type = MouseActionType::NO_ACTION;
		return true;
	}
	return false;
}
bool Events::PeekMouseButtonPress(const MouseButton mb, const RectI& rect) const {
	return PeekMouseButtonPress(mb) && rect.DoesContain(_mouseActions[U(mb)].positionPx);
}
bool Events::PopMouseButtonPress(const MouseButton mb, const RectI& rect) {
	if (PeekMouseButtonPress(mb) && rect.DoesContain(_mouseActions[U(mb)].positionPx)) {
		_mouseActions[U(mb)].type = MouseActionType::NO_ACTION;
		return true;
	}
	return false;
}
bool Events::PeekMouseButtonRelease(const MouseButton mb) const {
	return _mouseActions[U(mb)].type == MouseActionType::RELEASED;
}
bool Events::PopMouseButtonRelease(const MouseButton mb) {
	if (PeekMouseButtonRelease(mb)) {
		_mouseActions[U(mb)].type = MouseActionType::NO_ACTION;
		return true;
	}
	return false;
}
bool Events::PeekMouseButtonRelease(const MouseButton mb, const RectI& rect) const {
	return PeekMouseButtonRelease(mb) && rect.DoesContain(_mouseActions[U(mb)].positionPx);
}
bool Events::PopMouseButtonRelease(const MouseButton mb, const RectI& rect) {
	if (PeekMouseButtonRelease(mb) && rect.DoesContain(_mouseActions[U(mb)].positionPx)) {
		_mouseActions[U(mb)].type = MouseActionType::NO_ACTION;
		return true;
	}
	return false;
}
void Events::ClearMouseButtonActions(const RectI& rect) {
	for (unsigned i = 0; i < U(MouseButton::end); ++i) {
		if (rect.DoesContain(_mouseActions[i].positionPx)) {
			_mouseActions[i].type = MouseActionType::NO_ACTION;
		}
	}
}

int32_t Events::PopMouseWheelVerticalScroll() {
	const auto value = _verticalScrollCount;
	_verticalScrollCount = 0;
	return value;
}
int32_t Events::PopMouseWheelVerticalScroll(const RectI& rect) {
	if (rect.DoesContain(MousePosition())) { // TODO instead of MousePosition, store the position of the mouse when the event occurred
		const auto value = _verticalScrollCount;
		_verticalScrollCount = 0;
		return value;
	}
	return 0;
}
int32_t Events::PopMouseWheelHorizontalScroll() {
	const auto value = _horizontalScrollCount;
	_horizontalScrollCount = 0;
	return value;
}
int32_t Events::PopMouseWheelHorizontalScroll(const RectI& rect) {
	if (rect.DoesContain(MousePosition())) { // TODO instead of MousePosition, store the position of the mouse when the event occurred
		const auto value = _horizontalScrollCount;
		_horizontalScrollCount = 0;
		return value;
	}
	return 0;
}
void Events::ClearMouseWheelScrolls(const RectI& rect) {
	if (rect.DoesContain(MousePosition())) { // TODO instead of MousePosition, store the position of the mouse when the event occurred
		_verticalScrollCount = 0;
		_horizontalScrollCount = 0;
	}
}

std::optional<std::string> Events::PopTextInput() {
	if (auto str = _textInput.str(); not str.empty()) {
		_textInput = std::stringstream();
		return str;
	}
	return {};
}

void Events::ClearMouseButtonDown(const RectI& rect) {
	if (rect.DoesContain(MousePosition())) {
		_downButtons = {};
	}
}
