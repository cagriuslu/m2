#ifndef M2_M2_H
#define M2_M2_H

#include <string>
#include <array>

#define MAYBE [[maybe_unused]]
#define IF(cond) if (cond) cond

namespace m2 {
	////////////////////////////////////////////////////////////////////////
	/////////////////////////// META PROGRAMMING ///////////////////////////
	////////////////////////////////////////////////////////////////////////
	template <typename T>
	constexpr int to_int(T&& t) { return static_cast<int>(t); }

	template <typename T>
	constexpr unsigned to_unsigned(T&& t) { return static_cast<unsigned>(t); }

	std::string to_string(int);
	std::string to_string(unsigned int);
	std::string to_string(long);
	std::string to_string(unsigned long);
	std::string to_string(long long);
	std::string to_string(unsigned long long);
	std::string to_string(float);
	std::string to_string(double);
	std::string to_string(const char*);

	template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
	template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

	template<typename T, size_t N>
	constexpr std::array<T, N> make_array(T value) {
		std::array<T, N> a{};
		for (auto& x : a)
			x = value;
		return a;
	}

	////////////////////////////////////////////////////////////////////////
	///////////////////////////////// MATH /////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	constexpr float PI = 3.141592653589793f;
	constexpr float PI_MUL2 = 6.283185307179586f;
	constexpr float PI_DIV2 = 1.570796326794897f;
	constexpr float SQROOT_2 = 1.414213562373095f;

	uint32_t rand(uint32_t max); /// Generates numbers from set [0, max)
	uint64_t rand(uint64_t max); /// Generates numbers from set [0, max)
	float randf(); /// Generates numbers from set [0.0f, 1.0f)
	float apply_accuracy(float value, float accuracy); /// Returns value ± value * accuracy%
	bool is_near(float a, float b, float tolerance);
}

#endif //M2_M2_H
