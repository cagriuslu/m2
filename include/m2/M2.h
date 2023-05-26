#pragma once
#include "Meta.h"

#define MAYBE [[maybe_unused]]
#define IF(cond) if (cond) cond

namespace m2 {
	constexpr float PI = 3.141592653589793f;
	constexpr float PI_MUL2 = 6.283185307179586f;
	constexpr float PI_MUL3_DIV2 = 4.71238898038469f;
	constexpr float PI_DIV2 = 1.570796326794897f;
	constexpr float SQROOT_2 = 1.414213562373095f;

	uint32_t rand(uint32_t max); /// Generates numbers from set [0, max)
	uint64_t rand(uint64_t max); /// Generates numbers from set [0, max)
	float randf(); /// Generates numbers from set [0.0f, 1.0f)
	float apply_accuracy(float value, float max_swing, float accuracy); /// Returns value ± value * accuracy%
	bool is_near(float a, float b, float tolerance);
	constexpr float to_radians(float degrees) { return degrees / 180.0f * ::m2::PI; }
	constexpr float to_radians(int degrees) { return to_radians(static_cast<float>(degrees)); }
	constexpr float to_degrees(float radians) { return radians / ::m2::PI * 180.0f; }
}
