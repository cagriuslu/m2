#ifndef VEC2I_H
#define VEC2I_H

#include <stdbool.h>
#include <stdint.h>

#define VEC2I_ZERO ((Vec2I){0, 0})
#define VEC2I_TO2F(v2i) ((Vec2F) { (float)((v2i).x), (float)((v2i).y) })

// Forward declarations
typedef struct _Vec2F Vec2F;

typedef struct _Vec2I {
	int32_t x, y;
} Vec2I;

Vec2I Vec2I_Add(Vec2I lhs, Vec2I rhs);
bool Vec2I_Equals(Vec2I lhs, Vec2I rhs);

#endif
