#pragma once
#include <tl/expected.hpp>
#include <string>

namespace m2 {
	template <typename T>
	int I(T t) { return static_cast<int>(t); }

	template <typename T>
	unsigned U(T t) { return static_cast<unsigned>(t); }

	template <typename T>
	float F(T t) { return static_cast<float>(t); }

	std::string to_string(int);
	std::string to_string(unsigned int);
	std::string to_string(long);
	std::string to_string(unsigned long);
	std::string to_string(long long);
	std::string to_string(unsigned long long);
	std::string to_string(float);
	std::string to_string(double);
	std::string to_string(const char*);
	std::string to_string(const std::string&);

	template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
	template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

	template <typename InputIt, typename OutputIt, typename UnaryPredicate, typename UnaryOperation>
	OutputIt transform_copy_if(InputIt first, InputIt last, OutputIt destination, UnaryPredicate predicate, UnaryOperation operation) {
		for (; first != last; ++first) {
			if (predicate(*first)) {
				*destination = operation(*first);
				++destination;
			}
		}
		return destination;
	}

	struct Void {};
	using void_expected = tl::expected<void, std::string>;

	template <typename T>
	using expected = tl::expected<T, std::string>;
}
