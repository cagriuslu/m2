#ifndef VEC2I_H
#define VEC2I_H

#include <stdbool.h>
#include <stdint.h>

typedef struct _Vec2I {
	int32_t x, y;
} Vec2I;

Vec2I Vec2IAdd(Vec2I lhs, Vec2I rhs);
bool Vec2IEquals(Vec2I lhs, Vec2I rhs);

#endif
