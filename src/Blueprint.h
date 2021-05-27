#ifndef OBJECT_BLUEPRINT_H
#define OBJECT_BLUEPRINT_H

#include "Object.h"
#include "Error.h"

int StaticBoxInit(Object *obj, Vec2F position);
void StaticBoxDeinit(Object *obj);

int WallInit(Object *obj, Vec2F position);

#endif
