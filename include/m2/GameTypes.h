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
		// ReSharper disable once CppNonExplicitConvertingConstructor
		IFE(const int32_t i) : _value(i) {}
		// ReSharper disable once CppNonExplicitConvertingConstructor
		IFE(FE&& fe) : _value(fe) {}
		// ReSharper disable once CppNonExplicitConvertingConstructor
		IFE(const FE& fe) : _value(fe) {}
		// ReSharper disable once CppNonExplicitConvertingConstructor
		IFE(const pb::IFE&);

		explicit operator bool() const;
		explicit operator pb::IFE() const;

		[[nodiscard]] bool IsNull() const { return std::holds_alternative<std::monostate>(_value); }
		[[nodiscard]] bool IsNonNull() const { return not IsNull(); }
		[[nodiscard]] bool IsInt() const { return std::holds_alternative<int32_t>(_value); }
		[[nodiscard]] bool IsFE() const { return std::holds_alternative<FE>(_value); }
		[[nodiscard]] int32_t UnsafeGetInt() const { return std::get<int32_t>(_value); }
		[[nodiscard]] FE UnsafeGetFE() const { return std::get<FE>(_value); }

		[[nodiscard]] IFE UnsafeAdd(const IFE&) const;
		[[nodiscard]] IFE UnsafeSubtract(const IFE&) const;
		[[nodiscard]] IFE Negate() const;

		[[nodiscard]] int32_t GetIntOrZero() const;
		[[nodiscard]] FE GetFEOrZero() const;
	};
}
