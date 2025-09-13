#pragma once
#include <m2/ProxyTypes.h>

namespace m2 {
	class IFE {
		std::variant<int32_t, FE> _value{};

	public:
		IFE() = default;
		explicit IFE(const int32_t i) : _value(i) {}
		explicit IFE(FE&& fe) : _value(std::move(fe)) {}
		explicit IFE(const FE& fe) : _value(fe) {}
		explicit IFE(const pb::IFE&);

		explicit operator bool() const { return std::holds_alternative<int32_t>(_value) ? std::get<int32_t>(_value) : static_cast<bool>(std::get<FE>(_value)); }
		[[nodiscard]] bool IsInt() const { return std::holds_alternative<int32_t>(_value); }
		[[nodiscard]] bool IsFF() const { return std::holds_alternative<FE>(_value); }
		int32_t GetInt() const { return std::get<int32_t>(_value); }
		const FE& GetFF() const { return std::get<FE>(_value); }
	};
}
