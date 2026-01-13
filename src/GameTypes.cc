#include <m2/GameTypes.h>
#include <m2/BuildOptions.h>

using namespace m2;

IFE::IFE(const pb::IFE& ife) {
	if (ife.has_i()) {
		_value = ife.i();
	} else if (ife.has_fe()) {
		_value = FE{std::in_place, ife.fe()};
	}
}

IFE::operator bool() const {
	return std::holds_alternative<std::monostate>(_value)
		? false
		: std::holds_alternative<int32_t>(_value)
			? std::get<int32_t>(_value)
			: static_cast<bool>(std::get<FE>(_value));
}
IFE::operator pb::IFE() const {
	pb::IFE pbIfe;
	if (IsInt()) {
		pbIfe.set_i(std::get<int32_t>(_value));
	} else if (IsFE()) {
		pbIfe.set_fe(std::get<FE>(_value).ToRawValue());
	}
	return pbIfe;
}

IFE IFE::UnsafeAdd(const IFE& rhs) const {
	if (IsInt() && rhs.IsInt()) {
		return IFE{UnsafeGetInt() + rhs.UnsafeGetInt()};
	} else if (IsFE() && rhs.IsFE()) {
		return IFE{UnsafeGetFE() + rhs.UnsafeGetFE()};
	} else if (IsNull()) {
		return rhs;
	} else {
		return *this;
	}
}
IFE IFE::UnsafeSubtract(const IFE& rhs) const {
	if (IsInt() && rhs.IsInt()) {
		return IFE{UnsafeGetInt() - rhs.UnsafeGetInt()};
	} else if (IsFE() && rhs.IsFE()) {
		return IFE{UnsafeGetFE() - rhs.UnsafeGetFE()};
	} else if (IsNull()) {
		return rhs.Negate();
	} else {
		return *this;
	}
}
IFE IFE::Negate() const {
	if (IsNull()) {
		return *this;
	} else if (IsInt()) {
		return IFE{-UnsafeGetInt()};
	} else {
		return IFE{-UnsafeGetFE()};
	}
}

int32_t IFE::GetIntOrZero() const {
	return IsInt() ? UnsafeGetInt() : 0;
}
FE IFE::GetFEOrZero() const {
	return IsFE() ? UnsafeGetFE() : FE::Zero();
}
