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
	} else if (varVal.has_lv()) {
		_value.emplace<std::vector<int64_t>>(varVal.lv().vec().cbegin(), varVal.lv().vec().cend());
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
					: std::holds_alternative<FE>(_value)
						? static_cast<bool>(std::get<FE>(_value))
						: not std::get<std::vector<int64_t>>(_value).empty();
}
VariableValue::operator pb::VariableValue() const {
	pb::VariableValue p;
	if (IsInt()) {
		p.set_i(std::get<int32_t>(_value));
	} else if (IsLong()) {
		p.set_l(std::get<int64_t>(_value));
	} else if (IsVariableType()) {
		p.set_v(std::get<m2g::pb::VariableType>(_value));
	} else if (IsFE()) {
		p.set_fe(std::get<FE>(_value).ToRawValue());
	} else if (IsLongVector()) {
		const auto& lv = std::get<std::vector<int64_t>>(_value);
		p.mutable_lv()->mutable_vec()->Add(lv.cbegin(), lv.cend());
	}
	return p;
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
	} else if (IsLongVector()) {
		int32_t hash = initialValue;
		for (const auto l : UnsafeGetLongVector()) {
			hash = HashI(l, hash);
		}
		return hash;
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

const std::vector<int64_t>& VariableValue::GetLongVectorOrEmpty() const {
	static constexpr std::vector<int64_t> empty;
	return IsLongVector() ? UnsafeGetLongVector() : empty;
}
const std::vector<int64_t>& VariableValue::GetLongVectorOrDefault(const std::vector<int64_t>& defaultValue) const {
	return IsLongVector() ? UnsafeGetLongVector() : defaultValue;
}

VariableValue VariableValue::UnsafeAdd(const VariableValue& rhs) const {
	if (IsInt() && rhs.IsInt()) {
		return VariableValue{UnsafeGetInt() + rhs.UnsafeGetInt()};
	} else if (IsLong() && rhs.IsLong()) {
		return VariableValue{UnsafeGetLong() + rhs.UnsafeGetLong()};
	} else if (IsFE() && rhs.IsFE()) {
		return VariableValue{UnsafeGetFE() + rhs.UnsafeGetFE()};
	} else if (IsNull()) {
		return rhs;
	} else if (rhs.IsNull()) {
		return *this;
	} else {
		throw M2_ERROR("Attempt to add imcompatible types");
	}
}
VariableValue VariableValue::UnsafeSubtract(const VariableValue& rhs) const {
	if (IsInt() && rhs.IsInt()) {
		return VariableValue{UnsafeGetInt() - rhs.UnsafeGetInt()};
	} else if (IsLong() && rhs.IsLong()) {
		return VariableValue{UnsafeGetLong() - rhs.UnsafeGetLong()};
	} else if (IsFE() && rhs.IsFE()) {
		return VariableValue{UnsafeGetFE() - rhs.UnsafeGetFE()};
	} else if (IsNull()) {
		return rhs.Negate();
	} else if (rhs.IsNull()) {
		return *this;
	} else {
		throw M2_ERROR("Attempt to subtract imcompatible types");
	}
}
VariableValue VariableValue::Negate() const {
	if (IsNull()) {
		return *this;
	} else if (IsInt()) {
		return VariableValue{-UnsafeGetInt()};
	} else if (IsLong()) {
		return VariableValue{-UnsafeGetLong()};
	} else if (IsFE()) {
		return VariableValue{-UnsafeGetFE()};
	} else if (IsNull()) {
		return *this;
	} else {
		throw M2_ERROR("Attempt to negate an imcompatible type");
	}
}
