#ifndef EVENT_HANDLING_H
#define EVENT_HANDLING_H

#include "Controls.h"
#include "Vec2I.h"
#include "Vec2F.h"
#include <stdbool.h>

typedef struct _Events {
	bool quitEvent;
	
	bool windowResizeEvent;
	Vec2I windowDims;
	
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

	// Persistent states
	uint8_t keyStates[_KEY_COUNT];
	uint8_t buttonStates[_BUTTON_COUNT];
	Vec2I mousePosition;
} Events;

bool Events_Gather(Events* evs);

#endif
