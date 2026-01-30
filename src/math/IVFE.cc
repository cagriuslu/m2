#include <m2/math/IVFE.h>
#include <m2/math/Hash.h>
#include <m2/BuildOptions.h>

using namespace m2;

IVFE::IVFE(const pb::IVFE& ivfe) {
	if (ivfe.has_i()) {
		_value = ivfe.i();
	} else if (ivfe.has_l()) {
		_value = ivfe.l();
	} else if (ivfe.has_v()) {
		_value = ivfe.v();
	} else if (ivfe.has_fe()) {
		_value = FE{std::in_place, ivfe.fe()};
	}
}

IVFE::operator bool() const {
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
IVFE::operator pb::IVFE() const {
	pb::IVFE pbIvfe;
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

int32_t IVFE::Hash(const int32_t initialValue) const {
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

int32_t IVFE::GetIntOrZero() const {
	return IsInt() ? UnsafeGetInt() : 0;
}
int32_t IVFE::GetIntOrValue(const int32_t defaultValue) const {
	return std::holds_alternative<int32_t>(_value) ? std::get<int32_t>(_value) : defaultValue;
}

int64_t IVFE::GetLongOrZero() const {
	return IsLong() ? UnsafeGetLong() : 0;
}
int64_t IVFE::GetLongOrValue(int64_t defaultValue) const {
	return std::holds_alternative<int64_t>(_value) ? std::get<int64_t>(_value) : defaultValue;
}

int32_t IVFE::GetVariableTypeOrZero() const {
	return IsVariableType() ? UnsafeGetVariableType() : m2g::pb::VariableType::NO_VARIABLE;
}
int32_t IVFE::GetVariableTypeOrValue(const m2g::pb::VariableType defaultValue) const {
	return std::holds_alternative<m2g::pb::VariableType>(_value) ? std::get<m2g::pb::VariableType>(_value) : defaultValue;
}

FE IVFE::GetFEOrZero() const {
	return IsFE() ? UnsafeGetFE() : FE::Zero();
}
FE IVFE::GetFEOrValue(const FE defaultValue) const {
	return std::holds_alternative<FE>(_value) ? std::get<FE>(_value) : defaultValue;
}

IVFE IVFE::UnsafeAdd(const IVFE& rhs) const {
	if (IsInt() && rhs.IsInt()) {
		return IVFE{UnsafeGetInt() + rhs.UnsafeGetInt()};
	} else if (IsLong() && rhs.IsLong()) {
		return IVFE{UnsafeGetLong() + rhs.UnsafeGetLong()};
	} else if (IsFE() && rhs.IsFE()) {
		return IVFE{UnsafeGetFE() + rhs.UnsafeGetFE()};
	} else if (IsVariableType()) {
		throw M2_ERROR("Attempt to add to VariableType");
	} else if (IsNull()) {
		return rhs;
	} else {
		return *this;
	}
}
IVFE IVFE::UnsafeSubtract(const IVFE& rhs) const {
	if (IsInt() && rhs.IsInt()) {
		return IVFE{UnsafeGetInt() - rhs.UnsafeGetInt()};
	} else if (IsLong() && rhs.IsLong()) {
		return IVFE{UnsafeGetLong() - rhs.UnsafeGetLong()};
	} else if (IsFE() && rhs.IsFE()) {
		return IVFE{UnsafeGetFE() - rhs.UnsafeGetFE()};
	} else if (IsVariableType()) {
		throw M2_ERROR("Attempt to subtract from VariableType");
	} else if (IsNull()) {
		return rhs.Negate();
	} else {
		return *this;
	}
}
IVFE IVFE::Negate() const {
	if (IsNull()) {
		return *this;
	} else if (IsInt()) {
		return IVFE{-UnsafeGetInt()};
	} else if (IsLong()) {
		return IVFE{-UnsafeGetLong()};
	} else if (IsVariableType()) {
		throw M2_ERROR("Attempt to negate VariableType");
	} else {
		return IVFE{-UnsafeGetFE()};
	}
}
