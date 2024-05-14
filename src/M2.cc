#include <m2/M2.h>
#include <random>
#include <memory>

namespace {
	std::unique_ptr<std::mt19937> random_number_engine;
	std::unique_ptr<std::uniform_real_distribution<float>> random_number_distribution;
}
const std::string m2::empty_string;

std::string m2::to_string(BackgroundLayer layer) {
	return to_string(I(layer));
}

uint32_t m2::rand(uint32_t max) {
	return static_cast<uint32_t>(static_cast<float>(max) * m2::randf());
}

uint64_t m2::rand(uint64_t max) {
	return static_cast<uint64_t>(static_cast<double>(max) * m2::randf());
}

float m2::randf() {
	if (!random_number_distribution) {
		// Seed with std::random_device
		std::random_device rd;
		random_number_engine = std::make_unique<std::mt19937>(rd());
		random_number_distribution = std::make_unique<std::uniform_real_distribution<float>>(0.0f, 1.0f);
	}
	return (*random_number_distribution)(*random_number_engine);
}

float m2::apply_accuracy(float value, float max_swing, float accuracy) {
	if (accuracy == 1.0f) {
		return value;
	}
	return value + max_swing * (2.0f * randf() - 1.0f) * (1.0f - accuracy);
}

bool m2::is_near(float a, float b, float tolerance) {
	return fabsf(b - a) < fabsf(tolerance);
}
bool m2::is_far(float a, float b, float tolerance) {
	return fabsf(tolerance) < fabsf(b - a);
}
bool m2::is_less(float a, float b, float tolerance) {
	return fabsf(tolerance) < (b - a);
}
bool m2::is_less_or_equal(float a, float b, float tolerance) {
	return fabsf(tolerance) < (b - a) || is_near(a, b, tolerance);
}
