#ifndef EVENT_HANDLING_H
#define EVENT_HANDLING_H

#include "Controls.h"
#include "Vec2I.h"
#include "Vec2F.h"
#include <stdbool.h>

void GatherEvents(bool *outQuit, bool *outWindow, bool *outKey, bool *outMotion, bool *outButton, bool *outWheel);

uint16_t IsKeyPressed(Key key);
uint16_t IsKeyReleased(Key key);
bool IsKeyDown(Key key);

uint16_t IsButtonPressed(MouseButton button);
uint16_t IsButtonReleased(MouseButton button);
bool IsButtonDown(MouseButton button);
Vec2I PointerPosition();

uint16_t* KeysPressedArray();
uint16_t* KeysReleasedArray();
uint8_t* KeysStateArray();
uint16_t* ButtonsPressedArray();
uint16_t* ButtonsReleasedArray();
uint8_t* ButtonsStateArray();

#endif
