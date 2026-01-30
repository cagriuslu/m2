#include <m2/component/Character.h>
#include <m2/Game.h>
#include <algorithm>

using namespace m2;

Character::Character(uint64_t object_id) : Component(object_id) {}
std::unique_ptr<const Proxy::InterCharacterMessage> Character::ExecuteInteraction(Character& initiator, std::unique_ptr<const Proxy::InterCharacterMessage>&& data) {
	if (this->onMessage) {
		return this->onMessage(*this, &initiator, data);
	}
	return {};
}
std::unique_ptr<const Proxy::InterCharacterMessage> Character::ExecuteInteraction(std::unique_ptr<const Proxy::InterCharacterMessage>&& data) {
	if (this->onMessage) {
		return this->onMessage(*this, nullptr, data);
	}
	return {};
}

int32_t Character::UnsafeAddVariable(const m2g::pb::VariableType vt, const int32_t value, const std::optional<int32_t> maxValue) {
	if (GetVariable(vt).IsNonNull() && not GetVariable(vt).IsInt()) {
		throw M2_ERROR("Variable contains a different type of value");
	}
	if (maxValue) {
		UnsafeSetVariable(vt, std::min(GetVariable(vt).GetIntOrZero() + value, *maxValue));
	} else {
		UnsafeSetVariable(vt, GetVariable(vt).GetIntOrZero() + value);
	}
	return GetVariable(vt).UnsafeGetInt();
}
FE Character::UnsafeAddVariable(const m2g::pb::VariableType vt, const FE value, const std::optional<FE> maxValue) {
	if (GetVariable(vt).IsNonNull() && not GetVariable(vt).IsFE()) {
		throw M2_ERROR("Variable contains a different type of value");
	}
	if (maxValue) {
		UnsafeSetVariable(vt, std::min(GetVariable(vt).GetFEOrZero() + value, *maxValue));
	} else {
		UnsafeSetVariable(vt, GetVariable(vt).GetFEOrZero() + value);
	}
	return GetVariable(vt).UnsafeGetFE();
}
int32_t Character::UnsafeSubtractVariable(const m2g::pb::VariableType vt, const int32_t value, const std::optional<int32_t> minValue) {
	if (GetVariable(vt).IsNonNull() && not GetVariable(vt).IsInt()) {
		throw M2_ERROR("Variable contains a different type of value");
	}
	if (minValue) {
		UnsafeSetVariable(vt, std::max(GetVariable(vt).GetIntOrZero() - value, *minValue));
	} else {
		UnsafeSetVariable(vt, GetVariable(vt).GetIntOrZero() - value);
	}
	return GetVariable(vt).UnsafeGetInt();
}
FE Character::UnsafeSubtractVariable(const m2g::pb::VariableType vt, const FE value, const std::optional<FE> minValue) {
	if (GetVariable(vt).IsNonNull() && not GetVariable(vt).IsFE()) {
		throw M2_ERROR("Variable contains a different type of value");
	}
	if (minValue) {
		UnsafeSetVariable(vt, std::max(GetVariable(vt).GetFEOrZero() - value, *minValue));
	} else {
		UnsafeSetVariable(vt, GetVariable(vt).GetFEOrZero() - value);
	}
	return GetVariable(vt).UnsafeGetFE();
}

m2g::pb::CardCategory m2::ToCategoryOfCard(const m2g::pb::CardType ct) {
	return M2_GAME.GetCard(ct).Category();
}
