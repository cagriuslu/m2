#pragma once
#include "Error.h"
#include <tl/expected.hpp>
#include <string>
#include <vector>
#include <variant>
#include <set>
#include <ranges>
#include <optional>
#include <tuple>
#include <memory>
#include <cmath>

#define DEFAULT_OVERLOAD [](MAYBE const auto& _){}

namespace m2 {
	constexpr int I(const auto t) { return static_cast<int>(t); }
	constexpr unsigned U(const auto t) { return static_cast<unsigned>(t); }
	constexpr uint8_t U8(const auto t) { return static_cast<uint8_t>(t); }
	constexpr size_t Z(const auto t) { return static_cast<size_t>(t); }
	constexpr float F(const auto t) { return static_cast<float>(t); }
	constexpr double D(const auto t) { return static_cast<double>(t); }
	std::string S(const auto& s) { return std::string(s); }
	inline int RoundI(const float t) { return static_cast<int>(roundf(t)); }
	inline int RoundI(const double t) { return static_cast<int>(round(t)); }
	inline unsigned RoundU(const float t) { return static_cast<unsigned>(roundf(t)); }
	inline uint8_t RoundU8(const float t) { return static_cast<uint8_t>(roundf(t)); }
	inline size_t RoundZ(const float t) { return static_cast<size_t>(roundf(t)); }
	inline int FloorI(const float t) { return static_cast<int>(floorf(t)); }
	inline int CeilI(const float t) { return static_cast<int>(ceilf(t)); }
	int RoundDownToEvenI(float);

	template <typename... Types>
	constexpr int I(const std::tuple<Types...>& v) {
		return std::get<int>(v);
	}
	template <typename... Types>
	constexpr int I(const std::variant<Types...>& v) {
		return std::get<int>(v);
	}

	inline std::string ToString(const bool b) { return b ? "true" : "false"; }
	inline std::string ToString(const int i) { return std::to_string(i); }
	inline std::string ToString(const unsigned int u) { return std::to_string(u); }
	inline std::string ToString(const long l) { return std::to_string(l); }
	inline std::string ToString(const unsigned long ul) { return std::to_string(ul); }
	inline std::string ToString(const long long ll) { return std::to_string(ll); }
	inline std::string ToString(const unsigned long long ull) { return std::to_string(ull); }
	inline std::string ToString(const float f) { return std::to_string(f); }
	std::string ToString(float, unsigned precision);
	inline std::string ToString(const double d) { return std::to_string(d); }
	inline std::string ToString(const char* c) { return {c}; }
	inline std::string ToString(const std::string& s) { return s; }
	inline std::string ToString(const std::string_view sv) { return std::string{sv}; }
	template <typename T, typename U> std::string ToString(const std::pair<T,U>& pair) {
		return "(" + ToString(pair.first) + "," + ToString(pair.second) + ")";
	}

	template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
	template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

	// Range utilities

	template <typename FirstT, typename SecondT>
	constexpr auto ToFirst(const std::pair<FirstT,SecondT>& p) { return p.first; }
	template <typename FirstT, typename SecondT>
	constexpr auto ToSecond(const std::pair<FirstT,SecondT>& p) { return p.second; }

	template <typename FirstT, typename SecondT>
	constexpr std::function<bool(const std::pair<FirstT,SecondT>&)> IsFirstEquals(const FirstT& f) {
		return [&f](const std::pair<FirstT,SecondT>& p) -> bool {
			return p.first == f;
		};
	}
	template <typename FirstT, typename SecondT>
	constexpr std::function<bool(const std::pair<FirstT,SecondT>&)> IsFirstNotEquals(const FirstT& f) {
		return [&f](const std::pair<FirstT,SecondT>& p) -> bool {
			return p.first != f;
		};
	}
	template <typename FirstT, typename SecondT>
	constexpr std::function<bool(const std::pair<FirstT,SecondT>&)> IsSecondEquals(const SecondT& s) {
		return [&s](const std::pair<FirstT,SecondT>& p) -> bool {
			return p.second == s;
		};
	}
	template <typename FirstT, typename SecondT>
	constexpr std::function<bool(const std::pair<FirstT,SecondT>&)> IsSecondNotEquals(const SecondT& s) {
		return [&s](const std::pair<FirstT,SecondT>& p) -> bool {
			return p.second != s;
		};
	}

	// Converts a range into std::vector
	template<std::ranges::range R>
	constexpr auto ToVector(R&& r) {
		using elem_t = std::decay_t<std::ranges::range_value_t<R>>;
		return std::vector<elem_t>{r.begin(), r.end()};
	}
	// Converts a range into std::set
	template<std::ranges::range R>
	constexpr auto ToSet(R&& r) {
		using elem_t = std::decay_t<std::ranges::range_value_t<R>>;
		return std::set<elem_t>{r.begin(), r.end()};
	}

	// Algorithms

	template <typename InputIt, typename OutputIt, typename UnaryPredicate, typename UnaryOperation>
	OutputIt TransformIf(InputIt first, InputIt last, OutputIt destination, UnaryPredicate predicate, UnaryOperation operation) {
		for (; first != last; ++first) {
			if (predicate(*first)) {
				*destination = operation(*first);
				++destination;
			}
		}
		return destination;
	}

	template <typename InputIt, typename Operation>
	void ForEachAdjacentPair(InputIt first, InputIt last, Operation operation) {
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

	namespace detail {
		template <typename T>
		T&& _move_or_throw_error(const char* file, const int line, expected<T>&& e) {
			if (!e) {
				throw Error{file, line, e.error()};
			}
			return std::move(*e);
		}
		template <typename T>
		T&& _move_or_throw_message(const char* file, const int line, std::optional<T>&& o, const char* message) {
			if (!o) {
				throw Error{file, line, message};
			}
			return std::move(*o);
		}

		template <typename T>
		void _succeed_or_throw_error(const char* file, const int line, const expected<T>& e) {
			if (!e) {
				throw Error{file, line, e.error()};
			}
		}

		void _succeed_or_throw_message(const char* file, const int line, const auto& o, const char* message) {
			if (!o) {
				throw Error{file, line, message};
			}
		}
		void _succeed_or_throw_message(const char* file, const int line, const auto& o, const std::string& message) {
			if (!o) {
				throw Error{file, line, message};
			}
		}

		void _expect_zero_or_throw_message(const char* file, const int line, const auto integralType, const char* message) {
			if (integralType) {
				throw Error{file, line, message};
			}
		}
	}
}

#define m2ReflectUnexpected(expected_type)             \
	do {                                               \
		if (not (expected_type)) {                     \
			return ::m2::make_unexpected(              \
					std::move((expected_type).error()) \
			);                                         \
		}                                              \
	} while (false)

#define m2ReturnUnexpectedUnless(condition, msg)            \
	do {                                                    \
		if (not (condition)) {                              \
				return ::m2::make_unexpected(msg);          \
		}                                                   \
	} while (false)

/// Return the r-value reference to v if it contains a value, otherwise throw the contained error
#define m2MoveOrThrowError(expected_type) (::m2::detail::_move_or_throw_error(__FILE__, __LINE__, (expected_type)))
/// Return the r-value reference to v if it contains a value, otherwise throw the message
#define m2MoveOrThrowMessage(optional_type, msg) (::m2::detail::_move_or_throw_message(__FILE__, __LINE__, (optional_type), (msg)))

/// Do nothing if v contains a value, otherwise throw the contained error
#define m2SucceedOrThrowError(expected_type) (::m2::detail::_succeed_or_throw_error(__FILE__, __LINE__, (expected_type)))
/// Do nothing if optional_type contains a value, otherwise throw the message
#define m2SucceedOrThrowMessage(optional_type, msg) (::m2::detail::_succeed_or_throw_message(__FILE__, __LINE__, (optional_type), (msg)))
/// Do nothing if integral_type is zero, otherwise throw the message
#define m2ExpectZeroOrThrowMessage(integral_type, msg) (::m2::detail::_expect_zero_or_throw_message(__FILE__, __LINE__, (integral_type), (msg)))

#define _m2TokenConcat(x, y) x ## y
#define m2TokenConcat(x, y) _m2TokenConcat(x, y)
#define m2Repeat(n) for (int m2TokenConcat(_i, __LINE__) = 0; m2TokenConcat(_i, __LINE__) < (n); ++m2TokenConcat(_i, __LINE__))
