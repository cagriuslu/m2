#ifndef M2_M2_H
#define M2_M2_H

#include <string>

#define MAYBE [[maybe_unused]]
#define IF(cond) if (cond) cond

namespace m2 {
	constexpr float PI = 3.141592653589793f;

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

	template <typename T>
	constexpr int to_int(T&& t) { return static_cast<int>(t); }

	template <typename T>
	constexpr int to_unsigned(T&& t) { return static_cast<unsigned>(t); }

	std::string to_string(int);
	std::string to_string(unsigned int);
	std::string to_string(long);
	std::string to_string(unsigned long);
	std::string to_string(long long);
	std::string to_string(unsigned long long);
	std::string to_string(float);
	std::string to_string(double);
	std::string to_string(const char*);
}

// Variant helper classes
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// The usual suspects
#include "Pool.hh"
#include "Vec2f.h"
#include "Vec2i.h"

#endif //M2_M2_H
