#ifndef EVENT_HANDLING_H
#define EVENT_HANDLING_H

#include "m2/Controls.hh"
#include <m2/vec2i.hh>
#include <stdbool.h>

typedef struct _Events {
	bool quitEvent;
	
	bool windowResizeEvent;
	m2::vec2i windowDims;
	
	bool keyDownEvent;
	uint16_t keysPressed[_KEY_COUNT];
	
	bool keyUpEvent;
	uint16_t keysReleased[_KEY_COUNT];
	
	bool mouseMotionEvent;
	
	bool mouseButtonDownEvent;
	bool mouseWheelEvent;
	uint16_t buttonsPressed[_BUTTON_COUNT];

	bool mouseButtonUpEvent;
	uint16_t buttonsReleased[_BUTTON_COUNT];

	bool textInputEvent;
	char textInput[32];

	// Persistent states
	uint8_t keyStates[_KEY_COUNT];
	uint8_t buttonStates[_BUTTON_COUNT];
	m2::vec2i mousePosition;

	uint8_t rawKeyStates[SDL_NUM_SCANCODES];
} Events;

bool Events_Gather(Events* evs);

#endif
