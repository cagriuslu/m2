#include "Def.h"

float NORMALIZE_2PI(float angle) {
	float n = fmodf(angle, 2 * X_PI);
	if (n < 0.0f) {
		return n + 2 * X_PI;
	}
	return n;
}
