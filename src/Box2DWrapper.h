#ifndef BOX2D_WRAPPER_H
#define BOX2D_WRAPPER_H

#include "Vec2F.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void Box2DWorld;

Box2DWorld* Box2DWorldCreate(Vec2F gravity);

#ifdef __cplusplus
}
#endif

#endif
