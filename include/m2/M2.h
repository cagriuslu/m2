#ifndef M2_M2_H
#define M2_M2_H

#include <string>
#include <variant>

#define C(s) ((const char*)(s))

namespace m2 {
	constexpr long long ll(unsigned long long ull) { return static_cast<long long>(ull); }

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

#endif //M2_M2_H
