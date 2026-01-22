#pragma once
#include <m2/math/primitives/Exact.h>
#include <m2/math/primitives/Float.h>
#include <m2/ProxyTypes.h>
#include <IVFE.pb.h>

namespace m2 {
	class IVFE {
		std::variant<std::monostate, int32_t, m2g::pb::VariableType, FE> _value{};

	public:
		IVFE() = default;
		explicit IVFE(const int32_t i) : _value(i) {}
		explicit IVFE(const m2g::pb::VariableType vt) : _value(vt) {}
		explicit IVFE(FE&& fe) : _value(fe) {}
		explicit IVFE(const FE& fe) : _value(fe) {}
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		explicit IVFE(const float f) requires (Enable) : _value(FE{f}) {}
		explicit IVFE(const pb::IVFE&);

		explicit operator bool() const;
		explicit operator pb::IVFE() const;

		[[nodiscard]] bool IsNull() const { return std::holds_alternative<std::monostate>(_value); }
		[[nodiscard]] bool IsNonNull() const { return not IsNull(); }
		[[nodiscard]] bool IsInt() const { return std::holds_alternative<int32_t>(_value); }
		[[nodiscard]] bool IsVariableType() const { return std::holds_alternative<m2g::pb::VariableType>(_value); }
		[[nodiscard]] bool IsFE() const { return std::holds_alternative<FE>(_value); }
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		[[nodiscard]] bool IsF() const requires (Enable) { return std::holds_alternative<FE>(_value); }

		[[nodiscard]] int32_t UnsafeGetInt() const { return std::get<int32_t>(_value); }
		[[nodiscard]] int32_t GetIntOrZero() const;
		[[nodiscard]] int32_t GetIntOrValue(int32_t defaultValue) const;

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

		[[nodiscard]] IVFE UnsafeAdd(const IVFE&) const;
		[[nodiscard]] IVFE UnsafeSubtract(const IVFE&) const;
		[[nodiscard]] IVFE Negate() const;
	};
}
