#pragma once
#include <m2/math/primitives/Exact.h>
#include <m2/math/primitives/Float.h>
#include <m2/ProxyTypes.h>
#include <VariableValue.pb.h>

namespace m2 {
	// TODO rename to something more generic so that we don't have to add a new letter every time we add a new type
	// ReSharper disable CppNonExplicitConvertingConstructor
	class VariableValue {
		std::variant<std::monostate, int32_t, int64_t, m2g::pb::VariableType, FE> _value{};

	public:
		VariableValue() = default;
		VariableValue(const int32_t i) : _value(i) {}
		VariableValue(const int64_t l) : _value(l) {}
		VariableValue(const uint64_t l) : _value(static_cast<int64_t>(l)) {}
		VariableValue(const m2g::pb::VariableType vt) : _value(vt) {}
		VariableValue(FE&& fe) : _value(fe) {}
		VariableValue(const FE& fe) : _value(fe) {}
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		VariableValue(const float f) requires (Enable) : _value(FE{f}) {}
		VariableValue(const pb::VariableValue&);

		explicit operator bool() const;
		explicit operator pb::VariableValue() const;

		[[nodiscard]] bool IsNull() const { return std::holds_alternative<std::monostate>(_value); }
		[[nodiscard]] bool IsNonNull() const { return not IsNull(); }
		[[nodiscard]] bool IsInt() const { return std::holds_alternative<int32_t>(_value); }
		[[nodiscard]] bool IsLong() const { return std::holds_alternative<int64_t>(_value); }
		[[nodiscard]] bool IsVariableType() const { return std::holds_alternative<m2g::pb::VariableType>(_value); }
		[[nodiscard]] bool IsFE() const { return std::holds_alternative<FE>(_value); }
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		[[nodiscard]] bool IsF() const requires (Enable) { return std::holds_alternative<FE>(_value); }

		[[nodiscard]] int32_t Hash(int32_t initialValue) const;

		[[nodiscard]] int32_t UnsafeGetInt() const { return std::get<int32_t>(_value); }
		[[nodiscard]] int32_t GetIntOrZero() const;
		[[nodiscard]] int32_t GetIntOrValue(int32_t defaultValue) const;

		[[nodiscard]] int64_t UnsafeGetLong() const { return std::get<int64_t>(_value); }
		[[nodiscard]] int64_t GetLongOrZero() const;
		[[nodiscard]] int64_t GetLongOrValue(int64_t defaultValue) const;

		[[nodiscard]] m2g::pb::VariableType UnsafeGetVariableType() const { return std::get<m2g::pb::VariableType>(_value); }
		[[nodiscard]] int32_t GetVariableTypeOrZero() const;
		[[nodiscard]] int32_t GetVariableTypeOrValue(m2g::pb::VariableType defaultValue) const;

		[[nodiscard]] FE UnsafeGetFE() const { return std::get<FE>(_value); }
		[[nodiscard]] FE GetFEOrZero() const;
		[[nodiscard]] FE GetFEOrValue(FE defaultValue) const;

		template <bool Enable = not GAME_IS_DETERMINISTIC>
		[[nodiscard]] float UnsafeGetF() const requires (Enable) { return std::get<FE>(_value).ToFloat(); }
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		[[nodiscard]] float GetFOrZero() const requires (Enable) { return GetFEOrZero().ToFloat(); }
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		[[nodiscard]] float GetFOrValue(const float defaultValue) const requires (Enable) { return GetFEOrValue(FE{defaultValue}).ToFloat(); }

		[[nodiscard]] VariableValue UnsafeAdd(const VariableValue&) const;
		[[nodiscard]] VariableValue UnsafeSubtract(const VariableValue&) const;
		[[nodiscard]] VariableValue Negate() const;
	};
}
