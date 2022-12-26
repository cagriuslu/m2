#ifndef M2_NOISE_H
#define M2_NOISE_H

#include "Vec2f.h"

namespace m2 {
	float perlin(const Vec2f& point, float depth); // [0, 1]
}

#endif //M2_NOISE_H
