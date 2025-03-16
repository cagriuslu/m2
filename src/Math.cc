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
		throw M2_ERROR("Max is smaller than minimum");
	}
	return min + iround((max - min) * RandomF());
}
float m2::UniformRandomF(const float min, const float max) {
	if (max < min) {
		throw M2_ERROR("Max is smaller than minimum");
	}
	return min + (max - min) * RandomF();
}
float m2::apply_accuracy(float value, float max_swing, float accuracy) {
	if (accuracy == 1.0f) {
		return value;
	}
	return value + max_swing * (2.0f * RandomF() - 1.0f) * (1.0f - accuracy);
}

bool m2::is_near(float a, float b, float tolerance) {
	return fabsf(b - a) < fabsf(tolerance);
}
bool m2::is_far(float a, float b, float tolerance) {
	return fabsf(tolerance) < fabsf(b - a);
}
bool m2::is_equal(float a, float b, float tolerance) {
	return is_near(a, b, tolerance);
}
bool m2::is_not_equal(float a, float b, float tolerance) {
	return not is_near(a, b, tolerance);
}
bool m2::is_less(float a, float b, float tolerance) {
	return fabsf(tolerance) < (b - a);
}
bool m2::is_less_or_equal(float a, float b, float tolerance) {
	return fabsf(tolerance) < (b - a) || is_near(a, b, tolerance);
}
bool m2::is_zero(float a, float tolerance) {
	return is_equal(a, 0.0f, tolerance);
}
bool m2::is_nonzero(float a, float tolerance) {
	return not is_zero(a, tolerance);
}
bool m2::is_positive(float a, float tolerance) {
	return m2::is_less(0.0f, a, tolerance);
}
bool m2::is_negative(float a, float tolerance) {
	return m2::is_less(a, 0.0f, tolerance);
}
bool m2::is_one(float a, float tolerance) {
	return is_equal(a, 1.0f, tolerance);
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
