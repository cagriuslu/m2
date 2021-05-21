#include "Box2DWrapper.h"
#include <b2_world.h>
#include <b2_math.h>

// Utils
#define ToVec2(vec2f) (b2Vec2{vec2f.x, vec2f.y})

Box2DWorld* Box2DWorldCreate(Vec2F gravity) {
	return new b2World(ToVec2(gravity));
}
