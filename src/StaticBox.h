#ifndef STATIC_BOX_H
#define STATIC_BOX_H

#include "Object.h"
#include "Error.h"

int StaticBoxInit(Object *obj, Vec2F position);
void StaticBoxDeinit(Object *obj);

#endif
