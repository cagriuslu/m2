#pragma once
#include <cstdint>

namespace m2 {
	constexpr float PI = 3.141592653589793f;
	constexpr float PI_MUL2 = 6.283185307179586f;
	constexpr float PI_MUL3_DIV2 = 4.71238898038469f;
	constexpr float PI_DIV2 = 1.570796326794897f;
	constexpr float SQROOT_2 = 1.414213562373095f;

	/// Generates a number in [0,max). Max should be much smaller than 2^32.
	uint32_t Random(uint32_t max);
	/// Generates a number in [0,max). Max should be much smaller than 2^64.
	uint64_t Random64(uint64_t max);
	/// Generates a 64-bit random number.
	uint64_t Random64();
	/// Generates a non-zero 64-bit random number.
	uint64_t RandomNonZero64();
	/// Generates a number in [0.0f, 1.0f).
	float RandomF();
	int UniformRandom(int min, int max);
	float UniformRandomF(float min, float max);
	float apply_accuracy(float value, float max_swing, float accuracy); /// Returns value ± value * accuracy% TODO rename to something more generic

	// Floating point comparisons

	bool is_near(float a, float b, float tolerance); // a == b
	bool is_far(float a, float b, float tolerance); // a != b
	bool is_equal(float a, float b, float tolerance); // a == b
	bool is_not_equal(float a, float b, float tolerance); // a != b
	bool is_less(float a, float b, float tolerance); // a < b
	bool is_less_or_equal(float a, float b, float tolerance); // a <= b
	bool is_zero(float a, float tolerance);
	bool is_nonzero(float a, float tolerance);
	bool is_positive(float a, float tolerance);
	bool is_negative(float a, float tolerance);
	bool is_one(float a, float tolerance);

	constexpr float to_radians(const float degrees) { return degrees * PI / 180.0f; }
	constexpr float to_radians(const int degrees) { return to_radians(static_cast<float>(degrees)); }
	constexpr float to_degrees(const float radians) { return radians / ::m2::PI * 180.0f; }
	/// Returned value is in-between [0, 2*PI).
	float ClampRadiansTo2Pi(float rads);
	/// Returned value is in-between [-PI, PI).
	float ClampRadiansToPi(float rads);
	/// Returns the smaller angle (in radians, always positive) between the two given angles. The given angles will be
	/// clamped before difference calculation.
	float AngleAbsoluteDifference(float rads1, float rads2);
	/// Returns rads1 - rads2, clamped to [-PI, PI). The given angles will be clamped before difference calculation.
	float AngleDifference(float rads1, float rads2);

	/// Attempts to normalize the given value to [0,1] if it's in-between [min,max]. Otherwise, a scaled value is
	/// returned. For example, if the value is min+2*max, 2.0 is returned. If the value is min-max, -1.0 is returned.
	float Normalize(float value, float min, float max);
}
