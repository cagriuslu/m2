#ifndef OBJECT_BLUEPRINT_H
#define OBJECT_BLUEPRINT_H

#include "Object.h"
#include "Error.h"

int BlueprintStaticBoxInit(Object *obj, Vec2F position);
int BlueprintWallInit(Object* obj, Vec2F position);
int BlueprintSkeletonInit(Object* obj, Vec2F position);

#endif
