#ifndef M2_VALUE_H
#define M2_VALUE_H

#include <variant>
#include <functional>

#define C(s) ((const char*)(s))

namespace m2 {
	struct Void {};

	template <typename E>
	struct Failure {
		E _e;
		explicit constexpr Failure(E&& e) : _e(std::move(e)) {}
	};

	template <typename E = const char*>
	constexpr Failure<E> failure(E&& e) {
		return Failure<E>(std::forward<E>(e));
	}

	template <typename T, typename E = const char*>
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

		[[nodiscard]] bool has_error() const {
			return !has_value();
		}
		const E& error() const {
			return std::get<1>(*this)._e;
		}
	};

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
}

#endif //M2_VALUE_H
