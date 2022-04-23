#include <m2/M2.h>
#include <m2/Def.h>
#include <random>
#include <memory>

std::unique_ptr<std::mt19937> random_number_engine;
std::unique_ptr<std::uniform_real_distribution<float>> random_number_distribution;

float m2::randf() {
	if (!random_number_distribution) {
		// Seed with std::random_device
		std::random_device rd;
		random_number_engine = std::make_unique<std::mt19937>(rd());
		random_number_distribution = std::make_unique<std::uniform_real_distribution<float>>(0.0f, 1.0f);
	}
	return (*random_number_distribution)(*random_number_engine);
}

std::string m2::round_string(float f) {
	return std::to_string( static_cast<int>(roundf(f)));
}

float m2::apply_accuracy(float value, float accuracy) {
	return value + (1.0f - accuracy) * value * (randf() - 0.5f);
}

float m2::lerp(float min, float max, float ratio) {
	return min + (max - min) * ratio;
}

float m2::min(float a, float b) {
	return a < b ? a : b;
}

float m2::max(float a, float b) {
	return a < b ? b : a;
}

float m2::normalize_rad(float radians) {
	float n = fmodf(radians, 2 * M2_PI);
	if (n < 0.0f) {
		return n + 2 * M2_PI;
	}
	return n;
}
