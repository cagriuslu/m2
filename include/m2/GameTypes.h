#pragma once
#include <m2/math/primitives/Exact.h>
#include <m2/math/primitives/Float.h>
#include <m2/ProxyTypes.h>

namespace m2 {
	class IFE {
		std::variant<std::monostate, int32_t, FE> _value{};

	public:
		IFE() = default;
		explicit IFE(const int32_t i) : _value(i) {}
		explicit IFE(FE&& fe) : _value(std::move(fe)) {}
		explicit IFE(const FE& fe) : _value(fe) {}
		explicit IFE(const pb::IFE&);

		explicit operator bool() const;
		[[nodiscard]] bool IsInt() const;
		[[nodiscard]] bool IsFF() const;
		int32_t GetInt() const;
		FE GetFF() const;
	};
}
