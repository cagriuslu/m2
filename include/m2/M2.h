#ifndef M2_M2_H
#define M2_M2_H

namespace m2 {
	float randf();

	float apply_accuracy(float value, float accuracy);

	float lerp(float min, float max, float ratio);

	float min(float a, float b);
	float max(float a, float b);

	float normalize_rad(float radians);
}

#endif //M2_M2_H
