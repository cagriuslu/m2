#ifndef VEC2I_H
#define VEC2I_H

#include <stdint.h>

typedef struct _Vec2I {
	int32_t x, y;
} Vec2I;

Vec2I Vec2IAdd(Vec2I lhs, Vec2I rhs);

#endif
