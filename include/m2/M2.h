#ifndef M2_M2_H
#define M2_M2_H

#include <string>

namespace m2 {
	/// Generates from set [0, max)
	uint32_t rand(uint32_t max);
	uint64_t rand(uint64_t max);

	float randf();

	std::string round_string(float f);

	float apply_accuracy(float value, float accuracy);

	float lerp(float min, float max, float ratio);

	float min(float a, float b);
	float max(float a, float b);

	float normalize_rad(float radians);
}

#endif //M2_M2_H
