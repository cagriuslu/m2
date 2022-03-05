#include "Def.h"

float NORMALIZE_2PI(float angle) {
	float n = fmodf(angle, 2 * M2_PI);
	if (n < 0.0f) {
		return n + 2 * M2_PI;
	}
	return n;
}
