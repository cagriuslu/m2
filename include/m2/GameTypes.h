#pragma once
#include <m2/ProxyTypes.h>

namespace m2 {
	class IFF {
		std::variant<int32_t, FF> _value{};

	public:
		IFF() = default;
		explicit IFF(const int32_t i) : _value(i) {}
		explicit IFF(FF&& ff) : _value(std::move(ff)) {}
		explicit IFF(const FF& ff) : _value(ff) {}
		explicit IFF(const pb::IFF&);

		explicit operator bool() const { return std::holds_alternative<int32_t>(_value) ? std::get<int32_t>(_value) : static_cast<bool>(std::get<FF>(_value)); }
		[[nodiscard]] bool IsInt() const { return std::holds_alternative<int32_t>(_value); }
		[[nodiscard]] bool IsFF() const { return std::holds_alternative<FF>(_value); }
		int32_t GetInt() const { return std::get<int32_t>(_value); }
		const FF& GetFF() const { return std::get<FF>(_value); }
	};
}
