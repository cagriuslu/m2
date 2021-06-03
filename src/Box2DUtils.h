#ifndef BOX2D_UTILS_H
#define BOX2D_UTILS_H

#include "Box2DWrapper.h"

#define ALLOW_SLEEP (true)
#define DONT_SLEEP (false)

#define IS_SENSOR (true)
#define NOT_SENSOR (false)

Box2DBody* Box2DUtilsCreateStaticBox(Vec2F position, bool allowSleep, bool isSensor, Vec2F dims);
Box2DBody* Box2DUtilsCreateDynamicBox(Vec2F position, bool allowSleep, bool isSensor, Vec2F dims, float density, float linearDamping);

Box2DBody* Box2DUtilsCreateStaticDisk(Vec2F position, bool allowSleep, bool isSensor, float radius);
Box2DBody* Box2DUtilsCreateDynamicDisk(Vec2F position, bool allowSleep, bool isSensor, float radius, float density, float linearDamping);

#endif
