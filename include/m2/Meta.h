#pragma once
#include "Exception.h"
#include <tl/expected.hpp>
#include <string>
#include <vector>
#include <set>
#include <ranges>

#define DEFAULT_OVERLOAD [](MAYBE const auto& _){}

namespace m2 {
	int I(auto t) { return static_cast<int>(t); }
	unsigned U(auto t) { return static_cast<unsigned>(t); }
	size_t Z(auto t) { return static_cast<size_t>(t); }
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
	template <typename T, typename U> std::string to_string(const std::pair<T,U>& pair) {
		return "(" + to_string(pair.first) + "," + to_string(pair.second) + ")";
	}

	template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
	template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

	// Range utilities

	template <typename FirstT, typename SecondT>
	constexpr auto to_first_of(const std::pair<FirstT,SecondT>& p) { return p.first; }
	template <typename FirstT, typename SecondT>
	constexpr auto to_second_of(const std::pair<FirstT,SecondT>& p) { return p.second; }

	template <typename FirstT, typename SecondT>
	constexpr std::function<bool(const std::pair<FirstT,SecondT>&)> is_first_equals(const FirstT& f) {
		return [&f](const std::pair<FirstT,SecondT>& p) -> bool {
			return p.first == f;
		};
	}
	template <typename FirstT, typename SecondT>
	constexpr std::function<bool(const std::pair<FirstT,SecondT>&)> is_first_not_equals(const FirstT& f) {
		return [&f](const std::pair<FirstT,SecondT>& p) -> bool {
			return p.first != f;
		};
	}
	template <typename FirstT, typename SecondT>
	constexpr std::function<bool(const std::pair<FirstT,SecondT>&)> is_second_equals(const SecondT& s) {
		return [&s](const std::pair<FirstT,SecondT>& p) -> bool {
			return p.second == s;
		};
	}
	template <typename FirstT, typename SecondT>
	constexpr std::function<bool(const std::pair<FirstT,SecondT>&)> is_second_not_equals(const SecondT& s) {
		return [&s](const std::pair<FirstT,SecondT>& p) -> bool {
			return p.second != s;
		};
	}

	// Converts a range into an std::vector
	template<std::ranges::range R>
	constexpr auto to_vector(R&& r) {
		using elem_t = std::decay_t<std::ranges::range_value_t<R>>;
		return std::vector<elem_t>{r.begin(), r.end()};
	}
	template<std::ranges::range R>
	constexpr auto to_set(R&& r) {
		using elem_t = std::decay_t<std::ranges::range_value_t<R>>;
		return std::set<elem_t>{r.begin(), r.end()};
	}

	// Algorithms

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

	template <typename InputIt, typename Operation>
	void for_each_adjacent_pair(InputIt first, InputIt last, Operation operation) {
		if(first == last) {
			return;
		}
		InputIt next = first;
		for(++next; next != last; ++first, ++next) {
			operation(*first, *next);
		}
	}

	struct Void {};
	using void_expected = tl::expected<void, std::string>;

	template <typename T>
	using expected = tl::expected<T, std::string>;

	template <typename E>
	auto make_unexpected(E&& e) { return tl::make_unexpected(std::forward<E>(e)); }

	// Convenience functions using expected

	template <typename T>
	T&& _move_or_throw_error(const char* file, int line, expected<T>&& e) {
		if (!e) {
			throw Error{file, line, e.error()};
		}
		return std::move(*e);
	}

	template <typename T>
	T&& _move_or_throw_message(const char* file, int line, std::optional<T>&& o, const char* message) {
		if (!o) {
			throw Error{file, line, message};
		}
		return std::move(*o);
	}

	template <typename T>
	void _succeed_or_throw_error(const char* file, int line, const expected<T>& e) {
		if (!e) {
			throw Error{file, line, e.error()};
		}
	}

	template <typename T>
	void _succeed_or_throw_message(const char* file, int line, const std::optional<T>& o, const char* message) {
		if (!o) {
			throw Error{file, line, message};
		}
	}
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

#define m2_move_or_throw_error(v) (::m2::_move_or_throw_error(__FILE__, __LINE__, (v)))
#define m2_move_or_throw_message(v, msg) (::m2::_move_or_throw_message(__FILE__, __LINE__, (v), msg))
#define m2_succeed_or_throw_error(v) (::m2::_succeed_or_throw_error(__FILE__, __LINE__, (v)))
#define m2_succeed_or_throw_message(v, msg) (::m2::_succeed_or_throw_message(__FILE__, __LINE__, (v), msg))

#define _m2_token_concat(x, y) x ## y
#define m2_token_concat(x, y) _m2_token_concat(x, y)
#define m2_repeat(n) for (int m2_token_concat(_, __LINE__) = 0; m2_token_concat(_, __LINE__) < (n); ++m2_token_concat(_, __LINE__))
