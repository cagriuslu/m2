#pragma once
#include "Exception.h"
#include <tl/expected.hpp>
#include <string>

namespace m2 {
	int I(auto t) { return static_cast<int>(t); }
	unsigned U(auto t) { return static_cast<unsigned>(t); }
	float F(auto t) { return static_cast<float>(t); }
	std::string S(const auto& s) { return std::string(s); }
	inline int iround(float t) { return static_cast<int>(roundf(t)); }
	inline unsigned uround(float t) { return static_cast<unsigned>(roundf(t)); }

	std::string to_string(int);
	std::string to_string(unsigned int);
	std::string to_string(long);
	std::string to_string(unsigned long);
	std::string to_string(long long);
	std::string to_string(unsigned long long);
	std::string to_string(float);
	std::string to_string(float, unsigned precision);
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

	template <typename It>
	bool is_identical(It first1, It last1, It first2, It last2) {
		for (; first1 != last1 && first2 != last2; ++first1, ++first2) {
			if (*first1 != *first2) {
				return false;
			}
		}
		// Check if end has been reached for both container
		return first1 == last1 && first2 == last2;
	}

	struct Void {};
	using void_expected = tl::expected<void, std::string>;

	template <typename T>
	using expected = tl::expected<T, std::string>;

	template <typename E>
	auto make_unexpected(E&& e) { return tl::make_unexpected(std::forward<E>(e)); }
}

#define m2_reflect_failure(v)              \
	do {                                   \
		if (!(v)) {                        \
			return ::m2::make_unexpected(  \
					std::move((v).error()) \
			);                             \
		}                                  \
	} while (false)

#define m2_fail_unless(cond, err)                  \
	do {                                           \
		if (!(cond)) {                             \
				return ::m2::make_unexpected(err); \
		}                                          \
	} while (false)

#define m2_throw_failure_as_fatal(v)                            \
	do {                                                        \
		if (!(v)) {                                             \
			throw ::m2::Fatal(__FILE__, __LINE__, (v).error()); \
		}                                                       \
	} while (false)

#define m2_throw_failure_as_error(v)                            \
	do {                                                        \
		if (!(v)) {                                             \
			throw ::m2::Error(__FILE__, __LINE__, (v).error()); \
		}                                                       \
	} while (false)
