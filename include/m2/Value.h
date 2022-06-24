#ifndef M2_VALUE_H
#define M2_VALUE_H

#include <variant>

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
	class Value {
		std::variant<T, Failure<E>> _variant;

	public:
		Value() : _variant(T{}) {}
		Value(Failure<E>&& _f) : _variant(_f) {}

		operator bool() const {
			return has_value();
		}
		bool has_value() const {
			return _variant.index() == 0;
		}
		T& value() {
			return std::get<0>(_variant);
		}
		bool has_error() const {
			return not has_value();
		}
		const E& error() const {
			return std::get<1>(_variant)._e;
		}
	};
}

#endif //M2_VALUE_H
