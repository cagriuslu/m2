#pragma once
#include <string>
#include <optional>
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
	float ApplyAccuracy(float value, float max_swing, float accuracy); /// Returns value ± value * accuracy% TODO rename to something more generic

	// Floating point comparisons

	bool IsEqual(float a, float b, float tolerance); // a == b
	bool IsNotEqual(float a, float b, float tolerance); // a != b
	bool IsLess(float a, float b, float tolerance); // a < b
	bool IsLessOrEqual(float a, float b, float tolerance); // a <= b
	bool IsZero(float a, float tolerance);
	bool IsNonZero(float a, float tolerance);
	bool IsPositive(float a, float tolerance);
	bool IsNegative(float a, float tolerance);
	bool IsOne(float a, float tolerance);

	constexpr float ToRadians(const float degrees) { return degrees * PI / 180.0f; }
	constexpr float ToRadians(const int degrees) { return ToRadians(static_cast<float>(degrees)); }
	constexpr float ToDegrees(const float radians) { return radians / PI * 180.0f; }
	/// Returned value is in-between [0, 2*PI).
	float ClampRadiansTo2Pi(float rads);
	/// Returned value is in-between [-PI, PI).
	float ClampRadiansToPi(float rads);
	/// Returns the smaller angle (in radians, always positive) between the two given angles. The given angles will be
	/// clamped before difference calculation.
	float AngleAbsoluteDifference(float rads1, float rads2);
	/// Returns rads1 - rads2, clamped to [-PI, PI). The given angles will be clamped before difference calculation.
	float AngleDifference(float rads1, float rads2);

	/// String to number conversion
	std::optional<float> ToFloat(const std::string&);

	/// Finds the closes bin the value falls into if the unit value (1) is split into `unitBinCount` bins.
	/// Ex. if unitBinCount is 4, possible values are 0, 0.25, 0.5, 0.75, 1, 1.25, 1.5, ...
	float RoundToBin(float value, int unitBinCount);

	/// Attempts to normalize the given value to [0,1] if it's in-between [min,max]. Otherwise, a scaled value is
	/// returned. For example, if the value is min+2*max, 2.0 is returned. If the value is min-max, -1.0 is returned.
	float Normalize(float value, float min, float max);

	template <typename T> T AtLeastZero(T value) { if (value < T{}) { return T{}; } return value; }
	template <typename T> T AtMostZero(T value) { if (T{} < value) { return T{}; } return value; }
}
