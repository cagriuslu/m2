#ifndef EVENT_HANDLING_H
#define EVENT_HANDLING_H

#include "Controls.h"
#include <stdbool.h>

void GatherEvents(bool *outQuit);
bool IsKeyPressed(Key key);
bool IsKeyReleased(Key key);
bool IsKeyDown(Key key);

#endif
