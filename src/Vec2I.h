#ifndef VEC2I_H
#define VEC2I_H

#include <stdbool.h>
#include <stdint.h>

// Forward declarations
typedef struct _Vec2F Vec2F;

typedef struct _Vec2I {
	int32_t x, y;
} Vec2I;

#define VEC2I_ZERO ((Vec2I){0, 0})
#define VEC2I(x,y) ((Vec2I){(int32_t)(x), (int32_t)(y)})

Vec2I Vec2I_Add(Vec2I lhs, Vec2I rhs);

bool Vec2I_Equals(Vec2I lhs, Vec2I rhs);

Vec2I Vec2I_From2F(Vec2F v);

#endif
