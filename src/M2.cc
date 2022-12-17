#include <m2/M2.h>
#include <random>
#include <memory>

////////////////////////////////////////////////////////////////////////
/////////////////////////// META PROGRAMMING ///////////////////////////
////////////////////////////////////////////////////////////////////////
std::string m2::to_string(int n) {
	return std::to_string(n);
}

std::string m2::to_string(unsigned int n) {
	return std::to_string(n);
}

std::string m2::to_string(long n) {
	return std::to_string(n);
}

std::string m2::to_string(unsigned long n) {
	return std::to_string(n);
}

std::string m2::to_string(long long n) {
	return std::to_string(n);
}

std::string m2::to_string(unsigned long long n) {
	return std::to_string(n);
}

std::string m2::to_string(float n) {
	return std::to_string(n);
}

std::string m2::to_string(double n) {
	return std::to_string(n);
}

std::string m2::to_string(const char* s) {
	return {s};
}

////////////////////////////////////////////////////////////////////////
///////////////////////////////// MATH /////////////////////////////////
////////////////////////////////////////////////////////////////////////

std::unique_ptr<std::mt19937> random_number_engine;
std::unique_ptr<std::uniform_real_distribution<float>> random_number_distribution;

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

float m2::apply_accuracy(float value, float accuracy) {
	if (accuracy == 1.0f) {
		return value;
	}
	return value + (1.0f - accuracy) * value * (randf() - 0.5f);
}

float m2::lerp(float min, float max, float ratio) {
	return min + (max - min) * ratio;
}

bool m2::is_near(float a, float b, float tolerance) {
	return fabsf(a - b) < tolerance;
}
