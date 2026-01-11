#include <m2/GameTypes.h>
#include <m2/BuildOptions.h>

m2::IFE::IFE(const pb::IFE& ife) {
	if (ife.has_i()) {
		_value = ife.i();
	} else if (ife.has_fe()) {
		_value = FE{ife.fe()};
	}
}

m2::IFE::operator bool() const {
	return std::holds_alternative<std::monostate>(_value)
		? false
		: std::holds_alternative<int32_t>(_value) ? std::get<int32_t>(_value) : static_cast<bool>(std::get<FE>(_value));
}
m2::IFE::operator pb::IFE() const {
	pb::IFE pbIfe;
	if (IsInt()) {
		pbIfe.set_i(std::get<int32_t>(_value));
	} else if (IsFE()) {
		pbIfe.set_fe(std::get<FE>(_value).ToRawValue());
	}
	return pbIfe;
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
