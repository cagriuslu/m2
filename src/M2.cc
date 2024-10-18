#include <m2/M2.h>
#include <random>
#include <memory>

namespace {
	std::random_device rd;
	std::unique_ptr<std::mt19937> random_number_engine_f;
	std::unique_ptr<std::uniform_real_distribution<float>> random_number_distribution_f;
	std::unique_ptr<std::mt19937> random_number_engine_i;
	std::unique_ptr<std::uniform_int_distribution<uint64_t>> random_number_distribution_i;
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

uint64_t m2::rand() {
	if (!random_number_distribution_i) {
		// Seed with std::random_device
		random_number_engine_i = std::make_unique<std::mt19937>(rd());
		random_number_distribution_i = std::make_unique<std::uniform_int_distribution<uint64_t>>();
	}
	return (*random_number_distribution_i)(*random_number_engine_i);
}

uint64_t m2::rand_nonzero() {
	uint64_t n;
	while ((n = ::m2::rand()) == 0) {}
	return n;
}

float m2::randf() {
	if (!random_number_distribution_f) {
		// Seed with std::random_device
		random_number_engine_f = std::make_unique<std::mt19937>(rd());
		random_number_distribution_f = std::make_unique<std::uniform_real_distribution<float>>(0.0f, 1.0f);
	}
	return (*random_number_distribution_f)(*random_number_engine_f);
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
bool m2::is_one(float a, float tolerance) {
	return is_equal(a, 1.0f, tolerance);
}

size_t m2::utf8_codepoint_count(const char* s) {
	size_t len = 0;
	while (*s) {
		len += ((*s & 0xc0) != 0x80) ? 1 : 0;
		++s;
	}
	return len;
}
