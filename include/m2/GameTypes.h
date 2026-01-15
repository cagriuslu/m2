#pragma once
#include <m2/math/primitives/Exact.h>
#include <m2/math/primitives/Float.h>
#include <m2/ProxyTypes.h>
#include <IFE.pb.h>

namespace m2 {
	// TODO move to math/IFE.h
	class IFE {
		std::variant<std::monostate, int32_t, FE> _value{};

	public:
		IFE() = default;
		explicit IFE(const int32_t i) : _value(i) {}
		explicit IFE(FE&& fe) : _value(fe) {}
		explicit IFE(const FE& fe) : _value(fe) {}
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		explicit IFE(const float f) requires (Enable) : _value(FE{f}) {}
		explicit IFE(const pb::IFE&);

		explicit operator bool() const;
		explicit operator pb::IFE() const;

		[[nodiscard]] bool IsNull() const { return std::holds_alternative<std::monostate>(_value); }
		[[nodiscard]] bool IsNonNull() const { return not IsNull(); }
		[[nodiscard]] bool IsInt() const { return std::holds_alternative<int32_t>(_value); }
		[[nodiscard]] bool IsFE() const { return std::holds_alternative<FE>(_value); }
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		[[nodiscard]] bool IsF() const requires (Enable) { return std::holds_alternative<FE>(_value); }

		[[nodiscard]] int32_t UnsafeGetInt() const { return std::get<int32_t>(_value); }
		[[nodiscard]] int32_t GetIntOrZero() const;
		[[nodiscard]] int32_t GetIntOrValue(int32_t defaultValue) const;

		[[nodiscard]] FE UnsafeGetFE() const { return std::get<FE>(_value); }
		[[nodiscard]] FE GetFEOrZero() const;
		[[nodiscard]] FE GetFEOrValue(FE defaultValue) const;

		template <bool Enable = not GAME_IS_DETERMINISTIC>
		[[nodiscard]] float UnsafeGetF() const requires (Enable) { return std::get<FE>(_value).ToFloat(); }
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		[[nodiscard]] float GetFOrZero() const requires (Enable) { return GetFEOrZero().ToFloat(); }
		template <bool Enable = not GAME_IS_DETERMINISTIC>
		[[nodiscard]] float GetFOrValue(const float defaultValue) const requires (Enable) { return GetFEOrValue(FE{defaultValue}).ToFloat(); }

		[[nodiscard]] IFE UnsafeAdd(const IFE&) const;
		[[nodiscard]] IFE UnsafeSubtract(const IFE&) const;
		[[nodiscard]] IFE Negate() const;
	};
}
