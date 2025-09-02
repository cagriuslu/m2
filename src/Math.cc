#include <m2/Math.h>
#include <random>
#include <cmath>

#include "m2/Error.h"
#include "m2/Meta.h"

namespace {
	std::random_device rd;
	std::unique_ptr<std::mt19937> random_number_engine_f;
	std::unique_ptr<std::uniform_real_distribution<float>> random_number_distribution_f;
	std::unique_ptr<std::mt19937> random_number_engine_i;
	std::unique_ptr<std::uniform_int_distribution<uint64_t>> random_number_distribution_i;
}

uint32_t m2::Random(const uint32_t max) {
	return Random64() % max;
}
uint64_t m2::Random64(const uint64_t max) {
	return Random64() % max;
}
uint64_t m2::Random64() {
	if (not random_number_distribution_i) {
		// Seed with std::random_device
		random_number_engine_i = std::make_unique<std::mt19937>(rd());
		random_number_distribution_i = std::make_unique<std::uniform_int_distribution<uint64_t>>();
	}
	return (*random_number_distribution_i)(*random_number_engine_i);
}
uint64_t m2::RandomNonZero64() {
	uint64_t n;
	while ((n = Random64()) == 0) {}
	return n;
}
float m2::RandomF() {
	if (!random_number_distribution_f) {
		// Seed with std::random_device
		random_number_engine_f = std::make_unique<std::mt19937>(rd());
		random_number_distribution_f = std::make_unique<std::uniform_real_distribution<float>>(0.0f, 1.0f);
	}
	return (*random_number_distribution_f)(*random_number_engine_f);
}
int m2::UniformRandom(const int min, const int max) {
	if (max < min) {
		throw M2_ERROR("Max is smaller than min");
	}
	return min + RoundI((max - min) * RandomF());
}
float m2::UniformRandomF(const float min, const float max) {
	if (max < min) {
		throw M2_ERROR("Max is smaller than min");
	}
	return min + (max - min) * RandomF();
}
float m2::ApplyAccuracy(float value, float max_swing, float accuracy) {
	if (accuracy == 1.0f) {
		return value;
	}
	return value + max_swing * (2.0f * RandomF() - 1.0f) * (1.0f - accuracy);
}

bool m2::IsEqual(float a, float b, float tolerance) {
	return fabsf(b - a) < fabsf(tolerance);
}
bool m2::IsNotEqual(float a, float b, float tolerance) {
	return fabsf(tolerance) < fabsf(b - a);
}
bool m2::IsLess(float a, float b, float tolerance) {
	return fabsf(tolerance) < (b - a);
}
bool m2::IsLessOrEqual(float a, float b, float tolerance) {
	return fabsf(tolerance) < (b - a) || IsEqual(a, b, tolerance);
}
bool m2::IsZero(float a, float tolerance) {
	return IsEqual(a, 0.0f, tolerance);
}
bool m2::IsNonZero(float a, float tolerance) {
	return not IsZero(a, tolerance);
}
bool m2::IsPositive(float a, float tolerance) {
	return m2::IsLess(0.0f, a, tolerance);
}
bool m2::IsNegative(float a, float tolerance) {
	return m2::IsLess(a, 0.0f, tolerance);
}
bool m2::IsOne(float a, float tolerance) {
	return IsEqual(a, 1.0f, tolerance);
}

float m2::ClampRadiansTo2Pi(const float rads) {
	if (const auto firstMod = fmodf(rads, PI_MUL2); firstMod < 0.0f) {
		return firstMod + PI_MUL2;
	} else {
		return firstMod;
	}
}
float m2::ClampRadiansToPi(const float rads) {
	const auto clamped = ClampRadiansTo2Pi(rads);
	if (PI < clamped) {
		return clamped - PI_MUL2;
	}
	return clamped;
}
float m2::AngleAbsoluteDifference(const float rads1, const float rads2) {
	const auto clamped1 = ClampRadiansTo2Pi(rads1);
	const auto clamped2 = ClampRadiansTo2Pi(rads2);
	const auto smaller = std::min(clamped1, clamped2);
	const auto bigger = std::max(clamped1, clamped2);
	const auto diff1 = bigger - smaller;
	const auto diff2 = (smaller + PI_MUL2) - bigger;
	return std::min(diff1, diff2);
}
float m2::AngleDifference(const float rads1, const float rads2) {
	return ClampRadiansToPi(rads1 - rads2);
}

std::optional<float> m2::ToFloat(const std::string& s) {
	try {
		return std::stof(s);
	} catch (...) {
		return std::nullopt;
	}
}

float m2::RoundToBin(const float value, const int unitBinCount) {
	// Extract the whole and fractional part
	const auto floor = floorf(value);
	const auto fractional = value - floor;
	// Apply binning only to the fractional part, we don't want to explode the whole number
	const auto raised = fractional * ToFloat(unitBinCount);
	const auto raisedBinned = roundf(raised);
	const auto binned = raisedBinned / ToFloat(unitBinCount);
	return floor + binned;
}

float m2::Normalize(const float value, const float min, const float max) {
	if (max < min) {
		throw M2_ERROR("Max is smaller than min");
	}
	const auto distanceToOrigin = value - min;
	const auto unit = max - min;
	return distanceToOrigin / unit;
}

float m2::Lerp(const float from, const float to, const float ratio) {
	return from + (to - from) * ratio;
}
