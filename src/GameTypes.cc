#include <m2/GameTypes.h>
#include <m2/BuildOptions.h>

m2::IFE::IFE(const pb::IFE& ife) {
	if (ife.has_i()) {
		_value = ife.i();
	} else if (ife.has_f()) {
		if constexpr (GAME_IS_DETERMINISTIC) {
			throw M2_ERROR("Deterministic game shouldn't contain an IFE of type float");
		}
		_value = FE{ife.f()};
	} else if (ife.has_e6()) {
		if constexpr (not GAME_IS_DETERMINISTIC) {
			throw M2_ERROR("Non-deterministic game shouldn't contain an IFE of type exact");
		}
		_value = FE::FromProtobufRepresentation(ife.e6());
	} else {
		_value = 0;
	}
}

m2::IFE::operator bool() const {
	return std::holds_alternative<std::monostate>(_value)
		? false
		: std::holds_alternative<int32_t>(_value) ? std::get<int32_t>(_value) : static_cast<bool>(std::get<FE>(_value));
}
bool m2::IFE::IsZero() const {
	return !static_cast<bool>(*this);
}
bool m2::IFE::IsInt() const {
	return std::holds_alternative<int32_t>(_value);
}
bool m2::IFE::IsFE() const {
	return std::holds_alternative<FE>(_value);
}
int32_t m2::IFE::GetInt() const {
	return std::holds_alternative<std::monostate>(_value) ? 0 : std::get<int32_t>(_value);
}
m2::FE m2::IFE::GetFE() const {
	return std::holds_alternative<std::monostate>(_value) ? FE::Zero() : std::get<FE>(_value);
}
