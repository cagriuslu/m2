#ifndef M2_VALUE_H
#define M2_VALUE_H

#include "Exception.h"
#include <variant>
#include <functional>
#include <optional>
#include <string>

namespace m2 {
	template <typename E>
	struct Failure {
		E _e;
		explicit Failure(const E& e) : _e(e) {}
		explicit Failure(E&& e) : _e(std::move(e)) {}

		void throw_fatal(const char* file, int line) const {
			throw Fatal(file, line, _e);
		}
		void throw_error(const char* file, int line) const {
			throw Error(file, line, _e);
		}
	};

	template <typename E>
	Failure<E> failure(E&& e) {
		return Failure<E>(std::forward<E>(e));
	}
	inline Failure<std::string> failure(const char* e) { return Failure<std::string>(std::string{e}); }
	inline Failure<std::string> failure(const std::string& e) { return Failure<std::string>(e); }

	template <typename T, typename E = std::string>
	class Value : public std::variant<T,Failure<E>> {
	public:
		Value() : std::variant<T,Failure<E>>(T{}) {}
		Value(const T& t) : std::variant<T,Failure<E>>(t) {}
		Value(T&& t) : std::variant<T,Failure<E>>(std::move(t)) {}
		Value(Failure<E>&& f) : std::variant<T,Failure<E>>(std::move(f)) {}

		operator bool() const {
			return has_value();
		}

		[[nodiscard]] bool has_value() const {
			return this->index() == 0;
		}
		T& value() {
			return std::get<0>(*this);
		}
		T& operator*() {
			return value();
		}
		T* operator->() {
			return &value();
		}

		[[nodiscard]] bool has_error() const {
			return !has_value();
		}
		const E& error() const {
			return std::get<1>(*this)._e;
		}
		std::optional<Failure<E>> failure() const {
			if (has_error()) {
				return Failure<E>(error());
			}
			return {};
		}
	};

	struct Void {};
	using VoidValue = Value<Void>;

	template <typename T, typename E>
	bool assign_if(const Value<T,E>& v, T& t) {
		if (v) {
			t = v.value();
			return true;
		}
		return false;
	}

	template <typename T, typename E>
	bool assign_if(Value<T,E>&& v, T& t) {
		if (v) {
			t = std::move(v.value());
			return true;
		}
		return false;
	}

	template <typename T, typename E>
	bool exec_if(Value<T,E>&& v, std::function<void(T&)> f) {
		if (v) {
			f(v.value());
			return true;
		}
		return false;
	}

	template <typename R, typename T, typename E>
	R transform_if(Value<T,E>&& v, std::function<R(T&)> f, R&& def = R{}) {
		if (v) {
			return f(v.value());
		}
		return def;
	}
}

#define m2_reflect_failure(v)               \
	do {                                    \
		auto __failure__ = (v).failure();   \
		if (__failure__) {                  \
			return std::move(*__failure__); \
		}                                   \
	} while (false)

#define m2_fail_unless(cond, err)        \
	do {                                 \
		if (!(cond)) {                   \
			return ::m2::failure((err)); \
		}                                \
	} while (false)

#define m2_throw_failure_fatal(v)                       \
	do {                                                \
		auto __failure__ = (v).failure();               \
		if (__failure__) {                              \
			__failure__->throw_fatal(__FILE__, __LINE__); \
		}                                               \
	} while (false)

#define m2_throw_failure_error(v)                       \
	do {                                                \
		auto __failure__ = (v).failure();               \
		if (__failure__) {                              \
			__failure__->throw_error(__FILE__, __LINE__); \
		}                                               \
	} while (false)

#endif //M2_VALUE_H
