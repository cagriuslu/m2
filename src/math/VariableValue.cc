#include <m2/math/VariableValue.h>
#include <m2/math/Hash.h>
#include <m2/BuildOptions.h>

using namespace m2;

VariableValue::VariableValue(const pb::VariableValue& varVal) {
	if (varVal.has_i()) {
		_value = varVal.i();
	} else if (varVal.has_l()) {
		_value = varVal.l();
	} else if (varVal.has_v()) {
		_value = varVal.v();
	} else if (varVal.has_fe()) {
		_value = FE{std::in_place, varVal.fe()};
	}
}

VariableValue::operator bool() const {
	return std::holds_alternative<std::monostate>(_value)
		? false
		: std::holds_alternative<int32_t>(_value)
			? std::get<int32_t>(_value)
			: std::holds_alternative<int64_t>(_value)
				? std::get<int64_t>(_value)
				: std::holds_alternative<m2g::pb::VariableType>(_value)
					? std::get<m2g::pb::VariableType>(_value)
					: static_cast<bool>(std::get<FE>(_value));
}
VariableValue::operator pb::VariableValue() const {
	pb::VariableValue pbIvfe;
	if (IsInt()) {
		pbIvfe.set_i(std::get<int32_t>(_value));
	} else if (IsLong()) {
		pbIvfe.set_l(std::get<int64_t>(_value));
	} else if (IsVariableType()) {
		pbIvfe.set_v(std::get<m2g::pb::VariableType>(_value));
	} else if (IsFE()) {
		pbIvfe.set_fe(std::get<FE>(_value).ToRawValue());
	}
	return pbIvfe;
}

int32_t VariableValue::Hash(const int32_t initialValue) const {
	if constexpr (not GAME_IS_DETERMINISTIC) {
		// ReSharper disable once CppDFAUnreachableCode
		throw M2_ERROR("Game is not deterministic");
	}
	// ReSharper disable once CppDFAUnreachableCode
	if (IsInt()) {
		return HashI(UnsafeGetInt(), initialValue);
	} else if (IsLong()) {
		return HashI(UnsafeGetLong(), initialValue);
	} else if (IsVariableType()) {
		return HashI(UnsafeGetVariableType(), initialValue);
	} else if (IsFE()) {
		return HashI(ToRawValue(UnsafeGetFE()), initialValue);
	} else {
		return initialValue;
	}
}

int32_t VariableValue::GetIntOrZero() const {
	return IsInt() ? UnsafeGetInt() : 0;
}
int32_t VariableValue::GetIntOrValue(const int32_t defaultValue) const {
	return std::holds_alternative<int32_t>(_value) ? std::get<int32_t>(_value) : defaultValue;
}

int64_t VariableValue::GetLongOrZero() const {
	return IsLong() ? UnsafeGetLong() : 0;
}
int64_t VariableValue::GetLongOrValue(int64_t defaultValue) const {
	return std::holds_alternative<int64_t>(_value) ? std::get<int64_t>(_value) : defaultValue;
}

int32_t VariableValue::GetVariableTypeOrZero() const {
	return IsVariableType() ? UnsafeGetVariableType() : m2g::pb::VariableType::NO_VARIABLE;
}
int32_t VariableValue::GetVariableTypeOrValue(const m2g::pb::VariableType defaultValue) const {
	return std::holds_alternative<m2g::pb::VariableType>(_value) ? std::get<m2g::pb::VariableType>(_value) : defaultValue;
}

FE VariableValue::GetFEOrZero() const {
	return IsFE() ? UnsafeGetFE() : FE::Zero();
}
FE VariableValue::GetFEOrValue(const FE defaultValue) const {
	return std::holds_alternative<FE>(_value) ? std::get<FE>(_value) : defaultValue;
}

VariableValue VariableValue::UnsafeAdd(const VariableValue& rhs) const {
	if (IsInt() && rhs.IsInt()) {
		return VariableValue{UnsafeGetInt() + rhs.UnsafeGetInt()};
	} else if (IsLong() && rhs.IsLong()) {
		return VariableValue{UnsafeGetLong() + rhs.UnsafeGetLong()};
	} else if (IsFE() && rhs.IsFE()) {
		return VariableValue{UnsafeGetFE() + rhs.UnsafeGetFE()};
	} else if (IsVariableType()) {
		throw M2_ERROR("Attempt to add to VariableType");
	} else if (IsNull()) {
		return rhs;
	} else {
		return *this;
	}
}
VariableValue VariableValue::UnsafeSubtract(const VariableValue& rhs) const {
	if (IsInt() && rhs.IsInt()) {
		return VariableValue{UnsafeGetInt() - rhs.UnsafeGetInt()};
	} else if (IsLong() && rhs.IsLong()) {
		return VariableValue{UnsafeGetLong() - rhs.UnsafeGetLong()};
	} else if (IsFE() && rhs.IsFE()) {
		return VariableValue{UnsafeGetFE() - rhs.UnsafeGetFE()};
	} else if (IsVariableType()) {
		throw M2_ERROR("Attempt to subtract from VariableType");
	} else if (IsNull()) {
		return rhs.Negate();
	} else {
		return *this;
	}
}
VariableValue VariableValue::Negate() const {
	if (IsNull()) {
		return *this;
	} else if (IsInt()) {
		return VariableValue{-UnsafeGetInt()};
	} else if (IsLong()) {
		return VariableValue{-UnsafeGetLong()};
	} else if (IsVariableType()) {
		throw M2_ERROR("Attempt to negate VariableType");
	} else {
		return VariableValue{-UnsafeGetFE()};
	}
}
