#ifndef OBJECT_BLUEPRINT_H
#define OBJECT_BLUEPRINT_H

#include "GameObject.h"
#include "Error.h"

int BlueprintStaticBoxInit(GameObject *obj, Vec2F position);
int BlueprintWallInit(GameObject* obj, Vec2F position);
int BlueprintSkeletonInit(GameObject* obj, Vec2F position);
int BlueprintBulletInit(GameObject* obj, Vec2F position, Vec2F direction);

#endif
